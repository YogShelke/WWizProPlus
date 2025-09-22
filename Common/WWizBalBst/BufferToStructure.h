/************************************************************************************************
*  Program Name		: BufferToStructure.h
*  Description		: class declaration for 1 level binary tree of buffer -> structure
*  Author Name		: Ram Shelke
*  Date Of Creation	: 13 Jul 2019
*  Version No		: 3.1.0.0
*************************************************************************************************/

#pragma once
#include "balbstopt.h"

class CBufferToStructure : public CBalBSTOpt
{

public:

	CBufferToStructure(bool bIsEmbedded, DWORD dwSizeOfKey, DWORD dwSizeOfData, int iNumberSize = 0);
	virtual ~CBufferToStructure();

	bool AppendItemAscOrder(LPVOID lpvKey, LPVOID lpvData);
	bool AppendItem(LPVOID lpvKey, LPVOID lpvData);
	bool DeleteItem(LPVOID lpvKey);
	bool SearchItem(LPVOID lpvKey, LPVOID& lpvData);
	bool UpdateItem(LPVOID lpvKey, LPVOID lpvData);

	bool GetKey(PVOID pVPtr, LPVOID& lpvKey);
	bool GetData(PVOID pVPtr, LPVOID& lpvData);

	bool AppendObject(CBalBSTOpt& objToAdd);
	bool DeleteObject(CBalBSTOpt& objToDel);
	bool CreateObject(CBufferToStructure& objNewObject);
	bool SearchObject(CBalBSTOpt& objToSearch, bool bAllPresent = true);

	bool ReadB2St(LPBYTE& ptrData, PSIZE_T& ptrNode, LPBYTE byBuffer, DWORD cbBuffer);
	bool DumpB2St(HANDLE hFile, PNODEOPT pNode, DWORD& dwNodesCount, DWORD dwKeyLen, DWORD dwDataLen);
	bool Load(LPCTSTR szFileName, bool bCheckVersion = true);
	bool Save(LPCTSTR szFileName, bool bEncryptContents = true);
	bool LoadByVer(LPCTSTR szFileName, bool bCheckVersion = true, LPCSTR szVersion = "00.00");
	bool SaveByVer(LPCTSTR szFileName, bool bEncryptContents = true, LPCSTR szVersion = "00.00");

private:

	DWORD	m_dwSizeOfKey;
	DWORD	m_dwSizeOfData;
	bool	m_bByte;
	bool	m_bWord;
	bool	m_bDword;
	bool	m_bQword;
	CHAR	m_szVersion[6];

	virtual COMPARE_RESULT Compare(SIZE_T nKey1, SIZE_T nKey2);
	virtual void FreeKey(SIZE_T nKey);
	virtual void FreeData(SIZE_T nData);
};

