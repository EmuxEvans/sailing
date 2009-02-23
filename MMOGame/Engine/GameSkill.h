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

	virtual IPropertySet* GetUDataPropertySet() = 0;
	virtual IPropertySet* GetSDataPropertySet() = 0;

	virtual void Tick(unsigned int nCurTime, unsigned int nDelta) = 0;
	virtual void Action(TAreaActor* pActor, SkillUData* pData) = 0;
};
