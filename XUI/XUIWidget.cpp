#include <stddef.h>
#include <assert.h>
#include <set>

#include "XUIMisc.h"
#include "XUIDelegate.h"
#include "XUIWidget.h"
#include "XUIDevice.h"

#define DelWidget(_Parent, _Widget)						\
{														\
	if(_Widget->m_pNext) {								\
		_Widget->m_pNext->m_pPrev = _Widget->m_pPrev;	\
	} else {											\
		_Parent->m_pLastChild = _Widget->m_pPrev;		\
	}													\
	if(_Widget->m_pPrev) {								\
		_Widget->m_pPrev->m_pNext = _Widget->m_pNext;	\
	} else {											\
		_Parent->m_pFirstChild = _Widget->m_pNext;		\
	}													\
}

XUIWidget::XUIWidget(const char* pName, bool bManualFree)
{
	m_sName = pName;
	m_bManualFree = bManualFree;

	m_pParent = NULL;
	m_pNext = NULL;
	m_pPrev = NULL;
	m_pFirstChild = NULL;
	m_pLastChild = NULL;

	m_bDelete = false;
	m_bEnable = true;
	m_bVisable = true;
	m_bTransparent = false;
	m_bMouseIn = false;

	m_nLeft = 0;
	m_nTop = 0;
	m_nWidth = 0;
	m_nHeight = 0;

	m_nClientLeft = 0;
	m_nClientTop = 0;
	m_nClientRight = 0;
	m_nClientBottom = 0;

	m_bEnableScroll = false;
	m_nScrollX = 0;
	m_nScrollY = 0;
	m_nScrollWidth = 1;
	m_nScrollHeight = 1;
	m_bShowVerticalBar = true;
	m_bShowHorizontalBar = true;
	m_nScrollBarWidth = 8;
}

XUIWidget::XUIWidget(const char* pName, int nLeft, int nTop, int nWidth, int nHeight)
{
	m_sName = pName;
	m_bManualFree = false;

	m_pParent = NULL;
	m_pNext = NULL;
	m_pPrev = NULL;
	m_pFirstChild = NULL;
	m_pLastChild = NULL;

	m_bDelete = false;
	m_bEnable = true;
	m_bVisable = true;
	m_bTransparent = false;
	m_bMouseIn = false;

	m_nLeft = nLeft;
	m_nTop = nTop;
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	m_nClientLeft = 0;
	m_nClientTop = 0;
	m_nClientRight = 0;
	m_nClientBottom = 0;

	m_bEnableScroll = false;
	m_nScrollX = 0;
	m_nScrollY = 0;
	m_nScrollWidth = 1;
	m_nScrollHeight = 1;
	m_bShowVerticalBar = true;
	m_bShowHorizontalBar = true;
	m_nScrollBarWidth = 8;
}


XUIWidget::~XUIWidget()
{
}

XUI* XUIWidget::GetXUI()
{
	XUIWidgetRoot* pRoot = GetRoot();
	return pRoot?pRoot->GetXUI():NULL;
}

XUIWidgetRoot* XUIWidget::GetRoot()
{
	XUIWidget* pWidget = this;
	while(pWidget->GetParent())
		pWidget = pWidget->GetParent();
	XUIWidgetRoot* pRoot;
	try {
		pRoot = dynamic_cast<XUIWidgetRoot*>(pWidget);
	} catch(...) {
		pRoot = NULL;
	}
	return pRoot;
}

XUIWidget* XUIWidget::GetWidget(const char* pName)
{
	XUIWidget* pWidget = GetFirstChild();
	while(pWidget) {
		if(pWidget->GetWidgetName()==pName) return pWidget;
		pWidget = pWidget->GetNext();
	}
	return NULL;
}

bool XUIWidget::IsParent(XUIWidget* pWidget)
{
	while(pWidget) {
		if(pWidget==this) return true;
		pWidget = pWidget->GetParent();
	}
	return false;
}

XUIWidget* XUIWidget::GetParent()
{
	return m_pParent;
}

XUIWidget* XUIWidget::GetFirstChild()
{
	XUIWidget* pWidget = m_pFirstChild;
	while(pWidget!=NULL && pWidget->m_bDelete) {
		pWidget = pWidget->m_pNext;
	}
	return pWidget;
}

XUIWidget* XUIWidget::GetLastChild()
{
	XUIWidget* pWidget = m_pLastChild;
	while(pWidget!=NULL && pWidget->m_bDelete) {
		pWidget = pWidget->m_pPrev;
	}
	return pWidget;
}

XUIWidget* XUIWidget::GetNext()
{
	XUIWidget* pWidget = m_pNext;
	while(pWidget!=NULL && pWidget->m_bDelete) {
		pWidget = pWidget->m_pNext;
	}
	return pWidget;
}

XUIWidget* XUIWidget::GetPrev()
{
	XUIWidget* pWidget = m_pPrev;
	while(pWidget!=NULL && pWidget->m_bDelete) {
		pWidget = pWidget->m_pPrev;
	}
	return pWidget;
}

void XUIWidget::AddChild(XUIWidget* pWidget)
{
	if(m_pLastChild) {
		pWidget->m_pParent = this;
		pWidget->m_pNext = NULL;
		pWidget->m_pPrev = m_pLastChild;
		m_pLastChild->m_pNext = pWidget;
		m_pLastChild = pWidget;
	} else {
		pWidget->m_pParent = this;
		pWidget->m_pPrev = NULL;
		pWidget->m_pNext = NULL;
		m_pFirstChild = pWidget;
		m_pLastChild = pWidget;
	}
}

void XUIWidget::Close()
{
	m_bDelete = true;
}

void XUIWidget::BringToTop()
{
	if(!m_pParent)
		return;

	DelWidget(m_pParent, this);
	m_pParent->AddChild(this);
}

bool XUIWidget::IsVisable()
{
	return m_bVisable;
}

void XUIWidget::SetFocus()
{
	GetXUI()->SetFocus(this);
}

bool XUIWidget::HasFocus()
{
	XUIWidget* pFocus = GetXUI()->GetFocus();
	while(pFocus) {
		if(pFocus==this) return true;
		pFocus = pFocus->GetParent();
	}
	return false;
}

bool XUIWidget::IsEnable()
{
	XUIWidget* pWidget = this;
	while(pWidget) {
		if(!pWidget->m_bEnable) return false;
		pWidget = pWidget->GetParent();
	}
	return true;
}

bool XUIWidget::MouseIn()
{
	return m_bMouseIn;
}

XUIPoint& XUIWidget::ScreenToWidget(const XUIPoint& In, XUIPoint& Out)
{
	XUIPoint P = In;
	XUIWidget* pWidget = this;
	while(pWidget) {
		P.x -= pWidget->m_nLeft;
		P.y -= pWidget->m_nTop;
		if(pWidget!=this) {
			P.x += (pWidget->m_bEnableScroll?pWidget->m_nScrollX:0) - pWidget->m_nClientLeft;
			P.y += (pWidget->m_bEnableScroll?pWidget->m_nScrollY:0) - pWidget->m_nClientTop;
		}
		pWidget = pWidget->GetParent();
	}
	Out = P;
	return Out;
}

XUIPoint& XUIWidget::WidgetToScreen(const XUIPoint& In, XUIPoint& Out)
{
	XUIPoint P = In;
	XUIWidget* pWidget = this;
	while(pWidget) {
		P.x += pWidget->m_nLeft;
		P.y += pWidget->m_nTop;
		if(pWidget!=this) {
			P.x -= (pWidget->m_bEnableScroll?pWidget->m_nScrollX:0) - pWidget->m_nClientLeft;
			P.y -= (pWidget->m_bEnableScroll?pWidget->m_nScrollY:0) - pWidget->m_nClientTop;
		}
		pWidget = pWidget->GetParent();
	}
	Out = P;
	return Out;
}

void XUIWidget::CenterWidget()
{
	if(m_pParent) {
		SetWidgetPosition((m_pParent->GetClientWidth()-GetWidgetWidth())/2, (m_pParent->GetClientHeight()-GetWidgetHeight())/2);
	}
}

void XUIWidget::MaxiumWidget()
{
	if(m_pParent) {
		SetWidgetRect(0, 0, m_pParent->GetClientWidth(), m_pParent->GetClientHeight());
	}
}

void XUIWidget::SetWidgetRect(int nLeft, int nTop, int nWidth, int nHeight)
{
	bool bMove = (m_nLeft!=nLeft || m_nTop!=nTop);
	bool bSize = (m_nWidth!=nWidth || m_nHeight!=nHeight);
	m_nLeft = nLeft;
	m_nTop = nTop;
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	if(bMove) OnWidgetMove(m_nLeft, m_nTop);
	if(bSize) OnSizeChange(m_nWidth, m_nHeight);
}

void XUIWidget::SetWidgetPosition(int nLeft, int nTop)
{
	m_nLeft = nLeft;
	m_nTop = nTop;
	OnWidgetMove(m_nLeft, m_nWidth);
}

void XUIWidget::SetWidgetSize(int nWidth, int nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	OnSizeChange(m_nWidth, m_nHeight);
}

void XUIWidget::SetClientArea(int nLeft, int nTop, int nRight, int nBottom)
{
	m_nClientLeft	= nLeft;
	m_nClientTop	= nTop;
	m_nClientRight	= nRight;
	m_nClientBottom	= nBottom;
}

void XUIWidget::EnableScroll(bool bEnable)
{
	m_bEnableScroll = bEnable;
	AdjustScroll();
}

void XUIWidget::SetScrollPosition(const XUIPoint& Scroll)
{
	m_nScrollX = Scroll.x;
	m_nScrollY = Scroll.y;
	AdjustScroll();
}

void XUIWidget::SetScrollSize(int nWidth, int nHeight)
{
	m_nScrollWidth = nWidth;
	m_nScrollHeight = nHeight;
	AdjustScroll();
}

void XUIWidget::AdjustScroll(bool bSilence)
{
	if(m_bEnableScroll) {
		if(m_nScrollWidth<GetClientWidth() || !m_bShowHorizontalBar) {
			m_nScrollX = 0;
		} else {
			if(m_nScrollX<0) m_nScrollX = 0;
			if(m_nScrollX+GetClientWidth()>m_nScrollWidth) m_nScrollX = m_nScrollWidth - GetClientWidth();
		}

		if(m_nScrollHeight<GetClientHeight() || !m_bShowVerticalBar) {
			m_nScrollY = 0;
		} else {
			if(m_nScrollY<0) m_nScrollY = 0;
			if(m_nScrollY+GetClientHeight()>m_nScrollHeight) m_nScrollY = m_nScrollHeight - GetClientHeight();
		}

		if(!bSilence) {
			OnSizeChange(GetWidgetWidth(), GetWidgetHeight());
		}
	}
}

void XUIWidget::ShowVerticalBar(bool bShow)
{
	if(m_bShowVerticalBar!=bShow) {
		m_bShowVerticalBar = bShow;
		OnSizeChange(GetWidgetWidth(), GetWidgetHeight());
	}
}

void XUIWidget::ShowHorizontalBar(bool bShow)
{
	if(m_bShowHorizontalBar!=bShow) {
		m_bShowHorizontalBar = bShow;
		OnSizeChange(GetWidgetWidth(), GetWidgetHeight());
	}
}

void XUIWidget::SetScrollBarWidth(int nWidth)
{
	m_nScrollBarWidth = nWidth;
	if(m_bShowVerticalBar || m_bShowHorizontalBar) {
		OnSizeChange(GetWidgetWidth(), GetWidgetHeight());
	}
}

void XUIWidget::DoCommand(XUIWidget* pWidget, int nCode)
{
	assert(this==pWidget->m_pParent);
	_eventCommand(pWidget, nCode);
}

void XUIWidget::ActiveWidget(XUIWidget* pWidget)
{
	if(m_pParent) {
		m_pParent->ActiveWidget(pWidget);
	}
}

void XUIWidget::Destroy()
{
	while(m_pFirstChild) {
		XUIWidget* pWidget = m_pFirstChild;
		DelWidget(this, pWidget);
		pWidget->Destroy();
	}

	if(!m_bManualFree) delete this;
}

void XUIWidget::OnEraseBKGnd(XUIDevice* pDevice)
{
}

void XUIWidget::OnRender(XUIDevice* pDevice)
{
	if(m_bEnableScroll) {
		if(m_bShowVerticalBar) {
			int nBarStart, nBarHeight;
			if(GetScrollHeight()>GetClientHeight()) {
				nBarStart = (int)((float)GetScrollPositionY()/(float)GetScrollHeight() * GetClientHeight());
				nBarHeight = (int)((float)(GetScrollPositionY()+GetClientHeight())/(float)GetScrollHeight() * GetClientHeight()) - nBarStart;
				if(nBarStart+nBarHeight>GetClientHeight()) nBarHeight = GetClientHeight() - nBarStart;
			} else {
				nBarStart = 0;
				nBarHeight = GetClientHeight();
			}

			pDevice->AddRect(m_nWidth-m_nClientRight-m_nScrollBarWidth, m_nClientTop,           m_nScrollBarWidth, GetClientHeight(), m_nScrollBarWidth/2-1, XUI_RGBA(0,0,0,255));
			pDevice->AddRect(m_nWidth-m_nClientRight-m_nScrollBarWidth, m_nClientTop+nBarStart, m_nScrollBarWidth, nBarHeight,        m_nScrollBarWidth/2-1, XUI_RGBA(255,255,255,200));
		}

		if(m_bShowHorizontalBar) {
			int nBarStart, nBarWidth;
			if(GetScrollWidth()>GetClientWidth()) {
				nBarStart = (int)((float)GetScrollPositionX()/(float)GetScrollWidth() * GetClientWidth());
				nBarWidth = (int)((float)(GetScrollPositionX()+GetClientWidth())/(float)GetScrollWidth() * GetClientWidth()) - nBarStart;
				if(nBarStart+nBarWidth>GetClientWidth()) nBarWidth = GetClientWidth() - nBarStart;
			} else {
				nBarStart = 0;
				nBarWidth = GetClientWidth();
			}

			pDevice->AddRect(m_nClientLeft,			  m_nHeight-m_nClientBottom-m_nScrollBarWidth, GetClientWidth(), m_nScrollBarWidth, m_nScrollBarWidth/2-1, XUI_RGBA(0,0,0,255));
			pDevice->AddRect(m_nClientLeft+nBarStart, m_nHeight-m_nClientBottom-m_nScrollBarWidth, nBarWidth       , m_nScrollBarWidth, m_nScrollBarWidth/2-1, XUI_RGBA(255,255,255,200));
		}
	}

	XUIWidget* pWidget = GetFirstChild();
	if(pWidget) {
		if(m_bEnableScroll) {
			pDevice->AddBeginScissor(m_nClientLeft, m_nClientTop, GetClientWidth(), GetClientHeight());
			pDevice->AddBeginScissor(m_nClientLeft-m_nScrollX, m_nClientTop-m_nScrollY);
		} else {
			pDevice->AddBeginScissor(m_nClientLeft, m_nClientTop);
		}

		while(pWidget!=NULL) {
			if(pWidget->IsVisable()) {
				pDevice->AddBeginScissor(pWidget->m_nLeft, pWidget->m_nTop);
				if(!pWidget->m_bTransparent) {
					pWidget->OnEraseBKGnd(pDevice);
				}
				pWidget->OnRender(pDevice);
				pDevice->AddEndScissor();
			}
			pWidget = pWidget->GetNext();
		}

		if(m_bEnableScroll) {
			pDevice->AddEndScissor();
			pDevice->AddEndScissor();
		} else {
			pDevice->AddEndScissor();
		}
	}
}

void XUIWidget::OnLostFocus(XUIWidget* pNew)
{
	_eventLostFocus(this, pNew);
}

bool XUIWidget::OnSetFocus(XUIWidget* pOld)
{
	_eventSetFocus(this, pOld);
	return false;
}

void XUIWidget::OnMouseMove(const XUIPoint& Point)
{
	if(m_nCaptureScroll>=0) {
		if(m_nCaptureVertical) {
			int nScroll = m_nCaptureScroll + (int)(((float)Point.y-m_nCaptureV)/GetClientHeight()*GetScrollHeight());
			SetScrollPosition(XUIPoint(m_nScrollX, nScroll));
		} else {
			int nScroll = m_nCaptureScroll + (int)(((float)Point.x-m_nCaptureV)/GetClientWidth()*GetScrollWidth());
			SetScrollPosition(XUIPoint(nScroll, m_nScrollY));
		}
	}

	_eventMouseMove(this, Point);
}

void XUIWidget::OnMouseEnter()
{
	_eventMouseEnter(this);
}

void XUIWidget::OnMouseLeave()
{
	_eventMouseLeave(this);
}

bool XUIWidget::OnMouseWheel(const XUIPoint& Point, int _rel)
{
	if(m_bEnableScroll) {
		if(m_bShowVerticalBar) {
			SetScrollPosition(XUIPoint(m_nScrollX, m_nScrollY - _rel*20));
			return true;
		}
		if(m_bShowHorizontalBar) {
			SetScrollPosition(XUIPoint(m_nScrollX - _rel*20, m_nScrollX));
			return true;
		}
	}

	_eventMouseWheel(this, Point, _rel);
	return false;
}

void XUIWidget::OnMouseButtonPressed(const XUIPoint& Point, unsigned short nId)
{
	if(nId==XUI_INPUT::MOUSE_LBUTTON) {
		if(m_bShowVerticalBar && GetScrollHeight()>GetClientHeight()) {
			int nLeft = Point.x - (m_nWidth - m_nClientRight - m_nScrollBarWidth);
			int nTop  = Point.y - m_nClientTop;
			if(nLeft>=0 && nLeft<m_nScrollBarWidth && nTop>0 && nTop<GetClientHeight()) {
				int nBarStart = (int)((float)GetScrollPositionY()/(float)GetScrollHeight() * GetClientHeight());
				int nBarHeight = (int)((float)(GetScrollPositionY()+GetClientHeight())/(float)GetScrollHeight() * GetClientHeight()) - nBarStart;
				if(nTop>=nBarStart && nTop<nBarStart+nBarHeight) {
					m_nCaptureScroll = GetScrollPositionY();
					m_nCaptureV = Point.y;
				} else {
					m_nCaptureScroll = (int)((nTop-nBarHeight/2)/(float)GetClientHeight()*GetScrollHeight());
					if(m_nCaptureScroll<0) m_nCaptureScroll = 0;
					if(m_nCaptureScroll>=GetScrollHeight()-GetClientHeight()) m_nCaptureScroll = GetScrollHeight()-GetClientHeight();
					m_nCaptureV = Point.y;
					SetScrollPosition(XUIPoint(m_nScrollX, m_nCaptureScroll));
				}

				m_nCaptureVertical = true;
				GetXUI()->SetCapture(this, true);
			}
		}
		if(m_bShowHorizontalBar && GetScrollWidth()>GetClientWidth()) {
			int nTop  = Point.y - (m_nHeight - m_nClientBottom - m_nScrollBarWidth);
			int nLeft = Point.x - m_nClientLeft;
			if(nTop>=0 && nTop<m_nScrollBarWidth && nLeft>0 && nLeft<GetClientWidth()) {
				int nBarStart = (int)((float)GetScrollPositionX()/(float)GetScrollWidth() * GetClientWidth());
				int nBarWidth = (int)((float)(GetScrollPositionX()+GetClientWidth())/(float)GetScrollWidth() * GetClientWidth()) - nBarStart;
				if(nLeft>=nBarStart && nLeft<nBarStart+nBarWidth) {
					m_nCaptureScroll = GetScrollPositionX();
					m_nCaptureV = Point.x;
				} else {
					m_nCaptureScroll = (int)((nLeft-nBarWidth/2)/(float)GetClientWidth()*GetScrollWidth());
					if(m_nCaptureScroll<0) m_nCaptureScroll = 0;
					if(m_nCaptureScroll>=GetScrollWidth()-GetClientWidth()) m_nCaptureScroll = GetScrollWidth()-GetClientWidth();
					m_nCaptureV = Point.x;
					SetScrollPosition(XUIPoint(m_nCaptureScroll, m_nScrollY));
				}

				m_nCaptureVertical = false;
				GetXUI()->SetCapture(this, true);
			}
		}
	}
	_eventMouseButtonPressed(this, Point, nId);
}

void XUIWidget::OnMouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
	if(nId==XUI_INPUT::MOUSE_LBUTTON) {
		m_nCaptureScroll = -1;
		GetXUI()->SetCapture(this, false);
	}

	_eventMouseButtonReleased(this, Point, nId);
}

void XUIWidget::OnMouseButtonClick(const XUIPoint& Point, unsigned short nId)
{
	_eventMouseButtonClick(this, Point, nId);
}

void XUIWidget::OnMouseButtonDBClick(const XUIPoint& Point, unsigned short nId)
{
	_eventMouseButtonDoubleClick(this, Point, nId);
}

void XUIWidget::OnKeyPressed(unsigned short nKey)
{
	_eventKeyPressed(this, nKey);
}

void XUIWidget::OnKeyReleased(unsigned short nKey)
{
	_eventKeyReleased(this, nKey);
}

void XUIWidget::OnKeyChar(unsigned short nKey, unsigned int Char)
{
	_eventKeyChar(this, nKey, Char);
}

void XUIWidget::OnWidgetMove(int nLeft, int nTop)
{
	_eventWidgetMove(this, nLeft, nTop);
}

void XUIWidget::OnSizeChange(int nWidth, int nHeight)
{
	AdjustScroll(true);
	_eventSizeChange(this, nWidth, nHeight);
}

void XUIWidget::OnClose()
{
	_eventClose(this);
}

XUI::XUI()
{
	m_Root.m_pXUI = this;
	m_Root.SetWidgetPosition(0, 0);

	m_pFocus	= NULL;
	m_pOver		= NULL;
	m_pCapture	= NULL;

	m_pLButtonWidget = NULL;
	m_pRButtonWidget = NULL;
	m_pMButtonWidget = NULL;
}

XUI::~XUI()
{
	m_Root.Destroy();
}

XUIWidget* XUI::GetWidget(const XUIPoint& Point)
{
	XUIWidget* pWidget = m_Root.GetLastChild();
	XUIWidget* pReturn = NULL;
	XUIPoint P = Point;
	while(pWidget) {
		P.x -= pWidget->m_nLeft;
		P.y -= pWidget->m_nTop;
		if(P.x>=0 && P.y>=0 && P.x<pWidget->m_nWidth && P.y<pWidget->m_nHeight && pWidget->IsVisable()) {
			pReturn = pWidget;
			P.x -= pWidget->m_nClientLeft;
			P.y -= pWidget->m_nClientTop;
			if(P.x<0 || P.y<0 || P.x>=pWidget->GetClientWidth() || P.y>=pWidget->GetClientHeight()) break;
			if(pWidget->m_bEnableScroll) {
				P.x += pWidget->m_nScrollX;
				P.y += pWidget->m_nScrollY;
			}
			pWidget = pWidget->GetLastChild();
			continue;
		}
		P.x += pWidget->m_nLeft;
		P.y += pWidget->m_nTop;

		pWidget = pWidget->GetPrev();
	}
	return pReturn?pReturn:&m_Root;
}

void XUI::SetCapture(XUIWidget* pWidget, bool bEnable)
{
	if(bEnable) {
		if(pWidget)
			m_pCapture = pWidget;
	} else {
		if(m_pCapture==pWidget)
			m_pCapture = NULL;
	}
}

void XUI::SetFocus(XUIWidget* pWidget)
{
	static int nStaticSeq = 0;
	int nCurrentSeq = ++nStaticSeq;

	if(pWidget!=m_pFocus) {
		if(m_pFocus) {
			m_pFocus->OnLostFocus(pWidget);
			if(nCurrentSeq!=nStaticSeq) return;
		}

		XUIWidget* pOld = m_pFocus;
		m_pFocus = pWidget;
		if(m_pFocus) {
			m_pFocus->ActiveWidget(m_pFocus);
			m_pFocus->OnSetFocus(pOld);
		}
	}
}

void XUI::MouseMove(const XUIPoint& Point)
{
	XUIWidget* pWidget = GetWidget(Point);

	XUIPoint wp;
	if(pWidget) {
		if(m_pOver!=pWidget) {
			if(m_pOver) {
				m_pOver->m_bMouseIn = false;
				m_pOver->OnMouseLeave();
			}
			pWidget->m_bMouseIn = true;
			pWidget->OnMouseEnter();
		}
		pWidget->OnMouseMove(pWidget->ScreenToWidget(Point, wp));
	} else {
		if(m_pOver) {
			m_pOver->m_bMouseIn = false;
			m_pOver->OnMouseLeave();
		}
	}
	if(m_pCapture && m_pCapture!=pWidget) {
		m_pCapture->OnMouseMove(m_pCapture->ScreenToWidget(Point, wp));
	}

	m_pOver = pWidget;
}

void XUI::MouseWheel(const XUIPoint& Point, int _rel) 
{
	XUIWidget* pWidget = GetWidget(Point);
	while(pWidget) {
		XUIPoint wp;
		if(pWidget->OnMouseWheel(pWidget->ScreenToWidget(Point, wp), _rel)) break;
		pWidget = pWidget->GetParent();
	}
}

void XUI::MouseButtonPressed(const XUIPoint& Point, unsigned short nId)
{
	XUIWidget* pWidget = GetWidget(Point);

	switch(nId) {
	case XUI_INPUT::MOUSE_LBUTTON: m_pLButtonWidget = pWidget; m_LButtonPoint = Point; break;
	case XUI_INPUT::MOUSE_RBUTTON: m_pRButtonWidget = pWidget; m_RButtonPoint = Point; break;
	case XUI_INPUT::MOUSE_MBUTTON: m_pMButtonWidget = pWidget; m_MButtonPoint = Point; break;
	}

	if(m_pCapture!=pWidget && m_pCapture!=NULL) {
		XUIPoint wp;
		m_pCapture->OnMouseButtonPressed(m_pCapture->ScreenToWidget(Point, wp), nId);
	}

	if(pWidget) {
		XUIPoint wp;
		pWidget->OnMouseButtonPressed(pWidget->ScreenToWidget(Point, wp), nId);
		if(nId==XUI_INPUT::MOUSE_LBUTTON) {
			pWidget->ActiveWidget(pWidget);
		}
	}
}

void XUI::MouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
	XUIWidget* pWidget = GetWidget(Point);

	if(m_pCapture!=pWidget && m_pCapture!=NULL) {
		XUIPoint wp;
		m_pCapture->OnMouseButtonReleased(m_pCapture->ScreenToWidget(Point, wp), nId);
	}

	if(pWidget) {
		XUIPoint wp;
		pWidget->ScreenToWidget(Point, wp);
		pWidget->OnMouseButtonReleased(wp, nId);

		const XUIPoint* pOld = NULL;
		XUIWidget* pPressedWidget = pWidget;
		switch(nId) {
		case XUI_INPUT::MOUSE_LBUTTON: pPressedWidget = m_pLButtonWidget; pOld = &m_LButtonPoint; break;
		case XUI_INPUT::MOUSE_RBUTTON: pPressedWidget = m_pRButtonWidget; pOld = &m_RButtonPoint; break;
		case XUI_INPUT::MOUSE_MBUTTON: pPressedWidget = m_pLButtonWidget; pOld = &m_MButtonPoint; break;
		}
		if(pOld) {
			if(pPressedWidget==pWidget) {
				pWidget->OnMouseButtonClick(wp, nId);
			}
			/*
			int x = pOld->x - Point.x;
			int y = pOld->y - Point.y;
			if(x>=-1 && x<=1 && y>=-1 && y<=1) {
				pWidget->OnMouseButtonClick(wp, nId);
			}
			*/
		}
	}
}

void XUI::MouseButtonClick(const XUIPoint& Point, unsigned short nId)
{
	XUIWidget* pWidget = GetWidget(Point);
	if(pWidget) {
		XUIPoint wp;
		pWidget->OnMouseButtonClick(pWidget->ScreenToWidget(Point, wp), nId);
	}
}

void XUI::MouseButtonDoubleClick(const XUIPoint& Point, unsigned short nId)
{
	XUIWidget* pWidget = GetWidget(Point);
	if(pWidget) {
		XUIPoint wp;
		pWidget->OnMouseButtonDBClick(pWidget->ScreenToWidget(Point, wp), nId);
	}
}

void XUI::KeyPressed(unsigned short nKey)
{
	if(m_pFocus) {
		m_pFocus->OnKeyPressed(nKey);
	}
}

void XUI::KeyReleased(unsigned short nKey)
{
	if(m_pFocus) {
		m_pFocus->OnKeyReleased(nKey);
	}
}

void XUI::KeyChar(unsigned short nKey, unsigned int Char)
{
	if(m_pFocus) {
		m_pFocus->OnKeyChar(nKey, Char);
	}
}

void XUI::BeginFrame()
{
}

void XUI::EndFrame()
{
	// remove all deleted widgets
	XUIWidget* pStack[20];
	int nStack = 0;
	pStack[0] = m_Root.GetLastChild();
	for(;;) {
		if(!pStack[nStack]) {
			if(nStack==0) break;
			nStack--;
			continue;
		}

		XUIWidget* pWidget = pStack[nStack];
		pStack[nStack] = pStack[nStack]->GetPrev();

		if(pWidget->m_bDelete) {
			XUIWidget* pFocus = NULL;
			if(pWidget->HasFocus()) {
				m_pFocus = NULL;
				pFocus = pWidget->GetParent();
			}

			if(pWidget->IsParent(m_pOver)) {
				m_pOver = NULL;
			}

			if(pWidget->IsParent(m_pCapture)) {
				m_pCapture = NULL;
			}

			DelWidget(pWidget->GetParent(), pWidget);
			pWidget->Destroy();

			if(pFocus) {
				pFocus->ActiveWidget(pFocus);
			}
			continue;
		}

		if(pWidget->GetLastChild()) {
			pStack[++nStack] = pWidget->GetLastChild();
		}
	}
}

void XUI::Render(XUIDevice* pDevice)
{
	pDevice->Render(&m_Root);
}

void XUI::Reset(int nWidth, int nHeight)
{
	m_Root.SetWidgetSize(nWidth, nHeight);
}
