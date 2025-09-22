// MigFailedDlg1.cpp : implementation file
//

#include "stdafx.h"
#include "MigFailedDlg.h"
#include "afxdialogex.h"


// CMigFailedDlg dialog

IMPLEMENT_DYNAMIC(CMigFailedDlg, CDialogEx)

CMigFailedDlg::CMigFailedDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMigFailedDlg::IDD, pParent)
{

}

CMigFailedDlg::~CMigFailedDlg()
{
}

void CMigFailedDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMigFailedDlg, CDialogEx)
	ON_BN_CLICKED(ID_CANCEL, &CMigFailedDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_MFCLINK_WEB_PAGE, &CMigFailedDlg::OnBnClickedMfclinkWebPage)
END_MESSAGE_MAP()


// CMigFailedDlg message handlers


void CMigFailedDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}


void CMigFailedDlg::OnBnClickedMfclinkWebPage()
{
	ShellExecute(NULL, L"open", L"https://vibranium.co.in/get-new-key-using-your-old-key", NULL, NULL, SW_SHOWNORMAL);
}
