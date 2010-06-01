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
	virtual void OnRender(XUIDevice* pDevice);

	virtual void OnMouseButtonPressed(const XUIPoint& Point, unsigned short nId);
	virtual void OnMouseButtonReleased(const XUIPoint& Point, unsigned short nId);
	virtual void OnMouseButtonClick(const XUIPoint& Point, unsigned short nId);

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
	virtual void OnRender(XUIDevice* pDevice);

	virtual void OnMouseButtonClick(const XUIPoint& Point, unsigned short nId);

	bool m_bCheck;
};

class XUILabel : public XUIWidget
{
public:
	XUILabel(const char* pName="", bool bManualFree=false);
	XUILabel(const char* pName, const char* pText, int nAlign, int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~XUILabel();

	void SetText(const char* pText) { m_sText = pText; }
	const char* GetText() { return m_sText.c_str(); }

protected:
	virtual void OnRender(XUIDevice* pDevice);

private:
	XUIString m_sText;
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
	virtual void OnEraseBKGnd(XUIDevice* pDevice);

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
	virtual void OnRender(XUIDevice* pDevice);

	virtual void OnMouseMove(const XUIPoint& Point);
	virtual bool OnMouseWheel(const XUIPoint& Point, int _rel);
	virtual void OnMouseLeave();
	virtual void OnMouseButtonPressed(const XUIPoint& Point, unsigned short nId);
	virtual void OnMouseButtonReleased(const XUIPoint& Point, unsigned short nId);

private:
	XUIString m_Title;
	float m_fValue, m_fMin, m_fMax, m_fInc;
	bool m_bIn;
	int m_nCaptureX;
	float m_fCaptureValue;
};

class XUIListView;

class XUIListItem : public XUIWidget
{
	friend class XUIListView;
public:
	XUIListItem(const char* pName="", bool bManualFree=false);
	XUIListItem(const char* pName, int nLeft, int nTop, int nWidth, int nHeight);

	void SetUserData(void* pUserData) { m_pUserData = pUserData; }
	void* GetUserData() { return m_pUserData; }

	void SetSelect(bool bSelected);
	bool IsSelected() { return m_bSelected; }

	void SetText(const char* pText) { m_sText = pText; }
	const char* GetText() { return m_sText.c_str(); }

protected:
	virtual void OnRender(XUIDevice* pDevice);

	virtual void OnMouseButtonClick(const XUIPoint& Point, unsigned short nId);
	virtual void OnSizeChange(int nWidth, int nHeight);

private:
	XUIListView* m_pView;
	void* m_pUserData;
	bool m_bSelected;
	XUIString m_sText;
};

class XUIListView : public XUIWidget
{
	friend class XUIListItem;
public:
	XUIListView(const char* pName="", bool bManualFree=false);
	XUIListView(const char* pName, int nLeft, int nTop, int nWidth, int nHeight);

	void AddItem(XUIListItem* pItem);
	XUIListItem* AddString(const char* pName, const char* pText);

	bool RemoveItem(XUIListItem* pItem, bool bSilence=false);
	void RemoveAllItem();

	int GetItemCount();
	XUIListItem* GetItem(int nIndex);

	XUIListItem* GetSelectItem();
	void SetMultiSelect(bool bMultiSelect);
	bool GetMultiSelect() { return m_bMultiSelect; }

protected:
	virtual void OnEraseBKGnd(XUIDevice* pDevice);

private:
	void SetSelectItem(XUIListItem* pItem, bool bSelected);
	void AdjustItems();

	std::list<XUIListItem*> m_Items;
	bool m_bMultiSelect;
};

class XUIMenuItem : public XUIListItem
{
public:
	XUIMenuItem(const char* pText, int nCode);
	virtual ~XUIMenuItem();

	int GetCode() { return m_nCode; }

protected:
	virtual void OnRender(XUIDevice* pDevice);

private:
	int m_nCode;
};

class XUIPopMenu : public XUIListView
{
public:
	XUIPopMenu(bool bManualFree=false);
	virtual ~XUIPopMenu();

	void AddMenu(const char* pName, const char* pText, int nCode);
	void AddSeparator();
	void DelMenu(XUIMenuItem* pItem);
	void DelMenu(int nCode);

	XUIMenuItem* GetMenuItem(int nCode);

	void PopMenu(int nX=-1, int nY=-1);

};
