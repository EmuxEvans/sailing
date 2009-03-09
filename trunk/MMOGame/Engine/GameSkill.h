#pragma once

typedef struct SkillSData {
	unsigned int	nSkillId;
	unsigned int	nClassId;
	unsigned char	aData[100];
} SkillSData;

typedef struct SkillUData {
	unsigned int nSkillId;
	unsigned char aData[100];
} SkillUData;

template<class TAreaActor>
class ISkillLogic
{
public:
	virtual ~ISkillLogic() { }

	virtual void Tick(unsigned int nCurTime, unsigned int nDelta, TAreaActor* pActor, const SkillSData* pSData, SkillUData* pUData) = 0;
	virtual void Action(TAreaActor* pActor, const SkillSData* pSData, SkillUData* pUData) = 0;
};
