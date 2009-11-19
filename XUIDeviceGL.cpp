#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#include "XUIWidget.h"
#include "XUIDevice.h"
#include "XUIDeviceGL.h"

#define STBTT_malloc(x)    malloc(x)
#define STBTT_free(x)      free(x)
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "stb_truetype.h"

static const unsigned TEMP_COORD_COUNT = 100;
static float g_tempCoords[TEMP_COORD_COUNT*2];
static float g_tempNormals[TEMP_COORD_COUNT*2];

static const int CIRCLE_VERTS = 8*4;
static float g_circleVerts[CIRCLE_VERTS*2];

static stbtt_bakedchar g_cdata[96]; // ASCII 32..126 is 95 glyphs
static GLuint g_ftex = 0;

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

static void drawEllipse(float x, float y, float w, float h, float fth, unsigned int col)
{
	float verts[CIRCLE_VERTS*2];
	const float* cverts = g_circleVerts;
	float* v = verts;
	
	for (unsigned i = 0; i < CIRCLE_VERTS; ++i)
	{
		*v++ = x + cverts[i*2]*w;
		*v++ = y + cverts[i*2+1]*h;
	}
	
	drawPolygon(verts, CIRCLE_VERTS, fth, col);
}

static void drawRoundedRect(float x, float y, float w, float h, float r, float fth, unsigned int col)
{
	const unsigned n = CIRCLE_VERTS/4;
	float verts[(n+1)*4*2];
	const float* cverts = g_circleVerts;
	float* v = verts;
	
	for (unsigned i = 0; i <= n; ++i)
	{
		*v++ = x+w-r + cverts[i*2]*r;
		*v++ = y+h-r + cverts[i*2+1]*r;
	}
	
	for (unsigned i = n; i <= n*2; ++i)
	{
		*v++ = x+r + cverts[i*2]*r;
		*v++ = y+h-r + cverts[i*2+1]*r;
	}
	
	for (unsigned i = n*2; i <= n*3; ++i)
	{
		*v++ = x+r + cverts[i*2]*r;
		*v++ = y+r + cverts[i*2+1]*r;
	}
	
	for (unsigned i = n*3; i < n*4; ++i)
	{
		*v++ = x+w-r + cverts[i*2]*r;
		*v++ = y+r + cverts[i*2+1]*r;
	}
	*v++ = x+w-r + cverts[0]*r;
	*v++ = y+r + cverts[1]*r;
	
	drawPolygon(verts, (n+1)*4, fth, col);
}

static void getBakedQuad(stbtt_bakedchar *chardata, int pw, int ph, int char_index,
						 float *xpos, float *ypos, stbtt_aligned_quad *q)
{
	stbtt_bakedchar *b = chardata + char_index;
	int round_x = STBTT_ifloor(*xpos + b->xoff);
	int round_y = STBTT_ifloor(*ypos - b->yoff);
	
	q->x0 = (float)round_x;
	q->y0 = (float)round_y;
	q->x1 = (float)round_x + b->x1 - b->x0;
	q->y1 = (float)round_y - b->y1 + b->y0;
	
	q->s0 = b->x0 / (float)pw;
	q->t0 = b->y0 / (float)pw;
	q->s1 = b->x1 / (float)ph;
	q->t1 = b->y1 / (float)ph;
	
	*xpos += b->xadvance;
}


static float getTextLength(stbtt_bakedchar *chardata, const char* text)
{
	float xpos = 0;
	float len = 0;
	while (*text)
	{
		int c = (unsigned char)*text;
		if (c >= 32 && c < 128)
		{
			stbtt_bakedchar *b = chardata + c-32;
			int round_x = STBTT_ifloor((xpos + b->xoff) + 0.5);
			len = round_x + b->x1 - b->x0 + 0.5f;
			xpos += b->xadvance;
		}
		++text;
	}
	return len;
}

static void drawText(float x, float y, const char *text, int align, unsigned int col)
{
	if (!g_ftex) return;
	
	if (align == 1)
		x -= getTextLength(g_cdata, text)/2;
	else if (align == 2)
		x -= getTextLength(g_cdata, text);
	
	glColor4ub(col&0xff, (col>>8)&0xff, (col>>16)&0xff, (col>>24)&0xff);
	
	glEnable(GL_TEXTURE_2D);
	
	// assume orthographic projection with units = screen pixels, origin at top left
	glBindTexture(GL_TEXTURE_2D, g_ftex);
	
	glBegin(GL_TRIANGLES);
	
	while (*text)
	{
		int c = (unsigned char)*text;
		if (c >= 32 && c < 128)
		{			
			stbtt_aligned_quad q;
			getBakedQuad(g_cdata, 512,512, c-32, &x,&y,&q);
			
			glTexCoord2f(q.s0, q.t1);
			glVertex2f(q.x0, q.y0);
			glTexCoord2f(q.s1, q.t0);
			glVertex2f(q.x1, q.y1);
			glTexCoord2f(q.s1, q.t1);
			glVertex2f(q.x1, q.y0);
			
			glTexCoord2f(q.s0, q.t1);
			glVertex2f(q.x0, q.y0);
			glTexCoord2f(q.s0, q.t0);
			glVertex2f(q.x0, q.y1);
			glTexCoord2f(q.s1, q.t0);
			glVertex2f(q.x1, q.y1);
		}
		++text;
	}
	
	glEnd();	
	glDisable(GL_TEXTURE_2D);
}

XUIDeviceGL::XUIDeviceGL()
{
}

XUIDeviceGL::~XUIDeviceGL()
{
}

bool XUIDeviceGL::ResetDevice(int nWidth, int nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	for (unsigned i = 0; i < CIRCLE_VERTS; ++i)
	{
		float a = (float)i/(float)CIRCLE_VERTS * (float)M_PI*2;
		g_circleVerts[i*2+0] = cosf(a);
		g_circleVerts[i*2+1] = sinf(a);
	}

	// Load font.
	FILE* fp = 0;
	unsigned char* ttfBuffer = 0;
	unsigned char* bmap = 0;
	bool res = false;
	
	fp = fopen("DroidSans.ttf", "rb");
	if (!fp) goto error;
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	ttfBuffer = (unsigned char*)malloc(size); 
	if (!ttfBuffer) goto error;
	
	fread(ttfBuffer, 1, size, fp);
	fclose(fp);
	fp = 0;
	
	bmap = (unsigned char*)malloc(512*512);
	if (!bmap) goto error;
	
	stbtt_BakeFontBitmap(ttfBuffer,0, 15.0f, bmap,512,512, 32,96, g_cdata);
	
	// can free ttf_buffer at this point
	glGenTextures(1, &g_ftex);
	glBindTexture(GL_TEXTURE_2D, g_ftex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	res = true;

error:
	if (ttfBuffer)
		free(ttfBuffer);
	if (bmap)
		free(bmap);
	if (fp)
		fclose(fp);

	return res;
}

bool XUIDeviceGL::Render(XUIWidget* pWidget)
{
	m_nScissors = 0;
	m_rx = m_ry = 0;
	float pv[4];
	glGetFloatv(GL_SCISSOR_BOX, pv);
	m_nViewX = (int)pv[0];
	m_nViewY = (int)pv[1];
	m_nViewWidth  = (int)pv[2];
	m_nViewHeight = (int)pv[3];

	glScissor(m_nViewX, m_nViewY, m_nViewWidth, m_nViewHeight);
	glEnable(GL_SCISSOR_TEST);
	XUIDevice::InternalRender(pWidget);
	glDisable(GL_SCISSOR_TEST);
	assert(m_nScissors==0);
	return true;
}

void XUIDeviceGL::OnCmdRect(int x, int y, int w, int h, int r, XUIColor color)
{
	x += m_rx;
	y += m_ry;

	if(r==0) {
		drawRect((float)x+0.5f, (float)y+0.5f, (float)w-1, (float)h-1, 1.0f, color);
	} else {
		drawRoundedRect((float)x+0.5f, (float)y+0.5f, (float)w-1, (float)h-1, (float)r, 1.0f, color);
	}
}

void XUIDeviceGL::OnCmdTriangle(int x, int y, int w, int h, int d, XUIColor color)
{
	x += m_rx;
	y += m_ry;

	switch(d) {
	case 1:
		{
			const float verts[3*2] = {
				(float)x+0.5f, (float)y+0.5f,
				(float)x+0.5f+(float)w-1, (float)y+0.5f+(float)h/2-0.5f,
				(float)x+0.5f, (float)y+0.5f+(float)h-1,
			};
			drawPolygon(verts, 3, 1.0f, color);
		}
		break;
	case 2:
		{
			const float verts[3*2] = {
				(float)x+0.5f, (float)y+(float)h-1,
				(float)x+0.5f+(float)w/2-0.5f, (float)y+0.5f,
				(float)x+0.5f+(float)w-1, (float)y+0.5f+(float)h-1,
			};
			drawPolygon(verts, 3, 1.0f, color);
		}
		break;
	}
}

void XUIDeviceGL::OnCmdText(int x, int y, int align, XUIColor color, const char* text)
{
	x += m_rx;
	y += m_ry;

	drawText((float)x, (float)y, text, align, color);
}

void XUIDeviceGL::OnCmdBeginScissor(int x, int y, int w, int h)
{
	if(w>=0) {
		m_Scissors[m_nScissors].view = true;
		m_Scissors[m_nScissors].x = m_nViewX;
		m_Scissors[m_nScissors].y = m_nViewY;
		m_Scissors[m_nScissors].w = m_nViewWidth;
		m_Scissors[m_nScissors].h = m_nViewHeight;
		m_nViewX = m_rx + x;
		m_nViewY = m_ry + y;
		m_nViewWidth = w;
		m_nViewHeight = h;

		glScissor(m_nViewX, m_nHeight-m_nViewY-m_nViewHeight, m_nViewWidth, m_nViewHeight);
	} else {
		m_Scissors[m_nScissors].view = false;
		m_Scissors[m_nScissors].x = x;
		m_Scissors[m_nScissors].y = y;
		m_rx += x;
		m_ry += y;
	}
	m_nScissors++;
}

void XUIDeviceGL::OnCmdEndScissor()
{
	m_nScissors--;
	if(m_Scissors[m_nScissors].view) {
		glScissor(m_Scissors[m_nScissors].x, m_Scissors[m_nScissors].y, m_Scissors[m_nScissors].w, m_Scissors[m_nScissors].h);
		m_nViewX = m_Scissors[m_nScissors].x;
		m_nViewY = m_Scissors[m_nScissors].y;
		m_nViewWidth = m_Scissors[m_nScissors].w;
		m_nViewHeight = m_Scissors[m_nScissors].h;
		glScissor(m_nViewX, m_nHeight-m_nViewY-m_nViewHeight, m_nViewWidth, m_nViewHeight);
	} else {
		m_rx -= m_Scissors[m_nScissors].x;
		m_ry -= m_Scissors[m_nScissors].y;
	}
}
