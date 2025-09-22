// HTMLListCtrl.cpp : implementation file
/****************************************************
*  Program Name: HTMLListCtrl.cpp                                                                                                   
*  Description:  HTML list control with desire size for setting tab
		//
		//	Written by Manoj Subberwal (Monty) email : xMonty@gmail.com
		//	copyright 2005-2006
		
		// This code may be used in compiled form in any way you desire. This
		// file may be redistributed unmodified by any means PROVIDING it is 
		// not sold for profit without the authors written consent, and 
		// providing that this notice and the authors name is included. If 
		// the source code in  this file is used in any commercial application 
		// then acknowledgement must be made to the author of this file 
		// (in whatever form you wish).
		//
		// This file is provided "as is" with no expressed or implied warranty.
		// The author accepts no liability for any damage caused through use.
		//
		// Expect bugs.
		// 
		// Please use and enjoy. Please let me know of any bugs/mods/improvements 
		// that you have found/implemented and I will fix/incorporate them into this
		// file. 
		//
		/////////////////////////////////////////////////////////////////////////////
*  Author Name: Nitin Kolpkar                                                                                                      
*  Date Of Creation: 25 April 2014
*  Version No: 1.0.0.2
****************************************************/

#include "stdafx.h"
#include "HTMLListCtrl.h"
#include "memdc.h"
#include <shlwapi.h>
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "WardWizHomePage.h"
#include <afxmsg_.h>
#include "SettingsReports.h"
#include "SettingsScanTypeDlg.h"
#include "SettingsPassChangeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LPCTSTR g_GUIDofInstalledOutllookH[10]={L"11",L"12",L"14",L"15"};
/////////////////////////////////////////////////////////////////////////////
// CHTMLListCtrl

/***************************************************************************************************                    
*  Function Name  : CHTMLListCtrl                                                     
*  Description    : C'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
CHTMLListCtrl::CHTMLListCtrl()
{
	m_nTotalItems = 0;
	m_nListHeight = 0;
	m_nWndHeight = 0;
	m_nWndWidth = 0;
	m_pSettingsScanTypeDlg = NULL;


	m_nSelectedItem = NONE_SELECTED;
	//Create a light pen for border
	m_penLight.CreatePen(PS_SOLID,5,RGB(200,200,200));

	//Create the Font
	LOGFONT lf = {0};
	lf.lfHeight = -11;
	lf.lfWeight = FW_NORMAL;
	
	_tcscpy_s(lf.lfFaceName, L"Tahoma");
	m_font.CreateFontIndirect(&lf);

	m_clrBkSelectedItem = RGB(255,255,255);

	m_dwExtendedStyles = 0;

	m_bHasFocus = FALSE;

//	m_ImageList.Create(55,19,ILC_COLOR24|ILC_MASK,55,65);
	m_ImageList.Create(50,30,ILC_COLOR24|ILC_MASK,2,1);

	
	CBitmap bm;
	bm.LoadBitmap(IDB_BITMAP_SETTING_BTNOFF);

	m_ImageList.Add(&bm,RGB(255,0,255));

	m_pImageList = NULL;


	/*	ISSUE NO - 399	Settings tab:for reports & startup scan, drop down list needs to be provided to select timeline/scan type.
		NAME - NITIN K. TIME - 27st May 2014 */
	m_ImageListReports.Create(32,36,ILC_COLOR24|ILC_MASK,2,1);
	CBitmap bmReports;
	bmReports.LoadBitmap(IDB_BITMAP_BUTTON_SETTINGS);
	m_ImageListReports.Add(&bmReports,RGB(255,0,255));

	m_Key = L"SOFTWARE\\WardWiz Antivirus";
}

/***************************************************************************************************                    
*  Function Name  : ~CHTMLListCtrl                                                     
*  Description    : D'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
CHTMLListCtrl::~CHTMLListCtrl()
{
	//Name:Varada Ikhar, Date:06/01/2015, Issue No:9,	Version :1.8.3.4, 
	//Description: To disable the selection of startup scan type if it user off it from system try.
	if(m_pSettingsScanTypeDlg != NULL)
	{
		delete m_pSettingsScanTypeDlg;
		m_pSettingsScanTypeDlg = NULL;
	}
	DeleteAllItems();
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CHTMLListCtrl, CWnd)
	//{{AFX_MSG_MAP(CHTMLListCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CHTMLListCtrl message handlers
/***************************************************************************************************                    
*  Function Name  : Create                                                     
*  Description    : Creates html list control.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
BOOL CHTMLListCtrl::Create(CWnd *pParent, CRect rc, UINT nID,DWORD dwStyle)
{
	//Get a New Class Name
	CString sWindowClassName = ::AfxRegisterWndClass(CS_DBLCLKS); 

	//Try to create it with default styles
	if(!CWnd::Create(sWindowClassName,L"HTMLListCtrl",dwStyle,rc,pParent,nID))
	{
		return FALSE;
	}

	
	m_nControlID = nID;

	m_nWndWidth = rc.Width();
	m_nWndHeight = rc.Height();

	m_nListHeight = 0;

	//Set the scrolling info
	SCROLLINFO scrlinfo;
	scrlinfo.cbSize = sizeof(scrlinfo);

	scrlinfo.fMask = SIF_PAGE|SIF_RANGE;
	scrlinfo.nMax = 0;
	scrlinfo.nMin = 0;
	scrlinfo.nPage = 0;
	scrlinfo.nPos = 0;
	SetScrollInfo(SB_VERT,&scrlinfo);

	return TRUE;
}

/***************************************************************************************************                    
*  Function Name  : InsertItem                                                     
*  Description    : Insert item in html list control
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
int CHTMLListCtrl::InsertItem(CString sText, UINT uiImage, int nStyle, int nHeight)
{
	//Allocate memory
	HTMLLIST_ITEM *pItem = new HTMLLIST_ITEM;
	pItem->sItemText = sText;
	pItem->nStyle = nStyle;
	pItem->nItemNo = m_nTotalItems;
	pItem->uiImage = uiImage;
	
	if(nHeight == AUTO)
	{
		//Calculate items height
		pItem->nHeight = CalculateItemHeight(sText,nStyle,uiImage,m_nWndWidth);
		pItem->bHeightSpecified = FALSE;
	}
	else
	{
		pItem->nHeight = nHeight;
		pItem->bHeightSpecified = TRUE;
	}

	m_listItems.AddTail(pItem);
	
	m_nTotalItems ++;

	m_nListHeight += pItem->nHeight;

	SCROLLINFO scrlinfo;
	scrlinfo.cbSize = sizeof(scrlinfo);

	scrlinfo.fMask = SIF_PAGE|SIF_RANGE;
	scrlinfo.nMax = m_nListHeight;
	scrlinfo.nMin = 0;
	scrlinfo.nPage = m_nWndHeight;
	scrlinfo.nPos = 0;
	SetScrollInfo(SB_VERT,&scrlinfo);

	m_mapItems.SetAt(pItem->nItemNo,pItem);

	m_bOutlookFlag = false;
	if(m_nControlID == ID_SETTING_EMAIL)
	{
		if(!GetExistingPathofOutlook())
		{
			m_bOutlookFlag = true;
			makeRegistryEntryForSettingsTabOptions(ID_SETTING_EMAIL,1,true);
		}
		else
		{
			m_bOutlookFlag = false;
		}
	}

	//Issue No 658 If no of days  =0  email and update items in menu should be disable.
	// Neha Gharge 7 July,2015

	m_bDaysLeftZeroFlag = false;
	if (m_nControlID == ID_SETTING_EMAIL /*|| m_nControlID == ID_SETTING_UPDATE*/) //Ram: Commented as live update should happen in unregistered-product.
	{
		//Ram, Issue No: 0001216, Resolved
		if (theApp.m_dwDaysLeft == 0)
		{
			theApp.GetDaysLeft();
		}

		if (theApp.m_dwDaysLeft == 0)
		{
			m_bDaysLeftZeroFlag = true;
			makeRegistryEntryForSettingsTabOptions(ID_SETTING_EMAIL, 1, true);
			//makeRegistryEntryForSettingsTabOptions(ID_SETTING_UPDATE, 1, true);
		}
		else
		{
			m_bDaysLeftZeroFlag = false;
		}
	}
	//Fetching the Registry Values For Settiings Tab From Registry
	int ItemNo=pItem->nItemNo;
	DWORD value= ReadRegistryEntryofSetting(ItemNo);
	if(value==1)
	{
		pItem->bChecked = FALSE;
	}
	if(value==0)
	{
		pItem->bChecked = TRUE;
	}

	
	return (m_nTotalItems - 1);
}

/***************************************************************************************************                    
*  Function Name  : CalculateItemHeight                                                     
*  Description    : calculate height of each element in html list control.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
int CHTMLListCtrl::CalculateItemHeight(CString sText, int nStyle, UINT uiImage,int nItemWidth)
{
	int nImageWidth = 0;
	int nPadding = ITEM_PADDING_LEFT; //default space 

	if(m_dwExtendedStyles & HTMLLIST_STYLE_IMAGES)
	{
		if(m_pImageList)
		{
			if(m_pImageList->GetImageCount())
			{
				IMAGEINFO Info = {0};
				if(m_pImageList->GetImageInfo(uiImage,&Info))
				{
					nImageWidth = Info.rcImage.right - Info.rcImage.left;
					nImageWidth += (ITEM_IMAGE_PADDING_LEFT + ITEM_IMAGE_PADDING_RIGHT);
				}
			}
		}
	}

	if(m_dwExtendedStyles & HTMLLIST_STYLE_CHECKBOX)
	{
		nPadding += ITEM_PADDING_CHECKBOX_LEFT + ITEM_CHECKBOX_WIDTH ;
	}
	
	if(nStyle == NORMAL_TEXT)
	{
		CDC *pDC = GetDC();
	
		CFont *pOldFont = pDC->SelectObject(&m_font);

		CRect rc;
		rc.SetRectEmpty();

		rc.left = 0;

		rc.right = nItemWidth - nPadding;

		rc.right -= nImageWidth;

		pDC->DrawText(sText,&rc,DT_WORDBREAK|DT_CALCRECT|DT_EXTERNALLEADING );

		pDC->SelectObject(pOldFont);
		ReleaseDC(pDC);
		return rc.Height() + ITEM_PADDING_BOTTOM + ITEM_PADDING_TOP;
	}
	else if(nStyle == HTML_TEXT)
	{
		CDC *pDC = GetDC();
		
		CDC memDC;
		memDC.CreateCompatibleDC(pDC);

		CFont *pOldFont = memDC.SelectObject(&m_font);

		int nWidth = 0;

		nWidth = nItemWidth - nPadding;
		nWidth -= nImageWidth;

		CRect rc(0,0,nWidth,m_nWndHeight);
		
		int nHeight = DrawHTML(memDC.GetSafeHdc(),sText,sText.GetLength(),&rc,DT_LEFT|DT_CALCRECT|DT_WORDBREAK|DT_EXTERNALLEADING);

		memDC.SelectObject(pOldFont);
		ReleaseDC(pDC);
		return rc.Height() + ITEM_PADDING_BOTTOM + ITEM_PADDING_TOP;
	}
	else if(nStyle == SINGLE_LINE_TEXT)
	{
		CDC *pDC = GetDC();
	
		CFont *pOldFont = pDC->SelectObject(&m_font);

		CRect rc;
		rc.SetRectEmpty();

		rc.left = 0;

		rc.right = nItemWidth - nPadding;
		rc.right -= nImageWidth;

		pDC->DrawText(sText,&rc,DT_VCENTER|DT_CALCRECT|DT_SINGLELINE);

		pDC->SelectObject(pOldFont);
		ReleaseDC(pDC);
		return rc.Height() + ITEM_PADDING_BOTTOM + ITEM_PADDING_TOP;
	}
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : DeleteAllItems                                                     
*  Description    : Delete all html item and map items
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::DeleteAllItems()
{
	m_nListHeight = 0;
	m_nTotalItems = 0;

	POSITION pos = m_listItems.GetHeadPosition();
	for(int i = 0;i < m_listItems.GetCount();i++)
	{
		HTMLLIST_ITEM *pItem = m_listItems.GetNext(pos);
		delete pItem;
	}
	m_listItems.RemoveAll();
	m_mapItems.RemoveAll();
}

/***************************************************************************************************                    
*  Function Name  : GetItemData                                                     
*  Description    : Get item data from given position.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
LPARAM CHTMLListCtrl::GetItemData(int nPos)
{
	HTMLLIST_ITEM* pItem = GetInternalData(nPos);
	if(pItem)
		return pItem->lItemData;
	else
		return NULL;
}

/***************************************************************************************************                    
*  Function Name  : SetItemData                                                     
*  Description    : Set given item data into given position 
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::SetItemData(int nPos, LPARAM lItemData)
{
	HTMLLIST_ITEM* pItem = GetInternalData(nPos);
	if(pItem)
	{
		pItem->lItemData = lItemData;
	}
}

/***************************************************************************************************                    
*  Function Name  : OnPaint                                                     
*  Description    : The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CDC *pDC = &dc;
	//CMemDC pDC(&dc);

	CFont *pOldFont = pDC->SelectObject(&m_font);

	CRect rcWnd;
	GetClientRect(&rcWnd);

	CRect rcItem = rcWnd;
	rcItem.bottom = 0;
	
	int nScrollPos = GetScrollPos(SB_VERT);

	rcItem.OffsetRect(0,-nScrollPos);

	POSITION pos = m_listItems.GetHeadPosition();
	for(int i = 0;i < m_listItems.GetCount();i++)
	{
		HTMLLIST_ITEM *pItem = m_listItems.GetNext(pos);

		//Create the bounding rect for item
		rcItem.bottom = rcItem.top + pItem->nHeight;
		
		if(i == m_nSelectedItem)
		{
			DrawItem(pDC,rcItem,pItem,TRUE);
			/*//Call the virtual function for drawing the item
			CString ItemName=pItem->sItemText;
			DWORD value= ReadRegistryEntryofSetting(ItemName);
			if(value==1)
			{
				pItem->bChecked = TRUE;
				DrawItem(pDC,rcItem,pItem,TRUE);
			}
			if(value==0)
			{
				pItem->bChecked = FALSE;
				DrawItem(pDC,rcItem,pItem,FALSE);
			}*/
		}
		else
		{
			DrawItem(pDC,rcItem,pItem,FALSE);
			/*CString ItemName=pItem->sItemText;
			DWORD value= ReadRegistryEntryofSetting(ItemName);
			if(value==1)
			{
				pItem->bChecked = TRUE;
				DrawItem(pDC,rcItem,pItem,TRUE);
			}
			if(value==0)
			{
				pItem->bChecked = FALSE;
				DrawItem(pDC,rcItem,pItem,FALSE);
			}*/
		}

		pItem->rcItem = rcItem;
		//Move rcItem to next item
		rcItem.OffsetRect(0,pItem->nHeight);
	}

	//Release GDI stuff
	pDC->SelectObject(pOldFont);
}

/***************************************************************************************************                    
*  Function Name  : OnEraseBkgnd                                                     
*  Description    : Erasing background.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
BOOL CHTMLListCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

/***********************************************************************************************
*  Function Name  : OnLButtonDown
*  Description    : this function is  called every time when User Selects from any of the given option of Settings Tab
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 25 April 2014
***********************************************************************************************/
void CHTMLListCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();

	NM_HTMLLISTCTRL *pNMHDR = new NM_HTMLLISTCTRL;

	pNMHDR->hdr.code = HTMLLIST_LBUTTONDOWN;
	pNMHDR->hdr.hwndFrom = GetSafeHwnd();
	pNMHDR->hdr.idFrom = m_nControlID;

	pNMHDR->lItemData = 0;
	pNMHDR->nItemNo = -1;
	pNMHDR->sItemText = "";
	pNMHDR->bChecked = 0;
	//Send LButton down event first
	GetParent()->SendMessage(WM_NOTIFY,m_nControlID,(LPARAM)pNMHDR);

	delete pNMHDR;


	BOOL bItemSelected = FALSE;
	m_nSelectedItem = NONE_SELECTED;

	POSITION pos = m_listItems.GetHeadPosition();
	for(int i = 0;i < m_listItems.GetCount();i++)
	{
		HTMLLIST_ITEM *pItem = m_listItems.GetNext(pos);
		if(pItem->rcItem.PtInRect(point))
		{
			if(m_dwExtendedStyles & HTMLLIST_STYLE_CHECKBOX)
			{
				/*	ISSUE NO - 421 NAME - NITIN K. TIME - 26st May 2014 */
				CPoint pt = pItem->rcItem.TopLeft();
				CPoint ptSettingBtn = pItem->rcItem.TopLeft();
				/*	ISSUE NO - 536 NAME - NITIN K. TIME - 30th May 2014 */
				if( m_nControlID == ID_SETTING_GENERAL && theApp.m_dwProductID != BASIC)
				{
					//see if we clicked in the box
					pt = pItem->rcItem.TopLeft();
					pt.x += 368; /*here pt.x +=  is the starting point of ONOFF button;  
								 NOTE: WE HAD TO CHANGE THE X & Y CO-ORDINATES BECAUSE GENERAL TAB HAVNG MANY OPTIONS 
								 SO SCROLL BAR APPEARS SO NEED TO SHIFT THE CO-ORDINATES 12 POINTS*/
					pt.y += pItem->rcItem.Height() / 2 - 8;
				}
				else
				{
					//see if we clicked in the box
					pt = pItem->rcItem.TopLeft();
					pt.x += 380; //here pt.x +=  is the starting point of ONOFF button 
					pt.y += pItem->rcItem.Height() / 2 - 8;

				}
				/*	ISSUE NO - 399	Settings tab:for reports & startup scan, drop down list needs to be provided to select timeline/scan type.
				NAME - NITIN K. TIME - 27st May 2014 */
				if(m_nControlID == ID_SETTING_GENERAL && pItem->nItemNo == 1 || m_nControlID == ID_SETTING_SCAN && pItem->nItemNo == 1 || m_nControlID == ID_SETTING_GENERAL && pItem->nItemNo == 3 )
				{
					if( m_nControlID == ID_SETTING_GENERAL && theApp.m_dwProductID != BASIC)
					{
						ptSettingBtn = pItem->rcItem.TopLeft();
						ptSettingBtn.x += 440; //here pt.x += (428) is the starting point of Setting button  (428 previous value)
						ptSettingBtn.y += pItem->rcItem.Height() / 2 - 8;
					}
					else
					{
						ptSettingBtn = pItem->rcItem.TopLeft();
						ptSettingBtn.x += 440; //here pt.x +=  is the starting point of Setting button 
						ptSettingBtn.y += pItem->rcItem.Height() / 2 - 8;
					}

					CRect rcBox1(ptSettingBtn,CPoint(ptSettingBtn.x + 20,ptSettingBtn.y + 20)); // Here 50 & 30 is the Size of ONOFF Button
					if(rcBox1.PtInRect(point))
					{
						int m_ItemNo = 0;
					//	MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_SETTINGS_MSG_OK"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_OK);
					//issue no 730 Neha Gharge					
						if(m_nControlID == ID_SETTING_GENERAL && pItem->nItemNo == 1)
						{	

							/*	ISSUE NO - 744 NAME - NITIN K. TIME - 17th June 2014 */
							if(!pItem->bChecked)
							{
								// resolve by lalit kumawat 3-17-015
								//issue: -click on settings-> click delete reports->Click on settings button next to "Delete Reports"->Click on wardwiz tray->click exit.The delete report dialog should get close. 
								//(same issue applies to change password & scan computer at start up settings

								//CWardWizChangePassword m_objWwChangePass;
								//m_objWwChangePass.DoModal();
								if (theApp.m_SettingsReport == NULL)
								{
									theApp.m_SettingsReport = new CSettingsReports();
								}

								theApp.m_SettingsReport->DoModal();
							}
						}

						if(m_nControlID == ID_SETTING_SCAN && pItem->nItemNo == 1)
						{
							if(!pItem->bChecked)
							{
								//Name:Varada Ikhar, Date:06/01/2015, Issue No:9,	Version :1.8.3.4, 
								//Description: To disable the selection of startup scan type if it user off it from system try.

								// resolve by lalit kumawat 3-17-015
								//issue: -click on settings-> click delete reports->Click on settings button next to "Delete Reports"->Click on wardwiz tray->click exit.The delete report dialog should get close. 
								//(same issue applies to change password & scan computer at start up settings
								if (theApp.m_pSettingsScanTypeDlg == NULL)
								{
									theApp.m_pSettingsScanTypeDlg = new CSettingsScanTypeDlg();
								}
								if (theApp.m_pSettingsScanTypeDlg == NULL)
								{
									AddLogEntry(L"### m_pSettingsScanTypeDlg Allocation NULL", 0,0, true, SECONDLEVEL);
								}

								theApp.m_pSettingsScanTypeDlg->DoModal();
								if(m_pSettingsScanTypeDlg != NULL)
								{
									delete theApp.m_pSettingsScanTypeDlg;
									theApp.m_pSettingsScanTypeDlg = NULL;
								}
							}
						}

						if(m_nControlID == ID_SETTING_GENERAL && pItem->nItemNo == 3)
						{
							// resolve by lalit kumawat 3-17-015
							//issue: -click on settings-> click delete reports->Click on settings button next to "Delete Reports"->Click on wardwiz tray->click exit.The delete report dialog should get close. 
							//(same issue applies to change password & scan computer at start up settings

								if (theApp.m_SettingsPassChange == NULL)
								{
									theApp.m_SettingsPassChange = new CSettingsPassChangeDlg();
								}
								theApp.m_SettingsPassChange->DoModal();
							
						}
					}
				}

				CRect rcBox(pt,CPoint(pt.x + 50,pt.y + 30)); // Here 50 & 30 is the Size of ONOFF Button
				
				if(rcBox.PtInRect(point))
				{
					
					if(pItem->bChecked)
					{
						pItem->bChecked = FALSE;
						int ItemNo= pItem->nItemNo;
						BOOL flag=pItem->bChecked;
						makeRegistryEntryForSettingsTabOptions(m_nControlID,ItemNo,flag);
					}
					else
					{
						pItem->bChecked = TRUE;
						int ItemNo= pItem->nItemNo;
						BOOL flag=pItem->bChecked;
						makeRegistryEntryForSettingsTabOptions(m_nControlID,ItemNo,flag);
					}
					NM_HTMLLISTCTRL *pNMHDR = new NM_HTMLLISTCTRL;

					pNMHDR->hdr.code = HTMLLIST_ITEMCHECKED;
					pNMHDR->hdr.hwndFrom = GetSafeHwnd();
					pNMHDR->hdr.idFrom = m_nControlID;

					pNMHDR->lItemData = pItem->lItemData;
					pNMHDR->nItemNo = pItem->nItemNo;
					pNMHDR->sItemText = pItem->sItemText;
					pNMHDR->bChecked = pItem->bChecked;

					//Send check changed Event
					GetParent()->SendMessage(WM_NOTIFY,m_nControlID,(LPARAM)pNMHDR);
					delete pNMHDR;
				}
			}
			//Send WM_NOTIFY msg to parent
			NM_HTMLLISTCTRL *pNMHDR = new NM_HTMLLISTCTRL;

			pNMHDR->hdr.code = HTMLLIST_SELECTIONCHANGED;
			pNMHDR->hdr.hwndFrom = GetSafeHwnd();
			pNMHDR->hdr.idFrom = m_nControlID;

			pNMHDR->lItemData = pItem->lItemData;
			pNMHDR->nItemNo = pItem->nItemNo;
			pNMHDR->sItemText = pItem->sItemText;
			pNMHDR->bChecked = pItem->bChecked;

			//Send Selection changed Event
			GetParent()->SendMessage(WM_NOTIFY,m_nControlID,(LPARAM)pNMHDR);

			delete pNMHDR;

			m_nSelectedItem = i;
			Invalidate(FALSE);
			bItemSelected = TRUE;
			break;
		}
	}
	if(!bItemSelected)
	{
		NM_HTMLLISTCTRL *pNMHDR = new NM_HTMLLISTCTRL;

		pNMHDR->hdr.code = HTMLLIST_SELECTIONCHANGED;
		pNMHDR->hdr.hwndFrom = GetSafeHwnd();
		pNMHDR->hdr.idFrom = m_nControlID;

		pNMHDR->lItemData = 0;
		pNMHDR->nItemNo = NONE_SELECTED;
		pNMHDR->sItemText = "";
		
		GetParent()->SendMessage(WM_NOTIFY,m_nControlID,(LPARAM)pNMHDR);

		delete pNMHDR;
	}

	CWnd::OnLButtonDown(nFlags, point);
}



/***********************************************************************************************
*  Function Name  : UpdateUIWhenSystemTrayNotifiyUpdation
*  Description    : this function is called when wardwiz setting updated and Setting Dialog have focus by using System Tray and 
*  Author Name    : Lalit Kumawat
*  SR_NO		  :
*  Date           : 9/6/2014
***********************************************************************************************/
/*Issue number 127 If the setting tab is open and we change settings from tray, it is not immediately reflected on the setting tab.  */
void CHTMLListCtrl::UpdateUIWhenSystemTrayNotifiyUpdation(UINT nFlags, CPoint point) 
{
	SetFocus();

	m_nSelectedItem = NONE_SELECTED;

	POSITION pos = m_listItems.GetHeadPosition();
	for(int index = 0;index < m_listItems.GetCount();index++)
	{
		HTMLLIST_ITEM *pItem = m_listItems.GetNext(pos);

		pItem->rcItem.PtInRect(point);

		pItem->nItemNo= index;
		DWORD value= ReadRegistryEntryofSetting(index);
		if(value==1)
		{
			pItem->bChecked =FALSE;
		}
		if(value==0)
		{
			pItem->bChecked = TRUE;
		}
		
		int ItemNo= pItem->nItemNo;
		BOOL flag=pItem->bChecked;
		NM_HTMLLISTCTRL *pNMHDR = new NM_HTMLLISTCTRL;

		pNMHDR->hdr.code = HTMLLIST_ITEMCHECKED;
		pNMHDR->hdr.hwndFrom = GetSafeHwnd();
		pNMHDR->hdr.idFrom = m_nControlID;

		pNMHDR->lItemData = pItem->lItemData;
		pNMHDR->nItemNo = pItem->nItemNo;
		pNMHDR->sItemText = pItem->sItemText;
		pNMHDR->bChecked = pItem->bChecked;

		//Send check changed Event
		GetParent()->SendMessage(WM_NOTIFY,m_nControlID,(LPARAM)pNMHDR);
		delete pNMHDR;
	}

	//Name:Varada Ikhar     Date:06/01/2015
	//Issue No:9			Version :1.8.3.4
	//Description: To disable the selection of startup scan type if it user off it from system try.

	if(m_nControlID == ID_SETTING_SCAN)
	{
		DWORD value= ReadRegistryEntryofSetting(1);
		if(value == 0)
		{
			if((CSettingsScanTypeDlg*)m_pSettingsScanTypeDlg != NULL)
			{
				((CSettingsScanTypeDlg*)m_pSettingsScanTypeDlg)->m_CBScanType.EnableWindow(false);
				((CSettingsScanTypeDlg*)m_pSettingsScanTypeDlg)->m_btnOk.EnableWindow(false);
			}
		}
		else
		{
			if((CSettingsScanTypeDlg*)m_pSettingsScanTypeDlg != NULL)
			{
				((CSettingsScanTypeDlg*)m_pSettingsScanTypeDlg)->m_CBScanType.EnableWindow(true);
				((CSettingsScanTypeDlg*)m_pSettingsScanTypeDlg)->m_btnOk.EnableWindow(true);
			}
		}
	}
	//VARADA IKHAR*********************
	OnLButtonDown(nFlags, point);
}








/***************************************************************************************************                    
*  Function Name  : OnVScroll                                                     
*  Description    : Shows controls on vertical scrolling
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	//nPos is not valid in case of THUMB type msgs see below url
	//http://support.microsoft.com/kb/q152252/

	int nScrollPos = GetScrollPos(SB_VERT);
	int nLimit = GetScrollLimit(SB_VERT);

	int nScroll = nLimit;

	int SCROLL_AMT_Y = 50;

	switch(nSBCode) {
		case SB_LINEUP:      // Scroll up.      
		case SB_PAGEUP:
			if(nScrollPos <= 0)
			{
				return;
			}
			nScroll = min(nScrollPos,SCROLL_AMT_Y);
			SetScrollPos(SB_VERT,nScrollPos - nScroll);
			break;   
		case SB_LINEDOWN:   // Scroll down.
		case SB_PAGEDOWN:
			if(nScrollPos >= nLimit)
			{
				return;
			}
			nScroll = min(nScroll-nScrollPos,SCROLL_AMT_Y);
			SetScrollPos(SB_VERT,nScrollPos + nScroll);
		    break;
		case SB_THUMBPOSITION:
			{
				HWND hWndScroll;
				if ( pScrollBar == NULL )
					hWndScroll = m_hWnd;
				else
					hWndScroll = pScrollBar->m_hWnd;
				
				SCROLLINFO info;
				info.cbSize = sizeof(SCROLLINFO);
				info.fMask = SIF_TRACKPOS;
				::GetScrollInfo(hWndScroll, SB_VERT, &info);
				
				nPos = info.nTrackPos;

				int nScr = nScrollPos - nPos;
				
				SetScrollPos(SB_VERT,nPos);
			}
			break;
		case SB_THUMBTRACK:
			{
				HWND hWndScroll;
				if ( pScrollBar == NULL )
					hWndScroll = m_hWnd;
				else
					hWndScroll = pScrollBar->m_hWnd;
				
				SCROLLINFO info;
				info.cbSize = sizeof(SCROLLINFO);
				info.fMask = SIF_TRACKPOS;
				::GetScrollInfo(hWndScroll, SB_VERT, &info);
				
				nPos = info.nTrackPos;

				int nScr = nScrollPos - nPos;
				
				SetScrollPos(SB_VERT,nPos,FALSE);
			}
			break;
	}	
	
	Invalidate();
	UpdateWindow();	
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

/***************************************************************************************************                    
*  Function Name  : OnSize                                                     
*  Description    : Vary size of item insert into html list control.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	CRect rc;
	GetClientRect(&rc);

	m_nWndHeight = rc.Height();
	m_nWndWidth = rc.Width();

	ReArrangeWholeLayout();
	
	SCROLLINFO scrlinfo;
	scrlinfo.cbSize = sizeof(scrlinfo);

	scrlinfo.fMask = SIF_PAGE|SIF_RANGE;
	scrlinfo.nMax = m_nListHeight;
	scrlinfo.nMin = 0;
	scrlinfo.nPage = m_nWndHeight;
	scrlinfo.nPos = 0;
	SetScrollInfo(SB_VERT,&scrlinfo);
	Invalidate(FALSE);
}

/***************************************************************************************************                    
*  Function Name  : GetItemRect                                                     
*  Description    : Retrieves the bounding rectangle for all or part of an item in the current view
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
CRect CHTMLListCtrl::GetItemRect(int nPos)
{
	HTMLLIST_ITEM* pItem = GetInternalData(nPos);
	if(pItem)
	{
		return pItem->rcItem;
	}
	else
	{
		return NULL;
	}
}

/***************************************************************************************************                    
*  Function Name  : OnMouseWheel                                                     
*  Description    : The framework calls this member function as a user rotates the mouse wheel and encounters the wheel's next notch.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO		  :
*  Date           : 25 April 2014
****************************************************************************************************/
BOOL CHTMLListCtrl::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	int nVertScroll = GetScrollPos(SB_VERT);

	int maxpos = GetScrollLimit(SB_VERT);

	if(zDelta < 0)
	{
		int nNewPos = min(nVertScroll + 40 , maxpos);
		
		SetScrollPos(SB_VERT,nNewPos);

		UpdateWindow();
	}
	else
	{
		int nNewPos = max((nVertScroll - 40) , 0) ;
		
		SetScrollPos(SB_VERT, nNewPos);

		UpdateWindow();
	}
	Invalidate();
	return CWnd::OnMouseWheel(nFlags,zDelta,pt);
}

/***************************************************************************************************                    
*  Function Name  : GetSelectedItem                                                     
*  Description    : Retrives dword value of selected item.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
int CHTMLListCtrl::GetSelectedItem()
{
	return m_nSelectedItem;
}

/***************************************************************************************************                    
*  Function Name  : GetItemText                                                     
*  Description    : Retrieves the text of a list view item or subitem.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
CString CHTMLListCtrl::GetItemText(int nPos)
{
	HTMLLIST_ITEM* pItem = GetInternalData(nPos);
	if(pItem)
	{
		return pItem->sItemText;
	}
	else
	{
		return L"";
	}
}

/***************************************************************************************************                    
*  Function Name  : SetExtendedStyle                                                     
*  Description    : Sets the current extended styles of a list view control.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::SetExtendedStyle(DWORD dwExStyle)
{
	m_dwExtendedStyles = dwExStyle;

	ReArrangeWholeLayout();
	SCROLLINFO scrlinfo;
	scrlinfo.cbSize = sizeof(scrlinfo);

	scrlinfo.fMask = SIF_PAGE|SIF_RANGE;
	scrlinfo.nMax = m_nListHeight;
	scrlinfo.nMin = 0;
	scrlinfo.nPage = m_nWndHeight;
	scrlinfo.nPos = 0;
	SetScrollInfo(SB_VERT,&scrlinfo);

	Invalidate(FALSE);
}

/***************************************************************************************************                    
*  Function Name  : GetExtendedStyle                                                     
*  Description    : Retrieves the current extended styles of a list view control. 
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
DWORD CHTMLListCtrl::GetExtendedStyle()
{
	return m_dwExtendedStyles;
}

/***************************************************************************************************                    
*  Function Name  : OnSetFocus                                                     
*  Description    : The framework calls this member function after gaining the input focus.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	m_bHasFocus = TRUE;
	if(m_nSelectedItem != NONE_SELECTED)
	{
		InvalidateRect(GetItemRect(m_nSelectedItem));
	}
}

/***************************************************************************************************                    
*  Function Name  : OnKillFocus                                                     
*  Description    : The framework calls this member function after killing the input focus.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);
	m_bHasFocus = FALSE;
	if(m_nSelectedItem != NONE_SELECTED)
	{
		InvalidateRect(GetItemRect(m_nSelectedItem));
	}
}

/***************************************************************************************************                    
*  Function Name  : OnGetDlgCode                                                     
*  Description    : Called for a control so the control can process arrow-key and TAB-key input itself.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
UINT CHTMLListCtrl::OnGetDlgCode()
{
    return DLGC_WANTARROWS|DLGC_WANTCHARS;
}

/***************************************************************************************************                    
*  Function Name  : OnKeyDown                                                     
*  Description    : The framework calls this member function when a nonsystem key is pressed.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == VK_UP)
	{
		if(m_nSelectedItem == NONE_SELECTED)
		{
			m_nSelectedItem = 0;
		}
		else
		{
			if(m_nSelectedItem > 0)
			{
				m_nSelectedItem --;
			}
		}
		EnsureVisible(m_nSelectedItem);
		Invalidate(FALSE);
		SendSelectionChangeNotification(m_nSelectedItem);
	}
	else if(nChar == VK_DOWN)
	{
		if(m_nSelectedItem == NONE_SELECTED)
		{
			m_nSelectedItem = m_nTotalItems - 1;
		}
		else
		{
			if(m_nSelectedItem < (m_nTotalItems - 1))
			{
				m_nSelectedItem ++;
			}
		}
		EnsureVisible(m_nSelectedItem);
		Invalidate(FALSE);
		SendSelectionChangeNotification(m_nSelectedItem);
	}
	else if(nChar == VK_SPACE)
	{
		if(m_dwExtendedStyles & HTMLLIST_STYLE_CHECKBOX)
		{
			if(m_nSelectedItem != NONE_SELECTED)
			{
				if(GetItemCheck(m_nSelectedItem))
				{
					SetItemCheck(m_nSelectedItem,FALSE);
				}
				else
				{
					SetItemCheck(m_nSelectedItem);
				}
				SendCheckStateChangedNotification(m_nSelectedItem);
			}
		}
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

/***************************************************************************************************                    
*  Function Name  : EnsureVisible                                                     
*  Description    : Ensures that a list view item is at least partially visible.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::EnsureVisible(int nPos)
{
	int nScrollPos = GetScrollPos(SB_VERT);
	
	int nItemPos = 0;
	int nScrollAmount = 0;

	POSITION pos = m_listItems.GetHeadPosition();
	for(int i = 0;i < m_listItems.GetCount();i++)
	{
		HTMLLIST_ITEM *pItem = m_listItems.GetNext(pos);
		if(pItem->nItemNo == nPos)
		{
			if(nItemPos < nScrollPos)
			{
				//Item is above
				nScrollAmount = nScrollPos - nItemPos;
			}
			else if( (nItemPos + pItem->nHeight)> (nScrollPos + m_nWndHeight) )
			{
				//Item is below
				nScrollAmount = (nScrollPos + m_nWndHeight) - nItemPos - pItem->nHeight;
			}
			break;
		}
		nItemPos += pItem->nHeight;
	}

	if(nScrollAmount)
	{
		SetScrollPos(SB_VERT,nScrollPos - nScrollAmount);
		Invalidate(FALSE);
	}
}

/***************************************************************************************************                    
*  Function Name  : SetItemCheck                                                     
*  Description    : Sets whether the item appears in the UI with a check mark indicating that it is selected for synchronization.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::SetItemCheck(int nPos,BOOL bCheck)
{
	HTMLLIST_ITEM* pItem = GetInternalData(nPos);
	if(pItem)
	{
		pItem->bChecked = bCheck;
		InvalidateRect(pItem->rcItem,FALSE);
	}
}

/***************************************************************************************************                    
*  Function Name  : GetItemCheck                                                     
*  Description    : Retrieves a value indicating whether a check box for the specified item is checked, unchecked, or indeterminate.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
BOOL CHTMLListCtrl::GetItemCheck(int nPos)
{
	HTMLLIST_ITEM* pItem = GetInternalData(nPos);
	if(pItem)
	{
		return pItem->bChecked;
	}
	else
	{
		return FALSE;
	}
}

/***************************************************************************************************                    
*  Function Name  : OnRButtonDown                                                     
*  Description    : The framework calls this member function when the user presses the right mouse button.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	NM_HTMLLISTCTRL *pNMHDR = new NM_HTMLLISTCTRL;

	pNMHDR->hdr.code = HTMLLIST_RBUTTONDOWN;
	pNMHDR->hdr.hwndFrom = GetSafeHwnd();
	pNMHDR->hdr.idFrom = m_nControlID;

	pNMHDR->lItemData = 0;
	pNMHDR->nItemNo = -1;
	pNMHDR->sItemText = "";
	pNMHDR->bChecked = 0;

	GetParent()->SendMessage(WM_NOTIFY,m_nControlID,(LPARAM)pNMHDR);

	delete pNMHDR;	
	CWnd::OnRButtonDown(nFlags, point);
}

/***************************************************************************************************                    
*  Function Name  : SetItemText                                                     
*  Description    : Changes the text of a list view item or subitem.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::SetItemText(int nPos, CString sItemText,BOOL bCalculateHeight)
{
	HTMLLIST_ITEM* pItem = GetInternalData(nPos);
	if(pItem)
	{
		pItem->sItemText = sItemText;
		if(bCalculateHeight)
		{
			pItem->bHeightSpecified = FALSE;
			ReArrangeWholeLayout();
			Invalidate(FALSE);
		}
		else
		{
			InvalidateRect(pItem->rcItem,FALSE);
		}
	}
}

/***************************************************************************************************                    
*  Function Name  : DeleteItem                                                     
*  Description    : Delete item from list control.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
BOOL CHTMLListCtrl::DeleteItem(int nPos)
{
	POSITION pos = m_listItems.GetHeadPosition();
	for(int i = 0;i < m_listItems.GetCount();i++)
	{
		HTMLLIST_ITEM *pItem = m_listItems.GetNext(pos);
		if(pItem->nItemNo == nPos)
		{
			//Is this the last item
			if(pos != NULL)
			{
				//pos is now pointing to the next row, so go back
				m_listItems.GetPrev(pos);
				m_listItems.RemoveAt(pos);
			}
			else
			{
				m_listItems.RemoveAt(m_listItems.GetTailPosition());
			}
			
			if(m_nSelectedItem == pItem->nItemNo)
			{
				m_nSelectedItem = NONE_SELECTED;
			}

			//Adjust scrollbar 
			m_nListHeight -= pItem->nHeight;
			SCROLLINFO scrlinfo;
			scrlinfo.cbSize = sizeof(scrlinfo);

			scrlinfo.fMask = SIF_PAGE|SIF_RANGE;
			scrlinfo.nMax = m_nListHeight;
			scrlinfo.nMin = 0;
			scrlinfo.nPage = m_nWndHeight;
			scrlinfo.nPos = 0;
			SetScrollInfo(SB_VERT,&scrlinfo);	

			//delete pItem;
			m_nTotalItems --;

			ReArrangeItems();
			Invalidate(FALSE);
			return TRUE;
		}
	}
	return FALSE;
}

/***************************************************************************************************                    
*  Function Name  : ReArrangeItems                                                     
*  Description    : Re arrange item in list control.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::ReArrangeItems()
{
	m_mapItems.RemoveAll();
	POSITION pos = m_listItems.GetHeadPosition();
	for(int i = 0;i < m_listItems.GetCount();i++)
	{
		HTMLLIST_ITEM *pItem = m_listItems.GetNext(pos);
		pItem->nItemNo = i;
		SetInternalData(i,pItem);
	}
}

/***************************************************************************************************                    
*  Function Name  : ReArrangeWholeLayout                                                     
*  Description    : Rearrange whole layout of html list control.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::ReArrangeWholeLayout()
{
	m_nTotalItems = 0;
	m_nListHeight = 0;
	m_mapItems.RemoveAll();

	POSITION pos = m_listItems.GetHeadPosition();
	for(int i = 0;i < m_listItems.GetCount();i++)
	{
		HTMLLIST_ITEM *pItem = m_listItems.GetNext(pos);
		pItem->nItemNo = i;
		
		if(!pItem->bHeightSpecified)
		{
			//Calculate items height
			pItem->nHeight = CalculateItemHeight(pItem->sItemText,pItem->nStyle,pItem->uiImage,m_nWndWidth);
		}
		

		m_nTotalItems ++;

		m_nListHeight += pItem->nHeight;
		SetInternalData(i,pItem);
	}
}

/***************************************************************************************************                    
*  Function Name  : GetImage                                                     
*  Description    : Get Image into html list control accordingto position selected.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
UINT CHTMLListCtrl::GetImage(int nPos)
{
	HTMLLIST_ITEM *pItem = GetInternalData(nPos);
	if(pItem)
	{
		return pItem->uiImage;
	}
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : SetImage                                                     
*  Description    : Set Image into html list control accordingto position selected.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::SetImage(int nPos, UINT uiImage)
{
	HTMLLIST_ITEM *pItem = GetInternalData(nPos);
	if(pItem)
	{
		pItem->uiImage = uiImage;
		InvalidateRect(pItem->rcItem,FALSE);
	}
}

/***************************************************************************************************                    
*  Function Name  : OnLButtonDblClk                                                     
*  Description    : Handler for double click of left button.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{	
	NM_HTMLLISTCTRL *pNMHDR = new NM_HTMLLISTCTRL;

	pNMHDR->hdr.code = HTMLLIST_LBUTTONDBLCLICK;
	pNMHDR->hdr.hwndFrom = GetSafeHwnd();
	pNMHDR->hdr.idFrom = m_nControlID;

	pNMHDR->lItemData = 0;
	pNMHDR->nItemNo = -1;
	pNMHDR->sItemText = "";
	pNMHDR->bChecked = 0;

	GetParent()->SendMessage(WM_NOTIFY,m_nControlID,(LPARAM)pNMHDR);

	delete pNMHDR;	
	CWnd::OnLButtonDblClk(nFlags, point);
}

/***************************************************************************************************                    
*  Function Name  : DrawItem                                                     
*  Description    : Called by the framework when a visual aspect of an owner-draw list view control changes.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::DrawItem(CDC *pDC, CRect rcItem, HTMLLIST_ITEM *pItem, BOOL bSelected)
{
	CRect rcClipBox;
	pDC->GetClipBox(&rcClipBox);

	if(!IsRectVisible(rcClipBox,rcItem))
	{
		return;
	}

	COLORREF clrText = RGB(0,0,0);
	COLORREF clrOld;
	CRect rcImage(0,0,0,0);
		
	if(bSelected)
	{
		//clrText = RGB(255,255,255);
		pDC->FillSolidRect(&rcItem,m_clrBkSelectedItem);
	}
	else
	{
		pDC->FillSolidRect(&rcItem,RGB(255,255,255));
	}
	if(m_dwExtendedStyles & HTMLLIST_STYLE_GRIDLINES)
	{
		pDC->DrawEdge(&rcItem,BDR_SUNKENINNER,BF_LEFT|BF_BOTTOM|BF_FLAT);
	}
	
	CPoint ptCheckBox = rcItem.TopLeft();
//	ptCheckBox.x = rcItem.right - 50;

	if(m_dwExtendedStyles & HTMLLIST_STYLE_CHECKBOX)
	{
		if(m_nControlID == ID_SETTING_GENERAL && pItem->nItemNo == 3)
		{
			ptCheckBox = rcItem.TopLeft();
			ptCheckBox.x = rcItem.right - 35;	// rcItem.right - 40 where 40 is the co-ordinates location from Right Side of the UI Pane
//			ptCheckBox.x += ITEM_PADDING_CHECKBOX_LEFT;
			ptCheckBox.y += rcItem.Height() / 2 - 15;
			
			/*	ISSUE NO -  NAME - NITIN K. TIME - 27th June 2014 */
			// Setting Button for Change password was getting disabled 
			m_ImageListReports.Draw(pDC,0,ptCheckBox,ILD_TRANSPARENT);
			ptCheckBox.x += ITEM_CHECKBOX_WIDTH;
		}
		else
		{
			//Issue No 658 If no of days  =0  email and update items in menu should be disable.
			// Neha Gharge 7 July,2015
			if (m_bDaysLeftZeroFlag || m_bOutlookFlag)
			{
				pItem->bChecked = true;
			}
			ptCheckBox.x = rcItem.right - 90;	// rcItem.right - 100 where 100 is the co-ordinates location from Right Side of the UI Pane
	//		ptCheckBox.x += ITEM_PADDING_CHECKBOX_LEFT;
			ptCheckBox.y += rcItem.Height() / 2 - 11;
			
			m_ImageList.Draw(pDC,pItem->bChecked?1:0,ptCheckBox,ILD_TRANSPARENT);
			ptCheckBox.x += ITEM_CHECKBOX_WIDTH;


		
			/*	ISSUE NO - 399	Settings tab:for reports & startup scan, drop down list needs to be provided to select timeline/scan type.
			NAME - NITIN K. TIME - 27st May 2014 */
			if(m_nControlID == ID_SETTING_GENERAL && pItem->nItemNo == 1 || m_nControlID == ID_SETTING_SCAN && pItem->nItemNo == 1)
			{
				ptCheckBox = rcItem.TopLeft();
				ptCheckBox.x = rcItem.right - 35;	// rcItem.right - 40 where 40 is the co-ordinates location from Right Side of the UI Pane
	//			ptCheckBox.x += ITEM_PADDING_CHECKBOX_LEFT;
				ptCheckBox.y += rcItem.Height() / 2 - 15;
				
					
				m_ImageListReports.Draw(pDC,pItem->bChecked?1:0,ptCheckBox,ILD_TRANSPARENT);
				ptCheckBox.x += ITEM_CHECKBOX_WIDTH;
			}
		}
	}
	
	//Draw image if an imagelist is attached
	if(m_dwExtendedStyles & HTMLLIST_STYLE_IMAGES)
	{
		if(m_pImageList)
		{
			IMAGEINFO imgInfo = {0};
			m_pImageList->GetImageInfo(0,&imgInfo);
			rcImage = imgInfo.rcImage;
			
			CPoint pt = rcItem.TopLeft();//ptCheckBox;
			pt.x += ITEM_IMAGE_PADDING_LEFT;
			pt.y = rcItem.top;
			pt.y += rcItem.Height() / 2 - rcImage.Height()/2;
			
			m_pImageList->Draw(pDC,pItem->uiImage,pt,ILD_TRANSPARENT);			
		}
	}
	
	if(pItem->nStyle == NORMAL_TEXT)
	{
		clrOld = pDC->SetTextColor(clrText);
		
		CRect rc = rcItem;
		
		if(rcImage.Width())
		{
			//make space for the Image already drawn
			rc.DeflateRect(rcImage.Width() + ITEM_IMAGE_PADDING_LEFT + ITEM_IMAGE_PADDING_RIGHT,0,0,0);
		}
		
		if(m_dwExtendedStyles & HTMLLIST_STYLE_CHECKBOX)
		{
			rc.left += ITEM_PADDING_LEFT + ITEM_CHECKBOX_WIDTH;
		}
		else
		{
			rc.left += ITEM_PADDING_LEFT;
		}
		
		if(!pItem->bHeightSpecified)
			rc.top += ITEM_PADDING_TOP;
		
		pDC->DrawText(pItem->sItemText,pItem->sItemText.GetLength(),&rc,
			DT_LEFT|DT_WORDBREAK);
	}
	//We are Using the HTML style Text for WARDWIZ
	else if(pItem->nStyle == HTML_TEXT)
	{
		//Draw HTML
		clrOld = pDC->SetTextColor(clrText);
		
		CRect rc = rcItem;
		if(rcImage.Width())
		{
			//make space for the Image already drawn
			rc.DeflateRect(rcImage.Width() + ITEM_IMAGE_PADDING_LEFT + ITEM_IMAGE_PADDING_RIGHT,0,0,0);
		}
		if(m_dwExtendedStyles & HTMLLIST_STYLE_CHECKBOX)
		{
			rc.left += ITEM_PADDING_LEFT + ITEM_CHECKBOX_WIDTH;
		}
		else
		{
			rc.left += ITEM_PADDING_LEFT;
		}
		
		if(!pItem->bHeightSpecified)
			rc.top += ITEM_PADDING_TOP;
		
		DrawHTML(pDC->GetSafeHdc(),pItem->sItemText,pItem->sItemText.GetLength(),
			&rc,DT_LEFT|DT_WORDBREAK);
	}
	else if(pItem->nStyle == SINGLE_LINE_TEXT)
	{
		clrOld = pDC->SetTextColor(clrText);
		
		CRect rc = rcItem;
		if(rcImage.Width())
		{
			//make space for the Image already drawn
			rc.DeflateRect(rcImage.Width() + ITEM_IMAGE_PADDING_LEFT + ITEM_IMAGE_PADDING_RIGHT,0,0,0);
		}
		if(m_dwExtendedStyles & HTMLLIST_STYLE_CHECKBOX)
		{
			rc.left += ITEM_PADDING_LEFT + ITEM_CHECKBOX_WIDTH;
		}
		else
		{
			rc.left += ITEM_PADDING_LEFT;
		}
		
		if(!pItem->bHeightSpecified)
			rc.top += ITEM_PADDING_TOP;
		
		//See if we can fit the text in one line
		TCHAR szBuffer[_MAX_PATH];
		memset(szBuffer,0,_MAX_PATH);
		
		_tcscpy_s(szBuffer, pItem->sItemText);
		
		if(PathCompactPath(pDC->GetSafeHdc(),szBuffer,rc.Width()))
		{
			pDC->DrawText(szBuffer, static_cast<int>(_tcslen(szBuffer)), &rc,
				DT_LEFT|DT_SINGLELINE|DT_VCENTER);	
		}
		else
		{
			pDC->DrawText(pItem->sItemText,pItem->sItemText.GetLength(),&rc,
				DT_LEFT|DT_SINGLELINE|DT_VCENTER);
		}
	}
	
	pDC->SetTextColor(clrOld);
	
	//Draw the focus rect if focused
	if(m_bHasFocus && (bSelected))
	{
		pDC->DrawFocusRect(&rcItem);
	}
}

/***************************************************************************************************                    
*  Function Name  : SendSelectionChangeNotification                                                     
*  Description    : Send notification after selecting any changes.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::SendSelectionChangeNotification(int nPos)
{
	//Send WM_NOTIFY msg to parent
	HTMLLIST_ITEM *pItem = GetInternalData(nPos);
	if(pItem)
	{
		NM_HTMLLISTCTRL *pNMHDR = new NM_HTMLLISTCTRL;

		pNMHDR->hdr.code = HTMLLIST_SELECTIONCHANGED;
		pNMHDR->hdr.hwndFrom = GetSafeHwnd();
		pNMHDR->hdr.idFrom = m_nControlID;

		pNMHDR->lItemData = pItem->lItemData;
		pNMHDR->nItemNo = pItem->nItemNo;
		pNMHDR->sItemText = pItem->sItemText;
		pNMHDR->bChecked = pItem->bChecked;

		//Send Selection changed Event
		GetParent()->SendMessage(WM_NOTIFY,m_nControlID,(LPARAM)pNMHDR);
		delete pNMHDR;
		return;
	}
}

/***************************************************************************************************                    
*  Function Name  : SendCheckStateChangedNotification                                                     
*  Description    : this function is  called for Default Setting options
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::SendCheckStateChangedNotification(int nPos)
{
	//Issue No 658 If no of days  =0  email and update items in menu should be disable.
	// Neha Gharge 7 July,2015
	//Send WM_NOTIFY msg to parent
	HTMLLIST_ITEM *pItem = GetInternalData(nPos);
	if (pItem)
	{
		NM_HTMLLISTCTRL *pNMHDR = new NM_HTMLLISTCTRL;

		pNMHDR->hdr.code = HTMLLIST_ITEMCHECKED;
		pNMHDR->hdr.hwndFrom = GetSafeHwnd();
		pNMHDR->hdr.idFrom = m_nControlID;

		pNMHDR->lItemData = pItem->lItemData;
		pNMHDR->nItemNo = pItem->nItemNo;
		pNMHDR->sItemText = pItem->sItemText;
		pItem->bChecked = FALSE;
		pNMHDR->bChecked = pItem->bChecked;
		BOOL Flag = FALSE;
		if (m_nControlID == ID_SETTING_SCAN && nPos == 1)
		{
			pItem->bChecked = TRUE;
			pNMHDR->bChecked = pItem->bChecked;
			Flag = TRUE;
		}
		if (m_nControlID == ID_SETTING_UPDATE && nPos == 2)
		{
			pItem->bChecked = TRUE;
			pNMHDR->bChecked = pItem->bChecked;
			Flag = TRUE;
		}
		CPoint point;
		point.SetPoint(10, 10);
		OnLButtonDown(1, point);
		if (m_nControlID == ID_SETTING_EMAIL && nPos == 0 && (m_bOutlookFlag || m_bDaysLeftZeroFlag))
		{
			GetParent()->SendMessage(WM_NOTIFY,m_nControlID,(LPARAM)pNMHDR);
			delete pNMHDR;
			return;
		}
		if (m_nControlID == ID_SETTING_UPDATE && nPos == 0 /*&& m_bDaysLeftZeroFlag*/)//Ram: Commented as live update should happen in unregistered-product.
		{
			GetParent()->SendMessage(WM_NOTIFY, m_nControlID, (LPARAM)pNMHDR);
			//delete pNMHDR;
			//return;
		}

		makeRegistryEntryForSettingsTabOptions(m_nControlID,nPos,Flag );
		//Send Selection changed Event
		GetParent()->SendMessage(WM_NOTIFY,m_nControlID,(LPARAM)pNMHDR);
		delete pNMHDR;
		return;
	}
}

/***************************************************************************************************                    
*  Function Name  : IsRectVisible                                                     
*  Description    : Checks whether rectangle is visible or not.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
BOOL CHTMLListCtrl::IsRectVisible(CRect rcClipBox, CRect rcItem)
{
	if(rcClipBox.top > rcItem.bottom) 
	{
		//Item is above the clip box
		return FALSE;
	}
	else if(rcClipBox.bottom < rcItem.top)
	{
		return FALSE;
	}
	return TRUE;
}

/***************************************************************************************************                    
*  Function Name  : GetInternalData                                                     
*  Description    : Get internal data from lookup table
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
HTMLLIST_ITEM * CHTMLListCtrl::GetInternalData(int nPos)
{
	HTMLLIST_ITEM *pData = NULL;
	m_mapItems.Lookup(nPos,pData);
	return pData;
}

/***************************************************************************************************                    
*  Function Name  : SetInternalData                                                     
*  Description    : Set a data into lookup table
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::SetInternalData(int nPos, HTMLLIST_ITEM *pData)
{
	m_mapItems.SetAt(nPos,pData);
}

/***************************************************************************************************                    
*  Function Name  : OnDestroy                                                     
*  Description    : Destroy font object
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::OnDestroy() 
{
	CWnd::OnDestroy();
	if(m_font.m_hObject != NULL)
		m_font.DeleteObject();
}

/***************************************************************************************************                    
*  Function Name  : OnSetCursor                                                     
*  Description    : The framework calls this member function if mouse input is not captured and the mouse causes cursor movement within the CWnd object.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
BOOL CHTMLListCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	SetCursor(::LoadCursor(NULL, IDC_ARROW));
	return TRUE;//CWnd::OnSetCursor(pWnd, nHitTest, message);
}

/***************************************************************************************************                    
*  Function Name  : makeRegistryEntryForSettingsTabOptions                                                     
*  Description    : this function is  called for making the Registry Entries for the Settings Tab
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
void CHTMLListCtrl::makeRegistryEntryForSettingsTabOptions(UINT m_nControlID,int ItemNo,BOOL flag)
{
	try
	{
		//Issue No 658 If no of days  =0  email and update items in menu should be disable.
		// Neha Gharge 7 July,2015
		LPTSTR szEmailId_GUI = theApp.GetRegisteredEmailID();
		LPCTSTR emailScanOptionsPath = TEXT("Software\\WardWiz Antivirus\\EmailScanSetting");
		LPCTSTR settingsTabPath = TEXT("Software\\WardWiz Antivirus");
		DWORD flagValue;
		if (flag)
		{
			flagValue = 0;
		}
		else
		{
			flagValue = 1;
		}
		switch (m_nControlID)
		{
		case ID_SETTING_GENERAL:
			switch (ItemNo)
			{
			case 0:
				if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwShowTrayPopup", flagValue))
				{
					AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::dwShowTrayPopup", 0, 0, true, SECONDLEVEL);
				}
				break;
			case 1:
				if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwDaysToDelRep", flagValue))
				{
					AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::dewDeleteOldReports", 0, 0, true, SECONDLEVEL);
				}
				break;
			case 2:
				if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwShowStartupTips", flagValue))
				{
					AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::dwShowStartupTips", 0, 0, true, SECONDLEVEL);
				}
				break;
			case 3:
				break;
			}
			break;

		case ID_SETTING_SCAN:
			switch (ItemNo)
			{
			case 0:
				if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwUsbScan", flagValue))
				{
					AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::dwUsbScan", 0, 0, true, SECONDLEVEL);
				}
				break;
			case 1:
				if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwStartUpScan", flagValue))
				{
					AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
				}
				break;
			case 2:
				if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEnableSound", flagValue))
				{
					AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
				}
				/* ISSUE NO - 697 NAME - NITIN K. TIME - 15th June 2014 */
				if (flagValue == 0)
				{
					theApp.m_bEnableSound = false;
				}
				else
				{
					theApp.m_bEnableSound = true;
				}
				break;
			}
			break;


			//ISSue no :33 neha Gharge 31/5/2014 **********************/	
		case ID_SETTING_EMAIL:
			switch (ItemNo)
			{
			case 0:
				if (m_bDaysLeftZeroFlag)
				{
					if (szEmailId_GUI && _tcslen(szEmailId_GUI) == 0)
					{
						theApp.m_bIsPopUpDisplayed = true;
						MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_UNREGISTERED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
						theApp.m_bIsPopUpDisplayed = false;
					}
					else
					{
						if (theApp.m_szRegKey && _tcslen(theApp.m_szRegKey) == 0)
						{
							theApp.m_bIsPopUpDisplayed = true;
							MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_TRIAL_EXPIRED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
							theApp.m_bIsPopUpDisplayed = false;
						}
						else
						{
							theApp.m_bIsPopUpDisplayed = true;
							MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_EXPIRED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
							theApp.m_bIsPopUpDisplayed = false;
						}
					}
					if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEmailScan", 1))
					{
						AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
					}
				}
				else if (m_bOutlookFlag)
				{
					theApp.m_bIsPopUpDisplayed = true; 
					MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_INSTALL_OUTLOOK"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
					theApp.m_bIsPopUpDisplayed = false;
					if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEmailScan", 1))
					{
						AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
					}
				}
				else
				{
					if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEmailScan", flagValue))
					{
						AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
					}
					if (!SendEmailPluginChange2Service(ENABLE_DISABLE_EMAIL_PLUGIN, flagValue))
					{
						AddLogEntry(L"### Error in CSystemTray::SendEmailPluginChange2Service", 0, 0, true, SECONDLEVEL);
					}
					SendEnableEmailScanMessage2UI();
				}
				break;
			case 1:
				if (!WriteRegistryEntryOfSettingsTab(emailScanOptionsPath, L"dwEnableVirusScan", flagValue))
				{
					AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
				}
				break;
			case 2:
				if (!WriteRegistryEntryOfSettingsTab(emailScanOptionsPath, L"dwEnableSpamFilter", flagValue))
				{
					AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
				}
				break;
			case 3:
				if (!WriteRegistryEntryOfSettingsTab(emailScanOptionsPath, L"dwEnableContentFilter", flagValue))
				{
					AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
				}
				break;
			case 4:
				if (!WriteRegistryEntryOfSettingsTab(emailScanOptionsPath, L"dwEnableSignature", flagValue))
				{
					AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
				}
				break;
			}
			break;
		case ID_SETTING_UPDATE:
			switch (ItemNo)
			{
			case 0:
				//Ram: Commented as live update should happen in unregistered-product.
				///*if (m_bDaysLeftZeroFlag)*/
				//{
				//	if (szEmailId_GUI && _tcslen(szEmailId_GUI) == 0)
				//	{
				//			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_UNREGISTERED") ,theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION| MB_OK);
				//	}
				//	else
				//	{
				//		if (theApp.m_szRegKey && _tcslen(theApp.m_szRegKey) == 0)
				//		{
				//			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_TRIAL_EXPIRED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
				//		}
				//		else
				//		{
				//			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_EXPIRED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
				//		}
				//	}
				//	if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwAutoDefUpdate", 1))
				//	{
				//		AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
				//	}
				//}
				//else
				{
					if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwAutoDefUpdate", flagValue))
					{
						AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
					}
					//issue no 4: Autolive get notification as soon as we off check box from tray
					if (flagValue == 0)
					{
						SendRequestALUservToStopUpdate(STOP_UPDATE);
					}
				}
				break;
				/*case 1:
					if(!WriteRegistryEntryOfSettingsTab(settingsTabPath,L"dwAutoProductUpdate",flagValue))
					{
					AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
					}
					break;*/

				//Varada Ikhar, Date: 13/04/2015
				// Issue:1.Go to WardWiz Settings. 2.Open Update in Settings. 3.2nd setting "Apply Update on StartUp" setting should be removed.
				//Commented the below case.
				/*case 1:
					if(!WriteRegistryEntryOfSettingsTab(settingsTabPath,L"dwUpdateReboot",flagValue))
					{
					AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
					}
					break;*/
			}
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CHTMLListCtrl::makeRegistryEntryForSettingsTabOptions", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************                    
*  Function Name  : WriteRegistryEntryOfSettingsTab                                                     
*  Description    : this function is  called for Writing the Registry Key Value for Menu Items
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
BOOL CHTMLListCtrl::WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey,CString strKey, DWORD dwChangeValue)
{
	if(!SetRegistrykeyUsingService(SubKey, strKey, REG_DWORD, dwChangeValue,true))
	{
		AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
	}
	Sleep(20);
	return TRUE;
}

/***************************************************************************************************                    
*  Function Name  : SetRegistrykeyUsingService                                                     
*  Description    : this function is  called for setting the Registry Keys using the Services
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
bool CHTMLListCtrl::SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));

	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = SZ_DWORD; 
	//szPipeData.hHey = HKEY_LOCAL_MACHINE;

	wcscpy_s(szPipeData.szFirstParam, SubKey);
	wcscpy_s(szPipeData.szSecondParam, lpValueName );
	szPipeData.dwSecondValue = dwData;

	CISpyCommunicator objCom(SERVICE_SERVER);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to set data in CHTMLListCtrl : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CHTMLListCtrl : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : ReadRegistryEntryofSetting                                                     
*  Description    : this function is  called for Reading the Registry entries from the Registry
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
DWORD CHTMLListCtrl::ReadRegistryEntryofSetting(int ItemNo)
{
	try
	{
		CString strKey;

		/*if(ItemNo==0)
		{
		strKey=L"dwEmailScan";
		}
		else if(ItemNo==1)
		{
		strKey = L"dwUsbScan";
		}
		else if(ItemNo==2)
		{
		strKey = L"dwShowToolTip";
		}
		else if(ItemNo==3)
		{
		strKey = L"dwEnableSound";
		}
		else if(ItemNo==4)
		{
		strKey = L"dwStartUpScan";
		}*/
		switch (m_nControlID)
		{
		case ID_SETTING_GENERAL:
			switch (ItemNo)
			{
			case 0:
				strKey = L"dwShowTrayPopup";
				break;
			case 1:
				strKey = L"dwDaysToDelRep";
				break;
			case 2:
				strKey = L"dwShowStartupTips";
				break;
			case 3:
				strKey = L"dwShowSecNews";
				break;

			}
			break;
		case ID_SETTING_SCAN:
			switch (ItemNo)
			{
			case 0:
				strKey = L"dwUsbScan";
				break;
			case 1:
				strKey = L"dwStartUpScan";
				break;
			case 2:
				strKey = L"dwEnableSound";
				break;
			}
			break;
		case ID_SETTING_EMAIL:
			switch (ItemNo)
			{
			case 0:
				strKey = L"dwEmailScan";
				break;
			case 1:
				strKey = L"dwEnableVirusScan";
				m_Key = L"Software\\WardWiz Antivirus\\EmailScanSetting";
				break;
			case 2:
				strKey = L"dwEnableSpamFilter";
				m_Key = L"Software\\WardWiz Antivirus\\EmailScanSetting";
				break;
			case 3:
				strKey = L"dwEnableContentFilter";
				m_Key = L"Software\\WardWiz Antivirus\\EmailScanSetting";
				break;
			case 4:
				strKey = L"dwEnableSignature";
				m_Key = L"Software\\WardWiz Antivirus\\EmailScanSetting";
				break;
			}
			break;
		case ID_SETTING_UPDATE:
			switch (ItemNo)
			{
			case 0:
				strKey = L"dwAutoDefUpdate";
				break;
				/* its commented due to resloving coflict between UI  and system Tray  and  "Auto Product Update " menu from setting  tab is also remove  */

				/*case 1:
					strKey=L"dwAutoProductUpdate";
					break;*/

				//Varada Ikhar, Date: 13/04/2015
				// Issue:1.Go to WardWiz Settings. 2.Open Update in Settings. 3.2nd setting "Apply Update on StartUp" setting should be removed.
				//Commented the below case.
				/*case 1:
					strKey=L"dwUpdateReboot";
					break;*/
			}
			break;
		}


		DWORD dwType = REG_DWORD;
		DWORD returnDWORDValue;
		DWORD dwSize = sizeof(returnDWORDValue);
		HKEY hKey;
		returnDWORDValue = 0;
		LONG lResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_Key, 0, KEY_READ, &hKey);
		if (lResult == ERROR_SUCCESS)
			lResult = ::RegQueryValueEx(hKey, strKey, NULL, &dwType, (LPBYTE)&returnDWORDValue, &dwSize);
		if (lResult == ERROR_SUCCESS)
		{
			::RegCloseKey(hKey);
			return returnDWORDValue;
		}
		else
		{
			::RegCloseKey(hKey);
			return 0;
		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CHTMLListCtrl::ReadRegistryEntryofSetting", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : SendEnableEmailScanMessage2UI                                                     
*  Description    : Send message after enable/disable email scan 
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
bool CHTMLListCtrl::SendEnableEmailScanMessage2UI()
{
	__try
	{
		HWND hWindow = ::FindWindow( NULL,L"WRDWIZAVUI");
		::SendMessage(hWindow,CHECKEMAILSCANSTATUS,0,0);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CHTMLListCtrl::SendEnableEmailScanMessage2UI", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : SendEnableSecNews2UI                                                     
*  Description    : Send message enable/disable security news. 
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
bool CHTMLListCtrl::SendEnableSecNews2UI()
{
	__try
	{
		HWND hWindow = ::FindWindow( NULL,L"WRDWIZAVUI");
		::SendMessage(hWindow,CHECKSCROLLINGTEXT,0,0);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CHTMLListCtrl::SendEnableSecNews2UI", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;

}

/***************************************************************************************************                    
*  Function Name  : SendEmailPluginChange2Service                                                     
*  Description    : Send message to emailplugin
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
bool CHTMLListCtrl::SendEmailPluginChange2Service(int iMessageInfo, DWORD dwType,bool bEmailPluginWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	szPipeData.iMessageInfo = iMessageInfo;
	szPipeData.dwSecondValue = dwType;
	
	CISpyCommunicator objCom(EMAILPLUGIN_SERVER);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send Data in CSystemTray::SendEmailPluginChange2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bEmailPluginWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CSystemTray::SendEmailPluginChange2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
	
		if(szPipeData.dwValue != 1)
		{
			return false;
		}
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : GetExistingPathofOutlook                                                     
*  Description    : Get existing path of outlook from user machine. 
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
bool CHTMLListCtrl::GetExistingPathofOutlook()
{
	bool bReturn = false;
	try
	{
		LPTSTR ppszVersion;
		BOOL	pf64Bit = FALSE;
		HRESULT result;
		CString csActualGUIDs;
		CString csInstalledGUID;
		int iPos =0;
		result = GetOutlookVersionString(&ppszVersion,&pf64Bit);

		///if failed to retrive values means outlook is not installed
		if(FAILED(result))
		{
			AddLogEntry(L">>> EmailScanDlg : GetOutlookVersionString:outlook is not installed", 0, 0, true, ZEROLEVEL);
			// only for GetOutlookVersionString get failed for 2016
			// Hence It is remain disable. 
			if (CheckForOutllookExe())
			{
				return true;
			}
			return false;
		}

		for(int i = 0 ; i < 10 ; i++)
		{
			csActualGUIDs = g_GUIDofInstalledOutllookH[i];
			csInstalledGUID = (LPCTSTR)ppszVersion;
			csInstalledGUID = csInstalledGUID.Tokenize(_T("."),iPos);
			iPos =0;
			if(!(csActualGUIDs.Compare(csInstalledGUID)))
			{
				bReturn = true;
				//if(bReturn)
				//{
				//	//if(pf64Bit == 1)
				//	//{
				//	//	bReturn = false;
				//	//	break;
				//	//}
				//	break;
				//}
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXEmailScanDlg::GetExistingPathofOutlook", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************                    
*  Function Name  : GetOutlookVersionString                                                     
*  Description    : Get outlook version string
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
HRESULT CHTMLListCtrl::GetOutlookVersionString(LPTSTR* ppszVer, BOOL* pf64Bit)
{
    HRESULT hr = E_FAIL;
	LPTSTR pszTempPath = NULL;
	LPTSTR pszTempVer = NULL;

	__try
	{

		TCHAR pszOutlookQualifiedComponents[][MAX_PATH] = {
			TEXT("{E83B4360-C208-4325-9504-0D23003A74A5}"), // Outlook 2013
			TEXT("{1E77DE88-BCAB-4C37-B9E5-073AF52DFD7A}"), // Outlook 2010
			TEXT("{24AAE126-0911-478F-A019-07B875EB9996}"), // Outlook 2007
			TEXT("{BC174BAD-2F53-4855-A1D5-0D575C19B1EA}")  // Outlook 2003
		};
		int nOutlookQualifiedComponents = _countof(pszOutlookQualifiedComponents);
		int i = 0;
		DWORD dwValueBuf = 0;
		UINT ret = 0;

		*pf64Bit = FALSE;

		for (i = 0; i < nOutlookQualifiedComponents; i++)
		{
			ret = MsiProvideQualifiedComponent(
				pszOutlookQualifiedComponents[i],
				TEXT("outlook.x64.exe"),
				(DWORD) INSTALLMODE_DEFAULT,
				NULL,
				&dwValueBuf);
			if (ERROR_SUCCESS == ret) break;
		}

		if (ret != ERROR_SUCCESS)
		{
			for (i = 0; i < nOutlookQualifiedComponents; i++)
			{
				ret = MsiProvideQualifiedComponent(
					pszOutlookQualifiedComponents[i],
					TEXT("outlook.exe"),
					(DWORD) INSTALLMODE_DEFAULT,
					NULL,
					&dwValueBuf);
				if (ERROR_SUCCESS == ret) break;
			}
		}
		else
		{
			*pf64Bit = TRUE;
		}

		if (ret == ERROR_SUCCESS)
		{
			dwValueBuf += 1;
			pszTempPath = (LPTSTR) malloc(dwValueBuf * sizeof(TCHAR));
			if (pszTempPath != NULL)
			{
				if ((ret = MsiProvideQualifiedComponent(
					pszOutlookQualifiedComponents[i],
					TEXT("outlook.exe"),
					(DWORD) INSTALLMODE_EXISTING,
					pszTempPath,
					&dwValueBuf)) != ERROR_SUCCESS)
				{
					goto Error;
				}

				pszTempVer = (LPTSTR) malloc(MAX_PATH * sizeof(TCHAR));
				dwValueBuf = MAX_PATH;
				if ((ret = MsiGetFileVersion(pszTempPath,
					pszTempVer,
					&dwValueBuf,
					NULL,
					NULL))!= ERROR_SUCCESS)
				{
					goto Error;    
				}
				*ppszVer = pszTempVer;
				pszTempVer = NULL;
				hr = S_OK;
			}
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CGenXEmailScanDlg::GetOutlookVersionString", 0, 0, true, SECONDLEVEL);
	}

Error:
	if(pszTempVer)
	{
		free(pszTempVer);
	}
	if(pszTempPath)
	{
	    free(pszTempPath);
	}
	return hr;
}


/***********************************************************************************************                    
*  Function Name  : SendRequestCommon                                                     
*  Description    : Send request to auto live update services.
 
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 15- January -2015
*************************************************************************************************/
bool CHTMLListCtrl::SendRequestALUservToStopUpdate(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = {0};
		szPipeData.iMessageInfo = iRequest;
		szPipeData.dwValue = 2;

		CISpyCommunicator objCom(AUTOUPDATESRV_SERVER, false);
		if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data to AUTOUPDATESRV_SERVER in CHTMLListCtrl::SendRequestALUservToStopUpdate", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CHTMLListCtrl::SendRequestALUservToStopUpdate", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	CheckForOutllookExe()
*  Description    : It will check outlook exe default path.
*  SR.NO          :
*  Author Name    : Neha Gharge
*  Date           : 27th Aug, 2015
*********************************************************************************************************/
bool CHTMLListCtrl::CheckForOutllookExe()
{
	try
	{
		HKEY hKey;
		LONG ReadReg;
		TCHAR  szOutlookPath[1024];
		DWORD dwvalueSize = 1024;
		DWORD dwType = REG_SZ;
		if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\OUTLOOK.EXE"), &hKey) != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Unable to open registry key outlook exe", 0, 0, true, SECONDLEVEL);
			return false;
		}

		ReadReg = RegQueryValueEx(hKey, NULL, NULL, &dwType, (LPBYTE)&szOutlookPath, &dwvalueSize);
		if (ReadReg == ERROR_SUCCESS)
		{
			AddLogEntry(L">>> outlook default path %s", szOutlookPath, 0, true, FIRSTLEVEL);
			if (_tcscmp((LPCTSTR)szOutlookPath, L"") == 0)
			{
				RegCloseKey(hKey);
				return false;
			}
			else
			{
				RegCloseKey(hKey);
				return true;
			}
		}
		else
		{
			RegCloseKey(hKey);
			return false;
		}

		RegCloseKey(hKey);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExEmailScanDlg::CheckForOutllookExe()", 0, 0, true, SECONDLEVEL);
	}
	return true;
}