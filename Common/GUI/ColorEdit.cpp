/***************************************************************************                      
   Program Name          : ColorEdit.cpp
   Description           : Customize color and text of edit control in dialog
   Author Name           :                                                                         
   Date Of Creation      : 18th Oct,2013
   Version No            : 1.0.0.1
   Special Logic Used    : 
   Modification Log      :           
****************************************************************************/
#include "stdafx.h"
#include "ColorEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorEdit
/***************************************************************************************************                    
*  Function Name  : CColorEdit                                                     
*  Description    : C'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0104
*  Date           : 18 Oct 2013
****************************************************************************************************/
CColorEdit::CColorEdit()
{
	m_crBkColor = ::GetSysColor(COLOR_3DFACE); // Initializing background color to the system face color.
	m_crTextColor = BLACK; // Initializing text color to black
	m_brBkgnd.CreateSolidBrush(m_crBkColor); // Creating the Brush Color For the Edit Box Background
}

/***************************************************************************************************                    
*  Function Name  : CColorEdit                                                     
*  Description    : D'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0105
*  Date           : 18 Oct 2013
****************************************************************************************************/
CColorEdit::~CColorEdit()
{
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0106
*  Date           : 18 Oct 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CColorEdit, CEdit)
	//{{AFX_MSG_MAP(CColorEdit)
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorEdit message handlers

/***************************************************************************************************                    
*  Function Name  : SetTextColor                                                     
*  Description    : Set text color
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0107
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CColorEdit::SetTextColor(COLORREF crColor)
{
	m_crTextColor = crColor; // Passing the value passed by the dialog to the member varaible for Text Color
	RedrawWindow();
}

/***************************************************************************************************                    
*  Function Name  : SetBkColor                                                     
*  Description    : Set background color
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0108
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CColorEdit::SetBkColor(COLORREF crColor)
{
	m_crBkColor = crColor; // Passing the value passed by the dialog to the member varaible for Backgound Color
	m_brBkgnd.DeleteObject(); // Deleting any Previous Brush Colors if any existed.
	m_brBkgnd.CreateSolidBrush(crColor); // Creating the Brush Color For the Edit Box Background
	RedrawWindow();
}

/***************************************************************************************************                    
*  Function Name  : CtlColor                                                     
*  Description    : The framework calls this member function when a child control is about to be drawn.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0109
*  Date           : 18 Oct 2013
****************************************************************************************************/
HBRUSH CColorEdit::CtlColor(CDC* pDC, UINT nCtlColor)
{
	HBRUSH hbr;
	hbr = (HBRUSH)m_brBkgnd; // Passing a Handle to the Brush
	pDC->SetBkColor(m_crBkColor); // Setting the Color of the Text Background to the one passed by the Dialog
	pDC->SetTextColor(m_crTextColor); // Setting the Text Color to the one Passed by the Dialog

	if (nCtlColor)       // To get rid of compiler warning
      nCtlColor += 0;

	return hbr;
}

/***************************************************************************************************                    
*  Function Name  : SetReadOnly                                                     
*  Description    : Set background color on the basis of flag
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0110
*  Date           : 18 Oct 2013
****************************************************************************************************/
BOOL CColorEdit::SetReadOnly(BOOL flag)
{
   if (flag == TRUE)
      SetBkColor(m_crBkColor);
   else
      SetBkColor(WHITE);

   return CEdit::SetReadOnly(flag);
}

