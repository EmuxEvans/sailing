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

#include "DialogItemTemplate.h"



LPCWSTR CDialogItemTemplate::GetClassName() const
{
	const WORD* pw = GetDataPtr();
	if ( *pw == 0xFFFF )
		return (LPCWSTR) MAKEINTATOM(pw[1]);
	else
		return (LPCWSTR) pw;
}



void CDialogItemTemplate::SkipData(const WORD*& pw)
{
	ATLASSERT( pw != NULL );
	if ( *pw == 0xFFFF )
		pw += 2;
	else
		while (*pw++);
}



void CDialogItemTemplate::SkipExtraData(const WORD*& pw, bool bDialogEx)
{
	ATLASSERT( pw != NULL );
	// The extra data is prefixed with its length
	WORD cbExtra = *pw++;

	/* The 1-WORD size field is included in the length with the DLGITEMTEMPLATE
	 * format, but not with DLGITEMTEMPLATEEX. */
	if (cbExtra != 0 && !bDialogEx)
		cbExtra -= 2;

	pw += cbExtra / 2;
}




LPCWSTR CDialogItemTemplate::GetTitle() const
{
	// Get a pointer to the data
	const WORD* pw = GetDataPtr();
	// Skip the window class string
	SkipData(pw);

	// This is either a null-terminated string or a resource identifer.
	// (See documentation of DLGITEMTEMPLATE)
	if ( *pw == 0xFFFF )
		return (LPCWSTR) MAKEINTRESOURCEW(pw[1]);
	else
		return (LPCWSTR) pw;		
}



CDialogItemTemplate CDialogItemTemplate::GetNextItem() const
{
	// Get a pointer to the data
	const WORD* pw = GetDataPtr();
	// Skip all three data fields
	SkipData(pw); SkipData(pw); SkipExtraData(pw, m_bIsItemEx);

	// The next item template starts on the next DWORD boundary.
	const DLGITEMTEMPLATE* pItem = (const DLGITEMTEMPLATE*) AlignToDWORD(pw);
	return CDialogItemTemplate( m_rParent, pItem, m_bIsItemEx );
}
