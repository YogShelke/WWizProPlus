/************************************************************************************************
*  Program Name		: BBBSt.h
*  Description		: 
*  Author Name		: Ram Shelke
*  Date Of Creation	: 13 Jul 2019
*  Version No		: 3.1.0.0
*************************************************************************************************/

#pragma once
#include "balbstopt.h"
#include "BBSt.h"

int MDFile(HANDLE hFile, LPBYTE byMD5_16Bytes, SIZE_T cbMD5_16Bytes, DWORD dwOffset);
int MD5Buffer(LPBYTE byData, SIZE_T cbData, LPBYTE byMD5_16Bytes, SIZE_T cbMD5_16Bytes);

class CBBBSt : public CBalBSTOpt
{

public:

	CBBBSt(bool bIsEmbedded, DWORD cbKey1, DWORD cbKey2, DWORD cbKey3, DWORD cbData3, bool bLoadReadOnly = true);
	virtual ~CBBBSt();

	bool AppendItemAscOrder(LPVOID lpvKey, CBBSt& objBB2St);
	bool AppendItem(LPVOID lpvKey, CBBSt& objBB2St);
	bool DeleteItem(LPVOID lpvKey);
	bool SearchItem(LPVOID lpvKey, CBBSt& objBB2St);
	bool UpdateItem(LPVOID lpvKey, CBBSt& objBB2St);

	bool Balance();
	bool GetKey(PVOID pVPtr, LPVOID& lpvKey);
	bool GetData(PVOID pVPtr, CBBSt& objBB2St);

	bool AppendObject(CBalBSTOpt& objToAdd);
	bool DeleteObject(CBalBSTOpt& objToDel);
	bool CreateObject(CBBBSt& objNewCopy);
	bool SearchObject(CBalBSTOpt& objToSearch, bool bAllPresent = true);

	bool ReadBBBSt(LPBYTE& ptrData, PSIZE_T& ptrNode, LPBYTE byBuffer, DWORD cbBuffer);
	bool DumpBBBSt(HANDLE hFile, PNODEOPT pNode, DWORD& dwNodesCount, DWORD dwKeyLen1,
					DWORD dwKeyLen2, DWORD dwKeyLen3, DWORD dwDataLen3);
	bool Load(LPCTSTR szFileName, bool bCheckVersion = true);
	bool Save(LPCTSTR szFileName, bool bEncryptContents = true);

private:

	DWORD		m_cbKey1;
	DWORD		m_cbKey2;
	DWORD		m_cbKey3;
	DWORD		m_cbData3;
	DWORD		m_dwTotalObjectsCount;
	bool		m_bLoadReadOnly;

	virtual COMPARE_RESULT Compare(SIZE_T nKey1, SIZE_T nKey2);
	virtual void FreeKey(SIZE_T nKey);
	virtual void FreeData(SIZE_T nData);
};
