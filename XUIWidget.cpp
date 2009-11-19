#include <stddef.h>

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

XUIWidget::XUIWidget() : m_Scroll(0, 0)
{
	m_pParent = NULL;
	m_pNext = NULL;
	m_pPrev = NULL;
	m_pFirstChild = NULL;
	m_pLastChild = NULL;
	m_bDelete = false;
	m_bEnable = true;
	m_bVisable = false;
	m_nClientLeft = 0;
	m_nClientTop = 0;
	m_nClientRight = 0;
	m_nClientBottom = 0;
	m_nScrollWidth = 1;
	m_nScrollHeight = 1;
	m_bScroll = false;
}

XUIWidget::~XUIWidget()
{
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

bool XUIWidget::IsVisable()
{
	XUIWidget* pWidget = this;
	while(pWidget) {
		if(!pWidget->m_bVisable) return false;
		pWidget = pWidget->GetParent();
	}
	return true;
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
			P.x -= pWidget->m_nClientLeft - (pWidget->m_bScroll?pWidget->m_Scroll.x:0);
			P.y -= pWidget->m_nClientTop  - (pWidget->m_bScroll?pWidget->m_Scroll.y:0);
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
			P.x += pWidget->m_nClientLeft - (pWidget->m_bScroll?pWidget->m_Scroll.x:0);
			P.y += pWidget->m_nClientTop  - (pWidget->m_bScroll?pWidget->m_Scroll.y:0);
		}
		pWidget = pWidget->GetParent();
	}
	Out = P;
	return Out;
}

void XUIWidget::SetWidgetRect(int nLeft, int nTop, int nWidth, int nHeight)
{
	m_nLeft = nLeft;
	m_nTop = nTop;
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}

void XUIWidget::SetWidgetPosition(int nLeft, int nTop)
{
	m_nLeft = nLeft;
	m_nTop = nTop;
}

void XUIWidget::SetWidgetSize(int nWidth, int nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}

void XUIWidget::SetClientArea(int nLeft, int nTop, int nRight, int nBottom)
{
	m_nClientLeft = nLeft;
	m_nClientTop = nTop;
	m_nClientRight = nRight;
	m_nClientBottom = nBottom;
}

void XUIWidget::EnableScroll(bool bEnable)
{
	m_bScroll = bEnable;
	AdjustScroll();
}

void XUIWidget::SetScrollPosition(const XUIPoint& Scroll)
{
	m_Scroll = Scroll;
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
		m_Scroll.x = 0;
	} else {
		if(m_Scroll.x<0) m_Scroll.x = 0;
		if(m_Scroll.x+GetClientWidth()>m_nScrollWidth) m_Scroll.x = m_nScrollWidth - GetClientWidth();
	}

	if(m_nScrollHeight<GetClientHeight()) {
		m_Scroll.y = 0;
	} else {
		if(m_Scroll.y<0) m_Scroll.y = 0;
		if(m_Scroll.y+GetClientHeight()>m_nScrollHeight) m_Scroll.y = m_nScrollHeight - GetClientHeight();
	}
}

void XUIWidget::onRender(XUIDevice* pDevice)
{
	XUIWidget* pWidget = GetFirstChild();

	pDevice->AddBeginScissor(m_nClientLeft-(m_bScroll?m_Scroll.x:0), m_nClientTop-(m_bScroll?m_Scroll.y:0));
	while(pWidget!=NULL) {
		pDevice->AddBeginScissor(pWidget->m_nLeft, pWidget->m_nTop);
		pWidget->onRender(pDevice);
		pDevice->AddEndScissor();
		pWidget = pWidget->GetNext();
	}
	pDevice->AddEndScissor();
}

void XUIWidget::onLostFocus(XUIWidget* pNew)
{
}

void XUIWidget::onSetFocus(XUIWidget* pOld)
{
}

void XUIWidget::onMouseMove(const XUIPoint& Point)
{
}

void XUIWidget::onMouseEnter()
{
}

void XUIWidget::onMouseLeave()
{
}

void XUIWidget::onMouseWheel(const XUIPoint& Point, int _rel)
{
}

void XUIWidget::onMouseButtonPressed(const XUIPoint& Point, unsigned short nId)
{
}

void XUIWidget::onMouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
}

void XUIWidget::onMouseButtonClick(const XUIPoint& Point, unsigned short nId)
{
}

void XUIWidget::onMouseButtonDoubleClick(const XUIPoint& Point, unsigned short nId)
{
}

void XUIWidget::onKeyPressed(unsigned short nKey)
{
}

void XUIWidget::onKeyReleased(unsigned short nKey)
{
}

void XUIWidget::onKeyChar(unsigned short nKey, unsigned int Char)
{
}

XUIWidgetRoot::XUIWidgetRoot(XUI* pXUI)
{
	m_pXUI = pXUI;
	SetWidgetPosition(0, 0);
}

XUIWidgetRoot::~XUIWidgetRoot()
{
}

XUI::XUI()
{
	m_pRoot = new XUIWidgetRoot(this);
}

XUI::~XUI()
{
	delete m_pRoot;
}

XUIWidget* XUI::GetWidget(const XUIPoint& Point)
{
	XUIWidget* pWidget = m_pRoot->GetLastChild();
	XUIWidget* pReturn = NULL;
	XUIPoint P = Point;
	while(pWidget) {
		P.x -= pWidget->m_nLeft;
		P.y -= pWidget->m_nTop;
		if(P.x>=0 && P.y>=0 && P.x<pWidget->m_nWidth && P.y<pWidget->m_nHeight) {
			P.x -= pWidget->m_nClientLeft;
			P.y -= pWidget->m_nClientTop;
			pReturn = pWidget;
			if(P.x<0 || P.y<0 || P.x>=pWidget->GetClientWidth() || P.y>=pWidget->GetClientHeight()) break;

			if(pWidget->m_bScroll) {
				P.x += pWidget->m_Scroll.x;
				P.y += pWidget->m_Scroll.y;
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
	if(pWidget) {
		XUIPoint wp;
		pWidget->onMouseWheel(pWidget->ScreenToWidget(Point, wp), _rel);
	}
}

void XUI::MouseButtonPressed(const XUIPoint& Point, unsigned short nId)
{
	XUIWidget* pWidget = GetWidget(Point);

	if(pWidget) {
		XUIPoint wp;
		pWidget->onMouseButtonPressed(pWidget->ScreenToWidget(Point, wp), nId);
	}

	if(m_pCapture!=NULL && m_pCapture!=pWidget) {
		XUIPoint wp;
		m_pCapture->onMouseButtonPressed(m_pCapture->ScreenToWidget(Point, wp), nId);
	}
}

void XUI::MouseButtonReleased(const XUIPoint& Point, unsigned short nId)
{
	XUIWidget* pWidget = GetWidget(Point);

	if(pWidget) {
		XUIPoint wp;
		pWidget->onMouseButtonReleased(pWidget->ScreenToWidget(Point, wp), nId);
	}

	if(m_pCapture!=NULL && m_pCapture!=pWidget) {
		XUIPoint wp;
		m_pCapture->onMouseButtonReleased(m_pCapture->ScreenToWidget(Point, wp), nId);
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
	pStack[0] = m_pRoot->GetLastChild();
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
			delete pWidget;
			continue;
		}

		if(pWidget->GetLastChild()) {
			pStack[++nStack] = pWidget->GetLastChild();
		}
	}
}

void XUI::Render(XUIDevice* pDevice)
{
	m_pRoot->onRender(pDevice);
}

void XUI::Reset(int nWidth, int nHeight)
{
	m_pRoot->SetWidgetSize(nWidth, nHeight);
}
