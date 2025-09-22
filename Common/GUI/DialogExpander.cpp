

// Expanding and collapsing dialogs
//
// Michael Walz (walz@epsitec.ch)
// Dec. 99
//
/***************************************************************************                      
   Program Name          : DialogExpander.cpp
   Description           : Expander and shrink dialog
   Author Name           : Neha Gharge                                                                        
   Date Of Creation      : 18th April,2014
   Version No            : 1.0.0.1
   Special Logic Used    : 
   Modification Log      :           
****************************************************************************/
#include "stdafx.h"
#include "DialogExpander.h"

/***************************************************************************************************                    
*  Function Name  : ShrinkDialog                                                     
*  Description    : Shrink dialog
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0222
*  Date           : 18th March,2014
****************************************************************************************************/
void ShrinkDialog(CWnd *pDlg, int idmark)
{   
  ASSERT(pDlg != NULL) ;  
  CWnd *pWndMark = pDlg->GetDlgItem(idmark) ;
  ASSERT(pWndMark != NULL) ;
  CRect markrect ;
  CRect dlgrect ;
  CRect clientrect ;
  CWnd *pParentWnd = pDlg->GetParent() ;
  int offset ;

  pDlg->GetClientRect(&clientrect) ;  // clientrect of the dialog
  pDlg->GetWindowRect(&dlgrect) ;	  // rectangle of the dialog window

  // get height of the title bar
  offset = dlgrect.Height() - clientrect.bottom ;

  pWndMark->GetWindowRect(&markrect) ;
 

  pDlg->ScreenToClient(&markrect) ;

  // calculate the new rectangle of the dialog window
  dlgrect.bottom = dlgrect.top + markrect.bottom + offset;

  pDlg->MoveWindow (dlgrect.left, dlgrect.top, dlgrect.Width(), dlgrect.Height()+ 40) ;

}

/***************************************************************************************************                    
*  Function Name  : CExpandDialog                                                     
*  Description    : C'tor
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0223
*  Date           : 18th March,2014
****************************************************************************************************/
CExpandDialog::CExpandDialog()
{
  m_bIsInitialized = FALSE ;
}

/***************************************************************************************************                    
*  Function Name  : Initialize                                                     
*  Description    : Initialize dialog with expanded form or in shrink form.
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0224
*  Date           : 18th March,2014
****************************************************************************************************/
void CExpandDialog::Initialize(CWnd *pDialog, BOOL bInitiallyExpanded,
						  int IdExpandButton, int IdCollapsedMark,
						  int IdCollapsedText)
{
  m_IdExpandButton = IdExpandButton;
  m_IdCollapsedMark = IdCollapsedMark;
  m_IdCollapsedText = IdCollapsedText;
  m_bIsInitialized = TRUE ;
  m_pDialog = pDialog ;
  m_bIsExpanded = TRUE ;

 
  m_pDialog->GetWindowRect(m_dialogrect) ;	  // rectangle of the dialog window
  m_pDialog->GetDlgItemText(IdCollapsedMark, m_sTextMore) ; 
  m_pDialog->GetDlgItemText(IdCollapsedText, m_sTextLess) ; 

  CWnd *pWndMark = m_pDialog->GetDlgItem(m_IdCollapsedText) ;
  pWndMark->ShowWindow(SW_HIDE);	// hide the "delimiting" control

  pWndMark = m_pDialog->GetDlgItem(m_IdCollapsedMark) ;
  pWndMark->ShowWindow(SW_HIDE);	// hide the "delimiting" control

  if (bInitiallyExpanded)
  {
	CWnd *pButton = m_pDialog->GetDlgItem(m_IdExpandButton) ;
	pButton->SetWindowText(m_sTextMore) ;
  }
  else
	OnExpandButton() ;
}

/***************************************************************************************************                    
*  Function Name  : OnExpandButton                                                     
*  Description    : On click on expand. Dialog get expanded.
*  Author Name    : Neha Gharge
*  SR_NO          : WRDWIZCOMMON_0225
*  Date           : 18th March,2014
****************************************************************************************************/
void CExpandDialog::OnExpandButton()
{
  ASSERT(m_bIsInitialized) ;

  CWnd *pButton ;
  m_bIsExpanded = !m_bIsExpanded ;

  if (m_bIsExpanded)
  {
	  // ShrinkDialog(m_pDialog, m_IdExpandedMark) ;
	  CRect rect ;
	  m_pDialog->GetWindowRect(&rect) ;
	rect.bottom = rect.top + m_dialogrect.Height()  ;
	rect.right = rect.left + m_dialogrect.Width()- 3 ;
	  m_pDialog->MoveWindow (&rect) ;
  }
  else
	  ShrinkDialog(m_pDialog, m_IdCollapsedMark ) ;
  
  pButton = m_pDialog->GetDlgItem(m_IdExpandButton) ;
 //issue no 1198 : gap should be in hide text in german language
  CString sTextLess;
  switch (m_dwSelectedLangID)
  {
  case 0 :
	  pButton->SetWindowText(!m_bIsExpanded ? m_sTextMore : m_sTextLess);
	  break;
  case 2:
	  sTextLess.Format(L"%s%s",L"     ",m_sTextLess);
	  pButton->SetWindowText(!m_bIsExpanded ? m_sTextMore : sTextLess);
	  break;
  default:
	  pButton->SetWindowText(!m_bIsExpanded ? m_sTextMore : m_sTextLess);
	  break;

  }
	
  

}


