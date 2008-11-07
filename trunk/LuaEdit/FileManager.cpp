#include "StdAfx.h"

#include <stdio.h>

#include "SciLexerEdit.h"
#include "LuaEditView.h"
#include "FileManager.h"

static char* load_textfile(const char* filename)
{
	FILE* fp;
	int len, size;
	char* buf;

	fp = fopen(filename, "rt");
	if(fp==NULL) return NULL;

	fseek(fp, 0, SEEK_END);
	size = (int)ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buf = (char*)malloc(size+1);

	len = 0;
	for(;;) {
		if(fgets(buf+len, size-len, fp)==NULL) {
			break;
		}
		len += (int)strlen(buf+len);
	}
	buf[len] = '\0';

	fclose(fp);
	return buf;
}

static int save_textfile(const char* filename, char* buf, int buflen)
{
	FILE* fp;

	fp = fopen(filename, "wt");
	if(fp==NULL) return -1;

	fputs(buf, fp);

	fclose(fp);
	return 0;
}

static const char* get_filename(const char* filename)
{
	const char *p1, *p2;
	p1 = strrchr(filename, '\\');
	p2 = strrchr(filename, '/');
	if(p2>p1) p1 = p2;
	if(!p1) p1 = filename;
	else p1 += 1;
	return p1;
}

static void file_fullpath(const char* filename, char* ret)
{
	char cur_path[MAX_PATH];
	char file_path[MAX_PATH];
	char *p1, *p2;
	GetCurrentDirectoryA(sizeof(cur_path), cur_path);

	strcpy(file_path, filename);
	p1 = strrchr(file_path, '\\');
	p2 = strrchr(file_path, '/');
	if(p2>p1) p1 = p2;
	if(p1) {
		*p1 = '\0';
		SetCurrentDirectoryA(file_path);
		GetCurrentDirectoryA(sizeof(file_path), file_path);
	} else {
		strcpy(file_path, cur_path);
	}
	sprintf(ret, "%s\\%s", file_path, get_filename(filename));

	SetCurrentDirectoryA(cur_path);
}

CFileManager::CFileManager(CTabView* pTabView)
{
	m_pTabView = pTabView;
}

CFileManager::~CFileManager()
{
}

BOOL CFileManager::New()
{
	char szFileName[MAX_PATH];
	char szFullPath[MAX_PATH];
	CLuaEditView* pView = new CLuaEditView;
	pView->Create(m_pTabView->m_hWnd, CWindow::rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL, 0);
	pView->SetFont(AtlGetDefaultGuiFont());
	m_pTabView->AddPage(pView->m_hWnd, _T(""), -1, pView);
	pView->Init();
	GenNewFileName(szFileName);
	file_fullpath(szFileName, szFullPath);
	pView->SetFileName(szFileName, TRUE);
	return TRUE;
}

BOOL CFileManager::Open(LPCTSTR pFileName)
{
	char* pBuf;
	char szFullPath[MAX_PATH];

	strcpy(szFullPath, pFileName);
	file_fullpath(pFileName, szFullPath);

	for(int i=0; i<m_pTabView->GetPageCount(); i++) {
		CLuaEditView* pView;
		pView = (CLuaEditView*)m_pTabView->GetPageData(i);
		if(strcmp(pView->GetFileName(), szFullPath)==0) {
			m_pTabView->SetActivePage(i);
			return TRUE;
		}
	}

	pBuf = load_textfile(szFullPath);
	if(pBuf==NULL) return FALSE;

	CLuaEditView* pView = new CLuaEditView;
	pView->Create(m_pTabView->m_hWnd, CWindow::rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL, 0);
	pView->SetFont(AtlGetDefaultGuiFont());
	m_pTabView->AddPage(pView->m_hWnd, _T(""), -1, pView);
	pView->Init();
	pView->SetText(pBuf);
	pView->SetFileName(szFullPath);
	pView->SetSavePoint();
	free(pBuf);
	return TRUE;
}

BOOL CFileManager::Save(int nIndex, LPCTSTR pFileName)
{
	BOOL bRet;
	CLuaEditView* pView;
	pView = (CLuaEditView*)m_pTabView->GetPageData(nIndex);
	if(pFileName) {
		pView->SetFileName(pFileName);
	}
	bRet = save_textfile(pView->GetFileName(), pView->GetText(), 0)<0?FALSE:TRUE;
	if(!bRet) return FALSE;
	if(pView->IsNewFile()) {
		pView->SetFileName(pView->GetFileName());
	}
	pView->SetSavePoint();
	return TRUE;
}

BOOL CFileManager::Close(int nIndex)
{
	m_pTabView->RemovePage(nIndex);
	return TRUE;
}

void CFileManager::UpdateUI()
{
	int i;
	for(i=0; i<m_pTabView->GetPageCount(); i++) {
		CLuaEditView* pView;
		char szFileName[MAX_PATH];
		pView = (CLuaEditView*)m_pTabView->GetPageData(i);
		if(pView->GetModify() || pView->IsNewFile()) {
			sprintf(szFileName, "*%s", get_filename(pView->GetFileName()));
		} else {
			sprintf(szFileName, "%s", get_filename(pView->GetFileName()));
		}
		if(strcmp(m_pTabView->GetPageTitle(i), szFileName)!=0) {
			m_pTabView->SetPageTitle(i, szFileName);
		}
	}
}

void CFileManager::GenNewFileName(char* pFileName)
{
	int i, j;
	for(i=0; i<100; i++) {
		sprintf(pFileName, "NoName%d.lua", i);
		if(PathFileExistsA(pFileName)) continue;
		for(j=0; j<m_pTabView->GetPageCount(); j++) {
			CLuaEditView* pView;
			pView = (CLuaEditView*)m_pTabView->GetPageData(j);
			if(strcmp(pFileName, get_filename(pView->GetFileName()))==0) break;
		}
		if(j<m_pTabView->GetPageCount()) continue;
		return;
	}
	strcpy(pFileName, "fuckyou");
}

BOOL CFileManager::NeedSave(int nIndex)
{
	CLuaEditView* pView;
	pView = (CLuaEditView*)m_pTabView->GetPageData(nIndex);
	return pView->IsNewFile() || pView->GetModify();
}

BOOL CFileManager::IsNewFile(int nIndex)
{
	CLuaEditView* pView;
	pView = (CLuaEditView*)m_pTabView->GetPageData(nIndex);
	return pView->IsNewFile();
}

LPCTSTR CFileManager::GetFileName(int nIndex)
{
	CLuaEditView* pView;
	pView = (CLuaEditView*)m_pTabView->GetPageData(nIndex);
	return pView->GetFileName();
}

void CFileManager::GotoLine(int nLine, int nIndex)
{
	if(nIndex<0) nIndex = m_pTabView->GetActivePage();
	CLuaEditView* pView;
	pView = (CLuaEditView*)m_pTabView->GetPageData(nIndex);
	return pView->GotoLine(nLine);
}

void CFileManager::Find(const char* pWhat)
{
	if(m_pTabView->GetActivePage()<0) return;

	CLuaEditView* pView = (CLuaEditView*)m_pTabView->GetPageData(m_pTabView->GetActivePage());
	pView->FindNextBookmark();
}

void CFileManager::Replace(const char* pWhat, const char* pWith)
{
}

void CFileManager::ReplaceAll(const char* pWhat, const char* pWith)
{
}
