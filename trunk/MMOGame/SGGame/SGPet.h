#pragma once

class CSGPlayer;

class CSGPet : public CSGRole
{
public:
	CSGPet(CSGPlayer* pOwner);
	virtual ~CSGPet();

	bool GetViewData(CSGPlayer* pPlayer, SGPET_VIEWDATA* pData);

	CSGPlayer* GetOwner() {
		return m_pOwner;
	}

private:
	CSGPlayer* m_pOwner;
};
