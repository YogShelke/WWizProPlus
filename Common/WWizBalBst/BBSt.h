/************************************************************************************************
*  Program Name		: BBSt.h
*  Description		: 
*  Author Name		: Ram Shelke
*  Date Of Creation	: 13 Jul 2019
*  Version No		: 3.1.0.0
*************************************************************************************************/

#pragma once
#include "balbstopt.h"
#include "BufferToStructure.h"

class CBBSt : public CBalBSTOpt
{

public:

	CBBSt(bool bIsEmbedded, DWORD dwSizeOfKey, DWORD dwSizeOfEmbeddedKey, DWORD dwSizeOfEmbeddedData, bool bLoadReadOnly = true);
	virtual ~CBBSt();

	bool AppendItemAscOrder(LPVOID lpvKey, CBufferToStructure& objBufToStruct);
	bool AppendItem(LPVOID lpvKey, CBufferToStructure& objBufToStruct);
	bool DeleteItem(LPVOID lpvKey);
	bool SearchItem(LPVOID lpvKey, CBufferToStructure& objBufToStruct);
	bool UpdateItem(LPVOID lpvKey, CBufferToStructure& objBufToStruct);

	bool Balance();
	bool GetKey(PVOID pVPtr, LPVOID& lpvKey);
	bool GetData(PVOID pVPtr, CBufferToStructure& objBufToStruct);

	bool AppendObject(CBalBSTOpt& objToAdd);
	bool DeleteObject(CBalBSTOpt& objToDel);
	bool CreateObject(CBBSt& objNewCopy);
	bool SearchObject(CBalBSTOpt& objToSearch, bool bAllPresent = true);

	bool ReadBBSt(LPBYTE& ptrData, PSIZE_T& ptrNode, LPBYTE byBuffer, DWORD cbBuffer);
	bool DumpBBSt(HANDLE hFile, PNODEOPT pNode, DWORD& dwNodesCount, DWORD dwKeyLen,
					DWORD dwEmbdKeyLen, DWORD dwEmbdDataLen);
	bool Load(LPCTSTR szFileName, bool bCheckVersion = true);
	bool Save(LPCTSTR szFileName, bool bEncryptContents = true);

private:

	DWORD	m_dwSizeOfKey;
	DWORD	m_dwSizeOfEmbeddedKey;
	DWORD	m_dwSizeOfEmbeddedData;
	bool	m_bLoadReadOnly;

	virtual COMPARE_RESULT Compare(SIZE_T nKey1, SIZE_T nKey2);
	virtual void FreeKey(SIZE_T nKey);
	virtual void FreeData(SIZE_T nData);
};
