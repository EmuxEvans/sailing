#include <stddef.h>
#include <assert.h>

#include "XUIWidget.h"
#include "XUIDevice.h"

XUIDevice::XUIDevice()
{
}

XUIDevice::~XUIDevice()
{
}

bool XUIDevice::Render(XUIWidget* pWidget)
{
	m_nScissors = 0;
	m_rx = m_ry = 0;
	RenderBegin();
	pWidget->onRender(this);
	RenderEnd();
	assert(m_nScissors==0);
	return true;
}

bool XUIDevice::AddRect(int x, int y, int w, int h, int r, XUIColor color)
{
	OnCmdRect(x+m_rx, y+m_ry, w, h, r, color);
	return true;
}

bool XUIDevice::AddTriangle(int x, int y, int w, int h, int d, XUIColor color)
{
	OnCmdTriangle(x+m_rx, y+m_ry, w, h, d, color);
	return true;
}

bool XUIDevice::AddText(int x, int y, int align, XUIColor color, const char* text)
{
	OnCmdText(x+m_rx, y+m_ry, align, color, text);
	return true;
}

bool XUIDevice::AddBeginScissor(int x, int y, int w, int h)
{
	if(w<0) {
		m_Scissors[m_nScissors].hard = false;
		m_Scissors[m_nScissors].x = x;
		m_Scissors[m_nScissors].y = y;
		m_rx += x;
		m_ry += y;
	} else {
		OnCmdBeginScissor(x+m_rx, y+m_ry, w, h);
		m_Scissors[m_nScissors].hard = true;
	}
	m_nScissors++;
	return true;
}

bool XUIDevice::AddEndScissor()
{
	m_nScissors--;
	if(m_Scissors[m_nScissors].hard) {
		OnCmdEndScissor();
	} else {
		m_rx -= m_Scissors[m_nScissors].x;
		m_ry -= m_Scissors[m_nScissors].y;
	}
	return true;
}
