#include <string.h>
#include "XUIControlFactroy.h"

struct {
	XUIControlFactroy* pCF;
	const char* pName;
}          g_CFs[100];
static int g_CFCount = 0;

XUIControlFactroy* Get(const char* pName)
{
	for(int i=0; i<g_CFCount; i++) {
		if(strcmp(pName, g_CFs[i].pName)==0) return g_CFs[i].pCF;
	}
	return NULL;
}

XUIControlFactroy::XUIControlFactroy(const char* pName)
{
	g_CFs[g_CFCount].pCF	= this;
	g_CFs[g_CFCount].pName	= pName;
	g_CFCount++;
}

XUIControlFactroy::~XUIControlFactroy()
{
}
