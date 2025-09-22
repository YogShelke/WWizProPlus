/*********************************************************************
*  Program Name: WardWizDataCrypt.cpp
*  Description: WardWizDataCrypt Implementation
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 28 March 2016
*  Version No: 2.0.0.1
**********************************************************************/

#include "stdafx.h"
#include "WardWizUI.h"
#include "WardWizDataCrypt.h"
#include "SelectDialog.h"

#define SETDATAENCSTATUS_EVENT_CODE		(FIRST_APPLICATION_EVENT_CODE + 23)
// CWardWizDataCrypt

bool ValidateGivenPassWord(CString csPassword);


CWardWizDataCrypt::CWardWizDataCrypt() : behavior_factory("WardWizDataEncDec")
{
	m_bManualStop = false;
	m_bEncSuccess = false;
	m_bAlreadyEncrypted = false;
	m_iDataOpr = 0;
	m_csPassword = L"";
}

CWardWizDataCrypt::~CWardWizDataCrypt()
{
}


/***************************************************************************************************
*  Function Name  : On_OnStartRegistryOpt
*  Description    : Accepts the request from UI and starts the Registry Optimizer
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
json::value CWardWizDataCrypt::On_OpenBrowseWindow(SCITER_VALUE svBoolSelection, SCITER_VALUE svFunSetFileNameCB)
{
	try
	{
		const SCITER_VALUE Value = svBoolSelection;
		bool bValue = Value.get(false);

		CString csSelectedFile = BrowseFolderForFileSelection(bValue, NULL, NULL);
		if (csSelectedFile != L"")
			svFunSetFileNameCB.call((SCITER_STRING)csSelectedFile);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::On_OpenBrowseWindow", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	On_StartDataEncryption
*  Description    :	Start the Crypt operations
*  Author Name    : Nitin Kolapkar
*  Date           : 19th April 2016
*  SR_NO		  :
**********************************************************************************************************/
json::value CWardWizDataCrypt::On_StartDataEncryption(SCITER_VALUE svStrFilePath, SCITER_VALUE svStrPassword, SCITER_VALUE svBoolKeepOriginal, SCITER_VALUE svFunSetDataCryptStatusCB)
{
	try
	{
		SCITER_STRING strFilePath = svStrFilePath.get(L"");
		TCHAR chFilePath[MAX_MUL_FILE_PATH] = { 0 };
		_tcscpy_s(chFilePath, strFilePath.c_str());

		SCITER_STRING strPassword = svStrPassword.get(L"");
		TCHAR chPassWord[MAX_PATH] = { 0 };
		_tcscpy_s(chPassWord, strPassword.c_str());

		bool bValKeepOriginal = svBoolKeepOriginal.get(false);

		m_svSetDataCryptStatusCB = svFunSetDataCryptStatusCB;
		theApp.m_bIsDataEncDecUIReceptive = true;
		m_bIsCryptFinish = false;
		StartCryptOperation(ENCRYPTION, chFilePath, chPassWord, bValKeepOriginal);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::On_StartDataEncryption", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	On_StartDataDecryption
*  Description    :	Start the Crypt operations
*  Author Name    : Nitin Kplapkar
*  Date           : 19th April 2016
*  SR_NO		  :
**********************************************************************************************************/
json::value CWardWizDataCrypt::On_StartDataDecryption(SCITER_VALUE svStrFilePath, SCITER_VALUE svStrPassword, SCITER_VALUE svBoolKeepOriginal, SCITER_VALUE svFunSetDataCryptStatusCB)
{
	try
	{
		SCITER_STRING strFilePath = svStrFilePath.get(L"");
		TCHAR chFilePath[MAX_MUL_FILE_PATH] = { 0 };
		_tcscpy_s(chFilePath, strFilePath.c_str());

		SCITER_STRING strPassword = svStrPassword.get(L"");
		TCHAR chPassWord[MAX_PATH] = { 0 };
		_tcscpy_s(chPassWord, strPassword.c_str());

		bool bValKeepOriginal = svBoolKeepOriginal.get(false);

		m_svSetDataCryptStatusCB = svFunSetDataCryptStatusCB;
		m_bIsCryptFinish = false;
		StartCryptOperation(DECRYPTION, chFilePath, chPassWord, bValKeepOriginal);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::On_StartDataDecryption", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : On_OnCloseCryptPassDlg
Description    : to close Password dlg of Crypt operation
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 19th April 2016
***********************************************************************************************/
json::value CWardWizDataCrypt::On_CloseCryptPassDlg()
{
	try
	{
		if (theApp.m_bDataCryptOpr)
		{
			exit(0);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::On_OnCloseCryptPassDlg", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : On_StopDataCryptOperation
Description    : used to stop Data cryptOperation
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 19th April 2016
***********************************************************************************************/
json::value CWardWizDataCrypt::On_StopDataCryptOperation()
{
	try
	{
		ShutDownCryptOperations(true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::On_StopDataCryptOperation", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	BrowseFolderForFileSelection
*  Description    :	Browse folder for file selection to encrypt or decrypt(*.AK)
*  Modified By	  : Nitin K
*  Modification	  : Updated Browse Window for new Implementation of Enc/Dec
*  SR.N0		  :
*  Author Name    : Vilas & Prajakta
*  Date           : 16 Nov 2013
**********************************************************************************************************/
CString CWardWizDataCrypt::BrowseFolderForFileSelection(bool bEncryption,TCHAR *pName, TCHAR *pExt)
{
	try
	{
		bool bNetworkPath = false;
		bool bAlreadyAdded = false;
		CString csSelectionText = NULL;
		CString csSelection = NULL;
		if (bEncryption)
		{
			csSelectionText = _T("*.*");
			csSelection = _T("All files and folders(*.*)|*.*||");
		}
		else
		{
			csSelectionText = _T("*.AK||");
			csSelection = _T("All files and folders(*.*)|*.AK*||");
		}
		
		CSelectDialog ofd(TRUE, csSelectionText, NULL,
			OFN_HIDEREADONLY | OFN_NODEREFERENCELINKS | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON,
			csSelection);

		if (ofd.DoModal() != IDOK)
		{
			return L"";
		}

		for (int iPos = 0; iPos < ofd.m_SelectedItemList.GetCount(); iPos++)
		{
			LVFINDINFO lvInfo;
			lvInfo.flags = LVFI_STRING;
			CString csFileName = NULL;
			CString csScanPath = NULL;
			int iCListEntriesLength = 0;
			csFileName = ofd.m_SelectedItemList[iPos];
			lvInfo.psz = csFileName;
			if (PathIsNetworkPath(csFileName))
			{
				bNetworkPath = true;
				continue;
			}
			return ofd.m_SelectedItemList[iPos];
		}

		if (bNetworkPath)
		{
			theApp.MessageBoxUI(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_NETWORK_PATH_NOT_ALLOWED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"));
		}
	}
	catch (...)
	{
		AddLogEntry(L"### error in CDataEncryptionDlg::BrowseFolderForFileSelection", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/**********************************************************************************************************
*  Function Name  :	StartCryptOperation
*  Description    :	Start the Crypt operations
*  Author Name    : Nitin Kplapkar
*  Date           : 15th June 2015
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizDataCrypt::StartCryptOperation(DATACRYPTOPR CryptOperation,TCHAR *chFilePath, TCHAR *chPassword, bool bKeepOriginal)
{
	if (chFilePath == NULL || chPassword == NULL)
		return;

	TCHAR			szTemp[512] = { 0 };
	TCHAR			*pName = NULL;
	int				iKeepOriginal = 0;
	int				iDataOpr = 0;
	m_csPassword	= chPassword;
	m_bManualStop	= false;
	iDataOpr		= CryptOperation;
	m_iDataOpr		= CryptOperation;
	m_dwDecEncStatus = 0;
	m_bEncSuccess	= false;

	AddLogEntry(L">>> DataEnryption : Encryption/Decryption started", 0, 0, true, FIRSTLEVEL);

	_tcscpy_s(m_szFilePath, chFilePath);
	if (!m_szFilePath[0])
	{
		CString csNotificationCB;
		csNotificationCB = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_VALID_SELECTION");
		SetDataCryptOprStatusOnUI((SCITER_VALUE)SYSTEM_FILE_FOLDER_PATH, L"",(SCITER_STRING)csNotificationCB);
		return;
	}

	if (IsPathBelongsToWardWizDir(m_szFilePath))
	{
		CString csNotificationCB;
		csNotificationCB = theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_OWN_FILEPATH");
		SetDataCryptOprStatusOnUI((SCITER_VALUE)SYSTEM_FILE_FOLDER_PATH, L"",(SCITER_STRING)csNotificationCB);
		return;
	}

	if (IsPathBelongsFromRemovableDrive(m_szFilePath) == REMOVABLE_DRIVE)
	{
		CString csNotificationCB;
		csNotificationCB = theApp.m_objwardwizLangManager.GetString(L"IDS_ENCDEC_REM_DEVPATH");
		SetDataCryptOprStatusOnUI((SCITER_VALUE)FILE_REMOVBL_DEVICE, L"",(SCITER_STRING)csNotificationCB);
		//return;
	}
	
	if (IsPathBelongsFromRemovableDrive(m_szFilePath) == CD_DRIVE)
	{
		CString csNotificationCB;
		csNotificationCB = theApp.m_objwardwizLangManager.GetString(L"IDS_ENCR_WARDWIZ_DB_FILE");
		SetDataCryptOprStatusOnUI((SCITER_VALUE)SYSTEM_FILE_FOLDER_PATH, L"", (SCITER_STRING)csNotificationCB);
		return;
	}
	
	GetModuleFileName(NULL, szTemp, 511);
	pName = wcsrchr(szTemp, '\\');
	if (!pName)
	{
		CString csNotificationCB;
		csNotificationCB = theApp.m_objwardwizLangManager.GetString(L"IDS_WRDWIZAV_PATH_NOT_FOUNT");
		SetDataCryptOprStatusOnUI((SCITER_VALUE)SYSTEM_FILE_FOLDER_PATH, L"",(SCITER_STRING)csNotificationCB);
		return;
	}

	if (bKeepOriginal)
	{
		iKeepOriginal = KEEPORIGINAL;
	}
	else
	{
		iKeepOriginal = DELETEORIGINAL;
	}

	if (!SendDataEncryptionOperation2Service(SERVICE_SERVER, DATA_ENC_DEC_OPERATIONS, m_szFilePath, m_csPassword, iKeepOriginal, iDataOpr, true))
	{
		AddLogEntry(L"### Dataencryption : Error in DATA_ENC_DEC_OPERATIONS in CDataencryption:SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : IsPathBelongsToOSReservDirectory
Description    : Check if Path belongs to wardwiz directory
Author Name    : Ramkrushna Shelke
Date           : 9-11-2015
****************************************************************************/
bool CWardWizDataCrypt::IsPathBelongsToWardWizDir(CString csFilefolderPath)
{
	bool bReturn = false;

	try
	{
		//Check here if the path is lnk ( shortcut )
		if (csFilefolderPath.Right(4).CompareNoCase(L".lnk") == 0)
		{
			TCHAR szModulePath[MAX_PATH] = { 0 };
			if (GetModulePath(szModulePath, sizeof(szModulePath)))
			{
				CString csExpandedPath;
				csExpandedPath = ExpandShortcut(csFilefolderPath);
				if (csExpandedPath.Trim().GetLength() != 0)
				{
					int iPos = csExpandedPath.ReverseFind(L'\\');
					if (csExpandedPath.Left(iPos).CompareNoCase(szModulePath) == 0)
					{
						bReturn = true;
					}
				}
			}
		}
		else
		{
			TCHAR szModulePath[MAX_PATH] = { 0 }; 
			if (GetModulePath(szModulePath, sizeof(szModulePath)))
			{
				CString str = szModulePath;
				int ii  = str.GetLength();
				int iCount = csFilefolderPath.CompareNoCase(szModulePath);
				if (iCount >= ii || iCount== 0)
				{
					bReturn = true;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::IsPathBelongsToVibraniumDir", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : ExpandShortcut
*  Description    : Uses IShellLink to expand a shortcut.
*  Return value	  : the expanded filename, or "" on error or if filename
wasn't a shortcut
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
CString CWardWizDataCrypt::ExpandShortcut(CString& csFilename)
{
	CString csExpandedFile = NULL;

	try
	{
		USES_CONVERSION;		// For T2COLE() below

		// Make sure we have a path
		if (csFilename.IsEmpty())
		{
			ASSERT(FALSE);
			return csExpandedFile;
		}

		// Get a pointer to the IShellLink interface
		HRESULT hr;
		IShellLink* pIShellLink;

		hr = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
			IID_IShellLink, (LPVOID*)&pIShellLink);

		if (SUCCEEDED(hr))
		{
			// Get a pointer to the persist file interface
			IPersistFile* pIPersistFile;
			hr = pIShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pIPersistFile);

			if (SUCCEEDED(hr))
			{
				// Load the shortcut and resolve the path
				// IPersistFile::Load() expects a UNICODE string
				// so we're using the T2COLE macro for the conversion
				// For more info, check out MFC Technical note TN059
				// (these macros are also supported in ATL and are
				// so much better than the ::MultiByteToWideChar() family)
				hr = pIPersistFile->Load(T2COLE(csFilename), STGM_READ);

				if (SUCCEEDED(hr))
				{
					WIN32_FIND_DATA wfd;
					hr = pIShellLink->GetPath(csExpandedFile.GetBuffer(MAX_PATH),
						MAX_PATH,
						&wfd,
						SLGP_UNCPRIORITY);

					csExpandedFile.ReleaseBuffer(-1);
				}
				pIPersistFile->Release();
			}
			pIShellLink->Release();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::ExpandShortcut()", 0, 0, true, SECONDLEVEL);;
	}
	return csExpandedFile;
}

/***************************************************************************
Function Name  : IsPathBelongsFromRemovableDrive
Description    : Check if Path belongs to Removable devices.
Check with Tray using pipe communication.
Author Name    : Ramkrushna Shelke
Date           : 11-12-2015
Issue resolved : 0001165
****************************************************************************/
int CWardWizDataCrypt::IsPathBelongsFromRemovableDrive(CString csFilefolderPath)
{
	int iReturn = INVALID_DRIVE;
	try
	{

		if (csFilefolderPath.IsEmpty())
			return iReturn;

		UINT  uDriveType = 0;
		TCHAR szDriveRoot[] = _T("x:\\");
		_tcscpy_s(szDriveRoot, csFilefolderPath.Left(2));
		uDriveType = GetDriveType(szDriveRoot);

		if (uDriveType == DRIVE_REMOVABLE)
		{
			iReturn = REMOVABLE_DRIVE;
		}
		if (GetDriveType(szDriveRoot) == DRIVE_CDROM)
		{
			iReturn = CD_DRIVE;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::IsPathBelongsRemovableDrive, File :%s", csFilefolderPath, 0, true, SECONDLEVEL);
		iReturn = INVALID_DRIVE;
	}
	return iReturn;
}

/**********************************************************************************************************
*  Function Name  :	SendDataEncryptionOperation2Service
*  Description    :	Send Data encryption operation to comm service through named pipe.
*  SR.N0		  :
*  Author Name    :	Neha Gharge
*  Date           : 3 Dec 2013
**********************************************************************************************************/
bool CWardWizDataCrypt::SendDataEncryptionOperation2Service(const TCHAR * chPipeName, DWORD dwType, CString csSrcFilePath, CString csDestFilePath, DWORD dwValue, DWORD dwValueOperation, bool bDataEncryptionWait)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));

	szPipeData.iMessageInfo = dwType;
	szPipeData.dwValue = dwValue;
	szPipeData.dwSecondValue = dwValueOperation;
	wcscpy_s(szPipeData.szFirstParam, csSrcFilePath);
	wcscpy_s(szPipeData.szSecondParam, csDestFilePath);

	CISpyCommunicator objCom(chPipeName, true);
	if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CVibraniumDataCrypt : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bDataEncryptionWait)
	{
		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumDataCrypt : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}


/**********************************************************************************************************
*  Function Name  :	StopCryptOperations
*  Description    :	Stop the Crypt operations
*  Author Name    : Nitin Kplapkar
*  Date           : 15th June 2015
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizDataCrypt::StopCryptOperations(DWORD dwSuccessStatus)
{
	try
	{
		m_bIsCryptFinish = true;
		if (dwSuccessStatus == 1)
		{
			if (m_bEncSuccess == true)
			{
				//Operation completed successfully
				m_dwFinalStatus = CRYPT_FINISHED;
				if (theApp.m_bIsDataEncDecPageSwitched && theApp.m_bIsDataEncDecUIReceptive)
				{
					OnCallSetCryptStatus(0x00);
				}
				else
				{
					SetDataCryptOprStatusOnUI((SCITER_VALUE)CRYPT_FINISHED, (SCITER_STRING)theApp.m_csDataCryptFilePath, (SCITER_STRING)m_csFinalStatus);
				}
			}
			else
			{
				m_dwFinalStatus = m_dwDecEncStatus;
				if (m_dwDecEncStatus == OPR_CANCELED)
				{
					m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_CANCELED");
					if (theApp.m_bIsDataEncDecPageSwitched && theApp.m_bIsDataEncDecUIReceptive)
					{
						OnCallSetCryptStatus(0x00);
					}
					else
					{
						SetDataCryptOprStatusOnUI((SCITER_VALUE)OPR_CANCELED, (SCITER_STRING)theApp.m_csDataCryptFilePath, (SCITER_STRING)m_csFinalStatus);
					}
					m_csFinalStatus.Format(L"%s. %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_FAILED_MSG"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_FAILED_check_ACTION_MSG"));
					if (theApp.m_bIsDataEncDecPageSwitched)
					{
						OnCallSetCryptStatus(0x00);
					}
					else
					{
						SetDataCryptOprStatusOnUI((SCITER_VALUE)OPR_ABORTED, (SCITER_STRING)theApp.m_csDataCryptFilePath, (SCITER_STRING)m_csFinalStatus);
					}
				}
				else if (m_dwDecEncStatus == OPR_ABORTED)
				{
					m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_ABORTED_MSG");
					SetDataCryptOprStatusOnUI((SCITER_VALUE)OPR_CANCELED, (SCITER_STRING)theApp.m_csDataCryptFilePath, (SCITER_STRING)m_csFinalStatus);
					m_csFinalStatus.Format(L"%s. %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_FAILED_MSG"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_FAILED_check_ACTION_MSG"));
					if (theApp.m_bIsDataEncDecPageSwitched)
					{
						OnCallSetCryptStatus(0x00);
					}
					else
					{
						SetDataCryptOprStatusOnUI((SCITER_VALUE)OPR_ABORTED, (SCITER_STRING)theApp.m_csDataCryptFilePath, (SCITER_STRING)m_csFinalStatus);
					}
				}
				else if (m_dwDecEncStatus == ALREADY_ENCRYPTED || m_dwDecEncStatus == ZERO_KB_FILE || m_dwDecEncStatus == PASS_MISMATCH || m_dwDecEncStatus == OPR_FAILED || m_dwDecEncStatus == INVALID_FILE || m_dwDecEncStatus == NOT_ENCRYPTED || m_dwDecEncStatus == FILE_NOT_FOUND || m_dwDecEncStatus == FILE_SIZE_MORETHEN_3GB || m_dwDecEncStatus == OPR_VERSION_MISMATCH || m_dwDecEncStatus == INTIGRITY_FAILED || m_dwDecEncStatus == INTIGRITY_ROLLBACK_FAILED || m_dwDecEncStatus == DISK_SPACE_LOW || m_dwDecEncStatus == WARDWIZ_DB_FILE || m_dwDecEncStatus == FILE_ENC_USING_OLD_VERSION)
				{
					m_csFinalStatus.Format(L"%s. %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_FAILED_MSG"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_FAILED_check_ACTION_MSG"));
					if (theApp.m_bIsDataEncDecPageSwitched && theApp.m_bIsDataEncDecUIReceptive)
					{
						OnCallSetCryptStatus(0x00);
					}
					else
					{
						SetDataCryptOprStatusOnUI((SCITER_VALUE)ALREADY_ENCRYPTED, (SCITER_STRING)theApp.m_csDataCryptFilePath, (SCITER_STRING)m_csFinalStatus);
					}
					m_bAlreadyEncrypted = false;
				}
				else
				{
					m_csFinalStatus.Format(L"%s.", theApp.m_objwardwizLangManager.GetString(L"IDS_ENC_DEC_ZEROR_FILE_TO_PROCESS"));
					if (theApp.m_bIsDataEncDecPageSwitched && theApp.m_bIsDataEncDecUIReceptive)
					{
						OnCallSetCryptStatus(0x00);
					}
					else
					{
						SetDataCryptOprStatusOnUI((SCITER_VALUE)ALREADY_ENCRYPTED, (SCITER_STRING)theApp.m_csDataCryptFilePath, (SCITER_STRING)m_csFinalStatus);
					}
				}
			}
			::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
			Sleep(300);
			theApp.m_objCompleteEvent.ResetEvent();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::StopCryptOperations, ", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	StopCryptOperations
*  Description    :	Stop the Crypt operations
*  Author Name    : Nitin Kplapkar
*  Date           : 15th June 2015
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizDataCrypt::ShutDownCryptOperations(bool bManualStop)
{
	try
	{
		DWORD dwManualStop = 0;
		m_bManualStop = bManualStop;
		m_dwDecEncStatus = OPR_ABORTED;
		m_bEncSuccess = false;
		dwManualStop = (bManualStop == true) ? 1 : 0;
		if (!SendDataEncryptionOperation2Service(WWIZ_CRYPT_SERVER, STOP_CRYPT_OPR, m_szFilePath, m_csPassword, dwManualStop, 0, false))
		{
			AddLogEntry(L"### Dataencryption : Error in Vibranium_CRYPT_SERVER in CDataencryption:SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::ShutDownCryptOperations, ", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : UpdateDataCryptOpr
Description    : Displays the messages came from Crypt.exe to UI for Data Ecryption/Decryption
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 15th June 2015
***********************************************************************************************/
void CWardWizDataCrypt::UpdateDataCryptOpr(LPISPY_PIPE_DATA lpSpyData)
{
	try
	{
		DWORD dwFileStatus = lpSpyData->dwValue;
		DWORD dwInsertNewItem = lpSpyData->dwSecondValue;
		CString csFileSaveAsPath = L"";
		CString csTotalFileProcessed = L"";
		CString csMsgToLowSpace = L"";
		CString csProcessedFileCount;
		CString csIniFilePath = L"";
		switch (dwFileStatus)
		{
		case CRYPT_FINISHED:
			StopCryptOperations(dwInsertNewItem);
			return;
		case INSERT_NEW_ITEM:
			m_csEncryptnFilePath = theApp.m_csDataCryptFilePath;
			SetDataCryptOprStatusOnUI((SCITER_VALUE)INSERT_NEW_ITEM, (SCITER_STRING)theApp.m_csDataCryptFilePath, SCITER_STRING(L""));
			break;
		case OPR_FAILED:
			m_dwDecEncStatus = OPR_FAILED;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED");
			break;
		case ENCRYPT_SUCCESS:
			m_bEncSuccess = true;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_ENCRYPTED");
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"LastEncrypted", m_csEncryptnFilePath, csIniFilePath);
			if (theApp.m_bIsDataEncDecPageSwitched && theApp.m_bIsDataEncDecUIReceptive)
			{
				OnCallSetCryptStatus(ENCRYPT_SUCCESS);
			}
			else
			{
				SetDataCryptOprStatusOnUI((SCITER_VALUE)ENCRYPT_SUCCESS, (SCITER_STRING)theApp.m_csDataCryptFilePath, (SCITER_STRING)m_csFinalStatus);
			}
			break;
		case DECRYPT_SUCCESS:
			m_bEncSuccess = true;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_DECRYPTED");
			if (theApp.m_bIsDataEncDecPageSwitched && theApp.m_bIsDataEncDecUIReceptive)
			{
				OnCallSetCryptStatus(ENCRYPT_SUCCESS);
			}
			else
			{
				SetDataCryptOprStatusOnUI((SCITER_VALUE)ENCRYPT_SUCCESS, (SCITER_STRING)theApp.m_csDataCryptFilePath, (SCITER_STRING)m_csFinalStatus);
			}
			break;
		case ALREADY_ENCRYPTED:
			m_dwDecEncStatus = ALREADY_ENCRYPTED;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_ALREADY_ENCRYPTED");
			break;
		case INVALID_FILE:
			m_dwDecEncStatus = INVALID_FILE;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_INVALID_FILE");
			break;
		case PASS_MISMATCH:
			m_dwDecEncStatus = PASS_MISMATCH;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_PASSWORD_MISMATCH");
			break;
		case NOT_ENCRYPTED:
			m_dwDecEncStatus = NOT_ENCRYPTED;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_INVALID_FILE");
			break;
		case ENC_INPROGRESS:
			m_csFinalStatus = theApp.m_csDataCryptFilePath;
			break;
		case DEC_INPROGRESS:
			m_csFinalStatus = theApp.m_csDataCryptFilePath;
			break;
		case OPR_CANCELED:
			m_dwDecEncStatus = OPR_CANCELED;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_CANCELED");
			break;
		case OPR_FILE_COUNT:
			SetDataCryptOprStatusOnUI((SCITER_VALUE)OPR_FILE_COUNT, (SCITER_STRING)theApp.m_csDataCryptFilePath, (SCITER_STRING)theApp.m_csDataCryptFilePath);
			return;

		case SAVE_AS:
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"Waiting..");
			lpSpyData->iMessageInfo = SAVE_AS;
			csFileSaveAsPath = SaveAsDouplicateFile(theApp.m_csDataCryptFilePath);
			_tcscpy_s(lpSpyData->szFirstParam, csFileSaveAsPath);
			break;

		case FILE_NOT_FOUND:
			m_dwDecEncStatus = FILE_NOT_FOUND;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_FILE_NOT_FOUND");
			break;

		case FILE_SIZE_MORETHEN_3GB:
			m_dwDecEncStatus = FILE_SIZE_MORETHEN_3GB;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_FILE_SIZE_EXCEED");
			break;
		case OPR_VERSION_MISMATCH:
			m_csFinalStatus.Format(L"%s (%s)", theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_VERSION_MISMATCH"), lpSpyData->szFirstParam);
			break;

		case FILE_LOCKING:
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_ENCRYPTION_FINALIZING");
			break;
		case FILE_LOCKING_SUCCESS:
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_ENCRYPTED");
			break;
		case INTIGRITY_CHECKING:
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_DECR_INITIALIZING");
			break;
		case INTIGRITY_FAILED:
			m_dwDecEncStatus = INTIGRITY_FAILED;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_DECR_INTEGRITY_FAILED");
			break;
		case INTIGRITY_ROLLBACK_FAILED:
			m_dwDecEncStatus = INTIGRITY_ROLLBACK_FAILED;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_DECR_INTEGRITY_ROLLBACK_FAILED");
			break;

		case SYSTEM_FILE_FOLDER_PATH:
			csTotalFileProcessed.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_DECR_SYSTEM_FILE_FOLDER_PATH"));
			if (theApp.m_bIsDataEncDecPageSwitched && theApp.m_bIsDataEncDecUIReceptive)
			{
				OnCallSetCryptStatus(SYSTEM_FILE_FOLDER_PATH);
			}
			else
			{
				SetDataCryptOprStatusOnUI((SCITER_VALUE)SYSTEM_FILE_FOLDER_PATH, (SCITER_STRING)theApp.m_csDataCryptFilePath, (SCITER_STRING)csTotalFileProcessed);
			}
			return;

		case DISK_SPACE_LOW:
			m_dwDecEncStatus = DISK_SPACE_LOW;
			csMsgToLowSpace.Format(L"%s%s. %s", theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_LOW_DISK_SPACE1"), lpSpyData->szFirstParam, theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_LOW_DISK_SPACE2"));
			if (theApp.m_bIsDataEncDecPageSwitched && theApp.m_bIsDataEncDecUIReceptive)
			{
				OnCallSetCryptStatus(0x00);
			}
			else
			{
				SetDataCryptOprStatusOnUI((SCITER_VALUE)DISK_SPACE_LOW, (SCITER_STRING)theApp.m_csDataCryptFilePath, (SCITER_STRING)csMsgToLowSpace);
			}
			return;

		case WARDWIZ_DB_FILE:
			m_dwDecEncStatus = WARDWIZ_DB_FILE;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_ENCR_WARDWIZ_DB_FILE");
			break;

		case ZERO_KB_FILE:
			m_dwDecEncStatus = ZERO_KB_FILE;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_ZERO_KB_FILE");
			break;

		case FILE_ENC_USING_OLD_VERSION:
			m_dwDecEncStatus = FILE_ENC_USING_OLD_VERSION;
			m_csFinalStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_OLD_VERSION_REFER_CHM"); /*L"File Encrypted using older version, Please refer help file for further details";*/
			break;
		default:
			AddLogEntry(L"### Invalid entry CWardwizGUIDlg::UpdateDataCryptOpr", 0, 0, true, SECONDLEVEL);
			break;
		}
		SetDataCryptOprStatusOnUI((SCITER_VALUE)SHOW_STATUS_ON_UI, (SCITER_STRING)theApp.m_csDataCryptFilePath, (SCITER_STRING)m_csFinalStatus);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizGUIDlg::UpdateDataCryptOpr", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : SetDataCryptOprStatusOnUI
Description    : Set status on UI
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 19th April 2016
***********************************************************************************************/
void CWardWizDataCrypt::SetDataCryptOprStatusOnUI(SCITER_VALUE valAddUpdate, SCITER_STRING strFilePath, SCITER_STRING strFileStatus)
{
	try
	{
		m_svSetDataCryptStatusCB.call(valAddUpdate, strFilePath, strFileStatus);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::SetDataCryptOprStatusOnUI, ", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : SaveAsDoublicateFile
Description    : it provides functionality to save As option if same name file already exists.
SR.NO		   :
Author Name    : Lalit kumawat
Date           : 2-7- 2015
***********************************************************************************************/
CString  CWardWizDataCrypt::SaveAsDouplicateFile(CString csfilePath)
{
	DWORD dwFDialogOutput = 0;
	CString csNewFilePath = L"";
	CString csExtension = L"";
	CString csUserAddExtension = L"";
	CString csFileName = L"";
	try
	{
		csFileName = csfilePath.Mid(csfilePath.ReverseFind('\\') + 1);
		int iPos = csFileName.ReverseFind('.');

		if (iPos != -1)
		{
			CString csFileExt;
			csFileExt = csExtension = csfilePath.Mid(csfilePath.ReverseFind('.') + 1);
			csExtension = L"*." + csExtension;

			CFileDialog objFileDlg(FALSE, csFileExt, csExtension,
				OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, csExtension, theApp.m_pMainWnd);

			if (objFileDlg.DoModal() == IDOK)
			{
				csNewFilePath = objFileDlg.GetPathName();
				if (csExtension.CompareNoCase(L"*.AK") != -1)
				{
					csUserAddExtension = csNewFilePath.Mid(csNewFilePath.ReverseFind('.') + 1);
					if (csExtension.CompareNoCase(L"*.AK") == -1)
					{
						csNewFilePath = csNewFilePath + L".AK";
					}
				}
			}
			else
			{
				csNewFilePath = L"";
			}
		}
		else
		{
			static TCHAR szEncFilter[] = L"*.txt|";
			CFileDialog objFileDlg(FALSE, NULL, NULL,
				OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, (LPCTSTR)szEncFilter, theApp.m_pMainWnd);

			if (objFileDlg.DoModal() == IDOK)
			{
				csNewFilePath = objFileDlg.GetPathName();

				if (csExtension.CompareNoCase(L"*.AK") != -1)
				{
					csUserAddExtension = csNewFilePath.Mid(csNewFilePath.ReverseFind('.') + 1);
					if (csExtension.CompareNoCase(L"*.AK") == -1)
					{
						csNewFilePath = csNewFilePath + L".AK";
					}
				}
			}
			else
			{
				csNewFilePath = L"";
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::SaveAsDoublicateFile", 0, 0, true, SECONDLEVEL);
	}
	return csNewFilePath;
}

/**********************************************************************************************************
*  Function Name  :	On_PauseDataCryptOpr
*  Description    :	Pause Encryption/Decryption operation after user's click on close button.
*  Author Name    : Nitin Kolapkar
*  Date           : 25rd July 2015
*  SR_NO		  :
**********************************************************************************************************/
json::value CWardWizDataCrypt::On_PauseDataCryptOpr()
{
	try
	{
		if (!SendDataEncryptionOperation2Service(WWIZ_CRYPT_SERVER, PAUSE_CRYPT_OPR, L"", L"", 0, 0, true))
		{
			AddLogEntry(L"### Dataencryption : Error in Vibranium_CRYPT_SERVER in CDataencryption:SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
		}
		AddLogEntry(L">>> Encryption/Decryption operation Paused.", 0, 0, true, ZEROLEVEL);
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::PauseEncryptionDecryption", 0, 0, true, SECONDLEVEL);
		return false;
	}
}

/**********************************************************************************************************
*  Function Name  :	On_ResumeDataCryptOpr
*  Description    :	Resume Encryption/Decryption operation after user's response to message pop-up on click on close button.
*  Author Name    : Nitin Kolapkar
*  Date           : 25rd July 2015
*  SR_NO		  :
**********************************************************************************************************/
json::value CWardWizDataCrypt::On_ResumeDataCryptOpr()
{
	try
	{
		if (!SendDataEncryptionOperation2Service(WWIZ_CRYPT_SERVER, RESUME_CRYPT_OPR, L"", L"", 0, 0, true))
		{
			AddLogEntry(L"### Dataencryption : Error in Vibranium_CRYPT_SERVER in CDataencryption:SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
		}
		AddLogEntry(L">>> Encryption/Decryption operation Resumed.", 0, 0, true, ZEROLEVEL);
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::ResumeEncryptionDecryption", 0, 0, true, SECONDLEVEL);
		return false;
	}

}
/**********************************************************************************************************
*  Function Name  :	CheckForNetworkPath
*  Description    :	To check weather file is network file or not.
*  Author Name    : Jeena Mariam Saji
*  SR_NO		  :
*  Date           : 04 Oct 2016
**********************************************************************************************************/
json::value CWardWizDataCrypt::CheckForNetworkPath(SCITER_VALUE svFilePath)
{
	try
	{
		const std::wstring  chFilePath = svFilePath.get(L"");
		if (PathIsNetworkPath((LPTSTR)chFilePath.c_str()))
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::CheckForNetworkPath", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	OnCheckFileExists
*  Description    :	To check weather file exists or not.
*  Author Name    : Jeena Mariam Saji
*  SR_NO		  :
*  Date           : 09 Jan 2018
**********************************************************************************************************/
json::value CWardWizDataCrypt::OnCheckFileExists(SCITER_VALUE svFilePath)
{
	try
	{
		const std::wstring  chFilePath = svFilePath.get(L"");
		if (PathFileExists((LPTSTR)chFilePath.c_str()))
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::OnCheckFileExists", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	On_CallContinueDataEncDec
*  Description    :	To continue crypt operation in background
*  Author Name    : Jeena Mariam Saji
*  SR_NO		  :
*  Date           : 26 April 2018
**********************************************************************************************************/
json::value CWardWizDataCrypt::On_CallContinueDataEncDec()
{
	try
	{
		theApp.m_bIsDataEncDecUIReceptive = false;
		theApp.m_bIsDataEncDecPageSwitched = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::On_CallContinueDataEncDec", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : OnSetTimer
Description    : Called when crypt operation is to be continued
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 26 April 2019
***********************************************************************************************/
json::value CWardWizDataCrypt::OnSetTimer()
{
	try
	{
		theApp.m_bIsDataEncDecUIReceptive = true;
		if (m_bIsCryptFinish)
		{
			OnCallSetCryptStatus(0x00);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::OnSetTimer", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : OnCallSetFinishStatus
Description    : Called when crypt operation status is to be sent
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 26 April 2019
***********************************************************************************************/
void CWardWizDataCrypt::OnCallSetCryptStatus(DWORD dwStatus)
{
	try
	{
		if (dwStatus != 0x00)
		{
			m_dwFinalStatus = dwStatus;
		}
		
		if (theApp.m_bIsDataEncDecUIReceptive)
		{
			CString csFinalStatusVal;
			csFinalStatusVal.Format(L"%d", m_dwFinalStatus);
			sciter::value map;
			map.set_item("one", sciter::string(csFinalStatusVal));
			map.set_item("two", sciter::string(theApp.m_csDataCryptFilePath));
			map.set_item("three", sciter::string(m_csFinalStatus));

			//Send here event
			sciter::dom::element ela = self;
			BEHAVIOR_EVENT_PARAMS params;
			params.cmd = SETDATAENCSTATUS_EVENT_CODE;
			params.he = params.heTarget = ela;
			params.data = map;
			ela.fire_event(params, true);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::OnCallSetFinishStatus", 0, 0, true, SECONDLEVEL);
	}
}