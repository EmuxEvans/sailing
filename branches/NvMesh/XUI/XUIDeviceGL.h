#pragma once

class XUIWidget;

class XUIDeviceGL : public XUIDevice
{
public:
	XUIDeviceGL();
	virtual ~XUIDeviceGL();

	bool ResetDevice(int nWidth, int nHeight);

protected:
	virtual void RenderBegin();
	virtual void RenderEnd();

	virtual void OnCmdRect(int x, int y, int w, int h, int r, XUIColor color);
	virtual void OnCmdTriangle(int x, int y, int w, int h, int d, XUIColor color);
	virtual void OnCmdEllipse(float x, float y, float w, float h, XUIColor color);
	virtual void OnCmdText(int x, int y, int align, XUIColor color, const char* text);
	virtual void OnCmdBeginScissor(int x, int y, int w, int h);
	virtual void OnCmdEndScissor();
	virtual void OnCmdLine(int x1, int y1, int x2, int y2, float r, XUIColor color);

private:
	struct {
		int x, y, w, h;
	} m_Scissors[20];
	int m_nScissors;
	int m_nViewX, m_nViewY, m_nViewWidth, m_nViewHeight;
	int m_nWidth, m_nHeight;
};
