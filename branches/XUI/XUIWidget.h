#pragma once

class XUIPoint
{
public:
	XUIPoint() { }
	XUIPoint(int _nX, int _nY) {
		x = _nX;
		y = _nY;
	}
	XUIPoint(const XUIPoint& Point) {
		x = Point.x;
		y = Point.y;
	}
	int x, y;
};

namespace XUI_INPUT {
	static const unsigned short MOUSE_LBUTTON = 0x1;
	static const unsigned short MOUSE_RBUTTON = 0x2;
	static const unsigned short MOUSE_MBUTTON = 0x4;
};

class XUIRect
{
public:
	XUIRect() {
		left = 0;
		top = 0;
		right = 0;
		bottom = 0;
	}

	XUIRect(int l, int t, int r, int b) {
		left = l;
		top = t;
		right = r;
		bottom = b;
	}

	XUIRect(const XUIPoint& point, int width, int height) {
		right = (left = point.x) + width;
		bottom = (top = point.y) + height;
	}

	XUIRect(const XUIPoint& topLeft, const XUIPoint& bottomRight) {
		left = topLeft.x;
		top = topLeft.y;
		right = bottomRight.x;
		bottom = bottomRight.y;
	}

	int Width() const {
		return right - left;
	}

	int Height() const {
		return bottom - top;
	}

	XUIPoint& TopLeft()
	{
		return *((XUIPoint*)this);
	}

	XUIPoint& BottomRight()
	{
		return *((XUIPoint*)this + 1);
	}

	const XUIPoint& TopLeft() const {
		return *((XUIPoint*)this);
	}

	const XUIPoint& BottomRight() const {
		return *((XUIPoint*)this + 1);
	}

	bool PointInRect(const XUIPoint& point) const {
		return point.x>=left && point.x<right && point.y>=top && point.y<bottom;
	}

	void SetRect(int x1, int y1, int x2, int y2) {
		left = x1;
		top = y1;
		right = x2;
		bottom = y2;
	}

	void SetRect(const XUIPoint& topLeft, const XUIPoint& bottomRight) {
		SetRect(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
	}

	int left, top, right, bottom;
};

class XUI;
class XUIWidget;
class XUIWidgetRoot;
class XUIDevice;

class XUIWidget
{
	friend class XUI;
	friend class XUIDevice;
public:
	XUIWidget(bool bManualFree=false);
	virtual ~XUIWidget();

	XUIWidget* GetParent() { return m_pParent; }
	XUIWidget* GetFirstChild() { return m_pFirstChild; }
	XUIWidget* GetLastChild() { return m_pLastChild; }
	XUIWidget* GetNext() { return m_pNext; }
	XUIWidget* GetPrev() { return m_pPrev; }

	void AddChild(XUIWidget* pWidget);
	void BringToTop();
	void Delete() { m_bDelete = true; }

	XUI* GetXUI();
	XUIWidgetRoot* GetRoot();

	void SetVisable(bool bVisable=true) { m_bVisable = bVisable; }
	bool IsVisable();

	void SetEnable(bool bEnable=true) { m_bEnable = bEnable; }
	bool IsEnable();

	XUIPoint& ScreenToWidget(const XUIPoint& In, XUIPoint& Out);
	XUIPoint& WidgetToScreen(const XUIPoint& In, XUIPoint& Out);

	void SetWidgetRect(int nLeft, int nTop, int nWidth, int nHeight);
	void SetWidgetPosition(int nLeft, int nTop);
	void SetWidgetSize(int nWidth, int nHeight);

	int GetWidgetLeft() { return m_nLeft; }
	int GetWidgetTop() { return m_nTop; }
	int GetWidgetWidth() { return m_nWidth; }
	int GetWidgetHeight() { return m_nHeight; }

	void EnableScroll(bool bEnable);
	void SetScrollPosition(const XUIPoint& Scroll);
	void SetScrollSize(int nWidth, int nHeight);
	void AdjustScroll();

	const XUIPoint& GetScrollPosition() { return m_Scroll; }
	int GetScrollWidth() { return m_nScrollWidth; }
	int GetScrollHeight() { return m_nScrollHeight; }

	void ManualFree() { m_bManualFree = true; }

protected:

	void Destroy();

	virtual void onRender(XUIDevice* pDevice);

	virtual void onLostFocus(XUIWidget* pNew);
	virtual void onSetFocus(XUIWidget* pOld);
	virtual void onMouseMove(const XUIPoint& Point);
	virtual void onMouseEnter();
	virtual void onMouseLeave();
	virtual bool onMouseWheel(const XUIPoint& Point, int _rel);
	virtual void onMouseButtonPressed(const XUIPoint& Point, unsigned short nId);
	virtual void onMouseButtonReleased(const XUIPoint& Point, unsigned short nId);
	virtual void onMouseButtonClick(const XUIPoint& Point, unsigned short nId);
	virtual void onMouseButtonDoubleClick(const XUIPoint& Point, unsigned short nId);
	virtual void onKeyPressed(unsigned short nKey);
	virtual void onKeyReleased(unsigned short nKey);
	virtual void onKeyChar(unsigned short nKey, unsigned int Char);

	virtual void OnWidgetMove(int nLeft, int nTop);
	virtual void OnSizeChange(int nWidth, int nHeight);

private:
	bool m_bManualFree;
	XUIWidget* m_pParent;
	XUIWidget* m_pNext;
	XUIWidget* m_pPrev;
	XUIWidget* m_pFirstChild;
	XUIWidget* m_pLastChild;
	bool m_bDelete, m_bEnable, m_bVisable;

	int m_nLeft, m_nTop, m_nWidth, m_nHeight;

	bool m_bScroll;
	XUIPoint m_Scroll;
	int m_nScrollWidth, m_nScrollHeight;
};

class XUIWidgetRoot : public XUIWidget
{
	friend class XUI;
public:
	XUIWidgetRoot() : XUIWidget(true) { }

	XUI* GetXUI() { return m_pXUI; }

protected:
	XUI* m_pXUI;
};

class XUI
{
public:
	XUI();
	~XUI();

	XUIWidgetRoot* GetRoot() { return &m_Root; }
	XUIWidget* GetFocus() { return m_pFocus; }
	XUIWidget* GetOver() { return m_pOver; }

	XUIWidget* GetWidget(const XUIPoint& Point);

	void SetCapture(XUIWidget* pWidget, bool bEnable);
	void SetFocus(XUIWidget* pWidget);

	void MouseMove(const XUIPoint& Point);
	void MouseWheel(const XUIPoint& Point, int _rel);
	void MouseButtonPressed(const XUIPoint& Point, unsigned short nId);
	void MouseButtonReleased(const XUIPoint& Point, unsigned short nId);
	void MouseButtonClick(const XUIPoint& Point, unsigned short nId);
	void MouseButtonDoubleClick(const XUIPoint& Point, unsigned short nId);
	void KeyPressed(unsigned short nKey);
	void KeyReleased(unsigned short nKey);
	void KeyChar(unsigned short nKey, unsigned int Char);

	void BeginFrame();
	void EndFrame();
	void Render(XUIDevice* pDevice);
	void Reset(int nWidth, int nHeight);

private:
	XUIWidgetRoot m_Root;
	XUIWidget* m_pFocus;
	XUIWidget* m_pOver;
	XUIWidget* m_pCapture;
};
