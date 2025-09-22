/***************************************************************************                      
   Program Name          : xSkinButton.cpp
   Description           : Set bitmap on buttons.
   Author Name           :                                                                         
   Date Of Creation      : 18th Oct,2013
   Version No            : 1.0.0.1
   Special Logic Used    : 
   Modification Log      :           
****************************************************************************/
#include "stdafx.h"
#include "xSkinButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************************************                    
*  Function Name  : CxSkinButton                                                     
*  Description    : C'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0226
*  Date           : 18 Oct 2013
****************************************************************************************************/
CxSkinButton::CxSkinButton()
{
	try
	{
		m_DrawMode				=	1;			// normal drawing mode
		m_FocusRectMargin		=	0;	// disable focus dotted rect
		hClipRgn				=	NULL;			// no clipping region
		m_TextColor				=	RGB(17, 31, 118);
		m_DownTextColor			=	RGB(255, 255, 255);
		m_OverTextColor			=	RGB(255, 255, 255);
		m_FocusTextColor		=   0;
		m_button_down			=	m_tracking = m_Checked = false;
		m_AlignStyle			=	DT_VCENTER|DT_CENTER;
		m_bShowText				=	true;
		m_bShiftClickText		=	true;
		m_bShowMultiline		=	false;
		m_bShowTopline			=	false; //Displays text center + 3 pixels (Nupur)
		m_pstBoost				=	NULL;
		m_bBottom				=	false;
		m_bBtnRadio_down 		= false;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::CxSkinButton"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : ~CxSkinButton                                                     
*  Description    : D'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0227
*  Date           : 18 Oct 2013
****************************************************************************************************/
CxSkinButton::~CxSkinButton()
{
	try
	{
		if(hClipRgn)
			DeleteObject(hClipRgn);	// free clip region
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::~CxSkinButton"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0228
*  Date           : 18 Oct 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CxSkinButton, CButton)
	//{{AFX_MSG_MAP(CxSkinButton)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_CXSHADE_RADIO, OnRadioInfo)
	ON_MESSAGE(BM_SETCHECK, OnBMSetCheck)
	ON_MESSAGE(BM_GETCHECK, OnBMGetCheck)
END_MESSAGE_MAP()

/***************************************************************************************************                    
*  Function Name  : PreSubclassWindow                                                     
*  Description    : Before subclassing
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0229
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::PreSubclassWindow()
{
	try
	{
		m_Style=GetButtonStyle();	///get specific BS_ styles

		if((m_Style & BS_AUTOCHECKBOX) == BS_AUTOCHECKBOX)
			m_Style=BS_CHECKBOX;
		else if((m_Style & BS_AUTORADIOBUTTON) == BS_AUTORADIOBUTTON)
			m_Style=BS_RADIOBUTTON;
		else
			m_Style=BS_PUSHBUTTON;


		CButton::PreSubclassWindow();
		ModifyStyle(0, BS_OWNERDRAW);

	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::PreSubclassWindow"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : SetResourceHandle                                                     
*  Description    : Set Wardwiz Resource dll handler.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0230
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::SetResourceHandle(HANDLE hResHandle, HWND hWnd)
{
	AfxSetResourceHandle((HINSTANCE)hResHandle);
	this->m_hWnd = hWnd;

}

/***************************************************************************************************                    
*  Function Name  : OnEraseBkgnd                                                     
*  Description    : The framework calls this member function when the CWnd object background needs erasing (for example, when resized).
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0231
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CxSkinButton::OnEraseBkgnd(CDC* pDC)
{
	return 1; // doesn't erase the button background
}

/***************************************************************************************************                    
*  Function Name  : DrawItem                                                     
*  Description    : Called by the framework when a visual aspect of an owner-draw list view control changes.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0232
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	try
	{
		ASSERT (lpDrawItemStruct);

		//Check if the button state in not in inconsistent mode...
		POINT mouse_position;
		if((m_button_down) && (::GetCapture() == m_hWnd) && (::GetCursorPos(&mouse_position)))
		{
			if(::WindowFromPoint(mouse_position) == m_hWnd)
			{
				if((GetState()& BST_PUSHED) != BST_PUSHED)
				{
					SetState(TRUE);
					return;
				}
			}
			else
			{
				if((GetState()& BST_PUSHED) == BST_PUSHED)
				{
					SetState(FALSE);
					return;
				}
			}
		}

		CString sCaption;
		CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);	// get device context
		RECT r=lpDrawItemStruct->rcItem;					// context rectangle
		
		// get text box position
		RECT tr={r.left + m_FocusRectMargin + 2, r.top, r.right - m_FocusRectMargin - 2, r.bottom};

		GetWindowText(sCaption);							// get button text

		pDC->SetBkMode(TRANSPARENT);

		// Select the correct skin
		if(lpDrawItemStruct->itemState & ODS_DISABLED)
		{
			// DISABLED BUTTON
			if(m_bDisabled.m_hObject == NULL)
				// no skin selected for disabled state->standard button
				pDC->FillSolidRect(&r,GetSysColor(COLOR_BTNFACE));
			else // paint the skin
				DrawBitmap(pDC,(HBITMAP)m_bDisabled,r,m_DrawMode);
			// if needed, draw the standard 3D rectangular border
			if(m_Border)pDC->DrawEdge(&r,EDGE_RAISED,BF_RECT);
			// paint the etched button text
			//pDC->SetTextColor(GetSysColor(COLOR_3DHILIGHT));
			pDC->SetTextColor(RGB(150,150,150));//Disable button color Neha Gharge
			if(m_AlignStyle == DT_BOTTOM)
			{
				if(m_bBottom == false)
				{
					tr.top = tr.top - 2;
					tr.bottom = tr.bottom - 5;
				}
				else
				{
					tr.top = tr.top - 10;
					tr.bottom = tr.bottom - 13;
				}
			}
			
			if(m_bShowText && !m_bShowMultiline)
				pDC->DrawText(sCaption,&tr,DT_SINGLELINE|m_AlignStyle/*|DT_CENTER*/);

			//pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
			pDC->SetTextColor(RGB(150,150,150));//Disable button color Neha Gharge

			if(m_bShowText && !m_bShowMultiline)
				pDC->DrawText(sCaption,&tr,DT_SINGLELINE|m_AlignStyle/*|DT_CENTER*/);

			if(m_bShowMultiline)	
			{
				tr.top = tr.top + 43;
				tr.bottom = tr.bottom - 5;
				if(sCaption.Find(_T("/")) == -1)
					ModifyStyleEx(lpDrawItemStruct->hwndItem,0,BS_MULTILINE,0);
				pDC->DrawText(sCaption,&tr,DT_WORDBREAK /*| DT_CENTER*/);
			}
			//Nupur: Shifting text 3 pixels above the center for Max PC Boost
			else if(m_AlignStyle == DT_VCENTER && m_bShowTopline)
			{
				tr.top = tr.top - 3;
			}
		}
		else
		{										// SELECTED (DOWN)BUTTON
			if((lpDrawItemStruct->itemState & ODS_SELECTED) ||m_Checked)
			{
				if(m_bDown.m_hObject == NULL)
					// no skin selected for selected state->standard button
					pDC->FillSolidRect(&r,GetSysColor(COLOR_BTNFACE));
				else
				{ // paint the skin
					DrawBitmap(pDC,(HBITMAP)m_bDown,r,m_DrawMode);
				}
				if(m_bShiftClickText)
					OffsetRect(&tr,1,1);  //shift text
				// if needed, draw the standard 3D rectangular border
				if(m_Border)pDC->DrawEdge(&r,EDGE_SUNKEN,BF_RECT);
				// paint the enabled button text
				pDC->SetTextColor(m_DownTextColor);
			}
			else
			{											// DEFAULT BUTTON
				if(m_bNormal.m_hObject == NULL)
					// no skin selected for normal state->standard button
					pDC->FillSolidRect(&r,GetSysColor(COLOR_BTNFACE));
				else if((m_tracking) &&(m_bOver.m_hObject!=NULL))// paint the skin
				{
					DrawBitmap(pDC,(HBITMAP)m_bOver,r,m_DrawMode);
					if(m_FocusTextColor != 0)
						pDC->SetTextColor(m_FocusTextColor);
					else
						// paint the enabled button text
						pDC->SetTextColor(m_OverTextColor);
				}
				else
				{
					// paint the enabled button text
					pDC->SetTextColor(m_TextColor);
					if((lpDrawItemStruct->itemState & ODS_FOCUS) &&(m_bFocus.m_hObject!=NULL))
					{
						if(m_FocusTextColor != 0)
							pDC->SetTextColor(m_FocusTextColor);
						DrawBitmap(pDC,(HBITMAP)m_bFocus,r,m_DrawMode);
					}
					else
					{
						DrawBitmap(pDC,(HBITMAP)m_bNormal,r,m_DrawMode);
					}
				}
				// if needed, draw the standard 3D rectangular border
				if(m_Border)pDC->DrawEdge(&r,EDGE_RAISED,BF_RECT);

			}
			// paint the focus rect
			if((lpDrawItemStruct->itemState & ODS_FOCUS) &&(m_FocusRectMargin>0))
			{
				r.left   += m_FocusRectMargin;
				r.top    += m_FocusRectMargin;
				r.right  -= m_FocusRectMargin;
				r.bottom -= m_FocusRectMargin;
				DrawFocusRect (lpDrawItemStruct->hDC, &r);
			}

			if(m_AlignStyle == DT_BOTTOM)
			{
				if(m_bBottom == false)
				{
					tr.top = tr.top - 2;
					tr.bottom = tr.bottom - 5;
				}
				else
				{
					tr.top = tr.top - 10;
					tr.bottom = tr.bottom - 13;
				}
				if(m_bShowText && !m_bShowMultiline)
					pDC->DrawText(sCaption,&tr,DT_SINGLELINE|m_AlignStyle|DT_CENTER);
			}
			
			
			//Nupur: Shifting text 3 pixels above the center for Max PC Boost
			else if(m_AlignStyle == DT_VCENTER && m_bShowTopline)
			{
				tr.top = tr.top - 3;
				if(m_bShowText && !m_bShowMultiline)
					pDC->DrawText(sCaption,&tr,DT_SINGLELINE|m_AlignStyle|DT_CENTER);
			}
			else
			{
				if(m_bShowText && !m_bShowMultiline)
					pDC->DrawText(sCaption,&tr,DT_SINGLELINE|m_AlignStyle);
			}
			if(m_bShowMultiline)	
			{
				tr.top = tr.top + 43;
				if(m_pstBoost)
					tr.top += 5;

				tr.bottom = tr.bottom - 5;
				if(sCaption.Find(_T("/")) == -1)
					ModifyStyleEx(lpDrawItemStruct->hwndItem,0,BS_MULTILINE,0);
				pDC->DrawText(sCaption,&tr,DT_WORDBREAK | DT_CENTER);
			}

		}
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::DrawItem"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : GetBitmapWidth                                                     
*  Description    : Get bitmap image width
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0233
*  Date           : 18 Oct 2013
****************************************************************************************************/
int CxSkinButton::GetBitmapWidth (HBITMAP hBitmap)
{
	try
	{
		BITMAP bm; GetObject(hBitmap,sizeof(BITMAP),(PSTR)&bm);
		return bm.bmWidth;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::DrawItem"), 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : GetBitmapHeight                                                     
*  Description    : Get bitmap image height
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0234
*  Date           : 18 Oct 2013
****************************************************************************************************/
int CxSkinButton::GetBitmapHeight (HBITMAP hBitmap)
{
	try
	{
		BITMAP bm; GetObject(hBitmap,sizeof(BITMAP),(PSTR)&bm);
		return bm.bmHeight;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::GetBitmapHeight"), 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : DrawBitmap                                                     
*  Description    : Draw bitmap image
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0235
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::DrawBitmap(CDC* dc, HBITMAP hbmp, RECT r, int DrawMode)
{
	//	DrawMode: 0=Normal; 1=stretch; 2=tiled fill
	try
	{
		if(DrawMode == 2)
		{
			FillWithBitmap(dc,hbmp,r);
			return;
		}
		if(!hbmp)
			return;	//safe check

		int cx=r.right  - r.left;
		int cy=r.bottom - r.top;
		CDC dcBmp,dcMask;
		dcBmp.CreateCompatibleDC(dc);
		dcBmp.SelectObject(hbmp);

		if(m_bMask.m_hObject!=NULL)
		{
			dcMask.CreateCompatibleDC(dc);
			dcMask.SelectObject(m_bMask);

			CDC hdcMem;
			hdcMem.CreateCompatibleDC(dc);
			CBitmap hBitmap;
			hBitmap.CreateCompatibleBitmap(dc,cx,cy);
			hdcMem.SelectObject(hBitmap);

			hdcMem.BitBlt(r.left,r.top,cx,cy,dc,0,0,SRCCOPY);
			if(!DrawMode)
			{
				hdcMem.BitBlt(r.left,r.top,cx,cy,&dcBmp,0,0,SRCINVERT);
				hdcMem.BitBlt(r.left,r.top,cx,cy,&dcMask,0,0,SRCAND);
				hdcMem.BitBlt(r.left,r.top,cx,cy,&dcBmp,0,0,SRCINVERT);
			}
			else
			{
				int bx=GetBitmapWidth(hbmp);
				int by=GetBitmapHeight(hbmp);
				hdcMem.StretchBlt(r.left,r.top,cx,cy,&dcBmp,0,0,bx,by,SRCINVERT);
				hdcMem.StretchBlt(r.left,r.top,cx,cy,&dcMask,0,0,bx,by,SRCAND);
				hdcMem.StretchBlt(r.left,r.top,cx,cy,&dcBmp,0,0,bx,by,SRCINVERT);
			}
			dc->BitBlt(r.left,r.top,cx,cy,&hdcMem,0,0,SRCCOPY);

			hdcMem.DeleteDC();
			hBitmap.DeleteObject();

			DeleteDC(dcMask);
		}
		else
		{
			if(!DrawMode)
			{
				dc->BitBlt(r.left,r.top,cx,cy,&dcBmp,0,0,SRCCOPY);
			}
			else
			{
				int bx=GetBitmapWidth(hbmp);
				int by=GetBitmapHeight(hbmp);
				dc->StretchBlt(r.left,r.top,cx,cy,&dcBmp,0,0,bx,by,SRCCOPY);
			}
		}
		DeleteDC(dcBmp);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::DrawBitmap"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : FillWithBitmap                                                     
*  Description    : Fill with bitmap
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0236
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::FillWithBitmap(CDC* dc, HBITMAP hbmp, RECT r)
{
	try
	{
		if(!hbmp)return;
		CDC memdc;
		memdc.CreateCompatibleDC(dc);
		memdc.SelectObject(hbmp);
		int w = r.right - r.left;
		int	h = r.bottom - r.top;
		int x,y,z;
		int	bx=GetBitmapWidth(hbmp);
		int	by=GetBitmapHeight(hbmp);
		for (y = r.top; y < h; y += by)
		{
			if((y+by) >h)by=h-y;
			z=bx;
			for (x = r.left; x < w; x += z)
			{
				if((x+z) >w)z=w-x;
				dc->BitBlt(x, y, z, by, &memdc, 0, 0, SRCCOPY);
			}
		}
		DeleteDC(memdc);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::FillWithBitmap"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : SetSkin                                                     
*  Description    : Set skin for normal,hover,disable function
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0237
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::SetSkin(UINT normal,UINT down,UINT over,UINT disabled, UINT focus,UINT mask,
						   short drawmode, short border, short margin)
{
	try
	{
		m_bNormal.DeleteObject();	//free previous allocated bitmap
		m_bDown.DeleteObject();
		m_bOver.DeleteObject();
		m_bDisabled.DeleteObject();
		m_bMask.DeleteObject();
		m_bFocus.DeleteObject();

		if(normal>0)m_bNormal.LoadBitmap(normal);
		if(down>0)	  m_bDown.LoadBitmap(down);
		if(over>0)	  m_bOver.LoadBitmap(over);
		if(focus>0) m_bFocus.LoadBitmap(focus);

		if(disabled>0)m_bDisabled.LoadBitmap(disabled);
		else if(normal>0)m_bDisabled.LoadBitmap(normal);

		m_DrawMode=max(0,min(drawmode,2));
		m_Border=border;
		m_FocusRectMargin=max(0,margin);

		if(mask>0)
		{
			m_bMask.LoadBitmap(mask);
			if(hClipRgn)DeleteObject(hClipRgn);
			hClipRgn = CreateRgnFromBitmap(m_bMask,RGB(255,255,255));
			if(hClipRgn)
			{
				SetWindowRgn(hClipRgn, TRUE);
				SelectClipRgn((HDC)GetDC(),hClipRgn);
			}
			if(m_DrawMode == 0)
			{
				SetWindowPos(NULL,0,0,GetBitmapWidth(m_bMask),
					GetBitmapHeight(m_bMask),SWP_NOZORDER|SWP_NOMOVE);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::SetSkin"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : SetSkin                                                     
*  Description    : Set skin for normal,hover,disable function
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0238
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::SetSkin(HMODULE hinstDLL, UINT normal,UINT down,UINT over,UINT disabled, UINT focus,UINT mask,
						   short drawmode, short border, short margin)
{
	try
	{
		m_bNormal.DeleteObject();	//free previous allocated bitmap
		m_bDown.DeleteObject();
		m_bOver.DeleteObject();
		m_bDisabled.DeleteObject();
		m_bMask.DeleteObject();
		m_bFocus.DeleteObject();


		if(normal>0)
		{
			HANDLE hBmp = LoadBitmap(hinstDLL, MAKEINTRESOURCE(normal)); 
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bNormal.Attach(hBmp);
		}
		if(down>0)	  
		{
			HANDLE hBmp = LoadBitmap(hinstDLL, MAKEINTRESOURCE(down)); 
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bDown.Attach(hBmp);
			//m_bDown.LoadBitmap(down);
		}
		if(over>0)
		{
			HANDLE hBmp = LoadBitmap(hinstDLL, MAKEINTRESOURCE(over)); 
			
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bOver.Attach(hBmp);
			//m_bOver.LoadBitmap(over);
		}
		if(focus>0) 
		{
			//HANDLE hBmp = LoadImage(hinstDLL,focus,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE);
			HANDLE hBmp = LoadBitmap(hinstDLL, MAKEINTRESOURCE(focus)); 
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bFocus.Attach(hBmp);
			//m_bFocus.LoadBitmap(focus);
		}

		if(disabled>0)
		{
			//HANDLE hBmp = LoadImage(hinstDLL,disabled,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE);
			HANDLE hBmp = LoadBitmap(hinstDLL, MAKEINTRESOURCE(disabled));
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bDisabled.Attach(hBmp);
			//m_bDisabled.LoadBitmap(disabled);
		}
		else if(normal>0)
		{
			//HANDLE hBmp = LoadImage(hinstDLL,normal,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE);
			HANDLE hBmp = LoadBitmap(hinstDLL, MAKEINTRESOURCE(normal));
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bDisabled.Attach(hBmp);
			//m_bDisabled.LoadBitmap(normal);
		}

		m_DrawMode=max(0,min(drawmode,2));
		m_Border=border;
		m_FocusRectMargin=max(0,margin);

		if(mask>0)
		{
			HANDLE hBmp = LoadBitmap(hinstDLL, MAKEINTRESOURCE(mask)); 
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bMask.Attach(hBmp);

			//m_bMask.LoadBitmap(mask);
			if(hClipRgn)DeleteObject(hClipRgn);
			hClipRgn = CreateRgnFromBitmap(m_bMask,RGB(255,255,255));
			if(hClipRgn)
			{
				SetWindowRgn(hClipRgn, TRUE);
				SelectClipRgn((HDC)GetDC(),hClipRgn);
			}
			if(m_DrawMode == 0)
			{
				SetWindowPos(NULL,0,0,GetBitmapWidth(m_bMask),
					GetBitmapHeight(m_bMask),SWP_NOZORDER|SWP_NOMOVE);
			}
		}
	}
	catch(...)
	{
	}
}

/***************************************************************************************************                    
*  Function Name  : SetSkin                                                     
*  Description    : Set skin for normal,hover,disable function
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0239
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::SetSkin(LPCTSTR normal,LPCTSTR down,LPCTSTR over,LPCTSTR disabled, LPCTSTR focus,LPCTSTR mask,
						   short drawmode, short border, short margin)
{
	try
	{
		m_bNormal.DeleteObject();	//free previous allocated bitmap
		m_bDown.DeleteObject();
		m_bOver.DeleteObject();
		m_bDisabled.DeleteObject();
		m_bMask.DeleteObject();
		m_bFocus.DeleteObject();


		if(normal>0)
		{
			HANDLE hBmp = LoadImage(NULL,normal,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE);
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bNormal.Attach(hBmp);
		}
		if(down>0)	  
		{
			HANDLE hBmp = LoadImage(NULL,down,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE);
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bDown.Attach(hBmp);
			//m_bDown.LoadBitmap(down);
		}
		if(over>0)
		{
			HANDLE hBmp = LoadImage(NULL,over,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE);
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bOver.Attach(hBmp);
			//m_bOver.LoadBitmap(over);
		}
		if(focus>0) 
		{
			HANDLE hBmp = LoadImage(NULL,focus,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE);
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bFocus.Attach(hBmp);
			//m_bFocus.LoadBitmap(focus);
		}

		if(disabled>0)
		{
			HANDLE hBmp = LoadImage(NULL,disabled,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE);
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bDisabled.Attach(hBmp);
			//m_bDisabled.LoadBitmap(disabled);
		}
		else if(normal>0)
		{
			HANDLE hBmp = LoadImage(NULL,normal,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE);
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bDisabled.Attach(hBmp);
			//m_bDisabled.LoadBitmap(normal);
		}

		m_DrawMode=max(0,min(drawmode,2));
		m_Border=border;
		m_FocusRectMargin=max(0,margin);

		if(mask>0)
		{
			HANDLE hBmp = LoadImage(NULL,mask,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE);
			if (hBmp == NULL)
			{
			  return ;
			}

			m_bMask.Attach(hBmp);

			//m_bMask.LoadBitmap(mask);
			if(hClipRgn)DeleteObject(hClipRgn);
			hClipRgn = CreateRgnFromBitmap(m_bMask,RGB(255,255,255));
			if(hClipRgn)
			{
				SetWindowRgn(hClipRgn, TRUE);
				SelectClipRgn((HDC)GetDC(),hClipRgn);
			}
			if(m_DrawMode == 0)
			{
				SetWindowPos(NULL,0,0,GetBitmapWidth(m_bMask),
					GetBitmapHeight(m_bMask),SWP_NOZORDER|SWP_NOMOVE);
			}
		}
	}
	catch(...)
	{
	}
}

/***************************************************************************************************                    
*  Function Name  : CreateRgnFromBitmap                                                     
*  Description    : Create rectangle from bitmap
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0240
*  Date           : 18 Oct 2013
****************************************************************************************************/
HRGN CxSkinButton::CreateRgnFromBitmap(HBITMAP hBmp, COLORREF color)
{
	try
	{
		if(!hBmp)
			return NULL;

		BITMAP bm;
		GetObject(hBmp, sizeof(BITMAP), &bm);	// get bitmap attributes

		CDC dcBmp;
		dcBmp.CreateCompatibleDC(GetDC());	//Creates a memory device context for the bitmap
		dcBmp.SelectObject(hBmp);			//selects the bitmap in the device context

		const DWORD RDHDR = sizeof(RGNDATAHEADER);
		const DWORD MAXBUF = 40;		// size of one block in RECTs
		// (i.e.MAXBUF*sizeof(RECT)in bytes)
		LPRECT	pRects;
		DWORD	cBlocks = 0;			// number of allocated blocks

		INT		i, j;					// current position in mask image
		INT		first = 0;				// left position of current scan line
		// where mask was found
		bool	wasfirst = false;		// set when if mask was found in current scan line
		bool	ismask;					// set when current color is mask color

		// allocate memory for region data
		RGNDATAHEADER* pRgnData = (RGNDATAHEADER*)new BYTE[ RDHDR + ++cBlocks * MAXBUF * sizeof(RECT)];
		memset(pRgnData, 0, RDHDR + cBlocks * MAXBUF * sizeof(RECT));
		// fill it by default
		pRgnData->dwSize	= RDHDR;
		pRgnData->iType		= RDH_RECTANGLES;
		pRgnData->nCount	= 0;
		for(i = 0; i < bm.bmHeight; i++)
			for(j = 0; j < bm.bmWidth; j++)
			{
				// get color
				ismask=(dcBmp.GetPixel(j,bm.bmHeight-i-1) !=color);
				// place part of scan line as RECT region if transparent color found after mask color or
				// mask color found at the end of mask image
				if(wasfirst && ((ismask && (j == (bm.bmWidth-1))) ||(ismask ^ (j<bm.bmWidth))))
				{
					// get offset to RECT array if RGNDATA buffer
					pRects = (LPRECT)((LPBYTE)pRgnData + RDHDR);
					// save current RECT
					pRects[ pRgnData->nCount++]= CRect(first, bm.bmHeight - i - 1, j+(j == (bm.bmWidth-1)), bm.bmHeight - i);
					// if buffer full reallocate it
					if(pRgnData->nCount >= cBlocks * MAXBUF)
					{
						LPBYTE pRgnDataNew = new BYTE[ RDHDR + ++cBlocks * MAXBUF * sizeof(RECT)];
						memcpy(pRgnDataNew, pRgnData, RDHDR + (cBlocks - 1)* MAXBUF * sizeof(RECT));

						if(pRgnData)
						{
							delete[] pRgnData;
							pRgnData = NULL;
						}
						pRgnData = (RGNDATAHEADER*)pRgnDataNew;
					}
					wasfirst = false;
				}
				else if(!wasfirst && ismask)
				{		// set wasfirst when mask is found
					first = j;
					wasfirst = true;
				}
			}
			dcBmp.DeleteDC();	//release the bitmap

			// create region
			HRGN hRgn= CreateRectRgn(0, 0, 0, 0);
			ASSERT(hRgn!=NULL);
			pRects = (LPRECT)((LPBYTE)pRgnData + RDHDR);
			for(i=0;i<(int)pRgnData->nCount;i++)
			{
				HRGN hr=CreateRectRgn(pRects[i].left, pRects[i].top, pRects[i].right, pRects[i].bottom);
				VERIFY(CombineRgn(hRgn, hRgn, hr, RGN_OR) !=ERROR);
				if(hr)DeleteObject(hr);
			}
			ASSERT(hRgn!=NULL);

			if(pRgnData)
			{
				delete[] pRgnData;
				pRgnData = NULL;
			}
			return hRgn;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::CreateRgnFromBitmap"), 0, 0, true, SECONDLEVEL);
	}
	return CreateRectRgn(0, 0, 0, 0);
}

/***************************************************************************************************                    
*  Function Name  : SetTextColor                                                     
*  Description    : Set text color
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0241
*  Date           : 18 Oct 2013
****************************************************************************************************/
COLORREF CxSkinButton::SetTextColor(COLORREF new_color)
{
	try
	{
		COLORREF tmp_color = m_TextColor;
		m_TextColor = new_color;
		return tmp_color;			//returns the previous color
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::SetTextColor"), 0, 0, true, SECONDLEVEL);
	}
	return m_TextColor;
}

/***************************************************************************************************                    
*  Function Name  : SetFocusTextColor                                                     
*  Description    : Set focus color for button text.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0242
*  Date           : 18 Oct 2013
****************************************************************************************************/
COLORREF CxSkinButton::SetFocusTextColor(COLORREF new_color)
{
	try
	{
		COLORREF tmp_color = m_FocusTextColor;
		m_FocusTextColor = new_color;
		return tmp_color;			//returns the previous color
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::SetFocusTextColor"), 0, 0, true, SECONDLEVEL);
	}
	return m_FocusTextColor;
}

/***************************************************************************************************                    
*  Function Name  : SetTextColorA                                                     
*  Description    : Set button text color for normal,hover
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0243
*  Date           : 18 Oct 2013
****************************************************************************************************/
COLORREF CxSkinButton::SetTextColorA(COLORREF normal_color,COLORREF down_color,COLORREF over_color)
{
	try
	{
		COLORREF tmp_color = m_TextColor;
		m_TextColor=normal_color;
		if(down_color != 0)
		{
			m_DownTextColor = down_color;
		}
		if(over_color != 0)
		{
			m_OverTextColor = over_color;
		}
		return tmp_color;			//returns the previous color
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::SetTextColorA"), 0, 0, true, SECONDLEVEL);
	}
	return m_TextColor;
}

/***************************************************************************************************                    
*  Function Name  : SetToolTipText                                                     
*  Description    : Set tool tip for button text
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0244
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::SetToolTipText(CString s)
{
	try
	{
		if(m_tooltip.m_hWnd == NULL)
		{
			if(m_tooltip.Create(this))	//first assignment
				if(m_tooltip.AddTool(this, (LPCTSTR)s))
					m_tooltip.Activate(1);
		}
		else
		{
			m_tooltip.UpdateTipText((LPCTSTR)s,this);
		}
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::SetToolTipText"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : RelayEvent                                                     
*  Description    : Passes a mouse message to a tool tip control for processing.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0245
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::RelayEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	// This function will create a MSG structure, fill it in a pass it to
	// the ToolTip control, m_ttip. Note that we ensure the point is in window
	// coordinates (relative to the control's window).
	try
	{
		if(NULL != m_tooltip.m_hWnd)
		{
			MSG msg;
			msg.hwnd = m_hWnd;
			msg.message = message;
			msg.wParam = wParam;
			msg.lParam = lParam;
			msg.time = 0;
			msg.pt.x = LOWORD(lParam);
			msg.pt.y = HIWORD(lParam);

			m_tooltip.RelayEvent(&msg);
		}

	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::RelayEvent"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : OnLButtonDblClk                                                     
*  Description    : The framework calls this member function when the user double-clicks the left mouse button.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0246
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::OnLButtonDblClk(UINT flags, CPoint point)
{
	try
	{
		SendMessage(WM_LBUTTONDOWN, flags, MAKELPARAM(point.x, point.y));
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::OnLButtonDblClk"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : OnLButtonDown                                                     
*  Description    : The framework calls this member function when the user presses the left mouse button.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0247
*  Date           : 18 Oct 2013
* Modified		  : Lalit-4-13-2015,handle drag& drop radio button, both radio button getting select
****************************************************************************************************/
void CxSkinButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	try
	{	
		// resolved by lalit 4-13-2015
		//In Data Encryption->When we click continuously->Encrypt & Decrypt radio buttons are getting selected at same time.
		if (m_Style)
		{ 
			//track mouse for radio & check buttons
			POINT p2 = point;
			::ClientToScreen(m_hWnd, &p2);
			HWND mouse_wnd = ::WindowFromPoint(p2);
			if (mouse_wnd == m_hWnd)
			{ // mouse is in button
				
				if (m_Style == BS_RADIOBUTTON)
				{
					m_bBtnRadio_down = true;
				}
			}
		}
		//Pass this message to the ToolTip control
		RelayEvent(WM_LBUTTONDOWN,(WPARAM)nFlags,MAKELPARAM(LOWORD(point.x),LOWORD(point.y)));

		//If we are tracking this button, cancel it
		if(m_tracking){
			TRACKMOUSEEVENT t = {
				sizeof(TRACKMOUSEEVENT),
				TME_CANCEL | TME_LEAVE,
				m_hWnd,
				0
			};
			if(::_TrackMouseEvent(&t)){
				m_tracking = false;
			}
		}

		//Default-process the message
		CButton::OnLButtonDown(nFlags, point);
		if(m_pstBoost)
			m_pstBoost->Invalidate();

		m_button_down = true;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::OnLButtonDown"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : OnLButtonUp                                                     
*  Description    : The framework calls this member function when the user releases the left mouse button.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0248
*  Date           : 18 Oct 2013
* Modified		  : Lalit-4-13-2015,handle drag& drop radio button, both radio button getting select
****************************************************************************************************/
void CxSkinButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	try
	{
		if(m_Style)
		{ //track mouse for radio & check buttons
			POINT p2 = point;
			::ClientToScreen(m_hWnd, &p2);
			HWND mouse_wnd = ::WindowFromPoint(p2);
		   // resolved by lalit 4-13-2015
	    	//In Data Encryption->When we click continuously->Encrypt & Decrypt radio buttons are getting selected at same time.
			if (mouse_wnd == m_hWnd)
			{ 
				// mouse is in button
				if (m_Style == BS_CHECKBOX)SetCheck(m_Checked ? 0 : 1);
				if (m_Style == BS_RADIOBUTTON)
				{
					if (m_bBtnRadio_down)
					{
						SetCheck(1);
					}
				}
				
			}
		}
		//Pass this message to the ToolTip control
		RelayEvent(WM_LBUTTONUP,(WPARAM)nFlags,MAKELPARAM(LOWORD(point.x),LOWORD(point.y)));

		//Default-process the message
		m_button_down = false;
		m_bBtnRadio_down = false;
		CButton::OnLButtonUp(nFlags, point);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::OnLButtonUp"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : OnMouseMove                                                     
*  Description    : The framework calls this member function when the mouse cursor moves.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0249
*  Date           : 18 Oct 2013
* Modified		  : Lalit-4-13-2015,handle drag& drop radio button, both radio button getting select
****************************************************************************************************/
void CxSkinButton::OnMouseMove(UINT nFlags, CPoint point)
{
	try
	{
		//TRACE("* %08X: Mouse\n", ::GetTickCount());

		//Pass this message to the ToolTip control
		RelayEvent(WM_MOUSEMOVE,(WPARAM)nFlags,MAKELPARAM(LOWORD(point.x),LOWORD(point.y)));

		//If we are in capture mode, button has been pressed down
		//recently and not yet released - therefore check is we are
		//actually over the button or somewhere else.If the mouse
		//position changed considerably (e.g.we moved mouse pointer
		//from the button to some other place outside button area)
		//force the control to redraw
		//
		if((m_button_down) && (::GetCapture() == m_hWnd))
		{
			POINT p2 = point;
			::ClientToScreen(m_hWnd, &p2);
			HWND mouse_wnd = ::WindowFromPoint(p2);

			bool pressed = ((GetState()& BST_PUSHED) == BST_PUSHED);
			bool need_pressed = (mouse_wnd == m_hWnd);
			if(pressed != need_pressed)
			{
				SetState(need_pressed ? TRUE : FALSE);
				Invalidate();
				if(m_pstBoost)
					m_pstBoost->Invalidate();
			}
		// resolved by lalit 4-13-2015
		//In Data Encryption->When we click continuously->Encrypt & Decrypt radio buttons are getting selected at same time.

			if (mouse_wnd == m_hWnd)
			{
				if (m_Style != BS_RADIOBUTTON)
				{
					m_bBtnRadio_down = false;
				}
			}

		} else {

			//Otherwise the button is released.That means we should
			//know when we leave its area - and so if we are not tracking
			//this mouse leave event yet, start now!
			//
			if(!m_tracking)
			{
				TRACKMOUSEEVENT t =
				{
					sizeof(TRACKMOUSEEVENT),
					TME_LEAVE,
					m_hWnd,
					0
				};
				if(::_TrackMouseEvent(&t))
				{
					m_tracking = true;
					Invalidate();
					if(m_pstBoost)
						m_pstBoost->Invalidate();

				}
			}
		}


		//Forward this event to superclass
		CButton::OnMouseMove(nFlags, point);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::OnMouseMove"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : OnMouseLeave                                                     
*  Description    : The framework calls this member function when the cursor leaves
					the client area of the window specified in a prior call to TrackMouseEvent.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0250
*  Date           : 18 Oct 2013
****************************************************************************************************/
LRESULT CxSkinButton::OnMouseLeave(WPARAM, LPARAM)
{
	try
	{
		//ASSERT (m_tracking);
		m_tracking = false;
		Invalidate();
		if(m_pstBoost)
			m_pstBoost->Invalidate();


		return 0;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::OnMouseLeave"), 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : OnKillFocus                                                     
*  Description    : The framework calls this member function immediately before losing the input focus.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0251
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::OnKillFocus(CWnd *new_wnd)
{

	try
	{
		if(::GetCapture() == m_hWnd){
			::ReleaseCapture();
			ASSERT (!m_tracking);
			m_button_down = false;
		}
		CButton::OnKillFocus(new_wnd);
		if(m_pstBoost)
			m_pstBoost->Invalidate();

	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::OnKillFocus"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : OnSetFocus                                                     
*  Description    : The framework calls this member function after gaining the input focus.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0252
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::OnSetFocus(CWnd *new_wnd)
{

	try
	{
		CButton::OnSetFocus(new_wnd);
		if(m_pstBoost)
			m_pstBoost->Invalidate();

	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::OnSetFocus"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : OnClicked                                                     
*  Description    : handler for button clicked.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0253
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CxSkinButton::OnClicked()
{
	try
	{
		if(::GetCapture() == m_hWnd)
		{
			::ReleaseCapture();
			ASSERT (!m_tracking);
		}
		m_button_down = false;
		return FALSE;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::OnClicked"), 0, 0, true, SECONDLEVEL);
	}
	return FALSE;
}

/***************************************************************************************************                    
*  Function Name  : OnRadioInfo                                                     
*  Description    : On Radio Info
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0254
*  Date           : 18 Oct 2013
****************************************************************************************************/
LRESULT CxSkinButton::OnRadioInfo(WPARAM wparam, LPARAM)
{
	try
	{
		if(m_Checked){	//only checked buttons need to be unchecked
			m_Checked = false;
			Invalidate();
			if(m_pstBoost)
				m_pstBoost->Invalidate();

		}
		return 0;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::OnRadioInfo"), 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : OnKeyDown                                                     
*  Description    : The framework calls this member function when a nonsystem key is pressed.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0255
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	try
	{
		if((m_Style) &&(nChar == ' ')){ //needed stuff for check & radio buttons
			if(m_Style == BS_CHECKBOX)SetCheck(m_Checked ? 0 : 1);
			if(m_Style == BS_RADIOBUTTON)SetCheck(1);
		}

		CButton::OnKeyDown(nChar, nRepCnt, nFlags);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::OnKeyDown"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : OnBMSetCheck                                                     
*  Description    : Sets the check state of a radio button or check box.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0256
*  Date           : 18 Oct 2013
****************************************************************************************************/
LRESULT CxSkinButton::OnBMSetCheck(WPARAM wparam, LPARAM)
{
	try
	{
		m_Checked=wparam!=0;
		switch (m_Style)
		{
		case BS_RADIOBUTTON:
			if(m_Checked){ //uncheck the other radio buttons (in the same group)
				HWND hthis,hwnd2,hpwnd;
				hpwnd=GetParent() ->GetSafeHwnd();	//get button parent handle
				hwnd2=hthis=GetSafeHwnd();			//get this button handle
				if(hthis && hpwnd){				//consistency check
					for(;;){	//scan the buttons within the group
						hwnd2=::GetNextDlgGroupItem(hpwnd,hwnd2,0);
						//until we reach again this button
						if((hwnd2 == hthis) ||(hwnd2 == NULL))break;
						//post the uncheck message
						::PostMessage(hwnd2, WM_CXSHADE_RADIO, 0, 0);
					}
				}
			}
			break;
		case BS_PUSHBUTTON:
			m_Checked=false;
			ASSERT(false); // Must be a Check or Radio button to use this function
		}

		Invalidate();
		if(m_pstBoost)
			m_pstBoost->Invalidate();

		return 0;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CxSkinButton::OnBMSetCheck"), 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : OnBMGetCheck                                                     
*  Description    : Gets the check state of a radio button or check box.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0257
*  Date           : 18 Oct 2013
****************************************************************************************************/
LRESULT CxSkinButton::OnBMGetCheck(WPARAM wparam, LPARAM)
{
	return m_Checked; //returns the state for check & radio buttons
}

/***************************************************************************************************                    
*  Function Name  : SetTextAlignment                                                     
*  Description    : Specifies whether the text in the object is left-aligned, right-aligned, centered, or justified.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0258
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::SetTextAlignment(DWORD dwAlignment, bool bBottom)
{
	m_AlignStyle = dwAlignment;
	m_bBottom = bBottom;
}

/***************************************************************************************************                    
*  Function Name  : ShowMultilineText                                                     
*  Description    : Show Multiline text
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0259
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::ShowMultilineText(bool bMultiline)
{
	m_bShowMultiline = bMultiline;
}

/***************************************************************************************************                    
*  Function Name  : ShowToplineText                                                     
*  Description    : Show text at top line
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0260
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::ShowToplineText(bool bTopline)
{
	m_bShowTopline = bTopline;
}

/***************************************************************************************************                    
*  Function Name  : CreateFontStyle                                                     
*  Description    : Create font style
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0261
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CxSkinButton::CreateFontStyle(CFont& m_Font, CString csFontName, int iFontHeight, int iFontWidth, int iFontUnderline, int iFontWeight)
{
	try
	{

		LOGFONT lf;                        // Used to create the CFont.
		SecureZeroMemory(&lf, sizeof(LOGFONT));   // Clear out structure.
		lf.lfHeight = iFontHeight;
		lf.lfWidth  = iFontWidth;
		lf.lfUnderline = iFontUnderline;
		lf.lfWeight    = iFontWeight;
		wcscpy_s(lf.lfFaceName, LF_FACESIZE, csFontName);    //    with face name "Arial".
		m_Font.CreateFontIndirect(&lf);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CColorStatic::setFont"), 0, 0, true, SECONDLEVEL);
	}
}