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

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	if(height==0) {
		height = 1;
	}

	_width = width;
	_height = height;
}

void* sk_image_wbmp();
void* sk_image_libbmp();
void* sk_image_libico();

SkBitmap g_Nono;

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
	glViewport(0, 0, _width, _height);
	glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glDisable(GL_TEXTURE_2D);
	//XUI_DrawScene();

	glDisable(GL_CULL_FACE);
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
	//_gXUI.Render(&_gXUIDevice);
	glEnable(GL_DEPTH_TEST);

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

SkGLCanvas Canvas;
SkDevice* pDevice;
//	SkDevice* pDevice = Canvas.createDevice(SkBitmap::kARGB_8888_Config, 100, 100, true, true);

static const unsigned TEMP_COORD_COUNT = 100;
static float g_tempCoords[TEMP_COORD_COUNT*2];
static float g_tempNormals[TEMP_COORD_COUNT*2];

static const int CIRCLE_VERTS = 8*4;
static float g_circleVerts[CIRCLE_VERTS*2];

inline unsigned int RGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	return (r) | (g << 8) | (b << 16) | (a << 24);
}

static void drawPolygon(const float* coords, unsigned numCoords, float r, unsigned int col)
{
	if (numCoords > TEMP_COORD_COUNT) numCoords = TEMP_COORD_COUNT;
	
	for (unsigned i = 0, j = numCoords-1; i < numCoords; j=i++)
	{
		const float* v0 = &coords[j*2];
		const float* v1 = &coords[i*2];
		float dx = v1[0] - v0[0];
		float dy = v1[1] - v0[1];
		float d = sqrtf(dx*dx+dy*dy);
		if (d > 0)
		{
			d = 1.0f/d;
			dx *= d;
			dy *= d;
		}
		g_tempNormals[j*2+0] = dy;
		g_tempNormals[j*2+1] = -dx;
	}
	
	for (unsigned i = 0, j = numCoords-1; i < numCoords; j=i++)
	{
		float dlx0 = g_tempNormals[j*2+0];
		float dly0 = g_tempNormals[j*2+1];
		float dlx1 = g_tempNormals[i*2+0];
		float dly1 = g_tempNormals[i*2+1];
		float dmx = (dlx0 + dlx1) * 0.5f;
		float dmy = (dly0 + dly1) * 0.5f;
		float	dmr2 = dmx*dmx + dmy*dmy;
		if (dmr2 > 0.000001f)
		{
			float	scale = 1.0f / dmr2;
			if (scale > 10.0f) scale = 10.0f;
			dmx *= scale;
			dmy *= scale;
		}
		g_tempCoords[i*2+0] = coords[i*2+0]+dmx*r;
		g_tempCoords[i*2+1] = coords[i*2+1]+dmy*r;
	}
	
	unsigned int colTrans = RGBA(col&0xff, (col>>8)&0xff, (col>>16)&0xff, 0);
	
	glBegin(GL_TRIANGLES);
	
	glColor4ubv((GLubyte*)&col);
	
	for (unsigned i = 0, j = numCoords-1; i < numCoords; j=i++)
	{
		glVertex2fv(&coords[i*2]);
		glVertex2fv(&coords[j*2]);
		glColor4ubv((GLubyte*)&colTrans);
		glVertex2fv(&g_tempCoords[j*2]);
		
		glVertex2fv(&g_tempCoords[j*2]);
		glVertex2fv(&g_tempCoords[i*2]);
		
		glColor4ubv((GLubyte*)&col);
		glVertex2fv(&coords[i*2]);
	}
	
	glColor4ubv((GLubyte*)&col);
	for (unsigned i = 2; i < numCoords; ++i)
	{
		glVertex2fv(&coords[0]);
		glVertex2fv(&coords[(i-1)*2]);
		glVertex2fv(&coords[i*2]);
	}
	
	glEnd();
}

static void drawRect(float x, float y, float w, float h, float fth, unsigned int col)
{
	float verts[4*2] =
	{
		x, y,
		x+w, y,
		x+w, y+h,
		x, y+h,
	};
	drawPolygon(verts, 4, fth, col);
}

int XUI_AppMain()
{

	MSG		msg;
	BOOL	done = FALSE;
	fullscreen = FALSE;

	if(!CreateGLWindow("XUIMain", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, fullscreen)) {
		return 0;
	}

//	SkGraphics::Init();

	//XUI_AppInit();
	RECT rctClient;
	GetClientRect(hWnd, &rctClient);
	Canvas.setViewport(rctClient.right-rctClient.left, rctClient.bottom-rctClient.top);

	while(!done) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if(msg.message==WM_QUIT) {
				done = TRUE;
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else {

	glShadeModel(GL_SMOOTH);
	glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

			glViewport(0, 0, _width, _height);
			glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

			glDisable(GL_TEXTURE_2D);
			glDisable(GL_CULL_FACE);
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

			drawRect(10, 10, 100, 100, 0, 0xffffffff);

			SkPaint Paint;
	        Paint.setAntiAlias(true);
			Paint.setColor(SkColorSetARGB(255, 0, 0, 0));
			SkRect Rect;
			Rect.fLeft = 0;
			Rect.fTop = 0;
			Rect.fRight = 500;
			Rect.fBottom = 500;
			glEnableClientState(GL_VERTEX_ARRAY);
//			Canvas.drawRect(Rect, Paint);
			char szText[] = "welcome to china";
			Canvas.drawText(szText, sizeof(szText)-1, 100, 100, Paint);
			Paint.setColor(SkColorSetARGB(50, 0, 0, 0));
			Canvas.drawRoundRect(Rect, 10, 10, Paint);
			Canvas.drawBitmap(g_Nono, 200, 200);

			glDisable(GL_TEXTURE_2D);
			glDisable(GL_CULL_FACE);
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
			drawRect(200, 10, 100, 100, 0, 0xef1fefff);


			{
				glDisable(GL_TEXTURE_2D);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_COLOR_ARRAY);
				glEnableClientState(GL_VERTEX_ARRAY);
				glShadeModel(GL_FLAT);
				struct { float x, y; } vertex[4];
				vertex[0].x = 300; vertex[0].y = 300;
				vertex[1].x = 300; vertex[1].y = 400;
				vertex[2].x = 400; vertex[2].y = 400;
				vertex[3].x = 400; vertex[3].y = 300;
				glColor4ub(200, 200, 200, 128);
				GetLastError();
				glVertexPointer(2, GL_FLOAT, 0, &vertex);
				GetLastError();
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
				GetLastError();
			}

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

	//XUI_AppFinal();
	SkGraphics::Term();

	KillGLWindow();

	return (int)msg.wParam;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return XUI_AppMain();
}
