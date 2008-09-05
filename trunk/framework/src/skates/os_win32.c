#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"

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

	do {
		ret = WaitForSingleObject(cond->sem, INFINITE);
	} while(ret!=WAIT_OBJECT_0);

	while(1) {
		c = cond->count;
		v = CONDITION_MAKE(CONDITION_SEQ(c)+1, CONDITION_TCOUNT(c)-1, CONDITION_SCOUNT(c)-1);
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

int os_condition_broadcast(os_condition_t* cond)
{
	DWORD v, c;

	while(1) {
		c = cond->count;
		if(CONDITION_SCOUNT(c)<CONDITION_TCOUNT(c)+1) {
			v = CONDITION_MAKE(CONDITION_SEQ(c)+1, CONDITION_TCOUNT(c), CONDITION_TCOUNT(c)+1);
		} else {
			v = CONDITION_MAKE(CONDITION_SEQ(c)+1, CONDITION_TCOUNT(c), CONDITION_SCOUNT(c));
		}
		if(c==InterlockedCompareExchange(&cond->count, v, c)) break;
	}

	if(CONDITION_SCOUNT(c)<CONDITION_TCOUNT(c)+1) {
		BOOL ret;
		ret = ReleaseSemaphore(cond->sem, CONDITION_TCOUNT(c) - CONDITION_SCOUNT(c) + 1, NULL);
		return ret?0:GetLastError();
	} else {
		return 0;
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
