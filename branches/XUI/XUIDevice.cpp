#include <stddef.h>

#include "XUIWidget.h"
#include "XUIDevice.h"

XUIDevice::XUIDevice()
{
}

XUIDevice::~XUIDevice()
{
}

bool XUIDevice::ResetDevice(int nWidth, int nHeight)
{
	return true;
}

bool XUIDevice::AddRect(int x, int y, int w, int h, int r, XUIColor color)
{
	OnCmdRect(x, y, w, h, r, color);
	return true;
}

bool XUIDevice::AddTriangle(int x, int y, int w, int h, int d, XUIColor color)
{
	OnCmdTriangle(x, y, w, h, d, color);
	return true;
}

bool XUIDevice::AddText(int x, int y, int align, XUIColor color, const char* text)
{
	OnCmdText(x, y, align, color, text);
	return true;
}

bool XUIDevice::AddBeginScissor(int x, int y, int w, int h)
{
	OnCmdBeginScissor(x, y, w, h);
	return true;
}

bool XUIDevice::AddEndScissor()
{
	OnCmdEndScissor();
	return true;
}

void XUIDevice::InternalRender(XUIWidget* pWidget)
{
	pWidget->onRender(this);
}
