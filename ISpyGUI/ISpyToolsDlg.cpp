/**************************************************************************************************
*  Program Name: ISpyToolsDlg.cpp                                                                                                    
*  Description: Provides utilities to increase system performance
                -> Registry Optimizer
				-> Data Encryption
*  Author Name: Neha Gharge
*  Date Of Creation: 20 Nov 2013
*  Version No: 1.0.0.2
**************************************************************************************************/
#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "ISpyToolsDlg.h"

IMPLEMENT_DYNAMIC(CISpyToolsDlg, CDialog)

/**********************************************************************************************************                *  Function Name  :	CISpyToolsDlg                                                     
*  Description    :	C'tor
*  Author Name    : Neha                                                                                         
*  Date           : 20 Nov 2013
**********************************************************************************************************/
CISpyToolsDlg::CISpyToolsDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CISpyToolsDlg::IDD, pParent)
{
}

/**********************************************************************************************************                *  Function Name  :	~CISpyToolsDlg                                                     
*  Description    :	Destructor
*  Author Name    : Neha                                                                                         
*  Date           : 20 Nov 2013
**********************************************************************************************************/
CISpyToolsDlg::~CISpyToolsDlg()
{
}

/**********************************************************************************************************                *  Function Name  :	DoDataExchange                                                     
*  Description    :	Called by the framework to exchange and validate dialog data.
*  Author Name    : Neha
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyToolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_REGISTRYOPT, m_btnRegistryOpt);
	DDX_Control(pDX, IDC_BUTTON_DATAENCRYPT, m_btnDataEncrypt);
	DDX_Control(pDX, IDC_BUTTON_TOOLBACK, m_btnToolBack);
	DDX_Control(pDX, IDC_PIC_ENCRYPT, m_stToolHeader);
	DDX_Control(pDX, IDC_BUTTON_RECOVER, m_btnRecover);
	DDX_Control(pDX, IDC_BUTTON_FOLDERLOCK, m_btnFolderLocker);
}


BEGIN_MESSAGE_MAP(CISpyToolsDlg, CJpegDialog)
	ON_BN_CLICKED(IDC_BUTTON_REGISTRYOPT, &CISpyToolsDlg::OnBnClickedButtonRegistryopt)
	ON_BN_CLICKED(IDC_BUTTON_DATAENCRYPT, &CISpyToolsDlg::OnBnClickedButtonDataencrypt)
	ON_BN_CLICKED(IDC_BUTTON_TOOLBACK, &CISpyToolsDlg::OnBnClickedButtonToolback)
	ON_WM_SETCURSOR()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_BUTTON_RECOVER, &CISpyToolsDlg::OnBnClickedButtonRecover)
	ON_BN_CLICKED(IDC_BUTTON_FOLDERLOCK, &CISpyToolsDlg::OnBnClickedButtonFolderlock)
END_MESSAGE_MAP()


/**********************************************************************************************************                *  Function Name  :	OnInitDialog                                                     
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all					Microsoft 
					Foundation Class Library dialog boxes
*  Author Name    : Neha
*  Date           : 20 Nov 2013
**********************************************************************************************************/
BOOL CISpyToolsDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	if(!Load(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_JPG_TOOLBG), _T("JPG")))
	{
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}

	Draw();
	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));
	CRect rect1;
	this->GetClientRect(rect1);
	SetWindowPos(NULL, 1, 88, rect1.Width()-5, rect1.Height() - 5, SWP_NOREDRAW);
	m_bmpToolHeader.LoadBitmapW(IDB_BITMAP_TOOLHEADER);
	m_stToolHeader.SetBitmap(m_bmpToolHeader);
	m_stToolHeader.SetWindowPos(&wndTop,rect1.left +43,12,550,50, SWP_NOREDRAW);

	m_btnRegistryOpt.SetSkin(IDB_BITMAP_REGISTRYOPT,IDB_BITMAP_REGISTRYOPT,IDB_BITMAP_REGISRTYOPT_MOVER,IDB_BITMAP_REGISTRYOPT,0,0,0,0);
	m_btnRegistryOpt.SetWindowPos(&wndTop, rect1.left +80, 140, 120,120, SWP_NOREDRAW);

	m_btnDataEncrypt.SetSkin(IDB_BITMAP_DATAENCRYPT,IDB_BITMAP_DATAENCRYPT,IDB_BITMAP_DATAENCRYPT_MOVER,IDB_BITMAP_DATAENCRYPT,0,0,0,0);
	m_btnDataEncrypt.SetWindowPos(&wndTop, rect1.left +235,140,120,120, SWP_NOREDRAW);

	m_btnRecover.SetSkin(IDB_BITMAP_RECOVER,IDB_BITMAP_RECOVER,IDB_BITMAP_HRECOVER,IDB_BITMAP_RECOVER,0,0,0,0);
	m_btnRecover.SetWindowPos(&wndTop, rect1.left +390,140,120,120, SWP_NOREDRAW);

	m_btnFolderLocker.SetSkin(IDB_BITMAP_FOLDERLOCK,IDB_BITMAP_FOLDERLOCK,IDB_BITMAP_HFOLDERLOCK,IDB_BITMAP_FOLDERLOCK,0,0,0,0);
	m_btnFolderLocker.SetWindowPos(&wndTop, rect1.left +545,140,120,120, SWP_NOREDRAW);

	m_btnToolBack.SetSkin(IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROWDISABLE,0,0,0,0);
	m_btnToolBack.SetWindowPos(&wndTop, rect1.left + 21, 354, 31,32, SWP_NOREDRAW);
	return TRUE;  // return TRUE unless you set the focus to a control
}

/**********************************************************************************************************                *  Function Name  :	OnBnClickedButtonRegistryopt                                                     
*  Description    :	Registry Optimizer selection
*  Author Name    : Neha
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyToolsDlg::OnBnClickedButtonRegistryopt()
{
}

/**********************************************************************************************************                *  Function Name  :	OnBnClickedButtonDataencrypt                                                     
*  Description    :	Data Encryption selection
*  Author Name    : Neha
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyToolsDlg::OnBnClickedButtonDataencrypt()
{
	ShowHideToolsDlgControls(false);
}

/**********************************************************************************************************                *  Function Name  :	ShowHideToolsDlgControls                                                     
*  Description    :	Resets controls on back button click
*  Author Name    : Neha
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyToolsDlg::ShowHideToolsDlgControls(bool bEnable)
{
}

/**********************************************************************************************************                *  Function Name  :	OnBnClickedButtonToolback                                                     
*  Description    :	Allows to go back to parent dialog & also resets controls
*  Author Name    : Neha
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyToolsDlg::OnBnClickedButtonToolback()
{
}

/**********************************************************************************************************                *  Function Name  :	OnSetCursor                                                     
*  Description    :	The framework calls this member function if mouse input is not captured and the mouse causes cursor movement within 
					the CWnd object.
*  Author Name    : Neha                                                                                        
*  Date           : 20 Nov 2013
**********************************************************************************************************/
BOOL CISpyToolsDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( 
		iCtrlID == IDC_BUTTON_REGISTRYOPT	||
		iCtrlID == IDC_BUTTON_TOOLBACK		||
		iCtrlID == IDC_BUTTON_DATAENCRYPT)
	{
		CString csClassName;
		::GetClassName(pWnd->GetSafeHwnd(), csClassName.GetBuffer(80), 80);
		if(csClassName == _T("Button") && m_hButtonCursor)
		{
			::SetCursor(m_hButtonCursor);
			return TRUE;
		}
	}
	return CJpegDialog::OnSetCursor(pWnd, nHitTest, message);
}

/**********************************************************************************************************                *  Function Name  :	OnNcHitTest                                                     
*  Description    :	The framework calls this member function for the CWnd object that contains the cursor every time the mouse is moved.
*  Author Name    : Neha                                                                                         
*  Date           : 20 Nov 2013
**********************************************************************************************************/
LRESULT CISpyToolsDlg::OnNcHitTest(CPoint point)
{
	return HTNOWHERE;
}

/**********************************************************************************************************                *  Function Name  :	HideChildWindows                                                     
*  Description    :	Hides child windows of ISpyToolsDlg 
*  Author Name    : Neha                                                                                         
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyToolsDlg::HideChildWindows()
{
}

void CISpyToolsDlg::OnBnClickedButtonRecover()
{
}

BOOL CISpyToolsDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

void CISpyToolsDlg::OnBnClickedButtonFolderlock()
{
}
