#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#include "XUIMisc.h"
#include "XUIDelegate.h"
#include "XUIWidget.h"
#include "XUIControls.h"
#include "XUIDevice.h"

static const int BUTTON_HEIGHT = 20;
static const int SLIDER_HEIGHT = 20;
static const int SLIDER_MARKER_WIDTH = 10;
static const int CHECK_SIZE = 8;
static const int DEFAULT_SPACING = 4;
static const int TEXT_HEIGHT = 8;
static const int SCROLL_AREA_PADDING = 6;
static const int INTEND_SIZE = 5;
static const int AREA_HEADER = 30;
static const int SLIDER_WIDTH = 8;

XUIButton::XUIButton(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	m_bOver = false;
}

XUIButton::XUIButton(const char* pName, const char* pText, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName, nLeft, nTop, nWidth, nHeight)
{
	m_bOver = false;
	SetText(pText);
}

XUIButton::~XUIButton()
{
}

void XUIButton::OnRender(XUIDevice* pDevice)
{
	pDevice->AddRect(0, 0, GetWidgetWidth(), GetWidgetHeight(), GetWidgetHeight()/2-1, XUI_RGBA(128, 128, 128, IsEnable()&&m_bOver?196:96));

	if(IsEnable())
		pDevice->AddText(GetWidgetWidth()/2, GetWidgetHeight()/2-TEXT_HEIGHT/2-1, XUIALIGN_CENTER, m_bOver?XUI_RGBA(255,196,0,255):XUI_RGBA(255,255,255,200), m_Caption.c_str());
	else
		pDevice->AddText(GetWidgetWidth()/2, GetWidgetHeight()/2-TEXT_HEIGHT/2-1, XUIALIGN_CENTER, XUI_RGBA(128,128,128,200), m_Caption.c_str());
}

XUICheckBox::XUICheckBox(const char* pName, bool bManualFree) : XUIButton(pName, bManualFree)
{
	m_bCheck = false;
}

XUICheckBox::XUICheckBox(const char* pName, const char* pText, bool bCheck, int nLeft, int nTop, int nWidth, int nHeight) : XUIButton(pName, pText, nLeft, nTop, nWidth, nHeight)
{
	m_bCheck = bCheck;
}

XUICheckBox::~XUICheckBox()
{
}

void XUICheckBox::OnRender(XUIDevice* pDevice)
{
	const int cy = GetWidgetHeight() / 2;

	pDevice->AddRect(0, cy-CHECK_SIZE/2-3, CHECK_SIZE+6, CHECK_SIZE+6, 4, XUI_RGBA(128,128,128, m_bOver?196:96));
	if(m_bCheck) {
		if(IsEnable())
			pDevice->AddRect(3, cy-CHECK_SIZE/2, CHECK_SIZE, CHECK_SIZE, 4, XUI_RGBA(255,255,255,m_bOver?255:200));
		else
			pDevice->AddRect(3, cy-CHECK_SIZE/2, CHECK_SIZE, CHECK_SIZE, 4, XUI_RGBA(128,128,128,200));
	}
	if(IsEnable()) {
		pDevice->AddText(CHECK_SIZE+10, cy-TEXT_HEIGHT/2-1, XUIALIGN_RIGHT, m_bOver?XUI_RGBA(255,196,0,255):XUI_RGBA(255,255,255,200), m_Caption.c_str());
	} else {
		pDevice->AddText(CHECK_SIZE+10, cy-TEXT_HEIGHT/2-1, XUIALIGN_RIGHT, XUI_RGBA(128,128,128,200), m_Caption.c_str());
	}
}

void XUICheckBox::OnMouseButtonClick(const XUIPoint& Point, unsigned short nId)
{
	m_bCheck = !m_bCheck;
	XUIButton::OnMouseButtonClick(Point, nId);
}

XUILabel::XUILabel(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	m_nAlign = XUIALIGN_CENTER;
}

XUILabel::XUILabel(const char* pName, const char* pText, int nAlign, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName, nLeft, nTop, nWidth, nHeight)
{
	SetText(pText);
	m_nAlign = nAlign;
}

XUILabel::~XUILabel()
{
}

void XUILabel::OnRender(XUIDevice* pDevice)
{
	switch(m_nAlign) {
	case XUIALIGN_LEFT:
		pDevice->AddText(0, GetWidgetHeight()/2-TEXT_HEIGHT/2, XUIALIGN_RIGHT, XUI_RGBA(250, 250, 250, 255), m_sText.c_str());
		break;
	case XUIALIGN_RIGHT:
		pDevice->AddText(GetWidgetWidth(), GetWidgetHeight()/2-TEXT_HEIGHT/2, XUIALIGN_LEFT, XUI_RGBA(250, 250, 250, 255), m_sText.c_str());
		break;
	case XUIALIGN_CENTER:
	default:
		pDevice->AddText(GetWidgetWidth()/2, GetWidgetHeight()/2-TEXT_HEIGHT/2, m_nAlign, XUI_RGBA(250, 250, 250, 255), m_sText.c_str());
	}
}

XUIPanel::XUIPanel(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	m_nWidgetsHeight = 0;
	EnableScroll(true);
	ShowHorizontalBar(false);
}

XUIPanel::XUIPanel(const char* pName, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName, nLeft, nTop, nWidth, nHeight)
{
	m_nWidgetsHeight = 0;
	EnableScroll(true);
	ShowHorizontalBar(false);
}

bool XUIPanel::AddWidget(XUIWidget* pWidget)
{
	if(m_nWidgetsHeight) m_nWidgetsHeight += INTEND_SIZE;
	pWidget->SetWidgetRect(0, m_nWidgetsHeight, GetClientWidth(), pWidget->GetWidgetHeight());
	AddChild(pWidget);
	m_nWidgetsHeight += pWidget->GetWidgetHeight();
	SetScrollSize(GetClientWidth(), m_nWidgetsHeight);
	return true;
}

void XUIPanel::ClearWidgets()
{
	XUIWidget* pWidget = GetFirstChild();
	while(pWidget) {
		pWidget->Delete();
		pWidget = pWidget->GetNext();
	}
	m_nWidgetsHeight = 0;
}

void XUIPanel::OnRender(XUIDevice* pDevice)
{
	pDevice->AddRect(0, 0, GetWidgetWidth(), GetWidgetHeight(), 6, XUI_RGBA(0,0,0,192));
	XUIWidget::OnRender(pDevice);
}

XUISlider::XUISlider(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	SetRange(0.0f, 1.0f, 0.01f);
	SetValue(m_fMin);
	m_bIn = false;
	m_nCaptureX = -1;
}

XUISlider::XUISlider(const char* pName, const char* pTitle, int nLeft, int nTop, int nWidth, int nHeight, float vmin, float vmax, float vinc) : XUIWidget(pName, nLeft, nTop, nWidth, nHeight)
{
	m_Title = pTitle;
	SetRange(vmin, vmax, vinc);
	SetValue(m_fMin);
	m_bIn = false;
	m_nCaptureX = -1;
}

void XUISlider::SetRange(float vmin, float vmax, float vinc)
{
	if(vmin<vmax) {
		m_fMin = vmin;
		m_fMax = vmax;
	} else {
		m_fMin = vmax;
		m_fMax = vmin;
	}
	m_fInc = m_fMax - m_fMin;
	if(vinc<0.01) vinc = 0.01f;
	if(m_fInc>vinc) m_fInc = vinc;
}

void XUISlider::SetValue(float fValue)
{
	if(fValue<=m_fMin) { m_fValue = m_fMin; return; }
	if(fValue>=m_fMax) { m_fValue = m_fMax; return; }
	float nStep = floor((fValue - m_fMin + m_fInc/2) / m_fInc);
	m_fValue = m_fMin + nStep * m_fInc;
}

void XUISlider::OnRender(XUIDevice* pDevice)
{
	pDevice->AddRect(0, 0, GetWidgetWidth(), GetWidgetHeight(), 2, XUI_RGBA(0,0,0,128));

	int nPosX = (int)((GetWidgetWidth()-SLIDER_WIDTH) * (m_fValue-m_fMin) / (m_fMax-m_fMin));

	pDevice->AddRect(nPosX, 0, SLIDER_WIDTH, GetWidgetHeight(), 2, XUI_RGBA(255,255,255,64));
//	pDevice->AddRect(nPosX, 0, SLIDER_WIDTH, GetWidgetHeight(), 2, m_bIn||m_nCaptureX>=0?XUI_RGBA(255,255,255,255):XUI_RGBA(255,255,255,64));

	int digits = (int)(ceilf(log10f(m_fInc)));
	char fmt[16];
	sprintf(fmt, "%%.%df", digits >= 0 ? 0 : -digits);
	char msg[128];
	sprintf(msg, fmt, m_fValue);

	XUIColor color = m_bIn||m_nCaptureX>=0?XUI_RGBA(255,196,0,255):XUI_RGBA(255,255,255,200);
	pDevice->AddText(0, (GetWidgetHeight()-TEXT_HEIGHT)/2, XUIALIGN_RIGHT, color, m_Title.c_str());
	pDevice->AddText(GetWidgetWidth(), (GetWidgetHeight()-TEXT_HEIGHT)/2, XUIALIGN_LEFT, color, msg);
}

void XUISlider::OnMouseMove(const XUIPoint& Point)
{
	int nPosX = (int)((GetWidgetWidth()-SLIDER_WIDTH) * (m_fValue-m_fMin) / (m_fMax-m_fMin));

	if(m_nCaptureX>=0) {
		float fPixelValue = (m_fMax-m_fMin) / (GetWidgetWidth()-SLIDER_WIDTH);
		SetValue(m_fCaptureValue+(Point.x-m_nCaptureX)*fPixelValue);
	} else {
		if(Point.x>=nPosX && Point.x<nPosX+SLIDER_WIDTH && Point.y>=0 && Point.y<GetWidgetHeight()) {
			m_bIn = true;
		} else {
			m_bIn = false;
		}
	}
}

bool XUISlider::OnMouseWheel(const XUIPoint& Point, int _rel)
{
	return XUIWidget::OnMouseWheel(Point, _rel);
}

void XUISlider::OnMouseLeave()
{
	m_bIn = false;
}

void XUISlider::OnMouseButtonPressed(const XUIPoint& Point, unsigned short nId)
{
	if(nId==XUI_INPUT::MOUSE_LBUTTON) {
		int nPosX = (int)((GetWidgetWidth()-SLIDER_WIDTH) * (m_fValue-m_fMin) / (m_fMax-m_fMin));
		if(Point.x>=nPosX && Point.x<nPosX+SLIDER_WIDTH && Point.y>=0 && Point.y<GetWidgetHeight()) {
			m_fCaptureValue = GetValue();
			m_nCaptureX = Point.x;
			GetXUI()->SetCapture(this, true);
		}
	}

	XUIWidget::OnMouseButtonPressed(Point, nId);
}

void XUISlider::OnMouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
	if(nId==XUI_INPUT::MOUSE_LBUTTON) {
		m_nCaptureX = -1;
		GetXUI()->SetCapture(this, false);
	}

	XUIWidget::OnMouseButtonReleased(Point, nId);
}

XUIDialog::XUIDialog(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	m_bInMove = false;
	m_bBarLight = false;
	SetClientArea(SCROLL_AREA_PADDING, SCROLL_AREA_PADDING, SCROLL_AREA_PADDING, SCROLL_AREA_PADDING);
}

XUIDialog::XUIDialog(const char* pName, const char* pTitle, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName, nLeft, nTop, nWidth, nHeight)
{
	m_bInMove = false;
	m_bBarLight = false;
	m_Title = pTitle;
	SetClientArea(SCROLL_AREA_PADDING, AREA_HEADER, SCROLL_AREA_PADDING, SCROLL_AREA_PADDING);
}

XUIDialog::~XUIDialog()
{
}

void XUIDialog::OnRender(XUIDevice* pDevice)
{
	pDevice->AddRect(0, 0, GetWidgetWidth(), GetWidgetHeight(), 6, XUI_RGBA(0,0,0,192));

	pDevice->AddRect(SCROLL_AREA_PADDING, SCROLL_AREA_PADDING,
		GetWidgetWidth()-SCROLL_AREA_PADDING*2, AREA_HEADER-SCROLL_AREA_PADDING*2,
		2, m_bBarLight?XUI_RGBA(255, 150, 0, 192):XUI_RGBA(198, 112, 0, 192));
	pDevice->AddText(AREA_HEADER/2, AREA_HEADER/2-TEXT_HEIGHT/2-1, XUIALIGN_RIGHT,
		m_bBarLight?XUI_RGBA(255,255,255,255):XUI_RGBA(255,255,255,128), m_Title.c_str());

	XUIWidget::OnRender(pDevice);
}

void XUIDialog::OnMouseMove(const XUIPoint& Point)
{
	if(m_bInMove) {
		XUIPoint InMovePoint(Point.x, Point.y);
		WidgetToScreen(InMovePoint, InMovePoint);
		SetWidgetPosition(GetWidgetLeft()+InMovePoint.x-m_InMovePoint.x, GetWidgetTop()+InMovePoint.y-m_InMovePoint.y);
		m_InMovePoint.x = InMovePoint.x;
		m_InMovePoint.y = InMovePoint.y;
	}

	if(		Point.x>=SCROLL_AREA_PADDING && Point.y>=SCROLL_AREA_PADDING
		&&	Point.x<GetWidgetWidth()-SCROLL_AREA_PADDING
		&&	Point.y<AREA_HEADER-SCROLL_AREA_PADDING) {
		m_bBarLight = true;
	} else {
		m_bBarLight = false;
	}
}

void XUIDialog::OnMouseLeave()
{
	m_bBarLight = false;
}

void XUIDialog::OnMouseButtonPressed(const XUIPoint& Point, unsigned short nId)
{
	if(nId==XUI_INPUT::MOUSE_LBUTTON) {
		if(		Point.x>=SCROLL_AREA_PADDING
			&&	Point.y>=SCROLL_AREA_PADDING
			&&	Point.x<GetWidgetWidth()-SCROLL_AREA_PADDING
			&&	Point.y<AREA_HEADER-SCROLL_AREA_PADDING) {

			m_bInMove = true;
			m_InMovePoint.x = Point.x;
			m_InMovePoint.y = Point.y;
			WidgetToScreen(m_InMovePoint, m_InMovePoint);
			GetXUI()->SetCapture(this, true);
		}
	}
}

void XUIDialog::OnMouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
	if(nId==XUI_INPUT::MOUSE_LBUTTON) {
		m_bInMove = false;
		GetXUI()->SetCapture(this, true);
	}
}

XUIListItem::XUIListItem(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	m_pView = NULL;
	m_pUserData = NULL;
	m_bSelected = false;
}

XUIListItem::XUIListItem(const char* pName, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName, nLeft, nTop, nWidth, nHeight)
{
	m_pView = NULL;
	m_pUserData = NULL;
	m_bSelected = false;
}

void XUIListItem::SetSelect(bool bSelected)
{
	m_pView->SetSelectItem(this, bSelected);
}

void XUIListItem::OnRender(XUIDevice* pDevice)
{
	if(m_bSelected) {
		pDevice->AddRect(0, 0, GetWidgetWidth(), GetWidgetHeight(), 6, XUI_RGBA(255,0,0,192));
	}

	pDevice->AddText(GetWidgetWidth()/2, GetWidgetHeight()/2-TEXT_HEIGHT/2, XUIALIGN_CENTER, XUI_RGBA(250, 250, 250, 255), m_sText.c_str());

	XUIWidget::OnRender(pDevice);
}

void XUIListItem::OnMouseButtonClick(const XUIPoint& Point, unsigned short nId)
{
	if(m_pView->GetMultiSelect() && !m_bSelected) {
		m_pView->SetSelectItem(this, false);
	} else {
		m_pView->SetSelectItem(this, true);
	}

	XUIWidget::OnMouseButtonClick(Point, nId);
}

void XUIListItem::OnSizeChange(int nWidth, int nHeight)
{
	if(m_pView) m_pView->AdjustItems();

	XUIWidget::OnSizeChange(nWidth, nHeight);
}

XUIListView::XUIListView(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	m_bMultiSelect = false;
	EnableScroll(true);
	ShowVerticalBar(true);
	ShowHorizontalBar(false);
	SetScrollSize(GetClientWidth(), 0);
}

XUIListView::XUIListView(const char* pName, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName, nLeft, nTop, nWidth, nHeight)
{
	m_bMultiSelect = false;
	EnableScroll(true);
	ShowVerticalBar(true);
	ShowHorizontalBar(false);
	SetScrollSize(GetClientWidth(), 0);
}

void XUIListView::AddItem(XUIListItem* pItem)
{
	assert(pItem->m_pView==NULL);
	pItem->SetWidgetSize(GetClientWidth(), pItem->GetWidgetHeight());
	pItem->SetWidgetPosition(0, GetScrollHeight());
	pItem->m_pView = this;
	m_Items.push_back(pItem);
	AddChild(pItem);
	SetScrollSize(GetClientWidth(), GetScrollHeight()+pItem->GetWidgetHeight());
}

XUIListItem* XUIListView::AddString(const char* pName, const char* pText)
{
	XUIListItem* pItem = new XUIListItem(pName, 0, 0, 0, 20);
	pItem->SetText(pText);
	AddItem(pItem);
	return pItem;
}

bool XUIListView::RemoveItem(XUIListItem* pItem, bool bSilence)
{
	pItem->m_pView = NULL;
	m_Items.remove(pItem);
	pItem->Delete();
	if(!bSilence) AdjustItems();
	return true;
}

void XUIListView::RemoveAllItem()
{
	std::list<XUIListItem*>::iterator i;
	while(!m_Items.empty()) {
		RemoveItem(m_Items.front(), true);
	}
	AdjustItems();
}

int XUIListView::GetItemCount()
{
	return m_Items.size();
}

XUIListItem* XUIListView::GetItem(int nIndex)
{
	std::list<XUIListItem*>::iterator i;
	int ii = 0;
	for(i=m_Items.begin(); i!=m_Items.end(); i++) {
		if(ii==nIndex) return (*i);
		ii++;
	}
	return NULL;
}

XUIListItem* XUIListView::GetSelectItem()
{
	std::list<XUIListItem*>::iterator i;
	for(i=m_Items.begin(); i!=m_Items.end(); i++) {
		if((*i)->m_bSelected) return (*i);
	}
	return NULL;
}

void XUIListView::SetMultiSelect(bool bMultiSelect)
{
	m_bMultiSelect = bMultiSelect;

	if(!bMultiSelect) {
		XUIListItem* pSelected = GetSelectItem();
		if(pSelected) {
			SetSelectItem(pSelected, true);
		}
	}
}

void XUIListView::OnRender(XUIDevice* pDevice)
{
	XUIWidget::OnRender(pDevice);
}

void XUIListView::SetSelectItem(XUIListItem* pItem, bool bSelected)
{
	if(m_bMultiSelect) {
		pItem->m_bSelected = bSelected;
	} else {
		if(bSelected) {
			std::list<XUIListItem*>::iterator i;
			for(i=m_Items.begin(); i!=m_Items.end(); i++) {
				(*i)->m_bSelected = ((*i)==pItem);
			}
		}
	}
}

void XUIListView::AdjustItems()
{
	int nHeight = 0;
	std::list<XUIListItem*>::iterator i;
	for(i=m_Items.begin(); i!=m_Items.end(); i++) {
		if(GetClientWidth()!=(*i)->GetWidgetWidth()) {
			(*i)->SetWidgetSize(GetClientWidth(), (*i)->GetWidgetHeight());
		}
		(*i)->SetWidgetPosition(0, nHeight);
		nHeight += (*i)->GetWidgetHeight();
	}
	SetScrollSize(GetClientWidth(), nHeight);
}























XUIEditLine::XUIEditLine(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
}

XUIEditLine::XUIEditLine(const char* pName, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName, nLeft, nTop, nWidth, nHeight)
{
}

void XUIEditLine::OnRender(XUIDevice* pDevice)
{
	pDevice->AddText(0, 0, 0, XUI_RGB(255, 255, 255), m_sText.c_str());
}

void XUIEditLine::OnKeyChar(unsigned short nKey, unsigned int Char)
{
}
