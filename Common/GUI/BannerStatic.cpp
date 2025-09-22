/***************************************************************************                      
   Program Name          : BannerStatic.cpp
   Description           : Scroll static text from left to right or right to left
						   with given speed.
   Author Name           : Ramkrushna Shelke
   Date Of Creation      : 18th Oct,2013
   Version No            : 1.0.0.1
   Special Logic Used    : 
   Modification Log      :           
****************************************************************************/
#include "stdafx.h"
#include "BannerStatic.h"

const int CBannerStatic::MAXSPEED(1000);  
const int CBannerStatic::MAXSPEED_MODIFIER(CBannerStatic::MAXSPEED*10);
const int CBannerStatic::TIMERRESOLUTION(100);
const int CBannerStatic::STEPHEIGHT(250);


/***************************************************************************************************                    
*  Function Name  : CBannerStatic                                                     
*  Description    : C'tor
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0271
*  Date           : 18 Oct 2013
****************************************************************************************************/
CBannerStatic::CBannerStatic() : CMultiColorStatic()
{
   m_tmScroll = 0;
   m_nTextOut = 0;
   SetWrapText(TRUE);
   SetScrollSpeed(0);
   SetScrollDelay(100);
   SetScrollSize(-1);
   m_hItemCursor = NULL;
   m_hStdCursor = NULL;
   m_pfnItemClick = NULL;
   m_TextColor = RGB(255,255,255);
   m_bMouseOver = false;
}

/***************************************************************************************************                    
*  Function Name  : CBannerStatic                                                     
*  Description    : D'tor
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0272
*  Date           : 18 Oct 2013
****************************************************************************************************/
CBannerStatic::~CBannerStatic()
{

}

/***************************************************************************************************                    
*  Function Name  : SetScrollSpeed                                                     
*  Description    : This function validates the incoming data; if it's good, then it will 
				    either start or stop the banner depending on the value.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0273
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::SetScrollSpeed(const int& nSpeed)
{
	try
	{

		if ((nSpeed < -MAXSPEED) || (nSpeed > MAXSPEED))
		{
			ASSERT(FALSE); // speed must fall in the above range
			return;
		}

		if (nSpeed == 0)
		{
			if (m_tmScroll)
			{
				timeKillEvent(m_tmScroll);
				m_tmScroll = 0;
			}
		}
		else
		{
			m_nBannerSpeed = nSpeed;
			CalculateScrollParameters();
			//Issue Resolved: 0000185:On UI security news is not moving. for Win8 & Win8.1 64bit Resolved By: Nitin K. Date 9th May 2015
			m_tmScroll = timeSetEvent(GetScrollDelay(), TIMERRESOLUTION, (LPTIMECALLBACK)TimerProc,
				(DWORD_PTR)this, TIME_CALLBACK_FUNCTION);

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CBannerStatic::SetScrollSpeed", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************                    
*  Function Name  : GetScrollSpeed                                                     
*  Description    : This just returns the current scroll speed.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0274
*  Date           : 18 Oct 2013
****************************************************************************************************/
int CBannerStatic::GetScrollSpeed(void) const
{
   return (m_nBannerSpeed);
}

/***************************************************************************************************                    
*  Function Name  : SetWrapText                                                     
*  Description    : This function modifies whether or not text should wrap when it has all 
				    scrolled off of the banner
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0275
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::SetWrapText(const BOOL& fWrapText)
{
   m_fWrapText = fWrapText;
}

/***************************************************************************************************                    
*  Function Name  : GetWrapText                                                     
*  Description    : This function returns the status of the WrapText flag.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0276
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CBannerStatic::GetWrapText(void) const
{
   return (m_fWrapText);
}

/***************************************************************************************************                    
*  Function Name  : SetItemCursor                                                     
*  Description    : Set cursor on static text
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0277
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::SetItemCursor(const UINT& unItemCursor)
{
	try
	{


		if (m_hItemCursor)
		{
			DestroyCursor(m_hItemCursor);
		}

		if (unItemCursor)
		{
			m_hItemCursor = LoadCursor(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(unItemCursor));
		}
		else
		{
			m_hItemCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CBannerStatic::SetItemCursor", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : SetItemClickProc                                                     
*  Description    : Set item clicked process.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0278
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::SetItemClickProc(PFNBANNERITEMCLICK pfnItemClick)
{
   m_pfnItemClick = pfnItemClick;
}

/***************************************************************************************************                    
*  Function Name  : SetScrollSize                                                     
*  Description    : This function sets the amount of pixels that the window should scroll 
				    on every tick of the scroll-timer. 
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0279
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::SetScrollSize(const int& nScrollSize)
{
   m_nScrollSize = nScrollSize;
}

/***************************************************************************************************                    
*  Function Name  : GetScrollSize                                                     
*  Description    : This function returns the amount of pixels that the window will scroll 
					on every scroll-timer tick.That is this function's intended purpose,anyway.
					Scrollwindow seems to be doing a stretchblt. If you use m_nScrollSize,
					the window will scroll faster when its height is large ... but it'll be wider as well.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0280
*  Date           : 18 Oct 2013
****************************************************************************************************/
int CBannerStatic::GetScrollSize(void) const
{
   return (GetScrollSpeed() > 0 ? -1 : 1);
//   return (m_nScrollSize);
}


/***************************************************************************************************                    
*  Function Name  : SetScrollDelay                                                     
*  Description    : This is the actual number of milliseconds that will occur between each 
					timer-tick. 
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0281
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::SetScrollDelay(const DWORD& dwScrollDelay)
{
   m_dwScrollDelay = dwScrollDelay;
}


/***************************************************************************************************                    
*  Function Name  : GetScrollDelay                                                     
*  Description    : This function will return the number of milliseconds between each 
					timer-tick. 
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0282
*  Date           : 18 Oct 2013
****************************************************************************************************/
DWORD CBannerStatic::GetScrollDelay(void) const
{
   return (m_dwScrollDelay);
}


/***************************************************************************************************                    
*  Function Name  : CalculateScrollParameters                                                     
*  Description    : This function uses m_nBannerSpeed to calculate m_dwScrollDelay and 
					m_nScrollSize
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0283
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::CalculateScrollParameters(void)
{
   //---------------------------------------------------------------------------
   // MAXSPEED_MODIFIER is currently just MAXSPEED * 10; this will result in
   // a scroll delay of no less than 10ms
   //
   m_dwScrollDelay = abs(MAXSPEED_MODIFIER / m_nBannerSpeed);

   m_nScrollSize = (m_nBannerSpeed >= 0 ? -1 : 1);

   int nStepHeight = STEPHEIGHT;
   m_nScrollSize *= ((m_rcBounds.Height() / nStepHeight)+1);
}


/***************************************************************************************************                    
*  Function Name  : FindStringFromPoint                                                     
*  Description    : This function returns the string that's at the point; this is useful 
				    for detecting mouse input so that the client can be notified as to 
				    which string was clicked on. 
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0284
*  Date           : 18 Oct 2013
****************************************************************************************************/
int CBannerStatic::FindStringFromPoint(CPoint point)
{
	try
	{
		for (int i = 0; i < static_cast<int>(m_vnStrings.size()); i++)
		{
			if (m_vnStrings.at(i) > point.x)
			{
				return (i - 1);
			}

			//--------------------------------------
			// make sure they're on ONLY the last
			// string; make sure you cut the range
			// off at its length
			//
			if (i == (m_vnStrings.size() - 1))
			{
				if ((m_vnStrings.at(i) < m_nTextOut + m_nTextLength) &&
					(point.x < (m_nTextOut + m_nTextLength)))
				{
					return (i);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CBannerStatic::FindStringFromPoint", 0, 0, true, SECONDLEVEL);
	}
   return (-1);
}

 
/***************************************************************************************************                    
*  Function Name  : MakeParentPoint                                                     
*  Description    : In the event that a mouse-message isn't handled, we have to pass it to 
					the parent. We can convert our point to an LPARAM via this function; we 
					use this function as the parameter for LPARAM in PostMessage 
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0285
*  Date           : 18 Oct 2013
****************************************************************************************************/
LPARAM CBannerStatic::MakeParentPoint(CPoint point)
{
	LPARAM lReturn = 0;
	try
	{
		CWnd* pParent = GetParent();
		ClientToScreen(&point);
		pParent->ScreenToClient(&point);

		lReturn = ((0x0000FFFF & point.x) | (0xFFFF0000 & (point.y << 16)));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CBannerStatic::FindStringFromPoint", 0, 0, true, SECONDLEVEL);
	}
   return (lReturn);
}


/***************************************************************************************************                    
*  Function Name  : ScrollBanner                                                     
*  Description    : This function scrolls the banner and then resets the timer so that the 
				    window may be scrolled again. 
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0286
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::ScrollBanner(void)
{
	try
	{
		//-----------------------------------------------------------
		// if GetScrollSize isn't 1 or -1, it's stretching the
		// text; no it shouldn't be doing this. GetScrollSize is 
		// just returning 1 or -1 for now until this is addressed
		//
		CPoint ptScroll;
		ptScroll.x = GetScrollSize();
		ptScroll.y = 0;

		//--------------------------------------------------
		// this was an attempt at point conversion in the
		// hopes that the stretching when scrolling by
		// multiple pixels problem would be solved here
		//
		CDC* pDC = GetDC();
		LPtoDP(pDC->m_hDC, &ptScroll, 1);
		pDC->DPtoLP(&ptScroll, 1);
		ReleaseDC(pDC);

		ScrollWindow(ptScroll.x, ptScroll.y, m_rcBounds, m_rcBounds);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CBannerStatic::ScrollBanner", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************                    
*  Function Name  : TimerProc                                                     
*  Description    :  Multimedia timers are used for better precision on Windows9x [which has 
				     55ms precision for message based timers]. We just use multimedia timers
				     for frequency of ticks; the message is still handled via the message queue.
				     That is, we just post a message telling the banner to scroll here; we don't
				     actually scroll it ourselves in this function. [which goes along with the
				    documentation that says not to do anything except message-posting and timeFuncs
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0287
*  Date           : 18 Oct 2013
****************************************************************************************************/
//Issue Resolved: 0000185:On UI security news is not moving. for Win8 & Win8.1 64bit Resolved By: Nitin K. Date 9th May 2015
void CALLBACK CBannerStatic::TimerProc(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2)
{
	__try
	{
		CBannerStatic* pBanner = (CBannerStatic*)dwUser;

		if (::IsWindow(pBanner->m_hWnd))
		{
			pBanner->PostMessage(WM_TIMER, uID, 0);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		//AddLogEntry(L"### Exception in CBannerStatic::TimerProc", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************                    
*  Function Name  : PreSubclassWindow                                                     
*  Description    : before subclass
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0288
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::PreSubclassWindow(void)
{
	try
	{
		CMultiColorStatic::PreSubclassWindow();

		SetScrollSpeed(0);
		m_hStdCursor = AfxGetApp()->LoadStandardCursor(IDC_HAND);
		m_hItemCursor = AfxGetApp()->LoadStandardCursor(IDC_HAND);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CBannerStatic::PreSubclassWindow", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0289
*  Date           : 18 Oct 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CBannerStatic, CMultiColorStatic)
   ON_WM_TIMER()
   ON_WM_PAINT()
   ON_WM_SIZE()
   ON_WM_LBUTTONDOWN()
   ON_WM_LBUTTONUP()
   ON_WM_MBUTTONDOWN()
   ON_WM_MBUTTONUP()
   ON_WM_RBUTTONDOWN()
   ON_WM_RBUTTONUP()
   ON_WM_SETCURSOR()
   ON_WM_MOUSEMOVE()
   ON_WM_DESTROY()
   ON_WM_CTLCOLOR()
   ON_WM_CTLCOLOR_REFLECT()
   ON_WM_MOUSEHOVER()
   ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

 
/***************************************************************************************************                    
*  Function Name  : OnTimer                                                     
*  Description    : The framework calls this member function after each interval specified in the SetTimer member function used to install a timer.                                                    
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0290
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::OnTimer(UINT_PTR nIDEvent)
{
	try
	{
		if (nIDEvent == m_tmScroll)
		{
			if (GetNumStrings() > 0)
			{
				if (GetWrapText())
				{
					//-----------------------------------------------------------
					// if text wraps in this control ... if it goes off the left
					// side, it'll start over on the right side again
					//
					if (GetScrollSpeed() > 0)
					{
						if ((--m_nTextOut + m_nTextLength) < m_rcBounds.left)
						{
							m_nTextOut = m_rcBounds.right;
						}
					}
					else if (GetScrollSpeed() < 0)
					{
						if ((++m_nTextOut) > m_rcBounds.right)
						{
							m_nTextOut = m_rcBounds.left - m_nTextLength;

							//------------------------------------------------------
							// on WINNT4 SP6 VC++ 6.0 SP3, this was necessary.
							// Nothing would ever scroll back around the next time
							//
							Invalidate();
						}
					}
				}

				ScrollBanner();
			}
			//Issue Resolved: 0000185:On UI security news is not moving. for Win8 & Win8.1 64bit Resolved By: Nitin K. Date 9th May 2015
			m_tmScroll = timeSetEvent(GetScrollDelay(), TIMERRESOLUTION, (LPTIMECALLBACK)TimerProc, (DWORD_PTR)this, TIME_CALLBACK_FUNCTION);
			return;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CBannerStatic::OnTimer", 0, 0, true, SECONDLEVEL);
	}
	CMultiColorStatic::OnTimer(nIDEvent);
}


/***************************************************************************************************                    
*  Function Name  : OnPaint                                                     
*  Description    : The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0291
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::OnPaint()
{
	try
	{
		CPaintDC dc(this);

		CRect rcBounds = m_rcBounds;
		rcBounds.left = m_nTextOut;

		//------------------------------------------------------
		// do the background and make sure that we don't
		// paint outside of our client area
		//
		dc.FillRect(m_rcBounds, &m_brBackGround);
		dc.IntersectClipRect(m_rcBounds);

		m_nTextLength = 0;
		m_vnStrings.clear();

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
			dc.SetTextColor(WHITE);
			//dc.SetTextColor(pstrCurrent->GetColor());
			dc.GetOutputTextMetrics(&stFontMetrics);

			GetTextExtentPoint32(dc.GetSafeHdc(), *pstrCurrent, pstrCurrent->GetLength(), &stSize);

			//---------------------------------------------------
			// do the drawing and update the position-dependent
			// stuff
			//	
			dc.DrawText(*pstrCurrent, rcBounds, DT_LEFT);
			rcBounds.left += stSize.cx + stFontMetrics.tmOverhang;
			m_vnStrings.push_back(m_nTextOut + m_nTextLength);
			m_nTextLength += stSize.cx + stFontMetrics.tmOverhang;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CBannerStatic::OnPaint", 0, 0, true, SECONDLEVEL);
	}
}

 
/***************************************************************************************************                    
*  Function Name  : OnSize                                                     
*  Description    : Vary size of item.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0292
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::OnSize(UINT nType, int cx, int cy)
{
   CMultiColorStatic::OnSize(nType, cx, cy);
   //CalculateScrollParameters();
   //Invalidate();
}

/***************************************************************************************************                    
*  Function Name  : OnLButtonDown                                                     
*  Description    : The framework calls this member function when the user presses the left mouse button.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0293
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::OnLButtonDown(UINT nFlags, CPoint point)
{
   int nString = FindStringFromPoint(point);
   if (nString != -1)
   {
      // do the callback
      if (m_pfnItemClick)
      {
         m_pfnItemClick(GetParent()->GetSafeHwnd(), nString, GetSafeHwnd());
         return;
      }
   }

   //---------------------------------------------------
   // this will get called if there is no callback
   //
   GetParent()->PostMessage(WM_LBUTTONDOWN, nFlags, MakeParentPoint(point));
}

/***************************************************************************************************                    
*  Function Name  : OnLButtonUp                                                     
*  Description    : The framework calls this member function when the user releases the left mouse button.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0294
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::OnLButtonUp(UINT nFlags, CPoint point)
{
   GetParent()->PostMessage(WM_LBUTTONUP, nFlags, MakeParentPoint(point));
}

/***************************************************************************************************                    
*  Function Name  : OnMButtonDown                                                     
*  Description    : The framework calls this member function when the user presses the middle mouse button.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0295
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::OnMButtonDown(UINT nFlags, CPoint point)
{
   GetParent()->PostMessage(WM_MBUTTONDOWN, nFlags, MakeParentPoint(point));
}

/***************************************************************************************************                    
*  Function Name  : OnMButtonUp                                                     
*  Description    : The framework calls this member function when the user releases the middle mouse button.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0296
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::OnMButtonUp(UINT nFlags, CPoint point)
{
   GetParent()->PostMessage(WM_MBUTTONUP, nFlags, MakeParentPoint(point));
}

/***************************************************************************************************                    
*  Function Name  : OnRButtonDown                                                     
*  Description    : The framework calls this member function when the user releases the right mouse button.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0297
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::OnRButtonDown(UINT nFlags, CPoint point)
{
   GetParent()->PostMessage(WM_RBUTTONDOWN, nFlags, MakeParentPoint(point));
}

/***************************************************************************************************                    
*  Function Name  : OnRButtonUp                                                     
*  Description    : The framework calls this member function when the user releases the right mouse button.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0298
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::OnRButtonUp(UINT nFlags, CPoint point)
{
   GetParent()->PostMessage(WM_RBUTTONUP, nFlags, MakeParentPoint(point));
}


/***************************************************************************************************                    
*  Function Name  : OnSetCursor                                                     
*  Description    : The framework calls this member function if mouse input is not captured and the mouse causes cursor movement within the CWnd object.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0299
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CBannerStatic::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
   if (FindStringFromPoint(m_ptCursor) != -1)
   {
      ::SetCursor(m_hItemCursor);
      return (TRUE);
   }
   else
   {
      ::SetCursor(m_hStdCursor);
      return (TRUE);
   }
}


/***************************************************************************************************                    
*  Function Name  : OnMouseMove                                                     
*  Description    : The Mouse Move Message
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0300
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::OnMouseMove(UINT nFlags, CPoint point)
{
	try
	{
		m_ptCursor = point;
		//	CRect rectNew(point.x-20, point.y-20, point.x+20, point.y+20);
		//CClientDC dc(this);
		// WM_MOUSEMOVE + !m_bMouseTracking becomes the equivalent of
		// WM_MOUSEENTER of which there is no such thing.
		if (!m_bMouseOver)
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = this->m_hWnd;
			if (::_TrackMouseEvent(&tme))
			{
				m_bMouseOver = TRUE;
				// Draw new rect, but no last rect as we are starting anew
				//dc.DrawDragRect(rectNew, CSize(2,2), NULL, CSize(0,0));
			}
		}
		else
		{
			// Draw new rect and erase old rect
			//dc.DrawDragRect(rectNew, CSize(2,2), m_rectLast, CSize(2,2));
		}
		// Remember where we drew this rectangle
		//m_rectLast = rectNew;
		SetScrollSpeed(0);
		CMultiColorStatic::OnMouseMove(nFlags, point);






		//if (m_bMouseOver)
		//{
		//	CRect oRect;
		//	GetClientRect(&oRect);

		//	this->SetCursor(m_hItemCursor);
		//
		//	//check if the mouse is in the rect
		//	if (oRect.PtInRect(point) == false)
		//	{
		//		m_bMouseOver = false;
		//		//Release the Mouse capture previously take
		//		ReleaseCapture();
		//		RedrawWindow();
		//		return;
		//	}
		//}else
		//{
		//	m_bMouseOver = true;
		//	this->SetCursor(m_hItemCursor);
		//	
		//	RedrawWindow();
		//	SetScrollSpeed(0);
		//	//capture the mouse
		//	SetCapture();
		//}
		//if(m_bMouseOver)
		//{
		//	SetScrollSpeed(0);
		//}
		//RedrawWindow();

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CBannerStatic::OnMouseMove", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************                    
*  Function Name  : OnDestroy                                                     
*  Description    : this to clean up any cursors that we've loaded; we don't want to 
				    needlessly drain resources. 
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0301
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::OnDestroy()
{
   if (m_hItemCursor)
   {
      DestroyCursor(m_hItemCursor);
   }

   if (m_hStdCursor)
   {
      DestroyCursor(m_hStdCursor);
   }
}


/***************************************************************************************************                    
*  Function Name  : SetTextColor                                                     
*  Description    : Sets Text Color
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0302
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::SetTextColor(const COLORREF& crColor )
{	
	try
	{
		m_TextColor = crColor;
		Invalidate ();
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CColorStatic::SetTextColor"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : OnCtlColor                                                     
*  Description    : The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0303
*  Date           : 18 Oct 2013
****************************************************************************************************/
HBRUSH CBannerStatic::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CBannerStatic::OnCtlColor(pDC, pWnd, nCtlColor);

	try
	{
		UNREFERENCED_PARAMETER(nCtlColor);
		pDC->SetTextColor (m_TextColor);
		//pDC->SetBkMode(m_clrBack);
		return hbr;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CColorStatic::OnCtlColor"), 0, 0, true, SECONDLEVEL);
	}
	
	return hbr;
}

/***************************************************************************************************                    
*  Function Name  : CtlColor                                                     
*  Description    : The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0304
*  Date           : 18 Oct 2013
****************************************************************************************************/
HBRUSH CBannerStatic::CtlColor(CDC* pDC, UINT nCtlColor)
{
	return NULL;
}

/***************************************************************************************************                    
*  Function Name  : OnMouseHover                                                     
*  Description    : The framework calls this member function when the cursor hovers over
					the client area of the window for the period of time specified in a prior call to TrackMouseEvent.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0305
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::OnMouseHover(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	
	CMultiColorStatic::OnMouseHover(nFlags, point);
}

/***************************************************************************************************                    
*  Function Name  : OnMouseLeave                                                     
*  Description    : The framework calls this member function when the cursor leaves the client area of the window specified in a prior call to TrackMouseEvent.
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZCOMMON_0306
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CBannerStatic::OnMouseLeave()
{
	//	CClientDC dc(this);

	//dc.DrawDragRect(CRect(0,0,0,0), CSize(0,0), m_rectLast, CSize(2,2));

	//m_rectLast = CRect(0,0,0,0);

	m_bMouseOver = FALSE;
	RedrawWindow();
	SetScrollSpeed(400);
	CMultiColorStatic::OnMouseLeave();
}
