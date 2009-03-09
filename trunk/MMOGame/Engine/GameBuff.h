#pragma once

typedef struct BuffSData {
	unsigned int	nBuffId;
	unsigned int	nClassId;
	unsigned char	aData[100];
} BuffSData;

typedef struct BuffUData {
	unsigned int	nBuffId;
	unsigned char	aData[100];
} BuffUData;

template<class TAreaActor>
class IBuffLogic
{
public:
	virtual ~IBuffLogic() { }

	virtual void Tick(unsigned int nCurTime, unsigned int nDelta, TAreaActor* pActor, const BuffSData* pSData, BuffUData* pUData) = 0;
};
