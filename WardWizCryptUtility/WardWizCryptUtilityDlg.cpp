/**********************************************************************************************************
Program Name			: WardWizCryptUtilityDlg.cpp
Description				: This class is derived from CDialogEx, which is main Window for this project
Author Name				: Nitin Kolapkar
Date Of Creation		: 25th July 2015
Version No				: 1.12.0.0
Special Logic Used		:
Modification Log		:
***********************************************************************************************************/

#include "stdafx.h"
#include "WardWizCryptUtility.h"
#include "WardWizCryptUtilityDlg.h"
#include "afxdialogex.h"
#include "WrdwizDecDlg.h"
#include <Imagehlp.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CWardWizCryptUtilityDlg dialog



CWardWizCryptUtilityDlg::CWardWizCryptUtilityDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWardWizCryptUtilityDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_dwFileChecksum = 0;
}

void CWardWizCryptUtilityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FILE_PATH, m_edtFilePath);
	DDX_Control(pDX, IDC_BUTTON_BROWSE, m_btnBrowse);
	DDX_Control(pDX, IDC_BUTTON_START, m_btnStart);
	DDX_Control(pDX, IDC_LIST_FILE_DETAILS, m_lstFileDetails);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edtPassword);
}

BEGIN_MESSAGE_MAP(CWardWizCryptUtilityDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CWardWizCryptUtilityDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_START, &CWardWizCryptUtilityDlg::OnBnClickedButtonStart)
END_MESSAGE_MAP()


/***************************************************************************
Function Name  : OnInitDialog
Description    : For Initializing the dialog
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 25th July 2015
****************************************************************************/
BOOL CWardWizCryptUtilityDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CRect rect1;
	this->GetClientRect(rect1);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_lstFileDetails.InsertColumn(0, L"File Name", LVCFMT_LEFT, 200);
	m_lstFileDetails.InsertColumn(1, L"Password", LVCFMT_LEFT, 70);
	m_lstFileDetails.InsertColumn(2, L"File CRC", LVCFMT_LEFT, 150);
	m_lstFileDetails.InsertColumn(3, L"File Crypt Version", LVCFMT_LEFT, 100);
	m_lstFileDetails.SetTextColor(RGB(100, 100, 100));
	ListView_SetExtendedListViewStyle(m_lstFileDetails.m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWardWizCryptUtilityDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWardWizCryptUtilityDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWardWizCryptUtilityDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/***************************************************************************
Function Name  : OnBnClickedButtonBrowse
Description    : shows browse window for selecting .AK file for operation
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 25th July 2015
****************************************************************************/
void CWardWizCryptUtilityDlg::OnBnClickedButtonBrowse()
{
	try
	{
		static TCHAR szEncFilter[] = L"Vibranium Files (*.AK)|*.AK|";
		CFileDialog dlgBrowse(TRUE, L"AK Files(*.AK)", NULL, OFN_ALLOWMULTISELECT, (LPCTSTR)szEncFilter);
		dlgBrowse.DoModal();
		m_csFilePath = L"";
		m_csFilePath = dlgBrowse.GetPathName();
		m_edtFilePath.SetWindowTextW(m_csFilePath);
		m_edtPassword.SetWindowTextW(L"");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception CVibraniumCryptUtilityDlg::OnBnClickedButtonBrowse", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : OnBnClickedButtonStart
Description    : Starts the process for checking CRC and Password of selected file
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 25th July 2015
****************************************************************************/
void CWardWizCryptUtilityDlg::OnBnClickedButtonStart()
{
	try
	{
		//New Implementation: two seperate utiity for support and end user
		//implemented by Nitin K. Date: 21st Septmber 2015
		CString csPass = L"";
		m_edtPassword.GetWindowTextW(csPass);
		if (csPass == L"")
		{
			MessageBox(L"Please enter password", L"Vibranium", MB_ICONEXCLAMATION | MB_OK);
			return;
		}
		m_lstFileDetails.DeleteAllItems();
		m_lstFileDetails.InsertItem(0, m_csFilePath);

		DWORD dwIntegrtyRet = 0;
		//dwIntegrtyRet = CheckFileIntegrityBeforeDecryption(m_csFilePath);
		/*if (!dwIntegrtyRet)
		{
			DWORD dwRet = ValidateFile(m_csFilePath, csPass);
			switch (dwRet)
			{
			case 0x00:
				MessageBox(L"Operation completed. Please check in List for details", L"Vibranium", MB_OK);
				break;
			case 0x02:
				MessageBox(L"Unable to open file", L"Vibranium", MB_OK);
				break;
			case 0x08:
				MessageBox(L"Incorrect password \n Please provide correct password", L"Vibranium", MB_ICONEXCLAMATION | MB_OK);
				break;
			case 0x09:
				MessageBox(L"Version Mis-match", L"Vibranium", MB_ICONEXCLAMATION | MB_OK);
				break;
			default:
				MessageBox(L"Failed to access required information", L"Vibranium", MB_ICONEXCLAMATION | MB_OK);
				break;
			}
		}
		else
		{
			MessageBox(L"File modified by external resources, unable to process", L"Vibranium", MB_ICONEXCLAMATION | MB_OK);
			return;
		}*/
		dwIntegrtyRet = CheckFileIntegrityBeforeDecryption(m_csFilePath);
		if (dwIntegrtyRet)
		{
			MessageBox(L"File modified by external resources, unable to process", L"Vibranium", MB_ICONEXCLAMATION | MB_OK);
			return;
		}
		DWORD dwRet = ValidateFile(m_csFilePath, csPass);
		switch (dwRet)
		{
		case 0x00:
			dwIntegrtyRet = CheckFileIntegrityBeforeDecryption(m_csFilePath);
			if (!dwIntegrtyRet)
			{
				MessageBox(L"Operation completed. Please check in list for details", L"Vibranium", MB_OK);
			}
			else
			{
				MessageBox(L"File modified by external resources, unable to process", L"Vibranium", MB_ICONEXCLAMATION | MB_OK);
				return;
			}
			break;
		case 0x02:
			MessageBox(L"Unable to open file", L"Vibranium", MB_OK);
			break;
		case 0x06:
			m_lstFileDetails.SetItemText(0, 3, L"File encrypted using older version of Vibranium");
			MessageBox(L"File encrypted using older version of Vibranium", L"Vibranium", MB_OK);
			break;
		case 0x08:
			MessageBox(L"Incorrect password \n Please provide correct password", L"Vibranium", MB_ICONEXCLAMATION | MB_OK);
			break;
		case 0x09:
			MessageBox(L"Version mis-match", L"Vibranium", MB_ICONEXCLAMATION | MB_OK);
			break;
		default:
			MessageBox(L"Failed to access required information", L"Vibranium", MB_ICONEXCLAMATION | MB_OK);
			break;
		}
		m_edtPassword.SetWindowTextW(L"");
		m_edtFilePath.SetWindowTextW(L"");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception CVibraniumCryptUtilityDlg::OnBnClickedButtonStart", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : ValidateFile
Description    : function to check Password and Version No cted file
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 25th July 2015
****************************************************************************/
DWORD CWardWizCryptUtilityDlg::ValidateFile(LPCTSTR lpFilePath, LPCTSTR lpPassword)
{
	DWORD dwRet = 0x00;
	HANDLE hInputFileHandle = NULL;
	DWORD *pdwexpanded_key = NULL;
	QWORD dwFileSize = 0;

	
	
	//** Block for XOR operations in CBC Mode **
	BYTE _cbc_iv[16] = { 0 };				//IV
	BYTE _cbc_state_prev[16] = { 0 };		//Previous encrypted block

	try
	{
		hInputFileHandle = CreateFile(lpFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInputFileHandle == INVALID_HANDLE_VALUE)
		{
			dwRet = 0x02;
			goto CLEANUP;
		}

		//Function to handle file size more than 2 GB
		LARGE_INTEGER szLargeInt;
		if (GetFileSizeEx(hInputFileHandle, &szLargeInt) == FALSE)
		{
			dwRet = 0x02;
			goto CLEANUP;
		}

		dwFileSize = szLargeInt.QuadPart;

		if (dwFileSize == 0xFFFFFFFF)
		{
			dwRet = 0x02;
			goto CLEANUP;
		}
		//New Implementation: two seperate utiity for support and end user
		//implemented by Nitin K. Date: 21st Septmber 2015
		//Check here wheather the file is already encrypted by wardwiz
		/*if (!IsFileAlreadyEncrypted(hInputFileHandle))
		{*/
		DWORD dwRetValue = 0x00;
		dwRetValue = IsFileAlreadyEncrypted(hInputFileHandle);
		if (dwRetValue != 0x01 && dwRetValue != 0x02)
		{
			//Added By Nitin Kolapkar
			//Return type value changed
			dwRet = 0x07;
			goto CLEANUP;
		}
		if (dwRetValue == 0x02)
		{
			dwRet = 0x06;
			goto CLEANUP;
		}


		pdwexpanded_key = (DWORD *)malloc(240);

		unsigned char _CryptoCipherKey[32] = { 0 };

		TCHAR szKey[33] = { 0 };
		_tcscpy_s(szKey, lpPassword);

		//Import key
		for (UINT32 _i = 0; _i < _tcslen(szKey); _i++){
			_CryptoCipherKey[_i] = (szKey[_i] & 0x00ff);
		}

		//Fill with 0
		for (UINT32 _i = (UINT32)_tcslen(szKey); _i < 32; _i++){
			_CryptoCipherKey[_i] = 0x00;
		}

		DWORD dwBytesRead = 0;
		ReadFile(hInputFileHandle, &_cbc_iv, 16, &dwBytesRead, NULL);

		WRDWIZ_FILECRYPTOPARAM szCryptoParam = { 0 };
		memset(&szCryptoParam, 0, sizeof(WRDWIZ_FILECRYPTOPARAM));
		ReadFile(hInputFileHandle, &szCryptoParam, sizeof(WRDWIZ_FILECRYPTOPARAM), &dwBytesRead, NULL);

		dwFileSize -= WRDWIZ_SIG_SIZE;
		dwFileSize -= 16;
		dwFileSize -= sizeof(WRDWIZ_FILECRYPTOPARAM);

		//Key expansion
		Rijndael_set_key_decrypt(pdwexpanded_key, _CryptoCipherKey, 256);

		_AES_Decrypt_CBC((BYTE*)&szCryptoParam, (dwBytesRead / 16), pdwexpanded_key, _cbc_state_prev, _cbc_iv);

		if (!VerifyPassword(szCryptoParam.szPassword, lpPassword))
		{
			m_lstFileDetails.SetItemText(0, 1, L"Incorrect password");
			
			//Added By Nitin Kolapkar
			//Return type value changed
			dwRet = 0x08;
			goto CLEANUP;
		}
		m_lstFileDetails.SetItemText(0, 1, L"Correct password");
		CString csVersion = L"";
		GetDataEncVersion();
		m_dwDataEncVerfromFile = szCryptoParam.dwCryptVersion;
		CovertDataVersionNoDWORDToString();
		if (szCryptoParam.dwCryptVersion != m_dwDataEncVersionNo)
		{
			
			m_lstFileDetails.SetItemText(0, 3, m_szDataEncVerfromFile);
		
			
			dwRet = 0x09;
			goto CLEANUP;
		}
		m_lstFileDetails.SetItemText(0, 3, m_szDataEncVerfromFile);

		
		
	}
	catch (...)
	{
		AddLogEntry(L"### Exception CVibraniumCryptUtilityDlg::ValidateFile", 0, 0, true, SECONDLEVEL);
	}
CLEANUP:
	//Need to close file handle after collecting buffer
	if (hInputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hInputFileHandle);
		hInputFileHandle = INVALID_HANDLE_VALUE;
	}

	return dwRet;
}



/***************************************************************************************************
*  Function Name  : IsFileAlreadyEncrypted
*  Description    : Checks given file is already encrypted or not.
*  Author Name    : Prajkta Nanaware
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CWardWizCryptUtilityDlg::IsFileAlreadyEncrypted(HANDLE hFile)
{
	DWORD	dwReturn = 0x00;
	__try
	{
		int		iReadPos = 0x0;
		DWORD   dwBytesRead = 0x0;
		unsigned char	bySigBuff[WRDWIZ_SIG_SIZE] = { 0x00 };

		if (hFile == INVALID_HANDLE_VALUE)
		{
			dwReturn = 0x03;
			return dwReturn;
		}

		//check if file is already encrypted by checking existence of sig "WARDWIZ"
		SetFilePointer(hFile, iReadPos, NULL, FILE_BEGIN);
		ReadFile(hFile, &bySigBuff[0x0], WRDWIZ_SIG_SIZE * sizeof(unsigned char), &dwBytesRead, NULL);
		if (dwBytesRead != WRDWIZ_SIG_SIZE)
		{
			//New Implementation: two seperate utiity for support and end user
			//implemented by Nitin K. Date: 21st Septmber 2015
			dwReturn = 0x00;
			return dwReturn;
		}
		if (memcmp(&bySigBuff, WRDWIZ_SIG_NEW, WRDWIZ_SIG_SIZE) == 0)
		{
			dwReturn = 0x01;
			return dwReturn;
		}
		else if (memcmp(&bySigBuff, WRDWIZ_SIG, WRDWIZ_SIG_SIZE) == 0)
		{
			dwReturn = 0x02;
		}
		else
		{
			dwReturn = 0x00;
			return dwReturn;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwReturn = 0x01;
	}
	return dwReturn;
}


/***************************************************************************************************
*  Function Name  : CheckFileIntegrityBeforeDecryption()
*  Description    : Checks file for modified. If file is not modified, returns 0 else 1
*  Author Name    : Vilas
*  Date			  :	09-July-2015
*  SR No		  :
****************************************************************************************************/
DWORD CWardWizCryptUtilityDlg::CheckFileIntegrityBeforeDecryption(LPCTSTR lpFileName)
{
	DWORD	dwRet = 0x00;
	DWORD64 dwSize = 0x00;
	LARGE_INTEGER lpFileSize = { 0 };
	DWORD dwStoredChecksum = 0x00;
	DWORD dwReadBytes = 0x00;
	LARGE_INTEGER liDistToMove = { 0 };
	DWORD	dwFileOriginalChecksum = 0x00;
	DWORD	dwFileComputedChecksum = 0x00;

	try
	{
		HANDLE	hFile = CreateFile(lpFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //Open with write access

		if (hFile == INVALID_HANDLE_VALUE)
		{
			dwRet = 0x01;
			return dwRet;
		}

		GetFileSizeEx(hFile, &lpFileSize);
		dwSize = lpFileSize.QuadPart;

		if ((!dwSize) || (dwSize == 0xFFFFFFFF))
		{
			CloseHandle(hFile);
			dwRet = 0x02;

			return dwRet;
		}


		liDistToMove.QuadPart = lpFileSize.QuadPart - sizeof(DWORD);

		SetFilePointerEx(hFile, liDistToMove, NULL, FILE_BEGIN);
		ReadFile(hFile, &dwStoredChecksum, sizeof(DWORD), &dwReadBytes, NULL);

		SetFilePointerEx(hFile, liDistToMove, NULL, FILE_BEGIN);

		SetEndOfFile(hFile);
		CloseHandle(hFile);

		MapFileAndCheckSum(lpFileName, &dwFileOriginalChecksum, &dwFileComputedChecksum);

		DWORD dwOut = 0;
		if (dwFileComputedChecksum != dwStoredChecksum)
		{
			CString csCRC = L"";
			csCRC.Format(L"%d", dwFileComputedChecksum);
			m_lstFileDetails.SetItemText(0, 2, csCRC);
			dwRet = 0x03;
			dwOut = AddCheckSum(lpFileName, dwStoredChecksum);
			if (dwOut)
			{
				AddLogEntry(L">>> Error in CVibraniumCrypter::AddCheckSum to set removed checksum as file is modified by extenal resource", 0, 0, true, SECONDLEVEL);
				dwRet = 0x04;
			}

		}
		
		CString csCRC = L"";
		csCRC.Format(L"%d", dwFileComputedChecksum);
		m_lstFileDetails.SetItemText(0, 2, csCRC);
		dwOut = AddCheckSum(lpFileName, dwStoredChecksum);
		if (dwOut)
		{
			AddLogEntry(L">>> Error in CVibraniumCrypter::AddCheckSum to set removed checksum as file is modified by extenal resource", 0, 0, true, SECONDLEVEL);
			dwRet = 0x04;
		}
		m_dwFileChecksum = dwStoredChecksum;
		

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumCrypter::CheckFileIntegrityBeforeDecryption", 0, 0, true, SECONDLEVEL);
	}


	return dwRet;

}

/***************************************************************************************************
*  Function Name  : AddCheckSum()
*  Description    : it uses to add check sum byte to encrypted file at end of file
*  Author Name    : Vilas/Lalit
*  Date			  :	09-July-2015
*  SR No		  :
****************************************************************************************************/
DWORD CWardWizCryptUtilityDlg::AddCheckSum(LPCTSTR lpFileName, DWORD dwByteNeedtoAdd)
{
	DWORD	dwRet = 0x00;
	TCHAR szszStartTime[255] = { 0 };
	LARGE_INTEGER liDistToMove = { 0 };
	DWORD dwReadBytes = 0x00;

	try
	{

		HANDLE	hFile = CreateFile(lpFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //Open with write access

		if (hFile == INVALID_HANDLE_VALUE)
		{
			dwRet = 0x01;
			return dwRet;
		}

		SetFilePointerEx(hFile, liDistToMove, NULL, FILE_END);

		WriteFile(hFile, &dwByteNeedtoAdd, sizeof(DWORD), &dwReadBytes, NULL);

		if (dwReadBytes != sizeof(DWORD))
		{
			dwRet = 0x02;
		}

		CloseHandle(hFile);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumCrypter::AddCheckSum", 0, 0, true, SECONDLEVEL);
	}

	return dwRet;
}

/***********************************************************************************************
Function Name  : GetDataEncVersion
Description    : Get Data Encryption Version
SR.NO		   :
Author Name    : Neha Gharge
Date           : 10th July 2015
***********************************************************************************************/

DWORD CWardWizCryptUtilityDlg::GetDataEncVersion()
{
	__try
	{
		return GetDataEncVersionSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumCrypter::GetDataEncVersion", 0, 0, true, SECONDLEVEL);
	}
	return 0x00;
}

/***********************************************************************************************
Function Name  : GetDataEncVersionSEH
Description    : Get Data Encryption Version
SR.NO		   :
Author Name    : Neha Gharge
Date           : 10th July 2015
***********************************************************************************************/
DWORD CWardWizCryptUtilityDlg::GetDataEncVersionSEH()
{
	DWORD dwVersionNo = 0x00;
	try
	{
		HKEY hKey = NULL;

		TCHAR szDataEncVersion[1024];
		TCHAR szDataEncVer[1024] = { 0 };
		DWORD dwDataEncVersion = 1024;
		DWORD dwType = REG_SZ;
		DWORD dwVersionNo = 0x00;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed in CVibraniumCrypter::GetDataEncVersionSEH::RegOpenKeyEx", 0, 0, true, FIRSTLEVEL);
			return 0x00;
		}

		long ReadReg = RegQueryValueEx(hKey, L"DataEncVersion", NULL, &dwType, (LPBYTE)&szDataEncVersion, &dwDataEncVersion);
		if (ReadReg == ERROR_SUCCESS)
		{
			m_csDataEncVer = (LPCTSTR)szDataEncVersion;
			_tcscpy_s(szDataEncVer, _countof(szDataEncVer), m_csDataEncVer);
			dwVersionNo = ConvertStringTODWORD(szDataEncVer);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumCrypter::GetDataEncVersionSEH", 0, 0, true, SECONDLEVEL);
		return 0x00;
	}

	return dwVersionNo;
}

/***********************************************************************************************
Function Name  : ConvertStringTODWORD
Description    : Convert String to DWORD
SR.NO		   :
Author Name    : Neha Gharge
Date           : 10th July 2015
***********************************************************************************************/
DWORD CWardWizCryptUtilityDlg::ConvertStringTODWORD(TCHAR* szDataEncVersion)
{
	__try
	{
		TCHAR szDataEncVer[1024];
		_tcscpy_s(szDataEncVer, _countof(szDataEncVer), szDataEncVersion);
		return ConvertStringTODWORDSEH(szDataEncVer);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumCrypter::ConvertStringTODWORD", 0, 0, true, SECONDLEVEL);
	}
	return 0x00;
}

/***********************************************************************************************
Function Name  : ConvertStringTODWORDSEH
Description    : Convert String to DWORD
SR.NO		   :
Author Name    : Neha Gharge
Date           : 10th July 2015
***********************************************************************************************/
DWORD CWardWizCryptUtilityDlg::ConvertStringTODWORDSEH(TCHAR* szDataEncVersion)
{
	DWORD dwDataEncVersionNo = 0x00;
	try
	{
		if (!ParseVersionNo(szDataEncVersion))
		{
			AddLogEntry(L"### Failed to Parse data envryption version no in CVibraniumCrypter::ConvertStringTODWORDSEH", 0, 0, true, FIRSTLEVEL);
			return 0x00;
		}
		WORD wDataEncLogicNo = 0;
		WORD wDataEncPatchNo = 0;
		TCHAR szDataEncVer[256] = { 0 };

		_tcscpy_s(szDataEncVer, _countof(szDataEncVer), m_szDataEncLogicNo);
		swscanf_s(szDataEncVer, L"%hu", &wDataEncLogicNo);
		_tcscpy_s(szDataEncVer, _countof(szDataEncVer), m_szDataEncPatchNo);
		swscanf_s(szDataEncVer, L"%hu", &wDataEncPatchNo);

		dwDataEncVersionNo = (wDataEncLogicNo << 16) + (wDataEncPatchNo);
		m_dwDataEncVersionNo = dwDataEncVersionNo;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumCrypter::ConvertStringTODWORDSEH", 0, 0, true, SECONDLEVEL);
		return 0x00;
	}
	return dwDataEncVersionNo;
}

/***********************************************************************************************
Function Name  : ParseVersionNo
Description    : Parse String of version No.
SR.NO		   :
Author Name    : Neha Gharge
Date           : 10th July 2015
*******************************************************************************************/
bool CWardWizCryptUtilityDlg::ParseVersionNo(LPTSTR lpszVersionNo)
{
	__try
	{
		return ParseVersionNoSEH(lpszVersionNo);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumCrypter::ParseVersionNo", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
Function Name  : ParseVersionNoSEH
Description    : Parse String of version No.
SR.NO		   :
Author Name    : Neha Gharge
Date           : 10th July 2015
*******************************************************************************************/
bool CWardWizCryptUtilityDlg::ParseVersionNoSEH(LPTSTR lpszVersionNo)
{

	TCHAR	szToken[] = L".";
	TCHAR	*pToken = NULL;
	TCHAR	*pTokenNext = NULL;

	if (lpszVersionNo == NULL)
	{
		return false;
	}
	pToken = wcstok_s(lpszVersionNo, szToken, &pTokenNext);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from dataencrypt logic no", 0, 0, true, FIRSTLEVEL);
		return false;
	}

	TCHAR	szDataEncLogicNo[256] = { 0 };

	if (pToken)
		wcscpy_s(szDataEncLogicNo, _countof(szDataEncLogicNo), pToken);

	pToken = wcstok_s(NULL, szToken, &pTokenNext);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from dataencrypt version no", 0, 0, true, FIRSTLEVEL);
		return false;
	}

	TCHAR	szDataEncPatchNo[256] = { 0 };

	if (pToken)
		wcscpy_s(szDataEncPatchNo, _countof(szDataEncPatchNo), pToken);

	_tcscpy_s(m_szDataEncLogicNo, _countof(m_szDataEncLogicNo), szDataEncLogicNo);
	_tcscpy_s(m_szDataEncPatchNo, _countof(m_szDataEncPatchNo), szDataEncPatchNo);
	return true;
}

bool CWardWizCryptUtilityDlg::VerifyPassword(LPCTSTR szFilePassword, LPCTSTR szUserPassword)
{
	__try
	{
		if (!szFilePassword || !szUserPassword)
			return false;

		if (_tcscmp(szFilePassword, szUserPassword) != 0)
		{
			return false;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		return true;
	}
	return true;
}

/***********************************************************************************************
Function Name  : CovertDataVersionNoDWORDToString
Description    : Convert data version no dword to string
SR.NO		   :
Author Name    : Neha Gharge
Date           : 10th July 2015
*******************************************************************************************/
bool CWardWizCryptUtilityDlg::CovertDataVersionNoDWORDToString()
{
	try
	{
		WORD wDataEncrptPatchNo = LOWORD(m_dwDataEncVerfromFile);
		WORD wDataEncryptLogicNo = HIWORD(m_dwDataEncVerfromFile);

		CString csDataEnVersionNo(L"");
		csDataEnVersionNo.Format(L"%hu.%hu", wDataEncryptLogicNo, wDataEncrptPatchNo);
		_tcscpy_s(m_szDataEncVerfromFile, _countof(m_szDataEncVerfromFile), csDataEnVersionNo);

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumCrypter::CovertDataVersionNoDWORDToString()", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}