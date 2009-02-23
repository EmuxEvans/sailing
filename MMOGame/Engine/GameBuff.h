#pragma once

typedef struct BuffSData {
	os_dword nBuffId;
	os_dword nClassId;
	os_char aData[100];
} BuffSData;

typedef struct BuffUData {
	os_dword nBuffId;
	os_char aData[100];
} BuffUData;

template<class TAreaActor>
class IBuffLogic
{
public:
	virtual ~IBuffLogic() { }

	virtual void Tick(unsigned int nCurTime, unsigned int nDelta) = 0;
};
