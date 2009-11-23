// test01.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Resource.h"

#include <io.h>
#include <string.h>
#include <assert.h>
#include <windows.h>
#include <commdlg.h>
#include <gl\glew.h>

#include "XUIMisc.h"
#include "XUIDelegate.h"
#include "XUIWidget.h"
#include "XUIControls.h"
#include "XUIDevice.h"
#include "XUIApp.h"

#include <vector>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return XUI_AppMain();
}

class IXUIAppEventLink {
public:
	virtual ~IXUIAppEventLink() { }

};

class XUIApp {
public:
	void AppInit();
	void AppFinal();
	void DoTick();
	void DrawScene();

public:
	void OnWelcome_Close(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
		m_pWelcome->SetVisable(!m_pWelcome->IsVisable());
	}


	void OnFileOpen_Select(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
		m_pFileList->SetVisable(!m_pFileList->IsVisable());
		if(m_pFileList->IsVisable()) {
			m_pFileList->BringToTop();
			scanDirectory("meshes", ".obj");
		}
	}

	void OnFileListClick(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
		m_pFileList->SetVisable(false);
		m_pFileName->SetText(dynamic_cast<XUIButton*>(pWidget)->GetText());
	}

	void OnFileOpen_Open(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
		int i = 0;
	}

	void OnWidgetMove(XUIWidget* pWidget, int nLeft, int nTop)
	{
		m_pFileList->SetWidgetPosition(m_pFileOpen->GetWidgetLeft()+m_nFileListOffsetX, m_pFileOpen->GetWidgetTop()+m_nFileListOffsetY);
		m_pFileList->BringToTop();
	}

	void scanDirectory(const char* pPath, const char* ext) {
		clearFiles();

		_finddata_t dir;
		char pathWithExt[MAX_PATH];
		long fh;
		strcpy(pathWithExt, pPath);
		strcat(pathWithExt, "/*");
		strcat(pathWithExt, ext);
		fh = _findfirst(pathWithExt, &dir);
		if (fh == -1L) return;
		do {
			eventMouseButtonClickImpl<XUIApp>* pEvent = new eventMouseButtonClickImpl<XUIApp>();
			XUIButton* pButton = new XUIButton("", dir.name, 0, 0, 100, 15);
			m_pFileList->AddWidget(pButton);
			pButton->_eventMouseButtonClick.Register(pEvent->R(this, &XUIApp::OnFileListClick));
			events.push_back(new eventMouseButtonClickImpl<XUIApp>);
		} while (_findnext(fh, &dir) == 0);
		_findclose(fh);
	}

	void clearFiles() {
		for(size_t i=0; i<events.size(); i++) {
			delete events[i];
		}
		m_pFileList->ClearWidgets();
		events.clear();
	}

private:
	XUIDialog* m_pWelcome;
	XUIDialog* m_pFileOpen;
	XUILabel* m_pFileName;
	XUIScrollPanel* m_pFileList;
	int m_nFileListOffsetX, m_nFileListOffsetY;
	eventMouseButtonClickImpl<XUIApp> m_eventWelcome_Close;
	eventMouseButtonClickImpl<XUIApp> m_eventFileOpen_Select;
	eventMouseButtonClickImpl<XUIApp> m_eventFileOpen_Open;
	eventMouseButtonClickImpl<XUIApp> m_eventFileOpen_Welcome;
	eventWidgetMoveImpl<XUIApp> m_eventFileListMove;

	std::vector<eventMouseButtonClickImpl<XUIApp>*> events;
};

void XUIApp::AppInit()
{
	m_pWelcome = new XUIDialog("WELCOME", "Welcome", 0, 0, 200, 100);
	XUI_GetXUI().GetRoot()->AddChild(m_pWelcome);
	m_pWelcome->CenterWidget();
	m_pWelcome->AddChild(new XUILabel("", "I'm Mr.XUI. Who are you?", 0, 0, m_pWelcome->GetClientWidth(), 20));
	m_pWelcome->AddChild(new XUIButton("CLOSE", "Close", 0, 35, m_pWelcome->GetClientWidth(), 20));
	m_pWelcome->GetWidget("CLOSE")->_eventMouseButtonClick.Register(m_eventWelcome_Close.R(this, &XUIApp::OnWelcome_Close));

	m_pFileOpen = new XUIDialog("", "Main Tool", 10, 10, 200, 110);
	XUIScrollPanel* pFileOpen = new XUIScrollPanel("PANEL", 0, 0, m_pFileOpen->GetClientWidth(), m_pFileOpen->GetClientHeight());
	m_pFileName = new XUILabel("FILE", "", 10, 15, 100, 15);
	pFileOpen->AddWidget(m_pFileName);
	m_pFileName->SetWidgetSize(m_pFileName->GetWidgetWidth()-20, m_pFileName->GetWidgetHeight());
	pFileOpen->AddChild(new XUIButton("SELECT", "...", m_pFileName->GetWidgetWidth(), m_pFileName->GetWidgetTop(), pFileOpen->GetClientWidth()-m_pFileName->GetWidgetWidth(), m_pFileName->GetWidgetHeight()));

	pFileOpen->AddWidget(new XUISlider("", "Scale", 10, 20, 100, 20));
	pFileOpen->AddWidget(new XUIButton("OPEN", "Open", 10, 20, 100, 20));
	pFileOpen->AddWidget(new XUIButton("SHOW", "Show Welcome", 10, 20, 100, 20));
	pFileOpen->GetWidget("SELECT")->_eventMouseButtonClick.Register(m_eventFileOpen_Select.R(this, &XUIApp::OnFileOpen_Select));
	pFileOpen->GetWidget("OPEN")->_eventMouseButtonClick.Register(m_eventFileOpen_Open.R(this, &XUIApp::OnFileOpen_Open));
	pFileOpen->GetWidget("SHOW")->_eventMouseButtonClick.Register(m_eventFileOpen_Welcome.R(this, &XUIApp::OnWelcome_Close));

	pFileOpen->SetWidgetSize(pFileOpen->GetWidgetWidth(), pFileOpen->GetScrollHeight());
	m_pFileOpen->SetWidgetSize(
		m_pFileOpen->GetWidgetWidth(),
		pFileOpen->GetWidgetHeight()+m_pFileOpen->GetWidgetHeight()-m_pFileOpen->GetClientHeight());

	m_pFileOpen->AddChild(pFileOpen);
	XUI_GetXUI().GetRoot()->AddChild(m_pFileOpen);

	m_pFileList = new XUIScrollPanel("", 0, 0, 150, 100);
	m_pFileList->m_bShowBoard = true;
	m_pFileList->SetVisable(false);
	XUI_GetXUI().GetRoot()->AddChild(m_pFileList);
	m_pFileList->SetClientArea(m_pFileList->GetClientLeft()+5,
		m_pFileList->GetClientTop()+5,
		m_pFileList->GetClientRight()+5,
		m_pFileList->GetClientBottom()+5);

	XUIWidget* pSelect = m_pFileOpen->GetWidget("PANEL")->GetWidget("SELECT");
	XUIPoint P(pSelect->GetWidgetLeft() + pSelect->GetWidgetWidth() + 20, pSelect->GetWidgetTop()-5);
	pSelect->GetParent()->WidgetToScreen(P, P);
	m_pFileList->GetParent()->ScreenToWidget(P, P);
	m_pFileList->SetWidgetPosition(P.x, P.y);
	m_nFileListOffsetX = P.x - m_pFileOpen->GetWidgetLeft();
	m_nFileListOffsetY = P.y - m_pFileOpen->GetWidgetTop();
	m_pFileOpen->_eventWidgetMove.Register(m_eventFileListMove.R(this, &XUIApp::OnWidgetMove));
}

void XUIApp::AppFinal()
{
}

void XUIApp::DrawScene()
{
	static GLfloat	rtri = 0;
	static GLfloat	rquad = 0;

	glLoadIdentity();
	glTranslatef(-1.5f,0.0f,-6.0f);
	glRotatef(rtri,0.0f,1.0f,0.0f);
	glBegin(GL_TRIANGLES);
		glColor3f(1.0f,0.0f,0.0f);
		glVertex3f( 0.0f, 1.0f, 0.0f);
		glColor3f(0.0f,1.0f,0.0f);
		glVertex3f(-1.0f,-1.0f, 0.0f);
		glColor3f(0.0f,0.0f,1.0f);
		glVertex3f( 1.0f,-1.0f, 0.0f);
	glEnd();
	glLoadIdentity();
	glTranslatef(1.5f,0.0f,-6.0f);
	glRotatef(rquad,1.0f,0.0f,0.0f);
	glColor3f(0.5f,0.5f,1.0f);
	glBegin(GL_QUADS);
		glVertex3f(-1.0f, 1.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f, 0.0f);
		glVertex3f( 1.0f,-1.0f, 0.0f);
		glVertex3f(-1.0f,-1.0f, 0.0f);
	glEnd();
	rtri+=0.2f;
	rquad-=0.15f;
}

void XUIApp::DoTick()
{
	XUI_GetXUI().BeginFrame();
	XUI_GetXUI().EndFrame();
}

static XUIApp _App;

void XUI_AppInit()
{
	_App.AppInit();
}

void XUI_AppFinal()
{
	_App.AppFinal();
}

void XUI_DrawScene()
{
	_App.DrawScene();
}

void XUI_DoTick()
{
	_App.DoTick();
}
