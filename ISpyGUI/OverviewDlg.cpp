// OverviewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISpyGUI.h"
#include "OverviewDlg.h"


// COverviewDlg dialog

IMPLEMENT_DYNAMIC(COverviewDlg, CDialog)

COverviewDlg::COverviewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COverviewDlg::IDD, pParent)
{

}

COverviewDlg::~COverviewDlg()
{
}

void COverviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(COverviewDlg, CDialog)
END_MESSAGE_MAP()


// COverviewDlg message handlers
