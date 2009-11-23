#include <stddef.h>
#include <assert.h>

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

	m_nLeft = 0;
	m_nTop = 0;
	m_nWidth = 0;
	m_nHeight = 0;

	m_nClientLeft = 0;
	m_nClientTop = 0;
	m_nClientRight = 0;
	m_nClientBottom = 0;

	m_bScroll = false;
	m_nScrollX = 0;
	m_nScrollY = 0;
	m_nScrollWidth = 1;
	m_nScrollHeight = 1;
}

XUIWidget::~XUIWidget()
{
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

void XUIWidget::BringToTop()
{
	if(!m_pParent)
		return;

	DelWidget(m_pParent, this);
	m_pParent->AddChild(this);
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
		if(!pWidget->m_bVisable) return false;
		pWidget = pWidget->GetNext();
	}
	return NULL;
}

bool XUIWidget::IsVisable()
{
	return m_bVisable;
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

XUIPoint& XUIWidget::ScreenToWidget(const XUIPoint& In, XUIPoint& Out)
{
	XUIPoint P = In;
	XUIWidget* pWidget = this;
	while(pWidget) {
		P.x -= pWidget->m_nLeft;
		P.y -= pWidget->m_nTop;
		if(pWidget!=this) {
			P.x += (pWidget->m_bScroll?pWidget->m_nScrollX:0) - pWidget->m_nClientLeft;
			P.y += (pWidget->m_bScroll?pWidget->m_nScrollY:0) - pWidget->m_nClientTop;
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
			P.x -= (pWidget->m_bScroll?pWidget->m_nScrollX:0) - pWidget->m_nClientLeft;
			P.y -= (pWidget->m_bScroll?pWidget->m_nScrollY:0) - pWidget->m_nClientTop;
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
	m_bScroll = bEnable;
	AdjustScroll();
}

void XUIWidget::SetScrollPosition(const XUIPoint& Scroll)
{
	m_nScrollX = Scroll.x;
	m_nScrollY = Scroll.y;
	if(m_bScroll) AdjustScroll();
}

void XUIWidget::SetScrollSize(int nWidth, int nHeight)
{
	m_nScrollWidth = nWidth;
	m_nScrollHeight = nHeight;
	if(m_bScroll) AdjustScroll();
}

void XUIWidget::AdjustScroll()
{
	if(m_nScrollWidth<GetClientWidth()) {
		m_nScrollX = 0;
	} else {
		if(m_nScrollX<0) m_nScrollX = 0;
		if(m_nScrollX+GetClientWidth()>m_nScrollWidth) m_nScrollX = m_nScrollWidth - GetClientWidth();
	}

	if(m_nScrollHeight<GetClientHeight()) {
		m_nScrollY = 0;
	} else {
		if(m_nScrollY<0) m_nScrollY = 0;
		if(m_nScrollY+GetClientHeight()>m_nScrollHeight) m_nScrollY = m_nScrollHeight - GetClientHeight();
	}
}

void XUIWidget::onRender(XUIDevice* pDevice)
{
	XUIWidget* pWidget = GetFirstChild();

	if(pWidget) {
		if(m_bScroll) {
			pDevice->AddBeginScissor(m_nClientLeft, m_nClientTop, GetClientWidth(), GetClientHeight());
			pDevice->AddBeginScissor(m_nClientLeft-m_nScrollX, m_nClientTop-m_nScrollY);
		} else {
			pDevice->AddBeginScissor(m_nClientLeft, m_nClientTop);
		}

		while(pWidget!=NULL) {
			if(pWidget->IsVisable()) {
				pDevice->AddBeginScissor(pWidget->m_nLeft, pWidget->m_nTop);
				pWidget->onRender(pDevice);
				pDevice->AddEndScissor();
			}
			pWidget = pWidget->GetNext();
		}

		if(m_bScroll) {
			pDevice->AddEndScissor();
			pDevice->AddEndScissor();
		} else {
			pDevice->AddEndScissor();
		}
	}
}

void XUIWidget::onLostFocus(XUIWidget* pNew)
{
	_eventLostFocus.Invoke(this, pNew);
}

bool XUIWidget::onSetFocus(XUIWidget* pOld)
{
	_eventSetFocus.Invoke(this, pOld);
	return false;
}

void XUIWidget::onMouseMove(const XUIPoint& Point)
{
	_eventMouseMove.Invoke(this, Point);
}

void XUIWidget::onMouseEnter()
{
	_eventMouseEnter.Invoke(this);
}

void XUIWidget::onMouseLeave()
{
	_eventMouseLeave.Invoke(this);
}

bool XUIWidget::onMouseWheel(const XUIPoint& Point, int _rel)
{
	_eventMouseWheel.Invoke(this, Point, _rel);
	return false;
}

void XUIWidget::onMouseButtonPressed(const XUIPoint& Point, unsigned short nId)
{
	_eventMouseButtonPressed.Invoke(this, Point, nId);
}

void XUIWidget::onMouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
	_eventMouseButtonReleased.Invoke(this, Point, nId);
}

void XUIWidget::onMouseButtonClick(const XUIPoint& Point, unsigned short nId)
{
	_eventMouseButtonClick.Invoke(this, Point, nId);
}

void XUIWidget::onMouseButtonDoubleClick(const XUIPoint& Point, unsigned short nId)
{
	_eventMouseButtonDoubleClick.Invoke(this, Point, nId);
}

void XUIWidget::onKeyPressed(unsigned short nKey)
{
	_eventKeyPressed.Invoke(this, nKey);
}

void XUIWidget::onKeyReleased(unsigned short nKey)
{
	_eventKeyReleased.Invoke(this, nKey);
}

void XUIWidget::onKeyChar(unsigned short nKey, unsigned int Char)
{
	_eventKeyChar.Invoke(this, nKey, Char);
}

void XUIWidget::OnWidgetMove(int nLeft, int nTop)
{
	_eventWidgetMove.Invoke(this, nLeft, nTop);
}

void XUIWidget::OnSizeChange(int nWidth, int nHeight)
{
	_eventSizeChange.Invoke(this, nWidth, nHeight);
}

XUI::XUI()
{
	m_Root.m_pXUI = this;
	m_Root.SetWidgetPosition(0, 0);
	m_Root.ManualFree();
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
			if(pWidget->m_bScroll) {
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
	return pReturn;
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
	m_pFocus = pWidget;
}

void XUI::MouseMove(const XUIPoint& Point)
{
	XUIWidget* pWidget = GetWidget(Point);

	XUIPoint wp;
	if(pWidget) {
		if(m_pOver!=pWidget) {
			if(m_pOver) m_pOver->onMouseLeave();
			pWidget->onMouseEnter();
		}
		pWidget->onMouseMove(pWidget->ScreenToWidget(Point, wp));
	} else {
		if(m_pOver) {
			m_pOver->onMouseLeave();
		}
	}
	if(m_pCapture && m_pCapture!=pWidget) {
		m_pCapture->onMouseMove(m_pCapture->ScreenToWidget(Point, wp));
	}

	m_pOver = pWidget;
}

void XUI::MouseWheel(const XUIPoint& Point, int _rel) 
{
	XUIWidget* pWidget = GetWidget(Point);
	while(pWidget) {
		XUIPoint wp;
		if(pWidget->onMouseWheel(pWidget->ScreenToWidget(Point, wp), _rel)) break;
		pWidget = pWidget->GetParent();
	}
}

void XUI::MouseButtonPressed(const XUIPoint& Point, unsigned short nId)
{
	switch(nId) {
	case XUI_INPUT::MOUSE_LBUTTON: m_LButtonPoint = Point; break;
	case XUI_INPUT::MOUSE_RBUTTON: m_RButtonPoint = Point; break;
	case XUI_INPUT::MOUSE_MBUTTON: m_LButtonPoint = Point; break;
	}

	XUIWidget* pWidget = GetWidget(Point);

	if(m_pCapture!=pWidget && m_pCapture!=NULL) {
		XUIPoint wp;
		m_pCapture->onMouseButtonPressed(m_pCapture->ScreenToWidget(Point, wp), nId);
	}

	if(pWidget) {
		XUIPoint wp;
		pWidget->onMouseButtonPressed(pWidget->ScreenToWidget(Point, wp), nId);
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
		m_pCapture->onMouseButtonReleased(m_pCapture->ScreenToWidget(Point, wp), nId);
	}

	if(pWidget) {
		XUIPoint wp;
		pWidget->onMouseButtonReleased(pWidget->ScreenToWidget(Point, wp), nId);

		const XUIPoint* pOld = NULL;
		switch(nId) {
		case XUI_INPUT::MOUSE_LBUTTON: pOld = &m_LButtonPoint; break;
		case XUI_INPUT::MOUSE_RBUTTON: pOld = &m_RButtonPoint; break;
		case XUI_INPUT::MOUSE_MBUTTON: pOld = &m_MButtonPoint; break;
		}
		if(pOld) {
			int x = pOld->x - Point.x;
			int y = pOld->y - Point.y;
			if(x>=-1 && x<=1 && y>=-1 && y<=1) {
				pWidget->onMouseButtonClick(wp, nId);
			}
		}
	}
}

void XUI::MouseButtonClick(const XUIPoint& Point, unsigned short nId)
{
	XUIWidget* pWidget = GetWidget(Point);
	if(pWidget) {
		XUIPoint wp;
		pWidget->onMouseButtonClick(pWidget->ScreenToWidget(Point, wp), nId);
	}
}

void XUI::MouseButtonDoubleClick(const XUIPoint& Point, unsigned short nId)
{
	XUIWidget* pWidget = GetWidget(Point);
	if(pWidget) {
		XUIPoint wp;
		pWidget->onMouseButtonDoubleClick(pWidget->ScreenToWidget(Point, wp), nId);
	}
}

void XUI::KeyPressed(unsigned short nKey)
{
	if(m_pFocus) {
		m_pFocus->onKeyPressed(nKey);
	}
}

void XUI::KeyReleased(unsigned short nKey)
{
	if(m_pFocus) {
		m_pFocus->onKeyReleased(nKey);
	}
}

void XUI::KeyChar(unsigned short nKey, unsigned int Char)
{
	if(m_pFocus) {
		m_pFocus->onKeyChar(nKey, Char);
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
			DelWidget(pWidget->GetParent(), pWidget);
			pWidget->Destroy();
			continue;
		}

		if(pWidget->GetLastChild()) {
			pStack[++nStack] = pWidget->GetLastChild();
		}
	}
}

void XUI::Render(XUIDevice* pDevice)
{
	m_Root.onRender(pDevice);
}

void XUI::Reset(int nWidth, int nHeight)
{
	m_Root.SetWidgetSize(nWidth, nHeight);
}
