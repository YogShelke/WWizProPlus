/***************************************************************************                      
   Program Name          : ColorStatic.cpp
   Description           : Customize color and text font of static control in dialog
   Author Name           :                                                                         
   Date Of Creation      : 18th Oct,2013
   Version No            : 1.0.0.1
   Special Logic Used    : 
   Modification Log      :           
****************************************************************************/
#include "stdafx.h"
#include "ColorStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************************************                    
*  Function Name  : CColorStatic                                                     
*  Description    : C'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0111
*  Date           : 18 Oct 2013
****************************************************************************************************/
CColorStatic::CColorStatic()
{
	try
	{
		m_SetTransparent = false;
		m_clrText = RGB (0, 0, 0);
		m_clrBack = ::GetSysColor (COLOR_3DFACE);
		m_brBkgnd.CreateSolidBrush (m_clrBack);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CColorStatic::CColorStatic"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : ~CColorStatic                                                     
*  Description    : D'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0112
*  Date           : 18 Oct 2013
****************************************************************************************************/
CColorStatic::~CColorStatic()
{
	try
	{
		m_brBkgnd.DeleteObject();
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CColorStatic::~CColorStatic"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0113
*  Date           : 18 Oct 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CColorStatic, CStatic)
	//{{AFX_MSG_MAP(CColorStatic)
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
	//ON_WM_PAINT()
END_MESSAGE_MAP()


/***************************************************************************************************                    
*  Function Name  : SetTextColor                                                     
*  Description    : The SetTextColor function sets the text color for the specified device context to the specified color.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0114
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CColorStatic::SetTextColor (COLORREF clrText)
{
	try
	{
		m_clrText = clrText;
		Invalidate ();
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CColorStatic::SetTextColor"), 0, 0, true, SECONDLEVEL);
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
void CColorStatic::SetBkColor (COLORREF clrBack)
{
	try
	{
		m_clrBack = clrBack;
		m_brBkgnd.DeleteObject ();
		m_brBkgnd.CreateSolidBrush (clrBack);
		Invalidate ();
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CColorStatic::SetBkColor"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : SetTransparent                                                     
*  Description    : Set transparency in static txt
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0116
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CColorStatic::SetTransparent(bool bSetState)
{
	m_SetTransparent = bSetState;
}

/***************************************************************************************************                    
*  Function Name  : setFont                                                     
*  Description    : Sends the WM_SETFONT message to the window to use the specified font.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0117
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CColorStatic::setFont(CString csFontName, int iFontHeight, int iFontWidth,
						   int iFontUnderline, int iFontWeight)
{
	try
	{
		CFont m_font;
		LOGFONT lf;                        // Used to create the CFont.
		SecureZeroMemory(&lf, sizeof(LOGFONT));   // Clear out structure.
		lf.lfHeight = iFontHeight;
		lf.lfWidth  = iFontWidth;
		lf.lfUnderline = static_cast<BYTE>(iFontUnderline);
		lf.lfWeight    = iFontWeight;
		wcscpy_s(lf.lfFaceName, LF_FACESIZE, csFontName);    //    with face name "Arial".
		m_font.CreateFontIndirect(&lf);    // Create the font.
		SetFont(&m_font);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CColorStatic::setFont"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : CreateFontStyle                                                     
*  Description    : Create font style of static text
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0118
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CColorStatic::CreateFontStyle(CFont& m_Font, CString csFontName, int iFontHeight, 
								   int iFontWidth, int iFontUnderline, int iFontWeight)
{
	try
	{
		if(m_Font.m_hObject)
			m_Font.DeleteObject();
		LOGFONT lf;                        // Used to create the CFont.
		SecureZeroMemory(&lf, sizeof(LOGFONT));   // Clear out structure.
		lf.lfHeight = iFontHeight;
		lf.lfWidth  = iFontWidth;
		lf.lfUnderline =  static_cast<BYTE>(iFontUnderline);
		lf.lfWeight    = iFontWeight;
		wcscpy_s(lf.lfFaceName, LF_FACESIZE, csFontName);    //    with face name "Arial".
		m_Font.CreateFontIndirect(&lf);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CColorStatic::setFont"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : setUnderline                                                     
*  Description    : Set underline on static text
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0119
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CColorStatic::setUnderline( int iFontUnderline)
{
	try
	{
		CFont m_font;
		// Get window font
		CFont* pFont = GetFont();

		// Create LOGFONT structure
		LOGFONT lfLogFont;

		// Get LOGFONT structure of current font
		pFont->GetLogFont(&lfLogFont);

		// Set font to be bold
		lfLogFont.lfWeight = FW_NORMAL;

		// Create normal font that is bold (when not hovered)
		m_font.CreateFontIndirect(&lfLogFont);

		// Set underline attribute
		lfLogFont.lfUnderline = TRUE;

		// Create current font with underline attribute (when hovered)
		m_font.CreateFontIndirect(&lfLogFont);

		SetFont(&m_font);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CColorStatic::setUnderline"), 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************                    
*  Function Name  : CtlColor                                                     
*  Description    : The framework calls this member function when a child control is about to be drawn.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0120
*  Date           : 18 Oct 2013
****************************************************************************************************/
HBRUSH CColorStatic::CtlColor(CDC* pDC, UINT nCtlColor)
{
	try
	{
		UNREFERENCED_PARAMETER(nCtlColor);
		pDC->SetTextColor (m_clrText);
		pDC->SetBkMode(m_clrBack);
		return m_brBkgnd;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CColorStatic::CtlColor"), 0, 0, true, SECONDLEVEL);
	}
	return m_brBkgnd;
}
