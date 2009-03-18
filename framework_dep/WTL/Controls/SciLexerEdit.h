#pragma once

template <class TBase>
class CSciLexerEditT : public TBase
{
public:
// Constructors
	CSciLexerEditT(HWND hWnd = NULL) : TBase(hWnd) {
	   m_bLinenumbers = FALSE;
	   m_bSelection = TRUE;
	   m_bFolding = FALSE;
	   m_nSearchflags = 0;
	}
	~CSciLexerEditT() {
	}

	CSciLexerEditT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName() { return _T("Scintilla"); }
	BOOL GetDisplayLinenumbers() { return m_bLinenumbers; }
	BOOL GetDisplaySelection() { return m_bSelection; }
	BOOL GetDisplayFolding() { return m_bFolding; }
	void SetSearchflags(int nSearchflags) { m_nSearchflags = nSearchflags; }

	void SetText (
		LPCSTR szText) //@parm pointer to new text
	{
		LRESULT lResult = 0;
		if (szText != NULL)
			lResult = SendMessage(SCI_SETTEXT,0,(LPARAM)szText);
		GotoPosition(0);
		SetFocus();
	}
	LPSTR GetText ()
	{
		long lLen = SendMessage(SCI_GETLENGTH, 0, 0) + 1;
		if (lLen > 0)
		{
			TCHAR *pReturn = new TCHAR[lLen];
			if (pReturn != NULL)
			{
				*pReturn = '\0';
				SendMessage(SCI_GETTEXT, lLen, (long)pReturn);
				return pReturn;
			}
		}
		return NULL;
	}
	int GetLength() {
		return SendMessage(SCI_GETLENGTH, 0, 0);
	}
	void AppendText(LPCSTR pText, int nLength)
	{
		SendMessage(SCI_APPENDTEXT, (WPARAM)nLength, (LPARAM)pText);
	}


	BOOL LoadFile (
		LPCSTR szPath) //@parm filename of to load
	{
		// if pathname is empty do nothing
		if (szPath == NULL || *szPath == '\0')
			return TRUE;
		// try to get extension and figure out what lexer to use
		CString strFile(szPath);
		int nIndex = strFile.ReverseFind('.');
		CString strExtension = strFile.Right(strFile.GetLength()-nIndex-1);
		SendMessage(SCI_SETLEXER, GetFormatFromExtension(strExtension), 0);

		BOOL bReturn = TRUE;
		// ty to open file in sharing mode
		std::ifstream file;
		long len = 0L;
		long nTotal;
		TCHAR *szBuffer = NULL;
		file.open(szPath, ios::in, filebuf::sh_read);
		// ok success - try to get length of file
		if (file.is_open())
		{
			len = _filelength(file.fd());
			if (len > 0)
			{
				//       alloc new buffer of sie = filesize+1 for termination NULL
				szBuffer = new TCHAR[len+1];
				if (szBuffer != NULL)
				{
					file.read(szBuffer, len);
					nTotal = file.gcount();
					if (nTotal > 0 && nTotal <= len)
						szBuffer[nTotal] = '\0';
					//          read error
					if (file.bad())
					{
						file.close();
						bReturn = FALSE;
					}
				}
				else
				{
					file.close();
					bReturn = FALSE;
				}
			}
			file.close();
			//    set text to control
			SetText (szBuffer);
			//    tell scintilla that we have an unmodified document
			SendMessage(SCI_SETSAVEPOINT,0 , 0);
			GotoPosition(0);

		}
		// file open error - return
		else
			bReturn = FALSE;
		// clean up
		if (szBuffer != NULL)
			delete [] szBuffer;
		return bReturn;
	}
	BOOL SaveFile (
		LPCSTR szPath) //@parm filename to save to
	{
		::ofstream file;
		file.open(szPath);
		if (file.fail())
		{
			return FALSE;
		}
		int buflen = SendMessage(SCI_GETLENGTH)+1; //last NULL
		TCHAR *pBuffer = new TCHAR[buflen];
		if (pBuffer != NULL)
		{
			SendMessage(SCI_GETTEXT, buflen,(long)pBuffer);
			file.write(pBuffer, buflen-1);
			delete [] pBuffer;
		}
		file << ends;
		file.close();
		return TRUE;
	}
	void SetSavePoint()
	{
		SendMessage(SCI_SETSAVEPOINT, 0 , 0);
	}
	BOOL GetModify()
	{
		return SendMessage(SCI_GETMODIFY, 0, 0);
	}
	/////////////////////////////////////
	// @mfunc Try to find format for lexer by looking at the file extension.<nl>
	// See global arrays at top of file.
	// @rvalue int | Scintilla integer format for lexer
	//
	int GetFormatFromExtension (
		LPCSTR szExtension) //@parm filename extension without dot e.g. "cpp"
	{
		int i = 0;
		TCHAR *pExtension = szExtensions[i];
		while (pExtension != NULL)
		{
			CTokenizer st(szExtensions[i], _T("|"));
			while (st.HasMoreTokens())
			{
				if (st.Next().CompareNoCase(szExtension) == 0)
					return nFormats[i];
			}
			i++;
			pExtension = szExtensions[i];
		}
		return SCLEX_NULL;
	}
	int GetLinenumberChars ()
	{
		// try to get number of lines in control
		LRESULT lLines = SendMessage(SCI_GETLINECOUNT, 0, 0);

		int nChars = 1;
		while (lLines > 0)
		{
			lLines = lLines / 10;
			nChars++;
		}
		return nChars; // allow an extra char for safety
	}
	int GetLinenumberWidth ()
	{
		// get number of chars needed to display highest linenumber
		int nChars = GetLinenumberChars ()+1;
		// get width of character '9' in pixels
		LRESULT lWidth = SendMessage(SCI_TEXTWIDTH, STYLE_LINENUMBER, (long)_T("9"));
		return nChars * lWidth;
	}
	void SetDisplayLinenumbers(
		BOOL bFlag) //@parm flag if we shuld display line numbers
	{
		// if nothing changes just return
		if (GetDisplayLinenumbers() == bFlag)
			return;
		// if display is turned off we set margin 0 to 0
		if (!bFlag)
		{
			SendMessage(SCI_SETMARGINWIDTHN, 0, 0);
		}
		// if display is turned o we set margin 0 to the calculated width
		else
		{
			int nWidth = GetLinenumberWidth() + 4;
			SendMessage(SCI_SETMARGINWIDTHN, 0, nWidth);
		}
		m_bLinenumbers = bFlag;
	}
	void Cut()
	{
		SendMessage(SCI_CUT, 0, 0);
	}
	void Copy()
	{
		SendMessage(SCI_COPY, 0, 0);
	}
	void Paste()
	{
		SendMessage(SCI_PASTE, 0, 0);
	}
	void Clear()
	{
		SendMessage(SCI_CLEAR, 0, 0);
	}
	void SelectAll()
	{
		SendMessage(SCI_SELECTALL, 0, 0);
	}
	void Undo()
	{
		SendMessage(SCI_UNDO, 0, 0);
	}
	void Redo()
	{
		SendMessage(SCI_REDO, 0, 0);
	}
	BOOL CanUndo()
	{
		return SendMessage(SCI_CANUNDO, 0, 0) != 0;
	}
	BOOL CanRedo()
	{
		return SendMessage(SCI_CANREDO, 0, 0) != 0;
	}
	BOOL CanPaste()
	{
		return SendMessage(SCI_CANPASTE, 0, 0) != 0;
	}
	void SetReadOnly(BOOL bReadOnly)
	{
		SendMessage(SCI_SETREADONLY, (WPARAM)bReadOnly, 0);
	}
	BOOL GetReadOnly()
	{
		return (BOOL)SendMessage(SCI_GETREADONLY, 0, 0);
	}
	long GetLineCount()
	{
		return SendMessage(SCI_GETLINECOUNT, 0, 0);
	}
	long GetCurrentLine()
	{
		long lPos = SendMessage(SCI_GETCURRENTPOS, 0, 0);
		return SendMessage(SCI_LINEFROMPOSITION, lPos, 0) + 1;
	}
	long GetCurrentColumn()
	{
		long lPos = SendMessage(SCI_GETCURRENTPOS, 0, 0);
		return SendMessage(SCI_GETCOLUMN, lPos, 0) + 1;
	}
	void SetCurrentPosition(int nPos)
	{
		SendMessage(SCI_SETCURRENTPOS, (WPARAM)nPos, 0);
	}
	long GetCurrentPosition()
	{
		return SendMessage(SCI_GETCURRENTPOS, 0, 0);
	}
	int GetCurrentStyle()
	{
		long lPos = SendMessage(SCI_GETCURRENTPOS, 0, 0);
		return SendMessage(SCI_GETSTYLEAT, lPos, 0);
	}
	int GetCurrentFoldinglevel()
	{
		long lLine = GetCurrentLine();
		int level = (SendMessage(SCI_GETFOLDLEVEL, lLine, 0)) & SC_FOLDLEVELNUMBERMASK;
		return level-1024;
	}
	void SetFontname(int nStyle, LPCSTR szFontname)
	{
		SendMessage(SCI_STYLESETFONT, nStyle, (long)szFontname);
	}
	void SetFontheight(int nStyle, int nHeight)
	{
		SendMessage(SCI_STYLESETSIZE, nStyle, (long)nHeight);
	}
	void SetForeground(int nStyle, COLORREF crForeground)
	{
		SendMessage(SCI_STYLESETFORE, nStyle, (long)crForeground);
	}
	void SetBackground(int nStyle, COLORREF crBackground)
	{
		SendMessage(SCI_STYLESETBACK, nStyle, (long)crBackground);
	}
	void SetBold(int nStyle, BOOL bBold)
	{
		SendMessage(SCI_STYLESETBOLD, nStyle, (long)bBold);
	}
	void SetItalic(int nStyle, BOOL bItalic)
	{
		SendMessage(SCI_STYLESETITALIC, nStyle, (long)bItalic);
	}
	void SetUnderline(int nStyle, BOOL bUnderline)
	{
		SendMessage(SCI_STYLESETUNDERLINE, nStyle, (long)bUnderline);
	}
	BOOL GetOverstrike()
	{
		return SendMessage(SCI_GETOVERTYPE , 0, 0);
	}
	void SetOverstrike(BOOL bOverstrike)
	{
		SendMessage(SCI_SETOVERTYPE, bOverstrike, 0);
	}
	void SetDisplaySelection(BOOL bFlag)
	{
		m_bSelection = bFlag;
		if (bFlag)
			SendMessage(SCI_SETMARGINWIDTHN, 1, 16);
		else
			SendMessage(SCI_SETMARGINWIDTHN, 1, 0);
	}
	void SetDisplayFolding(BOOL bFlag)
	{
		m_bFolding = bFlag;
		if (bFlag)
			SendMessage(SCI_SETMARGINWIDTHN, 2, 16);
		else
			SendMessage(SCI_SETMARGINWIDTHN, 2, 0);
	}
	void SetWrapMode(int nMode)
	{
		SendMessage(SCI_SETWRAPMODE, (WPARAM)nMode, 0);
	}
	int GetWrapMode()
	{
		return SendMessage(SCI_GETWRAPMODE, 0, 0);
	}
	void LineScroll(int nLine, int nChars)
	{
		SendMessage(SCI_LINESCROLL, (WPARAM)nChars, (LPARAM)nLine);
	}
	int LinesOnScreen()
	{
		return (int)SendMessage(SCI_LINESONSCREEN, 0, 0);
	}
	int GetFirstVisableLine()
	{
		return SendMessage(SCI_GETFIRSTVISIBLELINE, 0, 0);
	}
	void Init()
	{
		// clear all text styles
		SendMessage(SCI_CLEARDOCUMENTSTYLE, 0, 0);
		// set the number of styling bits to 7 - the asp/html views need a lot of styling - default is 5
		// If you leave the default you will see twiggle lines instead of ASP code
		SendMessage(SCI_SETSTYLEBITS, 7, 0);
		// set the display for indetation guides to on - this displays virtical dotted lines from the beginning of 
		// a code block to the end of the block
		SendMessage(SCI_SETINDENTATIONGUIDES, TRUE, 0);
		// set tabwidth to 3
		SendMessage(SCI_SETTABWIDTH,3,0);
		// set indention to 3
		SendMessage(SCI_SETINDENT,3,0);
		// set the caret blinking time to 400 milliseconds
		SendMessage(SCI_SETCARETPERIOD,400,0);
		// source folding section
		// tell the lexer that we want folding information - the lexer supplies "folding levels"
		SendMessage(SCI_SETPROPERTY, (WPARAM)_T("fold"), (LPARAM)_T("1"));
		SendMessage(SCI_SETPROPERTY, (WPARAM)_T("fold.html"), (LPARAM)_T("1"));
		SendMessage(SCI_SETPROPERTY, (WPARAM)_T("fold.html.preprocessor"), (LPARAM)_T("1"));
		SendMessage(SCI_SETPROPERTY, (WPARAM)_T("fold.comment"), (LPARAM)_T("1"));
		SendMessage(SCI_SETPROPERTY, (WPARAM)_T("fold.at.else"), (LPARAM)_T("1"));
		SendMessage(SCI_SETPROPERTY, (WPARAM)_T("fold.flags"), (LPARAM)_T("1"));
		SendMessage(SCI_SETPROPERTY, (WPARAM)_T("fold.preprocessor"), (LPARAM)_T("1"));
		SendMessage(SCI_SETPROPERTY, (WPARAM)_T("styling.within.preprocessor"), (LPARAM)_T("1"));
		SendMessage(SCI_SETPROPERTY, (WPARAM)_T("asp.default.language"), (LPARAM)_T("1"));
		// Tell scintilla to draw folding lines UNDER the folded line
		SendMessage(SCI_SETFOLDFLAGS, 16,0);
		// Set margin 2 = folding margin to display folding symbols
		SendMessage(SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
		// allow notifications for folding actions
		SendMessage(SCI_SETMODEVENTMASK, SC_MOD_INSERTTEXT|SC_MOD_DELETETEXT, 0);
		//   SendMessage(SCI_SETMODEVENTMASK, SC_MOD_CHANGEFOLD|SC_MOD_INSERTTEXT|SC_MOD_DELETETEXT, 0);
		// make the folding margin sensitive to folding events = if you click into the margin you get a notification event
		SendMessage(SCI_SETMARGINSENSITIVEN, 2, TRUE);
		// define a set of markers to displa folding symbols
		SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS);
		SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PLUS);
		SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY);
		SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY);
		SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY);
		SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY);
		SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY);
		// set the forground color for some styles
		SendMessage(SCI_STYLESETFORE, 0, RGB(0,0,0));
		SendMessage(SCI_STYLESETFORE, 2, RGB(0,64,0));
		SendMessage(SCI_STYLESETFORE, 5, RGB(0,0,255));
		SendMessage(SCI_STYLESETFORE, 6, RGB(200,20,0));
		SendMessage(SCI_STYLESETFORE, 9, RGB(0,0,255));
		SendMessage(SCI_STYLESETFORE, 10, RGB(255,0,64));
		SendMessage(SCI_STYLESETFORE, 11, RGB(0,0,0));
		// set the backgroundcolor of brace highlights
		SendMessage(SCI_STYLESETBACK, STYLE_BRACELIGHT, RGB(0,255,0));
		// set end of line mode to CRLF
		SendMessage(SCI_CONVERTEOLS, 2, 0);
		SendMessage(SCI_SETEOLMODE, 2, 0);
		//   SendMessage(SCI_SETVIEWEOL, TRUE, 0);
		// set markersymbol for marker type 0 - bookmark
		SendMessage(SCI_MARKERDEFINE, 0, SC_MARK_CIRCLE);
		// display all margins
		SetDisplayLinenumbers(TRUE);
		SetDisplayFolding(TRUE);
		SetDisplaySelection(TRUE);

	}
	void UpdateUI()
	{
		// do brace matching
		long lStart = SendMessage(SCI_GETCURRENTPOS, 0, 0);
		long lEnd = SendMessage(SCI_BRACEMATCH, lStart-1, 0);
		// if there is a matching brace highlight it
		if (lEnd >= 0)
			SendMessage(SCI_BRACEHIGHLIGHT, lStart-1, lEnd);
		// if there is NO matching brace erase old highlight
		else
			SendMessage(SCI_BRACEHIGHLIGHT, -1, -1);
	}
	void DoDefaultFolding(
		int nMargin,      //@parm maring we want to handle - normally (2)
		long lPosition)   //@parm character position where user clicked margin
	{
		// simply toggle fold
		if (nMargin == 2)
		{
			long lLine = SendMessage(SCI_LINEFROMPOSITION, lPosition, 0);
			SendMessage(SCI_TOGGLEFOLD, lLine, 0);
		}
	}
	void Refresh()
	{
		SendMessage(SCI_COLOURISE, 0, -1);
	}
	void AddBookmark(
		long lLine) //@parm line where to add bookmark - lines start at 1
	{
		SendMessage(SCI_MARKERADD, lLine-1, 0);
	}
	void DeleteBookmark(
		long lLine) //@parm line where to delete bookmark - lines start at 1
	{
		if (HasBookmark(lLine))
		{
			SendMessage(SCI_MARKERDELETE, lLine-1, 0);
		}
	}
	BOOL HasBookmark(
		long lLine) //@parm line where to add bookmark - lines start at 1
	{
		int n = SendMessage(SCI_MARKERGET, lLine-1, 0);
		// check mask for markerbit 0
		if (n & 0x1)
			return TRUE;
		return FALSE;
	}
	void FindNextBookmark()
	{
		long lLine = SendMessage(SCI_MARKERNEXT, GetCurrentLine(), 1);
		if (lLine >= 0)
			SendMessage(SCI_GOTOLINE, lLine,0);
	}
	void FindPreviousBookmark()
	{
		long lLine = SendMessage(SCI_MARKERPREVIOUS, GetCurrentLine()-2, 1);
		if (lLine >= 0)
			SendMessage(SCI_GOTOLINE, lLine,0);
	}
	void GotoPosition(
		long lPos) //@parm new character position
	{
		SendMessage(SCI_GOTOPOS, lPos, 0);
	}
	void GotoLine(
		long lLine) //@parm new line - lines start at 1
	{
		SendMessage(SCI_GOTOLINE, lLine-1, 0);
	}
	BOOL SearchForward(
		LPSTR szText) //@parm text to search
	{
		if (szText == NULL)
			return FALSE;
		long lPos = GetCurrentPosition();
		TextToFind tf;
		tf.lpstrText = szText;
		tf.chrg.cpMin = lPos+1;
		tf.chrg.cpMax = SendMessage(SCI_GETLENGTH, 0, 0);
		lPos = SendMessage(SCI_FINDTEXT, m_nSearchflags, (long)&tf);
		if (lPos >= 0)
		{
			SetFocus();
			GotoPosition(lPos);
			SendMessage(SCI_SETSEL, tf.chrgText.cpMin, tf.chrgText.cpMax);
			SendMessage(SCI_FINDTEXT, m_nSearchflags, (long)&tf);
			return TRUE;
		}
		return FALSE;
	}
	BOOL SearchBackward(
		LPSTR szText) //@parm text to search
	{
		if (szText == NULL)
			return FALSE;
		long lPos = GetCurrentPosition();
		long lMinSel = GetSelectionStart();
		TextToFind tf;
		tf.lpstrText = szText;
		if (lMinSel >= 0)
			tf.chrg.cpMin = lMinSel-1;
		else
			tf.chrg.cpMin = lPos-1;
		tf.chrg.cpMax = 0;
		lPos = SendMessage(SCI_FINDTEXT, m_nSearchflags, (long)&tf);
		if (lPos >= 0)
		{
			SetFocus();
			GotoPosition(lPos);
			SendMessage(SCI_SETSEL, tf.chrgText.cpMin, tf.chrgText.cpMax);
			SendMessage(SCI_FINDTEXT, m_nSearchflags, (long)&tf);
			return TRUE;
		}
		return FALSE;
	}
	void ReplaceSearchedText(
		LPCSTR szText) //@parm new text
	{
		if (szText == NULL)
			return;
		SendMessage(SCI_TARGETFROMSELECTION, 0, 0);
		if (m_nSearchflags & SCFIND_REGEXP)
			SendMessage(SCI_REPLACETARGETRE, strlen(szText), (long)szText);
		else
			SendMessage(SCI_REPLACETARGET, strlen(szText), (long)szText);
	}
	void SetLexer(
		int nLexer) //@parm lexer code e.g. 
	{
		SendMessage(SCI_SETLEXER, nLexer, 0);
	}
	void SetKeyWords(
		int nLexer,
		LPCSTR pKeyWords) //@parm lexer code e.g. 
	{
		SendMessage(SCI_SETKEYWORDS, nLexer, (LPARAM)pKeyWords);
	}
	long GetSelectionStart()
	{
		return SendMessage(SCI_GETSELECTIONSTART, 0, 0);
	}
	long GetSelectionEnd()
	{
		return SendMessage(SCI_GETSELECTIONEND, 0, 0);
	}
	long GetSelectionLength()
	{
		return (GetSelectionEnd() - GetSelectionStart()) + 1;
	}
	BOOL GetSelectedText(LPTSTR p, long lLength)
	{
		long lLen = GetSelectionLength();
		if (lLen <= 0 || lLen<lLength)
			return FALSE;

		*p = '\0';
		SendMessage(SCI_GETSELTEXT, 0, (long)p);
		return TRUE;
	}
	int ReplaceAll(
		LPCSTR szFind, 
		LPCSTR szReplace, 
		BOOL bUseSelection)
	{
		int nCount = 0;
		// different branches for replace in selection or total file
		if (bUseSelection)
		{
			//    get starting selection range
			long lLen = 0;
			long lStart = GetSelectionStart();
			long lEnd = GetSelectionEnd();
			//    set target to selection
			SendMessage(SCI_SETTARGETSTART, lStart);
			SendMessage(SCI_SETTARGETEND, lEnd);
			//    try to find text in target for the first time
			long lPos = SendMessage(SCI_SEARCHINTARGET, strlen(szFind), (long)szFind);
			//    loop over selection until end of selection reached - moving the target start each time
			while (lPos < lEnd && lPos >= 0)
			{
				if (m_nSearchflags & SCFIND_REGEXP) // check for regular expression flag
					lLen = SendMessage(SCI_REPLACETARGETRE, strlen(szReplace), (long)szReplace);
				else
					lLen = SendMessage(SCI_REPLACETARGET, strlen(szReplace), (long)szReplace);
				//       the end of the selection was changed - recalc the end
				lEnd = GetSelectionEnd();
				//       move start of target behind last change and end of target to new end of selection
				SendMessage(SCI_SETTARGETSTART, lPos+lLen);
				SendMessage(SCI_SETTARGETEND, lEnd);
				//       find again - if nothing found loop exits
				lPos = SendMessage(SCI_SEARCHINTARGET, strlen(szFind), (long)szFind);
				nCount++;
			}
		}
		else
		{
			//    start with first and last char in buffer
			long lLen = 0;
			long lStart = 0;
			long lEnd = SendMessage(SCI_GETTEXTLENGTH, 0, 0);
			//    set target to selection
			SendMessage(SCI_SETTARGETSTART, lStart, 0);
			SendMessage(SCI_SETTARGETEND, lEnd, 0);
			//    try to find text in target for the first time
			long lPos = SendMessage(SCI_SEARCHINTARGET, strlen(szFind), (long)szFind);
			//    loop over selection until end of selection reached - moving the target start each time
			while (lPos < lEnd && lPos >= 0)
			{
				if (m_nSearchflags & SCFIND_REGEXP) // check for regular expression flag
					lLen = SendMessage(SCI_REPLACETARGETRE, strlen(szReplace), (long)szReplace);
				else
					lLen = SendMessage(SCI_REPLACETARGET, strlen(szReplace), (long)szReplace);
				//       the end of the selection was changed - recalc the end
				lEnd = SendMessage(SCI_GETTEXTLENGTH, 0, 0);;
				//       move start of target behind last change and end of target to new end of buffer
				SendMessage(SCI_SETTARGETSTART, lPos+lLen);
				SendMessage(SCI_SETTARGETEND, lEnd);
				//       find again - if nothing found loop exits
				lPos = SendMessage(SCI_SEARCHINTARGET, strlen(szFind), (long)szFind);
				nCount++;
			}
		}
		return nCount;
	}

protected:
   BOOL m_bLinenumbers;
   BOOL m_bSelection;
   BOOL m_bFolding;
   int  m_nSearchflags;

};

typedef CSciLexerEditT<ATL::CWindow>   CSciLexerEdit;
