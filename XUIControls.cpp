#include <stddef.h>

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

XUIButton::XUIButton()
{
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

XUILabel::XUILabel()
{
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

XUIScrollPanel::XUIScrollPanel()
{
	m_pText = NULL;
	m_nWidgetsHeight = 0;
	SetClientArea(SCROLL_AREA_PADDING, AREA_HEADER, SCROLL_AREA_PADDING*4, AREA_HEADER-SCROLL_AREA_PADDING);
	EnableScroll(true);
}

XUIScrollPanel::XUIScrollPanel(const char* pText, int nLeft, int nTop, int nWidth, int nHeight)
{
	SetText(pText);
	m_nWidgetsHeight = 0;
	SetWidgetRect(nLeft, nTop, nWidth, nHeight);
	SetClientArea(SCROLL_AREA_PADDING, AREA_HEADER, SCROLL_AREA_PADDING*4, AREA_HEADER-SCROLL_AREA_PADDING);
	EnableScroll(true);
}

XUIScrollPanel::~XUIScrollPanel()
{
}

bool XUIScrollPanel::AddWidget(XUIWidget* pWidget)
{
	if(m_nWidgetsHeight) m_nWidgetsHeight += INTEND_SIZE;
	pWidget->SetWidgetRect(0, m_nWidgetsHeight, GetClientWidth(), pWidget->GetWidgetHeight());
	AddChild(pWidget);
	m_nWidgetsHeight += pWidget->GetWidgetHeight();
	SetScrollSize(GetClientWidth(), m_nWidgetsHeight);
	return true;
}

void XUIScrollPanel::onRender(XUIDevice* pDevice)
{
	pDevice->AddRect(0, 0, GetWidgetWidth(), GetWidgetHeight(), 6, XUI_RGBA(0,0,0,192));
	pDevice->AddText(AREA_HEADER/2, AREA_HEADER/2-TEXT_HEIGHT/2, 0, XUI_RGBA(255,255,255,128), m_pText);

	int nBarStart, nBarHeight;
	if(GetScrollHeight()>=GetClientHeight()) {
		nBarStart = 0;
		nBarHeight = GetClientHeight();
	} else {
		nBarStart = (int)((float)GetScrollPosition().y/(float)GetScrollHeight() * GetClientHeight());
		nBarHeight = (int)((float)(GetScrollPosition().y+GetClientHeight())/(float)GetScrollHeight() * GetClientHeight()) - nBarStart;
		if(nBarHeight>GetClientHeight()) nBarHeight = GetClientHeight();
	}
	nBarStart = (int)((float)GetScrollPosition().y/(float)GetScrollHeight() * GetClientHeight());
	nBarHeight = (int)((float)(GetScrollPosition().y+GetClientHeight())/(float)GetScrollHeight() * GetClientHeight()) - nBarStart;
	if(nBarStart+nBarHeight>GetClientHeight()) nBarHeight = GetClientHeight() - nBarStart;

	int nBarLeft, nBarWidth;
	nBarLeft = GetWidgetWidth()-SCROLL_AREA_PADDING*2-SCROLL_AREA_PADDING/2 + 1;
	nBarWidth = SCROLL_AREA_PADDING*2 - 2;

	pDevice->AddRect(nBarLeft, GetClientTop(), nBarWidth, GetClientHeight(), nBarWidth/2-1, XUI_RGBA(0,0,0,255));
	pDevice->AddRect(nBarLeft, GetClientTop()+nBarStart, nBarWidth, nBarHeight, nBarWidth/2-1, XUI_RGBA(255,255,255,200));

	pDevice->AddBeginScissor(GetClientLeft(), GetClientTop(), GetClientWidth(), GetClientHeight());
	XUIWidget::onRender(pDevice);
	pDevice->AddEndScissor();
}

void XUIScrollPanel::onMouseMove(const XUIPoint& Point)
{
	if(m_nCaptureScroll<0) return;

	int nScroll = m_nCaptureScroll + (int)(((float)Point.y-m_nCaptureY)/GetClientHeight()*GetScrollHeight());
	if(nScroll<0) nScroll = 0;
	if(nScroll+GetClientHeight()>GetScrollHeight()) nScroll = GetScrollHeight() - GetClientHeight();
	SetScrollPosition(XUIPoint(0, nScroll));
}

void XUIScrollPanel::onMouseWheel(const XUIPoint& Point, int _rel)
{
	int nScroll = GetScrollPosition().x + _rel;
	if(nScroll<0) nScroll = 0;
	if(nScroll+GetClientHeight()>GetScrollHeight()) nScroll = GetScrollHeight() - GetClientHeight();
	SetScrollPosition(XUIPoint(0, nScroll));
}

void XUIScrollPanel::onMouseButtonPressed(const XUIPoint& Point, unsigned short nId)
{
	int nBarLeft, nBarWidth;
	nBarLeft = GetWidgetWidth()-SCROLL_AREA_PADDING*2-SCROLL_AREA_PADDING/2 + 1;
	nBarWidth = SCROLL_AREA_PADDING*2 - 2;

	if(Point.x<nBarLeft || Point.x>=nBarLeft+nBarWidth) return;
	if(Point.y<GetClientTop() || Point.y>=GetClientHeight()) return;
	m_nCaptureScroll = GetScrollPosition().y;
	m_nCaptureY = Point.y;

	GetXUI()->SetCapture(this, true);
}

void XUIScrollPanel::onMouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
	m_nCaptureScroll = -1;
	GetXUI()->SetCapture(this, false);
}
