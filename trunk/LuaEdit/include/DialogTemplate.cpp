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

#include "DialogTemplate.h"



HINSTANCE CDialogTemplate::s_hResourceInstance = GetModuleHandle(NULL);



const WORD* CDialogTemplate::GetDataPtr() const
{
	ATLASSERT( m_pTmpl != NULL );
	if ( m_bIsTmplEx )
		return (const WORD*) (m_pTmplEx + 1);
	else
        return (const WORD*) (m_pTmpl + 1);
}



void CDialogTemplate::SkipData(const WORD*& pw)
{
	ATLASSERT( pw != NULL );
	if ( *pw == 0x0000 )
		++pw;
	else if ( *pw == 0xFFFF )
		pw += 2;
	else
		while (*pw++);
}



void CDialogTemplate::SkipTitle(const WORD*& pw)
{
	// This is always a null-terminated string.
	while (*pw++);
}



void CDialogTemplate::SkipFontInfo(const WORD*& pw) const
{
	if (GetStyle() & DS_SETFONT)
	{
		pw += ( m_bIsTmplEx ? 3 : 1);
		while (*pw++);
	}
}



LPCWSTR CDialogTemplate::GetClassName() const
{
	const WORD* pw = GetDataPtr();
	SkipData(pw);
    if ( *pw == 0x0000 )
		return NULL;
	else if ( *pw == 0xFFFF )
		return (LPCWSTR) MAKEINTRESOURCEW( pw[1] );
	else
		return (LPCWSTR) pw;
}



LPCWSTR CDialogTemplate::GetMenuResource() const
{
	const WORD* pw = GetDataPtr();
	if ( *pw == 0x0000 )
		return NULL;
	else if ( *pw == 0xFFFF )
		return (LPCWSTR) MAKEINTRESOURCEW( pw[1] );
	else
		return (LPCWSTR) pw;
}




LPCWSTR CDialogTemplate::GetTitle() const
{
	const WORD* pw = GetDataPtr();
	SkipData(pw);
	SkipData(pw);
	if ( *pw == 0x0000 )
		return NULL;
	else
		return (LPCWSTR) pw;
}



CDialogItemTemplate CDialogTemplate::GetFirstDlgItem() const
{
	ATLASSERT( m_pTmpl != NULL );
	ATLASSERT( GetItemCount() > 0 );

	const WORD* pw = GetDataPtr();
	SkipData(pw); SkipData(pw); SkipTitle(pw); SkipFontInfo(pw);
	
	const DLGITEMTEMPLATE* pItem = (const DLGITEMTEMPLATE*) AlignToDWORD(pw);
	return CDialogItemTemplate( *this, pItem, m_bIsTmplEx );
}



CDialogItemTemplate CDialogTemplate::FindControl(UINT nCtrlID) const
{
	ATLASSERT( m_pTmpl != NULL );

	CDialogItemTemplate item = GetFirstDlgItem();
	for ( int i = 0; i < GetItemCount(); ++i )
	{
		if ( item.GetID() == nCtrlID )
			return item;
		if ( i < GetItemCount() - 1 )
			item = item.GetNextItem();
	}
	return CDialogItemTemplate(*this, NULL, m_bIsTmplEx);
}



HFONT CreatePointFontIndirectW(LOGFONTW logFont, HDC hDC = NULL)
{
	HDC hDC1 = (hDC != NULL) ? hDC : ::GetDC(NULL);

	POINT pt = { 0, 0 };
	pt.y = MulDiv( logFont.lfHeight, ::GetDeviceCaps(hDC1, LOGPIXELSY), 72 );
	::DPtoLP(hDC1, &pt, 1);
	POINT ptOrg = { 0, 0 };
	::DPtoLP(hDC1, &ptOrg, 1);
	logFont.lfHeight = -abs(pt.y - ptOrg.y);

	if(hDC == NULL)
		::ReleaseDC(NULL, hDC1);

	return CreateFontIndirectW(&logFont);
}



void CDialogTemplate::LoadFromResource(UINT nResourceID)
{
	HRSRC hRsrc = FindResource( s_hResourceInstance, MAKEINTRESOURCE(nResourceID),
		RT_DIALOG );
	ATLASSERT( hRsrc != NULL );

	HGLOBAL hGlobal = LoadResource( s_hResourceInstance, hRsrc );
	ATLASSERT( hGlobal != NULL );

	m_pTmpl = (DLGTEMPLATE*) LockResource(hGlobal);
	ATLASSERT( m_pTmpl != NULL );

	m_bIsTmplEx = (m_pTmplEx->signature == 0xFFFF );
}




BOOL CDialogTemplate::GetFontInfo(LOGFONTW& lf) const
{
	ZeroMemory(&lf, sizeof(LOGFONT));
	if ( GetStyle() & DS_SETFONT )
	{
		const WORD* pw = GetDataPtr();
		SkipData(pw); SkipData(pw); SkipTitle(pw);

		lf.lfHeight = *pw++;
		if ( m_bIsTmplEx )
		{
			lf.lfWeight = *pw++;
			lf.lfItalic = (BYTE) *pw++;
		}
		wcsncpy( lf.lfFaceName, (LPCWSTR) pw, 32 );
		return TRUE;
	}
	else
		return FALSE;
}


BOOL CDialogTemplate::MapDialogRect(LPRECT pRect) const
{
	ATLASSERT( m_pTmpl != NULL );

	if ( m_cxyBaseUnits.cx == 0 && m_cxyBaseUnits.cy == 0 )
	{
        if ( GetStyle() & DS_SETFONT )
		{
			HDC hdcScreen = ::GetDC(NULL);
			ATLASSERT( hdcScreen != NULL );

			LOGFONTW lf; GetFontInfo(lf);
			HFONT hFont = CreatePointFontIndirectW(lf, hdcScreen);
			
			SelectObject( hdcScreen, hFont );

			TEXTMETRIC tm;
			GetTextMetrics( hdcScreen, &tm );

			SIZE size;
			GetTextExtentPoint32( hdcScreen, _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"),
				52, &size );
			m_cxyBaseUnits.cx = (size.cx / 26 + 1) / 2;
			m_cxyBaseUnits.cy = tm.tmHeight;

			ReleaseDC( NULL, hdcScreen );
			DeleteObject(hFont);
		}
		else
			m_cxyBaseUnits = CSize( GetDialogBaseUnits() );
	}

	pRect->left = MulDiv( pRect->left, m_cxyBaseUnits.cx, 4 );
	pRect->top = MulDiv( pRect->top, m_cxyBaseUnits.cy, 8 );
	pRect->right = MulDiv( pRect->right, m_cxyBaseUnits.cx, 4 );
	pRect->bottom = MulDiv( pRect->bottom, m_cxyBaseUnits.cy, 8 );
	return TRUE;
}
