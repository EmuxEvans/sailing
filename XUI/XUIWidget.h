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

const int XUIALIGN_CENTER	= 0;
const int XUIALIGN_LEFT		= 0x1;
const int XUIALIGN_RIGHT	= 0x2;
const int XUIALIGN_TOP		= 0x4;
const int XUIALIGN_BOTTOM	= 0x8;

class XUIWidget : public sigslot::has_slots<>
{
	friend class XUI;
	friend class XUIDevice;
public:
	XUIWidget(const char* pName, bool bManualFree=false);
	virtual ~XUIWidget();

	XUI* GetXUI();
	XUIWidgetRoot* GetRoot();

	const XUIString& GetWidgetName() { return m_sName; }
	XUIWidget* GetWidget(const char* pName);
	void ManualFree() { m_bManualFree = true; }

	XUIWidget* GetParent() { return m_pParent; }
	XUIWidget* GetFirstChild() { return m_pFirstChild; }
	XUIWidget* GetLastChild() { return m_pLastChild; }
	XUIWidget* GetNext() { return m_pNext; }
	XUIWidget* GetPrev() { return m_pPrev; }

	void AddChild(XUIWidget* pWidget);
	void BringToTop();
	void Delete() { m_bDelete = true; }

	void SetVisable(bool bVisable=true) { m_bVisable = bVisable; }
	bool IsVisable();

	void SetEnable(bool bEnable=true) { m_bEnable = bEnable; }
	bool IsEnable();

	XUIPoint& ScreenToWidget(const XUIPoint& In, XUIPoint& Out);
	XUIPoint& WidgetToScreen(const XUIPoint& In, XUIPoint& Out);

	void CenterWidget();
	void SetWidgetRect(int nLeft, int nTop, int nWidth, int nHeight);
	void SetWidgetPosition(int nLeft, int nTop);
	void SetWidgetSize(int nWidth, int nHeight);

	int GetWidgetLeft() { return m_nLeft; }
	int GetWidgetTop() { return m_nTop; }
	int GetWidgetWidth() { return m_nWidth; }
	int GetWidgetHeight() { return m_nHeight; }

	void SetClientArea(int nLeft, int nTop, int nRight, int nBottom);

	int GetClientLeft() { return m_nClientLeft; }
	int GetClientTop() { return m_nClientTop; }
	int GetClientRight() { return m_nClientRight; }
	int GetClientBottom() { return m_nClientBottom; }
	int GetClientWidth() { return m_nWidth-m_nClientLeft-m_nClientRight; }
	int GetClientHeight() { return m_nHeight-m_nClientTop-m_nClientBottom; }

	void EnableScroll(bool bEnable);
	void SetScrollPosition(const XUIPoint& Scroll);
	void SetScrollSize(int nWidth, int nHeight);
	void AdjustScroll();

	XUIPoint GetScrollPosition() { return XUIPoint(m_nScrollX, m_nScrollY); }
	int GetScrollPositionX() { return m_nScrollX; }
	int GetScrollPositionY() { return m_nScrollY; }
	int GetScrollWidth() { return m_nScrollWidth; }
	int GetScrollHeight() { return m_nScrollHeight; }

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

	virtual void ActiveWidget(XUIWidget* pWidget);
	void Destroy();

	virtual void onRender(XUIDevice* pDevice);

	virtual void onLostFocus(XUIWidget* pNew);
	virtual bool onSetFocus(XUIWidget* pOld);
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
	XUIString m_sName;
	bool m_bManualFree;
	XUIWidget* m_pParent;
	XUIWidget* m_pNext;
	XUIWidget* m_pPrev;
	XUIWidget* m_pFirstChild;
	XUIWidget* m_pLastChild;
	bool m_bDelete, m_bEnable, m_bVisable;

	int m_nLeft, m_nTop, m_nWidth, m_nHeight;
	int m_nClientLeft, m_nClientTop, m_nClientRight, m_nClientBottom;

	bool m_bScroll;
	int m_nScrollX, m_nScrollY, m_nScrollWidth, m_nScrollHeight;
};

class XUIWidgetRoot : public XUIWidget
{
	friend class XUI;
public:
	XUIWidgetRoot() : XUIWidget("ROOT", true) { }

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
	XUIPoint m_LButtonPoint;
	XUIPoint m_RButtonPoint;
	XUIPoint m_MButtonPoint;
};
