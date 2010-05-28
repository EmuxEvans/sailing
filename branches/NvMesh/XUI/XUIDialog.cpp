#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#include "XUIMisc.h"
#include "XUIDelegate.h"
#include "XUIWidget.h"
#include "XUIDialog.h"
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
		2, HasFocus()?XUI_RGBA(255, 150, 0, 255):XUI_RGBA(198, 112, 0, 192));
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
