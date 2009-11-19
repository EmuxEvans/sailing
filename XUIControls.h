#pragma once

class XUIButton : public XUIWidget
{
public:
	XUIButton();
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
	XUILabel();
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
	XUIScrollPanel();
	XUIScrollPanel(const char* pText, int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~XUIScrollPanel();

	bool AddWidget(XUIWidget* pWidget);

	void SetText(const char* pText) { m_pText = pText; }

protected:
	virtual void onRender(XUIDevice* pDevice);

	virtual void MouseWheel(const XUIPoint& Point, int _rel);

private:
	const char* m_pText;
	int m_nScroll, m_nScrollCount;

};
