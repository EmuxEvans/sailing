#pragma once

class CFileManager
{
public:
	CFileManager(CTabView* pTabView);
	~CFileManager();

	BOOL New();
	BOOL Open(LPCTSTR pFileName);
	BOOL Save(LPCTSTR pFileName);
	BOOL SaveAll();
	BOOL CloseAll();

protected:
	CTabView* m_pTabView;
};