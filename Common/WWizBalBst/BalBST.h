/************************************************************************************************
*  Program Name		: BalBST.h
*  Description		: class declaration for 1 level binary tree of buffer -> structure
*  Author Name		: Ram Shelke
*  Date Of Creation	: 13 Jul 2019
*  Version No		: 3.1.0.0
*************************************************************************************************/

#pragma once

/*						TODO						*\
1.	Include encrypted flag in header
2.	Optimize node for non buffer classes
3.	Optimize buffer class for memory
4.	Sanity check for loading file for buffer classes
5.	Change class names to be more descriptive
6.	Change enumeration method to make it simple
7.	Optimize saving to file for all classes
8.	Save ClassName or ClassName ID in file
9.	Optimize balance for vine trees
10.	Avoid balance before saving
\*						TODO						*/

#define MKWORD(h,l)		((((unsigned short)(h))<<8)|(l))
#define MKDWORD(h,l)	((((unsigned long)(h))<<16)|(l))
#define MKQWORD(h,l)	((((unsigned __int64)(h))<<32)|(l))
#define HIDWORD(qw)		((unsigned int)(qw>>32))
#define LODWORD(qw)		((unsigned int)qw)

#define REV_BYT_ODR_WORD(w)			(MKWORD(LOBYTE(w),HIBYTE(w)))
#define REV_BYT_ODR_DWORD(dw)		(MKDWORD(REV_BYT_ODR_WORD(LOWORD(dw)),REV_BYT_ODR_WORD(HIWORD(dw))))
#define REV_BYT_ODR_QWORD(qw)		(MKQWORD(REV_BYT_ODR_DWORD(LODWORD(qw)),REV_BYT_ODR_DWORD(HIDWORD(qw))))
typedef enum{EQUAL, SMALL, LARGE}COMPARE_RESULT;

LPVOID Allocate(DWORD dwSize);
void Release(LPVOID& pVPtr);
LPBYTE DuplicateBuffer(LPBYTE pbyBuffer, DWORD nBufferSize);
LPTSTR DuplicateString(LPCTSTR szString);
LPSTR DuplicateStringA(LPCSTR szString);
bool MakeFullFilePath(LPCTSTR szFileName, LPTSTR szFullFileName, DWORD dwFullFileNameSize);
void CryptBlock(DWORD * Data, DWORD dwDataSize);
bool CryptBuffer(LPBYTE pbyBuffer, DWORD dwBufferSize);
bool CryptFileData(HANDLE hFile, DWORD dwStartOffset = 0);
bool CopyAndCryptFile(LPCTSTR szOrgFile, LPCTSTR szNewFile, DWORD dwMaxMemLimit, DWORD dwStartOffset);
bool CryptFile(LPCTSTR szFileName, const short iMAX_HEADER_SIZE = 48);
bool CreateCRC64(LPCTSTR szString, ULONG64& ul64CRC);
bool CreateCRC64Buffer(LPBYTE byBuffer, SIZE_T cbBuffer, ULONG64& ul64CRC);
bool CreateCRC32(LPCTSTR szString, DWORD& dwCRC32);
bool CreateCRC32Buffer(LPBYTE byBuffer, size_t cbBuffer, DWORD& dwCRC32);
bool CreateHeaderData(HANDLE hFile, LPCTSTR szFullFileName, LPBYTE Buffer, DWORD cbBuffer, ULONG64 ulCount = 0);
void DumpNumber(ULONG64 ulNumber, LPCTSTR szFilePath, LPCTSTR szAppendName);
void DumpBuffer(LPBYTE byBuffer, SIZE_T cbBuffer, LPCTSTR szFilePath, LPCTSTR szAppendName);
LPVOID VAllocate(SIZE_T nRegionSize);
LPVOID VRelease(LPVOID lpvMemBase);
BOOL VChangeProtection(LPVOID lpvMemBase, SIZE_T nRegionSize, BOOL bEnable);

#define CHECK_AND_MAKE_POINTER(pointer,base,start,size)										\
do																							\
{																							\
	if ( (*(pointer)) )																		\
	{																						\
		(*(pointer)) += base ;																\
		if ( ((LPBYTE)(*(pointer))) < (start) || ((LPBYTE)(*(pointer))) >= ((start)+(size)))\
			goto ERROR_EXIT ;																\
	}																						\
	(pointer)++ ;																			\
}while(0);																					\

#pragma pack(1)
typedef struct _tagNode
{
	ULONG64				dwKey;
	ULONG64				dwData;
	ULONG64				dwHold;
	union
	{
		struct _tagNode*	pLeft;
		ULONG64				ulLeftPad;
	};

	union
	{
		struct _tagNode*	pRight;
		ULONG64				ulRightPad;
	};

	union
	{
		struct _tagNode*	pParent;
		ULONG64				ulParentPad;
	};

} NODE, *PNODE;
#pragma pack()

#define SIZE_OF_NODE					sizeof(NODE)
#define SIZE_OF_ONE_NODE_ELEMENT		sizeof(ULONG64)
#define NUMBER_OF_NODE_ELEMENTS			((SIZE_OF_NODE)/(SIZE_OF_ONE_NODE_ELEMENT))

class CBalBST
{

public:

	CBalBST(bool bIsEmbedded);
	virtual ~CBalBST();
	PNODE GetDataPtr();
	bool SetDataPtr(PNODE pNode, LPBYTE pbyBuffer, DWORD nBufferSize);
	LPVOID GetFirst();
	LPVOID GetNext(LPVOID pPrev);
	DWORD GetCount();
	LPVOID GetLargestKey();

	virtual bool Balance();
	virtual bool RemoveAll();
	virtual bool Load(LPCTSTR szFileName, bool bCheckVersion = true)= 0;
	virtual bool Save(LPCTSTR szFileName, bool bEncryptContents = true)= 0;
	virtual bool AppendObject(CBalBST& objToAdd)= 0;
	virtual bool DeleteObject(CBalBST& objToDel)= 0;
	virtual bool SearchObject(CBalBST& objToSearch, bool bAllPresent = true);
	bool IsModified();
	void SetModified(bool bModified = true);

protected:

	NODE*				m_pRoot;
	NODE*				m_pTemp;
	bool				m_bLoadedFromFile;
	BYTE*				m_pBuffer;
	DWORD				m_nBufferSize;
	NODE*				m_pLastSearchResult;
	bool				m_bLoadError;
	bool				m_bSaveError;
	bool				m_bIsModified;

	bool AddNode(ULONG64 dwKey, ULONG64 dwData);
	bool AddNodeAscOrder(ULONG64 dwKey, ULONG64 dwData);
	bool DeleteNode(ULONG64 dwKey);
	bool FindNode(ULONG64 dwKey, ULONG64& dwData);
	inline void DestroyData();

private:

	NODE*				m_pLinearTail;
	bool				m_bTreeBalanced;
	bool				m_bIsEmbedded;
	DWORD				m_dwCount;

	virtual COMPARE_RESULT Compare(ULONG64 dwKey1, ULONG64 dwKey2)= 0;
	virtual void FreeKey(ULONG64 dwKey)= 0;
	virtual void FreeData(ULONG64 dwData)= 0;

	NODE* GetNode(ULONG64 dwKey, ULONG64 dwData);
	void ConvertTreeToVine(NODE* pRoot, int &size);
	void ConvertVineToTree(NODE* pRoot, int size);
	void Compress(NODE* pRoot, int count);
	int FullSize(int size);
	void AdjustParents(NODE * pRoot);
};
