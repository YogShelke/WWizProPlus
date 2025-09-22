#include "stdafx.h"
#include "GdipButton.h"
#include "CGdiPlusBitmap.h"
#include "MemDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************************************      
*  Function Name  : CGdipButton                                                     
*  Description    : C'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0394
*  Date           : 18 Oct 2013
****************************************************************************************************/
CGdipButton::CGdipButton()
{
	m_pStdImage = NULL;
	m_pAltImage = NULL;

	m_bHaveBitmaps = FALSE;
	m_bHaveAltImage = FALSE;

	m_pCurBtn = NULL;

	m_bIsDisabled = FALSE;
	m_bIsToggle = FALSE;

	m_bIsHovering = FALSE;
	m_bIsTracking = FALSE;

	m_nCurType = STD_TYPE;

	m_pToolTip = NULL;

}

/***************************************************************************************************      
*  Function Name  : CTextScroller                                                     
*  Description    : D'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0395
*  Date           : 18 Oct 2013
****************************************************************************************************/
CGdipButton::~CGdipButton()
{
	if(m_pStdImage) delete m_pStdImage;
	if(m_pAltImage) delete m_pAltImage;

	if(m_pToolTip)	delete m_pToolTip;
}

/***************************************************************************************************      
*  Function Name  : BEGIN_MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0396
*  Date           : 18 Oct 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CGdipButton, CButton)
	//{{AFX_MSG_MAP(CGdipButton)
	ON_WM_DRAWITEM()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/***************************************************************************************************      
*  Function Name  : LoadStdImage                                                     
*  Description    : The LoadStdImage() Loads the image for the button.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0397
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CGdipButton::LoadStdImage(UINT id, LPCTSTR pType)
{
	m_pStdImage = new CGdiPlusBitmapResource;
	return m_pStdImage->Load(id, pType);
}

/***************************************************************************************************      
*  Function Name  : LoadAltImage                                                     
*  Description    : The LoadAltImage() Loads the altername image for the button.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0398
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CGdipButton::LoadAltImage(UINT id, LPCTSTR pType)
{
	m_bHaveAltImage = TRUE;
	m_pAltImage = new CGdiPlusBitmapResource;
	return (m_pAltImage->Load(id, pType));
}

/***************************************************************************************************      
*  Function Name  : CtlColor                                                     
*  Description    : The framework calls this member function when a child control is about to br drawn.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0399
*  Date           : 18 Oct 2013
****************************************************************************************************/
HBRUSH CGdipButton::CtlColor(CDC* pScreenDC, UINT nCtlColor) 
{
	if(!m_bHaveBitmaps)
	{
		if(!m_pStdImage)
		{
			return NULL; // Load the standard image with LoadStdImage()
		}

		CBitmap bmp, *pOldBitmap;

		CRect rect;
		GetClientRect(rect);

		// do everything with mem dc
		CMemDCEX pDC(pScreenDC, rect);

		Gdiplus::Graphics graphics(pDC->m_hDC);

		// background
		if (m_dcBk.m_hDC == NULL)
		{

			CRect rect1;
			CClientDC clDC(GetParent());
			GetWindowRect(rect1);
			GetParent()->ScreenToClient(rect1);

			m_dcBk.CreateCompatibleDC(&clDC);
			bmp.CreateCompatibleBitmap(&clDC, rect.Width(), rect.Height());
			pOldBitmap = m_dcBk.SelectObject(&bmp);
			m_dcBk.BitBlt(0, 0, rect.Width(), rect.Height(), &clDC, rect1.left, rect1.top, SRCCOPY);
			bmp.DeleteObject();
		}

		// standard image
		if (m_dcStd.m_hDC == NULL)
		{
			PaintBk(pDC);

			graphics.DrawImage(*m_pStdImage, 0, 0);
		
			m_dcStd.CreateCompatibleDC(pDC);
			bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
			pOldBitmap = m_dcStd.SelectObject(&bmp);
			m_dcStd.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, 0, 0, SRCCOPY);
			bmp.DeleteObject();

			// standard image pressed
			if (m_dcStdP.m_hDC == NULL)
			{
				PaintBk(pDC);

				graphics.DrawImage(*m_pStdImage, 1, 1);

				m_dcStdP.CreateCompatibleDC(pDC);
				bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
				pOldBitmap = m_dcStdP.SelectObject(&bmp);
				m_dcStdP.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, 0, 0, SRCCOPY);
				bmp.DeleteObject();
			}

			// standard image hot
			if(m_dcStdH.m_hDC == NULL)
			{
				PaintBk(pDC);

				Gdiplus::ColorMatrix HotMat = {	1.05f, 0.00f, 0.00f, 0.00f, 0.00f,
																	0.00f, 1.05f, 0.00f, 0.00f, 0.00f,
																	0.00f, 0.00f, 1.05f, 0.00f, 0.00f,
																	0.00f, 0.00f, 0.00f, 1.00f, 0.00f,
																	0.05f, 0.05f, 0.05f, 0.00f, 1.00f	};

				Gdiplus::ImageAttributes ia;
				ia.SetColorMatrix(&HotMat);

				float width = (float)m_pStdImage->m_pBitmap->GetWidth();
				float height = (float)m_pStdImage->m_pBitmap->GetHeight();

				Gdiplus::RectF grect; grect.X=0, grect.Y=0; grect.Width = width; grect.Height = height;

				graphics.DrawImage(*m_pStdImage, grect, 0, 0, width, height, Gdiplus::UnitPixel, &ia);

				m_dcStdH.CreateCompatibleDC(pDC);
				bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
				pOldBitmap = m_dcStdH.SelectObject(&bmp);
				m_dcStdH.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, 0, 0, SRCCOPY);
				bmp.DeleteObject();
			}

			// grayscale image
			if(m_dcGS.m_hDC == NULL)
			{
				PaintBk(pDC);

				Gdiplus::ColorMatrix GrayMat = {	0.30f, 0.30f, 0.30f, 0.00f, 0.00f,
										0.59f, 0.59f, 0.59f, 0.00f, 0.00f,
										0.11f, 0.11f, 0.11f, 0.00f, 0.00f,
										0.00f, 0.00f, 0.00f, 1.00f, 0.00f,
										0.00f, 0.00f, 0.00f, 0.00f, 1.00f	};

				Gdiplus::ImageAttributes ia;
				ia.SetColorMatrix(&GrayMat);

				float width = (float)m_pStdImage->m_pBitmap->GetWidth();
				float height = (float)m_pStdImage->m_pBitmap->GetHeight();

				Gdiplus::RectF grect; grect.X=0, grect.Y=0; grect.Width = width; grect.Height = height;

				graphics.DrawImage(*m_pStdImage, grect, 0, 0, width, height, Gdiplus::UnitPixel, &ia);

				m_dcGS.CreateCompatibleDC(pDC);
				bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
				pOldBitmap = m_dcGS.SelectObject(&bmp);
				m_dcGS.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, 0, 0, SRCCOPY);
				bmp.DeleteObject();
			}
		}

		// alternate image
		if( (m_dcAlt.m_hDC == NULL) && m_bHaveAltImage )
		{
			PaintBk(pDC);

			graphics.DrawImage(*m_pAltImage, 0, 0);
		
			m_dcAlt.CreateCompatibleDC(pDC);
			bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
			pOldBitmap = m_dcAlt.SelectObject(&bmp);
			m_dcAlt.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, 0, 0, SRCCOPY);
			bmp.DeleteObject();

			// alternate image pressed
			if( (m_dcAltP.m_hDC == NULL) && m_bHaveAltImage )
			{
				PaintBk(pDC);

				graphics.DrawImage(*m_pAltImage, 1, 1);
			
				m_dcAltP.CreateCompatibleDC(pDC);
				bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
				pOldBitmap = m_dcAltP.SelectObject(&bmp);
				m_dcAltP.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, 0, 0, SRCCOPY);
				bmp.DeleteObject();
			}

			// alternate image hot
			if(m_dcAltH.m_hDC == NULL)
			{
				PaintBk(pDC);

				Gdiplus::ColorMatrix HotMat = {	1.05f, 0.00f, 0.00f, 0.00f, 0.00f,
										0.00f, 1.05f, 0.00f, 0.00f, 0.00f,
										0.00f, 0.00f, 1.05f, 0.00f, 0.00f,
										0.00f, 0.00f, 0.00f, 1.00f, 0.00f,
										0.05f, 0.05f, 0.05f, 0.00f, 1.00f	};

				Gdiplus::ImageAttributes ia;
				ia.SetColorMatrix(&HotMat);

				float width = (float)m_pStdImage->m_pBitmap->GetWidth();
				float height = (float)m_pStdImage->m_pBitmap->GetHeight();

				Gdiplus::RectF grect; grect.X=0, grect.Y=0; grect.Width = width; grect.Height = height;

				graphics.DrawImage(*m_pAltImage, grect, 0, 0, width, height, Gdiplus::UnitPixel, &ia);

				m_dcAltH.CreateCompatibleDC(pDC);
				bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
				pOldBitmap = m_dcAltH.SelectObject(&bmp);
				m_dcAltH.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, 0, 0, SRCCOPY);
				bmp.DeleteObject();
			}
		}

		if(m_pCurBtn == NULL)
		{
			m_pCurBtn = &m_dcStd;
		}

		m_bHaveBitmaps = TRUE;
	}

	return NULL;
}

// paint the background
/***************************************************************************************************      
*  Function Name  : PaintBk                                                     
*  Description    : paint the background
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0400
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CGdipButton::PaintBk(CDC *pDC)
{
	CRect rect;
	GetClientRect(rect);
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &m_dcBk, 0, 0, SRCCOPY);
}

// paint the bitmap currently pointed to with m_pCurBtn
/***************************************************************************************************      
*  Function Name  : PaintBtn                                                     
*  Description    : paint the bitmap currently pointed to with m_pCurBtn
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0401
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CGdipButton::PaintBtn(CDC *pDC)
{
	CRect rect;
	GetClientRect(rect);
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), m_pCurBtn, 0, 0, SRCCOPY);
}

// enables the toggle mode
// returns if it doesn't have the alternate image
/***************************************************************************************************      
*  Function Name  : EnableToggle                                                     
*  Description    : enables the toggle mode, returns if it doesn't have the alternate image
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0402
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CGdipButton::EnableToggle(BOOL bEnable)
{
	if(!m_bHaveAltImage) return;

	m_bIsToggle = bEnable; 

	// this actually makes it start in the std state since toggle is called before paint
	if(bEnable)	m_pCurBtn = &m_dcAlt;
	else		m_pCurBtn = &m_dcStd;

}

// sets the image type and disabled state then repaints
/***************************************************************************************************      
*  Function Name  : SetImage                                                     
*  Description    : sets the image type and disabled state then repaints
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0403
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CGdipButton::SetImage(int type)
{
	m_nCurType = type;

	(type == DIS_TYPE) ? m_bIsDisabled = TRUE : m_bIsDisabled = FALSE;

	Invalidate();
}

// set the control to owner draw
/***************************************************************************************************      
*  Function Name  : PreSubclassWindow                                                     
*  Description    : Display the previous window
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0404
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CGdipButton::PreSubclassWindow()
{
	// Set control to owner draw
	ModifyStyle(0, BS_OWNERDRAW, SWP_FRAMECHANGED);

	CButton::PreSubclassWindow();
}

// disable double click 
/***************************************************************************************************      
*  Function Name  : PreTranslateMessage                                                     
*  Description    : disable double click 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0405
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CGdipButton::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_LBUTTONDBLCLK)
		pMsg->message = WM_LBUTTONDOWN;

	if (m_pToolTip != NULL)
	{
		if (::IsWindow(m_pToolTip->m_hWnd))
		{
			m_pToolTip->RelayEvent(pMsg);		
		}
	}

	return CButton::PreTranslateMessage(pMsg);
}


//=============================================================================
// overide the erase function
//=============================================================================
/***************************************************************************************************      
*  Function Name  : OnEraseBkgnd                                                     
*  Description    : overide the erase function
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0406
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CGdipButton::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

// Paint the button depending on the state of the mouse
/***************************************************************************************************      
*  Function Name  : DrawItem                                                     
*  Description    : Paint the button depending on the state of the mouse
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0407
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CGdipButton::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	// handle disabled state
	if(m_bIsDisabled)
	{
		m_pCurBtn = &m_dcGS;
		PaintBtn(pDC);
		return;
	}

	BOOL bIsPressed = (lpDIS->itemState & ODS_SELECTED);

	// handle toggle button
	if(m_bIsToggle && bIsPressed)
	{
		(m_nCurType == STD_TYPE) ? m_nCurType = ALT_TYPE : m_nCurType = STD_TYPE;
	}

	if(bIsPressed)
	{
		if(m_nCurType == STD_TYPE)
			m_pCurBtn = &m_dcStdP;
		else
			m_pCurBtn = &m_dcAltP;
	}
	else if(m_bIsHovering)
	{

		if(m_nCurType == STD_TYPE)
			m_pCurBtn = &m_dcStdH;
		else
			m_pCurBtn = &m_dcAltH;
	}
	else
	{
		if(m_nCurType == STD_TYPE)
			m_pCurBtn = &m_dcStd;
		else
			m_pCurBtn = &m_dcAlt;
	}

	// paint the button
	PaintBtn(pDC);
}

/***************************************************************************************************      
*  Function Name  : OnMouseHover                                                     
*  Description    : Create a new Tooltip with new Button Size and Location on mouse over it.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0408
*  Date           : 18 Oct 2013
****************************************************************************************************/
LRESULT CGdipButton::OnMouseHover(WPARAM wparam, LPARAM lparam) 
{
	m_bIsHovering = TRUE;
	Invalidate();
	DeleteToolTip();

	// Create a new Tooltip with new Button Size and Location
	SetToolTipText(m_tooltext);

	if (m_pToolTip != NULL)
	{
		if (::IsWindow(m_pToolTip->m_hWnd))
		{
			//Display ToolTip
			m_pToolTip->Update();
		}
	}

	return 0;
}

/***************************************************************************************************      
*  Function Name  : OnMouseLeave                                                     
*  Description    : todo on mouse leave.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0409
*  Date           : 18 Oct 2013
****************************************************************************************************/
LRESULT CGdipButton::OnMouseLeave(WPARAM wparam, LPARAM lparam)
{
	m_bIsTracking = FALSE;
	m_bIsHovering = FALSE;
	Invalidate();
	return 0;
}

/***************************************************************************************************      
*  Function Name  : OnMouseMove                                                     
*  Description    : Function which states what to do on mouse move.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0410
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CGdipButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (!m_bIsTracking)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE|TME_HOVER;
		tme.dwHoverTime = 1;
		m_bIsTracking = _TrackMouseEvent(&tme);
	}
	
	CButton::OnMouseMove(nFlags, point);
}

/***************************************************************************************************      
*  Function Name  : SetBkGnd                                                     
*  Description    : Call this member function with a memory DC from the code that paints the parents background.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0411
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CGdipButton::SetBkGnd(CDC* pDC)
{
	CRect rect, rectS;
	CBitmap bmp, *pOldBitmap;

	GetClientRect(rect);
	GetWindowRect(rectS);
	GetParent()->ScreenToClient(rectS);
	
	m_dcBk.DeleteDC();

	m_dcBk.CreateCompatibleDC(pDC);
	bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	pOldBitmap = m_dcBk.SelectObject(&bmp);
	m_dcBk.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, rectS.left, rectS.top, SRCCOPY);
	bmp.DeleteObject();
}

/***************************************************************************************************      
*  Function Name  : SetToolTipText                                                     
*  Description    : Set the tooltip with a string resource
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0412
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CGdipButton::SetToolTipText(UINT nId, BOOL bActivate)
{
	// load string resource
	m_tooltext.LoadString(nId);

	// If string resource is not empty
	if (m_tooltext.IsEmpty() == FALSE)
	{
		SetToolTipText(m_tooltext, bActivate);
	}

}

// Set the tooltip with a CString
/***************************************************************************************************      
*  Function Name  : SetToolTipText                                                     
*  Description    : Set the tooltip with a CString
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0413
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CGdipButton::SetToolTipText(CString spText, BOOL bActivate)
{
	// We cannot accept NULL pointer
	if (spText.IsEmpty()) return;

	// Initialize ToolTip
	InitToolTip();
	m_tooltext = spText;

	// If there is no tooltip defined then add it
	if (m_pToolTip->GetToolCount() == 0)
	{
		CRect rectBtn; 
		GetClientRect(rectBtn);
		m_pToolTip->AddTool(this, m_tooltext, rectBtn, 1);
	}

	// Set text for tooltip
	m_pToolTip->UpdateTipText(m_tooltext, this, 1);
	m_pToolTip->SetDelayTime(2000);
	m_pToolTip->Activate(bActivate);
}

/***************************************************************************************************      
*  Function Name  : InitToolTip                                                     
*  Description    : Initialize the tool tip
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0414
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CGdipButton::InitToolTip()
{
	if (m_pToolTip == NULL)
	{
		m_pToolTip = new CToolTipCtrl;
		// Create ToolTip control
		m_pToolTip->Create(this);
		m_pToolTip->Activate(TRUE);
	}
} 

/***************************************************************************************************      
*  Function Name  : DeleteToolTip                                                     
*  Description    : Delete the tool tip.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0415
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CGdipButton::DeleteToolTip()
{
	// Destroy Tooltip incase the size of the button has changed.
	if (m_pToolTip != NULL)
	{
		delete m_pToolTip;
		m_pToolTip = NULL;
	}
}

