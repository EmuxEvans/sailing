#pragma once

class XUI;
class XUIWidget;
class XUIWidgetRoot;
class XUIDevice;

XUI_DELEGATE_DEFINE1(eventLostFocus, XUIWidget*)
XUI_DELEGATE_DEFINE1(eventSetFocus, XUIWidget*)
XUI_DELEGATE_DEFINE1(eventMouseMove, const XUIPoint&)
XUI_DELEGATE_DEFINE0(eventMouseEnter)
XUI_DELEGATE_DEFINE0(eventMouseLeave)
XUI_DELEGATE_DEFINE2(eventMouseWheel, const XUIPoint&, int)
XUI_DELEGATE_DEFINE2(eventMouseButtonPressed, const XUIPoint&, unsigned short)
XUI_DELEGATE_DEFINE2(eventMouseButtonReleased, const XUIPoint&, unsigned short)
XUI_DELEGATE_DEFINE2(eventMouseButtonClick, const XUIPoint&, unsigned short)
XUI_DELEGATE_DEFINE2(eventMouseButtonDoubleClick, const XUIPoint&, unsigned short)
XUI_DELEGATE_DEFINE1(eventKeyPressed, unsigned short)
XUI_DELEGATE_DEFINE1(eventKeyReleased, unsigned short)
XUI_DELEGATE_DEFINE2(eventKeyChar, unsigned short, unsigned int)
XUI_DELEGATE_DEFINE2(eventWidgetMove, int, int)
XUI_DELEGATE_DEFINE2(eventSizeChange, int, int)

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

	eventLostFocus				_eventLostFocus;
	eventSetFocus				_eventSetFocus;
	eventMouseMove				_eventMouseMove;
	eventMouseEnter				_eventMouseEnter;
	eventMouseLeave				_eventMouseLeave;
	eventMouseWheel				_eventMouseWheel;
	eventMouseButtonPressed		_eventMouseButtonPressed;
	eventMouseButtonReleased	_eventMouseButtonReleased;
	eventMouseButtonClick		_eventMouseButtonClick;
	eventMouseButtonDoubleClick	_eventMouseButtonDoubleClick;
	eventKeyPressed				_eventKeyPressed;
	eventKeyReleased			_eventKeyReleased;
	eventKeyChar				_eventKeyChar;
	eventWidgetMove				_eventWidgetMove;
	eventSizeChange				_eventSizeChange;

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

#undef MY_COMBINE
#undef MY_COMBINE1
