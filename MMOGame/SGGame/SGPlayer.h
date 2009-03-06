#pragma once

class CSGPlayer : public CSGRole
{
public:
	CSGPlayer(unsigned int nUserId);
	virtual ~CSGPlayer();

	unsigned int GetUserId() const {
		return m_nUserId;
	}

	bool DropItem(int nIndex);
	bool Equip(int nIndex, int nSolt);

	virtual void OnNotify(const CmdData* pCmdData);
	virtual void OnAction(const CmdData* pCmdData);
	virtual void OnPassive(CSGAreaActor* pWho, const CmdData* pCmdData);

	void Process(const CmdData* pCmdData);

private:
	unsigned int m_nUserId;
	ItemUData	m_Equip[SGEQUIPMENT_COUNT];
	ItemUData	m_Bag[SGBAG_MAX];
	ItemUData	m_Warehouse[SGWAREHOUSE_MAX];
};
