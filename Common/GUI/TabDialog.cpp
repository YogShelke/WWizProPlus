/***************************************************************************                      
   Program Name          : TabDialog.cpp
   Description           : Making tab control on main Ui and setting dialog
   Author Name           : Neha Gharge                                                                       
   Date Of Creation      : 18th March,2014
   Version No            : 1.0.0.1
   Special Logic Used    : 
   Modification Log      : Nitin K          
****************************************************************************/
#include "stdafx.h"
#include "TabDialog.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************************************                    
*  Function Name  : CTabDialog                                                     
*  Description    : C'tor
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0200
*  Date           : 18th March,2014
****************************************************************************************************/
CTabDialog::CTabDialog(UINT nID, CWnd* pParent /* = NULL*/): CDialog(nID, pParent)
{
	try
	{
		m_nTotalPage = 0;
		m_iPreSelected = 0;
		m_iCurrentSelectedButton = 0;
		m_dlgType=0;
		m_dwEmailScanEnable = 0;
		m_bOutlookInstalled = false;
		m_bUnRegisterProduct = false;
		m_pToolTip = new CToolTipCtrlEx; //To Create Tooltip
		m_pToolTip->Create(this);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CTabDialog::CTabDialog"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : ~CTabDialog                                                     
*  Description    : D'tor
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0201
*  Date           : 18th March,2014
****************************************************************************************************/
CTabDialog::~CTabDialog()
{
	try
	{
		if(m_pToolTip)
		{
			delete m_pToolTip;
			m_pToolTip = NULL;
		}
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CTabDialog::~CTabDialog"), 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************                    
*  Function Name  : DoDataExchange                                                     
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0202
*  Date           : 18th March,2014
****************************************************************************************************/
void CTabDialog::DoDataExchange(CDataExchange* pDX)
{
	try
	{
		CDialog::DoDataExchange(pDX);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CTabDialog::DoDataExchange"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0203
*  Date           : 18th March,2014
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CTabDialog, CDialog)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


/***************************************************************************************************                    
*  Function Name  : PreTranslateMessage                                                     
*  Description    : To translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0204
*  Date           : 18th March,2014
****************************************************************************************************/
BOOL CTabDialog::PreTranslateMessage(MSG* pMsg)
{
	try
	{
		if(NULL != m_pToolTip)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
		if(pMsg->message == WM_KEYDOWN)
		{
			int nVirtKey = (int)pMsg->wParam;
			if(nVirtKey == VK_ESCAPE)
			{
				return TRUE;
			}
		}
		return CDialog::PreTranslateMessage(pMsg);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CRamOptimizerDlg::PreTranslateMessage"), 0, 0, true, SECONDLEVEL);
		return false;
	}
}

/***************************************************************************************************                    
*  Function Name  : AddPage                                                     
*  Description    : Add dialog and button into look up table
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0205
*  Date           : 18th March,2014
****************************************************************************************************/
BOOL CTabDialog::AddPage(CDialog* pDialog, CxSkinButton* pButton, int iStringId)
{
	try
	{
		ASSERT (pDialog != NULL);
		ASSERT (pButton != NULL);

		//Add dialog to dialog map
		m_DialogMap.SetAt(m_nTotalPage, pDialog);
		//add button according to button map
		m_ButtonMap.SetAt(m_nTotalPage, pButton);
		// add the tool tip
		AttachToolTip(*pButton, iStringId);
		//increase the total page
		m_nTotalPage ++;

		return TRUE;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CTabDialog::AddPage"), 0, 0, true, SECONDLEVEL);
		return false;
	}
}

/***************************************************************************************************                    
*  Function Name  : InitPagesShow                                                     
*  Description    : Initial dialog to be shown
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0206
*  Date           : 18th March,2014
****************************************************************************************************/
void CTabDialog::InitPagesShow()
{
	try
	{
		//Create Round rectangle here.
		CRect rect1;
		this->GetClientRect(rect1);
		CRgn		 rgn;
		rgn.CreateRoundRectRgn(rect1.left, rect1.top, rect1.right, rect1.bottom, 0,0);
		this->SetWindowRgn(rgn, TRUE);

		InitButtonsShow();
		InitDialogShow();
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CTabDialog::InitPagesShow"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : InitButtonsShow                                                     
*  Description    : Initial button to be shown
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0207
*  Date           : 18th March,2014
****************************************************************************************************/
void CTabDialog::InitButtonsShow()
{
	try
	{
		
		//int iXpos = 0;
		//int iYpos = 30;

	
		
		int iXpos = 0;
		int iYpos = 33;
		for(int i = 0; i < m_nTotalPage; i ++)
		{
			CxSkinButton* pButton = NULL;
			if(m_ButtonMap.Lookup(i, pButton) == FALSE)
			{
				return;
			}
			//Nitin: 31-May-2014, General Tab default selection
			if(m_dlgType)
			{
				CxSkinButton *pBtnGeneralOption = (CxSkinButton *)GetDlgItem(ID_BUTTON_GENERAL_OPTION);
				if(pBtnGeneralOption != NULL)
				{
						/*	ISSUE NO - 611 NAME - NITIN K. TIME - 5th June 2014 */
					pBtnGeneralOption->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION_HOVER, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_SETTING_OPTION_HOVER, 0, 0, 0, 0, 0);
					pBtnGeneralOption->SetTextColorA(RGB(4,4,4),RGB(4,4,4),RGB(4,4,4));
					pBtnGeneralOption->SetFocusTextColor(RGB(4,4,4));
					pBtnGeneralOption->Invalidate();
				}
			}
		}
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CTabDialog::InitButtonsShow"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : InitDialogShow                                                     
*  Description    : As per button selectiob,Dialog to be shown 
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0208
*  Date           : 18th March,2014
****************************************************************************************************/
void CTabDialog::InitDialogShow()
{
	try
	{
		CRect oRect;
		GetClientRect(&oRect);
		for(int i = 0; i < m_nTotalPage; i ++)
		{
			CDialog* pDialog = NULL;
			m_DialogMap.Lookup(i, pDialog);
			if(pDialog == NULL)
			{
				return;	
			}

			if(i == 0)
			{
				//pDialog->SetWindowPos(NULL, oRect.left + 150 , 0, 600, 406, SWP_NOZORDER);
				pDialog->SetWindowPos(NULL, 150 , 0 , m_iDlgCx, m_iDlgCy, SWP_NOZORDER);
				pDialog->ShowWindow(SW_SHOW);
			}
			else
			{
				//pDialog->SetWindowPos(NULL, oRect.left + 150  , 0, 600, 406, SWP_NOZORDER);				
				pDialog->SetWindowPos(NULL, 150 , 0 , m_iDlgCx, m_iDlgCy, SWP_NOZORDER);
				//pDialog->ShowWindow(SW_SHOW);
			}

		}
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CTabDialog::InitDialogShow"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : HideAllPages                                                     
*  Description    : Hides all dialogs.
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0209
*  Date           : 18th March,2014
****************************************************************************************************/
void CTabDialog::HideAllPages()
{
	try
	{
		for(int i = 0; i < m_nTotalPage; i ++)
		{
			CDialog* pDialog = NULL;
			m_DialogMap.Lookup(i, pDialog);
			pDialog->ShowWindow(SW_HIDE);
		}
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CTabDialog::HideAllPages"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : SetDialogType                                                     
*  Description    : Set dialog type
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0210
*  Date           : 18th March,2014
****************************************************************************************************/
void CTabDialog::SetDialogType(DWORD dwDlgType)
{
	m_dlgType = dwDlgType;
}

/***************************************************************************************************                    
*  Function Name  : OnCommand                                                     
*  Description    : The framework calls this member function when the user selects an item from a menu,
					when a child control sends a notification message, or when an accelerator keystroke is translated.
*  Author Name    : Neha Gharge ,NItin K
*  SR_NO          : WRDWIZCOMMON_0211
*  Date           : 18th March,2014
* Modification	 : 28th may 2015 Neha Gharge adding a utility button in tab dialog.
****************************************************************************************************/
BOOL CTabDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	//Issue No :503 Neha gharge 29/5/2014***********************/
	try
	{
		// rajil yadav 6/6/2014***********************/
		bool bIsProcessAborted = false;
		CxSkinButton *pBtnQuickScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_QUICK_SCAN);
		CxSkinButton *pBtnFullScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_FULL_SCAN);
		CxSkinButton *pBtnCustomScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_CUSTOM_SCAN);
		if(HIWORD(wParam) == BN_CLICKED)
		{
			for(int i = 0; i < m_nTotalPage; i ++)
			{
				CxSkinButton* pButton = NULL;
				m_ButtonMap.Lookup(i, pButton);
				if(pButton->m_hWnd == (HWND)lParam)
				{
					CDialog* pDialog = NULL;
					//get the current dilaog and display it
					if(m_DialogMap.Lookup(i, pDialog))
					{
						if(m_iPreSelected != i)
						{
							//SetSelectedSkin(static_cast<int>(wParam));
							//HideAllPages();
							CISpyGUIDlg* objTabCtrlupdateWindHandle = NULL;
							objTabCtrlupdateWindHandle = (CISpyGUIDlg*)AfxGetMainWnd();
							if(!m_dlgType)
							{
								// issue : every dlg getting redraw when we click again and again on same dlg
								//resolved by lalit kumawat 8-6-2015
								m_dwPrevSelectedButton = m_SelectedButton;
								m_SelectedButton = GetSelectedDialog(i);
								
								if (m_dwPrevSelectedButton == m_SelectedButton)
								{
									return CDialog::OnCommand(wParam, lParam);
								}

								bool bIsAnyProcessRunning = false;
								bIsAnyProcessRunning = objTabCtrlupdateWindHandle->IsAnyTaskInProcess();
								
								switch(m_SelectedButton)
								{
									case QUICK_SCAN_DLG:

										if (bIsAnyProcessRunning && QUICK_SCAN_DLG != objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											bIsProcessAborted= objTabCtrlupdateWindHandle->StopRunningProcess();
											if (bIsProcessAborted)
											{
												if (!((CScanDlg *)pDialog)->OnBnClickedBtnQuickscan())
												{
													if (pBtnQuickScan != NULL)
													{
														pBtnQuickScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
														pBtnQuickScan->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
														pBtnQuickScan->SetFocusTextColor(RGB(4, 4, 4));
													}
													Invalidate();
													return TRUE;
												}
											}
										}
										else if (bIsAnyProcessRunning && QUICK_SCAN_DLG == objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											objTabCtrlupdateWindHandle->m_iRunningProcessNmB = 0;
											break;
										}
										else
										{
											if (!((CScanDlg *)pDialog)->OnBnClickedBtnQuickscan())
											{
												if (pBtnQuickScan != NULL)
												{
													pBtnQuickScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
													pBtnQuickScan->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
													pBtnQuickScan->SetFocusTextColor(RGB(4, 4, 4));
												}
												Invalidate();
												//Issue No: 950 When threats are detected in any of the scan and if user click any other scan option without cleaning threats no action is taking place.
												//Resolved By : Nitin Kolapkar Date: 10th Nov 2015
												m_SelectedButton = 0;
												return TRUE;
											}
										}
										
										/*else
											((CScanDlg *)pDialog)->OnBnClickedBtnFullscan();*/
	
										break;
									case FULL_SCAN_DLG:

										if (bIsAnyProcessRunning && FULL_SCAN_DLG != objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											OutputDebugString(L">>> In WARDWIZ_UPDATES_DLG before stop.. ");
											bIsProcessAborted = objTabCtrlupdateWindHandle->StopRunningProcess();
											OutputDebugString(L">>> In WARDWIZ_UPDATES_DLG after stop.. ");
											if (bIsProcessAborted)
											{
												if (!((CScanDlg *)pDialog)->OnBnClickedBtnFullscan())
												{
													if (pBtnFullScan != NULL)
													{
														pBtnFullScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
														pBtnFullScan->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
														pBtnFullScan->SetFocusTextColor(RGB(4, 4, 4));
													}
													Invalidate();
													return TRUE;
												}
											}
										}
										else if (bIsAnyProcessRunning && FULL_SCAN_DLG == objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											objTabCtrlupdateWindHandle->m_iRunningProcessNmB = 0;
											break;
										}
										else
										{
											if (!((CScanDlg *)pDialog)->OnBnClickedBtnFullscan())
											{
												if (pBtnFullScan != NULL)
												{
													pBtnFullScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
													pBtnFullScan->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
													pBtnFullScan->SetFocusTextColor(RGB(4, 4, 4));
												}
												Invalidate();
												//Issue No: 950 When threats are detected in any of the scan and if user click any other scan option without cleaning threats no action is taking place.
												//Resolved By : Nitin Kolapkar Date: 10th Nov 2015
												m_SelectedButton = 0;
												return TRUE;
											}
										}

										/*else
										((CScanDlg *)pDialog)->OnBnClickedBtnFullscan();*/

										break;
									case CUSTOM_SCAN_DLG:

										if (bIsAnyProcessRunning && CUSTOM_SCAN_DLG != objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											bIsProcessAborted = objTabCtrlupdateWindHandle->StopRunningProcess();
											if (bIsProcessAborted)
											{
												if (!((CScanDlg *)pDialog)->OnBnClickedBtnCustomscan())
												{
													if (pBtnCustomScan != NULL)
													{
														pBtnCustomScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
														pBtnCustomScan->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
														pBtnCustomScan->SetFocusTextColor(RGB(4, 4, 4));
													}
													Invalidate();
													return TRUE;
												}
											}
										}
										else if (bIsAnyProcessRunning && CUSTOM_SCAN_DLG == objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											objTabCtrlupdateWindHandle->m_iRunningProcessNmB = 0;
											break;
										}
										else
										{
											if (!((CScanDlg *)pDialog)->OnBnClickedBtnCustomscan())
											{
												if (pBtnCustomScan != NULL)
												{
													pBtnCustomScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
													pBtnCustomScan->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
													pBtnCustomScan->SetFocusTextColor(RGB(4, 4, 4));
												}
												Invalidate();
												//Issue No: 950 When threats are detected in any of the scan and if user click any other scan option without cleaning threats no action is taking place.
												//Resolved By : Nitin Kolapkar Date: 10th Nov 2015
												m_SelectedButton = 0;
												return TRUE;
											}
										}
										
										
										break;
									case RECOVER_DLG:
										if (bIsAnyProcessRunning && RECOVER_DLG != objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											bIsProcessAborted = objTabCtrlupdateWindHandle->StopRunningProcess();
											if (bIsProcessAborted)
											{
												((CISpyRecoverDlg *)pDialog)->OnBnClickedRecover();
											}
										}
										else if (bIsAnyProcessRunning && RECOVER_DLG == objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											objTabCtrlupdateWindHandle->m_iRunningProcessNmB = 0;
											break;
										}
										else
										{
											((CISpyRecoverDlg *)pDialog)->OnBnClickedRecover();
										}
										
										break;
										//ISSUE NO:- 82 RY Date :- 21/5/2014
									case REGISTRY_OPTIMIZER_DLG:

										if (bIsAnyProcessRunning && REGISTRY_OPTIMIZER_DLG != objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											bIsProcessAborted = objTabCtrlupdateWindHandle->StopRunningProcess();
											if (bIsProcessAborted)
											{
												((CRegistryOptimizerDlg *)pDialog)->ResetRegistryScanOption();
												((CRegistryOptimizerDlg *)pDialog)->ResetControls();
											}
										}
										else if (bIsAnyProcessRunning && REGISTRY_OPTIMIZER_DLG == objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											objTabCtrlupdateWindHandle->m_iRunningProcessNmB = 0;
											break;
										}
										else
										{
											((CRegistryOptimizerDlg *)pDialog)->ResetRegistryScanOption();
											((CRegistryOptimizerDlg *)pDialog)->ResetControls();
										}
										break;	
									/*****************ISSUE NO -170 Neha Gharge 22/5/14 ***************/
									case EMAIL_VIRUS_SCAN_DLG:
										if(((CISpyEmailScanDlg *)pDialog)->OnBnClickedButtonVirusscan())
										{
											HideAllPages();
											//((CISpyEmailScanDlg *)pDialog)->OnBnClickedButtonVirusscan();
											((CISpyEmailScanDlg *)pDialog)->ShowWindow(SW_SHOW);
											pDialog->Invalidate();
											pDialog->GetParent()->Invalidate();
											m_iCurrentSelectedButton = m_SelectedButton;
											SetSelectedSkin(static_cast<int>(wParam));
											break;
										}
										else
										{
											return TRUE;
										}
										break;
									case EMAIL_SPAM_FILTER_DLG:
										if(((CISpyEmailScanDlg *)pDialog)->OnBnClickedButtonSpamfilter())
										{
											HideAllPages();
											//((CISpyEmailScanDlg *)pDialog)->OnBnClickedButtonSpamfilter();
											((CISpyEmailScanDlg *)pDialog)->ShowWindow(SW_SHOW);
											pDialog->Invalidate();
											pDialog->GetParent()->Invalidate();
											m_iCurrentSelectedButton = m_SelectedButton;
											SetSelectedSkin(static_cast<int>(wParam));
											break;
										}
										else
										{
											return TRUE;
										}
										break;								
									case EMAIL_CONTENT_FILTER_DLG:
										if(((CISpyEmailScanDlg *)pDialog)->OnBnClickedButtonContentfilter())
										{
											HideAllPages();
											//((CISpyEmailScanDlg *)pDialog)->OnBnClickedButtonContentfilter();
											((CISpyEmailScanDlg *)pDialog)->ShowWindow(SW_SHOW);
											pDialog->Invalidate();
											pDialog->GetParent()->Invalidate();
											m_iCurrentSelectedButton = m_SelectedButton;
											SetSelectedSkin(static_cast<int>(wParam));
											break;
										}
										else
										{
											return TRUE;
										}
										break;
									case EMAIL_SIGNATURE_DLG:
										if(((CISpyEmailScanDlg *)pDialog)->OnBnClickedButtonSignature())
										{
											HideAllPages();
											//((CISpyEmailScanDlg *)pDialog)->OnBnClickedButtonSignature();
											((CISpyEmailScanDlg *)pDialog)->ShowWindow(SW_SHOW);
											pDialog->Invalidate();
											pDialog->GetParent()->Invalidate();
											m_iCurrentSelectedButton = m_SelectedButton;
											SetSelectedSkin(static_cast<int>(wParam));
											break;
										}
										else
										{
											return TRUE;
										}
										break;
										//ISSUE NO:- 186 RY Date :- 21/5/2014
									case WARDWIZ_UPDATES_DLG :
										//((CISpyUpdatesDlg *)pDialog)->ShowPageToDisplay();
									/*	CISpyGUIDlg* g_TabCtrlupdateWindHandle = NULL;
										g_TabCtrlupdateWindHandle = (CISpyGUIDlg*)AfxGetMainWnd();*/
										OutputDebugString(L">>> In WARDWIZ_UPDATES_DLG before stop.. ");
										if (bIsAnyProcessRunning && WARDWIZ_UPDATES_DLG != objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											bIsProcessAborted = objTabCtrlupdateWindHandle->StopRunningProcess();
											OutputDebugString(L">>> In WARDWIZ_UPDATES_DLG after stop.. ");
											if (bIsProcessAborted)
											{
												
												((CISpyUpdatesDlg *)pDialog)->ShowPageToDisplay();
											}
										}
										else if (bIsAnyProcessRunning && WARDWIZ_UPDATES_DLG == objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											objTabCtrlupdateWindHandle->m_iRunningProcessNmB = 0;
											break;
										}
										else
										{
											((CISpyUpdatesDlg *)pDialog)->ShowPageToDisplay();
										}
										
										break;
									case WARDWIZ_REPORTS_DLG:

										if (bIsAnyProcessRunning && WARDWIZ_REPORTS_DLG != objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											bIsProcessAborted = objTabCtrlupdateWindHandle->StopRunningProcess();
											if (bIsProcessAborted)
											{
												((CISpyReportsDlg *)pDialog)->OnBnClickedReports();
											}
										}
										else if (bIsAnyProcessRunning && WARDWIZ_REPORTS_DLG == objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											objTabCtrlupdateWindHandle->m_iRunningProcessNmB = 0;
											break;
										}
										else
										{
											((CISpyReportsDlg *)pDialog)->OnBnClickedReports();
										}
										
										break;

									case ANTIROOTKIT_SCAN_DLG:
										if (bIsAnyProcessRunning && ANTIROOTKIT_SCAN_DLG != objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											bIsProcessAborted = objTabCtrlupdateWindHandle->StopRunningProcess();
											if (bIsProcessAborted)
											{
												((CISpyAntiRootkit *)pDialog)->RootKitUIDispalyedOnbasisOfFlags();
											}
										}
										else if (bIsAnyProcessRunning && ANTIROOTKIT_SCAN_DLG == objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											objTabCtrlupdateWindHandle->m_iRunningProcessNmB = 0;
											break;
										}
										else
										{
											((CISpyAntiRootkit *)pDialog)->RootKitUIDispalyedOnbasisOfFlags();
										}
										
										break;
									case FOLDER_LOCKER_DLG:
										((CISpyFolderLocker *)pDialog)->OnBnClickFolderLock();
										break;
									/* ISSUE NO - 205 NAME - NITIN K. TIME - 21st May 2014 :: 8 pm*/
									case DATA_ENCRYPTION_DLG:
										/*CISpyGUIDlg* g_TabCtrlupdateWindHandle = NULL;
										g_TabCtrlupdateWindHandle = (CISpyGUIDlg*)AfxGetMainWnd();*/
										if (bIsAnyProcessRunning && DATA_ENCRYPTION_DLG != objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											bIsProcessAborted = objTabCtrlupdateWindHandle->StopRunningProcess();
											if (bIsProcessAborted)
											{
												((CDataEncryptionDlg *)pDialog)->ResetControl(TRUE);
											}
										}
										else if (bIsAnyProcessRunning && DATA_ENCRYPTION_DLG == objTabCtrlupdateWindHandle->m_iRunningProcessNmB)
										{
											objTabCtrlupdateWindHandle->m_iRunningProcessNmB = 0;
											break;
										}
										else
										{
											((CDataEncryptionDlg *)pDialog)->ResetControl(TRUE);
										}
										

										 break;
									case WARDWIZ_UTILITY_DLG:
										HideAllPages();
										pDialog->ShowWindow(SW_SHOW);
										pDialog->Invalidate();
										pDialog->GetParent()->Invalidate();
										m_iCurrentSelectedButton = m_SelectedButton;
										SetSelectedSkin(static_cast<int>(wParam));
										break;
								}
								if (bIsProcessAborted || objTabCtrlupdateWindHandle->m_iRunningProcessNmB == 0)
								{
									HideAllPages();
									
									m_iCurrentSelectedButton = m_SelectedButton;
									SetSelectedSkin(static_cast<int>(wParam));
								}
								//Issue: 0000382 : Issue with settings tab in UI.
								//Resolved By : Nitin K. Date 14th May 2015
								if (bIsProcessAborted || objTabCtrlupdateWindHandle->m_iRunningProcessNmB == 0)
								{

									pDialog->ShowWindow(SW_SHOW);
									pDialog->Invalidate();
									pDialog->GetParent()->Invalidate();
									objTabCtrlupdateWindHandle->m_iRunningProcessNmB = 0;
									bIsProcessAborted = false;
								}
								else
								{
									// issue : when any operation running and we click on other optin and make it abort no ,now  againg clicking on same option it does not showing abort message box
									//resolved by lalit kumawat 18-8-2015
									//Issue no 1266. Even if the Email Scan feature is off, the email scan feature window remains open.
									if ((m_SelectedButton != EMAIL_VIRUS_SCAN_DLG && m_SelectedButton != EMAIL_SPAM_FILTER_DLG && m_SelectedButton != EMAIL_CONTENT_FILTER_DLG))
									{
										m_SelectedButton = 0;
									}
								}
							}
							else
							{
								/*	ISSUE NO - 643 NAME - NITIN K. TIME - 10th June 2014 */
								HideAllPages();
								m_SelectedButtonSetting = GetSelectedDialogSetting(i);
								switch(i)
								{
								case SETTINGS_GENERAL_DLG:
									//((CSettingsGeneralDlg *)pDialog)->OnBnClickedBtnQuickscan();
										break;
								case SETTINGS_SCAN_DLG:
									//	((CSettingsScanDlg *)pDialog)->OnBnClickedBtnFullscan();
										break;
								case SETTINGS_EMAIL_DLG:
									//((CSettingsEmailDlg *)pDialog)->OnBnClickedBtnCustomscan();
										break;
								case SETTINGS_UPDATE_DLG:
									//((CSettingsUpdateDlg *)pDialog)->LoadExistingRecoverFile(true);
										break;
								}
								m_iPreSelected = i;

								//Varada Ikhar, Date : 18-04-2015
								//Issue: 0000137 : 1. Open WardWiz UI. 2. Click on settings icon. 3. Select Updates, it is not getting highlighted unlike General and Scan.
								//m_iCurrentSelectedButtonSetting = i;
								m_iCurrentSelectedButtonSetting = m_SelectedButtonSetting;

								SetSelectedSkinSetting(static_cast<int>(wParam));	
								//Issue: 0000382 : Issue with settings tab in UI.
								//Resolved By : Nitin K. Date 14th May 2015
								/*if (bIsProcessAborted || objTabCtrlupdateWindHandle->m_iRunningProcessNmB == 0)
								{*/
								
									pDialog->ShowWindow(SW_SHOW);
									pDialog->Invalidate();
									pDialog->GetParent()->Invalidate();
									/*objTabCtrlupdateWindHandle->m_iRunningProcessNmB = 0;
									bIsProcessAborted = false;
								}*/
							}		
						}
						return TRUE;
					}
				}
			}
			return FALSE;
		}
		
		return CDialog::OnCommand(wParam, lParam);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CTabDialog::OnCommand"), 0, 0, true, SECONDLEVEL);
		return false;
	}
}

/***************************************************************************
  Function Name  : SetSelectedSkin
  Description    : Setskin on button on selection basis.
  Author Name    : Neha gharge
  Date           : 28th may 2014
  Modification   : 28th may 2014
  SR_NO			 : WRDWIZCOMMON_0212
  Modification	 : 28th may 2015 Neha Gharge adding a utility button in tab dialog.
****************************************************************************/
void CTabDialog::SetSelectedSkin(int iButton)
{
	CxSkinButton *pBtnQuickScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_QUICK_SCAN);
	CxSkinButton *pBtnFullScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_FULL_SCAN);
	CxSkinButton *pBtnCustomScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_CUSTOM_SCAN);
	CxSkinButton *pBtnRegOpt = (CxSkinButton *)GetDlgItem(ID_BUTTON_REGISTYOPTIMIZER);
	CxSkinButton *pBtnDataEncrypt = (CxSkinButton *)GetDlgItem(ID_BUTTON_DATAENCRYPTION);
	CxSkinButton *pBtnRecover = (CxSkinButton *)GetDlgItem(ID_BUTTON_RECOVER);
	CxSkinButton *pBtnFolderLocker = (CxSkinButton *)GetDlgItem(ID_BUTTON_FOLDERLOCKER);
	CxSkinButton *pBtnVirusscan = (CxSkinButton *)GetDlgItem(ID_BUTTON_VIRUSSCAN);
	CxSkinButton *pBtnSpamFilter = (CxSkinButton *)GetDlgItem(ID_BUTTON_SPAMFILTER);
	CxSkinButton *pBtnContentFilter = (CxSkinButton *)GetDlgItem(ID_BUTTON_CONTENTFILTER);
	CxSkinButton *pBtnSignature = (CxSkinButton *)GetDlgItem(ID_BUTTON_SIGNATURE);
	CxSkinButton *pBtnAntiroot = (CxSkinButton *)GetDlgItem(ID_BUTTON_ANTIROOTKIT);
	CxSkinButton *pBtnUpdate = (CxSkinButton *)GetDlgItem(ID_BUTTON_UPDATE);
	CxSkinButton *pBtnReports = (CxSkinButton *)GetDlgItem(ID_BUTTON_REPORTS);
	CxSkinButton *pBtnUtility = (CxSkinButton *)GetDlgItem(ID_BUTTON_UTILITY_OPTION);

	if(pBtnQuickScan != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnQuickScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnQuickScan->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
		pBtnQuickScan->SetFocusTextColor(RGB(4,4,4));
	}
	if(pBtnFullScan != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnFullScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnFullScan->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
		pBtnFullScan->SetFocusTextColor(RGB(4,4,4));
	}
	if(pBtnCustomScan != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnCustomScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnCustomScan->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
		pBtnCustomScan->SetFocusTextColor(RGB(4,4,4));
	}
	if(pBtnAntiroot != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnAntiroot->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnAntiroot->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
		pBtnAntiroot->SetFocusTextColor(RGB(4,4,4));
	}
	if(pBtnRegOpt != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnRegOpt->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnRegOpt->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
		pBtnRegOpt->SetFocusTextColor(RGB(4,4,4));
	}
	if(pBtnDataEncrypt != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnDataEncrypt->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnDataEncrypt->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
		pBtnDataEncrypt->SetFocusTextColor(RGB(4,4,4));
	}
	if(pBtnRecover != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnRecover->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnRecover->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
		pBtnRecover->SetFocusTextColor(RGB(4,4,4));
	}
	if(pBtnFolderLocker != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnFolderLocker->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnFolderLocker->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
		pBtnFolderLocker->SetFocusTextColor(RGB(4,4,4));
	}
	// ISsue no 691 neha gharge 13/6/2014
	if(m_dwEmailScanEnable == 1)
	{
		if(m_bOutlookInstalled == true)
		{
			if(m_bUnRegisterProduct == false)
			{
				if(pBtnVirusscan != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnVirusscan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnVirusscan->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
					pBtnVirusscan->SetFocusTextColor(RGB(4,4,4));
				}
				if(pBtnSpamFilter != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnSpamFilter->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnSpamFilter->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
					pBtnSpamFilter->SetFocusTextColor(RGB(4,4,4));
				}
				if(pBtnContentFilter != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnContentFilter->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnContentFilter->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
					pBtnContentFilter->SetFocusTextColor(RGB(4,4,4));
				}
				if(pBtnSignature != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnSignature->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnSignature->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
					pBtnSignature->SetFocusTextColor(RGB(4,4,4));
				}
			}
		}
	}
	if(pBtnUpdate != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnUpdate->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnUpdate->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
		pBtnUpdate->SetFocusTextColor(RGB(4,4,4));
	}
	if(pBtnReports != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnReports->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnReports->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);	
		pBtnReports->SetFocusTextColor(RGB(4,4,4));
	}
	if (pBtnUtility != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnUtility->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnUtility->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
		pBtnUtility->SetFocusTextColor(RGB(4, 4, 4));
	}
	Invalidate();
	switch(m_iCurrentSelectedButton)
	{
		case QUICK_SCAN_DLG:
				if(pBtnQuickScan != NULL)
				{	//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnQuickScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnQuickScan->SetTextColorA(RGB(4,4,4),0,1);
					pBtnQuickScan->SetFocusTextColor(RGB(4,4,4));
					pBtnQuickScan->Invalidate();
				}
				break;
		case FULL_SCAN_DLG:
				if(pBtnFullScan != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnFullScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnFullScan->SetTextColorA(RGB(4,4,4),0,1);
					pBtnFullScan->SetFocusTextColor(RGB(4,4,4));
					pBtnFullScan->Invalidate();
				}
				break;
		case CUSTOM_SCAN_DLG:
				if(pBtnCustomScan != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnCustomScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnCustomScan->SetTextColorA(RGB(4,4,4),0,1);
					pBtnCustomScan->SetFocusTextColor(RGB(4,4,4));
					pBtnCustomScan->Invalidate();
				}
				break;
		case ANTIROOTKIT_SCAN_DLG:
				if(pBtnAntiroot != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnAntiroot->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnAntiroot->SetTextColorA(RGB(4,4,4),0,1);
					pBtnAntiroot->SetFocusTextColor(RGB(4,4,4));
					pBtnAntiroot->Invalidate();
				}
				break;
		case REGISTRY_OPTIMIZER_DLG:
				if(pBtnRegOpt != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnRegOpt->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnRegOpt->SetTextColorA(RGB(4,4,4),0,1);
					pBtnRegOpt->SetFocusTextColor(RGB(4,4,4));
					pBtnRegOpt->Invalidate();
				}
				break;
		case DATA_ENCRYPTION_DLG:
				if(pBtnDataEncrypt != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnDataEncrypt->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnDataEncrypt->SetTextColorA(RGB(4,4,4),0,1);
					pBtnDataEncrypt->SetFocusTextColor(RGB(4,4,4));
					pBtnDataEncrypt->Invalidate();
				}
				break;
		case RECOVER_DLG:
				if(pBtnRecover != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnRecover->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnRecover->SetTextColorA(RGB(4,4,4),0,1);
					pBtnRecover->SetFocusTextColor(RGB(4,4,4));
					pBtnRecover->Invalidate();
				}
				break;
		case FOLDER_LOCKER_DLG:
				if(pBtnFolderLocker != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnFolderLocker->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnFolderLocker->SetTextColorA(RGB(4,4,4),0,1);
					pBtnFolderLocker->SetFocusTextColor(RGB(4,4,4));
					pBtnFolderLocker->Invalidate();
				}
				break;
		case EMAIL_VIRUS_SCAN_DLG:
				if(pBtnVirusscan != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnVirusscan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnVirusscan->SetTextColorA(RGB(4,4,4),0,1);
					pBtnVirusscan->SetFocusTextColor(RGB(4,4,4));
					pBtnVirusscan->Invalidate();
				}
				break;
		case EMAIL_SPAM_FILTER_DLG:
				if(pBtnSpamFilter != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnSpamFilter->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnSpamFilter->SetTextColorA(RGB(4,4,4),0,1);
					pBtnSpamFilter->SetFocusTextColor(RGB(4,4,4));
					pBtnSpamFilter->Invalidate();
				}
				break;
		case EMAIL_CONTENT_FILTER_DLG:
				if(pBtnContentFilter != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnContentFilter->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnContentFilter->SetTextColorA(RGB(4,4,4),0,1);
					pBtnContentFilter->SetFocusTextColor(RGB(4,4,4));
					pBtnContentFilter->Invalidate();
				}
				break;
		case EMAIL_SIGNATURE_DLG:
				if(pBtnSignature != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnSignature->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnSignature->SetTextColorA(RGB(4,4,4),0,1);
					pBtnSignature->SetFocusTextColor(RGB(4,4,4));
					pBtnSignature->Invalidate();
				}
				break;
		case WARDWIZ_UPDATES_DLG:
				if(pBtnUpdate != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnUpdate->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnUpdate->SetTextColorA(RGB(4,4,4),0,1);
					pBtnUpdate->SetFocusTextColor(RGB(4,4,4));
					pBtnUpdate->Invalidate();
				}
				break;
		case WARDWIZ_REPORTS_DLG:
				if(pBtnReports != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnReports->SetSkin(theApp.m_hResDLL,IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnReports->SetTextColorA(RGB(4,4,4),0,1);
					pBtnReports->SetFocusTextColor(RGB(4,4,4));
					pBtnReports->Invalidate();
				}
				break;
		case WARDWIZ_UTILITY_DLG:
			if (pBtnUtility!= NULL)
			{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
				pBtnUtility->SetSkin(theApp.m_hResDLL, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
				pBtnUtility->SetTextColorA(RGB(4, 4, 4), 0, 1);
				pBtnUtility->SetFocusTextColor(RGB(4, 4, 4));
				pBtnUtility->Invalidate();
			}
			break;
		default:
			AddLogEntry(L"### Invalid Option Selected in CTabDialog::SetSelectedSkin", 0, 0, true, SECONDLEVEL);
				break;
	}
}


/***************************************************************************
  Function Name  : SetSelectedSkinSettings
  Description    : Setskin on button on selection basis.
  Author Name    : Nitin K.
  Date           : 28th may 2014
  Modification   : 28th may 2014
  SR_NO			 : WRDWIZCOMMON_0213
****************************************************************************/
void CTabDialog::SetSelectedSkinSetting(int iButton)
{
	CxSkinButton *pBtnGeneralOption = (CxSkinButton *)GetDlgItem(ID_BUTTON_GENERAL_OPTION);
	CxSkinButton *pBtnScanOption = (CxSkinButton *)GetDlgItem(ID_BUTTON_SCAN_OPTION);
	CxSkinButton *pBtnEmailOption = (CxSkinButton *)GetDlgItem(ID_BUTTON_EMAIL_OPTION);
	CxSkinButton *pBtnUpdateOption = (CxSkinButton *)GetDlgItem(ID_BUTTON_UPDATE_OPTION);

	if(pBtnGeneralOption != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnGeneralOption->SetSkin(theApp.m_hResDLL, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnGeneralOption->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
		pBtnGeneralOption->SetFocusTextColor(RGB(4,4,4));
	}
	if(pBtnScanOption != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnScanOption->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnScanOption->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
		pBtnScanOption->SetFocusTextColor(RGB(4,4,4));
	}
	if(pBtnEmailOption != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnEmailOption->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnEmailOption->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
		pBtnEmailOption->SetFocusTextColor(RGB(4,4,4));
	}
	if(pBtnUpdateOption != NULL)
	{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		pBtnUpdateOption->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		pBtnUpdateOption->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
		pBtnUpdateOption->SetFocusTextColor(RGB(4,4,4));
	}

	/*	ISSUE NO - 611 NAME - NITIN K. TIME - 5th June 2014 */
	Invalidate();
	switch(m_iCurrentSelectedButtonSetting)
	{
		case SETTINGS_GENERAL_DLG:
				if(pBtnGeneralOption != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnGeneralOption->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION_HOVER, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnGeneralOption->SetTextColorA(RGB(4,4,4),0,1);
					pBtnGeneralOption->SetFocusTextColor(RGB(4,4,4));
					pBtnGeneralOption->Invalidate();
				}
				break;
		case SETTINGS_SCAN_DLG:
				if(pBtnScanOption != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnScanOption->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION_HOVER, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnScanOption->SetTextColorA(RGB(4,4,4),0,1);
					pBtnScanOption->SetFocusTextColor(RGB(4,4,4));
					pBtnScanOption->Invalidate();
				}
				break;
		case SETTINGS_EMAIL_DLG:
				if(pBtnEmailOption != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnEmailOption->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION_HOVER, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnEmailOption->SetTextColorA(RGB(4,4,4),0,1);
					pBtnEmailOption->SetFocusTextColor(RGB(4,4,4));
					pBtnEmailOption->Invalidate();
				}
				break;
		case SETTINGS_UPDATE_DLG:
				if(pBtnUpdateOption != NULL)
				{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
					pBtnUpdateOption->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION_HOVER, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
					pBtnUpdateOption->SetTextColorA(RGB(4,4,4),0,1);
					pBtnUpdateOption->SetFocusTextColor(RGB(4,4,4));
					pBtnUpdateOption->Invalidate();
				}
				break;

		default:
				AddLogEntry(L"### Invalid Option Selected in CTabDialog::SetSelectedSkinSetting", 0, 0, true, SECONDLEVEL);
				break;
	}
}

/***************************************************************************************************                    
*  Function Name  : OnPaint                                                     
*  Description    : The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0214
*  Date           : 18th March,2014
****************************************************************************************************/
void CTabDialog::OnPaint()
{
	try
	{
		CPaintDC dc(this);
		CRect oRcDlgRect;
		this->GetClientRect(&oRcDlgRect);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CTabDialog::OnPaint"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : OnEraseBkgnd                                                     
*  Description    : The framework calls this member function when the CWnd object background needs erasing (for example, when resized).
*  Author Name    : Neha Gharge 
*  SR_NO          : WRDWIZCOMMON_0215
*  Date           : 18th March,2014
****************************************************************************************************/
BOOL CTabDialog::OnEraseBkgnd(CDC* pDC)
{
	CPaintDC dc(this);
	CRect oRcDlgRect;
	this->GetClientRect(&oRcDlgRect);
	// plane center
	//dc.FillSolidRect(&oRcDlgRect, RGB(192, 192, 192));

	return FALSE;
}

/***************************************************************************************************                    
*  Function Name  : AttachToolTip                                                     
*  Description    : Attach tool tip to button.
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0216
*  Date           : 18th March,2014
****************************************************************************************************/
void CTabDialog::AttachToolTip(CxSkinButton &objButton, int iStringId)
{
	try
	{
		CString csStringToDisPlay;
		LoadString(NULL, iStringId, csStringToDisPlay.GetBuffer(MAX_PATH), MAX_PATH);

		if(m_pToolTip != NULL)
		{
			m_pToolTip->AddTool(&objButton, csStringToDisPlay);
		}

		csStringToDisPlay.ReleaseBuffer();
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CTabDialog::AttachToolTip"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : DisableAllBtn                                                     
*  Description    : Disable all buttons
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0217
*  Date           : 18th March,2014
* Modification	 : 28th may 2015 Neha Gharge adding a utility button in tab dialog.
****************************************************************************************************/
void CTabDialog::DisableAllBtn()
{
	CxSkinButton *pBtnQuickScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_QUICK_SCAN);
	CxSkinButton *pBtnFullScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_FULL_SCAN);
	CxSkinButton *pBtnCustomScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_CUSTOM_SCAN);
	CxSkinButton *pBtnRegOpt = (CxSkinButton *)GetDlgItem(ID_BUTTON_REGISTYOPTIMIZER);
	CxSkinButton *pBtnDataEncrypt = (CxSkinButton *)GetDlgItem(ID_BUTTON_DATAENCRYPTION);
	CxSkinButton *pBtnRecover = (CxSkinButton *)GetDlgItem(ID_BUTTON_RECOVER);
	CxSkinButton *pBtnFolderLocker = (CxSkinButton *)GetDlgItem(ID_BUTTON_FOLDERLOCKER);
	CxSkinButton *pBtnVirusscan = (CxSkinButton *)GetDlgItem(ID_BUTTON_VIRUSSCAN);
	CxSkinButton *pBtnSpamFilter = (CxSkinButton *)GetDlgItem(ID_BUTTON_SPAMFILTER);
	CxSkinButton *pBtnContentFilter = (CxSkinButton *)GetDlgItem(ID_BUTTON_CONTENTFILTER);
	CxSkinButton *pBtnSignature = (CxSkinButton *)GetDlgItem(ID_BUTTON_SIGNATURE);
	CxSkinButton *pBtnAntiroot = (CxSkinButton *)GetDlgItem(ID_BUTTON_ANTIROOTKIT);
	CxSkinButton *pBtnUpdate = (CxSkinButton *)GetDlgItem(ID_BUTTON_UPDATE);
	CxSkinButton *pBtnReports = (CxSkinButton *)GetDlgItem(ID_BUTTON_REPORTS);
	CxSkinButton *pBtnUtility = (CxSkinButton *)GetDlgItem(ID_BUTTON_UTILITY_OPTION);

	
	if(pBtnQuickScan != NULL)
	{
		pBtnQuickScan->SetTextAlignment(TA_LEFT ,1);
		pBtnQuickScan->EnableWindow(false);
	}
	if(pBtnFullScan != NULL)
	{
		pBtnFullScan->SetTextAlignment(TA_LEFT ,1);
		pBtnFullScan->EnableWindow(false);
	}
	if(pBtnCustomScan != NULL)
	{
		pBtnCustomScan->SetTextAlignment(TA_LEFT ,1);
		pBtnCustomScan->EnableWindow(false);
	}
	if(pBtnRegOpt != NULL)
	{
		pBtnRegOpt->SetTextAlignment(TA_LEFT ,1);
		pBtnRegOpt->EnableWindow(false);
	}
	if(pBtnDataEncrypt != NULL)
	{
		pBtnDataEncrypt->SetTextAlignment(TA_LEFT ,1);
		pBtnDataEncrypt->EnableWindow(false);
	}
	if(pBtnRecover != NULL)
	{
		pBtnRecover->SetTextAlignment(TA_LEFT ,1);
		pBtnRecover->EnableWindow(false);
	}
	if(pBtnFolderLocker != NULL)
	{
		pBtnFolderLocker->SetTextAlignment(TA_LEFT ,1);
		pBtnFolderLocker->EnableWindow(false);
	}
	if(pBtnVirusscan != NULL)
	{
		pBtnVirusscan->SetTextAlignment(TA_LEFT ,1);
		pBtnVirusscan->EnableWindow(false);
	}
	if(pBtnSpamFilter != NULL)
	{
		pBtnSpamFilter->SetTextAlignment(TA_LEFT ,1);
		pBtnSpamFilter->EnableWindow(false);
	}
	if(pBtnContentFilter != NULL)
	{
		pBtnContentFilter->SetTextAlignment(TA_LEFT ,1);
		pBtnContentFilter->EnableWindow(false);
	}
	if(pBtnSignature != NULL)
	{
		pBtnSignature->SetTextAlignment(TA_LEFT ,1);
		pBtnSignature->EnableWindow(false);
	}
	if(pBtnAntiroot != NULL)
	{
		pBtnAntiroot->SetTextAlignment(TA_LEFT ,1);
		pBtnAntiroot->EnableWindow(false);
	}
	if(pBtnUpdate != NULL)
	{
		pBtnUpdate->SetTextAlignment(TA_LEFT ,1);
		pBtnUpdate->EnableWindow(false);
	}
	if(pBtnReports != NULL)
	{
		pBtnReports->SetTextAlignment(TA_LEFT ,1);
		pBtnReports->EnableWindow(false);
	}
	if (pBtnUtility != NULL)
	{
		pBtnUtility->SetTextAlignment(TA_LEFT, 1);
		pBtnUtility->EnableWindow(false);
	}
}

/***************************************************************************************************                    
*  Function Name  : EnableAllBtn                                                     
*  Description    : Enable all buttons
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0218
*  Date           : 18th March,2014
****************************************************************************************************/
void CTabDialog::EnableAllBtn()
{
	CxSkinButton *pBtnQuickScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_QUICK_SCAN);
	CxSkinButton *pBtnFullScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_FULL_SCAN);
	CxSkinButton *pBtnCustomScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_CUSTOM_SCAN);
	CxSkinButton *pBtnRegOpt = (CxSkinButton *)GetDlgItem(ID_BUTTON_REGISTYOPTIMIZER);
	CxSkinButton *pBtnDataEncrypt = (CxSkinButton *)GetDlgItem(ID_BUTTON_DATAENCRYPTION);
	CxSkinButton *pBtnRecover = (CxSkinButton *)GetDlgItem(ID_BUTTON_RECOVER);
	CxSkinButton *pBtnFolderLocker = (CxSkinButton *)GetDlgItem(ID_BUTTON_FOLDERLOCKER);
	CxSkinButton *pBtnVirusscan = (CxSkinButton *)GetDlgItem(ID_BUTTON_VIRUSSCAN);
	CxSkinButton *pBtnSpamFilter = (CxSkinButton *)GetDlgItem(ID_BUTTON_SPAMFILTER);
	CxSkinButton *pBtnContentFilter = (CxSkinButton *)GetDlgItem(ID_BUTTON_CONTENTFILTER);
	CxSkinButton *pBtnSignature = (CxSkinButton *)GetDlgItem(ID_BUTTON_SIGNATURE);
	CxSkinButton *pBtnAntiroot = (CxSkinButton *)GetDlgItem(ID_BUTTON_ANTIROOTKIT);
	CxSkinButton *pBtnUpdate = (CxSkinButton *)GetDlgItem(ID_BUTTON_UPDATE);
	CxSkinButton *pBtnReports = (CxSkinButton *)GetDlgItem(ID_BUTTON_REPORTS);
	CxSkinButton *pBtnUtility = (CxSkinButton *)GetDlgItem(ID_BUTTON_UTILITY_OPTION);

	if(pBtnRegOpt != NULL)
	{
		pBtnRegOpt->EnableWindow(true);
	}
	if(pBtnDataEncrypt != NULL)
	{
		pBtnDataEncrypt->EnableWindow(true);
	}
	if(pBtnRecover != NULL)
	{
		pBtnRecover->EnableWindow(true);
	}
	if(pBtnFolderLocker != NULL)
	{
		pBtnFolderLocker->EnableWindow(true);
	}
	if(pBtnVirusscan != NULL)
	{
		pBtnVirusscan->EnableWindow(true);
	}
	if(pBtnSpamFilter != NULL)
	{
		pBtnSpamFilter->EnableWindow(true);
	}
	if(pBtnContentFilter != NULL)
	{
		pBtnContentFilter->EnableWindow(true);
	}
	if(pBtnSignature != NULL)
	{
		pBtnSignature->EnableWindow(true);
	}
	if(pBtnUpdate != NULL)
	{
		pBtnUpdate->EnableWindow(true);
	}
	if(pBtnReports != NULL)
	{
		pBtnReports->EnableWindow(true);
	}
	if (pBtnUtility != NULL)
	{
		pBtnUtility->EnableWindow(true);
	}
}

/***************************************************************************
  Function Name  : EnableAllExceptSelected
  Description    : Enables scan button options.
  Author Name    : Niranjan Deshak.
  Date           : 25 dec 2014
  Modification   : 25 dec 2014

****************************************************************************/
void CTabDialog::EnableAllExceptSelected()
{
	//Resolved Issue No. 30 While Cleaning Threats, Other scan options should be disabled.
	//-Niranjan Deshak. 25/12/2014. Added new Function

	CxSkinButton *pBtnQuickScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_QUICK_SCAN);
	CxSkinButton *pBtnFullScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_FULL_SCAN);
	CxSkinButton *pBtnCustomScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_CUSTOM_SCAN);
	CxSkinButton *pBtnAntiroot = (CxSkinButton *)GetDlgItem(ID_BUTTON_ANTIROOTKIT);
		
	switch(m_iCurrentSelectedButton)
	{
		case 1: if(pBtnQuickScan != NULL)
				{
					pBtnQuickScan->EnableWindow(true);
				}
				if(pBtnFullScan != NULL)
				{
					pBtnFullScan->EnableWindow(true);
				}
				if(pBtnCustomScan != NULL)
				{
					pBtnCustomScan->EnableWindow(true);
				}
				if(pBtnAntiroot != NULL)
				{
					pBtnAntiroot->EnableWindow(true);
				}
	
				break;
		case 2: if(pBtnFullScan != NULL)
				{
					pBtnFullScan->EnableWindow(true);
				}
				if(pBtnQuickScan != NULL)
				{
					pBtnQuickScan->EnableWindow(true);
				}
				if(pBtnCustomScan != NULL)
				{
					pBtnCustomScan->EnableWindow(true);
				}
				if(pBtnAntiroot != NULL)
				{
					pBtnAntiroot->EnableWindow(true);
				}
				break;
		case 3: if(pBtnCustomScan != NULL)
				{
					pBtnCustomScan->EnableWindow(true);
				}
				if(pBtnQuickScan != NULL)
				{
					pBtnQuickScan->EnableWindow(true);
				}
				if(pBtnFullScan != NULL)
				{
					pBtnFullScan->EnableWindow(true);
				}
				
				if(pBtnAntiroot != NULL)
				{
					pBtnAntiroot->EnableWindow(true);
				}
				break;
		case 4: if(pBtnAntiroot != NULL)
				{
					pBtnAntiroot->EnableWindow(true);
				}
				if(pBtnQuickScan != NULL)
				{
					pBtnQuickScan->EnableWindow(true);
				}
				if(pBtnFullScan != NULL)
				{
					pBtnFullScan->EnableWindow(true);
				}
				if(pBtnCustomScan != NULL)
				{
					pBtnCustomScan->EnableWindow(true);
				}
				break;
				default : 
			AddLogEntry(L"### Invalid  Invalid Selected button in CTabDialog::DisableAllExceptSelected", 0, 0, true, SECONDLEVEL);
				break;
	}

}


void CTabDialog::DisableAllExceptSelected()
{
	CxSkinButton *pBtnQuickScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_QUICK_SCAN);
	CxSkinButton *pBtnFullScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_FULL_SCAN);
	CxSkinButton *pBtnCustomScan = (CxSkinButton *)GetDlgItem(ID_BUTTON_CUSTOM_SCAN);
	CxSkinButton *pBtnRegOpt = (CxSkinButton *)GetDlgItem(ID_BUTTON_REGISTYOPTIMIZER);
	CxSkinButton *pBtnDataEncrypt = (CxSkinButton *)GetDlgItem(ID_BUTTON_DATAENCRYPTION);
	CxSkinButton *pBtnRecover = (CxSkinButton *)GetDlgItem(ID_BUTTON_RECOVER);
	CxSkinButton *pBtnFolderLocker = (CxSkinButton *)GetDlgItem(ID_BUTTON_FOLDERLOCKER);
	CxSkinButton *pBtnVirusscan = (CxSkinButton *)GetDlgItem(ID_BUTTON_VIRUSSCAN);
	CxSkinButton *pBtnSpamFilter = (CxSkinButton *)GetDlgItem(ID_BUTTON_SPAMFILTER);
	CxSkinButton *pBtnContentFilter = (CxSkinButton *)GetDlgItem(ID_BUTTON_CONTENTFILTER);
	CxSkinButton *pBtnSignature = (CxSkinButton *)GetDlgItem(ID_BUTTON_SIGNATURE);
	CxSkinButton *pBtnAntiroot = (CxSkinButton *)GetDlgItem(ID_BUTTON_ANTIROOTKIT);
	CxSkinButton *pBtnUpdate = (CxSkinButton *)GetDlgItem(ID_BUTTON_UPDATE);
	CxSkinButton *pBtnReports = (CxSkinButton *)GetDlgItem(ID_BUTTON_REPORTS);
	CxSkinButton *pBtnUtility = (CxSkinButton *)GetDlgItem(ID_BUTTON_UTILITY_OPTION);
//	DisableAllBtn();



	switch(m_iCurrentSelectedButton)
	{
		case 1: if(pBtnQuickScan != NULL)
				{
					pBtnQuickScan->EnableWindow(true);
				}
				if(pBtnFullScan != NULL)
				{
					pBtnFullScan->EnableWindow(false);
				}
				if(pBtnCustomScan != NULL)
				{
					pBtnCustomScan->EnableWindow(false);
				}
				if(pBtnAntiroot != NULL)
				{
					pBtnAntiroot->EnableWindow(false);
				}
	
				break;
		case 2: if(pBtnQuickScan != NULL)
				{
					pBtnQuickScan->EnableWindow(false);
				}
				if(pBtnFullScan != NULL)
				{
					pBtnFullScan->EnableWindow(true);
				}
				if(pBtnCustomScan != NULL)
				{
					pBtnCustomScan->EnableWindow(false);
				}
				if(pBtnAntiroot != NULL)
				{
					pBtnAntiroot->EnableWindow(false);
				}
				break;
		case 3: if(pBtnQuickScan != NULL)
				{
					pBtnQuickScan->EnableWindow(false);
				}
				if(pBtnFullScan != NULL)
				{
					pBtnFullScan->EnableWindow(false);
				}
				if(pBtnCustomScan != NULL)
				{
					pBtnCustomScan->EnableWindow(true);
				}
				if(pBtnAntiroot != NULL)
				{
					pBtnAntiroot->EnableWindow(false);
				}
				break;
		case 4: if(pBtnAntiroot != NULL)
				{
					pBtnAntiroot->EnableWindow(true);
				}
				if(pBtnQuickScan != NULL)
				{
					pBtnQuickScan->EnableWindow(false);
				}
				if(pBtnFullScan != NULL)
				{
					pBtnFullScan->EnableWindow(false);
				}
				if(pBtnCustomScan != NULL)
				{
					pBtnCustomScan->EnableWindow(false);
				}
				break;
		case 5: if(pBtnRegOpt != NULL)
				{
					pBtnRegOpt->EnableWindow(true);
				}
				break;
		case 6: if(pBtnDataEncrypt != NULL)
				{
					pBtnDataEncrypt->EnableWindow(true);
				}
				break;
		case 7: if(pBtnRecover != NULL)
				{
					pBtnRecover->EnableWindow(true);
				}
				break;
		case 8: if(pBtnFolderLocker != NULL)
				{
					pBtnFolderLocker->EnableWindow(true);
				}
				break;
		case 9: if(pBtnVirusscan != NULL)
				{
					pBtnVirusscan->EnableWindow(true);
				}
				break;
		case 10:if(pBtnSpamFilter != NULL)
				{
					pBtnSpamFilter->EnableWindow(true);
				}
				break;
		case 11:if(pBtnContentFilter != NULL)
				{
					pBtnContentFilter->EnableWindow(true);
				}
				break;
		case 12:if(pBtnSignature != NULL)
				{
					pBtnSignature->EnableWindow(true);
				}
				break;
		case 13:if(pBtnUpdate != NULL)
				{
					pBtnUpdate->EnableWindow(true);
				}
				break;
		case 14:if(pBtnReports != NULL)
				{
					pBtnReports->EnableWindow(true);
				}
				break;
		case 15:if (pBtnUtility != NULL)
				{
					pBtnUtility->EnableWindow(true);
				}
				break;
		default : 
			AddLogEntry(L"### Invalid  Invalid Selected button in CTabDialog::DisableAllExceptSelected", 0, 0, true, SECONDLEVEL);
				break;
	}

}

/***************************************************************************
  Function Name  : GetSelectedDialog
  Description    : Get selected dialog according to editions
  Author Name    : Neha gharge
  Date           : 28th may 2014
  Modification   : 28th may 2014
  SR_NO			 : WRDWIZCOMMON_0220
  Modification	 : 28th may 2015 Neha Gharge adding a utility button in tab dialog.
  Modification   : 27th June 2015, Neha Gharge shifting of menus accroding 1.12.0.0 designs
****************************************************************************/
DWORD CTabDialog::GetSelectedDialog(int i)
{
	DWORD dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();
	switch(dwProductID)
	{
		case BASIC:
					switch(i)
					{
						case 1: return QUICK_SCAN_DLG;
						case 2: return FULL_SCAN_DLG;
						case 3: return CUSTOM_SCAN_DLG;
						//case 4: return ANTIROOTKIT_SCAN_DLG;
						//case 5: return REGISTRY_OPTIMIZER_DLG;
						//case 6: return DATA_ENCRYPTION_DLG;
						case 4: return RECOVER_DLG;
						case 5:return WARDWIZ_UTILITY_DLG;
						case 6: return WARDWIZ_UPDATES_DLG;
						case 7: return WARDWIZ_REPORTS_DLG;
						
					}
		case ESSENTIAL:
					switch(i)
					{
						case 1: return QUICK_SCAN_DLG;
						case 2: return FULL_SCAN_DLG;
						case 3: return CUSTOM_SCAN_DLG;
						case 4: return ANTIROOTKIT_SCAN_DLG;
						case 5: return REGISTRY_OPTIMIZER_DLG;
						case 6: return DATA_ENCRYPTION_DLG;
						case 7: return RECOVER_DLG;
						case 8:return WARDWIZ_UTILITY_DLG;
						case 9: return WARDWIZ_UPDATES_DLG;
						case 10: return WARDWIZ_REPORTS_DLG;
						
					}
		case PRO:
					switch(i)
					{
						case 1: return QUICK_SCAN_DLG;
						case 2: return FULL_SCAN_DLG;
						case 3: return CUSTOM_SCAN_DLG;
						case 4: return ANTIROOTKIT_SCAN_DLG;
						case 5: return EMAIL_VIRUS_SCAN_DLG;
						case 6: return EMAIL_SPAM_FILTER_DLG;
						case 7: return EMAIL_CONTENT_FILTER_DLG;
						//case 8: return EMAIL_SIGNATURE_DLG;
						case 8: return REGISTRY_OPTIMIZER_DLG;
						case 9: return DATA_ENCRYPTION_DLG;
						case 10: return RECOVER_DLG;
						//case 12: return FOLDER_LOCKER_DLG;
						case 11:return WARDWIZ_UTILITY_DLG;
						case 12: return WARDWIZ_UPDATES_DLG;
						case 13: return WARDWIZ_REPORTS_DLG;
						
					}
		case ELITE:
					switch(i)
					{
						case 1: return QUICK_SCAN_DLG;
						case 2: return FULL_SCAN_DLG;
						case 3: return CUSTOM_SCAN_DLG;
						case 4: return ANTIROOTKIT_SCAN_DLG;
						case 5: return EMAIL_VIRUS_SCAN_DLG;
						case 6: return EMAIL_SPAM_FILTER_DLG;
						case 7: return EMAIL_CONTENT_FILTER_DLG;
						//case 8: return EMAIL_SIGNATURE_DLG;
						case 9: return REGISTRY_OPTIMIZER_DLG;
						case 10: return DATA_ENCRYPTION_DLG;
						case 11: return RECOVER_DLG;
						case 12: return FOLDER_LOCKER_DLG;
						case 13:return WARDWIZ_UTILITY_DLG;
						case 14: return WARDWIZ_UPDATES_DLG;
						case 15: return WARDWIZ_REPORTS_DLG;
						
					}
		default :	return 1;
	}
}


/***************************************************************************
  Function Name  : GetSelectedDialogSetting
  Description    : Get selected dialog according to editions
  Author Name    : Neha gharge
  Date           : 28th may 2014
  Modification   : 28th may 2014
  SR_NO			 : WRDWIZCOMMON_0221
****************************************************************************/
DWORD CTabDialog::GetSelectedDialogSetting(int i)
{
	DWORD dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();
	switch(dwProductID)
	{
		//Varada Ikhar, Date : 18-04-2015
		//Issue: 0000137 : 1. Open WardWiz UI. 2. Click on settings icon. 3. Select Updates, it is not getting highlighted unlike General and Scan.
		//Case numbering changed from 1,2,3,4 to 0,1,2,3
		case ESSENTIAL:
					switch(i)
					{
						case 0: return SETTINGS_GENERAL_DLG;
						case 1: return SETTINGS_SCAN_DLG;
						case 2: return SETTINGS_UPDATE_DLG;
					}
		case PRO:
					switch(i)
					{
						case 0: return SETTINGS_GENERAL_DLG;
						case 1: return SETTINGS_SCAN_DLG;
						case 2: return SETTINGS_EMAIL_DLG;
						case 3: return SETTINGS_UPDATE_DLG;
						
					}
		case ELITE:
					switch(i)
					{
						case 0: return SETTINGS_GENERAL_DLG;
						case 1: return SETTINGS_SCAN_DLG;
						case 2: return SETTINGS_EMAIL_DLG;
						case 3: return SETTINGS_UPDATE_DLG;
					}
		//Varada Ikhar, Date : 18-04-2015
		//Issue: 0000137 : 1. Open WardWiz UI. 2. Click on settings icon. 3. Select Updates, it is not getting highlighted unlike General and Scan.
		// Added the case for ProductID = BASIC
		case BASIC:
					switch (i)
					{
						case 0: return SETTINGS_GENERAL_DLG;
						case 1: return SETTINGS_SCAN_DLG;
						case 2: return SETTINGS_UPDATE_DLG;
					}
		default :	return 1;
	}
}

/***************************************************************************
Function Name  : IsEmailScanDlgActive
Description    : it provides the functionality to check whether active dlg is email scan/spam filter,content filter/email virus scan
Author Name    : Lalit kumawat
Date           : 8-6-2015
Modification   : 
SR_NO			 : 
****************************************************************************/
bool CTabDialog::IsEmailScanDlgActive()
{
	if (m_SelectedButton == EMAIL_VIRUS_SCAN_DLG || m_SelectedButton == EMAIL_SPAM_FILTER_DLG || m_SelectedButton == EMAIL_CONTENT_FILTER_DLG)
	{
		return true;
	}

	return false;
}

/***************************************************************************
Function Name  : SetSelectedButton
Description    : Set preference on button on selection basis.
Author Name    : Nitin K
Date           : 3rd Dec 2015
SR_NO		   : 
****************************************************************************/
void CTabDialog::SetSelectedButton(DWORD dwButton)
{
	m_SelectedButton = dwButton;
}