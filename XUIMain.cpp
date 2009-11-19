#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>

#include "XUIMain.h"
#include "XUIDevice.h"
#include "XUIDeviceGL.h"
#include "XUIWidget.h"
#include "XUIControls.h"

#define SCREEN_WIDTH		800
#define SCREEN_HEIGHT		600
#define SCREEN_DEPTH		32

static HDC			hDC = NULL;
static HGLRC		hRC = NULL;
static HWND			hWnd = NULL;
static HINSTANCE	hInstance;

static bool	keys[256];
static bool	active = TRUE;
static bool	fullscreen = TRUE;
static GLsizei _width, _height;

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static XUI _gXUI;
static XUIDeviceGL _gXUIDevice;

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	if(height==0) {
		height = 1;
	}

	_width = width;
	_height = height;

	_gXUI.Reset(width, height);
	if(!_gXUIDevice.ResetDevice(width, height)) {
	}
}

int InitGL()
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	return TRUE;
}

int DrawGL()
{
	glViewport(0, 0, _width, _height);
	glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)_width/(GLfloat)_height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	DrawScene();
*/
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, _width, _height, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	DrawUI();
	glEnable(GL_DEPTH_TEST);

	return TRUE;									
}

GLvoid KillGLWindow(GLvoid)
{
	if(fullscreen) {
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(TRUE);
	}

	if(hRC) {
		if(!wglMakeCurrent(NULL, NULL)) {
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if(!wglDeleteContext(hRC)) {
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;
	}

	if(hDC && !ReleaseDC(hWnd, hDC)) {
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;
	}

	if(hWnd && !DestroyWindow(hWnd)) {
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;
	}

	if(!UnregisterClass("OpenGL", hInstance)) {
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;
	}
}

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;
	WNDCLASS	wc;
	DWORD		dwExStyle;
	DWORD		dwStyle;
	RECT		WindowRect;
	WindowRect.left = (long)0;
	WindowRect.right = (long)width;
	WindowRect.top = (long)0;
	WindowRect.bottom = (long)height;

	fullscreen = fullscreenflag;

	hInstance			= GetModuleHandle(NULL);
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= (WNDPROC)WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= "OpenGL";

	if(!RegisterClass(&wc)) {
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	if(fullscreen) {
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= width;
		dmScreenSettings.dmPelsHeight	= height;
		dmScreenSettings.dmBitsPerPel	= bits;
		dmScreenSettings.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		if(ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL) {
			if(MessageBox(NULL, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "NeHe GL", MB_YESNO|MB_ICONEXCLAMATION)==IDYES) {
				fullscreen = FALSE;
			} else {
				MessageBox(NULL, "Program Will Now Close.", "ERROR", MB_OK|MB_ICONSTOP);
				return FALSE;
			}
		}
	}

	if(fullscreen) {
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
		ShowCursor(FALSE);
	} else {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	hWnd = CreateWindowEx(dwExStyle, "OpenGL", title, dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, 0, 0, WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top, NULL, NULL, hInstance, NULL);
	if(!hWnd) {
		KillGLWindow();
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		bits,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	if (!(hDC = GetDC(hWnd))) {
		KillGLWindow();
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd))) {
		KillGLWindow();
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!SetPixelFormat(hDC, PixelFormat, &pfd)) {
		KillGLWindow();
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!(hRC = wglCreateContext(hDC))) {
		KillGLWindow();
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!wglMakeCurrent(hDC, hRC)) {
		KillGLWindow();
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	ReSizeGLScene(width, height);

	if(!InitGL()) {
		KillGLWindow();
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	case WM_ACTIVATE:
		active = !HIWORD(wParam)?TRUE:FALSE;
		return 0;
	case WM_SYSCOMMAND:
		if(wParam==SC_SCREENSAVE || wParam==SC_MONITORPOWER) return 0;
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_KEYDOWN:
		keys[wParam] = TRUE;
		return 0;
	case WM_KEYUP:
		keys[wParam] = FALSE;
		return 0;
	case WM_MOUSEMOVE:
		_gXUI.MouseMove(XUIPoint(LOWORD(lParam), HIWORD(lParam)));
		break;
	case WM_LBUTTONDOWN:
		_gXUI.MouseButtonPressed(XUIPoint(LOWORD(lParam), HIWORD(lParam)), XUI_INPUT::MOUSE_LBUTTON);
		break;
	case WM_RBUTTONDOWN:
		_gXUI.MouseButtonPressed(XUIPoint(LOWORD(lParam), HIWORD(lParam)), XUI_INPUT::MOUSE_RBUTTON);
		break;
	case WM_MBUTTONDOWN:
		_gXUI.MouseButtonPressed(XUIPoint(LOWORD(lParam), HIWORD(lParam)), XUI_INPUT::MOUSE_MBUTTON);
		break;
	case WM_LBUTTONUP:
		_gXUI.MouseButtonReleased(XUIPoint(LOWORD(lParam), HIWORD(lParam)), XUI_INPUT::MOUSE_LBUTTON);
		break;
	case WM_RBUTTONUP:
		_gXUI.MouseButtonReleased(XUIPoint(LOWORD(lParam), HIWORD(lParam)), XUI_INPUT::MOUSE_RBUTTON);
		break;
	case WM_MBUTTONUP:
		_gXUI.MouseButtonReleased(XUIPoint(LOWORD(lParam), HIWORD(lParam)), XUI_INPUT::MOUSE_MBUTTON);
		break;
	case WM_LBUTTONDBLCLK:
		_gXUI.MouseButtonDoubleClick(XUIPoint(LOWORD(lParam), HIWORD(lParam)), XUI_INPUT::MOUSE_LBUTTON);
		break;
	case WM_RBUTTONDBLCLK:
		_gXUI.MouseButtonDoubleClick(XUIPoint(LOWORD(lParam), HIWORD(lParam)), XUI_INPUT::MOUSE_RBUTTON);
		break;
	case WM_MBUTTONDBLCLK:
		_gXUI.MouseButtonDoubleClick(XUIPoint(LOWORD(lParam), HIWORD(lParam)), XUI_INPUT::MOUSE_MBUTTON);
		break;

	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static void DoTick();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG		msg;
	BOOL	done = FALSE;
	fullscreen = FALSE;

	if(!CreateGLWindow("XUIMain", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, fullscreen)) {
		return 0;
	}

	glEnable(GL_CULL_FACE);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);

	while(!done) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if(msg.message==WM_QUIT) {
				done = TRUE;
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else {

			DoTick();

			if((active && !DrawGL()) || keys[VK_ESCAPE]) {
				done = TRUE;
			} else {
				SwapBuffers(hDC);
			}

			if(keys[VK_F1]) {
				keys[VK_F1] = FALSE;
				KillGLWindow();
				fullscreen = !fullscreen;
				if(!CreateGLWindow("XUIMain", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, fullscreen)) {
					return 0;
				}
			}
		}
	}

	KillGLWindow();

	return (msg.wParam);
}

void DoTick()
{
	static bool bInit = false;
	if(!bInit) {
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
		_gXUI.GetRoot()->AddChild(pPanel);
		bInit = true;
	}

//	_gXUI.Render(NULL);
}

int DrawScene()
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

	return 0;
}

int DrawUI()
{
	_gXUIDevice.Render(_gXUI.GetRoot());
//	_gXUI.Render(&_gXUIDevice);
	return 0;
}
