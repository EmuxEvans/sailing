#pragma once

class XUIButton : public XUIWidget
{
public:
	XUIButton(const char* pName, bool bManualFree);
	XUIButton(const char* pName, const char* pText, int nLeft, int nTop, int nWidth, int nHeight);
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
	XUILabel(const char* pName, bool bManualFree);
	XUILabel(const char* pName, const char* pText, int nLeft, int nTop, int nWidth, int nHeight);
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
	XUIScrollPanel(const char* pName, bool bManualFree);
	XUIScrollPanel(const char* pName, const char* pText, int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~XUIScrollPanel();

	bool AddWidget(XUIWidget* pWidget);

	void SetText(const char* pText) { m_pText = pText; }

protected:
	virtual void onRender(XUIDevice* pDevice);

	virtual void onMouseMove(const XUIPoint& Point);
	virtual void onMouseLeave();
	virtual bool onMouseWheel(const XUIPoint& Point, int _rel);
	virtual void onMouseButtonPressed(const XUIPoint& Point, unsigned short nId);
	virtual void onMouseButtonReleased(const XUIPoint& Point, unsigned short nId);

private:
	const char* m_pText;
	bool m_bInMove, m_bBarLight;
	XUIPoint m_InMovePoint;
	int m_nInMoveX, m_nInMoveY;
	int m_nCaptureY, m_nCaptureScroll;
	int m_nWidgetsHeight;
};
