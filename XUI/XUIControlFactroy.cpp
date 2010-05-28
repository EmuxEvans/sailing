#include <string.h>
#include "XUIControlFactroy.h"

struct {
	XUIControlFactroy* pCF;
	const char* pClassName;
}          g_CFs[100];
static int g_CFCount = 0;

XUIControlFactroy* Get(const char* pClassName)
{
	for(int i=0; i<g_CFCount; i++) {
		if(strcmp(pClassName, g_CFs[i].pClassName)==0) return g_CFs[i].pCF;
	}
	return NULL;
}

XUIControlFactroy::XUIControlFactroy(const char* pClassName)
{
	g_CFs[g_CFCount].pCF		= this;
	g_CFs[g_CFCount].pClassName	= pClassName;
	g_CFCount++;
}

XUIControlFactroy::~XUIControlFactroy()
{
}
