/************************************************************************************************
*  Program Name		: BBBSt.cpp
*  Description		: 
*  Author Name		: Ram Shelke
*  Date Of Creation	: 13 Jul 2019
*  Version No		: 3.1.0.0
*************************************************************************************************/

#include "stdafx.h"
#include "BBBSt.h"

//#define GENERATE_DEBUG_DATA

BYTE HEADER_BBBST[24] = { "WRDWIZURLC|03.05.00.14|" };
BYTE HEADER_BBBST_DATA[24]={0};

/***************************************************************************************************
Function       : CBBBSt
Description    : constructor 
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
CBBBSt::CBBBSt(bool bIsEmbedded, DWORD cbKey1, DWORD cbKey2, DWORD cbKey3, DWORD cbData3, bool bLoadReadOnly):
		CBalBSTOpt(bIsEmbedded)
{
	m_cbKey1 = cbKey1;
	m_cbKey2 = cbKey2;
	m_cbKey3 = cbKey3;
	m_cbData3 = cbData3;
	m_bLoadError = false;
	m_bSaveError = false;
	m_dwTotalObjectsCount = 0;
	m_bLoadReadOnly = bLoadReadOnly;
}

/***************************************************************************************************
Function       : ~CBBBSt
Description    : destructor 
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
CBBBSt::~CBBBSt()
{
	RemoveAll();
}

/***************************************************************************************************
Function       : Compare
Description    : compare key and return large, small, equal
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
COMPARE_RESULT CBBBSt::Compare(SIZE_T nKey1, SIZE_T nKey2)
{
	LPBYTE f = (LPBYTE)nKey1;
	LPBYTE s = (LPBYTE)nKey2;

	for(DWORD i = 0; i < m_cbKey1; i++, f++, s++)
	{
		if(*f < *s)
		{
			return SMALL;
		}

		if(*f > *s)
		{
			return LARGE;
		}
	}

	return EQUAL;
}

/***************************************************************************************************
Function       : FreeKey
Description    : release key memory
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
void CBBBSt::FreeKey(SIZE_T nKey)
{
	if(((LPBYTE)nKey < m_pBuffer) ||((LPBYTE)nKey >= m_pBuffer + m_nBufferSize))
	{
		Release((LPVOID &)nKey);
	}
}

/***************************************************************************************************
Function       : FreeData
Description    : release data memory
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
void CBBBSt::FreeData(SIZE_T nData)
{
	CBBSt objBBSt(false, m_cbKey2, m_cbKey3, m_cbData3);
	objBBSt.SetDataPtr((PNODEOPT)nData, m_pBuffer, m_nBufferSize);
	objBBSt.RemoveAll ();
	return;
}

/***************************************************************************************************
Function       : AppendItemAscOrder
Description    : append node in ascending order in vine
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::AppendItemAscOrder(LPVOID lpvKey, CBBSt& objBB2St)
{
	LPBYTE pbyKeyDup = 0;

	pbyKeyDup = DuplicateBuffer((LPBYTE)lpvKey, m_cbKey1);
	if(NULL == pbyKeyDup)
	{
		return false;
	}

	if(!AddNodeAscOrder((SIZE_T)pbyKeyDup,(SIZE_T)objBB2St.GetDataPtr()))
	{
		Release((LPVOID&)pbyKeyDup);
		return false;
	}

	return true;
}

/***************************************************************************************************
Function       : AppendItem
Description    : append node in tree
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::AppendItem(LPVOID lpvKey, CBBSt& objBB2St)
{
	LPBYTE pbyKeyDup = 0;

	pbyKeyDup = DuplicateBuffer((LPBYTE)lpvKey, m_cbKey1);
	if(NULL == pbyKeyDup)
	{
		return false;
	}

	if(!AddNode((SIZE_T)pbyKeyDup,(SIZE_T)objBB2St.GetDataPtr()))
	{
		Release((LPVOID&)pbyKeyDup);
		return false;
	}

	return true;
}

/***************************************************************************************************
Function       : DeleteItem
Description    : delete item from tree
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::DeleteItem(LPVOID lpvKey)
{
	return (DeleteNode((SIZE_T)lpvKey));
}

/***************************************************************************************************
Function       : SearchItem
Description    : search item and return data
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::SearchItem(LPVOID lpvKey, CBBSt& objBB2St)
{
	SIZE_T nData = 0;

	if(!FindNode((SIZE_T)lpvKey, nData))
	{
		return false;
	}

	objBB2St.SetDataPtr((PNODEOPT)nData, m_pBuffer, m_nBufferSize);
	return true;
}

/***************************************************************************************************
Function       : UpdateItem
Description    : update the data of given key
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::UpdateItem(LPVOID lpvKey, CBBSt& objBB2St)
{
	if(!m_pLastSearchResult || EQUAL != Compare(m_pLastSearchResult->nKey,(SIZE_T)lpvKey))
	{
		SIZE_T nData = 0;

		if(!FindNode((SIZE_T)lpvKey, nData))
		{
			return false;
		}
	}

	m_pLastSearchResult->nData = (SIZE_T)objBB2St.GetDataPtr();
	return true;
}

/***************************************************************************************************
Function       : GetKey
Description    : 
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::GetKey(PVOID pVPtr, LPVOID& lpvKey)
{
	if(!pVPtr)
	{
		return false;
	}

	lpvKey = (LPVOID&)(((PNODEOPT)pVPtr) -> nKey);
	return true;
}

/***************************************************************************************************
Function       : GetData
Description    : 
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::GetData(PVOID pVPtr, CBBSt& objBB2St)
{
	if(!pVPtr)
	{
		return false;
	}

	objBB2St.SetDataPtr((PNODEOPT)(((PNODEOPT)pVPtr) -> nData), m_pBuffer, m_nBufferSize);
	return true;
}

bool CBBBSt::Balance()
{
	LPVOID Position = NULL;

	CBalBSTOpt::Balance();

	Position = GetFirst();
	while(Position)
	{
		CBBSt objBBSt(true, m_cbKey2, m_cbKey3, m_cbData3);
		GetData(Position, objBBSt);
		objBBSt.Balance();
		((PNODEOPT)Position) -> nData = (SIZE_T)objBBSt.GetDataPtr();
		Position = GetNext(Position);
	}

	return true;
}

/***************************************************************************************************
Function       : AppendObject
Description    : merge another object into this object
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::AppendObject(CBalBSTOpt& objToAdd)
{
	CBBBSt& objAddL1 = (CBBBSt&)objToAdd;
	LPVOID lpContext1 = NULL, lpContext2 = NULL, lpContext3 = NULL;
	LPBYTE lpKey1 = NULL, lpKey2 = NULL, lpKey3 = NULL, lpData3 = NULL;
	CBufferToStructure objAddL3(true, m_cbKey3, m_cbData3);
	CBufferToStructure objThisL3(true, m_cbKey3, m_cbData3);
	CBBSt objAddL2(true, m_cbKey2, m_cbKey3, m_cbData3);
	CBBSt objThisL2(true, m_cbKey2, m_cbKey3, m_cbData3);

	lpContext1 = objAddL1.GetFirst();
	while(lpContext1)
	{
		objAddL1.GetKey(lpContext1, (LPVOID&)lpKey1);
		objAddL1.GetData(lpContext1, objAddL2);

		if(SearchItem(lpKey1, objThisL2))
		{
			lpContext2 = objAddL2.GetFirst();
			while(lpContext2)
			{
				objAddL2.GetKey(lpContext2, (LPVOID&)lpKey2);
				objAddL2.GetData(lpContext2, objAddL3);

				if(objThisL2.SearchItem(lpKey2, objThisL3))
				{
					lpContext3 = objAddL3.GetFirst();
					while(lpContext3)
					{
						objAddL3.GetKey(lpContext3, (LPVOID&)lpKey3);
						objAddL3.GetData(lpContext3, (LPVOID&)lpData3);

						if(lpKey3 && lpData3)
						{
							if(objThisL3.AppendItem(lpKey3, lpData3))
							{
								SetModified();
							}
						}

						lpContext3 = objAddL3.GetNext(lpContext3);
					}
				}
				else
				{
					if(objAddL3.GetFirst())
					{
						objThisL3.RemoveAll();
						if(objAddL3.CreateObject(objThisL3))
						{
							if(objThisL2.AppendItem(lpKey2, objThisL3))
							{
								SetModified();
							}
						}
					}
				}

				lpContext2 = objAddL2.GetNext(lpContext2);
			}
		}
		else
		{
			if(objAddL2.GetFirst())
			{
				objThisL2.RemoveAll();
				if(objAddL2.CreateObject(objThisL2))
				{
					if(AppendItem(lpKey1, objThisL2))
					{
						SetModified();
					}
				}
			}
		}

		lpContext1 = objAddL1.GetNext(lpContext1);
	}

	return true;
}

/***************************************************************************************************
Function       : DeleteObject
Description    : delete all entries from this object of the given object
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::DeleteObject(CBalBSTOpt& objToDel)
{
	LPBYTE lpKey1 = 0, lpKey2 = 0, lpKey3 = 0;
	LPVOID lpContext1 = 0, lpContext2 = 0, lpContext3 = 0;
	CBBBSt& objDelL1 = (CBBBSt&)objToDel;
	CBufferToStructure objDelL3(true, m_cbKey3, m_cbData3);
	CBufferToStructure objThisL3(true, m_cbKey3, m_cbData3);
	CBBSt objDelL2(true, m_cbKey2, m_cbKey3, m_cbData3);
	CBBSt objThisL2(true, m_cbKey2, m_cbKey3, m_cbData3);

	lpContext1 = objDelL1.GetFirst();
	while(lpContext1)
	{
		objDelL1.GetKey(lpContext1, (LPVOID&)lpKey1);
		objDelL1.GetData(lpContext1, objDelL2);

		if(SearchItem(lpKey1, objThisL2))
		{
			lpContext2 = objDelL2.GetFirst();
			while(lpContext2)
			{
				objDelL2.GetKey(lpContext2, (LPVOID&)lpKey2);
				objDelL2.GetData(lpContext2, objDelL3);

				if(objThisL2.SearchItem(lpKey2, objThisL3))
				{
					lpContext3 = objDelL3.GetFirst();
					while(lpContext3)
					{
						objDelL3.GetKey(lpContext3, (LPVOID&)lpKey3);

						if(lpKey3)
						{
							if(objThisL3.DeleteItem(lpKey3))
							{
								SetModified();
							}
						}

						lpContext3 = objDelL3.GetNext(lpContext3);
					}

					objThisL2.UpdateItem(lpKey2, objThisL3);
					if(!objThisL3.GetFirst())
					{
						if(objThisL2.DeleteItem(lpKey2))
						{
							SetModified();
						}
					}
				}

				lpContext2 = objDelL2.GetNext(lpContext2);
			}

			UpdateItem(lpKey1, objThisL2);
			if(!objThisL2.GetFirst())
			{
				if(DeleteItem(lpKey1))
				{
					SetModified();
				}
			}
		}

		lpContext1 = objDelL1.GetNext(lpContext1);
	}

	return true;
}

/***************************************************************************************************
Function       : CreateObject
Description    : make a new copy of this object
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::CreateObject(CBBBSt& objNewCopy)
{
	LPBYTE lpKey = 0;
	LPVOID lpContext = 0;
	CBBSt objNewL2(true, m_cbKey2, m_cbKey3, m_cbData3);
	CBBSt objThisL2(true, m_cbKey2, m_cbKey3, m_cbData3);

	lpContext = GetFirst();
	while(lpContext)
	{
		lpKey = NULL;
		objNewL2.RemoveAll();
		objThisL2.RemoveAll();

		GetKey(lpContext, (LPVOID&)lpKey);
		GetData(lpContext, objThisL2);

		if(lpKey && objThisL2.GetFirst())
		{
			if(objThisL2.CreateObject(objNewL2))
			{
				objNewCopy.AppendItem(lpKey, objNewL2);
			}
		}

		lpContext = GetNext(lpContext);
	}

	return true;
}

/***************************************************************************************************
Function       : SearchObject
Description    : search all the entries from 'objToSearch'
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::SearchObject(CBalBSTOpt& objToSearch, bool bAllPresent)
{
	bool bSuccess = true, bFound = false;
	LPBYTE lpKey1 = 0, lpKey2 = 0, lpKey3 = 0, lpData3 = 0;
	LPVOID lpContext1 = 0, lpContext2 = 0, lpContext3 = 0;
	CBBBSt& objSearchL1 = (CBBBSt&)objToSearch;
	CBufferToStructure objThisL3(true, m_cbKey3, m_cbData3);
	CBufferToStructure objSearchL3(true, m_cbKey3, m_cbData3);
	CBBSt objThisL2(true, m_cbKey2, m_cbKey3, m_cbData3);
	CBBSt objSearchL2(true, m_cbKey2, m_cbKey3, m_cbData3);

	lpContext1 = objSearchL1.GetFirst();
	while(lpContext1 && bSuccess)
	{
		objSearchL1.GetKey(lpContext1, (LPVOID&)lpKey1);
		objSearchL1.GetData(lpContext1, objSearchL2);

		if(SearchItem(lpKey1, objThisL2))
		{
			lpContext2 = objSearchL2.GetFirst();
			while(lpContext2 && bSuccess)
			{
				objSearchL2.GetKey(lpContext2, (LPVOID&)lpKey2);
				objSearchL2.GetData(lpContext2, objSearchL3);

				if(objThisL2.SearchItem(lpKey2, objThisL3))
				{
					lpContext3 = objSearchL3.GetFirst();
					while(lpContext3 && bSuccess)
					{
						lpContext3 = objSearchL3.GetFirst();
						while(lpContext3 && bSuccess)
						{
							objSearchL3.GetKey(lpContext3, (LPVOID&)lpKey3);

							if(lpKey3)
							{
								bFound = objThisL3.SearchItem(lpKey3, (LPVOID&)lpData3);
								if((bFound && !bAllPresent) || (!bFound && bAllPresent))
								{
									bSuccess = false;
								}
							}

							lpContext3 = objSearchL3.GetNext(lpContext3);
						}
					}
				}
				else
				{
					bSuccess = bAllPresent?false:bSuccess;
				}

				lpContext2 = objSearchL2.GetNext(lpContext2);
			}
		}
		else
		{
			bSuccess = bAllPresent?false:bSuccess;
		}

		lpContext1 = objSearchL1.GetNext(lpContext1);
	}

	return bSuccess;
}

/***************************************************************************************************
Function       : ReadBBBSt
Description    : read bbbst object from file
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::ReadBBBSt(LPBYTE& ptrData, PSIZE_T& ptrNode, LPBYTE byBuffer, DWORD cbBuffer)
{
	LPBYTE ptrDataDup = 0;
	PSIZE_T ptrNext = NULL;
	BOOL bAllowNull = FALSE;
	CBBSt objBBSt(true, m_cbKey2, m_cbKey3, m_cbData3);
	DWORD dwNodesCount = 0, dwTotalNodesToSkip = 0, dwL2Count = 0, dwL3Count = 0;

	VALIDATE_POINTER(ptrData,byBuffer,cbBuffer);
	dwNodesCount = *((LPDWORD)ptrData);
	ptrData += sizeof(dwNodesCount);

	for(ULONG64 i = 0; i < dwNodesCount; i++)
	{
		if(i + 1 < dwNodesCount)
		{
			ptrDataDup = ptrData;
			dwL2Count = dwL3Count = dwTotalNodesToSkip = 0;

			ptrDataDup += m_cbKey1;
			VALIDATE_POINTER(ptrDataDup,byBuffer,cbBuffer);
			dwL2Count = *((LPDWORD)ptrDataDup);
			ptrDataDup += sizeof(DWORD);

			for(ULONG64 i = 0; i < dwL2Count; i++)
			{
				ptrDataDup += m_cbKey2;
				VALIDATE_POINTER(ptrDataDup,byBuffer,cbBuffer);
				dwL3Count = *((LPDWORD)ptrDataDup);
				ptrDataDup += sizeof(DWORD);
				dwTotalNodesToSkip += dwL3Count;
				ptrDataDup += ((m_cbKey3 + m_cbData3) * dwL3Count);
			}

			dwTotalNodesToSkip += dwL2Count + 1;
			ptrNext = ptrNode + (dwTotalNodesToSkip * NUMBER_OF_NODEOPT_ELEMENTS);
			bAllowNull = FALSE;
		}
		else
		{
			ptrNext = NULL;
			bAllowNull = TRUE;
		}

		CHECK_AND_MAKE_POINTER2(ptrNode, (SIZE_T)ptrData, byBuffer, cbBuffer, FALSE);
		ptrData += m_cbKey1;
		ptrNode++;

		CHECK_AND_MAKE_POINTER2(ptrNode, (SIZE_T)(ptrNode+3), byBuffer, cbBuffer, FALSE);
		ptrNode++;

		CHECK_AND_MAKE_POINTER2(ptrNode, (SIZE_T)NULL, byBuffer, cbBuffer, TRUE);
		ptrNode++;

		CHECK_AND_MAKE_POINTER2(ptrNode, (SIZE_T)ptrNext, byBuffer, cbBuffer, bAllowNull);
		ptrNode++;

		if(!objBBSt.ReadBBSt(ptrData, ptrNode, byBuffer, cbBuffer))
		{
			return false;
		}
	}

	return true;

ERROR_EXIT:
	return false;
}

/***************************************************************************************************
Function       : Load
Description    : loading the file
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::Load(LPCTSTR szFileName, bool bCheckVersion)
{
	LPBYTE ptrData = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwFileSize = 0, dwBytesRead = 0;
	TCHAR szFullFileName[MAX_PATH]={0};
	PSIZE_T ptrNode = NULL;
	ULONG64 ulTotalNodesCount = 0;
	BYTE byHdrBfr[sizeof(HEADER_BBBST) + sizeof(HEADER_BBBST_DATA)] ={0};

	m_pBuffer = NULL;

	if(false == MakeFullFilePath(szFileName, szFullFileName, _countof(szFullFileName)))
	{
		return false;
	}

	hFile = CreateFile(szFullFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		return false;
	}

	if(FALSE == ReadFile(hFile, byHdrBfr, sizeof(byHdrBfr), &dwBytesRead, 0))
	{
		goto ERROR_EXIT;
	}

	if(!CreateHeaderData(hFile, szFullFileName, HEADER_BBBST_DATA, sizeof(HEADER_BBBST_DATA)))
	{
		goto ERROR_EXIT;
	}

	if(bCheckVersion && memcmp(HEADER_BBBST, byHdrBfr, sizeof(HEADER_BBBST)))
	{
		goto ERROR_EXIT;
	}

	if(memcmp(byHdrBfr + sizeof(HEADER_BBBST), HEADER_BBBST_DATA, 8))
	{
		goto ERROR_EXIT;
	}

	memcpy(&ulTotalNodesCount, byHdrBfr + sizeof(HEADER_BBBST) + 8 + 8, sizeof(ulTotalNodesCount));

	if(0 == ulTotalNodesCount)
	{
		CloseHandle(hFile);
		return true;
	}

	dwFileSize = GetFileSize(hFile, 0);
	if(dwFileSize <= sizeof(byHdrBfr))
	{
		goto ERROR_EXIT;
	}

	dwFileSize -= sizeof(byHdrBfr);
	m_nBufferSize = dwFileSize + (((DWORD)ulTotalNodesCount) * SIZE_OF_NODEOPT);

	m_pBuffer = (LPBYTE)VAllocate(m_nBufferSize);
	if(NULL == m_pBuffer)
	{
		goto ERROR_EXIT;
	}

	if(FALSE == ReadFile(hFile, m_pBuffer, dwFileSize, &dwBytesRead, 0))
	{
		goto ERROR_EXIT;
	}

	if(dwFileSize != dwBytesRead)
	{
		goto ERROR_EXIT;
	}

	CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;

	/*
	//md5 checksum
	{
		ULONG64 ulMD5CRC = 0;
		BYTE byMD5Checksum[16] = {0};
		if(!MD5Buffer(m_pBuffer, dwFileSize, byMD5Checksum, sizeof(byMD5Checksum)))
		{
			AddLogEntry(_T("Failed creating header checksum while loading: %s"), szFullFileName);
			goto ERROR_EXIT;
		}

		CreateCRC64Buffer(byMD5Checksum, sizeof(byMD5Checksum), ulMD5CRC);
		memcpy(byMD5Checksum, &ulMD5CRC, sizeof(ulMD5CRC));
		if(memcmp(byMD5Checksum, byHdrBfr + sizeof(HEADER_BBBST) + 8, sizeof(ulMD5CRC)))
		{
			AddLogEntry(_T("Header checksum mismatch while loading: %s"), szFullFileName);
			goto ERROR_EXIT;
		}
	}
	*/
#ifdef GENERATE_DEBUG_DATA
	DumpBuffer(m_pBuffer, dwBytesRead, _T("C:\\DebugDataBeforeCrypt.txt"), szFullFileName);
#endif

	/*if(!CryptBuffer(m_pBuffer, dwFileSize))
	{
		goto ERROR_EXIT;
	}*/

#ifdef GENERATE_DEBUG_DATA
	DumpBuffer(m_pBuffer, dwBytesRead, _T("C:\\DebugDataAfterCrypt.txt"), szFullFileName);
#endif

	// code required for debugging loading issues using debugdata files
	/*DWORD dwDebugData = 39647740;
	LPBYTE byDebugData = 0;
	{
		HANDLE hDebugFile = 0;
		LPCTSTR szDebugFileName = L"C:\\DebugDataAfterCrypt.txt_MaxScanner.exe_SD30.DB.txt";
		byDebugData = (LPBYTE)Allocate(dwDebugData);
		hDebugFile = CreateFile(szDebugFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		ReadFile(hDebugFile, byDebugData, dwDebugData, &dwBytesRead, 0);
		CloseHandle(hDebugFile);
		memcpy(m_pBuffer, byDebugData, dwBytesRead);
	}*/

	ptrData = m_pBuffer;
	ptrNode =(PSIZE_T)(m_pBuffer + dwFileSize);
	m_pRoot =(PNODEOPT)ptrNode;

	if(!ReadBBBSt(ptrData, ptrNode, m_pBuffer, m_nBufferSize))
	{
		goto ERROR_EXIT;
	}

	if(!Balance())
	{
		goto ERROR_EXIT;
	}

	if(m_bLoadReadOnly)
	{
		VChangeProtection(m_pBuffer, m_nBufferSize, TRUE);
	}

	m_bLoadedFromFile = true;
	return true;

ERROR_EXIT:

	if(hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
	}

	if(m_pBuffer)
	{
		VRelease(m_pBuffer);
		m_pBuffer = NULL;
	}

	m_pRoot = m_pTemp = NULL;
	m_bLoadedFromFile = false;
	m_nBufferSize = 0;
	//DeleteFile(szFullFileName);
	AddLogEntry(L"Error loading file: %s.File deleted", szFullFileName);
	return false;
}

/***************************************************************************************************
Function       : DumpBBBSt
Description    : dump bbbst object to file
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::DumpBBBSt(HANDLE hFile, PNODEOPT pNode, DWORD& dwNodesCount, DWORD cbKey1,
						DWORD cbKey2, DWORD cbKey3, DWORD cbData3)
{
	bool bWriteError = false;
	DWORD dwThisNodesCountOffset = 0, dwCurrentOffset = 0, dwBytesWritten = 0;
	DWORD dwThisNodesCount = 0, dwEmdbNodesCount = 0;
	CBBSt objBBSt(true, cbKey2, cbKey3, cbData3);
	CPtrStack objPtrStack;

	dwThisNodesCountOffset = SetFilePointer(hFile, 0, 0, FILE_CURRENT);
	if(INVALID_SET_FILE_POINTER == dwThisNodesCountOffset)
	{
		return false;
	}

	if(FALSE == WriteFile(hFile, &dwThisNodesCount, sizeof(dwThisNodesCount), &dwBytesWritten, 0))
	{
		return false;
	}

	m_pTemp = pNode;
	while(NULL != m_pTemp || !objPtrStack.IsEmpty())
	{
		if(m_pTemp)
		{
			objPtrStack.Push(m_pTemp);
			m_pTemp = m_pTemp->pLeft;
		}
		else
		{
			m_pTemp = (PNODEOPT)objPtrStack.Pop();

			dwEmdbNodesCount = 0;
			dwThisNodesCount++;
			dwNodesCount++;

			if(!WriteFile(hFile,(LPVOID)m_pTemp->nKey, cbKey1, &dwBytesWritten, 0))
			{
				bWriteError = true;
				break;
			}

			if(!objBBSt.DumpBBSt(hFile, (PNODEOPT)m_pTemp->nData, dwEmdbNodesCount, cbKey2, cbKey3, cbData3))
			{
				bWriteError = true;
				break;
			}

			dwNodesCount += dwEmdbNodesCount;
			m_pTemp = m_pTemp->pRight;
		}
	}

	if(bWriteError)
	{
		return false;
	}

	dwCurrentOffset = SetFilePointer(hFile, 0, 0, FILE_CURRENT);
	if(INVALID_SET_FILE_POINTER == dwCurrentOffset)
	{
		return false;
	}

	if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwThisNodesCountOffset, 0, FILE_BEGIN))
	{
		return false;
	}

	if(!WriteFile(hFile, &dwThisNodesCount, sizeof(dwThisNodesCount), &dwBytesWritten, 0))
	{
		return false;
	}

	if( INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwCurrentOffset, 0, FILE_BEGIN))
	{
		return false;
	}

	return true;
}

/***************************************************************************************************
Function       : Save
Description    : save to file
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBBBSt::Save(LPCTSTR szFileName, bool bEncryptContents)
{
	ULONG64 ulCount = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwBytesWritten = 0, dwNodesCount = 0;
	BYTE byHdrBfr[sizeof(HEADER_BBBST) + sizeof(HEADER_BBBST_DATA)] = {0};

	hFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS,
						FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		return false;
	}

	if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile, sizeof(byHdrBfr), 0, FILE_BEGIN))
	{
		goto ERROR_EXIT;;
	}

	if(!DumpBBBSt(hFile, m_pRoot, dwNodesCount, m_cbKey1, m_cbKey2, m_cbKey3, m_cbData3))
	{
		goto ERROR_EXIT;
	}

	ulCount = dwNodesCount;
	/*if(bEncryptContents && !CryptFileData(hFile, sizeof(byHdrBfr)))
	{
		goto ERROR_EXIT;
	}*/

	if(!CreateHeaderData(hFile, szFileName, HEADER_BBBST_DATA, sizeof(HEADER_BBBST_DATA), ulCount))
	{
		goto ERROR_EXIT;
	}

	/*
	//md5 checksum
	{
		ULONG64 ulMD5CRC = 0;
		BYTE byMD5Checksum[16] = {0};

		if(!MDFile(hFile, byMD5Checksum, sizeof(byMD5Checksum), sizeof(byHdrBfr)))
		{
			AddLogEntry(_T("Failed creating header checksum while saving: %s"), szFileName);
			goto ERROR_EXIT;
		}

		CreateCRC64Buffer(byMD5Checksum, sizeof(byMD5Checksum), ulMD5CRC);
		memcpy(HEADER_BBBST_DATA + 8, &ulMD5CRC, sizeof(ulMD5CRC));
	}
	*/

	memcpy(byHdrBfr, HEADER_BBBST, sizeof(HEADER_BBBST));
	memcpy(byHdrBfr + sizeof(HEADER_BBBST), HEADER_BBBST_DATA, sizeof(HEADER_BBBST_DATA));

	if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, 0, FILE_BEGIN))
	{
		goto ERROR_EXIT;
	}

	if(FALSE == WriteFile(hFile, byHdrBfr, sizeof(byHdrBfr), &dwBytesWritten, 0))
	{
		goto ERROR_EXIT;
	}

	CloseHandle(hFile);
	return true;

ERROR_EXIT:

	if(INVALID_HANDLE_VALUE != hFile)
	{
		CloseHandle(hFile);
	}

	DeleteFile(szFileName);
	AddLogEntry(L"Error saving file: %s.File deleted.", szFileName);
	return false;
}
