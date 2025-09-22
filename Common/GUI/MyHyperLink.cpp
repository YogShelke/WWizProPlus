// MyHyperLink.cpp : implementation file
/***************************************************************************                      
   Program Name          : MyHyperLink.cpp
   Description           : Show text in hyperlink.
   Author Name           :                                                                         
   Date Of Creation      : 18th Oct,2013
   Version No            : 1.0.0.1
   Special Logic Used    : 
   Modification Log      :           
****************************************************************************/
#include "stdafx.h"
#include "MyHyperLink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************************************                    
*  Function Name  : CMyHyperLink                                                     
*  Description    : C'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0141
*  Date           : 18 Oct 2013
****************************************************************************************************/
CMyHyperLink::CMyHyperLink()
{
	m_sLinkColor = RGB(0, 0 ,255);
	m_sHoverColor = RGB(255, 0, 0);
	m_sVisitedColor = RGB(5, 34, 143);

	m_bFireChild = false;
	m_bMouseOver = false;
	m_bEnableToolTip = false;
	m_bVisited =  false;
	m_hResDLL = NULL;
	//Create Tooltip
}

/***************************************************************************************************                    
*  Function Name  : CMyHyperLink                                                     
*  Description    : Parameterized C'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0142
*  Date           : 18 Oct 2013
****************************************************************************************************/
CMyHyperLink::CMyHyperLink(HMODULE hResDLL)
{
	m_sLinkColor = RGB(0, 0 ,255);
	m_sHoverColor = RGB(255, 0, 0);
	m_sVisitedColor = RGB(5, 34, 143);

	m_bFireChild = false;
	m_bMouseOver = false;
	m_bEnableToolTip = false;
	m_bVisited =  false;
	m_hResDLL = hResDLL;
}

/***************************************************************************************************                    
*  Function Name  : ~CMyHyperLink                                                     
*  Description    : D'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0143
*  Date           : 18 Oct 2013
****************************************************************************************************/
CMyHyperLink::~CMyHyperLink()
{
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0144
*  Date           : 18 Oct 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CMyHyperLink, CStatic)
	//{{AFX_MSG_MAP(CMyHyperLink)
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//Sets the Link Color
/***************************************************************************************************                    
*  Function Name  : SetLinkColor                                                     
*  Description    : Sets the Link Color
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0145
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::SetLinkColor(COLORREF sLinkColor)
{
	m_sLinkColor = sLinkColor;

}

//open the URL by Windows ShellExecute()
/***************************************************************************************************                    
*  Function Name  : GoToLinkUrl                                                     
*  Description    : open the URL by Windows ShellExecute() 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0146
*  Date           : 18 Oct 2013
****************************************************************************************************/
bool CMyHyperLink::GoToLinkUrl(CString csLink)
{

	HINSTANCE hInstance = (HINSTANCE)ShellExecute(NULL, _T("open"), csLink.operator LPCTSTR(), NULL, NULL, 2);

	if ((UINT)hInstance < HINSTANCE_ERROR){
		return false;
	}else
		return true;
}

//User can Active/Inactive the Tooltip already they set 
/***************************************************************************************************                    
*  Function Name  : ActiveToolTip                                                     
*  Description    : User can Active/Inactive the Tooltip already they set 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0147
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::ActiveToolTip(int nFlag)
{
	if (nFlag)
		m_bEnableToolTip = true;
	else
		m_bEnableToolTip = false;
}

//change The Tooltip text
/***************************************************************************************************                    
*  Function Name  : SetTootTipText                                                     
*  Description    : change The Tooltip text
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0148
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::SetTootTipText(LPCTSTR szToolTip)
{
	if (m_bEnableToolTip )
	{
		m_ToolTip.UpdateTipText(szToolTip,this,1001);
	}

}

//The Mouse Move Message
/***************************************************************************************************                    
*  Function Name  : OnMouseMove                                                     
*  Description    : The Mouse Move Message
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0149
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::OnMouseMove(UINT nFlags, CPoint point) 
{
	CStatic::OnMouseMove(nFlags, point);

	if (m_bMouseOver)
	{
		CRect oRect;
		GetClientRect(&oRect);

		this->SetCursor(m_hHyperCursor);
	
		//check if the mouse is in the rect
		if (oRect.PtInRect(point) == false)
		{
			m_bMouseOver = false;
			//Release the Mouse capture previously take
			ReleaseCapture();
			RedrawWindow();
			return;
		}
	}else
	{
		m_bMouseOver = true;
		this->SetCursor(m_hHyperCursor);

		RedrawWindow();
		//capture the mouse
		SetCapture();
	}
}

//before Subclassing 
/***************************************************************************************************                    
*  Function Name  : PreSubclassWindow                                                     
*  Description    : before Subclassing 
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0150
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::PreSubclassWindow() 
{

	//Enable the Static to send the Window Messages To its parent
	DWORD dwStyle = GetStyle();
	SetWindowLong(GetSafeHwnd() ,GWL_STYLE ,dwStyle | SS_NOTIFY);

	TCHAR szCurretText[MAX_PATH];
	GetWindowText(szCurretText, MAX_PATH);
	if ((szCurretText) == NULL){
		SetWindowText(m_csLinkText.operator LPCTSTR());
	}
	
	//LOGFONT sLogFont;
	//GetFont()->GetLogFont(&sLogFont);
	//Set the Link UnderLined
	//sLogFont.lfUnderline = true;
	//Set the Font to  the Control
	//m_oTextFont.CreateFontIndirect(&sLogFont);
	//this->SetFont(&m_oTextFont, true);
	
	//Adjust the window
	//IsValidURL();

	//Set the Cursor Hand
	//WinHlp32.exe in windows folder ResourceID 106
	//is a standard window HAND cursor 
	//courtesy www.codeguru.com
	//you can use a custom Hand cursor resourse also
	// i added that  as a resourse in this project with 
	// ID - IDC_CURSOR_HAND
	m_hHyperCursor = LoadCursor(m_hResDLL, MAKEINTRESOURCE(IDC_CURSOR_HAND));
	if(m_hResDLL == NULL)
	{
		AddLogEntry(L"### Resource DLL handle not found returning..CMyHyperLink::PreSubclassWindow", 0, 0, true, SECONDLEVEL);
		return;
	}

	//TCHAR szWindowsDir[MAX_PATH*2];
	//GetWindowsDirectory(szWindowsDir ,MAX_PATH*2);
	//wcscat(szWindowsDir,L"\\Winhlp32.exe");
	//HMODULE hModule = LoadLibrary(szWindowsDir);

	//AfxGetResourceHandle();
	
	//m_hHyperCursor = ::LoadCursor(hModule,MAKEINTRESOURCE(IDC_CURSOR_HAND));
	//}
	//m_hHyperCursor = AfxGetApp()->LoadCursor(IDC_CURSOR_UNLOCK_HANDLE);
//	m_hHyperCursor = AfxGetApp()->LoadStandardCursor();
	
	
	if(m_hHyperCursor)
	this->SetCursor(m_hHyperCursor);
	else MessageBeep(3);
	

	////free the module
	//if (hModule)
	//	FreeLibrary(hModule);

	CStatic::PreSubclassWindow();
	this->SetCursor(m_hHyperCursor);
	
	m_ToolTip.Create(this,TTS_ALWAYSTIP);
	CRect oRect;
	GetClientRect(&oRect);
	m_ToolTip.AddTool(this,L"",oRect,1001);
	m_ToolTip.ShowWindow(SW_HIDE);
}

/***************************************************************************************************                    
*  Function Name  : SetLinkText                                                     
*  Description    : Sets link color
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0151
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::SetLinkText(CString csLinkText)
{
	m_csLinkText = csLinkText;
	this->SetWindowText(csLinkText.operator LPCTSTR());

}

/***************************************************************************************************                    
*  Function Name  : PreTranslateMessage                                                     
*  Description    : To translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0152
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CMyHyperLink::PreTranslateMessage(MSG* pMsg) 
{
	//m_ToolTip.RelayEvent(pMsg);
	return CStatic::PreTranslateMessage(pMsg);
}

/***************************************************************************************************                    
*  Function Name  : OnSetCursor                                                     
*  Description    : The framework calls this member function if mouse input is not captured and the mouse causes cursor movement within the CWnd object.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0153
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CMyHyperLink::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{

	::SetCursor(m_hHyperCursor);
	return true;
	//return CStatic::OnSetCursor(pWnd, nHitTest, message);
}

/***************************************************************************************************                    
*  Function Name  : OnClicked                                                     
*  Description    : On clicked on Hyperlink. It send message _HYPERLINK_EVENT
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0154
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::OnClicked() 
{
	if (m_bFireChild){
		//Fire the Event to Parent Window
		CWnd *pParent;
		pParent = GetParent();
		int nCtrlID = GetDlgCtrlID();
		::SendMessage(pParent->m_hWnd, _HYPERLINK_EVENT, (WPARAM)nCtrlID, 0);
		//::PostMessage(pParent->m_hWnd, __EVENT_ID_, (WPARAM)nCtrlID, 0);

	}else
	{
		GoToLinkUrl(m_csUrl);
	}

	m_bVisited = false;
	//reddraw the control 
	this->Invalidate(true);
}

/***************************************************************************************************                    
*  Function Name  : CtlColor                                                     
*  Description    : The framework calls this member function when a child control is about to be drawn.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0155
*  Date           : 18 Oct 2013
****************************************************************************************************/
HBRUSH CMyHyperLink::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	if (m_bMouseOver){
		if (m_bVisited)
			pDC->SetTextColor(m_sVisitedColor);
		else
			pDC->SetTextColor(m_sHoverColor);
	}else {
		if (m_bVisited)
			pDC->SetTextColor(m_sVisitedColor);
		else
			pDC->SetTextColor(m_sLinkColor);
	}
	pDC->SetBkMode(TRANSPARENT);
	return((HBRUSH)GetStockObject(NULL_BRUSH));
}

/***************************************************************************************************                    
*  Function Name  : SetToolTipTextColor                                                     
*  Description    : Sets a tool tip text color
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0156
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::SetToolTipTextColor(COLORREF sToolTipText) {
	m_ToolTip.SetTipTextColor(sToolTipText);
}

/***************************************************************************************************                    
*  Function Name  : SetToolTipBgColor                                                     
*  Description    : Sets a toll tip background color
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0157
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::SetToolTipBgColor(COLORREF sToolTipBgColor)
{
	m_ToolTip.SetTipBkColor(sToolTipBgColor);

}

/***************************************************************************************************                    
*  Function Name  : GetLinkText                                                     
*  Description    : Gets anchor/hyperlink text
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0158
*  Date           : 18 Oct 2013
****************************************************************************************************/
CString CMyHyperLink::GetLinkText()  {
	if (m_csLinkText.IsEmpty())
		return CString("");
	return m_csLinkText;
}

/***************************************************************************************************                    
*  Function Name  : SetLinkUrl                                                     
*  Description    : Set url to link
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0159
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::SetLinkUrl(CString csUrl) {
	m_csUrl= csUrl;
}

/***************************************************************************************************                    
*  Function Name  : GetLinkUrl                                                     
*  Description    : Get url
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0160
*  Date           : 18 Oct 2013
****************************************************************************************************/
CString CMyHyperLink::GetLinkUrl() {
	return m_csUrl;
}

/***************************************************************************************************                    
*  Function Name  : SetVisitedColor                                                     
*  Description    : Set color on click link
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0161
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::SetVisitedColor(COLORREF sVisitedColor) {
	m_sVisitedColor = sVisitedColor ;
}

/***************************************************************************************************                    
*  Function Name  : SetHoverColor                                                     
*  Description    : Sets color on hover link
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0162
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::SetHoverColor(COLORREF cHoverColor) {
	m_sHoverColor = cHoverColor;
}

/***************************************************************************************************                    
*  Function Name  : SetFireChild                                                     
*  Description    : Sets fire child
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0163
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::SetFireChild(int nFlag) {
	if (nFlag)
		m_bFireChild = true;
	else
		m_bFireChild = false;
}

/***************************************************************************************************                    
*  Function Name  : OnNotify                                                     
*  Description    : The framework calls this member function to inform the parent window of a control that an event has occurred in the control
					or that the control requires some kind of information.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0164
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CMyHyperLink::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	NMHDR* pMsgHdr;
	pMsgHdr = (NMHDR*) lParam;

	switch (pMsgHdr->code){
	case NM_RCLICK:
		break;
	default:
	;
	}
	
	return CStatic::OnNotify(wParam, lParam, pResult);
}

/***************************************************************************************************                    
*  Function Name  : SetResourceDllHandle                                                     
*  Description    : Sets wardwiz resource dll ha
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0165
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CMyHyperLink::SetResourceDllHandle(HMODULE hResDLL)
{
	if(hResDLL == NULL)
	{
		return;
	}
	m_hResDLL = hResDLL;
}