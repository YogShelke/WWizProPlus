/****************************************************************************
Program Name          : WWizLinkedList.cpp
Description           : This file contains implementation of dynamic doubly linked 
						list which stores structure in nodes.
Author Name			  : Ram Shelke
Date Of Creation      : 05th Oct 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#include "WWizLinkedList.h"

HANDLE						m_hHeapMem = NULL;
ListNodePtr					m_pRoot = NULL;
ListNodePtr					m_pLast = NULL;

ExlcudeLSTNodePtr			m_pExcludeDBRoot = NULL;
ExlcudeLSTNodePtr			m_pExcludeDBLast = NULL;

ExlcudeExtLSTNodePtr		m_pExcludeExtDBRoot = NULL;
ExlcudeExtLSTNodePtr		m_pExcludeExtDBLast = NULL;

ScanByNameLSTNodePtr		m_pScanByNameDBRoot = NULL;
ScanByNameLSTNodePtr		m_pScanByNameDBLast = NULL;


/***********************************************************************************************
*  Function Name  : Insert
*  Description    : Given a reference (pointer to pointer) to the head
					of a DLL and an int, appends a new node at the end.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
void Insert(ListNode** head_ref, LPBYTE value, DWORD dwSize)
{
	__try
	{
		/* 1. allocate node */
		ListNode* new_node = (ListNode*)kmalloc(m_hHeapMem, sizeof(ListNode));

		ListNode *last = *head_ref;  /* used in step 5*/

		/* 2. put in the data  */
		memcpy(new_node->data, value, dwSize);

		/* 3. Make next of new node as head and previous as NULL */
		new_node->nextPtr = (*head_ref);
		new_node->prevPtr = NULL;

		/* 4. change prev of head node to new node */
		if ((*head_ref) != NULL)
			(*head_ref)->prevPtr = new_node;

		/* 5. move the head to point to the new node */
		(*head_ref) = new_node;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in Insert");
	}
	return;
}


/***********************************************************************************************
*  Function Name  : Insert
*  Description    : Given a reference (pointer to pointer) to the head
					of a path and an int, appends a new node at the end.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
void Insert(ExlcudeLSTNode** head_ref, LPSTR value, DWORD dwSize, BYTE byISSub)
{
	__try
	{
		/* 1. allocate node */
		ExlcudeLSTNode* new_node = (ExlcudeLSTNode*)kmalloc(m_hHeapMem, sizeof(ExlcudeLSTNode));

		ExlcudeLSTNode *last = *head_ref;  /* used in step 5*/

		/* 2. put in the data  */
		memcpy(new_node->szPath, value, dwSize);
		new_node->byISFolder = byISSub;

		/* 3. Make next of new node as head and previous as NULL */
		new_node->nextPtr = (*head_ref);
		new_node->prevPtr = NULL;

		/* 4. change prev of head node to new node */
		if ((*head_ref) != NULL)
			(*head_ref)->prevPtr = new_node;

		/* 5. move the head to point to the new node */
		(*head_ref) = new_node;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in Insert");
	}
	return;
}


/***********************************************************************************************
*  Function Name  : Insert
*  Description    : Given a reference (pointer to pointer) to the head
of a path and an int, appends a new node at the end.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
void Insert(ExlcudeExtLSTNode** head_ref, LPSTR value, DWORD dwSize)
{
	__try
	{
		/* 1. allocate node */
		ExlcudeExtLSTNode* new_node = (ExlcudeExtLSTNode*)kmalloc(m_hHeapMem, sizeof(ExlcudeExtLSTNode));

		ExlcudeExtLSTNode *last = *head_ref;  /* used in step 5*/

		/* 2. put in the data  */
		memcpy(new_node->szExtension, value, dwSize);

		/* 3. Make next of new node as head and previous as NULL */
		new_node->nextPtr = (*head_ref);
		new_node->prevPtr = NULL;

		/* 4. change prev of head node to new node */
		if ((*head_ref) != NULL)
			(*head_ref)->prevPtr = new_node;

		/* 5. move the head to point to the new node */
		(*head_ref) = new_node;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in Insert");
	}
	return;
}




/***********************************************************************************************
*  Function Name  : Insert
*  Description    : Given a reference (pointer to pointer) to the head
of a path and an int, appends a new node at the end.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
void Insert(ScanByNameListNode** head_ref, LPSTR value, DWORD dwSize)
{
	//DbgPrint("\n >>> In Insert [%s]", value);

	__try
	{
		/* 1. allocate node */
		ScanByNameLSTNode* new_node = (ScanByNameLSTNode*)kmalloc(m_hHeapMem, sizeof(ScanByNameLSTNode));

		ScanByNameLSTNode *last = *head_ref;  /* used in step 5*/

		/* 2. put in the data  */
		memcpy(new_node->szScanByName, value, dwSize);

		/* 3. Make next of new node as head and previous as NULL */
		new_node->nextPtr = (*head_ref);
		new_node->prevPtr = NULL;

		/* 4. change prev of head node to new node */
		if ((*head_ref) != NULL)
			(*head_ref)->prevPtr = new_node;

		/* 5. move the head to point to the new node */
		(*head_ref) = new_node;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in Insert");
	}

	//DbgPrint("\n >>> return Insert [%s]", value);
	return;
}

/***********************************************************************************************
*  Function Name  : FindPosition
*  Description    : This function matches contents of linked list starting from the given node.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool FindPosition(ListNode *node, LPBYTE byMD5Buff, DWORD dwBufLen, DWORD &dwIndex, DWORD &dwVirusID)
{
	bool bReturn = false;
	__try
	{
		DWORD	dwSearchIndex	= 0x00;

		ListNode *first;
		first = node;

		DWORD iIndex = 0x00;
		while (first != NULL)
		{
			//Forward the list node from where we need to search next match.
			if (iIndex < dwIndex)
			{
				first = first->prevPtr;
				dwSearchIndex++;
				iIndex++;
				continue;
			}

			LPBYTE lpHash = first->data;
			if (lpHash)
			{
				if (lpHash[0x00] == byMD5Buff[0x00] &&
					lpHash[0x03] == byMD5Buff[0x03])
				{

					if (memcmp(lpHash, byMD5Buff, 0x04) == 0)
					{
						bReturn = true;
						dwVirusID = dwIndex = dwSearchIndex;
						break;
					}
				}
			}

			first = first->prevPtr;
			dwSearchIndex++;
			iIndex++;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in FindPosition");
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : Find
*  Description    : This function matches contents of linked list starting from the given node.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool Find(LPSTR value, DWORD dwBufLen, bool &bISSubFolderExcluded)
{
	bool bReturn = false;
	__try
	{
		bReturn = Find(m_pExcludeDBRoot, value, dwBufLen, bISSubFolderExcluded);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in Find");
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : Find
*  Description    : This function matches contents of linked list starting from the given node.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool Find(LPSTR value, DWORD dwBufLen)
{
	bool bReturn = false;
	__try
	{
		bReturn = FindExt(m_pExcludeExtDBRoot, value, dwBufLen);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in Find");
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : FindScanByName
*  Description    : This function matches contents of linked list starting from the given node.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool FindScanByName(LPSTR value, DWORD dwBufLen)
{
	bool bReturn = false;
	__try
	{
		bReturn = FindScanByName(m_pScanByNameDBRoot, value, dwBufLen);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in Find");
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : Find
*  Description    : This function matches contents of linked list starting from the given node.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool Find(ExlcudeLSTNode *node, LPSTR value, DWORD dwBufLen, bool &bISSubFolderExcluded)
{
	bool bReturn = false;
	__try
	{
		ExlcudeLSTNode *first;
		first = node;

		DWORD iIndex = 0x00;
		while (first != NULL)
		{
			LPSTR lpPath = first->szPath;

			if (lpPath)
			{
				if (first->byISFolder == '1')
				{
					if (ISSubFilePath(value, lpPath))
					{
						bReturn = true;
						bISSubFolderExcluded = true;
						break;
					}
				}
				else
				{
					DECLARE_UNICODE_STRING_SIZE(ulFilePath, MAX_FILEPATH_LENGTH);
					FillUnicodeStringWithAnsi(&ulFilePath, value);

					DECLARE_UNICODE_STRING_SIZE(ulPathInDB, MAX_FILEPATH_LENGTH);
					FillUnicodeStringWithAnsi(&ulPathInDB, lpPath);

					//Check here for Parent directory if its file
					if (!FolderExists(ulFilePath.Buffer) && FolderExists(ulPathInDB.Buffer))
					{
						char szDirPath[MAX_FILEPATH_LENGTH] = { 0 };
						strcpy(szDirPath, value);

						char *szDir = strrchr(szDirPath, '\\');
						szDir[0] = '\0';

						if (szDirPath != NULL)
						{
							if (strcmp(szDirPath, lpPath) == 0)
							{
								bReturn = true;
								break;
							}
						}
					}
					else
					{
						if (memcmp(lpPath, value, dwBufLen) == 0)
						{
							bReturn = true;
							break;
						}
					}
				}
			}
			first = first->prevPtr;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in Find");
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : Find
*  Description    : This function matches contents of linked list starting from the given node.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool FindScanByName(ScanByNameLSTNode *node, LPSTR value, DWORD dwBufLen)
{
	bool bReturn = false;
	__try
	{
		ScanByNameLSTNode *first;
		first = node;

		DWORD iIndex = 0x00;
		while (first != NULL)
		{
			LPSTR lpPath = first->szScanByName;

			if (lpPath)
			{
				DECLARE_UNICODE_STRING_SIZE(ulFilePath, 0x96 * 2);
				FillUnicodeStringWithAnsi(&ulFilePath, lpPath);

				DECLARE_UNICODE_STRING_SIZE(ulValue, 0x96 * 2);
				FillUnicodeStringWithAnsi(&ulValue, value);

				//DbgPrint("\n lpPath: [%S] | value: [%S] ", ulFilePath.Buffer, ulValue.Buffer);

				if (memcmp(lpPath, value, dwBufLen) == 0)
				{
					bReturn = true;
					break;
				}
			}
			first = first->prevPtr;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in FindScanByName");
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : Find
*  Description    : This function matches contents of linked list starting from the given node.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool FindExt(ExlcudeExtLSTNode *node, LPSTR value, DWORD dwBufLen)
{
	bool bReturn = false;
	__try
	{
		ExlcudeExtLSTNode *first;
		first = node;

		DWORD iIndex = 0x00;
		while (first != NULL)
		{
			LPSTR lpPath = first->szExtension;

			if (lpPath)
			{
				if (memcmp(lpPath, value, dwBufLen) == 0)
				{
					bReturn = true;
					break;
				}
			}
			first = first->prevPtr;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in FindExt");
	}
	return bReturn;
}


/***********************************************************************************************
*  Function Name  : FindPosition
*  Description    : This function call another FindPosition function which matches contents of linked list 
					starting from root node.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool FindPosition(LPBYTE byMD5Buff, DWORD dwBufLen, DWORD &dwIndex, DWORD &dwVirusID)
{
	__try
	{
		return FindPosition(m_pRoot, byMD5Buff, dwBufLen, dwIndex, dwVirusID);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in FindPosition");
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : InsertItem
*  Description    : This function inserts structure one by one.
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool InsertItem(LPBYTE value, DWORD dwSize)
{
	__try
	{
		Insert(&m_pLast, value, dwSize);

		static int iFirstNode = 0;
		if (iFirstNode == 0)
		{
			m_pRoot = m_pLast;
			iFirstNode++;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in InsertItem");
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : InsertItem
*  Description    : This function inserts structure one by one.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool InsertItem(LPSTR value, DWORD dwSize, BYTE byISSub)
{
	__try
	{
		if (!value)
			return false;

		AnsiToLower(value, (int)strlen(value));

		Insert(&m_pExcludeDBLast, value, dwSize, byISSub);

		static int iFirstExcludeDBNode = 0;
		if (iFirstExcludeDBNode == 0)
		{
			m_pExcludeDBRoot = m_pExcludeDBLast;
			iFirstExcludeDBNode++;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in InsertItem");
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : InsertItem
*  Description    : This function inserts structure one by one into exlcude extension DB
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 03-Oct-2018
*************************************************************************************************/
bool InsertFileExtItem(LPSTR value, DWORD dwSize)
{
	__try
	{
		if (!value)
			return false;

		AnsiToLower(value, (int)strlen(value));

		Insert(&m_pExcludeExtDBLast, value, dwSize);

		static int iExcludeExtDBNode = 0;
		if (iExcludeExtDBNode == 0)
		{
			m_pExcludeExtDBRoot = m_pExcludeExtDBLast;
			iExcludeExtDBNode++;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in InsertFileExtItem");
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : InsertScanByNameItem
*  Description    : This function inserts structure one by one into scan by name list.
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 03-Oct-2018
*************************************************************************************************/
bool InsertScanByNameItem(LPSTR value, DWORD dwSize)
{
	//DbgPrint("\n >>> In InsertScanByNameItem [%s]", value);
	__try
	{
		if (!value)
			return false;

		AnsiToLower(value, (int)strlen(value));

		//DbgPrint("\n >>> After AnsiToLower [%s]", value);

		Insert(&m_pScanByNameDBLast, value, dwSize);

		static int iFirstScanByNameNode = 0;
		if (iFirstScanByNameNode == 0)
		{
			m_pScanByNameDBRoot = m_pScanByNameDBLast;
			iFirstScanByNameNode++;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in InsertScanByNameItem");
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : UnInitializeTree
*  Description    : Function which deletes all node values/free's memory.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 13-Oct-2017
*************************************************************************************************/
bool UnInitializeTree()
{
	__try
	{
		RemoveAll(m_pRoot);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in UnInitializeTree");
		return false;
	}
	return true;
}


/***********************************************************************************************
*  Function Name  : FreeHeapAllocTreeMemory
*  Description    : Function which deletes all node values/free's memory.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 13-Oct-2017
*************************************************************************************************/
bool FreeHeapAllocTreeMemory()
{
	__try
	{
		RemoveAll(m_pExcludeDBRoot);
		RemoveAll(m_pExcludeExtDBRoot);
		RemoveAll(m_pScanByNameDBRoot);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in FreeHeapAllocTreeMemory");
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : UnInitializeExcludeTree
*  Description    : Function which deletes all node values/free's memory.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 13-Oct-2017
*************************************************************************************************/
bool UnInitializeExcludeTree()
{
	__try
	{
		RemoveAll(m_pExcludeDBRoot);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in UnInitializeTree");
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : RemoveAll
*  Description    : Function which deletes all node values/free's memory.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 13-Oct-2017
*************************************************************************************************/
bool RemoveAll(ListNode *node)
{
	__try
	{
		ListNode *first;
		first = node;
		while (first != NULL)
		{
			ListNode *toDelete = first;
			first = first->prevPtr;

			if (toDelete != NULL)
			{
				kfree(m_hHeapMem, toDelete);
				toDelete = NULL;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RemoveAll");
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : RemoveAll
*  Description    : Function which deletes all node values/free's memory.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 13-Oct-2017
*************************************************************************************************/
bool RemoveAll(ExlcudeLSTNode *node)
{
	__try
	{
		ExlcudeLSTNode *first;
		first = node;
		while (first != NULL)
		{
			ExlcudeLSTNode *toDelete = first;
			first = first->prevPtr;

			if (toDelete != NULL)
			{
				kfree(m_hHeapMem, toDelete);
				toDelete = NULL;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RemoveAll");
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : RemoveAll
*  Description    : Function which deletes all node values/free's memory.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 13-Oct-2017
*************************************************************************************************/
bool RemoveAll(ExlcudeExtListNode *node)
{
	__try
	{
		ExlcudeExtListNode *first;
		first = node;
		while (first != NULL)
		{
			ExlcudeExtListNode *toDelete = first;
			first = first->prevPtr;

			if (toDelete != NULL)
			{
				kfree(m_hHeapMem, toDelete);
				toDelete = NULL;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RemoveAll");
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : RemoveAll
*  Description    : Function which deletes all node values/free's memory.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 13-Oct-2017
*************************************************************************************************/
bool RemoveAll(ScanByNameListNode *node)
{
	__try
	{
		ScanByNameListNode *first;
		first = node;
		while (first != NULL)
		{
			ScanByNameListNode *toDelete = first;
			first = first->prevPtr;

			if (toDelete != NULL)
			{
				kfree(m_hHeapMem, toDelete);
				toDelete = NULL;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RemoveAll");
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ISSubFilePath
*  Description    : Function which checks for is the papth is sub file/folder path.
*  Author Name    : Ram Shelke
*  Date           : 26 Apr,2016
****************************************************************************************************/
bool ISSubFilePath(LPSTR lpString, LPSTR lpSearchString)
{
	bool bReturn = false;
	__try
	{
		//sanity check
		if (!lpString || !lpSearchString)
		{
			return bReturn;
		}

		//make here copy to tokenize
		char szString[MAX_FILEPATH_LENGTH] = { 0 };
		char szSearchString[MAX_PATH] = { 0 };
		strcpy(szString, lpString);
		strcpy(szSearchString, lpSearchString);

		//Tokenize buffer
		char seps[] = "\\";
		char *tokenFirst = NULL;
		char *tokenSecond = NULL;

		tokenFirst = xstrtok(szString, seps);
		tokenSecond = xstrtoktwo(szSearchString, seps);

		if (tokenFirst == NULL || tokenSecond == NULL)
		{
			return bReturn;
		}

		//check is search string token is exists.
		while (tokenSecond != NULL)
		{
			//if token not exists to compare means searchstring is shorter than actual string
			if (!tokenFirst)
			{
				bReturn = false;
				break;
			}
			
			//compare token here
			if (strcmp(tokenFirst, tokenSecond) != 0)
			{
				bReturn = false;
				break;
			}

			bReturn = true;

			tokenFirst = xstrtok(NULL, seps);
			tokenSecond = xstrtoktwo(NULL, seps);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		RtlAddLogEntry(SECONDLEVEL, L"### ### Exception in ISSubFilePath");
	}
	return bReturn;
}