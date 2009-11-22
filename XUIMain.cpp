#include <assert.h>
#include <windows.h>
#include <gl\glew.h>

#include "XUIMisc.h"
#include "XUIDelegate.h"
#include "XUIWidget.h"
#include "XUIControls.h"
#include "XUIDevice.h"
#include "XUIApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return XUI_AppMain();
}

class XUIApp {
public:
	void AppInit();
	void AppFinal();
	void DoTick();
	void DrawScene();

public:
	void onMouseButtonClick(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
		int i = 0;
	}

private:
	eventMouseButtonClickImpl<XUIApp>	_eventA001;
};

void XUIApp::AppInit()
{
	XUIScrollPanel* pPanel = new XUIScrollPanel("PANEL", "Panel", 10, 10, 200, 500);
	pPanel->AddWidget(new XUIButton("A001", "B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("A002", "B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("A003", "B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("A004", "B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("A005", "B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("A006", "B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("A007", "B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("A008", "B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("A009", "B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("A010", "B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("A011", "B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("A012", "B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("A013", "B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("A014", "B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("A015", "B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("A016", "B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("A017", "B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("A018", "B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("A019", "B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("A020", "B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("A021", "B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("A022", "B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("A023", "B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("A024", "B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("A025", "B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("A026", "B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("A027", "B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("A028", "B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("A029", "B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("A030", "B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("A031", "B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("A032", "B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("A033", "B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("A034", "B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("A035", "B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("A036", "B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUILabel ("A037", "END", 10, 160, 100, 40));
	XUI_GetXUI().GetRoot()->AddChild(pPanel);

	pPanel->GetWidget("A001")->_eventMouseButtonClick.Register(_eventA001.R(this, &XUIApp::onMouseButtonClick));
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
