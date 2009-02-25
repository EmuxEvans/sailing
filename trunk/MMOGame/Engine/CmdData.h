#pragma once

typedef struct CmdData {
	unsigned int nCmd;
	unsigned int nWho;
	const void* pData;
	unsigned int nSize;
} CmdData;
