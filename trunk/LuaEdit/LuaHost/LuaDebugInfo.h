#ifndef __LUADEBUGINFO_H__
#define __LUADEBUGINFO_H__

typedef struct LUADEBUG_CALLSTACK {
	char name[100];	/* (n) */
	char namewhat[100];	/* (n) `global', `local', `field', `method' */
	char what[100];	/* (S) `Lua', `C', `main', `tail' */
	char source[100];	/* (S) */
	int currentline;	/* (l) */
	int nups;		/* (u) number of upvalues */
	int linedefined;	/* (S) */
	int lastlinedefined;	/* (S) */
	char short_src[100]; /* (S) */
} LUADEBUG_CALLSTACK;

#endif
