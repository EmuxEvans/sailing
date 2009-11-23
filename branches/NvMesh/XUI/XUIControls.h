#pragma once

class XUIButton : public XUIWidget
{
public:
	XUIButton(const char* pName="", bool bManualFree=false);
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
	XUILabel(const char* pName="", bool bManualFree=false);
	XUILabel(const char* pName, const char* pText, int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~XUILabel();

	void SetText(const char* pText) { m_pText = pText; }

protected:
	virtual void onRender(XUIDevice* pDevice);

private:
	const char* m_pText;
};

class XUIPanel : public XUIWidget
{
public:
	XUIPanel(const char* pName, bool bManualFree=false);
	XUIPanel(const char* pName, int nLeft, int nTop, int nWidth, int nHeight);

	bool AddWidget(XUIWidget* pWidget);

protected:
	virtual void onRender(XUIDevice* pDevice);
	int m_nWidgetsHeight;
};

class XUIScrollPanel : public XUIPanel
{
public:
	XUIScrollPanel(const char* pName="", bool bManualFree=false);
	XUIScrollPanel(const char* pName, int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~XUIScrollPanel();

	bool m_bShowBoard;

protected:
	virtual void onRender(XUIDevice* pDevice);

	virtual void onMouseMove(const XUIPoint& Point);
	virtual bool onMouseWheel(const XUIPoint& Point, int _rel);
	virtual void onMouseButtonPressed(const XUIPoint& Point, unsigned short nId);
	virtual void onMouseButtonReleased(const XUIPoint& Point, unsigned short nId);

private:
	int m_nCaptureY, m_nCaptureScroll;
};

class XUIDialog : public XUIWidget
{
public:
	XUIDialog(const char* pName="", bool bManualFree=false);
	XUIDialog(const char* pName, const char* pTitle, int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~XUIDialog();

protected:
	virtual void onRender(XUIDevice* pDevice);

	virtual void onMouseMove(const XUIPoint& Point);
	virtual void onMouseLeave();
	virtual void onMouseButtonPressed(const XUIPoint& Point, unsigned short nId);
	virtual void onMouseButtonReleased(const XUIPoint& Point, unsigned short nId);

private:
	const char* m_pTitle;
	bool m_bInMove, m_bBarLight;
	XUIPoint m_InMovePoint;
	int m_nInMoveX, m_nInMoveY;
};
