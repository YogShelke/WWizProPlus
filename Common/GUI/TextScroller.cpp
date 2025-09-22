// TextScroller.cpp : implementation file
//

#include "stdafx.h"
#include "TextScroller.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************************************      
*  Function Name  : CTextScroller                                                     
*  Description    : C'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0352
*  Date           : 18 Oct 2013
****************************************************************************************************/
CTextScroller::CTextScroller()
{
	m_CurrentY		= 0;
	m_bFirstTime	= TRUE;
	m_TextColor		= RGB(0,0,0);
	m_BkColor		= RGB(204,204,255);
	m_bDecreasePos	= TRUE;
	m_nHeight		= 0;
	m_nWidth		= 0;
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT),sizeof(m_lf),&m_lf);
	m_font.CreateFontIndirect(&m_lf);
}

/***************************************************************************************************  
*  Function Name  : CTextScroller                                                     
*  Description    : D'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0353
*  Date           : 18 Oct 2013
****************************************************************************************************/
CTextScroller::~CTextScroller()
{
	FreeList();
	m_font.DeleteObject();
}
/***************************************************************************************************  
*  Function Name  : BEGIN_MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0354
*  Date           : 18 Oct 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CTextScroller, CStatic)
	//{{AFX_MSG_MAP(CTextScroller)
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/***************************************************************************************************  
*  Function Name  : AddLine                                                     
*  Description    : Add Line for scrolling
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0355
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CTextScroller::AddLine(CStringArray &str)
{
	Line *_line = NULL;
	for(int ii =0 ;ii<str.GetSize();ii++)
	{
		_line = new Line;
		_line->strLine  = str.GetAt(ii);
		_line->pos	    = 0;
		_line->Initpos  = 0;
		_line->bDecrease= TRUE;
		m_Lines.AddTail(_line);
	}
	FormatLines();
	Invalidate();
}

/***************************************************************************************************  
*  Function Name  : PreSubclassWindow                                                     
*  Description    : Previous Sub class window is displayed.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0356
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CTextScroller::PreSubclassWindow() 
{
	CStatic::PreSubclassWindow();
	ModifyStyle(0,SS_BITMAP, SS_OWNERDRAW);
	ModifyStyle(0,WS_BORDER,SWP_DRAWFRAME);
}

/***************************************************************************************************      
*  Function Name  : OnPaint                                                     
*  Description    : The framework calls this member function when Windows or an application makes a 
					request to repaint a portion of an application's window.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0357
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CTextScroller::OnPaint() 
{
	KillTimer(0);
	CPaintDC dc(this); // device context for painting
	DrawBkGround(&dc);
	SetTimer(0,100,NULL);
}

/***************************************************************************************************      
*  Function Name  : DrawBkGround                                                     
*  Description    : Draw the background UI.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0358
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CTextScroller::DrawBkGround(CDC *pDC,CRect InvalidRect)
{
	CRect rectClient;
	GetClientRect(rectClient);
	if(InvalidRect == CRect(0,0,0,0))
		InvalidRect = rectClient;
	if(m_bFirstTime)
	{
		if(m_Lines.GetCount())
		{
			POSITION pos = m_Lines.GetHeadPosition();
			Line *_line = NULL;
			int offset = rectClient.bottom; 
			CFont *pOldFont = pDC->SelectObject(&m_font);
			CSize size = pDC->GetTextExtent(_T("TEST"));
			pDC->SelectObject(pOldFont);
			while(pos)
			{
				_line  = m_Lines.GetNext(pos);
				if(_line)
				{
					_line->pos	   = offset;
					_line->Initpos = offset;
					offset += size.cy + 1;
				}
			}
			m_bFirstTime = 0;
		}
	}
	pDC->FillSolidRect(&InvalidRect,m_BkColor);
}

/***************************************************************************************************      
*  Function Name  : OnTimer                                                     
*  Description    : The framework calls this member function after each interval specified in the SetTimer member 
					function used to install a timer.                                                    
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0359
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CTextScroller::OnTimer(UINT_PTR nIDEvent) 
{
	if(nIDEvent == 0)
	{
		MoveTextToTheTop();
	}
	CStatic::OnTimer(nIDEvent);
}

/***************************************************************************************************      
*  Function Name  : MoveTextToTheTop                                                     
*  Description    : Shift the Text to the top.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0360
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CTextScroller::MoveTextToTheTop()
{
	int count = m_Lines.GetCount();
	if(!count)
		return;
	CDC *pDC = GetDC();
	CRect rectClient;
	GetClientRect(rectClient);
	CFont *pOldFont = pDC->SelectObject(&m_font);
	int iOldMode    = pDC->SetBkMode(TRANSPARENT);
	COLORREF col    = pDC->SetTextColor(m_TextColor);
	CSize size = pDC->GetTextExtent(_T("TEST"));
	Line *_line = NULL;
	POSITION pos = m_Lines.GetHeadPosition();
	int nIndex = -1;
	while(pos)
	{
		 _line = m_Lines.GetNext(pos);
		 nIndex++;
		 if(!_line)
			 return;
		 if(_line->bDecrease)
			(_line->pos)--;
		if(_line->pos < 0)
		{
			DrawBkGround(pDC,CRect(0,_line->pos - 1,rectClient.right,_line->pos + size.cy - 1));
			_line->pos = _line->Initpos;
			if(nIndex == m_Lines.GetCount()-1)
			{
				pos = m_Lines.GetHeadPosition();
				while(pos)
				{
					_line = m_Lines.GetNext(pos);
					_line->bDecrease = TRUE;
				}
				break;
			}
			else
				_line->bDecrease = FALSE;
			continue;
		}
		if(_line->pos > rectClient.bottom - size.cy)
			continue;
		DrawBkGround(pDC,CRect(0,_line->pos - 1,rectClient.right,_line->pos + size.cy + 1));
		pDC->DrawText(_line->strLine,CRect(0,_line->pos,rectClient.right,_line->pos + size.cy),DT_CENTER);
	}
	pDC->SetBkMode(iOldMode);
	pDC->SelectObject(pOldFont);
	pDC->SetTextColor(col);
//Clean the top of the window
	DrawBkGround(pDC,CRect(0,0,rectClient.right,size.cy));
	ReleaseDC(pDC);
}

/***************************************************************************************************      
*  Function Name  : FormatLines                                                     
*  Description    : Get nbr character that can fit the client rectangle
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0361
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CTextScroller::FormatLines()
{
//Get nbr caractere that can fit the client rectangle
	CDC *pDC = GetDC();
	CFont *pOldFont = pDC->SelectObject(&m_font);
	CSize size		= pDC->GetTextExtent(L"T");
	pDC->SelectObject(pOldFont);
	CRect rectClient;
	GetClientRect(rectClient);
	int nbrCarPerLine = (rectClient.Width() / size.cx);
//format text now
	POSITION pos = m_Lines.GetHeadPosition();
	int posEsp   = 0;
	Line *_line  = NULL;
	CStringArray tmpArray;
	CString strLine;
	CString tmp;
	while(pos)
	{
		_line = m_Lines.GetNext(pos);
		strLine = _line->strLine;
		while(!strLine.IsEmpty())
		{
			posEsp = 0;
			strLine.TrimRight();
			if(nbrCarPerLine < strLine.GetLength() && strLine.GetAt(nbrCarPerLine) != ' ')
			{
				int Oldpos = 0;
				while(posEsp <= nbrCarPerLine  && posEsp != -1)
				{
					Oldpos = posEsp;
					posEsp = strLine.Find(' ',posEsp+1);
				}
				posEsp = Oldpos;
			}
			else
			{
				if(nbrCarPerLine < strLine.GetLength())
					posEsp = nbrCarPerLine;
				else
				{
					strLine += ' ';
					posEsp = strLine.GetLength()-1;
				}
			}
			tmp = strLine;
			if(posEsp)
				strLine.Delete(posEsp,strLine.GetLength()-posEsp);
			tmpArray.Add(strLine);
			tmp.Delete(0,posEsp+1);
			strLine = tmp;
		}
	}
	FreeList();
//fill the list with new strings
	for(int ii = 0;ii<tmpArray.GetSize();ii++)
	{
		_line = new Line;
		_line->strLine  = tmpArray.GetAt(ii);
		_line->pos	    = 0;
		_line->Initpos  = 0;
		_line->bDecrease= TRUE;
		m_Lines.AddTail(_line);
	}
}

/***************************************************************************************************      
*  Function Name  : FreeList                                                     
*  Description    : Frees the List.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0362
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CTextScroller::FreeList()
{
	POSITION pos = m_Lines.GetHeadPosition();
	Line *_line = NULL;
	while(pos)
	{
		_line = m_Lines.GetNext(pos);
		delete _line;
	}
	m_Lines.RemoveAll();
}

/***************************************************************************************************      
*  Function Name  : SetBkColor                                                     
*  Description    : Set BackGround List.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0363
*  Date           : 18 Oct 2013
****************************************************************************************************/
COLORREF CTextScroller::SetBkColor(COLORREF color)
{
	COLORREF OldCol = m_BkColor;
	m_BkColor = color;
	Invalidate();
	return OldCol;
}

/***************************************************************************************************      
*  Function Name  : SetScrollTimer                                                     
*  Description    : Set the scrolling timer.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0364
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CTextScroller::SetScrollTimer(int nElapse)
{
	if(nElapse >0)
	{
		KillTimer(0);
		SetTimer(0,nElapse,NULL);
	}
}

/***************************************************************************************************      
*  Function Name  : SetTextColor                                                     
*  Description    : Set the Text Color
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0365
*  Date           : 18 Oct 2013
****************************************************************************************************/
COLORREF CTextScroller::SetTextColor(COLORREF color)
{
	COLORREF OldCol = m_TextColor;
	m_TextColor = color;
	Invalidate();
	return OldCol;
}

/***************************************************************************************************      
*  Function Name  : SetBorder                                                     
*  Description    : Set The Border.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0366
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CTextScroller::SetBorder(BOOL bSet)
{
	if(bSet)
		ModifyStyle(0,WS_BORDER,SWP_DRAWFRAME);
	else
		ModifyStyle(WS_BORDER,0,SWP_DRAWFRAME);
}
/*
CString CTextScroller::SetFontName(CString strFont)
{
	CString OldFontName = _T(m_lf.lfFaceName);
	//strcpy(reinterpret_cast <char *>(m_lf.lfFaceName),const_cast <char *>(strFont));
	m_lf.lfFaceName = strFont;
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&m_lf);
	Invalidate();
	return OldFontName;
}*/

/***************************************************************************************************      
*  Function Name  : SetFontBold                                                     
*  Description    : Set the Font to Bold.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0367
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CTextScroller::SetFontBold(BOOL bIsBold)
{
	m_lf.lfWeight = bIsBold ? FW_BOLD : FW_NORMAL;
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&m_lf);
	Invalidate();
}

/***************************************************************************************************      
*  Function Name  : SetFontItalic                                                     
*  Description    : Set the Font to Bold.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0368
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CTextScroller::SetFontItalic(BOOL bItalic)
{
	m_lf.lfItalic = bItalic;
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&m_lf);
	Invalidate();
}

/***************************************************************************************************      
*  Function Name  : SetFontSize                                                     
*  Description    : Set the Font Size.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0369
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CTextScroller::SetFontSize(int nSize)
{
	nSize*=-1;
	m_lf.lfHeight = nSize;
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&m_lf);
	Invalidate();
}
