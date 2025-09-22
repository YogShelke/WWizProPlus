// CustomScanDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISpyGUI.h"
#include "CustomScanDlg.h"


// CCustomScanDlg dialog

IMPLEMENT_DYNAMIC(CCustomScanDlg, CDialog)

CCustomScanDlg::CCustomScanDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCustomScanDlg::IDD, pParent)
{

}

CCustomScanDlg::~CCustomScanDlg()
{
}

void CCustomScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCustomScanDlg, CDialog)
END_MESSAGE_MAP()


// CCustomScanDlg message handlers
