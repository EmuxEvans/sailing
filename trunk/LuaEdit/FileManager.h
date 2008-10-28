#pragma once

class CFileManager
{
public:
	CFileManager(CTabView* pTabView);
	~CFileManager();

	BOOL New();
	BOOL Open(LPCTSTR pFileName);
	BOOL Save(int nIndex, LPCTSTR pFileName=NULL);
	BOOL Close(int nIndex);

	void UpdateUI();

	void GenNewFileName(char* pFileName);
	BOOL NeedSave(int nIndex);
	BOOL IsNewFile(int nIndex);
	LPCTSTR GetFileName(int nIndex);

	void GotoLine(int nLine, int nIndex=-1);

protected:
	CTabView* m_pTabView;
};