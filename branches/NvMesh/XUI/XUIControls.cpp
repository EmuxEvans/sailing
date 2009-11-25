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
	if(bManualFree) ManualFree();
	m_bOver = false;
}

XUIButton::XUIButton(const char* pName, const char* pText, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName)
{
	m_bOver = false;
	SetText(pText);
	SetWidgetRect(nLeft, nTop, nWidth, nHeight);
}

XUIButton::~XUIButton()
{
}

void XUIButton::onRender(XUIDevice* pDevice)
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

void XUICheckBox::onRender(XUIDevice* pDevice)
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

void XUICheckBox::onMouseButtonClick(const XUIPoint& Point, unsigned short nId)
{
	m_bCheck = !m_bCheck;
	XUIButton::onMouseButtonClick(Point, nId);
}

XUILabel::XUILabel(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	if(bManualFree) ManualFree();
	m_nAlign = XUIALIGN_CENTER;
}

XUILabel::XUILabel(const char* pName, const char* pText, int nAlign, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName)
{
	SetText(pText);
	m_nAlign = nAlign;
	SetWidgetRect(nLeft, nTop, nWidth, nHeight);
}

XUILabel::~XUILabel()
{
}

void XUILabel::onRender(XUIDevice* pDevice)
{
	switch(m_nAlign) {
	case XUIALIGN_LEFT:
		pDevice->AddText(0, GetWidgetHeight()/2-TEXT_HEIGHT/2, XUIALIGN_RIGHT, XUI_RGBA(250, 250, 250, 255), m_Text.c_str());
		break;
	case XUIALIGN_RIGHT:
		pDevice->AddText(GetWidgetWidth(), GetWidgetHeight()/2-TEXT_HEIGHT/2, XUIALIGN_LEFT, XUI_RGBA(250, 250, 250, 255), m_Text.c_str());
		break;
	case XUIALIGN_CENTER:
	default:
		pDevice->AddText(GetWidgetWidth()/2, GetWidgetHeight()/2-TEXT_HEIGHT/2, m_nAlign, XUI_RGBA(250, 250, 250, 255), m_Text.c_str());
	}
}

XUIPanel::XUIPanel(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	m_nWidgetsHeight = 0;
	SetClientArea(0, 0, SCROLL_AREA_PADDING*2, 0);
}

XUIPanel::XUIPanel(const char* pName, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName)
{
	m_nWidgetsHeight = 0;
	SetClientArea(0, 0, SCROLL_AREA_PADDING*2, 0);
	SetWidgetRect(nLeft, nTop, nWidth, nHeight);
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

void XUIPanel::onRender(XUIDevice* pDevice)
{
	pDevice->AddRect(0, 0, GetWidgetWidth(), GetWidgetHeight(), 6, XUI_RGBA(0,0,0,192));
	XUIWidget::onRender(pDevice);
}

XUISlider::XUISlider(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	SetRange(0.0f, 1.0f, 0.01f);
	SetValue(m_fMin);
	m_bIn = false;
	m_nCaptureX = -1;
}

XUISlider::XUISlider(const char* pName, const char* pTitle, int nLeft, int nTop, int nWidth, int nHeight, float vmin, float vmax, float vinc) : XUIWidget(pName)
{
	m_Title = pTitle;
	SetWidgetRect(nLeft, nTop, nWidth, nHeight);
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

void XUISlider::onRender(XUIDevice* pDevice)
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

void XUISlider::onMouseMove(const XUIPoint& Point)
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

bool XUISlider::onMouseWheel(const XUIPoint& Point, int _rel)
{
	return XUIWidget::onMouseWheel(Point, _rel);
//	return true;
}

void XUISlider::onMouseLeave()
{
	m_bIn = false;
}

void XUISlider::onMouseButtonPressed(const XUIPoint& Point, unsigned short nId)
{
	if(nId==XUI_INPUT::MOUSE_LBUTTON) {
		int nPosX = (int)((GetWidgetWidth()-SLIDER_WIDTH) * (m_fValue-m_fMin) / (m_fMax-m_fMin));
		if(Point.x>=nPosX && Point.x<nPosX+SLIDER_WIDTH && Point.y>=0 && Point.y<GetWidgetHeight()) {
			m_fCaptureValue = GetValue();
			m_nCaptureX = Point.x;
			GetXUI()->SetCapture(this, true);
		}
	}

	XUIWidget::onMouseButtonPressed(Point, nId);
}

void XUISlider::onMouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
	if(nId==XUI_INPUT::MOUSE_LBUTTON) {
		m_nCaptureX = -1;
	}

	XUIWidget::onMouseButtonReleased(Point, nId);
}

XUIScrollPanel::XUIScrollPanel(const char* pName, bool bManualFree) : XUIPanel(pName, bManualFree)
{
	if(bManualFree) ManualFree();
	EnableScroll(true);
	m_bShowBoard = false;
}

XUIScrollPanel::XUIScrollPanel(const char* pName, int nLeft, int nTop, int nWidth, int nHeight) : XUIPanel(pName)
{
	SetWidgetRect(nLeft, nTop, nWidth, nHeight);
	EnableScroll(true);
	m_bShowBoard = false;
}

XUIScrollPanel::~XUIScrollPanel()
{
}

void XUIScrollPanel::onRender(XUIDevice* pDevice)
{
	if(m_bShowBoard) {
		XUIPanel::onRender(pDevice);
	} else {
		XUIWidget::onRender(pDevice);
	}

	if(m_nWidgetsHeight>GetClientHeight()) {
		int nBarStart, nBarHeight;
		nBarStart = (int)((float)GetScrollPositionY()/(float)m_nWidgetsHeight * GetClientHeight());
		nBarHeight = (int)((float)(GetScrollPositionY()+GetClientHeight())/(float)m_nWidgetsHeight * GetClientHeight()) - nBarStart;
		if(nBarStart+nBarHeight>GetClientHeight()) nBarHeight = GetClientHeight() - nBarStart;
		int nBarWidth;
		nBarWidth = SCROLL_AREA_PADDING*2 - 2;
		pDevice->AddRect(GetWidgetWidth()-nBarWidth, GetClientTop(),           nBarWidth, GetClientHeight(), nBarWidth/2-1, XUI_RGBA(0,0,0,255));
		pDevice->AddRect(GetWidgetWidth()-nBarWidth, GetClientTop()+nBarStart, nBarWidth, nBarHeight, nBarWidth/2-1, XUI_RGBA(255,255,255,200));
	}
}

void XUIScrollPanel::onMouseMove(const XUIPoint& Point)
{
	if(m_nCaptureScroll>=0) {
		int nScroll = m_nCaptureScroll + (int)(((float)Point.y-m_nCaptureY)/GetClientHeight()*m_nWidgetsHeight);
		SetScrollPosition(XUIPoint(0, nScroll));
	}

	XUIWidget::onMouseMove(Point);
}

bool XUIScrollPanel::onMouseWheel(const XUIPoint& Point, int _rel)
{
	int nScroll = GetScrollPositionY() - _rel*20;
	SetScrollPosition(XUIPoint(0, nScroll));
	XUIWidget::onMouseWheel(Point, _rel);
	return true;
}

void XUIScrollPanel::onMouseButtonPressed(const XUIPoint& Point, unsigned short nId)
{
	if(nId==XUI_INPUT::MOUSE_LBUTTON) {
		if(GetClientHeight()<m_nWidgetsHeight) {
			int nBarWidth;
			nBarWidth = SCROLL_AREA_PADDING*2 - 2;
			if(Point.x>=GetWidgetWidth()-nBarWidth && Point.x<GetWidgetWidth()) {
				if(Point.y>=GetClientTop() && Point.y<GetClientTop()+GetClientHeight()) {
					int nBarTop, nBarHeight;
					nBarTop = (int)(GetClientHeight()*GetScrollPositionY()/(float)m_nWidgetsHeight);
					nBarHeight = (int)(GetClientHeight()*GetClientHeight()/(float)m_nWidgetsHeight);

					if(Point.y>GetClientTop()+nBarTop && Point.y<GetClientTop()+nBarTop+nBarHeight) {
						m_nCaptureScroll = GetScrollPositionY();
						m_nCaptureY = Point.y;
					} else {
						m_nCaptureScroll = (int)((Point.y-GetClientTop()-nBarHeight/2)/(float)GetClientHeight()*m_nWidgetsHeight);
						if(m_nCaptureScroll<0) m_nCaptureScroll = 0;
						if(m_nCaptureScroll>=m_nWidgetsHeight-GetClientHeight()) m_nCaptureScroll = m_nWidgetsHeight-GetClientHeight();
						m_nCaptureY = Point.y;
						SetScrollPosition(XUIPoint(0, m_nCaptureScroll));
					}

					GetXUI()->SetCapture(this, true);
				}
			}
		}
	}

	XUIWidget::onMouseButtonPressed(Point, nId);
}

void XUIScrollPanel::onMouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
	if(nId==XUI_INPUT::MOUSE_LBUTTON) {
		m_nCaptureScroll = -1;
		GetXUI()->SetCapture(this, false);
	}

	XUIWidget::onMouseButtonReleased(Point, nId);
}

XUIDialog::XUIDialog(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	m_bInMove = false;
	m_bBarLight = false;
	SetClientArea(SCROLL_AREA_PADDING, SCROLL_AREA_PADDING, SCROLL_AREA_PADDING, SCROLL_AREA_PADDING);
}

XUIDialog::XUIDialog(const char* pName, const char* pTitle, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName, false)
{
	m_bInMove = false;
	m_bBarLight = false;
	m_Title = pTitle;
	SetWidgetRect(nLeft, nTop, nWidth, nHeight);
	SetClientArea(SCROLL_AREA_PADDING, AREA_HEADER, SCROLL_AREA_PADDING, SCROLL_AREA_PADDING);
}

XUIDialog::~XUIDialog()
{
}

void XUIDialog::ActiveWidget(XUIWidget* pWidget)
{
	BringToTop();
	XUIWidget::ActiveWidget(pWidget);
}

void XUIDialog::onRender(XUIDevice* pDevice)
{
	pDevice->AddRect(0, 0, GetWidgetWidth(), GetWidgetHeight(), 6, XUI_RGBA(0,0,0,192));

	pDevice->AddRect(SCROLL_AREA_PADDING, SCROLL_AREA_PADDING,
		GetWidgetWidth()-SCROLL_AREA_PADDING*2, AREA_HEADER-SCROLL_AREA_PADDING*2,
		2, m_bBarLight?XUI_RGBA(255, 150, 0, 192):XUI_RGBA(198, 112, 0, 192));
	pDevice->AddText(AREA_HEADER/2, AREA_HEADER/2-TEXT_HEIGHT/2-1, XUIALIGN_RIGHT,
		m_bBarLight?XUI_RGBA(255,255,255,255):XUI_RGBA(255,255,255,128), m_Title.c_str());

	XUIWidget::onRender(pDevice);
}

void XUIDialog::onMouseMove(const XUIPoint& Point)
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

void XUIDialog::onMouseLeave()
{
	m_bBarLight = false;
}

void XUIDialog::onMouseButtonPressed(const XUIPoint& Point, unsigned short nId)
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

void XUIDialog::onMouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
	if(nId==XUI_INPUT::MOUSE_LBUTTON) {
		m_bInMove = false;
	}
}
