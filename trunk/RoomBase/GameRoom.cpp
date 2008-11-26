#include <string.h>
#include <assert.h>

#include "GameRoom.h"
#include "GameRoom.inl"

static unsigned int g_nGameSeq = 0x1980;

unsigned int GenGameSeq()
{
	unsigned int nRet;
	do {
		nRet = g_nGameSeq++;
	} while((nRet&0xffff)==0);
	return nRet;
}
