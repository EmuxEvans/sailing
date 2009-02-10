/* DialogLayout
 * Copyright (C) Till Krullmann.
 *
 * The use and distribution terms for this software are covered by the
 * Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
 * which can be found in the file CPL.TXT at the root of this distribution.
 * By using this software in any fashion, you are agreeing to be bound by
 * the terms of this license. You must not remove this notice, or
 * any other, from this software.
 */

#include "DialogLayout.h"


/*
 * Since we use DeferWindowPos() for repositioning, we have to store the new coordinates
 * for controls in a separate place, because GetWindowRect() won't return the new coordinates
 * yet. This is done using window properties.
 */
ATOM g_atomPropHDWP = GlobalAddAtom(_T("DialogLayout_HDWP"));
ATOM g_atomPropTopLeft = GlobalAddAtom(_T("DialogLayout_TopLeft"));
ATOM g_atomPropBottomRight = GlobalAddAtom(_T("DialogLayout_BottomRight"));

#pragma warning(push)
#pragma warning(disable: 4312)



void GetNewWindowRect(HWND hWnd, HDWP hDwp, LPRECT pRect)
{
	ATLASSERT( ::IsWindow(hWnd) );
	ATLASSERT( pRect != NULL );

	if ( GetProp( hWnd, MAKEINTATOM(g_atomPropHDWP) ) == hDwp )
	{
		HANDLE hptTopLeft = GetProp( hWnd, MAKEINTATOM(g_atomPropTopLeft) ),
			hptBottomRight = GetProp( hWnd, MAKEINTATOM(g_atomPropBottomRight) );
		SetRect( pRect, LOWORD(hptTopLeft), HIWORD(hptTopLeft),
			LOWORD(hptBottomRight), HIWORD(hptBottomRight) );
	}
	else
	{
		::GetWindowRect(hWnd, pRect);

		CWindow wndParent = ::GetParent(hWnd);
		ATLASSERT( ::IsWindow(wndParent) );
		wndParent.ScreenToClient(pRect);
	}
}



void StoreNewWindowRect(HWND hWnd, HDWP hDwp, LPCRECT pRect)
{
	ATLASSERT( ::IsWindow(hWnd) );
	ATLASSERT( pRect != NULL );

	SetProp( hWnd, MAKEINTATOM(g_atomPropHDWP), (HANDLE) hDwp );

	HANDLE hptTopLeft = (HANDLE) MAKELONG( (SHORT) pRect->left, (SHORT) pRect->top ),
		hptBottomRight = (HANDLE) MAKELONG( (SHORT) pRect->right, (SHORT) pRect->bottom );
	SetProp( hWnd, MAKEINTATOM(g_atomPropTopLeft), hptTopLeft );
	SetProp( hWnd, MAKEINTATOM(g_atomPropBottomRight), hptBottomRight );
}

#pragma warning(pop)



///////////////////////////////////////////////////////////////////////////////
// CLayoutRuleAbsolute

LONG CLayoutRuleAbsolute::Apply(const CDialogTemplate& tmpl, UINT nDirection,
								 const CRect& rcLayout) const
{
	CRect rect(0, 0, 0, 0);
	if ( nDirection == LAYOUT_DIRECTION_LEFT || nDirection == LAYOUT_DIRECTION_RIGHT )
	{
		rect.right = labs(m_nPos);
		return (m_nPos >= 0) ? rcLayout.left + rect.right : rcLayout.right - rect.right;
	}
	else
	{
		rect.bottom = labs(m_nPos);
		return (m_nPos >= 0) ? rcLayout.top + rect.bottom : rcLayout.bottom - rect.bottom;
	}
}



LONG CLayoutRuleAbsolute::Apply(CWindow wndLayout, UINT nDirection,
								 const CRect& rcLayout) const
{
	CRect rect(0, 0, 0, 0);
	if ( nDirection == LAYOUT_DIRECTION_LEFT || nDirection == LAYOUT_DIRECTION_RIGHT )
	{
		rect.right = labs(m_nPos);
		::MapDialogRect( wndLayout, rect );
		return (m_nPos >= 0) ? rcLayout.left + rect.right : rcLayout.right - rect.right;
	}
	else
	{
		rect.bottom = labs(m_nPos);
		::MapDialogRect( wndLayout, rect );
		return (m_nPos >= 0) ? rcLayout.top + rect.bottom : rcLayout.bottom - rect.bottom;
	}
}



///////////////////////////////////////////////////////////////////////////////
// CLayoutRuleRelativeControl

LONG CLayoutRuleRelativeControl::Apply(const CDialogTemplate& tmpl, UINT nDirection,
										const CRect& rcLayout) const
{
	CDialogItemTemplate item = tmpl.FindControl( m_nCtrlID );
	ATLASSERT( item.IsValid() );

	switch ( this->m_nDirection )
	{
		case LAYOUT_DIRECTION_LEFT:
			ATLASSERT( nDirection == LAYOUT_DIRECTION_LEFT || nDirection == LAYOUT_DIRECTION_RIGHT );
			return item.GetX() - m_nPadding;
		case LAYOUT_DIRECTION_TOP:
			ATLASSERT( nDirection == LAYOUT_DIRECTION_TOP || nDirection == LAYOUT_DIRECTION_BOTTOM );
			return item.GetY() - m_nPadding;
		case LAYOUT_DIRECTION_RIGHT:
			ATLASSERT( nDirection == LAYOUT_DIRECTION_LEFT || nDirection == LAYOUT_DIRECTION_RIGHT );
			return item.GetX() + item.GetWidth() + m_nPadding;
		case LAYOUT_DIRECTION_BOTTOM:
			ATLASSERT( nDirection == LAYOUT_DIRECTION_TOP || nDirection == LAYOUT_DIRECTION_BOTTOM );
			return item.GetY() + item.GetHeight() + m_nPadding;
		default:
			ATLASSERT(FALSE);
			return 0;
	}

}



LONG CLayoutRuleRelativeControl::Apply(CWindow wndLayout, UINT nDirection,
										const CRect& rcLayout) const
{
	CWindow wndCtrl = wndLayout.GetDlgItem( m_nCtrlID );
	ATLASSERT( ::IsWindow(wndCtrl) );

	CRect rcPadding( 0, 0, m_nPadding, m_nPadding );
	::MapDialogRect( wndLayout, rcPadding );

	HDWP hDwp = (HDWP) GetProp( wndLayout, MAKEINTATOM(g_atomPropHDWP) );
	CRect rcCtrl;
	GetNewWindowRect( wndCtrl, hDwp, rcCtrl );

	switch ( this->m_nDirection )
	{
		case LAYOUT_DIRECTION_LEFT:
			ATLASSERT( nDirection == LAYOUT_DIRECTION_LEFT || nDirection == LAYOUT_DIRECTION_RIGHT );
			return rcCtrl.left - rcPadding.Width();
		case LAYOUT_DIRECTION_TOP:
			ATLASSERT( nDirection == LAYOUT_DIRECTION_TOP || nDirection == LAYOUT_DIRECTION_BOTTOM );
			return rcCtrl.top - rcPadding.Height();
		case LAYOUT_DIRECTION_RIGHT:
			ATLASSERT( nDirection == LAYOUT_DIRECTION_LEFT || nDirection == LAYOUT_DIRECTION_RIGHT );
			return rcCtrl.right + rcPadding.Width();
		case LAYOUT_DIRECTION_BOTTOM:
			ATLASSERT( nDirection == LAYOUT_DIRECTION_TOP || nDirection == LAYOUT_DIRECTION_BOTTOM );
			return rcCtrl.bottom + rcPadding.Height();
		default:
			ATLASSERT(FALSE);
			return 0;
	}
}



///////////////////////////////////////////////////////////////////////////////
// CLayoutRuleRatio

LONG CLayoutRuleRatio::Apply(const CDialogTemplate& tmpl, UINT nDirection,
							  const CRect& rcLayout) const
{
	return Apply( (HWND) NULL, nDirection, rcLayout );
}



LONG CLayoutRuleRatio::Apply(CWindow wndLayout, UINT nDirection,
							  const CRect& rcLayout) const
{
	if ( nDirection == LAYOUT_DIRECTION_LEFT || nDirection == LAYOUT_DIRECTION_RIGHT )
		return rcLayout.left + (LONG) ( m_fRatio * rcLayout.Width() );
	else
		return rcLayout.top + (LONG) ( m_fRatio * rcLayout.Height() );
}



///////////////////////////////////////////////////////////////////////////////
// CLayoutControl

CLayoutControl::CLayoutControl(const CDialogTemplate& rTmpl,
							   const CLayoutContainer& parent,
							   UINT nID, UINT nAnchor)
	: m_nID(nID), m_nAnchor(nAnchor)
{
	CDialogItemTemplate item = rTmpl.FindControl(nID);
	ATLASSERT( item.IsValid() );

	const CRect& rcParent = parent.GetBounds();
	
	m_rcMargins.SetRect( item.GetX() - rcParent.left,
		item.GetY() - rcParent.top,
		rcParent.right - item.GetX() - item.GetWidth(),
        rcParent.bottom - item.GetY() - item.GetHeight() );
	//ATLASSERT( m_rcMargins.left >= 0 && m_rcMargins.top >= 0 );
	rTmpl.MapDialogRect(m_rcMargins);
}



HWND CLayoutControl::CalcRect( CWindow wndLayout, LPCRECT prcLayout, LPRECT prc )
{
	ATLASSERT( wndLayout.IsWindow() );
	ATLASSERT( prcLayout != NULL );
	ATLASSERT( prc != NULL );

	// Retrieve the control's window handle
	CWindow wndControl = wndLayout.GetDlgItem( m_nID );
	ATLASSERT( wndControl.IsWindow() );

	// Get the control's current bounds
	CRect rcOld;
	wndControl.GetWindowRect(rcOld);
	wndLayout.ScreenToClient(rcOld);

	CopyRect( prc, rcOld );

	if ( m_nAnchor & LAYOUT_ANCHOR_LEFT )
	{
		prc->left = prcLayout->left + m_rcMargins.left;
		if ( !(m_nAnchor & LAYOUT_ANCHOR_RIGHT) )
			prc->right = prc->left + rcOld.Width();
	}
	if ( m_nAnchor & LAYOUT_ANCHOR_TOP )
	{
		prc->top = prcLayout->top + m_rcMargins.top;
		if ( !(m_nAnchor & LAYOUT_ANCHOR_BOTTOM) )
			prc->bottom = prc->top + rcOld.Height();
	}
	if ( m_nAnchor & LAYOUT_ANCHOR_RIGHT )
	{
		prc->right = prcLayout->right - m_rcMargins.right;
		if ( !(m_nAnchor & LAYOUT_ANCHOR_LEFT) )
			prc->left = prc->right - rcOld.Width();
	}
	if ( m_nAnchor & LAYOUT_ANCHOR_BOTTOM )
	{
		prc->bottom = prcLayout->bottom - m_rcMargins.bottom;
		if ( !(m_nAnchor & LAYOUT_ANCHOR_TOP) )
			prc->top = prc->bottom - rcOld.Height();
	}

	return (HWND) wndControl;
}



void CLayoutControl::DoLayout( CWindow wndLayout, HDWP hDwp, const CRect& rcLayout )
{
	CRect rcControl;
	CWindow wndControl = CalcRect( wndLayout, rcLayout, rcControl );

	StoreNewWindowRect( wndControl, hDwp, rcControl );

	wndControl.DeferWindowPos( hDwp, NULL, RECT_BREAK(rcControl),
		SWP_NOACTIVATE | SWP_NOZORDER /*| SWP_DRAWFRAME*/ );
}



///////////////////////////////////////////////////////////////////////////////
// CLayoutContainer

CLayoutContainer::CLayoutContainer(const CDialogTemplate& rTmpl,
								   const CLayoutContainer* pParent,
                                   CLayoutRule* pLeftRule, CLayoutRule* pTopRule,
								   CLayoutRule* pRightRule, CLayoutRule* pBottomRule )
	: m_rTmpl(rTmpl)
{
	if ( pParent == NULL )
	{
		m_rcBounds.SetRect( 0, 0, rTmpl.GetWidth(), rTmpl.GetHeight() );
	}
	else
	{
		ATLASSERT( pLeftRule != NULL );
		m_Rules.Add( CLayoutRulePtr(pLeftRule) );
		if ( pTopRule != NULL )
		{
			m_Rules.Add( CLayoutRulePtr(pTopRule) );
			ATLASSERT( pRightRule != NULL );
			m_Rules.Add( CLayoutRulePtr(pRightRule) );
			ATLASSERT( pBottomRule != NULL );
			m_Rules.Add( CLayoutRulePtr(pBottomRule) );
		}
		else
		{
			pTopRule = pRightRule = pBottomRule = pLeftRule;
		}

		const CRect& rcParent = pParent->GetBounds();

		m_rcBounds.SetRect(
			pLeftRule->Apply(m_rTmpl, LAYOUT_DIRECTION_LEFT, rcParent),
			pTopRule->Apply(m_rTmpl, LAYOUT_DIRECTION_TOP, rcParent),
			pRightRule->Apply(m_rTmpl, LAYOUT_DIRECTION_RIGHT, rcParent),
			pBottomRule->Apply(m_rTmpl, LAYOUT_DIRECTION_BOTTOM, rcParent) );
	}
}



void CLayoutContainer::AddLayoutNode(CLayoutNode* pNode)
{
	m_LayoutList.AddTail( CLayoutNodePtr(pNode) );
}



void CLayoutContainer::CalcRect( HWND hwndLayout, const CRect& rcLayout)
{
	ATLASSERT( ::IsWindow(hwndLayout) );

	if ( m_Rules.IsEmpty() )
	{
		// If this is the top level container (the dialog itself), it has no rules, and
		// can use the client rectangle
		m_rcBounds.CopyRect( rcLayout );
	}
	else
	{
		// apply the layout rules to get the bounds

		ATLASSERT( m_Rules.GetCount() == 1 || m_Rules.GetCount() == 4 );

		CLayoutRule *pLeftRule = m_Rules[0], *pTopRule, *pRightRule, *pBottomRule;

		if ( m_Rules.GetCount() == 1 )
			pTopRule = pRightRule = pBottomRule = pLeftRule;
		else
		{
			pTopRule = m_Rules[1];
			pRightRule = m_Rules[2];
			pBottomRule = m_Rules[3];
		}
		
		m_rcBounds.SetRect(
			pLeftRule->Apply( hwndLayout, LAYOUT_DIRECTION_LEFT, rcLayout ),
			pTopRule->Apply( hwndLayout, LAYOUT_DIRECTION_TOP, rcLayout ),
			pRightRule->Apply( hwndLayout, LAYOUT_DIRECTION_RIGHT, rcLayout ),
			pBottomRule->Apply( hwndLayout, LAYOUT_DIRECTION_BOTTOM, rcLayout ) );
	}
}



void CLayoutContainer::DoLayout( CWindow wndLayout, HDWP hDwp, const CRect& rcLayout )
{
	CalcRect(wndLayout, rcLayout);

	// iterate through all layout entries
	for ( POSITION pos = m_LayoutList.GetHeadPosition(); pos != NULL; )
	{
		CLayoutNode* pNode = m_LayoutList.GetNext(pos);
		pNode->DoLayout( wndLayout, hDwp, m_rcBounds );
	}
}
