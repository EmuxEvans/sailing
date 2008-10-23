#pragma once

class CFileManager
{
public:
	CFileManager(CTabView* pTabView);
	~CFileManager();

	BOOL New();
	BOOL Open(LPCTSTR pFileName);
	BOOL Save(LPCTSTR pFileName=NULL);
	BOOL SaveAll();
	BOOL CloseAll();

	void UpdateUI();

protected:
	CTabView* m_pTabView;
};