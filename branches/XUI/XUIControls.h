#pragma once

class XUIButton : public XUIWidget
{
public:
	XUIButton(bool bManualFree);
	XUIButton(const char* pText, int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~XUIButton();

	void SetText(const char* pText) { m_pText = pText; }

protected:
	virtual void onRender(XUIDevice* pDevice);

	virtual void onMouseEnter() { m_bOver = true; }
	virtual void onMouseLeave() { m_bOver = false; }

private:
	bool m_bOver;
	const char* m_pText;
};

class XUILabel : public XUIWidget
{
public:
	XUILabel(bool bManualFree);
	XUILabel(const char* pText, int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~XUILabel();

	void SetText(const char* pText) { m_pText = pText; }

protected:
	virtual void onRender(XUIDevice* pDevice);

private:
	const char* m_pText;
};

class XUIScrollPanel : public XUIWidget
{
public:
	XUIScrollPanel(bool bManualFree);
	XUIScrollPanel(const char* pText, int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~XUIScrollPanel();

	bool AddWidget(XUIWidget* pWidget);

	void SetText(const char* pText) { m_pText = pText; }

protected:
	virtual void onRender(XUIDevice* pDevice);

	virtual void onMouseMove(const XUIPoint& Point);
	virtual void onMouseWheel(const XUIPoint& Point, int _rel);
	virtual void onMouseButtonPressed(const XUIPoint& Point, unsigned short nId);
	virtual void onMouseButtonReleased(const XUIPoint& Point, unsigned short nId);

	virtual void OnSizeChange(int nWidth, int nHeight);

private:
	const char* m_pText;
	int m_nCaptureY, m_nCaptureScroll;
	int m_nWidgetsHeight;
	XUIWidget m_ClientArea;

};
