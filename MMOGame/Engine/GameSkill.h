#pragma once

typedef struct SkillSData {
	os_dword nSkillId;
	os_dword nClassId;
	os_byte aData[100];
} SkillSData;

typedef struct SkillUData {
	os_dword nSkillId;
	os_byte aData[100];
} SkillUData;

template<class TAreaActor>
class ISkillLogic
{
public:
	virtual ~ISkillLogic() { }

	virtual void Tick(unsigned int nCurTime, unsigned int nDelta, TAreaActor* pActor, const SkillSData* pSData, SkillUData* pUData) = 0;
	virtual void Action(TAreaActor* pActor, const SkillSData* pSData, SkillUData* pUData) = 0;
};
