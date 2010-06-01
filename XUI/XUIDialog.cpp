#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#include "XUIMisc.h"
#include "XUIDelegate.h"
#include "XUIWidget.h"
#include "XUIDialog.h"
#include "XUIDevice.h"
/*
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
*/

static const int DAILOG_EDGE_WIDTH = 6;
static const int DAILOG_HEADER_HEIGHT = 18;
static const int TEXT_HEIGHT = 8;

XUIDialog::XUIDialog(const char* pName, bool bManualFree) : XUIWidget(pName, bManualFree)
{
	m_bInMove = false;
	m_bBarLight = false;
	m_bTitleBar = false;
	m_nEdgeWidth = DAILOG_EDGE_WIDTH;
	SetTitleBar(true);
}

XUIDialog::XUIDialog(const char* pName, const char* pTitle, int nLeft, int nTop, int nWidth, int nHeight) : XUIWidget(pName, nLeft, nTop, nWidth, nHeight)
{
	m_bInMove = false;
	m_bBarLight = false;
	m_Title = pTitle;
	m_bTitleBar = false;
	m_nEdgeWidth = DAILOG_EDGE_WIDTH;
	SetTitleBar(true);
}

XUIDialog::~XUIDialog()
{
}

void XUIDialog::SetTitleBar(bool bShow)
{
	if(m_bTitleBar!=bShow) {
		m_bTitleBar = bShow;
		SetClientArea(m_nEdgeWidth, m_nEdgeWidth+(m_bTitleBar?DAILOG_HEADER_HEIGHT+m_nEdgeWidth:0), m_nEdgeWidth, m_nEdgeWidth);
	}
}

void XUIDialog::SetEdgeWidth(int nEdgeWidth)
{
	if(m_nEdgeWidth!=nEdgeWidth && m_nEdgeWidth>0) {
		SetClientArea(m_nEdgeWidth, m_nEdgeWidth+(m_bTitleBar?DAILOG_HEADER_HEIGHT:0), m_nEdgeWidth, m_nEdgeWidth);
		m_nEdgeWidth = nEdgeWidth;
	}
}

void XUIDialog::OnRender(XUIDevice* pDevice)
{
	pDevice->AddRect(0, 0, GetWidgetWidth(), GetWidgetHeight(), 6, XUI_RGBA(0,0,0,192));

	pDevice->AddRect(m_nEdgeWidth, m_nEdgeWidth,
		GetWidgetWidth()-m_nEdgeWidth*2, DAILOG_HEADER_HEIGHT,
		2, HasFocus()?XUI_RGBA(198, 112, 0, 255):XUI_RGBA(198, 112, 0, 128));
	pDevice->AddText(m_nEdgeWidth+DAILOG_HEADER_HEIGHT/2, m_nEdgeWidth+DAILOG_HEADER_HEIGHT/2-TEXT_HEIGHT/2-1, XUIALIGN_RIGHT,
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

	if(		Point.x>=m_nEdgeWidth && Point.y>=m_nEdgeWidth
		&&	Point.x<GetWidgetWidth()-m_nEdgeWidth
		&&	Point.y<m_nEdgeWidth+DAILOG_HEADER_HEIGHT) {
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
		if(		Point.x>=m_nEdgeWidth
			&&	Point.y>=m_nEdgeWidth
			&&	Point.x<GetWidgetWidth()-m_nEdgeWidth
			&&	Point.y<m_nEdgeWidth+DAILOG_HEADER_HEIGHT) {

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
