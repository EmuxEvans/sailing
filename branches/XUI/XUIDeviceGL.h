#pragma once

class XUIWidget;

class XUIDeviceGL : public XUIDevice
{
public:
	XUIDeviceGL();
	virtual ~XUIDeviceGL();

	virtual bool ResetDevice(int nWidth, int nHeight);
	virtual bool Render(XUIWidget* pWidget);

protected:
	virtual void OnCmdRect(int x, int y, int w, int h, int r, XUIColor color);
	virtual void OnCmdTriangle(int x, int y, int w, int h, int d, XUIColor color);
	virtual void OnCmdText(int x, int y, int align, XUIColor color, const char* text);
	virtual void OnCmdBeginScissor(int x, int y, int w, int h);
	virtual void OnCmdEndScissor();

private:
	struct {
		int x, y;
		bool view;
		int l, t, w, h;
	} m_Scissors[20];
	int m_nScissors;
	int m_rx, m_ry;
	int m_nViewX, m_nViewY, m_nViewWidth, m_nViewHeight;
};
