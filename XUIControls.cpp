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
static const int INTEND_SIZE = 16;
static const int AREA_HEADER = 28;

XUIButton::XUIButton(bool bManualFree)
{
	if(bManualFree) ManualFree();
	m_bOver = false;
	m_pText = "";
}

XUIButton::XUIButton(const char* pText, int nLeft, int nTop, int nWidth, int nHeight)
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

XUILabel::XUILabel(bool bManualFree)
{
	if(bManualFree) ManualFree();
	m_pText = NULL;
}

XUILabel::XUILabel(const char* pText, int nLeft, int nTop, int nWidth, int nHeight)
{
	SetText(pText);
	SetWidgetRect(nLeft, nTop, nWidth, nHeight);
}

XUILabel::~XUILabel()
{
}

void XUILabel::onRender(XUIDevice* pDevice)
{
	pDevice->AddRect(0, 0, GetWidgetWidth(), GetWidgetHeight(), 0, XUI_RGBA(128,128,128, 196));
	pDevice->AddText(GetWidgetWidth()/2, GetWidgetHeight()/2-TEXT_HEIGHT/2, 1, XUI_RGBA(250, 250, 250, 255), m_pText);
}

XUIScrollPanel::XUIScrollPanel(bool bManualFree) : m_ClientArea(true)
{
	if(bManualFree) ManualFree();
	m_pText = NULL;
	m_nWidgetsHeight = 0;

	m_ClientArea.EnableScroll(true);
	AddChild(&m_ClientArea);
}

XUIScrollPanel::XUIScrollPanel(const char* pText, int nLeft, int nTop, int nWidth, int nHeight) : m_ClientArea(true)
{
	SetText(pText);
	m_nWidgetsHeight = 0;
	SetWidgetRect(nLeft, nTop, nWidth, nHeight);
	m_ClientArea.EnableScroll(true);
	AddChild(&m_ClientArea);
}

XUIScrollPanel::~XUIScrollPanel()
{
}

bool XUIScrollPanel::AddWidget(XUIWidget* pWidget)
{
	if(m_nWidgetsHeight) m_nWidgetsHeight += INTEND_SIZE;
	pWidget->SetWidgetRect(0, m_nWidgetsHeight, m_ClientArea.GetWidgetWidth(), pWidget->GetWidgetHeight());
	m_ClientArea.AddChild(pWidget);
	m_nWidgetsHeight += pWidget->GetWidgetHeight();
	m_ClientArea.SetScrollSize(m_ClientArea.GetWidgetWidth(), m_nWidgetsHeight);
	return true;
}

void XUIScrollPanel::onRender(XUIDevice* pDevice)
{
	pDevice->AddRect(0, 0, GetWidgetWidth(), GetWidgetHeight(), 6, XUI_RGBA(0,0,0,192));
	pDevice->AddText(AREA_HEADER/2, AREA_HEADER/2-TEXT_HEIGHT/2, 0, XUI_RGBA(255,255,255,128), m_pText);

	int nBarStart, nBarHeight;
	if(m_ClientArea.GetScrollHeight()>=m_ClientArea.GetWidgetHeight()) {
		nBarStart = 0;
		nBarHeight = m_ClientArea.GetWidgetHeight();
	} else {
		nBarStart = (int)((float)m_ClientArea.GetScrollPosition().y/(float)m_ClientArea.GetScrollHeight() * m_ClientArea.GetWidgetHeight());
		nBarHeight = (int)((float)(m_ClientArea.GetScrollPosition().y+m_ClientArea.GetWidgetHeight())/(float)m_ClientArea.GetScrollHeight() * m_ClientArea.GetWidgetHeight()) - nBarStart;
		if(nBarHeight>m_ClientArea.GetWidgetHeight()) nBarHeight = m_ClientArea.GetWidgetHeight();
	}
	nBarStart = (int)((float)m_ClientArea.GetScrollPosition().y/(float)m_ClientArea.GetScrollHeight() * m_ClientArea.GetWidgetHeight());
	nBarHeight = (int)((float)(m_ClientArea.GetScrollPosition().y+m_ClientArea.GetWidgetHeight())/(float)m_ClientArea.GetScrollHeight() * m_ClientArea.GetWidgetHeight()) - nBarStart;
	if(nBarStart+nBarHeight>m_ClientArea.GetWidgetHeight()) nBarHeight = m_ClientArea.GetWidgetHeight() - nBarStart;

	int nBarLeft, nBarWidth;
	nBarLeft = GetWidgetWidth()-SCROLL_AREA_PADDING*2-SCROLL_AREA_PADDING/2 + 1;
	nBarWidth = SCROLL_AREA_PADDING*2 - 2;

	pDevice->AddRect(nBarLeft, m_ClientArea.GetWidgetTop(), nBarWidth, m_ClientArea.GetWidgetHeight(), nBarWidth/2-1, XUI_RGBA(0,0,0,255));
	pDevice->AddRect(nBarLeft, m_ClientArea.GetWidgetTop()+nBarStart, nBarWidth, nBarHeight, nBarWidth/2-1, XUI_RGBA(255,255,255,200));

	XUIWidget::onRender(pDevice);
}

void XUIScrollPanel::onMouseMove(const XUIPoint& Point)
{
	if(m_nCaptureScroll<0) return;

	int nScroll = m_nCaptureScroll + (int)(((float)Point.y-m_nCaptureY)/m_ClientArea.GetWidgetHeight()*m_ClientArea.GetScrollHeight());
	m_ClientArea.SetScrollPosition(XUIPoint(0, nScroll));
}

bool XUIScrollPanel::onMouseWheel(const XUIPoint& Point, int _rel)
{
	int nScroll = m_ClientArea.GetScrollPosition().y - _rel*20;
	m_ClientArea.SetScrollPosition(XUIPoint(0, nScroll));
	return true;
}

void XUIScrollPanel::onMouseButtonPressed(const XUIPoint& Point, unsigned short nId)
{
	if(m_ClientArea.GetWidgetHeight()<m_nWidgetsHeight) {
		int nBarLeft, nBarWidth;
		nBarLeft = GetWidgetWidth()-SCROLL_AREA_PADDING*2-SCROLL_AREA_PADDING/2 + 1;
		nBarWidth = SCROLL_AREA_PADDING*2 - 2;

		if(Point.x<nBarLeft || Point.x>=nBarLeft+nBarWidth) return;
		if(Point.y<m_ClientArea.GetWidgetTop() || Point.y>=m_ClientArea.GetWidgetHeight()) return;

		int nBarTop, nBarHeight;
		nBarTop = (int)(m_ClientArea.GetWidgetHeight()*m_ClientArea.GetScrollPosition().y/(float)m_nWidgetsHeight);
		nBarHeight = (int)(m_ClientArea.GetWidgetHeight()*m_ClientArea.GetWidgetHeight()/(float)m_nWidgetsHeight);

		if(Point.y>m_ClientArea.GetWidgetTop()+nBarTop && Point.y<m_ClientArea.GetWidgetTop()+nBarTop+nBarHeight) {
			m_nCaptureScroll = m_ClientArea.GetScrollPosition().y;
			m_nCaptureY = Point.y;
		} else {
			m_nCaptureScroll = (int)((Point.y-m_ClientArea.GetWidgetTop()-nBarHeight/2)/(float)m_ClientArea.GetWidgetHeight()*m_nWidgetsHeight);
			if(m_nCaptureScroll<0) m_nCaptureScroll = 0;
			if(m_nCaptureScroll>=m_nWidgetsHeight-m_ClientArea.GetWidgetHeight()) m_nCaptureScroll = m_nWidgetsHeight-m_ClientArea.GetWidgetHeight();
			m_nCaptureY = Point.y;
			m_ClientArea.SetScrollPosition(XUIPoint(0, m_nCaptureScroll));
		}

		GetXUI()->SetCapture(this, true);
	}
}

void XUIScrollPanel::onMouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
	m_nCaptureScroll = -1;
	GetXUI()->SetCapture(this, false);
}

void XUIScrollPanel::OnSizeChange(int nWidth, int nHeight)
{
	m_ClientArea.SetWidgetRect(SCROLL_AREA_PADDING, AREA_HEADER,
		nWidth  - SCROLL_AREA_PADDING - SCROLL_AREA_PADDING*4,
		nHeight - AREA_HEADER - AREA_HEADER-SCROLL_AREA_PADDING);
}
