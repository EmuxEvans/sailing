#pragma once

class IGameFES;

class CSGPlayer : public CSGRole
{
public:
	CSGPlayer(IGameFES* pFES, unsigned int nUserId, FESClientData& ClientData);
	virtual ~CSGPlayer();

	unsigned int GetUserId() const {
		return m_nUserId;
	}
	const FESClientData& GetFESClientData() const {
		return m_ClientData;
	}

	bool DropItem(int nIndex);
	bool Equip(int nIndex, int nSolt);

	virtual void OnNotify(const CmdData* pCmdData);
	virtual void OnAction(const CmdData* pCmdData);
	virtual void OnPassive(CSGAreaActor* pWho, const CmdData* pCmdData);

	void Process(const CmdData* pCmdData);

private:
	IGameFES* m_pFES;
	unsigned int m_nUserId;
	FESClientData m_ClientData;
	ItemUData	m_Equip[SGEQUIPMENT_COUNT];
	ItemUData	m_Bag[SGBAG_MAX];
	ItemUData	m_Warehouse[SGWAREHOUSE_MAX];
};
