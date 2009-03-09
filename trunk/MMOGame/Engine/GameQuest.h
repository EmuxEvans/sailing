#pragma once

typedef struct QuestStep {
	unsigned char aData[100];
} QuestStep;

typedef struct QuestSData {
	unsigned int	nQuestId;
	unsigned int	nClassId;
	char			sTitle[100];
	char			sBody[100];
	unsigned char	aAcceptable[100];
	unsigned char	aAward[100];
	unsigned int	nStepCount;
	QuestStep		aStep[5];
} QuestSData;

typedef struct QuestUData {
	unsigned int nQuestId;
	unsigned int nStep;
	unsigned char aData[100];
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
