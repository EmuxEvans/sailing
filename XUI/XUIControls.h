#pragma once

class XUIButton : public XUIWidget
{
public:
	XUIButton(const char* pName="", bool bManualFree=false);
	XUIButton(const char* pName, const char* pText, int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~XUIButton();

	void SetText(const char* pText) { m_Caption = pText; }
	const char* GetText() { return m_Caption.c_str(); }

protected:
	virtual void onRender(XUIDevice* pDevice);

	virtual void onMouseEnter() { m_bOver = true; }
	virtual void onMouseLeave() { m_bOver = false; }

	bool m_bOver;
	XUIString m_Caption;
};

class XUICheckBox : public XUIButton
{
public:
	XUICheckBox(const char* pName="", bool bManualFree=false);
	XUICheckBox(const char* pName, const char* pText, bool bCheck, int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~XUICheckBox();

	void SetCheck(bool bCheck) { m_bCheck = bCheck; }
	bool GetCheck() { return m_bCheck; }

protected:
	virtual void onRender(XUIDevice* pDevice);

	virtual void onMouseButtonClick(const XUIPoint& Point, unsigned short nId);

	bool m_bCheck;
};

class XUILabel : public XUIWidget
{
public:
	XUILabel(const char* pName="", bool bManualFree=false);
	XUILabel(const char* pName, const char* pText, int nAlign, int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~XUILabel();

	void SetText(const char* pText) { m_Text = pText; }
	const char* GetText() { return m_Text.c_str(); }

protected:
	virtual void onRender(XUIDevice* pDevice);

private:
	XUIString m_Text;
	int m_nAlign;
};

class XUIPanel : public XUIWidget
{
public:
	XUIPanel(const char* pName, bool bManualFree=false);
	XUIPanel(const char* pName, int nLeft, int nTop, int nWidth, int nHeight);

	bool AddWidget(XUIWidget* pWidget);
	void ClearWidgets();

protected:
	virtual void onRender(XUIDevice* pDevice);
	int m_nWidgetsHeight;
};

class XUISlider : public XUIWidget
{
public:
	XUISlider(const char* pName, bool bManualFree=false);
	XUISlider(const char* pName, const char* pTitle, int nLeft, int nTop, int nWidth, int nHeight, float vmin=0.0f, float vmax=1.0f, float vinc=0.01f);

	void SetRange(float vmin, float vmax, float vinc);
	void SetValue(float fValue);
	float GetValue() { return m_fValue; }

protected:
	virtual void onRender(XUIDevice* pDevice);

	virtual void onMouseMove(const XUIPoint& Point);
	virtual bool onMouseWheel(const XUIPoint& Point, int _rel);
	virtual void onMouseLeave();
	virtual void onMouseButtonPressed(const XUIPoint& Point, unsigned short nId);
	virtual void onMouseButtonReleased(const XUIPoint& Point, unsigned short nId);

private:
	XUIString m_Title;
	float m_fValue, m_fMin, m_fMax, m_fInc;
	bool m_bIn;
	int m_nCaptureX;
	float m_fCaptureValue;
};

class XUIScrollBar : public XUIWidget
{
public:
	XUIScrollBar(const char* pName="", bool bManualFree=false);
	XUIScrollBar(const char* pName, int nLeft, int nTop, int nWidth, int nHeight, bool bVertical);

	bool IsVertical() { return m_bVertical; }
	void SetVertical(bool bVertical) { m_bVertical = bVertical; }

protected:
	virtual void onMouseMove(const XUIPoint& Point);
	virtual bool onMouseWheel(const XUIPoint& Point, int _rel);
	virtual void onMouseLeave();
	virtual void onMouseButtonPressed(const XUIPoint& Point, unsigned short nId);
	virtual void onMouseButtonReleased(const XUIPoint& Point, unsigned short nId);

private:
	bool m_bVertical;
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
	virtual void ActiveWidget(XUIWidget* pWidget);

	virtual void onRender(XUIDevice* pDevice);

	virtual void onMouseMove(const XUIPoint& Point);
	virtual void onMouseLeave();
	virtual void onMouseButtonPressed(const XUIPoint& Point, unsigned short nId);
	virtual void onMouseButtonReleased(const XUIPoint& Point, unsigned short nId);

private:
	XUIString m_Title;
	bool m_bInMove, m_bBarLight;
	XUIPoint m_InMovePoint;
	int m_nInMoveX, m_nInMoveY;
};

class XUIList;

class XUIListItem : public XUIWidget
{
	friend class XUIList;
public:
	XUIListItem(const char* pName="", bool bManualFree=false);
	XUIListItem(const char* pName, int nLeft, int nTop, int nWidth, int nHeight);

	void SetUserData(void* pUserData) { m_pUserData = pUserData; }
	void* GetUserData() { return m_pUserData; }

private:
	XUIList* m_pList;
	void* m_pUserData;
	bool m_bSelected;
};

class XUIList : public XUIScrollPanel
{
public:
	XUIList(const char* pName="", bool bManualFree=false);
	XUIList(const char* pName, int nLeft, int nTop, int nWidth, int nHeight);

	void AddItem(XUIListItem* pItem);
	XUIListItem* AddString(const char* pName);

	bool RemoveItem(XUIListItem* pItem);
	void RemoveAllItem();

	int GetItemCount();
	XUIListItem* GetItem(int nIndex);

	XUIListItem* GetSelectItem();
	void SetMultiSelect(bool bMultiSelect);
	bool GetMultiSelect() { return m_bMultiSelect; }

private:
	bool m_bMultiSelect;
};

class XUIEditLine : public XUIWidget
{
public:
	XUIEditLine(const char* pName="", bool bManualFree=false);
	XUIEditLine(const char* pName, int nLeft, int nTop, int nWidth, int nHeight);

	const XUIString& GetText() { return m_sText; }

protected:
	virtual void onRender(XUIDevice* pDevice);

	virtual void onKeyChar(unsigned short nKey, unsigned int Char);

private:
	XUIString m_sText;
};
