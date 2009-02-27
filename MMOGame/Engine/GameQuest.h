#pragma once

typedef struct QuestStep {
	os_byte aData[100];
} QuestStep;

typedef struct QuestSData {
	os_dword nQuestId;
	os_dword nClassId;
	char sTitle[100];
	char sBody[100];
	os_byte aAcceptable[100];
	os_byte aAward[100];
	os_dword nStepCount;
	QuestStep aStep[5];
} QuestSData;

typedef struct QuestUData {
	os_dword nQuestId;
	os_dword nStep;
	os_byte aData[100];
} QuestUData;

template<class TAreaActor>
class IQuestLogic
{
public:
	virtual ~IQuestLogic() { }

	virtual bool Visible(TAreaActor* pActor, const QuestSData* pSData) = 0;
	virtual bool Acceptable(TAreaActor* pActor, const QuestSData* pSData) = 0;
	virtual bool IsCompleted(TAreaActor* pActor, const QuestSData* pSData, QuestUData* pUData) = 0;
	virtual bool Finish(TAreaActor* pActor, const QuestSData* pSData, QuestUData* pUData) = 0;
	virtual void Tick(unsigned int nCurTime, unsigned int nDelta, TAreaActor* pActor, const QuestSData* pSData, QuestUData* pUData) = 0;
};
