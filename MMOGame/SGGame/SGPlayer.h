#pragma once

class IGameFES;
class CSGBattleField;

#define SGPLAYERTEAM_MEMBER_MAX		5

class CSGPlayerTeam;
class CSGPlayer;
class CSGPet;

class CSGPlayerTeam
{
public:
	CSGPlayerTeam();
	~CSGPlayerTeam();

	CSGPlayer* GetMember(int nIndex);

	bool Join(CSGPlayer* pPlayer);
	bool Leave(CSGPlayer* pPlayer);

	void OnData(const CmdData* pCmdData);
	void SyncData();

private:
	unsigned int m_nLeader;
	CSGPlayer* m_Members[SGPLAYERTEAM_MEMBER_MAX];
};

class CSGPlayer : public CSGRole
{
	friend class CSGPet;
public:
	CSGPlayer(IGameFES* pFES, unsigned int nPlayerId, FESClientData& ClientData);
	virtual ~CSGPlayer();

	unsigned int GetPlayerId() const {
		return m_nPlayerId;
	}
	const FESClientData& GetFESClientData() const {
		return m_ClientData;
	}
	CSGBattleField* GetBattleField() {
		return m_pBattleField;
	}
	CSGPlayerTeam* GetPlayerTeam() {
		return m_pPlayerTeam;
	}
	CSGPet* GetPet() {
		return m_pPet;
	}

	bool GetViewData(CSGPlayer* pPlayer, SGPLAYER_VIEWDATA* pData);

	bool DropItem(int nIndex);
	bool Equip(int nIndex, int nSolt);

	virtual void OnNotify(const CmdData* pCmdData);
	virtual void OnAction(const CmdData* pCmdData);
	virtual void OnPassive(CSGAreaActor* pWho, const CmdData* pCmdData);

	void Process(const CmdData* pCmdData);

protected:
	void OnPetAttach(CSGPet* pPet) {
		assert(!m_pPet);
		m_pPet = pPet;
	}
	void OnPetDetach(CSGPet* pPet) {
		assert(m_pPet==pPet);
		m_pPet = NULL;
	}

private:
	IGameFES* m_pFES;
	unsigned int m_nPlayerId;
	FESClientData m_ClientData;

	CSGBattleField* m_pBattleField;
	CSGPlayerTeam* m_pPlayerTeam;
	CSGPet* m_pPet;

	ItemUData	m_Equip[SGEQUIPMENT_COUNT];
	ItemUData	m_Bag[SGBAG_MAX];
	ItemUData	m_Warehouse[SGWAREHOUSE_MAX];

};
