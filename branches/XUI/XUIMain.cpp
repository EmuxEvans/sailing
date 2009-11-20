#include <windows.h>
#include <gl\glew.h>

#include "XUIDevice.h"
#include "XUIWidget.h"
#include "XUIControls.h"
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

class XUI_Delegate
{
public:
	void Invoke();
};

class XUI_IDelegate {
public:
	XUI_IDelegate() {
		m_pDelegate = NULL;
		m_pNext = NULL;
		m_pPrev = NULL;
	}
	~XUI_IDelegate() {
	}

	virtual void Invoke() = 0;

	XUI_Delegate* m_pDelegate;
	XUI_IDelegate* m_pNext;
	XUI_IDelegate* m_pPrev;
};

template<class T>
class XUI_DelegateImpl {
public:
	virtual void Invoke() {
		(m_pThis->*m_pInvoke)();
	}

	T* m_pThis;
	void (*T::m_pInvoke)();
};
