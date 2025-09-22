#include "StdAfx.h"
#include "MultiColorStatic.h"

/***************************************************************************************************      
*  Function Name  : CMultiColorStatic                                                     
*  Description    : C'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0370
*  Date           : 18 Oct 2013
****************************************************************************************************/
CMultiColorStatic::CMultiColorStatic()
{
   SetBackColor();
   m_astrData.RemoveAll();
   m_fAutoSize = true;
}

/***************************************************************************************************      
*  Function Name  : CMultiColorStatic                                                     
*  Description    : D'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0371
*  Date           : 18 Oct 2013
****************************************************************************************************/
CMultiColorStatic::~CMultiColorStatic()
{
   RemoveAllStrings();
}


/***************************************************************************************************      
*  Function Name  : SetString                                                     
*  Description    : Set the String and check if string lenght is 0
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0372
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CMultiColorStatic::SetString(const int& nIndex, const CColorString& strData)
{
	CObject* pNewOne;
   BOOL     fStringRemoved = FALSE;

	if ((nIndex < 0) || (nIndex > m_astrData.GetSize()))
	{
		ASSERT(FALSE);
		return (FALSE);
	}

   //-------------------------------------------------
   // who believes in strings of 0 length????
   //
   if (strData.GetLength() == 0)
   {
      return (FALSE);
   }

   //--------------------------------------------------
   // if we're setting a previously-allocated string,
   // then we need to delete the old one.
   //
   if (nIndex < m_astrData.GetSize())
   {
      RemoveString(nIndex);
      fStringRemoved = TRUE;
   }

	pNewOne = reinterpret_cast<CObject*>(new CColorString(strData));

	if (pNewOne)
	{
      if (fStringRemoved)
      {
		   m_astrData.InsertAt(nIndex, pNewOne);
      }
      else
      {
         m_astrData.SetAtGrow(nIndex, pNewOne);
      }
      Invalidate();
		return (TRUE);
	}

	return (FALSE);
}

/***************************************************************************************************      
*  Function Name  : GetString                                                     
*  Description    : This function returns the string referenced by nIndex. 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0373
*  Date           : 18 Oct 2013
****************************************************************************************************/
CColorString CMultiColorStatic::GetString(const int& nIndex /* = 0 */) const
{
	CColorString strTemp;		// for returning

	if ((nIndex < 0) || (nIndex > m_astrData.GetUpperBound()))
	{
		ASSERT(FALSE);
		return (FALSE);
	}

	strTemp = *(reinterpret_cast<CColorString*>(m_astrData.GetAt(nIndex)));

	return (strTemp);
}

/***************************************************************************************************      
*  Function Name  : AddString                                                     
*  Description    : This function adds strData to the end of the array of strings.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0374
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CMultiColorStatic::AddString(const CColorString& strData)
{
	return (SetString(static_cast<const int>(m_astrData.GetSize()), strData));
}

/***************************************************************************************************      
*  Function Name  : AddString                                                     
*  Description    : This is the overloaded function that adds a string and its properties
					to the end of the array of strings.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0375
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CMultiColorStatic::AddString(LPCTSTR lpszText, const DWORD& dwStyle /* = 0x00000000*/,
                              const COLORREF& crBackColor /* = 0xFFFFFFFF */)
{
   return (AddString(CColorString(lpszText, dwStyle, crBackColor)));
}

/***************************************************************************************************      
*  Function Name  : RemoveString                                                     
*  Description    : This function removes a string at an index.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0376
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CMultiColorStatic::RemoveString(const int& nIndex)
{
   //-----------------------------------------------------
   // make sure they pass in a valid index
   //
   if ((nIndex < 0) || (nIndex > m_astrData.GetUpperBound()))
   {
      ASSERT(FALSE);    // an invalid index was passed in
      return (FALSE);
   }

   CColorString* pstrCurrent = reinterpret_cast<CColorString*>(m_astrData.GetAt(nIndex));
   if (pstrCurrent)
   {
      m_astrData.RemoveAt(nIndex);
      delete pstrCurrent;
   }

   //--------------------------------------
   // the destructor calls this indirectly
   // it's not a bad idea to check anyway
   // i guess
   //
   if (::IsWindow(m_hWnd))
   {
      Invalidate();
   }

   return (TRUE);
}

/***************************************************************************************************      
*  Function Name  : RemoveAllStrings                                                     
*  Description    : This function removes all strings in the control. 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0377
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMultiColorStatic::RemoveAllStrings(void)
{
  	while (m_astrData.GetSize() > 0)
	{
		RemoveString(static_cast<const int>(m_astrData.GetUpperBound()));
	}
   
   //------------------------------------------
   // this isn't really needed; i just do it
   // to make sure
   //
   if (::IsWindow(m_hWnd))
   {
      Invalidate();
   }
}

/***************************************************************************************************      
*  Function Name  : GetNumStrings                                                     
*  Description    : This function returns the number of strings in the control. 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0378
*  Date           : 18 Oct 2013
****************************************************************************************************/
DWORD CMultiColorStatic::GetNumStrings(void) const
{
	return (static_cast<DWORD>(m_astrData.GetSize()));
}

/***************************************************************************************************      
*  Function Name  : SetBackColor                                                     
*  Description    : This function sets the default back color of the control.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0379
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMultiColorStatic::SetBackColor(const COLORREF& crBackColor /* = 0xFFFFFFFF */)
{
	if (crBackColor == 0xFFFFFFFF)
	{
		m_crBackColor = ::GetSysColor(COLOR_BTNFACE);
	}
	else
	{
		m_crBackColor = crBackColor;
	}

	m_brBackGround.DeleteObject();
	m_brBackGround.CreateSolidBrush(m_crBackColor);

   if (::IsWindow(m_hWnd))
   {
      Invalidate();
   }
}

/***************************************************************************************************      
*  Function Name  : GetBackColor                                                     
*  Description    : This function returns the background color of the entire control. 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0380
*  Date           : 18 Oct 2013
****************************************************************************************************/
COLORREF CMultiColorStatic::GetBackColor(void) const
{
	return (m_crBackColor);
}

/***************************************************************************************************      
*  Function Name  : GetWindowText                                                     
*  Description    : This function returns the window text as if there were no color-data.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0381
*  Date           : 18 Oct 2013
****************************************************************************************************/
int CMultiColorStatic::GetWindowText(LPTSTR lpszBuffer, int nMaxCount)
{
   memset(lpszBuffer, 0, nMaxCount);
   CalculateWindowText();
  // strncpy(lpszBuffer, m_strWindowText, nMaxCount);

   lpszBuffer = m_strWindowText.GetBuffer(m_strWindowText.GetLength());
   if (m_strWindowText.GetLength() == nMaxCount)
   {
      lpszBuffer[nMaxCount] = 0;
   }

   return (m_strWindowText.GetLength());
}

/***************************************************************************************************      
*  Function Name  : GetWindowText                                                     
*  Description    : This function returns the window text as if there were no color-data 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0382
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMultiColorStatic::GetWindowText(CString& strData)
{
   CalculateWindowText();
   strData = m_strWindowText;
   return;
}

/***************************************************************************************************      
*  Function Name  : SetWindowText                                                     
*  Description    : This function sets the window text; the text will have a normal font.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0383
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMultiColorStatic::SetWindowText(LPCTSTR lpszValue)
{
   RemoveAllStrings();
   AddString(lpszValue);
}

/***************************************************************************************************      
*  Function Name  : SetAutoSize                                                     
*  Description    : When the banner-size gets large, the text will increase in size along 
                    with it. Otherwise the text will be tiny. 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0384
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMultiColorStatic::SetAutoSize(const bool& fAutoSize)
{
   m_fAutoSize = fAutoSize;
}

/***************************************************************************************************      
*  Function Name  : GetAutoSize                                                     
*  Description    : This returns the AutoSize flag. 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0385
*  Date           : 18 Oct 2013
****************************************************************************************************/
bool CMultiColorStatic::GetAutoSize(void) const
{
   return (m_fAutoSize);
}

/***************************************************************************************************      
*  Function Name  : DetermineFont                                                     
*  Description    : This function determines the font based on a the value in pstrData. 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0386
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMultiColorStatic::DetermineFont(const CColorString* const pstrData)
{
   LOGFONT stFont;

   m_ftText.DeleteObject();

   //---------------------------------------------------
   // set up the font based on pstrData
   //
   if (!GetFont()->GetLogFont(&stFont))
   {		
	   memset(&stFont, 0, sizeof(stFont));

	   stFont.lfWidth = ((stFont.lfHeight * 7) / 16);
	   stFont.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
   }

   if (m_fAutoSize)
   {
	   stFont.lfHeight = m_rcBounds.Height();
   }

   stFont.lfWeight = (pstrData->GetBold() ? FW_HEAVY : FW_NORMAL);
   stFont.lfUnderline = pstrData->GetUnderlined();
   stFont.lfItalic = pstrData->GetItalic();
   stFont.lfQuality = PROOF_QUALITY;
   m_ftText.CreateFontIndirect(&stFont);
}

/***************************************************************************************************      
*  Function Name  : CalculateWindowText                                                     
*  Description    : This function calculates the window text based on all strings in control. 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0387
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMultiColorStatic::CalculateWindowText(void)
{
   m_strWindowText.Empty();

   for (int i = 0; i < m_astrData.GetSize(); i++)
   {
      CColorString* pCurrent = (CColorString*)m_astrData.GetAt(i);
      if (pCurrent)
      {
         m_strWindowText += *pCurrent;
      }
   }
}

/***************************************************************************************************      
*  Function Name  : PreSubclassWindow                                                     
*  Description    : disply the sub classs window
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0388
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMultiColorStatic::PreSubclassWindow(void)
{
   GetClientRect(m_rcBounds);

   CDC* pDC = GetDC();
   pDC->FillRect(m_rcBounds, &m_brBackGround);
   ReleaseDC(pDC);

   //---------------------------------------
   // derived classes should be able to 
   // notify parent of mouse input
   //
   ModifyStyle(0, SS_NOTIFY);
}

/***************************************************************************************************      
*  Function Name  : BEGIN_MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0389
*  Date           : 18 Oct 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CMultiColorStatic, CStatic)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/***************************************************************************************************      
*  Function Name  : OnPaint                                                     
*  Description    : The framework calls this member function when Windows or an application makes a request
					to repaint a portion of an application's window.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0390
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMultiColorStatic::OnPaint() 
{
   CPaintDC dc(this);
      
   CRect rcBounds = m_rcBounds;

   dc.FillRect(m_rcBounds, &m_brBackGround);

   //------------------------------------------------------
   // draw each string with its own font
   //
   for (int i = 0; i < m_astrData.GetSize(); i++)
   {
      CColorString* pstrCurrent = reinterpret_cast<CColorString*>(m_astrData.GetAt(i));
      TEXTMETRIC    stFontMetrics;
      SIZE          stSize;

      DetermineFont(pstrCurrent);

		//---------------------------------------------------
		// set up the drawing attributes
		//
		dc.SelectObject(&m_ftText)->DeleteObject(); 
      if (pstrCurrent->GetBackColor() == ::GetSysColor(COLOR_BTNFACE))
      {
         dc.SetBkColor(m_crBackColor);
      }
      else
      {
         dc.SetBkColor(pstrCurrent->GetBackColor());
      }
		dc.SetTextColor(pstrCurrent->GetColor());
		dc.GetOutputTextMetrics(&stFontMetrics);

		GetTextExtentPoint32(dc.GetSafeHdc(), *pstrCurrent, pstrCurrent->GetLength(), &stSize);
				
		//---------------------------------------------------
		// do the drawing -- DrawText won't let us
		// go outside our client-area
		//			
      dc.DrawText(*pstrCurrent, rcBounds, DT_LEFT);
      rcBounds.left += stSize.cx + stFontMetrics.tmOverhang;
   }
}

// we need to keep track of our client rect; i hate recalculating when i need it
/***************************************************************************************************      
*  Function Name  : OnSize                                                     
*  Description    : Vary size of item insert into html list control.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0391
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMultiColorStatic::OnSize(UINT nType, int cx, int cy) 
{
	CStatic::OnSize(nType, cx, cy);
	GetClientRect(m_rcBounds);
}

// make sure we clean up
/***************************************************************************************************      
*  Function Name  : OnDestroy                                                     
*  Description    : Clean up unused space.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0392
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMultiColorStatic::OnDestroy() 
{
	CStatic::OnDestroy();
   m_ftText.DeleteObject();
   m_brBackGround.DeleteObject();
}

// we're taking care of this with fillrect; forget about it here
/***************************************************************************************************      
*  Function Name  : OnEraseBkgnd                                                     
*  Description    : On Background close.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0393
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CMultiColorStatic::OnEraseBkgnd(CDC* pDC) 
{	
   return (TRUE);
}

