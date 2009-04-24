#pragma once

class IGameFES;

#define SGPLAYERTEAM_MEMBER_MAX		5

class CSGTeam;
class CSGPlayer;
class CSGPet;
class CSGConnection;
class CSGBattleField;

class CSGTeam
{
public:
	CSGTeam();
	~CSGTeam();

	unsigned int GetLeadActorId() { return m_nLeader; }

	CSGPlayer* GetMember(int nIndex);
	CSGPlayer* GetMember(unsigned int nActorId);

	bool Join(CSGPlayer* pPlayer);
	bool Leave(CSGPlayer* pPlayer);

	void OnData(CSGPlayer* pPlayer, const CmdData* pCmdData);
	void SendData(const void* pData, unsigned int nSize);

private:
	unsigned int m_nLeader;
	CSGPlayer* m_Members[SGPLAYERTEAM_MEMBER_MAX];
};

class CSGPlayer : public CSGRole
{
	friend class CSGPet;
public:
	CSGPlayer(CSGConnection* pConnection, unsigned int nPlayerId);
	virtual ~CSGPlayer();

	CSGConnection* GetConnection() {
		return m_pConnection;
	}

	unsigned int GetPlayerId() const {
		return m_nPlayerId;
	}
	const char* GetName() const {
		return m_szName;
	}
	CSGBattleField* GetBattleField() {
		return m_pBattle;
	}
	CSGTeam* GetTeam() {
		return m_pTeam;
	}
	CSGPet* GetPet() {
		return m_pPet;
	}
	const SGPLAYER_EQUIPMENTS& GetEqiups() {
		return m_Equips;
	}

	bool InitPlayer();
	bool QuitPlayer();

	bool GetViewData(CSGPlayer* pPlayer, SGPLAYER_VIEWDATA* pData);
	bool GetViewDataInTeam(SGPLAYER_VIEWDATA_INTEAM* pData);

	bool DropItem(int nIndex);
	bool Equip(int nIndex, int nSolt);

	virtual void OnNotify(const CmdData* pCmdData);
	virtual void OnAction(const CmdData* pCmdData);
	virtual void OnPassive(const CmdData* pCmdData);

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
	CSGConnection* m_pConnection;
	unsigned int m_nPlayerId;
	char m_szName[100];

	CSGBattleField* m_pBattle;
	CSGTeam* m_pTeam;
	CSGPet* m_pPet;

	SGPLAYER_EQUIPMENTS	m_Equips;

	ItemUData	m_Equip[SGEQUIPMENT_COUNT];
	ItemUData	m_Bag[SGBAG_MAX];
	ItemUData	m_Warehouse[SGWAREHOUSE_MAX];

};
