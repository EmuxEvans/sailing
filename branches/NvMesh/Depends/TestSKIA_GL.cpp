#include <assert.h>
#include <windows.h>
#include <gl\glew.h>
#include <SkGLCanvas.h>
#include <SkGraphics.h>

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

SkBitmap g_Nono;
SkGLCanvas Canvas;

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	if(height==0) {
		height = 1;
	}

	_width = width;
	_height = height;

	Canvas.setViewport(_width, _height);
}

void* sk_image_wbmp();
void* sk_image_libbmp();
void* sk_image_libico();

#include <SkStream.h>
#include <SkImageDecoder.h>

static int InitGL()
{
	sk_image_wbmp();
	sk_image_libbmp();
	sk_image_libico();

	glShadeModel(GL_SMOOTH);
	glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	{
		SkFILEStream Stream("images/nono.bmp");

		if (!SkImageDecoder::DecodeStream(&Stream, &g_Nono)) {//, SkBitmap::kARGB_8888_Config, SkImageDecoder::kDecodeBounds_Mode, NULL)) {
			return -1;
		}

	}

	return TRUE;
}

static int DrawGL()
{

	SkPaint Paint;
    Paint.setAntiAlias(true);
	Paint.setColor(SkColorSetARGB(255, 0, 0, 0));
	SkRect Rect;
	Rect.fLeft = 0;
	Rect.fTop = 0;
	Rect.fRight = 500;
	Rect.fBottom = 500;
	glEnableClientState(GL_VERTEX_ARRAY);
	char szTextA[] = "abcdefghijklmnopqrstuvwxyz";
	char szTextB[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char szTextC[] = "0123456789";
//			char szText[] = "welcome to china";
	Paint.setTextSize(40);
	Canvas.drawText(szTextB, sizeof(szTextB)-1, 100, 200, Paint);
	Canvas.drawText(szTextA, sizeof(szTextA)-1, 100, 100, Paint);
	Canvas.drawText(szTextC, sizeof(szTextC)-1, 100, 300, Paint);

	Paint.setColor(SkColorSetARGB(255, 255, 0, 0));

	Canvas.drawText(szTextB, sizeof(szTextB)-1, 100, 400, Paint);
	Canvas.drawText(szTextA, sizeof(szTextA)-1, 100, 500, Paint);
	Canvas.drawText(szTextC, sizeof(szTextC)-1, 100, 600, Paint);
	Paint.setColor(SkColorSetARGB(50, 0, 0, 0));
	Canvas.drawRoundRect(Rect, 10, 10, Paint);
//	Canvas.drawBitmap(g_Nono, 200, 200);

	return TRUE;									
}

static GLvoid KillGLWindow(GLvoid)
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

static BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
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

	glewInit();

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

static DWORD g_mbClick = 0;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int XUI_AppMain()
{

	MSG		msg;
	BOOL	done = FALSE;
	fullscreen = FALSE;

	if(!CreateGLWindow("XUIMain", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, fullscreen)) {
		return 0;
	}

	while(!done) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if(msg.message==WM_QUIT) {
				done = TRUE;
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else {

			glDisable(GL_TEXTURE_2D);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);

			glEnable(GL_TEXTURE_2D);
			glEnable(GL_SCISSOR_TEST);
			glEnableClientState(GL_VERTEX_ARRAY);

			glViewport(0, 0, _width, _height);
			glScissor(0, 0, _width, _height);
			glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D(0, _width, _height, 0);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
/*
glShadeModel(GL_SMOOTH);
glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
glClearDepth(1.0f);
glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LEQUAL);
glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
*/
/*
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glEnable(GL_POINT_SMOOTH);
glEnable(GL_LINE_SMOOTH);
*/

/*

			glEnable(GL_TEXTURE_2D);
			glEnable(GL_SCISSOR_TEST);
			glEnableClientState(GL_VERTEX_ARRAY);

			glDisable(GL_DEPTH_TEST);

			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);

			glViewport(0, 0, _width, _height);
			glScissor(0, 0, _width, _height);
			gluOrtho2D(0, _width, _height, 0);
			glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

*/
//			glDisable(GL_TEXTURE_2D);
//			glDisable(GL_CULL_FACE);


			DrawGL();

			SwapBuffers(hDC);

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

	g_Nono.reset();

	//XUI_AppFinal();
	SkGraphics::Term();

	KillGLWindow();

	return (int)msg.wParam;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return XUI_AppMain();
}
