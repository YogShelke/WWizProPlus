
// WardWizDBUpgradeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "WardWizDBUpgrade.h"
#include "WardWizDBUpgradeDlg.h"
#include "WrdwizEncDecManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWardWizDBUpgradeDlg dialog



CWardWizDBUpgradeDlg::CWardWizDBUpgradeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWardWizDBUpgradeDlg::IDD, pParent)
	, m_lpszBuffer(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CWardWizDBUpgradeDlg::~CWardWizDBUpgradeDlg()
{
	if (m_lpszBuffer != NULL)
	{
		delete[]m_lpszBuffer;
		m_lpszBuffer = NULL;
	}
	
}

void CWardWizDBUpgradeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_STATUS, m_editStatus);
	DDX_Control(pDX, IDC_EDIT_FILEPATH, m_editFilePath);
	DDX_Control(pDX, IDC_STATIC_TOTALSIGCOUNT, m_stTotalSigCount);
	DDX_Control(pDX, IDC_BUTTON_START, m_btnStart);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EDIT_VERSIONNO, m_editVersionNo);
	DDX_Control(pDX, IDC_EDIT_DBSIGNATURE, m_editDBSignature);
}

BEGIN_MESSAGE_MAP(CWardWizDBUpgradeDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CWardWizDBUpgradeDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_START, &CWardWizDBUpgradeDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CWardWizDBUpgradeDlg::OnBnClickedButtonCancel)
END_MESSAGE_MAP()


// CWardWizDBUpgradeDlg message handlers

BOOL CWardWizDBUpgradeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	UpdateStatus(L"Please select Old DB file which you want to convert to newer version...");
	m_editVersionNo.SetWindowTextW(L"1.1.0.0");
	m_editDBSignature.SetWindowTextW(L"WRDWIZDB05");


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWardWizDBUpgradeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWardWizDBUpgradeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
	
void CWardWizDBUpgradeDlg::OnBnClickedButtonBrowse()
{
	try
	{
		static TCHAR szEncFilter[] = L"All Files(*.*)|*.*|";
		CFileDialog FileDlg(TRUE, L"All Files(*.*)", NULL, OFN_ALLOWMULTISELECT, (LPCTSTR)szEncFilter);
		if (FileDlg.DoModal() == IDOK)
		{
			CString csFilePath = FileDlg.GetPathName();
			m_editFilePath.SetWindowText(csFilePath);
			UpdateStatus(L"Please click on start button to convert...");
		}
	}
	catch (...)
	{
	}
}

void CWardWizDBUpgradeDlg::OnBnClickedButtonStart()
{
	CString csFilePath, csVersionNo, csDBSignature, csSigCount;
	m_editFilePath.GetWindowTextW(csFilePath);
	m_editVersionNo.GetWindowTextW(csVersionNo);
	m_editDBSignature.GetWindowTextW(csDBSignature);

	if (!PathFileExists(csFilePath))
	{
		UpdateStatus(L"Invalid file path, please try again...");
		AfxMessageBox(L"Invalid file path, please try again");
		return;
	}

	if (csVersionNo.Trim().GetLength() == 0)
	{
		UpdateStatus(L"Please enter version number to add in database...");
		AfxMessageBox(L"Please enter version number to add in database");
		return;
	}

	if (csDBSignature.Trim().GetLength() == 0)
	{
		UpdateStatus(L"Please enter DB Signature ...");
		AfxMessageBox(L"Please enter DB Signature");
		return;
	}


	bool bReturn = false;

	DWORD dwMajorVersionNo = 0x00;
	DWORD dwVersionLength = 0x00;
	TCHAR szDecryptedFilePath[MAX_PATH] = { 0 };
	CWrdwizEncDecManager        objWrdwizEncDecManager;
	if (!objWrdwizEncDecManager.DecryptDBFileData(csFilePath, szDecryptedFilePath, dwMajorVersionNo, dwVersionLength))
	{
		AddLogEntry(L"### Failed to call DecryptDBFileData in CWardWizDBUpgradeDlg::OnBnClickedButtonStart, File: %s", csFilePath, 0, true, SECONDLEVEL);
		return;
	}

	if (!objWrdwizEncDecManager.LoadContentFromFile(szDecryptedFilePath))
	{
		AddLogEntry(L"### Failed to call LoadContentFromFile in LoadMD5DatabaseSEH MD5DLL, File: %s", szDecryptedFilePath, 0, true, SECONDLEVEL);
		return;
	}

	//Delete temporary created file
	if (PathFileExists(szDecryptedFilePath))
	{
		SetFileAttributes(szDecryptedFilePath, FILE_ATTRIBUTE_NORMAL);
		if (!DeleteFile(szDecryptedFilePath))
		{
			AddLogEntry(L"### Failed to DeleteFile in CMD5ScanDLLApp::LoadMD5DatabaseSEH, File [%s]", szDecryptedFilePath, 0, true, SECONDLEVEL);
		}
	}

	DWORD dwCount = static_cast<DWORD>(objWrdwizEncDecManager.m_objEncDec.m_cContactsList.GetCount());

	if (dwCount == 0)
	{
		AddLogEntry(L"No valid signatures found, you may selected invalid file", 0, 0, true, SECONDLEVEL);
		UpdateStatus(L"No valid signatures found, you may selected invalid file");
		return;
	}

	DWORD dwBufSize = dwCount * 0x10;
	//Allocate here memory
	m_lpszBuffer = new BYTE[dwBufSize];

	if (m_lpszBuffer == NULL)
	{
		AddLogEntry(L"memory allocation failed in CWardWizDBUpgradeDlg::OnBnClickedButtonStart", szDecryptedFilePath, 0, true, SECONDLEVEL);
		return;
	}

	CString csContactList;
	csContactList.Format(L">>> MD5: The count of Contactlist : %lu", dwCount);
	OutputDebugString(csContactList);

	DWORD dwBufOffset = 0x00;
	CString csFirstEntry = NULL, csSecondEntry = NULL;
	const ContactList& contacts = objWrdwizEncDecManager.m_objEncDec.GetContacts();

	bool bISFailed = false;

	DWORD dwIdxForMD5Coll = 0;
	POSITION pos = contacts.GetHeadPosition();
	while (pos != NULL)
	{
		const CIspyList contact = contacts.GetNext(pos);

		csFirstEntry = contact.GetFirstEntry();

		size_t   tSize;
		char szSigOnly[1024] = { 0 };
		wcstombs_s(&tSize, szSigOnly, sizeof(szSigOnly), csFirstEntry.GetBuffer(), sizeof(szSigOnly) + 1);

		if (strlen(szSigOnly) > 0)
		{
			BYTE szMD5Data[16] = { 0 };
			char *szDummy = NULL;
			int iMD5Index = 0;
			for (int iIndex = 0; iIndex < 32;)
			{
				char szHex[3] = { 0 };
				szHex[0] = szSigOnly[iIndex];
				szHex[1] = szSigOnly[iIndex + 1];
				szHex[2] = '\0';	

				szMD5Data[iMD5Index] = static_cast<BYTE>(strtol(szHex, &szDummy, 0x10));

				iMD5Index++;
				iIndex += 2;
			}
			
			memcpy(&m_lpszBuffer[dwBufOffset], szMD5Data, 0x10);
			dwBufOffset += 0x10;
		}
		else
		{
			bISFailed = true;
			break;
		}
	}

	//Check buffer CRC here
	ULONG64	ulBufCRC = 0x00;
	CWWizCRC64	objWWizCRC64;
	objWWizCRC64.CalcCRC64(ulBufCRC, m_lpszBuffer, dwBufOffset);

	if (bISFailed)
	{
		UpdateStatus(L"Blank line, Process it again, file: " + csFilePath);
		AfxMessageBox(L"Blank line, Process it again");
		goto CLEANUP;
	}

	csFilePath += L".NEW";

	size_t   tSize;
	char szDBSig[0x0B] = { 0 };
	wcstombs_s(&tSize, szDBSig, sizeof(szDBSig), csDBSignature.GetBuffer(), sizeof(szDBSig) + 1);

	size_t   tVersionSize;
	dwVersionLength = _tcslen(csVersionNo);
	char * lpVersion = (char*)malloc(dwVersionLength + 1);
	if (!lpVersion)
	{
		goto CLEANUP;
	}
	
	memset(lpVersion, 0, dwVersionLength + 1);
	wcstombs_s(&tVersionSize, lpVersion, dwVersionLength + 1, csVersionNo.GetBuffer(), dwVersionLength + 1);

	if (!DumpBufferInFile(csFilePath.GetBuffer(), m_lpszBuffer, dwBufSize, szDBSig, lpVersion, ulBufCRC, dwVersionLength))
	{
		AddLogEntry(L"### Function DumpBufferInFile failed, %s", csFilePath, 0, true, SECONDLEVEL); 
		UpdateStatus(L"Oops.. Failed to updgrade");
		goto CLEANUP;
	}

	csSigCount.Format(L"Total converted Signatures: %d", dwCount);
	m_stTotalSigCount.SetWindowText(csSigCount);

	if (lpVersion != NULL)
	{
		delete[]lpVersion;
		lpVersion = NULL;
	}

	UpdateStatus(L"Successfully Process completed..., New FilePath: " + csFilePath);

CLEANUP:
	//release memory  here
	if (m_lpszBuffer != NULL)
	{
		delete[]m_lpszBuffer;
		m_lpszBuffer = NULL;
	}
}

void CWardWizDBUpgradeDlg::OnBnClickedButtonCancel()
{
	OnCancel();
}

void CWardWizDBUpgradeDlg::UpdateStatus(CString csStatus)
{
	m_editStatus.SetWindowText(csStatus);
}


/***************************************************************************************************
*  Function Name  : DumpBufferInFile
*  Description    : Function to write buffer in DB file.
*  Author Name    : Ram Shelke
*  Date			  :	10 May 2016
****************************************************************************************************/
bool CWardWizDBUpgradeDlg::DumpBufferInFile(LPTSTR szFilePath, LPBYTE bFileBuffer, DWORD dwBufSize, LPSTR szSignature, LPSTR szVersion, ULONG64 ulFileCRC, DWORD dwVersionLength)
{
	bool bReturn = false;

	HANDLE	hOutputFileHandle = NULL;
	try
	{
		hOutputFileHandle = CreateFile(szFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutputFileHandle == INVALID_HANDLE_VALUE)
		{
			return bReturn;
		}

		DWORD dwSigOffset = 0x00;
		
		DWORD dwBufferOffset = dwSigOffset;
		SetFilePointer(hOutputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);
		
		DWORD dwSigLength = 0x0A + 2 + dwVersionLength + 2;
		LPBYTE bySigBuffer = new BYTE[dwSigLength];
		if (!bySigBuffer)
		{
			bReturn = false;
			goto CLEANUP;
		}

		//Initialize memory
		memset(bySigBuffer, 0, dwSigLength);

		memcpy(&bySigBuffer[dwSigOffset], szSignature, 0x0A);
		dwSigOffset += 0x0A;

		memcpy(&bySigBuffer[dwSigOffset], "|", 0x01);
		dwSigOffset += 0x01;

		memcpy(&bySigBuffer[dwSigOffset], szVersion, dwVersionLength);
		dwSigOffset += dwVersionLength;

		memcpy(&bySigBuffer[dwSigOffset], "|", 0x01);
		dwSigOffset += 0x01;



		//Write here wardwizSignature
		DWORD dwBytesWritten = 0;
		if (!WriteFile(hOutputFileHandle, bySigBuffer, dwSigOffset, &dwBytesWritten, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesWritten != dwSigOffset)
		{
			bReturn = false;
			goto CLEANUP;
		}


		dwBufferOffset = dwSigOffset;
		SetFilePointer(hOutputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		dwBytesWritten = 0;
		if (!WriteFile(hOutputFileHandle, &ulFileCRC, sizeof(ulFileCRC), &dwBytesWritten, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesWritten != sizeof(ulFileCRC))
		{
			bReturn = false;
			goto CLEANUP;
		}

		//Read here remaining file
		dwBufferOffset += dwBytesWritten;
		SetFilePointer(hOutputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		dwBytesWritten = 0;
		if (!WriteFile(hOutputFileHandle, bFileBuffer, dwBufSize, &dwBytesWritten, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesWritten != dwBufSize)
		{
			bReturn = false;
			goto CLEANUP;
		}

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::LoadContaintFromFile, %s", szFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
CLEANUP:

	//Need to close file handle after collecting buffer
	if (hOutputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hOutputFileHandle);
		hOutputFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}