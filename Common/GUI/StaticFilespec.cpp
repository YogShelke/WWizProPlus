/*
 *  StaticFilespec.cpp
 *
 *  CStaticFilespec implementation
 *    A simple class for displaying long filespecs in static text controls
 *
 *  Usage:          
 *  Edit history:   
 */

#include "stdafx.h"
#include "StaticFilespec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStaticFilespec

CStaticFilespec::CStaticFilespec
  (DWORD  dwFormat      /* = DT_LEFT | DT_NOPREFIX | DT_VCENTER */,
   BOOL   bPathEllipsis /* = FALSE */,
   DWORD dwScanType /*= 0x01*/)
{
	try
	{
		m_bPathEllipsis = bPathEllipsis;
		m_dwFormat = dwFormat;
		m_font = NULL;
		m_dwScanType = dwScanType;
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception caught in CStaticFilespec::CStaticFilespec"), 0, 0, true, SECONDLEVEL);
	}
}

CStaticFilespec::~CStaticFilespec()
{
	try
	{
		//destructor
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception caught in CStaticFilespec::~CStaticFilespec"), 0, 0, true, SECONDLEVEL);
	}
}

BEGIN_MESSAGE_MAP(CStaticFilespec, CWnd)
	//{{AFX_MSG_MAP(CStaticFilespec)
  ON_MESSAGE( WM_SETTEXT, OnSetText )
 	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStaticFilespec message handlers

BOOL CStaticFilespec::OnEraseBkgnd(CDC* pDC) 
{
	try
	{
		RECT  rectWnd;    // window rectangle

		// Erase background
		GetClientRect(&rectWnd);
		pDC->FillSolidRect(&rectWnd, m_clrBack);//::GetSysColor (COLOR_3DFACE));

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CStaticFilespec::OnEraseBkgnd",0,0,true,SECONDLEVEL);
	}
	return (TRUE);
}

void CStaticFilespec::OnPaint() 
{
	try
	{
		CPaintDC	dc(this);             // device context for painting
		DWORD       dwFormat;             // text format
		RECT		rectWnd;			  // window rectangle
		CString		strText;              // window text
		CWnd*       pWndParent = NULL;    // parent window

		// Set default font
		// pWndParent = GetParent();
		// if (pWndParent)
		dc.SelectObject(m_font);
		dc.SetTextColor(m_clrText);

		// Draw text
		GetWindowText(strText);
		GetClientRect(&rectWnd);
		if (m_dwScanType == ESCANDIALOG)
		{
			if (strText.GetLength() < 495)
			{
				m_dwFormat = DT_LEFT | DT_VCENTER;
			}
			else
			{
				m_dwFormat = DT_RIGHT | DT_VCENTER;
			}
		}
		if (m_dwScanType == EUSBDETECTDIALOG)
		{
			if (strText.GetLength() < 60)
			{
				m_dwFormat = DT_LEFT | DT_VCENTER;
			}
			else
			{
				m_dwFormat = DT_RIGHT | DT_VCENTER;
			}
		}
		dwFormat = m_dwFormat | (m_bPathEllipsis ? DT_PATH_ELLIPSIS : DT_END_ELLIPSIS);
		::DrawTextEx(dc.m_hDC,
			strText.GetBuffer(0),
			strText.GetLength(),
			&rectWnd,
			dwFormat,
			NULL);
		strText.ReleaseBuffer();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CStaticFilespec::OnPaint()", 0, 0, true, SECONDLEVEL);
	}
	// Do not call CWnd::OnPaint() for painting messages
}

LRESULT CStaticFilespec::OnSetText(WPARAM wParam,   LPARAM lParam)
{
	try
	{
		DefWindowProc(WM_SETTEXT, wParam, lParam);
		Invalidate();
		
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CStaticFilespec::OnSetText", 0, 0, true, SECONDLEVEL);
	}
	return (TRUE);
}
   
// End StaticFilespec.cpp


HBRUSH CStaticFilespec::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	try
	{
		HBRUSH hbr = CStaticFilespec::OnCtlColor(pDC, pWnd, nCtlColor);
		UNREFERENCED_PARAMETER(nCtlColor);
		pDC->SetTextColor (m_clrText);
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
		return hbr;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CStaticFilespec::OnCtlColor"), 0, 0, true, SECONDLEVEL);
	}
	return m_brBkgnd;
}


/***************************************************************************************************
*  Function Name  : SetTextColor
*  Description    : The SetTextColor function sets the text color for the specified device context to the specified color.
*  Author Name    :
*  SR_NO          : WRDWIZCOMMON_0114
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CStaticFilespec::SetTextColor(COLORREF clrText)
{
	try
	{
		m_clrText = clrText;
		Invalidate();
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception caught in CStaticFilespec::SetTextColor"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : SetBkColor
*  Description    : The SetBkColor function sets the current background color to the specified color value,
or to the nearest physical color if the device cannot represent the specified color value.
*  Author Name    :
*  SR_NO          : WRDWIZCOMMON_0115
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CStaticFilespec::SetBkColor(COLORREF clrBack)
{
	try
	{
		m_clrBack = clrBack;
		m_brBkgnd.DeleteObject();
		m_brBkgnd.CreateSolidBrush(clrBack);
		Invalidate();
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception caught in CStaticFilespec::SetBkColor"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : SetTransparent
*  Description    : Set transparency in static txt
*  Author Name    :
*  SR_NO          : WRDWIZCOMMON_0116
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CStaticFilespec::SetTransparent(bool bSetState)
{
	try
	{
		m_SetTransparent = bSetState;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CStaticFilespec::SetTransparent", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : setFont
*  Description    : Sends the WM_SETFONT message to the window to use the specified font.
*  Author Name    :
*  SR_NO          : WRDWIZCOMMON_0117
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CStaticFilespec::setFont(CFont* specFont)//(CString csFontName, int iFontHeight, int iFontWidth,
	//int iFontUnderline, int iFontWeight)
{
	try
	{
		if (specFont == NULL)
		{
			return;
		}

		if (m_font == NULL)
		{
			m_font = specFont;
			SetFont(m_font);
		}

		//LOGFONT lf;                        // Used to create the CFont.
		//SecureZeroMemory(&lf, sizeof(LOGFONT));   // Clear out structure.
		//lf.lfHeight = iFontHeight;
		//lf.lfWidth = iFontWidth;
		//lf.lfUnderline = static_cast<BYTE>(iFontUnderline);
		//lf.lfWeight = iFontWeight;
		//wcscpy_s(lf.lfFaceName, LF_FACESIZE, csFontName);    //    with face name "Arial".
		//m_font.CreateFontIndirect(&lf);    // Create the font.
		
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception caught in CStaticFilespec::setFont"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CreateFontStyle
*  Description    : Create font style of static text
*  Author Name    :
*  SR_NO          : WRDWIZCOMMON_0118
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CStaticFilespec::CreateFontStyle(CFont& mFont, CString csFontName, int iFontHeight,
	int iFontWidth, int iFontUnderline, int iFontWeight)
{
	try
	{
		if (mFont.m_hObject)
			mFont.DeleteObject();
		LOGFONT lf;                        // Used to create the CFont.
		SecureZeroMemory(&lf, sizeof(LOGFONT));   // Clear out structure.
		lf.lfHeight = iFontHeight;
		lf.lfWidth = iFontWidth;
		lf.lfUnderline = static_cast<BYTE>(iFontUnderline);
		lf.lfWeight = iFontWeight;
		wcscpy_s(lf.lfFaceName, LF_FACESIZE, csFontName);    //    with face name "Arial".
		mFont.CreateFontIndirect(&lf);
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception caught in CStaticFilespec::CreateFontStyle"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : setUnderline
*  Description    : Set underline on static text
*  Author Name    :
*  SR_NO          : WRDWIZCOMMON_0119
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CStaticFilespec::setUnderline(int iFontUnderline)
{
	try
	{
		CFont font;
		// Get window font
		CFont* pFont = GetFont();

		// Create LOGFONT structure
		LOGFONT lfLogFont;

		// Get LOGFONT structure of current font
		pFont->GetLogFont(&lfLogFont);

		// Set font to be bold
		lfLogFont.lfWeight = FW_NORMAL;

		// Create normal font that is bold (when not hovered)
		font.CreateFontIndirect(&lfLogFont);

		// Set underline attribute
		lfLogFont.lfUnderline = TRUE;

		// Create current font with underline attribute (when hovered)
		font.CreateFontIndirect(&lfLogFont);

		SetFont(&font);
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception caught in CStaticFilespec::setUnderline"), 0, 0, true, SECONDLEVEL);
	}
}

void CStaticFilespec::setTextFormat(DWORD dwFormat)
{
	try
	{
		m_dwFormat = dwFormat;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CStaticFilespec::setTextFormat", 0, 0, true, SECONDLEVEL);
	}
}