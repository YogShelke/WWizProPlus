// WrdwizDecDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WardWizCryptUtility.h"
#include "WrdwizDecDlg.h"
#include "afxdialogex.h"


// CWrdwizDecDlg dialog
TCHAR	m_szProgramData[256] = { 0 };
DWORD WINAPI DecryptFileThread(LPVOID lpParam);


IMPLEMENT_DYNAMIC(CWrdwizDecDlg, CDialogEx)

CWrdwizDecDlg::CWrdwizDecDlg(CWnd* pParent /*=NULL*/)
: CJpegDialog(CWrdwizDecDlg::IDD, pParent)
{
	m_pbyEncDecKey = (unsigned char *)calloc(WRDWIZ_KEY_SIZE, sizeof(unsigned char));
}

CWrdwizDecDlg::~CWrdwizDecDlg()
{
}

void CWrdwizDecDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SELECT_FILE, m_edtSelectFile);
	DDX_Control(pDX, IDC_BUTTON_BROWSE, m_btnBrowse);
	DDX_Control(pDX, IDC_BUTTON_START, m_btnStart);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EDIT_FILE_STATUS, m_edtFileStatus);
	DDX_Control(pDX, IDC_STATIC_DECRYPT_TOOL_DESC, m_stDecryptToolDesc);
	DDX_Control(pDX, IDC_STATIC_TOOL_HEADER, m_stDecryptToolHeader);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_STATIC_ALL_RIGHTS_RESERVED, m_stAllRightsReserved);
}


BEGIN_MESSAGE_MAP(CWrdwizDecDlg, CJpegDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CWrdwizDecDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_START, &CWrdwizDecDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CWrdwizDecDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CWrdwizDecDlg::OnBnClickedButtonClose)
END_MESSAGE_MAP()


// CWrdwizDecDlg message handlers


BOOL CWrdwizDecDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	if (!Load(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_JPG_DECRYPT_TOOL_BG), _T("JPG")))
	{
		MessageBox(L"Failded to load JPG file. Please reinstall setup.", L"Vibranium", MB_ICONERROR);
	}
	Draw();
	this->GetClientRect(m_rect);
	CRgn		 rgn;
	rgn.CreateRectRgn(m_rect.left, m_rect.top, m_rect.right - 3, m_rect.bottom - 3/*, 41, 41*/);
	this->SetWindowRgn(rgn, TRUE);

	m_stDecryptToolHeader.SetWindowPos(&wndTop, m_rect.left + 30, 60, 550, 20, SWP_NOREDRAW);
	m_stDecryptToolHeader.SetWindowTextW(L"Decrypt file(s) which are encrypted by Vibranium version lower than 1.11.0.0");

	m_stDecryptToolDesc.SetWindowPos(&wndTop, m_rect.left + 45, 110, 400, 30, SWP_NOREDRAW);
	m_stDecryptToolDesc.SetWindowTextW(L"This utility is to decrypt file(s) which are encrypted by Vibranium version lower than 1.11.0.0");
	m_stDecryptToolDesc.SetBkColor(RGB(255, 255, 255));

	m_btnClose.SetWindowPos(&wndTop, m_rect.left + 483, 0, 26, 17, SWP_NOREDRAW);
	m_btnClose.SetSkin(AfxGetResourceHandle(), IDB_BITMAP_CLOSE_BUTTON, IDB_BITMAP_CLOSE_BUTTON, IDB_BITMAP_CLOSE_BUTTON_HOVER, IDB_BITMAP_CLOSE_BUTTON, IDB_BITMAP_CLOSE_BUTTON_HOVER, 0, 0, 0, 0);

	m_edtSelectFile.SetWindowPos(&wndTop, m_rect.left + 48, 155, 350, 20, SWP_NOREDRAW);
	GetDlgItem(IDC_EDIT_SELECT_FILE)->ModifyStyle(0, WS_DISABLED);

	m_btnBrowse.SetWindowPos(&wndTop, m_rect.left + 410, 153, 60, 25, SWP_NOREDRAW);
	
	m_btnStart.SetWindowPos(&wndTop, m_rect.left + 340, 190, 60, 25, SWP_NOREDRAW);

	m_btnCancel.SetWindowPos(&wndTop, m_rect.left + 410, 190, 60, 25, SWP_NOREDRAW);

	m_edtFileStatus.SetWindowPos(&wndTop, m_rect.left + 48, 230, 420, 20, SWP_NOREDRAW);
	m_edtFileStatus.SetBkColor(RGB(255,255,255));
	m_edtFileStatus.SetWindowTextW(L"Please select a file for Decryption.");
	GetDlgItem(IDC_EDIT_FILE_STATUS)->ModifyStyle(0, WS_DISABLED);

	CString csFooter;
	csFooter = L"All rights reserved";
	csFooter += _T(" \u00AE ");
	csFooter += L"WardWiz 2015";
	m_stAllRightsReserved.SetWindowTextW(csFooter);
	m_stAllRightsReserved.SetWindowPos(&wndTop, m_rect.left + 20, 315, 320, 24, SWP_NOREDRAW);
	//m_stCopyRightText.SetFont(&theApp.m_fontWWTextNormal);
	m_stAllRightsReserved.SetTextColor(RGB(255, 255, 255));
	m_stAllRightsReserved.SetBkColor(RGB(88, 88, 90));
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CWrdwizDecDlg::OnBnClickedButtonBrowse()
{
	static TCHAR szEncFilter[] = L"Vibranium Files (*.AK)|*.AK|";
	CFileDialog dlgBrowse(TRUE, L"AK Files(*.AK)", NULL, OFN_ALLOWMULTISELECT, (LPCTSTR)szEncFilter);
	dlgBrowse.DoModal();
	m_csFilePath = L"";
	m_csFilePath = dlgBrowse.GetPathName();
	m_edtSelectFile.SetWindowTextW(m_csFilePath);
	m_edtFileStatus.SetWindowTextW(L"Click on start button to decrypt file.");
}


void CWrdwizDecDlg::OnBnClickedButtonStart()
{
	if (m_csFilePath.Compare(L"") == 0)
	{
		MessageBox(L"Please select a file for decryption.",L"Vibranium", MB_ICONEXCLAMATION | MB_OK );
		return;
	}
	m_btnStart.EnableWindow(false);
	m_bIsEnDepInProgress = true;
	m_btnBrowse.EnableWindow(false);
	m_hEncDecThread = ::CreateThread(NULL, 0, DecryptFileThread, (LPVOID)this, 0, 0);
}

DWORD WINAPI DecryptFileThread(LPVOID lpParam)
{
	CWrdwizDecDlg *pDlg = (CWrdwizDecDlg*)lpParam;

	if (!pDlg)
		return 0;

	DWORD	dwStatus = 0x00;
	DWORD dwRet = pDlg->Decrypt_File(pDlg->m_csFilePath.GetBuffer(), dwStatus);

	if (dwRet == 3)
	{
		pDlg->m_bIsEnDepInProgress = false;
		pDlg->m_edtFileStatus.SetWindowTextW(L"Failed to decrypt a file."); 
		pDlg->MessageBox(L"No data for decryption(file size : 0)", L"Vibranium", MB_ICONEXCLAMATION);
		pDlg->m_edtSelectFile.SetWindowTextW(L"");
		

	}
	else if (dwRet == 8)
	{
		pDlg->m_bIsEnDepInProgress = false;
		pDlg->m_edtFileStatus.SetWindowTextW(L"Failed to decrypt a file."); 
		pDlg->MessageBox(L"Selected file is not encrypted using Vibranium \n\nPlease select valid file", L"Vibranium", MB_ICONEXCLAMATION);
		pDlg->m_edtSelectFile.SetWindowTextW(L"");
		
	}
	else if (dwRet == 9)
	{
		pDlg->m_bIsEnDepInProgress = false;
		pDlg->m_edtFileStatus.SetWindowTextW(L"Failed to decrypt a File.");
		pDlg->MessageBox(L"Please use Vibranium 1.11.0.0 and above  to decrypt selected file", L"Vibranium", MB_ICONEXCLAMATION);
		pDlg->m_edtSelectFile.SetWindowTextW(L"");

	}
	else if (dwRet == 0 )
	{
		pDlg->m_edtFileStatus.SetWindowTextW(L"File decrypted successFully.");
		pDlg->MessageBox(TEXT("File decrypted successfully."), L"Vibranium", MB_ICONINFORMATION);
		
	}
	else
	{
		pDlg->m_edtFileStatus.SetWindowTextW(L"Failed to decrypt a File.");
		pDlg->MessageBox(TEXT("File decryption failed. Please try again."), L"Vibranium", MB_ICONEXCLAMATION);
	}
	pDlg->m_bIsEnDepInProgress = false;
	pDlg->m_btnStart.EnableWindow(true);
	pDlg->m_btnBrowse.EnableWindow(true);
	pDlg->m_csFilePath = L"";
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	Decrypt_File
*  Description    :	Decrypts selected file with extension *.AK
*  SR.N0		  :
*  Author Name    : Vilas & Prajakta
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD CWrdwizDecDlg::Decrypt_File(TCHAR *m_szFilePath, DWORD &dwStatus)
{

	DWORD	dwRet = 0x00;
	DWORD	dwFileSize = 0x00, dwBytesRead = 0x00;
	TCHAR	szTemp[1024] = { 0 }, szFileName[1024] = { 0 };
	TCHAR	szExt[16] = { 0 };
	DWORD	dwLen = 0x00;
	LPBYTE	lpFileData = NULL;
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE;

	try
	{

		if (!PathFileExists(m_szFilePath))
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		dwLen = wcslen(m_szFilePath);
		if ((dwLen < 0x08) || (dwLen > 0x400))
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		memcpy(szExt, &m_szFilePath[dwLen - 0x05], 0x0A);
		if (_wcsicmp(szExt, TEXT(".AK")) != 0x00)
		{
			dwRet = 0x03;
			goto Cleanup;
		}

		hFile = CreateFile(m_szFilePath, GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### DataEncryption : Error in opening existing file %s", m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x02;
			goto Cleanup;
		}

		dwFileSize = GetFileSize(hFile, NULL);
		if (!dwFileSize)
		{
			AddLogEntry(L"### DataEncryption : Error in GetFileSize of %s", m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x03;
			goto Cleanup;
		}

		lpFileData = (LPBYTE)malloc(dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE));
		if (!lpFileData)
		{
			AddLogEntry(L"### DataEncryption : Error in allocation of memory", 0, 0, true, SECONDLEVEL);
			dwRet = 0x04;
			goto Cleanup;
		}

		memset(lpFileData, 0x00, (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)));
		SetFilePointer(hFile, (0x00 + (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)), NULL, FILE_BEGIN);
		ReadFile(hFile, lpFileData, (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)), &dwBytesRead, NULL);
		if ((dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)) != dwBytesRead)
		{
			AddLogEntry(L"### DataEncryption : Error in ReadFile %s", m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x04;
			goto Cleanup;
		}

		//read key from file
		DWORD dwRetValue = 0x00;
		dwRetValue = ReadKeyFromEncryptedFile(hFile);
		if (dwRetValue == 0x00)
		{
			AddLogEntry(L"### DataEncryption : Error in ReadKeyFromEncryptedFile :: Invalid File", 0, 0, true, SECONDLEVEL);
			dwRet = 0x08;
			goto Cleanup;
		}
		if (dwRetValue == 0x01)
		{
			AddLogEntry(L"### DataEncryption : Error in ReadKeyFromEncryptedFile:: New version File", 0, 0, true, SECONDLEVEL);
			dwRet = 0x09;
			goto Cleanup;
		}

		if (DecryptData((LPBYTE)lpFileData, dwBytesRead))
		{
			AddLogEntry(L"### DataEncryption : Error in DecryptData", 0, 0, true, SECONDLEVEL);
			dwRet = 0x05;
			goto Cleanup;
		}

		/*CString csSrcFilePath(m_szFilePath);
		int iPos = csSrcFilePath.ReverseFind(L'\\');
		CString csSrcFileName = csSrcFilePath.Right(csSrcFilePath.GetLength() - (iPos + 1));
		swprintf(szTemp, 1024, TEXT("%s\\%s"), m_szProgramData, csSrcFileName);
		dwLen = 0x0;
		dwLen = wcslen(szTemp);
		szTemp[dwLen - 0x05] = '\0';*/

		CString csSrcFilePath(m_szFilePath);
		int iPos = csSrcFilePath.ReverseFind(L'\\');
		CString csSrcFileName = csSrcFilePath.Right(csSrcFilePath.GetLength() - (iPos + 1));
		CString csFolderPath = csSrcFilePath.Left(iPos);
		CString csOriginalFilePath = L"";
		int iLength = csSrcFilePath.ReverseFind('.');
		if (iLength != -1)
		{
			csSrcFilePath.Truncate(iLength);
			csOriginalFilePath = csSrcFilePath;
		}
		//TCHAR szDecryptedFilePath[1024], szTemp[1024];
		//swprintf(szTemp, 1024, TEXT("%s\\%s"), m_szProgramData, csSrcFileName);
		//DWORD dwLen = wcslen(szTemp);
		//szTemp[dwLen - 0x05] = '\0';
		//wcscpy_s(szDecryptedFilePath, szTemp);
		//if (!PathFileExists(szDecryptedFilePath))
		//{
		//	//pDlg->m_bIsEnDepInProgress = false;
		//	return false;
		//}
		swprintf(szTemp, 1024, TEXT("%s"), csOriginalFilePath);

		hFileEnc = CreateFile(szTemp, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### DataEncryption : Error in Opening file %s", szTemp, 0, true, SECONDLEVEL);
			dwRet = 0x06;
			goto Cleanup;
		}

		dwBytesRead = 0x00;
		WriteFile(hFileEnc, lpFileData, (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)), &dwBytesRead, NULL);
		if ((dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)) != dwBytesRead)
			dwRet = 0x07;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::Decrypt_File", 0, 0, true, SECONDLEVEL);
		return false;
	}
Cleanup:

	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;

	if (hFileEnc != INVALID_HANDLE_VALUE)
		CloseHandle(hFileEnc);
	hFile = INVALID_HANDLE_VALUE;

	if (lpFileData)
		free(lpFileData);
	lpFileData = NULL;

	if (m_pbyEncDecKey != NULL)
	{
		free(m_pbyEncDecKey);
		m_pbyEncDecKey = NULL;
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : ReadKeyFromEncryptedFile
*  Description    : Read key from any encrypted file
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CWrdwizDecDlg::ReadKeyFromEncryptedFile(HANDLE hFile)
{
	DWORD	dwRet = 0x00;
	try
	{
		//bool	bReturn = false;
		int		iReadPos = 0x0;
		DWORD   dwBytesRead = 0x0;
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return dwRet;
		}

		dwRet = IsFileAlreadyEncrypted(hFile);
		if (dwRet == 0x00 || dwRet == 0x01)
		{
			return dwRet;
		}

		//read encryption key
		if (m_pbyEncDecKey == NULL)
		{
			m_pbyEncDecKey = (unsigned char *)calloc(WRDWIZ_KEY_SIZE, sizeof(unsigned char));
		}
		memset(m_pbyEncDecKey, 0x00, WRDWIZ_KEY_SIZE * sizeof(unsigned char));
		SetFilePointer(hFile, (0x0 + WRDWIZ_SIG_SIZE), NULL, FILE_BEGIN);
		ReadFile(hFile, &m_pbyEncDecKey[0x0], WRDWIZ_KEY_SIZE * sizeof(unsigned char), &dwBytesRead, NULL);
		if (dwBytesRead != WRDWIZ_KEY_SIZE)
		{
			dwRet = 0x00;
			return dwRet;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::ReadKeyFromEncryptedFile", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : DecryptData
*  Description    : Decrypt data of buffer.
*  Author Name    : Prajakta Nanaware
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CWrdwizDecDlg::DecryptData(LPBYTE lpBuffer, DWORD dwSize)
{
	if (IsBadWritePtr(lpBuffer, dwSize))
		return 1;

	DWORD	iIndex = 0;
	DWORD jIndex = 0;

	if (lpBuffer == NULL || dwSize == 0x00)
	{
		return 1;
	}

	for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
	{
		lpBuffer[iIndex] ^= m_pbyEncDecKey[jIndex++];
		if (jIndex == WRDWIZ_KEY_SIZE)
		{
			jIndex = 0x00;
		}
		if (iIndex >= dwSize)
		{
			break;
		}
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : IsFileAlreadyEncrypted
*  Description    : Checks given file is already encrypted or not.
*  Author Name    : Prajkta Nanaware
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CWrdwizDecDlg::IsFileAlreadyEncrypted(HANDLE hFile)
{
	DWORD	dwReturn = 0x00;
	__try
	{
		int		iReadPos = 0x0;
		DWORD   dwBytesRead = 0x0;
		unsigned char	bySigBuff[WRDWIZ_SIG_SIZE] = { 0x00 };

		if (hFile == INVALID_HANDLE_VALUE)
		{
			return dwReturn;
		}

		//check if file is already encrypted by checking existence of sig "WARDWIZ"
		SetFilePointer(hFile, iReadPos, NULL, FILE_BEGIN);
		ReadFile(hFile, &bySigBuff[0x0], WRDWIZ_SIG_SIZE * sizeof(unsigned char), &dwBytesRead, NULL);
		if (dwBytesRead != WRDWIZ_SIG_SIZE)
		{
			return dwReturn;
		}
		if (memcmp(&bySigBuff, WRDWIZ_SIG_NEW, WRDWIZ_SIG_SIZE) == 0)
		{
			dwReturn = 0x01;
		}
		else if (memcmp(&bySigBuff, WRDWIZ_SIG, WRDWIZ_SIG_SIZE) == 0)
		{
			dwReturn = 0x02;
		}
		else
		{
			return dwReturn;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwReturn = 0x00;
	}
	return dwReturn;
}

/**********************************************************************************************************
*  Function Name  :	OnBnClickedButtonCancel
*  Description    :	To close Dialog.
*  SR.N0		  :
*  Author Name    : Nitin Kolapkar
*  Date           : 19th August 2015
**********************************************************************************************************/
void CWrdwizDecDlg::OnBnClickedButtonCancel()
{
	if (m_bIsEnDepInProgress == true)
	{
		MessageBox(L"Decryption Operation is in Progress Please wait.", L"Vibranium", MB_ICONINFORMATION | MB_OK);
		return;
	}
	OnCancel();
}

/**********************************************************************************************************
*  Function Name  :	OnBnClickedButtonClose
*  Description    :	To close Dialog.
*  SR.N0		  :
*  Author Name    : Nitin Kolapkar
*  Date           : 19th August 2015
**********************************************************************************************************/
void CWrdwizDecDlg::OnBnClickedButtonClose()
{
	OnBnClickedButtonCancel();
}
