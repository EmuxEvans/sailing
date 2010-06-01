#pragma once

class XUIDialog : public XUIWidget
{
public:
	XUIDialog(const char* pName="", bool bManualFree=false);
	XUIDialog(const char* pName, const char* pTitle, int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~XUIDialog();

	void SetTitleBar(bool bShow);
	void SetEdgeWidth(int nEdgeWidth);
	bool GetTitleBar() { return m_bTitleBar; }
	int GetEdgeWidth() { return m_nEdgeWidth; }

protected:
	virtual void ActiveWidget(XUIWidget* pWidget) {
		if(!HasFocus()) SetFocus();
		BringToTop();
		XUIWidget::ActiveWidget(pWidget);
	}

	virtual void OnRender(XUIDevice* pDevice);

	virtual void OnMouseMove(const XUIPoint& Point);
	virtual void OnMouseLeave();
	virtual void OnMouseButtonPressed(const XUIPoint& Point, unsigned short nId);
	virtual void OnMouseButtonReleased(const XUIPoint& Point, unsigned short nId);

private:
	XUIString m_Title;
	bool m_bInMove, m_bBarLight;
	XUIPoint m_InMovePoint;
	int m_nInMoveX, m_nInMoveY;
	bool m_bTitleBar;
	int m_nEdgeWidth;
};
