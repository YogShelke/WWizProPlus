/**********************************************************************************************************
Program Name			: WardWizCrypter.cpp
Description				: Wrapper class to encrypt files/folders using AES 256 algorithm.
Author Name				: Ramkrushna Shelke
Date Of Creation		: 5th Jun 2015
Version No				: 1.11.0.0
Special Logic Used		:
Modification Log		:
***********************************************************************************************************/
#include "stdafx.h"
#include "WardWizCrypter.h"
#include <io.h>
#include <Imagehlp.h>

CWardWizCrypter::CWardWizCrypter()
{
	m_bStopOpr = false;
	m_csOutputFrmDecrpt = L"";
	m_csSaveAsPathDecrpt = L"";
	m_dwFileChecksum = 0 ;
}

CWardWizCrypter::~CWardWizCrypter()
{
}

void CWardWizCrypter::Initialize()
{
}

DWORD CWardWizCrypter::EncryptFile(LPCTSTR lpFilePath, LPCTSTR lpOrgFileName, LPCTSTR lpOutFilePath, LPCTSTR lpPassword, void(*_callback)(DWORD dwPercent, void * param), void	*callbackParam)
{
	DWORD dwRet = 0x00;
	HANDLE hInputFileHandle = NULL;
	HANDLE hOutputFileHandle = NULL;
	m_bStopOpr = false;
	__try
	{
		if (!lpFilePath || !lpOutFilePath || !lpPassword )
		{
			dwRet = 0x01;
			goto CLEANUP;
		}

		Initialize();

		DWORD dwBytesWritten = 0;
		QWORD dwFileSize = 0;
		QWORD dwBufferOffset = 0;
		BYTE  cbc_state[16] = { 0 };			//IV

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
		//Added By Nitin Kolapkar
		//For Checking whether the file is already encrypted or not
		//if (IsFileAlreadyEncrypted(hInputFileHandle))
		//Issue: Encryption Key Changed for New Implementation. SO need to check the specific Key at the time of Decryption
		//Return types : If not encrypted = 0x00;  If encrypted by new version : 0x01; If encrypted by old version : 0x02; If failed : 0x03;
		//Resolved By: Nitin K. Date: 14th August 2015
		DWORD dwRetValue = 0x00;
		dwRetValue = IsFileAlreadyEncrypted(hInputFileHandle);
		if (dwRetValue != 0x00)
		{
			dwRet = 0x06;
			goto CLEANUP;
		}
		SetFilePointer(hInputFileHandle, 0x00, NULL, FILE_BEGIN);

		hOutputFileHandle = CreateFile(lpOutFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutputFileHandle == INVALID_HANDLE_VALUE)
		{
			dwRet = 0x03;
			goto CLEANUP;
		}

		//Write here WardWiz Signature
		SetFilePointer(hOutputFileHandle, 0x00, NULL, FILE_BEGIN);
		WriteFile(hOutputFileHandle, WRDWIZ_SIG_NEW, WRDWIZ_SIG_SIZE, &dwBytesWritten, NULL); // Write sig "WARDWIZ"
		if (dwBytesWritten != WRDWIZ_SIG_SIZE)
		{
			dwRet = 0x4;
			goto CLEANUP;
		}

		//Generate Random IV & Write to Output File
		srand(GetTickCount());
		for (unsigned char _ix = 0; _ix < 16; _ix++){
			cbc_state[_ix] = (rand() & 0x00ff);
		}

		DWORD *pdwexpanded_key = NULL;		//Expanded key
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

		//Key expansion
		Rijndael_set_key_encrypt(pdwexpanded_key, _CryptoCipherKey, 256);

		TCHAR szFileName[MAX_PATH * 2] = { 0 };
		//_tcscpy_s(szFilePath, lpFilePath);

		if (!GetFileNameFromPath(lpFilePath, szFileName, _countof(szFileName)))
		{
			dwRet = 0x5;
			goto CLEANUP;
		}
		//Neha Gharge 10 july,2015 If DataEncVersionno Mismatched
		GetDataEncVersion();
		WRDWIZ_FILECRYPTOPARAM szCryptoParam = { 0 };
		memset(&szCryptoParam, 0, sizeof(WRDWIZ_FILECRYPTOPARAM));
		//_tcscpy_s(szCryptoParam.szFileName, szFileName);
		_tcscpy_s(szCryptoParam.szFileName, lpOrgFileName);
		_tcscpy_s(szCryptoParam.szPassword, lpPassword);
		szCryptoParam.dwCryptVersion = m_dwDataEncVersionNo;

		WriteFile(hOutputFileHandle, &cbc_state, 16, &dwBytesWritten, NULL); // Write encrypted data in file

		//CBC Encrypt Blocks
		_AES_Encrypt_CBC((BYTE*)&szCryptoParam, (sizeof(WRDWIZ_FILECRYPTOPARAM) / 16), pdwexpanded_key, cbc_state);

		WriteFile(hOutputFileHandle, &szCryptoParam, sizeof(WRDWIZ_FILECRYPTOPARAM), &dwBytesWritten, NULL); // Write encrypted data in file

		while (true)
		{
			if (m_bStopOpr == true)
			{
				dwRet = 0x09;
				break;
			}
			memset(m_szFileBuffer, 0, sizeof(m_szFileBuffer));

			DWORD dwBytesRead = 0;
			if (!ReadFile(hInputFileHandle, &m_szFileBuffer, FILEBUFFSIZE, &dwBytesRead, NULL))
			{
				dwRet = 0x03;
				goto CLEANUP;
			}

			dwBufferOffset += dwBytesRead;

			//Calculate Percentage here
			DWORD dwPercent = (unsigned int)(100 * ((dwBufferOffset + 1) * 1.00 / dwFileSize));
			static DWORD dwPreperc = dwPercent;
			if (dwPreperc != dwPercent)
			{
				// Operation Process
				if (_callback != NULL){
					_callback(dwPercent, callbackParam);
				}
			}
			dwPreperc = dwPercent;

			if (dwBytesRead == FILEBUFFSIZE)
			{
				//CBC Encrypt Blocks
				_AES_Encrypt_CBC(m_szFileBuffer, (FILEBUFFSIZE / 16), pdwexpanded_key, cbc_state);

				WriteFile(hOutputFileHandle, &m_szFileBuffer, dwBytesRead, &dwBytesWritten, NULL); // Write encrypted data in file
			}
			else
			{
				//Pad
				m_szFileBuffer[dwBytesRead] = 0x57;
				dwBytesRead += 1;

				m_szFileBuffer[dwBytesRead] = 0x57;
				dwBytesRead += 1;

				m_szFileBuffer[dwBytesRead] = 0x49;
				dwBytesRead += 1;

				m_szFileBuffer[dwBytesRead] = 0x5A;
				dwBytesRead += 1;

				dwBytesRead += (16 - ((dwBytesRead) % 16));

				//CBC Encrypt Blocks
				_AES_Encrypt_CBC(m_szFileBuffer, (dwBytesRead / 16), pdwexpanded_key, cbc_state);

				//Write file
				WriteFile(hOutputFileHandle, &m_szFileBuffer, dwBytesRead, &dwBytesWritten, NULL); // Write encrypted data in file

				//Exit for
				break;

			}
		}

		//Need to close file handle after collecting buffer
		if (hInputFileHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hInputFileHandle);
			hInputFileHandle = INVALID_HANDLE_VALUE;
		}

		//Need to close file handle after Writting buffer
		if (hOutputFileHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hOutputFileHandle);
			hOutputFileHandle = INVALID_HANDLE_VALUE;
		}

		if (pdwexpanded_key != NULL)
		{
			delete[]pdwexpanded_key;
			pdwexpanded_key = NULL;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardWizCrypter::EncryptFile [File: %s]", lpFilePath, 0, true, SECONDLEVEL);
	}
CLEANUP:
	//Need to close file handle after collecting buffer
	if (hInputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hInputFileHandle);
		hInputFileHandle = INVALID_HANDLE_VALUE;
	}
	
	//Need to close file handle after Writting buffer
	if (hOutputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hOutputFileHandle);
		hOutputFileHandle = INVALID_HANDLE_VALUE;
	}
	return dwRet;
}

DWORD CWardWizCrypter::DecryptFile(LPCTSTR lpFilePath, LPCTSTR lpOutFilePath, LPCTSTR lpTempPath,BOOL bTmpRequired, LPCTSTR lpPassword, void(*_callback)(DWORD dwPercent, void * param), void(*_callbackSaveAs)(LPTSTR lpOrgPath, void * param), void	*callbackParam)
{
	DWORD dwRet = 0x00;
	HANDLE hInputFileHandle = NULL;
	HANDLE hOutputFileHandle = NULL;
	DWORD *pdwexpanded_key = NULL;		//Expanded key
	m_bStopOpr = false;
	m_csOutputFrmDecrpt = L"";
	__try
	{
		if (!lpFilePath || !lpOutFilePath || !lpPassword)
		{
			dwRet = 0x01;
			goto CLEANUP;
		}

		Initialize();

		DWORD dwBytesWritten = 0;
		QWORD dwFileSize = 0;
		QWORD dwBufferOffset = 0;
		bool  bLastBlock = 0;
		//** Block for XOR operations in CBC Mode **
		BYTE _cbc_iv[16] = { 0 };				//IV
		BYTE _cbc_state_prev[16] = { 0 };		//Previous encrypted block
		BYTE _cbc_state_curr[16] = { 0 };		//Current encrypted block
		//** CBC Mode End **

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

		//Check here wheather the file is already encrypted by wardwiz
		//Issue: Encryption Key Changed for New Implementation. SO need to check the specific Key at the time of Decryption
		//Return types : If not encrypted = 0x00;  If encrypted by new version : 0x01; If encrypted by old version : 0x02; If failed : 0x03;
		//Resolved By: Nitin K. Date: 14th August 2015
		DWORD dwRetValue = 0x00;
		dwRetValue = IsFileAlreadyEncrypted(hInputFileHandle);
		//if (!IsFileAlreadyEncrypted(hInputFileHandle))
		if (dwRetValue != 0x01)
		{
			//Added By Nitin Kolapkar
			//Return type value changed
			dwRet = 0x07;
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

		//Neha Gharge 10 july,2015 If DataEncVersionno Mismatched
		GetDataEncVersion();
		if (szCryptoParam.dwCryptVersion != m_dwDataEncVersionNo)
		{
			m_dwDataEncVerfromFile = szCryptoParam.dwCryptVersion;
			dwRet = 0x0A;
			goto CLEANUP;
		}

		//Check here wheather the file is already encrypted by wardwiz
		if (!VerifyPassword(szCryptoParam.szPassword, lpPassword))
		{
			//Added By Nitin Kolapkar
			//Return type value changed
			dwRet = 0x08;
			goto CLEANUP;
		}
		//szCryptoParam.szFileName
		TCHAR szOutputPath[MAX_PATH] = { 0 };
		TCHAR szTempOutputPath[MAX_PATH] = { 0 };
		swprintf_s(szOutputPath, _countof(szOutputPath), L"%s", lpOutFilePath);

		TCHAR	*pTemp = wcsrchr(szOutputPath, '\\');
		if (!pTemp)
		{
			AddLogEntry(L"### Failed to creating output file path CWardWizCrypter::DecryptFile");
			dwRet = 0x04;
			goto CLEANUP;
		}
		*pTemp = '\0';

		swprintf_s(szOutputPath, _countof(szOutputPath), L"%s\\%s", szOutputPath, szCryptoParam.szFileName);
	// issue - functionality of saveAs added if orginal file already exist.
    // lalit kumawat 7-4-2015
		swprintf_s(szTempOutputPath, _countof(szTempOutputPath), L"%s", szOutputPath);
		
		m_csOutputFrmDecrpt = szOutputPath; /// fath for decrypt file output 

		TCHAR	*pTemp2 = wcsrchr(szTempOutputPath, '.');
		if (!pTemp2)
		{
			AddLogEntry(L"### Failed to create output file path CWardWizCryptDlg::DecryptFile");
			//return false; // it will overwrite existing file if fail to create path.
		}
		*pTemp2 = '\0';

		if (PathFileExists(szTempOutputPath))
		{
			if (_callbackSaveAs != NULL)
			{
				_callbackSaveAs(szOutputPath,callbackParam);

				if (m_csSaveAsPathDecrpt == L"")
				{
					AddLogEntry(L">>> Failed Decryption cancelled by user during saveAs in CWardWizCrypter::DecryptFile [File: %s]", lpFilePath, 0, true, SECONDLEVEL);
					dwRet = 0x04;
					goto CLEANUP;
				}
				swprintf_s(szOutputPath, _countof(szOutputPath), L"%s.tmp", m_csSaveAsPathDecrpt);
				m_csOutputFrmDecrpt = szOutputPath;
			}
		}

		// resolved by lalit kumawat 7-27-2015
		// issue :if default disk does not have required space then creating temp file on another drive which have free space
		if (bTmpRequired)
		{
			TCHAR szFileName[MAX_PATH] = {0};

			if (!GetFileNameFromPath(szOutputPath, szFileName, _countof(szFileName)))
			{
				dwRet = 0x5;
				goto CLEANUP;
			}

			swprintf_s(szOutputPath, _countof(szOutputPath), L"%s\\%s", lpTempPath, szFileName);
			m_csOutputFrmDecrpt = szOutputPath;

		}
		

		hOutputFileHandle = CreateFile(szOutputPath, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutputFileHandle == INVALID_HANDLE_VALUE)
		{
			dwRet = 0x04;
			goto CLEANUP;
		}

		SetFilePointer(hOutputFileHandle, 0x00, NULL, FILE_BEGIN);

		while (true)
		{
			if (m_bStopOpr == true)
			{
				dwRet = 0x09;
				break;
			}

			//BYTE szFileBuffer[FILEBUFFSIZE] = { 0 };
			memset(m_szFileBuffer, 0, sizeof(m_szFileBuffer));

			if (!ReadFile(hInputFileHandle, &m_szFileBuffer, FILEBUFFSIZE, &dwBytesRead, NULL))
			{
				dwRet = 0x05;
				goto CLEANUP;
			}

			dwBufferOffset += dwBytesRead;


			//Calculate Percentage here
			DWORD dwPercent = (unsigned int)(100 * ((dwBufferOffset + 1) * 1.00 / dwFileSize));
			static DWORD dwPreperc = dwPercent;
			if (dwPreperc != dwPercent)
			{
				// Operation Process
				if (_callback != NULL){
					_callback(dwPercent, callbackParam);
				}
			}
			dwPreperc = dwPercent;

			//Pos += read _readBytes
			if (dwFileSize == dwBufferOffset){ bLastBlock = 1; }

			//CBC Decrypt Blocks
			if (dwBufferOffset == dwBytesRead)
			{
				//Set iv
				for (unsigned char _ix = 0; _ix < 16; _ix++){
					_cbc_state_curr[_ix] = _cbc_iv[_ix];
				}
			}

			//CBC Encrypt Blocks
			_AES_Decrypt_CBC(m_szFileBuffer, (dwBytesRead / 16), pdwexpanded_key, _cbc_state_prev, _cbc_state_curr);

			if (bLastBlock)
			{
				//UnPad
				//Remove 0x57 0x57 0x49...
				for (;;)
				{
					if ((m_szFileBuffer))
					{
						if ((m_szFileBuffer[dwBytesRead - 4] == 0x57) &&
							(m_szFileBuffer[dwBytesRead - 3] == 0x57) &&
							(m_szFileBuffer[dwBytesRead - 2] == 0x49) &&
							(m_szFileBuffer[dwBytesRead - 1] == 0x5A))
						{
							//remove 0x01
							dwBytesRead -= 4;
							break;
						}
						//remove 0x00
						dwBytesRead--;
					}
					else{
						//remove last byte of last block
						//_chsize_s(_file_out->_file, dwFileSize - 16 - 1);
						dwBytesRead = 0;
						break;
					}
				}
				//write last block
				//fwrite(_buf, _readBytes, 1, _file_out);
				WriteFile(hOutputFileHandle, &m_szFileBuffer, dwBytesRead, &dwBytesWritten, NULL); // Write encrypted data in file
				break;
			}

			WriteFile(hOutputFileHandle, &m_szFileBuffer, dwBytesRead, &dwBytesWritten, NULL); // Write encrypted data in file

			if ((dwBytesRead != FILEBUFFSIZE) || (dwBufferOffset == dwFileSize))
			{
				break;
			}
		}

		//Need to close file handle after collecting buffer
		if (hInputFileHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hInputFileHandle);
			hInputFileHandle = INVALID_HANDLE_VALUE;
		}

		//Need to close file handle after Writting buffer
		if (hOutputFileHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hOutputFileHandle);
			hOutputFileHandle = INVALID_HANDLE_VALUE;
		}

		if (pdwexpanded_key != NULL)
		{
			delete[]pdwexpanded_key;
			pdwexpanded_key = NULL;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardWizCrypter::DecryptFile [File: %s]", lpFilePath, 0, true, SECONDLEVEL);
	}
CLEANUP:
	//Need to close file handle after collecting buffer
	if (hInputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hInputFileHandle);
		hInputFileHandle = INVALID_HANDLE_VALUE;
	}

	//Need to close file handle after Writting buffer
	if (hOutputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hOutputFileHandle);
		hOutputFileHandle = INVALID_HANDLE_VALUE;
	}
	
	if (pdwexpanded_key != NULL)
	{
		delete[]pdwexpanded_key;
		pdwexpanded_key = NULL;
	}
	return dwRet;
}

DWORD CWardWizCrypter::Stop()
{
	DWORD dwRet = 0x00;
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : IsFileAlreadyEncrypted
*  Description    : Checks given file is already encrypted or not.
*  Author Name    : Prajkta Nanaware
*  SR_NO
*  Date           : 27 May 2014
* Return values:  If not encrypted = 0x00;  If encrypted by new version : 0x01; If encrypted by old version : 0x02; If failed : 0x03;
****************************************************************************************************/
DWORD CWardWizCrypter::IsFileAlreadyEncrypted(HANDLE hFile)
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
			//SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, INVALID_FILE);
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
		dwReturn = 0x03;
	}
	return dwReturn;
}

bool CWardWizCrypter::GetFileNameFromPath(LPCTSTR lpFilePath, LPTSTR lpOutFileName, DWORD dwOutSize)
{
	__try
	{
		TCHAR szFilePath[MAX_PATH * 2] = { 0 };
		_tcscpy_s(szFilePath, lpFilePath);

		const TCHAR *szPtr = _tcsrchr(szFilePath, L'\\');
		if (szPtr == NULL)
		{
			return false;
		}
		szPtr++;

		_tcscpy_s(lpOutFileName, dwOutSize, szPtr);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		return true;
	}
	return true;
}

bool CWardWizCrypter::VerifyPassword(LPCTSTR szFilePassword, LPCTSTR szUserPassword)
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
Function Name  : GetDataEncVersion
Description    : Get Data Encryption Version
SR.NO		   :
Author Name    : Neha Gharge
Date           : 10th July 2015
***********************************************************************************************/

DWORD CWardWizCrypter::GetDataEncVersion()
{
	__try
	{
		return GetDataEncVersionSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardWizCrypter::GetDataEncVersion", 0, 0, true, SECONDLEVEL);
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
DWORD CWardWizCrypter::GetDataEncVersionSEH()
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
			AddLogEntry(L"### Failed in CWardWizCrypter::GetDataEncVersionSEH::RegOpenKeyEx", 0, 0, true, FIRSTLEVEL);
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
		AddLogEntry(L"### Exception in CWardWizCrypter::GetDataEncVersionSEH", 0, 0, true, SECONDLEVEL);
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
DWORD CWardWizCrypter::ConvertStringTODWORD(TCHAR* szDataEncVersion)
{
	__try
	{
		TCHAR szDataEncVer[1024];
		_tcscpy_s(szDataEncVer, _countof(szDataEncVer), szDataEncVersion);
		return ConvertStringTODWORDSEH(szDataEncVer);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardWizCrypter::ConvertStringTODWORD", 0, 0, true, SECONDLEVEL);
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
DWORD CWardWizCrypter::ConvertStringTODWORDSEH(TCHAR* szDataEncVersion)
{
	DWORD dwDataEncVersionNo = 0x00;
	try
	{
		if (!ParseVersionNo(szDataEncVersion))
		{
			AddLogEntry(L"### Failed to Parse data envryption version no in CWardWizCrypter::ConvertStringTODWORDSEH", 0, 0, true, FIRSTLEVEL);
			return 0x00;
		}
		WORD wDataEncLogicNo = 0;
		WORD wDataEncPatchNo = 0;
		TCHAR szDataEncVer[256] = { 0 };
	
		_tcscpy_s(szDataEncVer, _countof(szDataEncVer),m_szDataEncLogicNo);
		swscanf_s(szDataEncVer, L"%hu", &wDataEncLogicNo);
		_tcscpy_s(szDataEncVer, _countof(szDataEncVer),m_szDataEncPatchNo);
		swscanf_s(szDataEncVer, L"%hu", &wDataEncPatchNo);

		dwDataEncVersionNo = (wDataEncLogicNo << 16) +(wDataEncPatchNo);
		m_dwDataEncVersionNo = dwDataEncVersionNo;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCrypter::ConvertStringTODWORDSEH", 0, 0, true, SECONDLEVEL);
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
bool CWardWizCrypter::ParseVersionNo(LPTSTR lpszVersionNo)
{
	__try
	{
		return ParseVersionNoSEH(lpszVersionNo);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardWizCrypter::ParseVersionNo", 0, 0, true, SECONDLEVEL);
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
bool CWardWizCrypter::ParseVersionNoSEH(LPTSTR lpszVersionNo)
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

/***********************************************************************************************
Function Name  : CovertDataVersionNoDWORDToString
Description    : Convert data version no dword to string
SR.NO		   :
Author Name    : Neha Gharge
Date           : 10th July 2015
*******************************************************************************************/
bool CWardWizCrypter::CovertDataVersionNoDWORDToString()
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
		AddLogEntry(L"### Exception in CWardWizCrypter::CovertDataVersionNoDWORDToString()", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}
/***************************************************************************************************
*  Function Name  : AddCheckSum()
*  Description    : it uses to add check sum byte to encrypted file at end of file
*  Author Name    : Vilas
*  Date			  :	09-July-2015
*  SR No		  :
****************************************************************************************************/
DWORD CWardWizCrypter::AddCheckSum(LPCTSTR lpFileName)
{
	DWORD	dwRet = 0x00;
	DWORD dwReadBytes = 0x00;
	LARGE_INTEGER liDistToMove = { 0 };
	DWORD	dwFileOriginalChecksum = 0x00;
	DWORD dwFileComputedChecksum = 0x00;

	try
	{
		HANDLE	hFile = CreateFile(lpFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //Open with write access
		//if not handle
		if (hFile == INVALID_HANDLE_VALUE)
		{
			dwRet = 0x01;
			return dwRet;
		}

		SetFilePointerEx(hFile, liDistToMove, NULL, FILE_END);

		MapFileAndCheckSum(lpFileName, &dwFileOriginalChecksum, &dwFileComputedChecksum);

		WriteFile(hFile, &dwFileComputedChecksum, sizeof(DWORD), &dwReadBytes, NULL);

		CloseHandle(hFile);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCrypter::AddCheckSum", 0, 0, true, SECONDLEVEL);
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
DWORD CWardWizCrypter::AddCheckSum(LPCTSTR lpFileName, DWORD dwByteNeedtoAdd)
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
		AddLogEntry(L"### Exception in CWardWizCrypter::AddCheckSum", 0, 0, true, SECONDLEVEL);
	}

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : CheckFileIntegrityBeforeDecryption()
*  Description    : Checks file for modified. If file is not modified, returns 0 else 1
*  Author Name    : Vilas
*  Date			  :	09-July-2015
*  SR No		  :
****************************************************************************************************/
DWORD CWardWizCrypter::CheckFileIntegrityBeforeDecryption(LPCTSTR lpFileName)
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
			dwRet = 0x03;
			dwOut = AddCheckSum(lpFileName, dwStoredChecksum);
			if (dwOut)
			{
				AddLogEntry(L">>> Error in CWardWizCrypter::AddCheckSum to set removed checksum as file is modified by extenal resource", 0, 0, true, SECONDLEVEL);
				dwRet = 0x04;
			}
			
		}
		/*else
		{*/ 
			m_dwFileChecksum = dwStoredChecksum;
		//}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCrypter::CheckFileIntegrityBeforeDecryption", 0, 0, true, SECONDLEVEL);
	}


	return dwRet;

}