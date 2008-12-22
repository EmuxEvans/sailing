#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"

char* os_getcwd(char* path, int len)
{
	return getcwd(path, (size_t)len);
}

int os_chdir(char* path)
{
	return chdir(path);
}

os_dword atom_unix_add(os_dword volatile *p, os_dword v)
{
    asm volatile ("lock; xaddl %0,%1"
                  : "=r" (val), "=m" (*mem)
                  : "0" (val), "m" (*mem)
                  : "memory", "cc");
    return val;
}

os_dword atom_unix_sub(os_dword volatile *p, os_dword v)
{
    asm volatile ("lock; subl %1, %0"
                  : /* no output */
                  : "m" (*(mem)), "r" (val)
                  : "memory", "cc");
}

os_dword atom_unix_inc(os_dword volatile* mem)
{
    return apr_atomic_add32(mem, 1);
}

os_dword atom_unix_dec(os_dword volatile* mem)
{
    unsigned char prev;

    asm volatile ("lock; decl %0; setnz %1"
                  : "=m" (*mem), "=qm" (prev)
                  : "m" (*mem)
                  : "memory");

    return prev;
}

os_dword atom_unix_swap(os_dword volatile* mem, os_dword prev)
{
    apr_uint32_t prev = val;

    asm volatile ("xchgl %0, %1"
                  : "=r" (prev), "+m" (*mem)
                  : "0" (prev));
    return prev;
}

os_dword atom_unix_cas(os_dword volatile* mem, os_dword val, os_dword cmp)
{
    apr_uint32_t prev;

    asm volatile ("lock; cmpxchgl %1, %2"
                  : "=a" (prev)
                  : "r" (with), "m" (*(mem)), "0"(cmp)
                  : "memory", "cc");
    return prev;
}

void* atom_unix_cas_ptr(void* volatile *p, void* v, void* c)
{
    void *prev;
#if sizeof(prev) == 4
    asm volatile ("lock; cmpxchgl %2, %1"
                  : "=a" (prev), "=m" (*mem)
                  : "r" (with), "m" (*mem), "0" (cmp));
#elif sizeof(prev) == 8
    asm volatile ("lock; cmpxchgq %q2, %1"
                  : "=a" (prev), "=m" (*mem)
                  : "r" ((unsigned long)with), "m" (*mem),
                    "0" ((unsigned long)cmp));
#else
#error sizeof(void*) value not supported
#endif
}

void atom_slist_init(ATOM_SLIST_HEADER* header)
{
	header->First = NULL;
	header->Count = 0;
}

ATOM_SLIST_ENTRY* atom_slist_pop(ATOM_SLIST_HEADER* head)
{
	ATOM_SLIST_ENTRY* val;

#ifndef __x86_64__
	asm volatile( "push			%%ebx\n\t"
				  "mov			%1, %%edi\n\t"
				  "mov			(%%edi), %%eax\n\t"
				  "mov			4(%%edi), %%edx\n\t"
				  "__L__2:\n\t"
				  "or			%%eax, %%eax\n\t"
				  "je __L__1\n\t"
				  "mov			(%%eax), %%ebx\n\t"
				  "mov			%%edx, %%ecx\n\t"
				  "dec			%%ecx\n\t"
				  "lock\n\t"
				  "cmpxchg8b	(%%edi)\n\t"
				  "jne			__L__2\n\t"
				  "__L__1:\n\t"
				  "pop			%%ebx\n\t"
				  "mov			%%eax, %0\n\t"
				: "=g" (val)
				: "g" (head)
				: "eax", "ecx", "edx", "edi", "memory");
#else
	asm volatile( "push			%%rbx\n\t"
				  "mov			%1, %%rdi\n\t"
				  "mov			(%%rdi), %%rax\n\t"
				  "mov			8(%%rdi), %%rdx\n\t"
				  "__L__2:\n\t"
				  "or			%%rax, %%rax\n\t"
				  "je __L__1\n\t"
				  "mov			(%%rax), %%rbx\n\t"
				  "mov			%%rdx, %%rcx\n\t"
				  "dec			%%rcx\n\t"
				  "lock\n\t"
				  "cmpxchg16b	(%%rdi)\n\t"
				  "jne			__L__2\n\t"
				  "__L__1:\n\t"
				  "pop			%%rbx\n\t"
				  "mov			%%rax, %0\n\t"
				: "=g" (val)
				: "g" (head)
				: "rax", "rcx", "rdx", "rdi", "memory");
#endif

	return val;
}

ATOM_SLIST_ENTRY* atom_slist_push(ATOM_SLIST_HEADER* head, ATOM_SLIST_ENTRY* ListEntry)
{
	ATOM_SLIST_ENTRY* p;

	assert(((unsigned long)head&(sizeof(long)-1))==0);
	assert(((unsigned long)ListEntry&(sizeof(long)-1))==0);

#ifndef __x86_64__
	asm volatile( "push			%%ebx\n\t"
				  "mov			%2, %%ebx\n\t"
				  "mov			%1, %%edi\n\t"
				  "mov			(%%edi), %%eax\n\t"
				  "mov			4(%%edi), %%edx\n\t"
				  "__L__3:\n\t"
				  "mov			%%eax, (%%ebx)\n\t"
				  "mov			%%edx, %%ecx\n\t"
				  "inc			%%ecx\n\t"
				  "lock\n\t"
				  "cmpxchg8b	(%%edi)\n\t"
				  "jne			__L__3\n\t"
				  "pop			%%ebx\n\t"
				  "mov			%%eax, %0\n\t"
				: "=g" (p)
				: "g" (head), "g" (ListEntry)
				: "eax", "ecx", "edx", "edi", "memory");
#else
	asm volatile( "push			%%rbx\n\t"
				  "mov			%2, %%rbx\n\t"
				  "mov			%1, %%rdi\n\t"
				  "mov			(%%rdi), %%rax\n\t"
				  "mov			8(%%rdi), %%rdx\n\t"
				  "__L__3:\n\t"
				  "mov			%%rax, (%%rbx)\n\t"
				  "mov			%%rdx, %%rcx\n\t"
				  "inc			%%rcx\n\t"
				  "lock\n\t"
				  "cmpxchg16b	(%%rdi)\n\t"
				  "jne			__L__3\n\t"
				  "pop			%%rbx\n\t"
				  "mov			%%rax, %0\n\t"
				: "=g" (p)
				: "g" (head), "g" (ListEntry)
				: "rax", "rcx", "rdx", "rdi", "memory");
#endif
	return(p);
}

ATOM_SLIST_ENTRY* atom_slist_flush(ATOM_SLIST_HEADER* head)
{
	ATOM_SLIST_ENTRY* p;

	assert(((unsigned long)head&(sizeof(long)-1))==0);

#ifndef __x86_64__
	asm volatile( "push			%%ebx\n\t"
				  "mov			%1, %%edi\n\t"
				  "mov			(%%edi), %%eax\n\t"
				  "mov			4(%%edi), %%edx\n\t"
				  "xor			%%ebx, %%ebx\n\t"
				  "xor			%%edx, %%edx\n\t"
				  "__L__4:\n\t"
				  "or			%%eax, %%eax\n\t"
				  "je			__L__5\n\t"
				  "lock\n\t"
				  "cmpxchg8b	(%%edi)\n\t"
				  "jne			__L__4\n\t"
				  "__L__5:\n\t"
				  "pop			%%ebx\n\t"
				  "mov			%%eax, %0"
				: "=g" (p)
				: "g" (head)
				: "eax", "ecx", "edx", "edi", "memory");
#else
	asm volatile( "push			%%rbx\n\t"
				  "mov			%1, %%rdi\n\t"
				  "mov			(%%rdi), %%rax\n\t"
				  "mov			8(%%rdi), %%rdx\n\t"
				  "xor			%%rbx, %%rbx\n\t"
				  "xor			%%rdx, %%rdx\n\t"
				  "__L__4:\n\t"
				  "or			%%rax, %%rax\n\t"
				  "je			__L__5\n\t"
				  "lock\n\t"
				  "cmpxchg16b	(%%rdi)\n\t"
				  "jne			__L__4\n\t"
				  "__L__5:\n\t"
				  "pop			%%rbx\n\t"
				  "mov			%%rax, %0"
				: "=g" (p)
				: "g" (head)
				: "rax", "rcx", "rdx", "rdi", "memory");
#endif
	return(p);
}

int os_fileexist(const char* file)
{
	int ret;
	struct stat s;
	ret = stat(file, &s);
	return ret==0 && (s.st_mode&S_IFDIR)==0;
}

int os_isdir(const char* dir)
{
	int ret;
	struct stat s;
	ret = stat(dir, &s);
	return ret==0 && (s.st_mode&S_IFDIR)!=0;
}

int os_mkdir(const char* dir)
{
	int ret;
	struct stat s;
	char tmp[200];
	const char* cur;
	char* cur1, * cur2;

	ret = stat(dir, &s);
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

		ret = stat(tmp, &s);
		if(ret==0) {
			if((s.st_mode&S_IFDIR)==0) {
				return -1;
			} else {
				continue;
			}
		}

		ret = mkdir(tmp, 0777);
		if(ret==-1) return errno;
	}

	return 0;
}
