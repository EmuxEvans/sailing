/*
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
	void OnMouseButtonClick(XUIWidget* pWidget, const XUIPoint& Point, unsigned short nId) {
		int i = 0;
	}

private:
	eventMouseButtonClickImpl<XUIApp>	_eventA001;
	XUIScrollPanel* pWelcome;
};

void XUIApp::AppInit()
{
	XUIScrollPanel* pPanel = new XUIScrollPanel("PANEL", "Panel", 10, 10, 200, 500);
	pPanel->AddWidget(new XUIButton("A001", "B01", 10,  10, 100, 20));
	pPanel->AddWidget(new XUILabel ("A002", "B02", 10,  60, 100, 40));
	pPanel->AddWidget(new XUILabel ("A003", "B03", 10,  60, 100, 40));
	pPanel->AddWidget(new XUILabel ("A004", "B04", 10,  60, 100, 40));
	pPanel->AddWidget(new XUILabel ("A005", "B05", 10,  60, 100, 40));
	pPanel->AddWidget(new XUILabel ("A006", "B06", 10,  60, 100, 40));
	pPanel->AddWidget(new XUILabel ("A007", "B07", 10,  60, 100, 40));
	pPanel->AddWidget(new XUILabel ("A008", "B08", 10,  60, 100, 40));
	pPanel->AddWidget(new XUILabel ("A009", "B09", 10,  60, 100, 40));
	pPanel->AddWidget(new XUILabel ("A010", "B10", 10,  60, 100, 40));
	pPanel->AddWidget(new XUILabel ("A011", "B11", 10,  60, 100, 40));
	pPanel->AddWidget(new XUILabel ("A012", "B12", 10,  60, 100, 40));
	pPanel->AddWidget(new XUILabel ("A009", "END", 10, 160, 100, 40));
	XUI_GetXUI().GetRoot()->AddChild(pPanel);
	pPanel->GetWidget("A001")->_eventMouseButtonClick.Register(_eventA001.R(this, &XUIApp::OnMouseButtonClick));
	pWelcome = new XUIScrollPanel("WELCOME", "Welcome", 0, 0, 200, 100);
	pWelcome->AddWidget(new XUILabel("", "I'm Mr.XUI. Who are you?", 10, 20, 100, 20));
	XUI_GetXUI().GetRoot()->AddChild(pWelcome);
	pWelcome->CenterWidget();
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



int UTF16ToUTF8(uchar *out, int *outlen, const uchar *inb, int *inlenb)
{
	uchar* outstart = out;
	const uchar* processed = inb;
	uchar* outend = out + *outlen;
	ushort* in = (unsigned short*) inb;
	ushort* inend;
	unsigned int c, d, inlen;
	unsigned char *tmp;
	int bits;

	if((*inlenb % 2) == 1) (*inlenb)--;
	inlen = *inlenb / 2;
	inend = in + inlen;
	while((in < inend) && (out - outstart + 5 < *outlen))
	{
		if(isLittleEndian)
		{
			c= *in++;
		}
		else
		{
			tmp = (unsigned char *) in;
			c = *tmp++;
			c = c | (((unsigned int)*tmp) << 8);
			in++;
		}
		if((c & 0xFC00) == 0xD800)
		{
			if(in >= inend) break;
			if(isLittleEndian) { d = *in++; } 
			else 
			{
				tmp = (unsigned char *) in;
				d = *tmp++;
				d = d | (((unsigned int)*tmp) << 8);
				in++;
			}
			if((d & 0xFC00) == 0xDC00)
			{
				c &= 0x03FF;
				c <<= 10;
				c |= d & 0x03FF;
				c += 0x10000;
			}
			else
			{
				*outlen = out - outstart;
				*inlenb = processed - inb;
				return -1;
			}
		}

		if(out >= outend) break;
		if (c < 0x80) { *out++ = c; bits= -6; }
		else if(c < 0x800) { *out++ = ((c >> 6) & 0x1F) | 0xC0; bits= 0; }
		else if(c < 0x10000) { *out++ = ((c >> 12) & 0x0F) | 0xE0; bits= 6; }
		else { *out++ = ((c >> 18) & 0x07) | 0xF0; bits= 12; }

		for(; bits >= 0; bits-= 6)
		{
			if (out >= outend)
				break;
			*out++ = ((c >> bits) & 0x3F) | 0x80;
		}
		processed = (const unsigned char*) in;
	}

	*outlen = out - outstart;
	*inlenb = processed - inb;

	return(*outlen);
}

int UTF8ToUTF16(uchar *outb, int *outlen, const uchar *in, int *inlen)
{
	ushort *out = (unsigned short*) outb;
	const uchar* processed = in;
	const uchar *const instart = in;
	ushort *outstart= out, *outend;
	const uchar* inend;
	unsigned int c, d;
	int trailing;
	uchar *tmp;
	ushort tmp1, tmp2;

	if((out == NULL) || (outlen == NULL) || (inlen == NULL)) return -1;
	if(in == NULL)
	{
		*outlen = 0;
		*inlen = 0;
		return 0;
	}
	inend= in + *inlen;
	outend = out + (*outlen / 2);
	while(in < inend)
	{
		d= *in++;
		if (d < 0x80) { c= d; trailing= 0; }
		else if(d < 0xC0)
		{
			*outlen = (out - outstart) * 2;
			*inlen = processed - instart;
			return -1;
		}
		else if(d < 0xE0) { c= d & 0x1F; trailing= 1; }
		else if(d < 0xF0) { c= d & 0x0F; trailing= 2; }
		else if(d < 0xF8) { c= d & 0x07; trailing= 3; }
		else 
		{
			*outlen = (out - outstart) * 2;
			*inlen = processed - instart;
			return -1;
		}

		if(inend - in < trailing) break;

		for(; trailing; trailing--)
		{
			if((in >= inend) || (((d= *in++) & 0xC0) != 0x80))
				break;
			c <<= 6;
			c |= d & 0x3F;
		}

		if(c < 0x10000)
		{
			if(out >= outend) break;
			if(isLittleEndian) { *out++ = c; } 
			else
			{
				tmp = (unsigned char *) out;
				*tmp = c ;
				*(tmp + 1) = c >> 8 ;
				out++;
			}
		}
		else if(c < 0x110000)
		{
			if(out+1 >= outend) break;
			c -= 0x10000;
			if(isLittleEndian)
			{
				*out++ = 0xD800 | (c >> 10);
				*out++ = 0xDC00 | (c & 0x03FF);
			} 
			else
			{
				tmp1 = 0xD800 | (c >> 10);
				tmp = (unsigned char *) out;
				*tmp = (unsigned char) tmp1;
				*(tmp + 1) = tmp1 >> 8;
				out++;

				tmp2 = 0xDC00 | (c & 0x03FF);
				tmp = (unsigned char *) out;
				*tmp = (unsigned char) tmp2;
				*(tmp + 1) = tmp2 >> 8;
				out++;
			}
		}
		else break;

		processed = in;
	}

	*outlen = (out - outstart) * 2;
	*inlen = processed - instart;

	return(*outlen);
}
*/
