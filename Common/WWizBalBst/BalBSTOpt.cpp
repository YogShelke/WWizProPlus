/************************************************************************************************
*  Program Name		: BalBSTOpt.cpp
*  Description		: 
*  Author Name		: Ram Shelke
*  Date Of Creation	: 13 Jul 2019
*  Version No		: 3.1.0.0
*************************************************************************************************/

#include "stdafx.h"
#include "BalBSTOpt.h"

/***************************************************************************************************
Function       : CBalBSTOpt
Description    : constructor
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
CBalBSTOpt::CBalBSTOpt(bool bIsEmbedded): m_bIsEmbedded(bIsEmbedded)
{
	m_iThreadsCount = -1;
	DestroyData();
}

/***************************************************************************************************
Function       : ~CBalBSTOpt
Description    : destructor
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
CBalBSTOpt::~CBalBSTOpt()
{
}

/***************************************************************************************************
Function       : Lock
Description    : acquire exclusive access of object
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
void CBalBSTOpt::Lock()
{
	while(InterlockedIncrement(&m_iThreadsCount))
	{
		Sleep(1);
		InterlockedDecrement(&m_iThreadsCount);
	}
}

/***************************************************************************************************
Function       : Unlock
Description    : relinquish exclusive access of object
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
void CBalBSTOpt::Unlock()
{
	while(-1 != InterlockedDecrement(&m_iThreadsCount))
	{
		Sleep(1);
	}
}

/***************************************************************************************************
Function       : DestroyData
Description    : reset the class object to nulls
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
void CBalBSTOpt::DestroyData()
{
	m_pBuffer = NULL;
	m_dwCount = m_nBufferSize = 0;
	m_bLoadedFromFile = m_bTreeBalanced = m_bIsModified = false;
	m_pRoot = m_pTemp = m_pLastSearchResult = m_pLastSearchResultParent = m_pLinearTail = NULL;
}

/***************************************************************************************************
Function       : GetNode
Description    : get one node with intialised data
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
PNODEOPT CBalBSTOpt::GetNode(SIZE_T nKey, SIZE_T nData)
{
	PNODEOPT pTemp = (PNODEOPT)Allocate(sizeof NODEOPT);
	if(NULL == pTemp)
	{
		return NULL;
	}

	pTemp->nData = nData;
	pTemp->nKey = nKey;
	pTemp->pLeft = pTemp->pRight = NULL;
	return pTemp;
}

/***************************************************************************************************
Function       : AddNode
Description    : add a node to tree
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBalBSTOpt::AddNode(SIZE_T nKey, SIZE_T nData)
{
	PNODEOPT* ppNewNodeLocation = NULL;
	COMPARE_RESULT CompResult = EQUAL;

	ppNewNodeLocation = m_pRoot ? ppNewNodeLocation : &m_pRoot;
	for(m_pTemp = m_pRoot; m_pTemp && !ppNewNodeLocation ; )
	{
		CompResult = Compare(m_pTemp->nKey, nKey);
		if(CompResult == LARGE)
		{
			if(m_pTemp->pLeft)
			{
				m_pTemp = m_pTemp->pLeft;
			}
			else
			{
				ppNewNodeLocation = &m_pTemp->pLeft;
			}
		}
		else if(CompResult == SMALL)
		{
			if(m_pTemp->pRight)
			{
				m_pTemp = m_pTemp->pRight;
			}
			else
			{
				ppNewNodeLocation = &m_pTemp->pRight;
			}
		}
		else
		{
			m_pTemp = NULL;
		}
	}

	if(NULL == ppNewNodeLocation)
	{
		return false;
	}

	*ppNewNodeLocation = GetNode(nKey, nData);
	if(NULL == *ppNewNodeLocation)
	{
		return false;
	}

	m_dwCount++;
	m_bIsModified = true;
	m_bTreeBalanced = false;
	return true;
}

/***************************************************************************************************
Function       : AddNodeAscOrder
Description    : add a node in ascending order
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBalBSTOpt::AddNodeAscOrder(SIZE_T nKey, SIZE_T nData)
{
	PNODEOPT pNewNode = NULL;

	pNewNode = GetNode(nKey, nData);
	if(NULL == pNewNode)
	{
		return false;
	}

	if(NULL == m_pRoot)
	{
		m_pRoot = pNewNode;
		m_pLinearTail = pNewNode;
	}
	else
	{
		m_pLinearTail->pRight = pNewNode;
		m_pLinearTail = m_pLinearTail->pRight;
	}

	m_dwCount++;
	m_bIsModified = true;
	m_bTreeBalanced = false;
	return true;
}

/***************************************************************************************************
Function       : DeleteNode
Description    : delete a node and restructure the tree
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBalBSTOpt::DeleteNode(SIZE_T nKey)
{
	SIZE_T nData = 0;
	NODEOPT PseudoRoot ={0};
	PNODEOPT pNodeToDelete = 0, pNodeToDeleteParent = 0, pNodeToReplaceWith = 0, pNodeToReplaceWithParent = 0;

	if((NULL == m_pLastSearchResult) ||(m_pLastSearchResult->nKey != nKey))
	{
		if(!FindNode(nKey, nData))
		{
			return false;
		}
	}

	pNodeToDelete = m_pLastSearchResult;
	pNodeToDeleteParent = m_pLastSearchResultParent;
	PseudoRoot.pRight = m_pRoot;

	if(pNodeToDelete == m_pRoot)
	{
		pNodeToDeleteParent = &PseudoRoot;
	}

	if(pNodeToDelete->pLeft && pNodeToDelete->pRight)
	{
		pNodeToReplaceWithParent = pNodeToDelete;
		pNodeToReplaceWith = pNodeToDelete->pLeft;
		while(pNodeToReplaceWith->pRight)
		{
			pNodeToReplaceWithParent = pNodeToReplaceWith;
			pNodeToReplaceWith = pNodeToReplaceWith->pRight;
		}

		if(pNodeToReplaceWithParent->pLeft == pNodeToReplaceWith)
		{
			pNodeToReplaceWithParent->pLeft = pNodeToReplaceWith->pLeft;
		}
		else if(pNodeToReplaceWithParent->pRight == pNodeToReplaceWith)
		{
			pNodeToReplaceWithParent->pRight = pNodeToReplaceWith->pLeft;
		}

		pNodeToReplaceWith->pLeft = pNodeToDelete->pLeft;
		pNodeToReplaceWith->pRight = pNodeToDelete->pRight;

		if(pNodeToDeleteParent->pLeft == pNodeToDelete)
		{
			pNodeToDeleteParent->pLeft = pNodeToReplaceWith;
		}
		else if(pNodeToDeleteParent->pRight == pNodeToDelete)
		{
			pNodeToDeleteParent->pRight = pNodeToReplaceWith;
		}
	}
	else
	{
		m_pTemp = pNodeToDelete->pLeft?pNodeToDelete->pLeft:pNodeToDelete->pRight?pNodeToDelete->pRight:NULL;
		if(pNodeToDeleteParent->pLeft == pNodeToDelete)
		{
			pNodeToDeleteParent->pLeft = m_pTemp;
		}
		else
		{
			pNodeToDeleteParent->pRight = m_pTemp;
		}
	}

	m_pRoot = PseudoRoot.pRight;
	FreeData(pNodeToDelete->nData);
	FreeKey(pNodeToDelete->nKey);

	if(((LPBYTE)pNodeToDelete < m_pBuffer) ||((LPBYTE)pNodeToDelete >= m_pBuffer + m_nBufferSize))
	{
		Release((LPVOID&)pNodeToDelete);
	}

	if(m_dwCount)
	{
		m_dwCount--;
	}

	m_bIsModified = true;
	m_bTreeBalanced = false;
	m_pLastSearchResult = NULL;
	m_pLastSearchResultParent = NULL;
	return (true);
}

/***************************************************************************************************
Function       : RemoveAll
Description    : this frees all the memory and sets the tree object to null
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBalBSTOpt::RemoveAll()
{
	PNODEOPT pHold = NULL;
	CPtrStack objStack;

	if(m_bIsEmbedded)
	{
		DestroyData();
		return (true);
	}

	m_pTemp = m_pRoot;
	while(m_bIsModified && (m_pTemp || !objStack.IsEmpty()))
	{
		if(m_pTemp->pLeft)
		{
			objStack.Push(m_pTemp);
			m_pTemp = m_pTemp->pLeft;
		}
		else if(m_pTemp->pRight)
		{
			objStack.Push(m_pTemp);
			m_pTemp = m_pTemp->pRight;
		}
		else
		{
			pHold = (PNODEOPT)objStack.Pop();
			if(pHold)
			{
				if(pHold->pLeft == m_pTemp)
				{
					pHold->pLeft = NULL;
				}
				else if(pHold->pRight == m_pTemp)
				{
					pHold->pRight = NULL;
				}
			}

			FreeKey(m_pTemp->nKey);
			FreeData(m_pTemp->nData);
			if(((LPBYTE)m_pTemp < m_pBuffer) ||((LPBYTE)m_pTemp >= m_pBuffer + m_nBufferSize))
			{
				Release((LPVOID&)m_pTemp);
			}

			m_pTemp = pHold;
		}
	}

	if(m_bLoadedFromFile)
	{
		VRelease(m_pBuffer);
		m_pBuffer = NULL;
	}

	DestroyData();
	return (true);
}

/***************************************************************************************************
Function       : GetCount
Description    : traverse and count the nodes in tree
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
DWORD CBalBSTOpt::GetCount()
{
	CPtrStack objPtrStack;

	/*if(m_dwCount)
	{
		return m_dwCount;
	}*/

	m_dwCount = 0;
	m_pTemp = m_pRoot;
	while(NULL != m_pTemp || !objPtrStack.IsEmpty())
	{
		if(m_pTemp)
		{
			objPtrStack.Push(m_pTemp);
			m_pTemp = m_pTemp->pLeft;
		}
		else
		{
			m_dwCount++;
			m_pTemp = (PNODEOPT)objPtrStack.Pop();
			m_pTemp = m_pTemp->pRight;
		}
	}

	return (m_dwCount);
}

/***************************************************************************************************
Function       : GetFirst
Description    : get the root pointer
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
LPVOID CBalBSTOpt::GetFirst()
{
	return (m_pRoot);
}

/***************************************************************************************************
Function       : GetNext
Description    : get next preorder node, used in tree traversal
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
LPVOID CBalBSTOpt::GetNext(LPVOID pPrev)
{
	PNODEOPT pNode = (PNODEOPT)pPrev;

	if(pNode == m_pRoot)
	{
		m_objStack.RemoveAll();
	}
	else
	{
		pNode = pNode->pRight;
	}

	while(NULL != pNode || !m_objStack.IsEmpty())
	{
		if(pNode)
		{
			m_objStack.Push(pNode);
			pNode = pNode->pLeft;
		}
		else
		{
			pNode = (PNODEOPT)m_objStack.Pop();

			if(pNode != m_pRoot)
			{
				break;
			}

			pNode = pNode->pRight;
		}
	}

	return pNode;
}

/***************************************************************************************************
Function       : FullSize
Description    : calculate the size to compress when converting to tree
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
int CBalBSTOpt::FullSize(int size)
{
	int Rtn = 1;

	while(Rtn <= size)
	{
		Rtn = Rtn + Rtn + 1;
	}

	return Rtn / 2;
}

/***************************************************************************************************
Function       : Compress
Description    : rotate and balance the tree
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
void CBalBSTOpt::Compress(PNODEOPT pRoot, int count)
{
	PNODEOPT scanner = pRoot;

	for(int j = 0; j < count; j++)
	{
		PNODEOPT child = scanner->pRight;
		scanner->pRight = child->pRight;
		scanner = scanner->pRight;
		child->pRight = scanner->pLeft;
		scanner->pLeft = child;
	}
}

/***************************************************************************************************
Function       : ConvertVineToTree
Description    : make the tree of vine, used in balancing
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
void CBalBSTOpt::ConvertVineToTree(PNODEOPT pRoot, int size)
{
	int full_count = FullSize(size);
	Compress(pRoot, size - full_count);
	for(size = full_count; size > 1; size /= 2)
	{
		Compress(pRoot, size / 2);
	}
}

/***************************************************************************************************
Function       : ConvertTreeToVine
Description    : make the vine of tree, used in balancing
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
void CBalBSTOpt::ConvertTreeToVine(PNODEOPT pRoot, int &size)
{
	PNODEOPT vineTail = 0;
	PNODEOPT remainder = 0;
	PNODEOPT tempPtr = 0;

	vineTail = pRoot;
	remainder = vineTail->pRight;
	size = 0;

	while(remainder != NULL)
	{
		if(remainder->pLeft == NULL)
		{
			vineTail = remainder;
			remainder = remainder->pRight;
			size++;
		}
		else
		{
			tempPtr = remainder->pLeft;
			remainder->pLeft = tempPtr->pRight;
			tempPtr->pRight = remainder;
			remainder = tempPtr;
			vineTail->pRight = tempPtr;
		}
	}
}

/***************************************************************************************************
Function       : Balance
Description    : balance the tree
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBalBSTOpt::Balance()
{
	int iCount = 0;
	NODEOPT Pseudo_Root = {0};

	Pseudo_Root.pRight = m_pRoot;
	ConvertTreeToVine(&Pseudo_Root, iCount);
	ConvertVineToTree(&Pseudo_Root, iCount);
	m_pRoot = Pseudo_Root.pRight;

	m_bTreeBalanced = true;
	return true;
}

/***************************************************************************************************
Function       : FindNode
Description    : search for a node in tree by key
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBalBSTOpt::FindNode(SIZE_T nKey, SIZE_T& nData)
{
	bool bFound = false;
	COMPARE_RESULT CompResult = EQUAL;

	m_pLastSearchResultParent = NULL;
	m_pLastSearchResult = NULL;
	m_pTemp = m_pRoot;
	while(m_pTemp)
	{
		CompResult = Compare(nKey, m_pTemp->nKey);
		if(SMALL == CompResult)
		{
			m_pLastSearchResultParent = m_pTemp;
			m_pTemp = m_pTemp->pLeft;
		}
		else if(LARGE == CompResult)
		{
			m_pLastSearchResultParent = m_pTemp;
			m_pTemp = m_pTemp->pRight;
		}
		else
		{
			m_pLastSearchResult = m_pTemp;
			nData = m_pTemp->nData;
			bFound = true;
			break;
		}
	}

	m_pLastSearchResultParent = m_pLastSearchResult ? m_pLastSearchResultParent : NULL;
	return (bFound);
}

/***************************************************************************************************
Function       : GetDataPtr
Description    : get pointer to internal tree root
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
PNODEOPT CBalBSTOpt::GetDataPtr()
{
	return (m_pRoot);
}

/***************************************************************************************************
Function       : SetDataPtr
Description    : set the data pointers and values
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBalBSTOpt::SetDataPtr(PNODEOPT pNode, LPBYTE pbyBuffer, DWORD nBufferSize)
{
	m_pRoot = pNode;
	m_pBuffer = pbyBuffer;
	m_nBufferSize = nBufferSize;
	m_bIsModified = true;
	return (true);
}

/***************************************************************************************************
Function       : IsModified
Description    : return true if object modified else false
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBalBSTOpt::IsModified()
{
	return m_bIsModified;
}

/***************************************************************************************************
Function       : SetModified
Description    : sets the object modified flag true
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
void CBalBSTOpt::SetModified(bool bModified)
{
	m_bIsModified = bModified;
}

/***************************************************************************************************
Function       : SearchObject
Description    : when 'bAllPresent' is true the function searches to see that all entries
				 in 'objToSearch' are present in 'this' object. when 'bAllPresent' is false
				 the function searches to see that all entries in 'objToSearch' are absent
				 in 'this' object.
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
bool CBalBSTOpt::SearchObject(CBalBSTOpt& objToSearch, bool bAllPresent)
{
	return true;
}

/***************************************************************************************************
Function       : GetHighest
Description    : return highest node
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
LPVOID CBalBSTOpt::GetHighest()
{
	PNODEOPT pNode = m_pRoot;

	m_objStack.RemoveAll();
	while(pNode || !m_objStack.IsEmpty())
	{
		if(pNode)
		{
			m_objStack.Push(pNode);
			pNode = pNode->pRight;
		}
		else
		{
			pNode = (PNODEOPT)m_objStack.Pop();
			break;
		}
	}

	return pNode;
}

/***************************************************************************************************
Function       : GetHighestNext
Description    : return next highest node
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
LPVOID CBalBSTOpt::GetHighestNext(LPVOID pContext)
{
	PNODEOPT pNode = (PNODEOPT)pContext;

	pNode = pNode->pLeft;
	while(pNode || !m_objStack.IsEmpty())
	{
		if(pNode)
		{
			m_objStack.Push(pNode);
			pNode = pNode->pRight;
		}
		else
		{
			pNode = (PNODEOPT)m_objStack.Pop();
			break;
		}
	}

	return pNode;
}

/***************************************************************************************************
Function       : GetLowest
Description    : return lowest node
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
LPVOID CBalBSTOpt::GetLowest()
{
	PNODEOPT pNode = m_pRoot;

	m_objStack.RemoveAll();
	while(pNode || !m_objStack.IsEmpty())
	{
		if(pNode)
		{
			m_objStack.Push(pNode);
			pNode = pNode->pLeft;
		}
		else
		{
			pNode = (PNODEOPT)m_objStack.Pop();
			break;
		}
	}

	return pNode;
}

/***************************************************************************************************
Function       : GetLowestNext
Description    : return next lowest node
Author         : Ram Shelke
Date           : 15-JUL-2019
***************************************************************************************************/
LPVOID CBalBSTOpt::GetLowestNext(LPVOID pContext)
{
	PNODEOPT pNode = (PNODEOPT)pContext;

	pNode = pNode->pRight;
	while(pNode || !m_objStack.IsEmpty())
	{
		if(pNode)
		{
			m_objStack.Push(pNode);
			pNode = pNode->pLeft;
		}
		else
		{
			pNode = (PNODEOPT)m_objStack.Pop();
			break;
		}
	}

	return pNode;
}
