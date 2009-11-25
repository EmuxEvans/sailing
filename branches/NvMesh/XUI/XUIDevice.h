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

	bool Render(XUIWidget* pWidget);

	bool AddRect(int x, int y, int w, int h, int r, XUIColor color);
	bool AddTriangle(int x, int y, int w, int h, int d, XUIColor color);
	bool AddText(int x, int y, int align, XUIColor color, const char* text);
	bool AddBeginScissor(int x, int y, int w=-1, int h=-1);
	bool AddEndScissor();

protected:
	virtual void RenderBegin() = 0;
	virtual void RenderEnd() = 0;

	virtual void OnCmdRect(int x, int y, int w, int h, int r, XUIColor color) = 0;
	virtual void OnCmdTriangle(int x, int y, int w, int h, int d, XUIColor color) = 0;
	virtual void OnCmdEllipse(float x, float y, float w, float h, XUIColor color) = 0;
	virtual void OnCmdText(int x, int y, int align, XUIColor color, const char* text) = 0;
	virtual void OnCmdBeginScissor(int x, int y, int w, int h) = 0;
	virtual void OnCmdEndScissor() = 0;
	virtual void OnCmdLine(int x1, int y1, int x2, int y2, float r, XUIColor color) = 0;

private:
	struct {
		bool hard;
		int x, y;
	} m_Scissors[20];
	int m_nScissors;
	int m_rx, m_ry;
};
