#pragma once

typedef struct CmdData {
	unsigned int nCmd;
	unsigned int nSource;
	unsigned int nTarget;
	const void* pData;
} CmdData;
