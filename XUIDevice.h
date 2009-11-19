#pragma once

typedef unsigned int XUIColor;
#define XUI_RGB(r, g, b)		((((XUIColor)r&0xff))|(((XUIColor)g&0xff)<<8)|(((XUIColor)b&0xff)<<16))
#define XUI_RGBA(r, g, b, a)	((((XUIColor)r&0xff))|(((XUIColor)g&0xff)<<8)|(((XUIColor)b&0xff)<<16)|(((XUIColor)a&0xff)<<24))

class XUIWidget;

class XUIDevice
{
public:
	XUIDevice();
	virtual ~XUIDevice();

	virtual bool ResetDevice(int nWidth, int nHeight);
	virtual bool Render(XUIWidget* pWidget) = 0;

	bool AddRect(int x, int y, int w, int h, int r, XUIColor color);
	bool AddTriangle(int x, int y, int w, int h, int d, XUIColor color);
	bool AddText(int x, int y, int align, XUIColor color, const char* text);
	bool AddBeginScissor(int x, int y, int w=-1, int h=-1);
	bool AddEndScissor();

protected:
	void InternalRender(XUIWidget* pWidget);

	virtual void OnCmdRect(int x, int y, int w, int h, int r, XUIColor color) = 0;
	virtual void OnCmdTriangle(int x, int y, int w, int h, int d, XUIColor color) = 0;
	virtual void OnCmdText(int x, int y, int align, XUIColor color, const char* text) = 0;
	virtual void OnCmdBeginScissor(int x, int y, int w, int h) = 0;
	virtual void OnCmdEndScissor() = 0;
};
