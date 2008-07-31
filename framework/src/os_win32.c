#include "../inc/errcode.h"
#include "../inc/os.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <direct.h>

#define CONDITION_SEQ(v)						((v)>>16)
#define CONDITION_TCOUNT(v)						(((v)>>8)&0xff)
#define CONDITION_SCOUNT(v)						((v)&0xff)
#define CONDITION_MAKE(seq, tcount, scount)		(((((DWORD)(seq))&0xffff)<<16) | ((((DWORD)(tcount))&0xff)<<8) | (((DWORD)(scount))&0xff))

int os_condition_wait(os_condition_t* cond, os_mutex_t* mtx)
{
	DWORD v, c, ret;

	while(1) {
		c = cond->count;
		v = CONDITION_MAKE(CONDITION_SEQ(c)+1, CONDITION_TCOUNT(c)+1, CONDITION_SCOUNT(c));
		if(c==InterlockedCompareExchange(&cond->count, v, c)) break;
	}

	LeaveCriticalSection(mtx);

	ret = WaitForSingleObject(cond->sem, INFINITE);

	while(1) {
		c = cond->count;
		v = CONDITION_MAKE(CONDITION_SEQ(c)+1, CONDITION_TCOUNT(c), CONDITION_SCOUNT(c)+1);
		if(c==InterlockedCompareExchange(&cond->count, v, c)) break;
	}

	EnterCriticalSection(mtx);

	return ret==WAIT_OBJECT_0?0:GetLastError();
}

int os_condition_signal(os_condition_t* cond)
{
	DWORD v, c;

	while(1) {
		c = cond->count;
		if(CONDITION_SCOUNT(c)<CONDITION_TCOUNT(c)+1) {
			v = CONDITION_MAKE(CONDITION_SEQ(c)+1, CONDITION_TCOUNT(c), CONDITION_SCOUNT(c)+1);
		} else {
			v = CONDITION_MAKE(CONDITION_SEQ(c)+1, CONDITION_TCOUNT(c), CONDITION_SCOUNT(c));
		}
		if(c==InterlockedCompareExchange(&cond->count, v, c)) break;
	}

	if(CONDITION_SCOUNT(c)<CONDITION_TCOUNT(c)+1) {
		BOOL ret;
		ret = ReleaseSemaphore(cond->sem, 1, NULL);
		return ret?0:GetLastError();
	} else {
		return 0;
	}
}

int os_condition_boardcast(os_condition_t* cond)
{
	DWORD v, c;

	while(1) {
		c = cond->count;
		if(CONDITION_TCOUNT(c)>CONDITION_SCOUNT(c)) {
			v = CONDITION_MAKE(CONDITION_SEQ(c)+1, CONDITION_TCOUNT(c), CONDITION_TCOUNT(c));
		} else {
			v = CONDITION_MAKE(CONDITION_SEQ(c)+1, CONDITION_TCOUNT(c), CONDITION_SCOUNT(c));
		}
		if(c==InterlockedCompareExchange(&cond->count, v, c)) break;
	}

	if(CONDITION_TCOUNT(c)>CONDITION_SCOUNT(c)) {
		BOOL ret;
		ret = ReleaseSemaphore(cond->sem, CONDITION_TCOUNT(c) - CONDITION_SCOUNT(c), NULL);
		return ret?0:GetLastError();
	} else {
		return 0;
	}
}

int os_shm_create(os_shm_t* sm, const char* name, unsigned int size)
{
	HANDLE handle;
	void* base;

	handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size+4, name);
	if(handle==NULL) return (int)GetLastError();

	base = MapViewOfFile(handle, FILE_MAP_WRITE|FILE_MAP_READ, 0, 0, 4);
	if(base==NULL) {
		CloseHandle(handle);
		return (int)GetLastError();
	}
	memcpy(base, &size, sizeof(size));
	if(!UnmapViewOfFile(base)) {
		CloseHandle(handle);
		return (int)GetLastError();
	}

	*sm = handle;
	return 0;
}

int os_shm_open(os_shm_t* sm, const char* name)
{
	HANDLE handle;
	handle = OpenFileMapping(PAGE_READWRITE, FALSE, name);
	if(handle==NULL) return (int)GetLastError();
	*sm = handle;
	return 0;
}

int os_shm_close(os_shm_t sm)
{
	CloseHandle(sm);
	return 0;
}

void* os_shm_map(os_shm_t sm, unsigned int* size)
{
	void* base;
	unsigned int fsize;

	base = MapViewOfFile(sm, FILE_MAP_WRITE|FILE_MAP_READ, 0, 0, 4);
	if(base==NULL) {
		GetLastError();
		return NULL;
	}
	memcpy(&fsize, base, sizeof(fsize));
	if(!UnmapViewOfFile(base)) {
		GetLastError();
		return NULL;
	}

	base = MapViewOfFile(sm, FILE_MAP_WRITE|FILE_MAP_READ, 0, 0, fsize+4);
	if(base==NULL) {
		GetLastError();
		return NULL;
	}

	*size = fsize;
	return (char*)base + 4;
}

int os_shm_unmap(os_shm_t sm, void* ptr, unsigned int size)
{
	if(UnmapViewOfFile((char*)ptr-4)) {
		return 0;
	} else {
		return (int)GetLastError();
	}
}

int os_fileexist(const char* file)
{
	int ret;
	struct _stat s;
	ret = _stat(file, &s);
	return ret==0 && (s.st_mode&S_IFDIR)==0;
}

int os_isdir(const char* dir)
{
	int ret;
	struct _stat s;
	ret = _stat(dir, &s);
	return ret==0 && (s.st_mode&S_IFDIR)!=0;
}

int os_mkdir(const char* dir)
{
	int ret;
	struct _stat s;
	char tmp[200];
	const char* cur;
	char* cur1, * cur2;

	ret = _stat(dir, &s);
	if(ret==0) {
		return (s.st_mode&S_IFDIR)==0?-1:0;
	}

	cur = dir;

	for(;;) {
		cur1 = strchr(cur, '/');
		cur2 = strchr(cur, '\\');
		if(cur1==cur++) continue;
		if(cur1==NULL && cur2==NULL) break;
		if(cur1!=NULL && cur2!=NULL && cur1>cur2) cur1 = cur2;
		if(cur1==NULL) cur1=cur2;

		strcpy(tmp, dir);
		tmp[cur1-dir] = '\0';

		ret = _stat(tmp, &s);
		if(ret==0) {
			if((s.st_mode&S_IFDIR)==0) {
				return -1;
			} else {
				continue;
			}
		}

		ret = _mkdir(tmp);
		if(ret==-1) return errno;
	}

	return ERR_NOERROR;
}
