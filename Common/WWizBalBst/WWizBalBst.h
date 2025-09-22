/**********************************************************************************************************
	Program Name			: CWWizBalBst
	Description				: This class is written to load object in binary search tree(BST).
							  BST helps to search the node faster.
	Author Name				: Ramkrushna Shelke
	Date Of Creation		: 04/06/2016
	Version No				: 1.14.1.10
	Special Logic Used		:
	Modification Log		:
	1. Name					: Description
***********************************************************************************************************/
#pragma once
#include "BalBstError.h"
#include "BalBstConst.h"
#include <Imagehlp.h>

BALBSTCLSTMPLT
class CNode
{
public:
	CNode(CNodeT* pLeftChild, CNodeT* pRightChild, const T* const pSmallKey);
	CNode(void);
	~CNode(void);
	CNode* getChild(eTraverseDirection eIndex) const;
	void setChild(eTraverseDirection eIndex, CNodeT* pChild = 0);
	void setParent(CNodeT* pNode);
	CNodeT* getParent(void) const;
	const T* const getSmallKey(void) const;
	void setSmallKey(const T* const tSmallKey);
	const T* const getBigKey(void) const;
	void setBigKey(const T* const pBigKey);
	eNodeSize getSize(void) const;
	bool isLeaf(void) const;
private:
	T m_tSmallKey;
	T m_tBigKey;
	CNodeT* m_pParent;
	bool m_bBigKeyIsSet;
	bool m_bSmallKeyIsSet;
	CNodeT* m_pChildren[MAX_CHILDREN];
};

BALBSTCLSTMPLT
class CWWizBalBst
{
public:
	CWWizBalBst(void);
	CWWizBalBst(DWORD);
	~CWWizBalBst(void);
	int insert(const T* const pKey);
	const T* const find(const T* const);
	int deleteItem(const T* const pKey);
	int print(eTreeTraversal eTraversalMethod, int *pNrOfItemsPrinted) const;
	int removeAll(void);
private:
	int add(CNodeT* const pNode, const T* const pKey);
	int split(T* pKey, 	CNodeT* const pNode2Split,	CNodeT** const ppAddedNode);
	int search(const T* const,	CNodeT** ppNodeFound,	bool *bFound) const;
	CNodeT* getMinNode(CNodeT* pNode) const;
	CNodeT* getMaxNode(CNodeT* pNode) const;
	CNodeT* m_pRoot;
	CNodeT* getNextInorderKey(CNodeT* pNode, T* pKey) const;
	CNodeT* getNextPreorderKey(CNodeT* pNode, T* pKey) const;
	CNodeT* getNextPostorderKey(CNodeT* pNode, T* pKey) const;
	int removeFromLeaf(CNodeT* pNode, const T* const pKey);
	int localRotation(CNodeT* pNode);
	int globalRotation(CNodeT* pNode);
	int localNodeMerger(CNodeT* pNode);
	int globalNodeMerger(CNodeT* pNode, CNodeT** ppNextNode2Fix);
	int swapWithInOrderSuccessor(CNodeT* pNode, const T* const pKey, CNodeT** ppLeafNode);
	int TCompare(const T* const pObject1, const T* const pObject2) const;
	DWORD FileIntegrityCheck(LPTSTR lpszFilePath);
	int FileWrite(HANDLE hHandle, eTreeTraversal eTraversalMethod, int *pNrOfItemsPrinted) const;
	DWORD AddCheckSum(LPCTSTR lpFileName, DWORD dwByteNeedtoAdd);
private:
	DWORD	m_dwMaxCount;
	DWORD	m_dwNodeCount;
public:
	bool Load(LPTSTR lpszFilePath);
	bool Save(LPTSTR lpszFilePath);
};


/***************************************************************************************************
*  Function Name  : CNode Const'r
*  Description    : Constructor initializes variables.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
****************************************************************************************************/
BALBSTCLSTMPLT
CNodeT::CNode(void)
{
	m_pChildren[leftChild] = 0;
	m_pChildren[middleChild] = 0;
	m_pChildren[rightChild] = 0;
	m_pParent = 0;
	m_bSmallKeyIsSet = false;
	m_bBigKeyIsSet = false;
}

/***************************************************************************************************
*  Function Name  : ~CNode Destructor
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
****************************************************************************************************/
BALBSTCLSTMPLT
CNodeT::~CNode(void)
{
}

/***************************************************************************************************
*  Function Name  : CNode
*  Description    : Release resources used by this DLL
*					Overload constructor taking three arguments.
*					Left and right child are set for this node. Small key is also set.
*					@param[in] pLeftChild: address to node which will be set as left child of this node.
*					@param[in] pRightChild: address to node which will be set as right child of this node.
*					@param[in] pSmallKey: address to small key.
*					@return none
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
****************************************************************************************************/
BALBSTCLSTMPLT
CNodeT::CNode(CNodeT* pLeftChild, CNodeT* pRightChild, const T* const pSmallKey)
{
	__try
	{
		m_pChildren[leftChild] = pLeftChild;
		m_pChildren[middleChild] = 0;
		m_pChildren[rightChild] = pRightChild;
		m_bBigKeyIsSet = false;
		m_pParent = 0;

		if (pLeftChild != 0)
		{
			pLeftChild->setParent(this);
		}

		if (pRightChild != 0)
		{
			pRightChild->setParent(this);
		}

		if (pSmallKey != 0)
		{
			m_tSmallKey = *pSmallKey;
			m_bSmallKeyIsSet = true;
		}
		else
		{
			m_bSmallKeyIsSet = false;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CNodeT::CNode", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : getChild
*  Description    :	Member function taking one argument and returning node address.
*					Function returns the node address to the requested child.
*					@param[in] eIndex: child index { leftChild, middleChild, rightChild }.
*					@return Node address
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
****************************************************************************************************/
BALBSTCLSTMPLT
inline CNodeT* CNodeT::getChild(eTraverseDirection eIndex) const
{
	return m_pChildren[eIndex];
}

/***************************************************************************************************
*  Function Name  : setChild
*  Description    :	Member function taking two arguments and returning none.
*					Function sets child node for a given node.
*					@param[in] eIndex: child index {leftChild,middleChild,rightChild}
*					@param[in] pChild: Node address
*					@return none
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
****************************************************************************************************/
BALBSTCLSTMPLT
inline void CNodeT::setChild(eTraverseDirection eIndex, CNodeT* pChild)
{
	__try
	{
		m_pChildren[eIndex] = pChild;
		if (pChild != 0)
		{
			pChild->setParent(this);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CNodeT::setChild", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : setParent
*  Description    :	Member function taking one argument and returning none.
*					Sets the parent of node.
*					@return none.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
****************************************************************************************************/
BALBSTCLSTMPLT
inline void CNodeT::setParent(CNode* pNode)
{
	m_pParent = pNode;
}

/***************************************************************************************************
*  Function Name  : getParent
*  Description    : Member function taking no arguments and returning a pointer to parent node.
*					@return pointer to parent node.
*					@return none.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
inline CNodeT* CNodeT::getParent(void) const
{
	return m_pParent;
}

/***************************************************************************************************
*	Function Name  : getSmallKey
*	Description    : Member function taking no arguments and returning small key address.
*						Small key is a template type.
*						@return small key address.
*	Author Name    : Ram Shelke
*	Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
inline const T* const CNodeT::getSmallKey(void) const
{
	return &m_tSmallKey;
}

/***************************************************************************************************
*  Function Name  : setSmallKey
*  Description    : Member function taking one argument and returning none.
*					Small key value of node is set. Small key is a template type.
*					Flag m_bSmallKeyIsSet is used to keep track whether the value is set.
Flag is used because template variables cannot easily be checked for zero.
@param[in] address to small key.
@return none
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
inline void CNodeT::setSmallKey(const T* const pSmallKey)
{
	if (pSmallKey == 0)
	{
		m_bSmallKeyIsSet = false;
	}
	else
	{
		m_tSmallKey = *pSmallKey;
		m_bSmallKeyIsSet = true;
	}
}

/***************************************************************************************************
*  Function Name  : getBigKey
*  Description    : Member function taking no arguments and returning address of big key.
*					@return address of big key
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
inline const T* const CNodeT::getBigKey(void) const
{
	return &m_tBigKey;
}

/***************************************************************************************************
*  Function Name  : setBigKey
*  Description    : Member function taking one argument and returning none.
*					Big key value of the node is set. Big key is a template type.
*					Flag m_bBigKeyIsSet is used to to keep track whether value is set.
*					Flag is used because template variables cannot easily be checked for zero.
*					@param[in] address to big key.
*					@return none
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
inline void CNodeT::setBigKey(const T* const pBigKey)
{
	if (pBigKey == 0)
	{
		m_bBigKeyIsSet = false;
	}
	else
	{
		m_tBigKey = *pBigKey;
		m_bBigKeyIsSet = true;
	}
}

/***************************************************************************************************
*  Function Name  : getSize
*  Description    : Member function taking no arguments and returning the size of the node.
*					Node size: { empty, twoNode, threeNode }
*					@return node size (enum)
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
eNodeSize CNodeT::getSize(void) const
{
	eNodeSize eSize = empty;
	__try
	{

		if (m_bBigKeyIsSet == true && m_bSmallKeyIsSet == true)
		{
			eSize = threeNode;
		}
		else if (m_bSmallKeyIsSet == true)
		{
			eSize = twoNode;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CNodeT::getSize", 0, 0, true, SECONDLEVEL);
	}
	return eSize;
}

/***************************************************************************************************
*  Function Name  : isLeaf
*  Description    : Member function taking no arguments and returning a boolean.
*					true if node is a leaf, false otherwise.
*					@return boolean.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
bool CNodeT::isLeaf(void) const
{
	if (m_pChildren[leftChild] == 0 &&
		m_pChildren[middleChild] == 0 &&
		m_pChildren[middleChild] == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/***************************************************************************************************
*  Function Name  : CWWizBalBst
*  Description    : Constructor that sets variables to zero.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
CWWizBalBstT::CWWizBalBst(void) : 
m_pRoot(0),
m_dwNodeCount(0)
{

	m_dwMaxCount = 0x7FFFFFFF;
}

/***************************************************************************************************
*  Function Name  : CWWizBalBst
*  Description    : Constructor that sets variables to zero.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
CWWizBalBstT::CWWizBalBst(DWORD dwMaxCount) : 
m_pRoot(0),
m_dwNodeCount(0)
{
	m_dwMaxCount = dwMaxCount;
}

/***************************************************************************************************
*  Function Name  : ~CWWizBalBst
*  Description    : Destructor cleanup the tree. Delete all nodes.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT CWWizBalBstT::~CWWizBalBst(void)
{
	if (m_pRoot != 0)
	{
		removeAll();
	}
}

/***************************************************************************************************
*  Function Name  : removeAll
*  Description    : @brief     Member function taking no argument and returning an error code.
*					Deletes all the nodes of the tree. Error codes are specified in "CTreeError.h".
*					@param[in] None.
*					@return    SUCCESS when no error encountered.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::removeAll(void)
{
	int iReturnCode = FAILURE;
	__try
	{
		CNodeT *pNodeIterator = 0;
		CNodeT *pParentNode = 0;
		CNodeT *pRightNode = 0;
		CNodeT *pMiddleNode = 0;
		CNodeT *pLeftNode = 0;

		/* If no root present, no need to delete nodes. */
		if (m_pRoot == 0)
		{
			iReturnCode = IS_EMPTY;
		}
		else
		{
			pNodeIterator = getMinNode(m_pRoot);

			while ((pParentNode = pNodeIterator->getParent()) != 0)
			{
				pRightNode = pParentNode->getChild(rightChild);
				pMiddleNode = pParentNode->getChild(middleChild);
				pLeftNode = pParentNode->getChild(leftChild);

				if ((pNodeIterator == pLeftNode &&
					pMiddleNode == 0) ||
					(pNodeIterator == pMiddleNode))
				{
					delete pNodeIterator;
					pNodeIterator = getMinNode(pRightNode);
				}
				else if (pNodeIterator == pLeftNode)
				{
					delete pNodeIterator;
					pNodeIterator = getMinNode(pMiddleNode);
				}
				else if (pNodeIterator == pRightNode)
				{
					delete pNodeIterator;
					pNodeIterator = pParentNode;
				}
			}
			/* Finally delete root. */
			delete m_pRoot;
			m_pRoot = 0;
			iReturnCode = SUCCESS;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::removeAll", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : insert
*  Description    : @brief     Member function taking one argument and returning an error code.
*					           Adds a key value to tree. Error codes are specified in "CTreeError.h".
*					@param[in] pKey: address of key value to be added to tree.
*					@return    SUCCESS when no error encountered.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::insert(const T* const pKey)
{
	int iReturnCode = FAILURE;
	try
	{
		if (m_dwNodeCount > m_dwMaxCount)
			return iReturnCode;

		CNodeT* pNodeFound = 0;
		CNodeT* pAddedNode = 0;
		bool bTreeIsFixed = false;
		bool bFound = false;

		/* If no root present, create one. */
		if (m_pRoot == 0)
		{
			m_pRoot = new CNodeT();
			add(m_pRoot, pKey);
			iReturnCode = SUCCESS;
		}
		else
		{   /* Find place in tree to insert key.*/
			iReturnCode = search(pKey,
				&pNodeFound,
				&bFound);
		}

		if (iReturnCode == SUCCESS &&
			pNodeFound != 0)
		{
			/* Error duplicate value. */
			if (bFound == true)
			{
				iReturnCode = DUPLICATE_VALUE;
			}
			/* Unique Key value, add it to tree (straightforward).*/
			else if (pNodeFound->getSize() == twoNode)
			{
				add(pNodeFound, pKey);
				iReturnCode = SUCCESS;
			}
			/* In case of three node we need to split node. */
			else if (pNodeFound->getSize() == threeNode)
			{
				/* Template key variable needed for iteration purposes. */
				/* Node splitting can propagate up the tree using tKey as output. */
				T tKey = *pKey;
				do
				{
					/* Split node. */
					iReturnCode = split(&tKey,
						pNodeFound,
						&pAddedNode);

					if (iReturnCode == SUCCESS)
					{
						/* Node splitting has propagated up to root node, create new root. */
						if (pNodeFound->getParent() == 0)
						{
							CNodeT* pNewRoot = new CNodeT(pAddedNode, pNodeFound, &tKey);
							m_pRoot = pNewRoot;
							bTreeIsFixed = true;
							iReturnCode = SUCCESS;
						}
						else
						{
							pNodeFound = pNodeFound->getParent();
							if (pNodeFound->getSize() == twoNode)
							{
								if (TCompare(&tKey, pNodeFound->getSmallKey()) == LESS)
								{
									add(pNodeFound, &tKey);
									pNodeFound->setChild(middleChild, pNodeFound->getChild(leftChild));
									pNodeFound->setChild(leftChild, pAddedNode);
									bTreeIsFixed = true;
									iReturnCode = SUCCESS;
								}
								else if (TCompare(&tKey, pNodeFound->getSmallKey()) == GREATER)
								{
									add(pNodeFound, &tKey);
									pNodeFound->setChild(middleChild, pAddedNode);
									bTreeIsFixed = true;
									iReturnCode = SUCCESS;
								}
							}
						}
					}
				} while (bTreeIsFixed == false && iReturnCode == SUCCESS);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::insert", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : find
*  Description    : @brief     Member function taking one argument and returning the address of key object.
*					           Searches the tree for the specified key.
*					@param[in] pKey: address of key value to look for in tree.
*					@return    address of key object, 0 if key object can not be found in tree.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
const T* const CWWizBalBstT::find(const T* const pKey)
{
	__try
	{
		CNodeT *pNodeFound = 0;
		bool bKeyFound = false;

		search(pKey, &pNodeFound, &bKeyFound);
		if (bKeyFound == true &&
			TCompare(pKey, pNodeFound->getSmallKey()) == EQUAL)
		{
			return pNodeFound->getSmallKey();
		}
		else if (bKeyFound == true &&
			pNodeFound->getSize() == threeNode &&
			TCompare(pKey, pNodeFound->getBigKey()) == EQUAL)
		{
			return pNodeFound->getBigKey();
		}
		else
		{
			return 0;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::find", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : search
*  Description    : @brief      Member function taking three arguments and returning an error code.
*					            Function searches the tree to find a leaf node where the new key value should be inserted.
*					@param[in]  pKey: address of key value to search for.
*					@param[out] ppNodeFound: address of node pointer which points to a leaf node where the new key value should be inserted.
@param[out] bFound: If key value already present in tree then bool value at address bFound will be set to true, false otherwise.
@return     SUCCESS when no error encountered.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::search(const T* const pKey, CNodeT** ppNodeFound, bool* bFound) const
{
	eTraverseDirection eSearchDirection;
	CNodeT *pNodeIterator = 0;
	int iReturnCode = FAILURE;
	__try
	{
		*bFound = false;
		*ppNodeFound = 0;

		if (m_pRoot == 0)
		{
			iReturnCode = IS_EMPTY;
		}
		else
		{
			pNodeIterator = m_pRoot;

			do
			{
				/* Key found in tree, DUPLICATE_VALUE. */
				if (TCompare(pKey, pNodeIterator->getSmallKey()) == EQUAL ||
					(pNodeIterator->getSize() == threeNode &&
					TCompare(pKey, pNodeIterator->getBigKey()) == EQUAL))
				{
					*bFound = true;
					*ppNodeFound = pNodeIterator;
					iReturnCode = SUCCESS;
				}
				/* If (search) key is smaller than smallkey node, traverse to the left child. */
				else if (TCompare(pKey, pNodeIterator->getSmallKey()) == LESS)
				{
					eSearchDirection = leftChild;
				}
				/* If (search) key is larger than bigkey node, traverse to the right child. */
				else if (pNodeIterator->getSize() == threeNode &&
					TCompare(pKey, pNodeIterator->getBigKey()) == GREATER ||
					(pNodeIterator->getSize() == twoNode &&
					TCompare(pKey, pNodeIterator->getSmallKey()) == GREATER))
				{
					eSearchDirection = rightChild;
				}
				/* Else traverse to the middle child. */
				else
				{
					eSearchDirection = middleChild;
				}

				/* Move to child. */
				if (*bFound == false && pNodeIterator->getChild(eSearchDirection) == 0)
				{
					*ppNodeFound = pNodeIterator;
					iReturnCode = SUCCESS;
				}
				else if (*bFound == false)
				{
					pNodeIterator = pNodeIterator->getChild(eSearchDirection);
				}
			} while (*ppNodeFound == 0);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::search", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : add
*  Description    : Member function taking two arguments and returning an error code.
*					Function adds a key value to an empty or a two node.
*					@param[in]  pNode: address of node to which the key value will be added.
*					@param[in]  pKey: address of key value which will be added to node.
*					@return     SUCCESS when no error encountered.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::add(CNodeT* const pNode, const T* const pKey)
{
	int iReturnCode = FAILURE;
	__try
	{
		if (m_dwNodeCount > m_dwMaxCount)
			return iReturnCode;

		eNodeSize eSize = pNode->getSize();

		if (eSize == empty)
		{
			pNode->setSmallKey(pKey);
			iReturnCode = SUCCESS;
		}
		else if (eSize == twoNode && TCompare(pKey, pNode->getSmallKey()) == GREATER)
		{
			pNode->setBigKey(pKey);
			iReturnCode = SUCCESS;
		}
		else if (eSize == twoNode && TCompare(pKey, pNode->getSmallKey()) == LESS)
		{
			pNode->setBigKey(pNode->getSmallKey());
			pNode->setSmallKey(pKey);
			iReturnCode = SUCCESS;
		}

		if (iReturnCode == SUCCESS)
		{
			m_dwNodeCount++;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::add", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : split
*  Description    : Member function taking three arguments and returning an error code.
*					Function splits a three node in order to add a key.
*					Node Splitting can propagate all the way up to the root node.
*					Due to this propagation effect a key value will be written to address pKey.
*					@param[in/out] pKey: address of key value which will be added to three node.
*					@param[in/out] pNode2Split: address of three node to which the key will be added.
*					@param[in/out] ppAddedNode: address of node pointer to newly created node.
*					@return        SUCCESS when no error encountered.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::split(T* const pKey, CNodeT* const pNode2Split, CNodeT** ppAddedNode)
{
	int iReturnCode = FAILURE;
	try
	{
		CNodeT* pNode = 0;

		if (pNode2Split == 0)
		{
			iReturnCode = IS_EMPTY;
		}
		else
		{
			/* Three ways to split a node, depending on key value. */
			if (TCompare(pKey, pNode2Split->getBigKey()) == GREATER)
			{
				pNode = new CNodeT(pNode2Split->getChild(leftChild),
					pNode2Split->getChild(middleChild),
					pNode2Split->getSmallKey());
				pNode2Split->setSmallKey(pKey);
				*pKey = *pNode2Split->getBigKey();
				pNode2Split->setChild(leftChild, *ppAddedNode);
				iReturnCode = SUCCESS;
			}
			else if (TCompare(pKey, pNode2Split->getSmallKey()) == LESS)
			{
				pNode = new CNodeT(*ppAddedNode,
					pNode2Split->getChild(leftChild),
					pKey);
				*pKey = *pNode2Split->getSmallKey();
				pNode2Split->setSmallKey(pNode2Split->getBigKey());
				pNode2Split->setChild(leftChild, pNode2Split->getChild(middleChild));
				iReturnCode = SUCCESS;
			}
			else /* Split in the middle. */
			{
				pNode = new CNodeT(pNode2Split->getChild(leftChild),
					*ppAddedNode,
					pNode2Split->getSmallKey());
				pNode2Split->setSmallKey(pNode2Split->getBigKey());
				pNode2Split->setChild(leftChild, pNode2Split->getChild(middleChild));
				iReturnCode = SUCCESS;
			}

			pNode2Split->setBigKey(0);
			pNode2Split->setChild(middleChild, 0);
			*ppAddedNode = pNode;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::split", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : getMinNode
*  Description    : Member function taking one arguments and returning the address of a node.
Function finds and returns the node with the smallest key value for a given tree entry.
@param[in]    pNode: node address of tree entry (where search starts from).
@return       address of a node holding smallest key value.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
CNodeT* CWWizBalBstT::getMinNode(CNodeT* pNode) const
{
	CNodeT* pNodeIterator = pNode;
	while (pNodeIterator->getChild(leftChild) != 0)
	{
		pNodeIterator = pNodeIterator->getChild(leftChild);
	}
	return pNodeIterator;
}

/***************************************************************************************************
*  Function Name  : getMaxNode
*  Description    : Member function taking one arguments and returning the address of a node.
*					Function finds and returns the node with the largest key value for a given tree entry.
*					@param[in]    pNode: node address of tree entry (where search starts from).
*					@return       address of a node holding largest key value.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
CNodeT* CWWizBalBstT::getMaxNode(CNodeT* pNode) const
{
	CNodeT* pNodeIterator = pNode;
	while (pNodeIterator->getChild(rightChild) != 0)
	{
		pNodeIterator = pNodeIterator->getChild(rightChild);
	}
	return pNodeIterator;
}

/***************************************************************************************************
*  Function Name  : deleteItem
*  Description    : Member function taking one arguments and returning an error code.
*					Function deletes the specified key value from tree.
*					@param[in]    pKey: address of key value which needs to be deleted from tree.
*					@return       SUCCESS when no error encountered.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::deleteItem(const T* const pKey)
{
	CNodeT* pNode = 0;
	CNodeT* pNextNode2Fix = 0;
	int iReturnCode = FAILURE;
	__try
	{
		bool bTreeIsFixed = false;
		bool bFound = false;

		if (m_pRoot == 0) /* Tree is empty*/
		{
			iReturnCode = IS_EMPTY;
		}
		else /* Find key value. */
		{
			iReturnCode = search(pKey, &pNode, &bFound);
		}

		/* Key not found in Tree. */
		if (iReturnCode == SUCCESS && !bFound)
		{
			iReturnCode = NOT_FOUND;
		}

		/* Key found in Tree. */
		if (iReturnCode == SUCCESS)
		{
			/* If pNode is internal node swap with in order successor. */
			if (!pNode->isLeaf())
			{
				iReturnCode = swapWithInOrderSuccessor(pNode, pKey, &pNode);
			}

			/* Leaf is three node, delete is straightforward. */
			if (iReturnCode == SUCCESS && pNode->getSize() == threeNode)
			{
				iReturnCode = removeFromLeaf(pNode, pKey);
			}
			else if (iReturnCode == SUCCESS) /* Delete can propagate up to the root. */
			{
				do
				{
					if (pNode == m_pRoot)
					{
						m_pRoot = pNode->getChild(leftChild);
						/* If tree is empty, no need to reset parent root. */
						if (m_pRoot != 0)
						{
							m_pRoot->setParent(0);
						}
						delete pNode;
						bTreeIsFixed = true;
					}
					/* Check if local rotation is possible. */
					/* Check if global rotation is possible. */
					/* Check if local node merger is possible. */
					else if (localRotation(pNode) == SUCCESS ||
						globalRotation(pNode) == SUCCESS ||
						localNodeMerger(pNode) == SUCCESS)
					{
						bTreeIsFixed = true;
					}
					else if (globalNodeMerger(pNode, &pNextNode2Fix) == SUCCESS)
					{
						pNode = pNextNode2Fix;
						iReturnCode = SUCCESS;
					}
					else
					{
						iReturnCode = FAILURE;
					}
				} while (!bTreeIsFixed && iReturnCode == SUCCESS);
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::deleteItem", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : removeFromLeaf
*  Description    : Member function taking two arguments and returning an error code.
*					Function deletes the specified key value from the specified three node.
*					Delete is straightforward.
*					@param[in]  pNode: address of three node containing key value to be deleted.
*					@param[in]  pKey: address of key value to be deleted.
*					@return     SUCCESS when no error encountered.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::removeFromLeaf(CNodeT* pNode,
const T* const pKey)
{
	int iReturnCode = FAILURE;
	__try
	{
		if (pNode == 0)
		{
			iReturnCode = IS_EMPTY;
		}
		else
		{
			if (TCompare(pKey, pNode->getSmallKey()) == EQUAL)
			{
				pNode->setSmallKey(pNode->getBigKey());
				pNode->setBigKey(0);
				iReturnCode = SUCCESS;
			}
			else if (TCompare(pKey, pNode->getBigKey()) == EQUAL)
			{
				pNode->setBigKey(0);
				iReturnCode = SUCCESS;
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::removeFromLeaf", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : localRotation
*  Description    : Member function taking one arguments and returning an error code.
*					Function checks if a local tree rotation is possible. If so the specified
*					node will be deleted and a local tree rotation will be performed to fix the tree.
*					@param[in]  pNode: address of node to be deleted.
*					@return     SUCCESS when no error encountered and local rotation is possible.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::localRotation(CNodeT* pNode)
{
	CNodeT* pParentNode = 0;
	CNodeT* pRightNode = 0;
	CNodeT* pMiddleNode = 0;
	CNodeT* pLeftNode = 0;
	int iReturnCode = FAILURE;
	__try
	{
		if (pNode == 0)
		{
			iReturnCode = IS_EMPTY;
		}
		else
		{
			pParentNode = pNode->getParent();
			pRightNode = pParentNode->getChild(rightChild);
			pMiddleNode = pParentNode->getChild(middleChild);
			pLeftNode = pParentNode->getChild(leftChild);
			/* Case 1: Take from right. */
			if (pNode == pLeftNode &&
				pParentNode->getSize() == twoNode &&
				pRightNode->getSize() == threeNode)
			{
				pLeftNode->setSmallKey(pParentNode->getSmallKey());
				pLeftNode->setChild(rightChild, pRightNode->getChild(leftChild));
				pParentNode->setSmallKey(pRightNode->getSmallKey());
				pRightNode->setSmallKey(pRightNode->getBigKey());
				pRightNode->setBigKey(0);
				pRightNode->setChild(leftChild, pRightNode->getChild(middleChild));
				pRightNode->setChild(middleChild, 0);
				iReturnCode = SUCCESS;
			}
			/* Case 2: Take from middle first. */
			else if (pNode == pLeftNode &&
				pParentNode->getSize() == threeNode &&
				pMiddleNode->getSize() == threeNode)
			{
				pLeftNode->setSmallKey(pParentNode->getSmallKey());
				pLeftNode->setChild(rightChild, pMiddleNode->getChild(leftChild));
				pParentNode->setSmallKey(pMiddleNode->getSmallKey());
				pMiddleNode->setSmallKey(pMiddleNode->getBigKey());
				pMiddleNode->setBigKey(0);
				pMiddleNode->setChild(leftChild, pMiddleNode->getChild(middleChild));
				pMiddleNode->setChild(middleChild, 0);
				iReturnCode = SUCCESS;
			}
			/* Case 3: Take from right. */
			else if (pNode == pLeftNode &&
				pParentNode->getSize() == threeNode &&
				pRightNode->getSize() == threeNode)
			{
				pLeftNode->setSmallKey(pParentNode->getSmallKey());
				pLeftNode->setChild(rightChild, pMiddleNode->getChild(leftChild));
				pParentNode->setSmallKey(pMiddleNode->getSmallKey());
				pMiddleNode->setSmallKey(pParentNode->getBigKey());
				pMiddleNode->setChild(leftChild, pMiddleNode->getChild(rightChild));
				pMiddleNode->setChild(rightChild, pRightNode->getChild(leftChild));
				pParentNode->setBigKey(pRightNode->getSmallKey());
				pRightNode->setSmallKey(pRightNode->getBigKey());
				pRightNode->setBigKey(0);
				pRightNode->setChild(leftChild, pRightNode->getChild(middleChild));
				pRightNode->setChild(middleChild, 0);
				iReturnCode = SUCCESS;
			}
			/* Case 4: Take from right first. */
			else if (pNode == pMiddleNode &&
				pRightNode->getSize() == threeNode)
			{
				pMiddleNode->setSmallKey(pParentNode->getBigKey());
				pMiddleNode->setChild(rightChild, pRightNode->getChild(leftChild));
				pParentNode->setBigKey(pRightNode->getSmallKey());
				pRightNode->setSmallKey(pRightNode->getBigKey());
				pRightNode->setBigKey(0);
				pRightNode->setChild(leftChild, pRightNode->getChild(middleChild));
				pRightNode->setChild(middleChild, 0);
				iReturnCode = SUCCESS;
			}
			/* Case 5: Take from left. */
			else if (pNode == pMiddleNode &&
				pLeftNode->getSize() == threeNode)
			{
				pMiddleNode->setSmallKey(pParentNode->getSmallKey());
				pMiddleNode->setChild(rightChild, pMiddleNode->getChild(leftChild));
				pMiddleNode->setChild(leftChild, pLeftNode->getChild(rightChild));
				pParentNode->setSmallKey(pLeftNode->getBigKey());
				pLeftNode->setBigKey(0);
				pLeftNode->setChild(rightChild, pLeftNode->getChild(middleChild));
				pLeftNode->setChild(middleChild, 0);
				iReturnCode = SUCCESS;
			}
			/* Case 6: Take from left. */
			if (pNode == pRightNode &&
				pParentNode->getSize() == twoNode &&
				pLeftNode->getSize() == threeNode)
			{
				pRightNode->setSmallKey(pParentNode->getSmallKey());
				pRightNode->setChild(rightChild, pRightNode->getChild(leftChild));
				pRightNode->setChild(leftChild, pLeftNode->getChild(rightChild));
				pParentNode->setSmallKey(pLeftNode->getBigKey());
				pLeftNode->setBigKey(0);
				pLeftNode->setChild(rightChild, pLeftNode->getChild(middleChild));
				pLeftNode->setChild(middleChild, 0);
				iReturnCode = SUCCESS;
			}
			/* Case 7: Take from middle. */
			else if (pNode == pRightNode &&
				pParentNode->getSize() == threeNode &&
				pMiddleNode->getSize() == threeNode)
			{
				pRightNode->setSmallKey(pParentNode->getBigKey());
				pRightNode->setChild(rightChild, pRightNode->getChild(leftChild));
				pRightNode->setChild(leftChild, pMiddleNode->getChild(rightChild));
				pParentNode->setBigKey(pMiddleNode->getBigKey());
				pMiddleNode->setBigKey(0);
				pMiddleNode->setChild(rightChild, pMiddleNode->getChild(middleChild));
				pMiddleNode->setChild(middleChild, 0);
				iReturnCode = SUCCESS;
			}
			/* Case 8: Take from left. */
			else if (pNode == pRightNode &&
				pParentNode->getSize() == threeNode &&
				pLeftNode->getSize() == threeNode)
			{
				pRightNode->setSmallKey(pParentNode->getBigKey());
				pRightNode->setChild(rightChild, pRightNode->getChild(leftChild));
				pRightNode->setChild(leftChild, pMiddleNode->getChild(rightChild));
				pParentNode->setBigKey(pMiddleNode->getSmallKey());
				pMiddleNode->setSmallKey(pParentNode->getSmallKey());
				pMiddleNode->setChild(rightChild, pMiddleNode->getChild(leftChild));
				pMiddleNode->setChild(leftChild, pLeftNode->getChild(rightChild));
				pParentNode->setSmallKey(pLeftNode->getBigKey());
				pLeftNode->setBigKey(0);
				pLeftNode->setChild(rightChild, pLeftNode->getChild(middleChild));
				pLeftNode->setChild(middleChild, 0);
				iReturnCode = SUCCESS;
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::localRotation", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : globalRotation
*  Description    : Member function taking one arguments and returning an error code.
*					Function checks if a global tree rotation is possible. If so the specified
*					node will be deleted and a global tree rotation will be performed to fix the tree.
*					@param[in]  pNode: address of node to be deleted.
*					@return     SUCCESS when no error encountered and global rotation is possible.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::globalRotation(CNodeT* pNode)
{
	CNodeT* pRightMinNode = 0;
	CNodeT* pLeftMaxNode = 0;
	CNodeT* pMiddleMinNode = 0;
	CNodeT* pMiddleMaxNode = 0;
	int iReturnCode = FAILURE;
	__try
	{

		if (pNode == 0)
		{
			iReturnCode = IS_EMPTY;
		}
		else
		{
			/* Leftmost child of the right child of the root. */
			pRightMinNode = getMinNode(m_pRoot->getChild(rightChild));
			/* Rightmost child of the left child of the root. */
			pLeftMaxNode = getMaxNode(m_pRoot->getChild(leftChild));
			/* Four rotations around root possible. */
			if (m_pRoot->getSize() == threeNode)
			{
				/* Leftmost child of the middle child of the root. */
				pMiddleMinNode = getMinNode(m_pRoot->getChild(middleChild));
				/* Rightmost child of the middle child of the root. */
				pMiddleMaxNode = getMaxNode(m_pRoot->getChild(middleChild));
				/* Case 9: Take from leftmost child of the middle child of the root. */
				if (pNode == pLeftMaxNode &&
					pMiddleMinNode->getSize() == threeNode)
				{
					pLeftMaxNode->setSmallKey(m_pRoot->getSmallKey());
					m_pRoot->setSmallKey(pMiddleMinNode->getSmallKey());
					pMiddleMinNode->setSmallKey(pMiddleMinNode->getBigKey());
					pMiddleMinNode->setBigKey(0);
					iReturnCode = SUCCESS;
				}
				/* Case 10: Take from rightmost child of the left child of the root. */
				else if (pNode == pMiddleMinNode &&
					pLeftMaxNode->getSize() == threeNode)
				{
					pMiddleMinNode->setSmallKey(m_pRoot->getSmallKey());
					m_pRoot->setSmallKey(pLeftMaxNode->getBigKey());
					pLeftMaxNode->setBigKey(0);
					iReturnCode = SUCCESS;
				}
				/* Case 11: Take from leftmost child of the right child of the root. */
				else if (pNode == pMiddleMaxNode &&
					pRightMinNode->getSize() == threeNode)
				{
					pMiddleMaxNode->setSmallKey(m_pRoot->getBigKey());
					m_pRoot->setBigKey(pRightMinNode->getSmallKey());
					pRightMinNode->setSmallKey(pRightMinNode->getBigKey());
					pRightMinNode->setBigKey(0);
					iReturnCode = SUCCESS;
				}
				/* Case 12: Take from rightmost child of the middle child of the root. */
				else if (pNode == pRightMinNode &&
					pMiddleMaxNode->getSize() == threeNode)
				{
					pRightMinNode->setSmallKey(m_pRoot->getBigKey());
					m_pRoot->setBigKey(pMiddleMaxNode->getBigKey());
					pMiddleMaxNode->setBigKey(0);
					iReturnCode = SUCCESS;
				}
			}
			else /* TWO_NODE: Two rotations around root possible. */
			{
				/* Case 13: Take from rightmost child of the left child of the root. */
				if (pNode == pRightMinNode &&
					pLeftMaxNode->getSize() == threeNode)
				{
					pRightMinNode->setSmallKey(m_pRoot->getSmallKey());
					m_pRoot->setSmallKey(pLeftMaxNode->getBigKey());
					pLeftMaxNode->setBigKey(0);
					iReturnCode = SUCCESS;
				}
				/* Case 14: Take from leftmost child of the right child of the root. */
				else if (pNode == pLeftMaxNode &&
					pRightMinNode->getSize() == threeNode)
				{
					pLeftMaxNode->setSmallKey(m_pRoot->getSmallKey());
					m_pRoot->setSmallKey(pRightMinNode->getSmallKey());
					pRightMinNode->setSmallKey(pRightMinNode->getBigKey());
					pRightMinNode->setBigKey(0);
					iReturnCode = SUCCESS;
				}
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::globalRotation", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : localNodeMerger
*  Description    : Member function taking one arguments and returning an error code.
*					Function checks if a local node merger is possible. If so the specified
*					node will be deleted and a local node merger will be performed to fix the tree.
*					@param[in]  pNode: address of node to be deleted.
*					@return     SUCCESS when no error encountered and local node merger is possible.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::localNodeMerger(CNodeT* pNode)
{
	CNodeT* pParentNode = 0;
	CNodeT* pRightNode = 0;
	CNodeT* pMiddleNode = 0;
	CNodeT* pLeftNode = 0;
	int iReturnCode = FAILURE;
	__try
	{
		if (pNode == 0)
		{
			iReturnCode = IS_EMPTY;
		}
		else
		{
			pParentNode = pNode->getParent();
			pRightNode = pParentNode->getChild(rightChild);
			pMiddleNode = pParentNode->getChild(middleChild);
			pLeftNode = pParentNode->getChild(leftChild);

			if (pNode == pLeftNode &&
				pParentNode->getSize() == threeNode) /* Case 15: Merge to the left. */
			{
				pLeftNode->setSmallKey(pParentNode->getSmallKey());
				pLeftNode->setBigKey(pMiddleNode->getSmallKey());
				pLeftNode->setChild(middleChild, pMiddleNode->getChild(leftChild));
				pLeftNode->setChild(rightChild, pMiddleNode->getChild(rightChild));
				pParentNode->setSmallKey(pParentNode->getBigKey());
				pParentNode->setBigKey(0);
				pParentNode->setChild(middleChild, 0);
				delete pMiddleNode;
				iReturnCode = SUCCESS;
			}
			else if (pNode == pMiddleNode &&
				pParentNode->getSize() == threeNode) /* Case 16: Merge to the left. */
			{
				pLeftNode->setBigKey(pParentNode->getSmallKey());
				pLeftNode->setChild(middleChild, pLeftNode->getChild(rightChild));
				pLeftNode->setChild(rightChild, pMiddleNode->getChild(leftChild));
				pParentNode->setSmallKey(pParentNode->getBigKey());
				pParentNode->setBigKey(0);
				pParentNode->setChild(middleChild, 0);
				delete pMiddleNode;
				iReturnCode = SUCCESS;
			}
			else if (pNode == pRightNode &&
				pParentNode->getSize() == threeNode) /* Case 17: Merge to the right. */
			{
				pRightNode->setSmallKey(pMiddleNode->getSmallKey());
				pRightNode->setBigKey(pParentNode->getBigKey());
				pRightNode->setChild(rightChild, pRightNode->getChild(leftChild));
				pRightNode->setChild(middleChild, pMiddleNode->getChild(rightChild));
				pRightNode->setChild(leftChild, pMiddleNode->getChild(leftChild));
				pParentNode->setBigKey(0);
				pParentNode->setChild(middleChild, 0);
				delete pMiddleNode;
				iReturnCode = SUCCESS;
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::localNodeMerger", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : globalNodeMerger
*  Description    :  Member function taking one arguments and returning an error code.
*					Function checks if a global node merger is possible. If so the specified
*					node will be deleted and a global node merger will be performed to fix the tree.
*					@param[in]  pNode: address of node to be deleted.
*					@param[out] ppNextNode2Fix: pointer to node pointer which points to the next node to fix, because node merger can propagate.
*					@return     SUCCESS when no error encountered and global node merger is possible.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::globalNodeMerger(CNodeT* pNode, CNodeT** ppNextNode2Fix)
{
	CNodeT* pParentNode = 0;
	CNodeT* pRightNode = 0;
	CNodeT* pLeftNode = 0;
	int iReturnCode = FAILURE;
	__try
	{
		*ppNextNode2Fix = 0;

		if (pNode == 0)
		{
			iReturnCode = IS_EMPTY;
		}
		else
		{
			pParentNode = pNode->getParent();
			pRightNode = pParentNode->getChild(rightChild);
			pLeftNode = pParentNode->getChild(leftChild);

			/* Case 18: Node merger propagates. */
			if (pNode == pLeftNode)
			{
				pLeftNode->setSmallKey(pParentNode->getSmallKey());
				pLeftNode->setBigKey(pRightNode->getSmallKey());
				pLeftNode->setChild(middleChild, pRightNode->getChild(leftChild));
				pLeftNode->setChild(rightChild, pRightNode->getChild(rightChild));
				pParentNode->setSmallKey(0);
				pParentNode->setChild(rightChild, 0);
				delete pRightNode;
				*ppNextNode2Fix = pParentNode;
				iReturnCode = SUCCESS;
			}
			/* Case 19: Node merger propagates. */
			else if (pNode == pRightNode)
			{
				pLeftNode->setBigKey(pParentNode->getSmallKey());
				pLeftNode->setChild(middleChild, pLeftNode->getChild(rightChild));
				pLeftNode->setChild(rightChild, pRightNode->getChild(leftChild));
				pParentNode->setChild(rightChild, 0);
				pParentNode->setSmallKey(0);
				delete pRightNode;
				*ppNextNode2Fix = pParentNode;
				iReturnCode = SUCCESS;
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::globalNodeMerger", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : swapWithInOrderSuccessor
*  Description    : Member function taking three arguments and returning an error code.
*					Function swaps an internal key value with its inorder successor.
*					Inorder successor of an internal key is always located in a leaf node.
*					@param[in]  pNode: node pointer to an internal node containing key value to be swapped.
*					@param[in]  pKey: address of key value to be swapped with its inorder successor.
*					@param[out] ppLeafNode: pointer to node pointer which points to a leaf node containing pKey value after swap.
*					@return     SUCCESS when no error encountered.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::swapWithInOrderSuccessor(CNodeT* pNode, const T* const pKey, CNodeT** ppLeafNode)
{
	int iReturnCode = FAILURE;
	__try
	{
		CNodeT* pLeafNode = 0;
		T tLeafKey = *pKey;
		*ppLeafNode = 0;

		if (pNode == 0)
		{
			iReturnCode = IS_EMPTY;
		}
		else /* Swap key with in order successor */
		{
			pLeafNode = getNextInorderKey(pNode, &tLeafKey);
			/* Successor internal node is always smallest value leafnode. */
			if (TCompare(pKey, pNode->getSmallKey()) == EQUAL)
			{
				pLeafNode->setSmallKey(pKey);
				pNode->setSmallKey(&tLeafKey);
				*ppLeafNode = pLeafNode;
				iReturnCode = SUCCESS;
			}
			else if (TCompare(pKey, pNode->getBigKey()) == EQUAL)
			{
				pLeafNode->setSmallKey(pKey);
				pNode->setBigKey(&tLeafKey);
				*ppLeafNode = pLeafNode;
				iReturnCode = SUCCESS;
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::swapWithInOrderSuccessor", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : print
*  Description    : Member function taking two arguments and returning an error code.
*					Function traverses the tree and prints the objects present in tree using the specified print function.
*					Three traversal ways implemented [inorder, postorder, preorder].
*					@param[in]  eTraversalMethod: enum traversal type [inOrder, postOrder, preOrder].
*					@param[out] pNrOfItemsPrinted: int Nr of objects in tree.
*					@return     SUCCESS when no error encountered.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::print(eTreeTraversal eTraversalMethod, int *pNrOfItemsPrinted) const
{
	int iReturnCode = FAILURE;
	__try
	{
		CNodeT* (CWWizBalBstT::*pTraversalFunc)(CNodeT* pNode, T* pKey) const = 0;
		CNodeT* pNodeIterator = 0;
		T tKey;
		int iItemCnt = 0;
		*pNrOfItemsPrinted = 0;

		if (m_pRoot == 0)
		{
			iReturnCode = IS_EMPTY;
		}
		else
		{
			if (eTraversalMethod == inorder)
			{
				pNodeIterator = getMinNode(m_pRoot);
				tKey = *pNodeIterator->getSmallKey();
				pTraversalFunc = &CWWizBalBstT::getNextInorderKey;
			}
			else if (eTraversalMethod == preorder)
			{
				pNodeIterator = m_pRoot;
				tKey = *pNodeIterator->getSmallKey();
				pTraversalFunc = &CWWizBalBstT::getNextPreorderKey;
			}
			else if (eTraversalMethod == postorder)
			{
				pNodeIterator = getMinNode(m_pRoot);
				tKey = *pNodeIterator->getSmallKey();
				pTraversalFunc = &CWWizBalBstT::getNextPostorderKey;
			}

			do
			{
				iItemCnt++;
				std::cout << tKey << std::endl;
				if (pTraversalFunc != 0)
				{
					pNodeIterator = (this->*pTraversalFunc)(pNodeIterator, &tKey);
				}
			} while (pNodeIterator != 0);
		}
		*pNrOfItemsPrinted = iItemCnt;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::print", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : getNextInorderKey
*  Description    : Member function taking two arguments and returning address of next inorder node.
*					Function for inorder traversing the tree. Function gets the next
*					inorder key relative to pNode and pKey. The next inorder key value
*					will be written to address pKey and next inorder node address will be returned.
*					@param[in/out] pNode: address of inorder node.
*					@param[in/out] pKey: address of inorder key value.
*					@return        address of next inorder node, returns 0 if no more inorder successors.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
CNodeT* CWWizBalBstT::getNextInorderKey(CNodeT* pNode, T* pKey) const
{
	CNodeT* pNodeIterator = pNode;
	__try
	{
		bool bFinished = false;

		if (pNodeIterator->getSize() == threeNode &&
			TCompare(pKey, pNodeIterator->getSmallKey()) == EQUAL)
		{
			if (pNodeIterator->isLeaf())
			{
				*pKey = *pNodeIterator->getBigKey();
			}
			else
			{
				pNodeIterator = getMinNode(pNodeIterator->getChild(middleChild));
				*pKey = *pNodeIterator->getSmallKey();
			}
		}
		else if (!pNodeIterator->isLeaf() &&
			((pNodeIterator->getSize() == twoNode &&
			TCompare(pKey, pNodeIterator->getSmallKey()) == EQUAL) ||
			(pNodeIterator->getSize() == threeNode &&
			TCompare(pKey, pNodeIterator->getBigKey()) == EQUAL)))
		{
			pNodeIterator = getMinNode(pNodeIterator->getChild(rightChild));
			*pKey = *pNodeIterator->getSmallKey();
		}
		else
		{
			do
			{
				pNodeIterator = pNodeIterator->getParent();
				if (pNodeIterator == 0)
				{
					bFinished = true;
				}
				else if (TCompare(pKey, pNodeIterator->getSmallKey()) == LESS)
				{
					*pKey = *pNodeIterator->getSmallKey();
					bFinished = true;
				}
				else if (pNodeIterator->getSize() == threeNode &&
					TCompare(pKey, pNodeIterator->getBigKey()) == LESS)
				{
					*pKey = *pNodeIterator->getBigKey();
					bFinished = true;
				}
			} while (bFinished == false);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::getNextInorderKey", 0, 0, true, SECONDLEVEL);
	}
	return pNodeIterator;
}

/***************************************************************************************************
*  Function Name  : getNextPostorderKey
*  Description    : Member function taking two arguments and returning address of next postorder node.
*					Function for postorder traversing the tree. Function gets the next
*					postorder key relative to pNode and pKey. The next postorder key value
*					will be written to address pKey and next postorder node address will be returned.
*					@param[in/out] pNode: address of postorder node.
*					@param[in/out] pKey: address of postorder key value.
*					@return        address of next postorder node, returns 0 if no more postorder successors.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
CNodeT* CWWizBalBstT::getNextPostorderKey(CNodeT* pNode, T* pKey) const
{
	CNodeT* pNodeIterator = pNode;
	__try
	{
		CNodeT* pChildNode = pNode;

		if (pNodeIterator->getSize() == threeNode &&
			TCompare(pKey, pNodeIterator->getSmallKey()) == EQUAL)
		{
			if (pNodeIterator->isLeaf())
			{
				*pKey = *pNodeIterator->getBigKey();
			}
			else
			{
				pNodeIterator = getMinNode(pNodeIterator->getChild(rightChild));
				*pKey = *pNodeIterator->getSmallKey();
			}
		}
		else if ((pNodeIterator = pNodeIterator->getParent()) != 0)
		{
			if (pNodeIterator->getSize() == twoNode)
			{
				if (pNodeIterator->getChild(leftChild) == pChildNode)
				{
					pNodeIterator = getMinNode(pNodeIterator->getChild(rightChild));
					*pKey = *pNodeIterator->getSmallKey();
				}
				else if (pNodeIterator->getChild(rightChild) == pChildNode)
				{
					*pKey = *pNodeIterator->getSmallKey();
				}
			}
			else if (pNodeIterator->getSize() == threeNode)
			{
				if (pNodeIterator->getChild(leftChild) == pChildNode)
				{
					pNodeIterator = getMinNode(pNodeIterator->getChild(middleChild));
					*pKey = *pNodeIterator->getSmallKey();
				}
				else if (pNodeIterator->getChild(middleChild) == pChildNode)
				{
					*pKey = *pNodeIterator->getSmallKey();
				}
				else if (pNodeIterator->getChild(rightChild) == pChildNode)
				{
					*pKey = *pNodeIterator->getBigKey();
				}
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::getNextPostorderKey", 0, 0, true, SECONDLEVEL);
	}
	return pNodeIterator;
}

/***************************************************************************************************
*  Function Name  : getNextPreorderKey
*  Description    : Member function taking two arguments and returning address of next postorder node.
*					Function for preorder traversing the tree. Function gets the next
*					preorder key relative to pNode and pKey. The next preorder key value
*					will be written to address pKey and next preorder node address will be returned.
*					@param[in/out] pNode: address of preorder node.
*					@param[in/out] pKey: address of preorder key value.
*					@return        address of next preorder node, returns 0 if no more postorder successors.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
CNodeT* CWWizBalBstT::getNextPreorderKey(CNodeT* pNode, T* pKey) const
{
	CNodeT* pNodeIterator = pNode;
	__try
	{
		CNodeT* pPrevNode = pNode;

		if (pNodeIterator->getSize() == threeNode &&
			TCompare(pKey, pNodeIterator->getSmallKey()) == EQUAL &&
			pNodeIterator->isLeaf())
		{
			*pKey = *pNodeIterator->getBigKey();
		}
		else if (TCompare(pKey, pNodeIterator->getSmallKey()) == EQUAL &&
			!pNodeIterator->isLeaf())
		{
			pNodeIterator = pNodeIterator->getChild(leftChild);
			*pKey = *pNodeIterator->getSmallKey();
		}
		else if (TCompare(pKey, pNodeIterator->getBigKey()) == EQUAL &&
			!pNodeIterator->isLeaf())
		{
			pNodeIterator = pNodeIterator->getChild(middleChild);
			*pKey = *pNodeIterator->getSmallKey();
		}
		else
		{
			while ((pNodeIterator = pNodeIterator->getParent()) != 0 &&
				pNodeIterator->getChild(rightChild) == pPrevNode)
			{
				pPrevNode = pNodeIterator;
			}

			if (pNodeIterator != 0)
			{
				if ((pNodeIterator->getSize() == twoNode &&
					pNodeIterator->getChild(leftChild) == pPrevNode) ||
					(pNodeIterator->getSize() == threeNode &&
					pNodeIterator->getChild(middleChild) == pPrevNode))
				{
					pNodeIterator = pNodeIterator->getChild(rightChild);
					*pKey = *pNodeIterator->getSmallKey();
				}
				else if (pNodeIterator->getChild(leftChild) == pPrevNode &&
					pNodeIterator->getSize() == threeNode)
				{
					*pKey = *pNodeIterator->getBigKey();
				}
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::getNextPreorderKey", 0, 0, true, SECONDLEVEL);
	}
	return pNodeIterator;
}

/***************************************************************************************************
*  Function Name  : TCompare
*  Description    : Member function taking two argument and returning an int.
*					Function compares the value of *pT1 and *pT1. Function returns EQUAL if
*					*pT1 and *pT2 are equal. If *pT1 > *pT2 function returns GREATER.
*					If *pT1 < *pT2 function returns LESS.
*					Comparison of object can be accomplished by overloading the < operator in the template class.
*					@param[in] pT1: Address of first key value used in comparison.
*					@param[in] pT2: Address of second key value used in comparison.
*					@return if *pT1 == *pT1 returns EQUAL, if *pT1 > *pT2 returns GREATER
*					        if *pT1 < *pT2 returns LESS.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::TCompare(const T* const pT1, const T* const pT2) const
{
	int iReturnCode = FAILURE;
	__try
	{
		if (*pT1 < *pT2)
		{
			iReturnCode = LESS;
		}
		else if (*pT2 < *pT1)
		{
			iReturnCode = GREATER;
		}
		else
		{
			iReturnCode = EQUAL;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::TCompare", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : TCompare
*  Description    : Member function taking one argument as file path
and load into tree memory.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
bool CWWizBalBstT::Load(LPTSTR lpszFilePath)
{
	bool bReturn = false;
	HANDLE hFileHandle = NULL;

	if (!PathFileExists(lpszFilePath))
	{
		AddLogEntry(L"### File doesn't exists [%s]", lpszFilePath, 0, true, SECONDLEVEL);
		return false;
	}

	if (FileIntegrityCheck(lpszFilePath) != 0x00)
	{
		AddLogEntry(L"### File integrity failed file [%s]", lpszFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
		goto CLEANUP;
	}

	hFileHandle = CreateFile(lpszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFileHandle == INVALID_HANDLE_VALUE)
	{
		bReturn = false;
		goto CLEANUP;
	}

	DWORD dwCount = 0x00;
	while (true)
	{
		//if Count is greater than allowed count then exit from the loop.
		if (dwCount > m_dwMaxCount)
			break;

		T ulCRCValue = 0;
		QWORD dwBufferOffset = 0;

		DWORD dwBytesRead = 0;
		if (!ReadFile(hFileHandle, &ulCRCValue, sizeof(T), &dwBytesRead, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != sizeof(T))
		{
			break;
		}

		insert(&ulCRCValue);

		dwBufferOffset += dwBytesRead;
		dwCount++;
	}

	m_dwNodeCount = dwCount;

	bReturn = true;

	//Need to close file handle
	if (hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFileHandle);
		hFileHandle = INVALID_HANDLE_VALUE;
	}

CLEANUP:
	//Need to close file handle
	if (hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFileHandle);
		hFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : Save
*  Description    : Member function taking one argument as file path
and save Tree memory in file.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
bool CWWizBalBstT::Save(LPTSTR lpszFilePath)
{
	bool bReturn = false;
	
	//Check here the drive type
	TCHAR szDrivePath[4] = _T("x:\\");
	szDrivePath[0] = lpszFilePath[0];
	if (GetDriveType(szDrivePath) != DRIVE_FIXED)
	{
		return bReturn;
	}

	//Delete Previous file
	if (PathFileExists(lpszFilePath))
	{
		SetFileAttributes(lpszFilePath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(lpszFilePath);
	}

	HANDLE hFileHandle = NULL;
	hFileHandle = CreateFile(lpszFilePath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFileHandle == INVALID_HANDLE_VALUE)
	{
		bReturn = false;
		goto CLEANUP;
	}

	//write tree values in file.
	int iNumberOfItemWritten = 0;;
	if (FileWrite(hFileHandle, inorder, &iNumberOfItemWritten) == FAILURE)
	{
		bReturn = false;
		goto CLEANUP;
	}

	//Need to close file handle
	if (hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFileHandle);
		hFileHandle = INVALID_HANDLE_VALUE;
	}

	//Calculate hre file checksum
	DWORD	dwFileOriginalChecksum = 0x00;
	DWORD	dwFileComputedChecksum = 0x00;

	MapFileAndCheckSum(lpszFilePath, &dwFileOriginalChecksum, &dwFileComputedChecksum);
	DWORD dwOut = AddCheckSum(lpszFilePath, dwFileComputedChecksum);
	if (dwOut)
	{
		bReturn = false;
		goto CLEANUP;
	}

	bReturn = true;

CLEANUP:
	//Need to close file handle
	if (hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFileHandle);
		hFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : CheckFileIntegrityBeforeDecryption()
*  Description    : Checks file for modified. If file is not modified, returns 0 else 1
*  Author Name    : Vilas
*  Date			  :	09-July-2015
*  SR No		  :
****************************************************************************************************/
BALBSTCLSTMPLT
DWORD CWWizBalBstT::FileIntegrityCheck(LPTSTR lpszFilePath)
{
	HANDLE			hFile = NULL;
	DWORD			dwRet = 0x00;
	DWORD64			dwSize = 0x00;
	LARGE_INTEGER	lpFileSize = { 0 };
	DWORD			dwStoredChecksum = 0x00;
	DWORD			dwReadBytes = 0x00;
	LARGE_INTEGER	liDistToMove = { 0 };
	DWORD			dwFileOriginalChecksum = 0x00;
	DWORD			dwFileComputedChecksum = 0x00;

	try
	{
		hFile = CreateFile(lpszFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			dwRet = 0x01;
			goto CLEANUP;
		}

		GetFileSizeEx(hFile, &lpFileSize);
		dwSize = lpFileSize.QuadPart;

		if ((!dwSize) || (dwSize == 0xFFFFFFFF))
		{
			dwRet = 0x02;
			goto CLEANUP;
		}

		liDistToMove.QuadPart = lpFileSize.QuadPart - sizeof(DWORD);

		SetFilePointerEx(hFile, liDistToMove, NULL, FILE_BEGIN);
		ReadFile(hFile, &dwStoredChecksum, sizeof(DWORD), &dwReadBytes, NULL);

		SetFilePointerEx(hFile, liDistToMove, NULL, FILE_BEGIN);

		SetEndOfFile(hFile);
		CloseHandle(hFile);
		hFile = NULL;

		MapFileAndCheckSum(lpszFilePath, &dwFileOriginalChecksum, &dwFileComputedChecksum);

		DWORD dwOut = 0;
		if (dwFileComputedChecksum != dwStoredChecksum)
		{
			dwRet = 0x03;
			goto CLEANUP;
		}
	
		dwOut = AddCheckSum(lpszFilePath, dwFileComputedChecksum);
		if (dwOut)
		{
			dwRet = 0x04;
			goto CLEANUP;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::FileIntegrityCheck, File: [%s]", lpszFilePath, 0, true, SECONDLEVEL);
	}
CLEANUP:
	if (hFile != NULL)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	return dwRet;
}


/***************************************************************************************************
*  Function Name  : print
*  Description    : Member function taking two arguments and returning an error code.
*					Function traverses the tree and prints the objects present in tree using the specified print function.
*					Three traversal ways implemented [inorder, postorder, preorder].
*					@param[in]  eTraversalMethod: enum traversal type [inOrder, postOrder, preOrder].
*					@param[out] pNrOfItemsPrinted: int Nr of objects in tree.
*					@return     SUCCESS when no error encountered.
*  Author Name    : Ram Shelke
*  Date			  :	06-Apr-2016
/**************************************************************************************************/
BALBSTCLSTMPLT
int CWWizBalBstT::FileWrite(HANDLE hHandle, eTreeTraversal eTraversalMethod, int *pNrOfItemsPrinted) const
{
	int iReturnCode = FAILURE;
	__try
	{
		if (hHandle == NULL)
		{
			return iReturnCode;
		}

		CNodeT* (CWWizBalBstT::*pTraversalFunc)(CNodeT* pNode, T* pKey) const = 0;
		CNodeT* pNodeIterator = 0;
		T tKey;
		DWORD iItemCnt = 0;
		*pNrOfItemsPrinted = 0;

		if (m_pRoot == 0)
		{
			iReturnCode = IS_EMPTY;
		}
		else
		{
			if (eTraversalMethod == inorder)
			{
				pNodeIterator = getMinNode(m_pRoot);
				tKey = *pNodeIterator->getSmallKey();
				pTraversalFunc = &CWWizBalBstT::getNextInorderKey;
			}
			else if (eTraversalMethod == preorder)
			{
				pNodeIterator = m_pRoot;
				tKey = *pNodeIterator->getSmallKey();
				pTraversalFunc = &CWWizBalBstT::getNextPreorderKey;
			}
			else if (eTraversalMethod == postorder)
			{
				pNodeIterator = getMinNode(m_pRoot);
				tKey = *pNodeIterator->getSmallKey();
				pTraversalFunc = &CWWizBalBstT::getNextPostorderKey;
			}

			iReturnCode = SUCCESS;
			
			DWORD dwCount = 0x00;
			do
			{
				//if Count is greater than allowed count then exit from the loop.
				if (iItemCnt > m_dwMaxCount)
					break;

				iItemCnt++;
				DWORD dwWritten = 0;
				BOOL bres = WriteFile(hHandle, &tKey, sizeof(T), &dwWritten, NULL);
				if (!bres)
				{
					iReturnCode = FAILURE;
					break;
				}

				if (pTraversalFunc != 0)
				{
					pNodeIterator = (this->*pTraversalFunc)(pNodeIterator, &tKey);
				}
			} while (pNodeIterator != 0);
		}
		*pNrOfItemsPrinted = iItemCnt;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumBalBstT::print", 0, 0, true, SECONDLEVEL);
	}
	return iReturnCode;
}

/***************************************************************************************************
*  Function Name  : AddCheckSum()
*  Description    : it uses to add check sum byte to encrypted file at end of file
*  Author Name    : Vilas/Lalit
*  Date			  :	09-July-2015
*  SR No		  :
****************************************************************************************************/
BALBSTCLSTMPLT
DWORD CWWizBalBstT::AddCheckSum(LPCTSTR lpFileName, DWORD dwByteNeedtoAdd)
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
		AddLogEntry(L"### Exception in CVibraniumCrypter::AddCheckSum", 0, 0, true, SECONDLEVEL);
	}

	return dwRet;
}