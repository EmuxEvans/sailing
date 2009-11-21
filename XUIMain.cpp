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

void XUI_AppInit()
{
	XUIScrollPanel* pPanel = new XUIScrollPanel("PANEL", 10, 10, 200, 500);
	pPanel->AddWidget(new XUIButton("B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUIButton("B1", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("B2", 10,  60, 100, 40));
	pPanel->AddWidget(new XUIButton("B321", 10, 110, 100, 20));
	pPanel->AddWidget(new XUILabel ("B4", 10, 160, 100, 40));
	pPanel->AddWidget(new XUILabel ("END", 10, 160, 100, 40));
	XUI_GetXUI().GetRoot()->AddChild(pPanel);
}

void XUI_AppFinal()
{
}

void XUI_DrawScene()
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

void XUI_DoTick()
{
	XUI_GetXUI().BeginFrame();
	XUI_GetXUI().EndFrame();
}
/*
#include <assert.h>
#include "XUIDelegate.h"

class server {
public:
	XUI_Delegate1<bool> eventKK;

};

class my {
public:

	void dododo(bool v1)
	{
	}

	void vvvv(server* pp) {
		XUI_DelegateImpl1<my, bool>* ppp = new XUI_DelegateImpl1<my, bool>();
		pp->eventKK.Register(ppp->R(this, &my::dododo));
	}

};

void aaa()
{
	XUI_Delegate1<bool> a1;
	server s;
	my c;
	c.vvvv(&s);
	s.eventKK.Invoke(true);
}
*/
