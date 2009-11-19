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
	pDevice->AddRect(0, 0, m_nWidth, m_nHeight, m_nHeight/2-1, XUI_RGBA(128, 128, 128, IsEnable()&&m_bOver?196:96));

	if(IsEnable())
		pDevice->AddText(m_nWidth/2, m_nHeight/2-TEXT_HEIGHT/2-1, 1, m_bOver?XUI_RGBA(255,196,0,255):XUI_RGBA(255,255,255,200), m_pText);
	else
		pDevice->AddText(m_nWidth/2, m_nHeight/2-TEXT_HEIGHT/2-1, 1, XUI_RGBA(128,128,128,200), m_pText);
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
	pDevice->AddRect(0, 0, m_nWidth, m_nHeight, 0, XUI_RGBA(128,128,128, 196));
	pDevice->AddText(m_nWidth/2, m_nHeight/2-TEXT_HEIGHT/2, 1, XUI_RGBA(250, 250, 250, 255), m_pText);
}

XUIScrollPanel::XUIScrollPanel()
{
	m_pText = NULL;
	m_nScrollCount = 0;
	m_nScroll = 0;
	SetClientArea(SCROLL_AREA_PADDING, AREA_HEADER, SCROLL_AREA_PADDING*4, AREA_HEADER-SCROLL_AREA_PADDING);
}

XUIScrollPanel::XUIScrollPanel(const char* pText, int nLeft, int nTop, int nWidth, int nHeight)
{
	SetText(pText);
	SetWidgetRect(nLeft, nTop, nWidth, nHeight);
	m_nScrollCount = 0;
	m_nScroll = 0;
	SetClientArea(SCROLL_AREA_PADDING, AREA_HEADER, SCROLL_AREA_PADDING*4, AREA_HEADER-SCROLL_AREA_PADDING);
}

XUIScrollPanel::~XUIScrollPanel()
{
}

bool XUIScrollPanel::AddWidget(XUIWidget* pWidget)
{
	if(m_nScrollCount>0) m_nScrollCount += INTEND_SIZE;
	pWidget->SetWidgetRect(0, m_nScrollCount, GetClientWidth(), pWidget->GetWidgetHeight());
	AddChild(pWidget);
	m_nScrollCount += pWidget->GetWidgetHeight();
	return true;
}

void XUIScrollPanel::onRender(XUIDevice* pDevice)
{
	pDevice->AddRect(0, 0, m_nWidth, m_nHeight, 6, XUI_RGBA(0,0,0,192));
	pDevice->AddText(AREA_HEADER/2, AREA_HEADER/2-TEXT_HEIGHT/2, 0, XUI_RGBA(255,255,255,128), m_pText);

	int nBarStart, nBarHeight;
	if(m_nScrollCount>=GetClientHeight()) {
		nBarStart = 0;
		nBarHeight = GetClientHeight();
	} else {
		nBarStart = (int)((float)m_nScroll/(float)m_nScrollCount * GetClientHeight());
		nBarHeight = (int)((float)(m_nScroll+GetClientHeight())/(float)m_nScrollCount * GetClientHeight()) - nBarStart;
		if(nBarHeight>GetClientHeight()) nBarHeight = GetClientHeight();
	}
	nBarStart = (int)((float)m_nScroll/(float)m_nScrollCount * GetClientHeight());
	nBarHeight = (int)((float)(m_nScroll+GetClientHeight())/(float)m_nScrollCount * GetClientHeight()) - nBarStart;
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

void XUIScrollPanel::MouseWheel(const XUIPoint& Point, int _rel)
{
}
