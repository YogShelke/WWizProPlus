/************************************************************************************************
*  Program Name		: BalBSTOpt.cpp
*  Description		:
*  Author Name		: Ram Shelke
*  Date Of Creation	: 13 Jul 2019
*  Version No		: 3.1.0.0
*************************************************************************************************/

#pragma once
#include "BalBST.h"
#include "PtrStack.h"

#define SFPTR(FH,DL,DH,MM,RV)	\
if(INVALID_SET_FILE_POINTER == ((RV) = (SetFilePointer((FH), (DL), (DH), (MM)))))						\
{																										\
	goto ERROR_EXIT;																					\
}																										\

#define WFILE(HF, DATA, SIZE, WRITTEN, OVERLAPPED) \
if(FALSE == WriteFile((HF), (DATA), (SIZE), (WRITTEN), (OVERLAPPED)))									\
{																										\
	goto ERROR_EXIT;																					\
}																										\

#define CHECK_AND_MAKE_POINTER2(pointer,value,start,size,allownull)										\
do																										\
{																										\
	(*(pointer)) = (SIZE_T)(value);																		\
	if((0 == (value)))																					\
	{																									\
		if(0 == (allownull))																			\
			goto ERROR_EXIT;																			\
	}																									\
	else																								\
	{																									\
		if((((LPBYTE)(*(pointer))) < (start)) || (((LPBYTE)(*(pointer))) >= ((start)+(size))))			\
			goto ERROR_EXIT;																			\
	}																									\
}while(0);																								\

#define VALIDATE_POINTER(pointer,base,size)																\
{																										\
	if(((SIZE_T)(pointer)) < ((SIZE_T)(base)))															\
		goto ERROR_EXIT;																				\
	if(((SIZE_T)(pointer)) >= (((SIZE_T)(base)) + ((SIZE_T)(size))))									\
		goto ERROR_EXIT;																				\
}																										\

#pragma pack(1)
typedef struct _tagNodeOpt
{
	SIZE_T				nKey;
	SIZE_T				nData;
	struct _tagNodeOpt*	pLeft;
	struct _tagNodeOpt*	pRight;
} NODEOPT, *PNODEOPT;
#pragma pack()

#define SIZE_OF_NODEOPT					sizeof(NODEOPT)
#define SIZE_OF_ONE_NODEOPT_ELEMENT		sizeof(SIZE_T)
#define NUMBER_OF_NODEOPT_ELEMENTS		((SIZE_OF_NODEOPT)/(SIZE_OF_ONE_NODEOPT_ELEMENT))

class CBalBSTOpt
{

public:

	CBalBSTOpt(bool bIsEmbedded);
	virtual ~CBalBSTOpt();
	PNODEOPT GetDataPtr();
	bool SetDataPtr(PNODEOPT pNode, LPBYTE pbyBuffer, DWORD nBufferSize);
	LPVOID GetFirst();
	LPVOID GetNext(LPVOID pContext);
	DWORD GetCount();
	LPVOID GetHighest();
	LPVOID GetLowest();
	LPVOID GetHighestNext(LPVOID pContext);
	LPVOID GetLowestNext(LPVOID pContext);

	virtual bool Balance();
	virtual bool RemoveAll();
	virtual bool Load(LPCTSTR szFileName, bool bCheckVersion = true)= 0;
	virtual bool Save(LPCTSTR szFileName, bool bEncryptContents = true)= 0;
	virtual bool AppendObject(CBalBSTOpt& objToAdd)= 0;
	virtual bool DeleteObject(CBalBSTOpt& objToDel)= 0;
	virtual bool SearchObject(CBalBSTOpt& objToSearch, bool bAllPresent = true);
	bool IsModified();
	void SetModified(bool bModified = true);

protected:

	PNODEOPT			m_pRoot;
	PNODEOPT			m_pTemp;
	bool				m_bLoadedFromFile;
	LPBYTE				m_pBuffer;
	DWORD				m_nBufferSize;
	PNODEOPT			m_pLastSearchResult;
	PNODEOPT			m_pLastSearchResultParent;
	bool				m_bLoadError;
	bool				m_bSaveError;
	bool				m_bIsModified;
	CPtrStack			m_objStack;
	long				m_iThreadsCount;

	void Lock();
	void Unlock();
	bool AddNode(SIZE_T nKey, SIZE_T nData);
	bool AddNodeAscOrder(SIZE_T nKey, SIZE_T nData);
	bool DeleteNode(SIZE_T nKey);
	bool FindNode(SIZE_T nKey, SIZE_T& nData);
	inline void DestroyData();

private:

	PNODEOPT			m_pLinearTail;
	bool				m_bTreeBalanced;
	bool				m_bIsEmbedded;
	DWORD				m_dwCount;

	virtual COMPARE_RESULT Compare(SIZE_T nKey1, SIZE_T nKey2)= 0;
	virtual void FreeKey(SIZE_T nKey)= 0;
	virtual void FreeData(SIZE_T nData)= 0;

	PNODEOPT GetNode(SIZE_T nKey, SIZE_T nData);
	void ConvertTreeToVine(PNODEOPT pRoot, int &size);
	void ConvertVineToTree(PNODEOPT pRoot, int size);
	void Compress(PNODEOPT pRoot, int count);
	int FullSize(int size);
};
