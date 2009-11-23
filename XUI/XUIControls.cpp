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

XUIButton::XUIButton(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	if(bManualFree) ManualFree();
	m_bOver = false;
	m_pText = "";
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
		pDevice->AddText(GetWidgetWidth()/2, GetWidgetHeight()/2-TEXT_HEIGHT/2-1, 1, m_bOver?XUI_RGBA(255,196,0,255):XUI_RGBA(255,255,255,200), m_pText);
	else
		pDevice->AddText(GetWidgetWidth()/2, GetWidgetHeight()/2-TEXT_HEIGHT/2-1, 1, XUI_RGBA(128,128,128,200), m_pText);
}

XUILabel::XUILabel(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	if(bManualFree) ManualFree();
	m_pText = NULL;
}

XUILabel::XUILabel(const char* pName, const char* pText, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName)
{
	SetText(pText);
	SetWidgetRect(nLeft, nTop, nWidth, nHeight);
}

XUILabel::~XUILabel()
{
}

void XUILabel::onRender(XUIDevice* pDevice)
{
	pDevice->AddText(GetWidgetWidth()/2, GetWidgetHeight()/2-TEXT_HEIGHT/2, 1, XUI_RGBA(250, 250, 250, 255), m_pText);
}

XUIPanel::XUIPanel(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	m_nWidgetsHeight = 0;
}

XUIPanel::XUIPanel(const char* pName, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName)
{
	m_nWidgetsHeight = 0;
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

void XUIPanel::onRender(XUIDevice* pDevice)
{
	pDevice->AddRect(0, 0, GetWidgetWidth(), GetWidgetHeight(), 6, XUI_RGBA(0,0,0,192));
}

XUIScrollPanel::XUIScrollPanel(const char* pName, bool bManualFree) : XUIPanel(pName, bManualFree)
{
	if(bManualFree) ManualFree();
	SetClientArea(0, 0, SCROLL_AREA_PADDING*2, 0);
	EnableScroll(true);
	m_bShowBoard = false;
}

XUIScrollPanel::XUIScrollPanel(const char* pName, int nLeft, int nTop, int nWidth, int nHeight) : XUIPanel(pName)
{
	SetWidgetRect(nLeft, nTop, nWidth, nHeight);
	SetClientArea(0, 0, SCROLL_AREA_PADDING*2, 0);
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

	XUIWidget::onMouseButtonPressed(Point, nId);
}

void XUIScrollPanel::onMouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
	m_nCaptureScroll = -1;
	GetXUI()->SetCapture(this, false);

	XUIWidget::onMouseButtonReleased(Point, nId);
}

XUIDialog::XUIDialog(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	m_bInMove = false;
	m_bBarLight = false;
	m_pTitle = "";
	SetClientArea(SCROLL_AREA_PADDING, SCROLL_AREA_PADDING, SCROLL_AREA_PADDING*4, SCROLL_AREA_PADDING);
}

XUIDialog::XUIDialog(const char* pName, const char* pTitle, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName, false)
{
	m_bInMove = false;
	m_bBarLight = false;
	m_pTitle = pTitle;
	SetWidgetRect(nLeft, nTop, nWidth, nHeight);
	SetClientArea(SCROLL_AREA_PADDING, AREA_HEADER, SCROLL_AREA_PADDING, SCROLL_AREA_PADDING);
}

XUIDialog::~XUIDialog()
{
}

void XUIDialog::onRender(XUIDevice* pDevice)
{
	pDevice->AddRect(0, 0, GetWidgetWidth(), GetWidgetHeight(), 6, XUI_RGBA(0,0,0,192));

	pDevice->AddRect(SCROLL_AREA_PADDING, SCROLL_AREA_PADDING,
		GetWidgetWidth()-SCROLL_AREA_PADDING*2, AREA_HEADER-SCROLL_AREA_PADDING*2,
		2, m_bBarLight?XUI_RGBA(255, 150, 0, 192):XUI_RGBA(198, 112, 0, 192));
	pDevice->AddText(AREA_HEADER/2, AREA_HEADER/2-TEXT_HEIGHT/2-1, 0,
		m_bBarLight?XUI_RGBA(255,255,255,255):XUI_RGBA(255,255,255,128), m_pTitle);

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

void XUIDialog::onMouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
	m_bInMove = false;
}
