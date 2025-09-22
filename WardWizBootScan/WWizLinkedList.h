/****************************************************************************
Program Name          : WWizLinkedList.h
Description           : This file contains implementation of dynamic doubly linked
						list which stores structure in nodes.
Author Name			  : Ram Shelke
Date Of Creation      : 05th Oct 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#pragma once

#include "Header.h"
#include "CommonFunctions.h"

/* self-referential structure */
struct listNode {   
	BYTE data[0x04];
	struct listNode *nextPtr;
	struct listNode *prevPtr;
	
};

/* structure for Exclude File Path*/
struct ExlcudeListNode {
	CHAR szPath[MAX_FILEPATH_LENGTH];
	BYTE byISFolder;
	struct ExlcudeListNode *nextPtr;
	struct ExlcudeListNode *prevPtr;
};

/* structure for Exclude Extension */
struct ExlcudeExtListNode {
	CHAR szExtension[0x32];
	struct ExlcudeExtListNode *nextPtr;
	struct ExlcudeExtListNode *prevPtr;
};

/* structure for Exclude Extension */
struct ScanByNameListNode {
	CHAR szScanByName[0x96];
	struct ScanByNameListNode *nextPtr;
	struct ScanByNameListNode *prevPtr;
};

typedef struct listNode ListNode;
typedef ListNode *ListNodePtr;

typedef struct ExlcudeListNode ExlcudeLSTNode;
typedef ExlcudeListNode *ExlcudeLSTNodePtr;

typedef struct ExlcudeExtListNode ExlcudeExtLSTNode;
typedef ExlcudeExtLSTNode *ExlcudeExtLSTNodePtr;

typedef struct ScanByNameListNode ScanByNameLSTNode;
typedef ScanByNameLSTNode *ScanByNameLSTNodePtr;

void Insert(ListNode** head_ref, LPBYTE value, DWORD dwSize);
void Insert(ExlcudeLSTNode** head_ref, LPSTR value, DWORD dwSize, BYTE byISSub);
void Insert(ExlcudeExtLSTNode** head_ref, LPSTR value, DWORD dwSize);
bool FindPosition(ListNode *node, LPBYTE byMD5Buff, DWORD dwBufLen, DWORD &dwIndex, DWORD &dwVirusID);
bool InsertItem(LPBYTE value, DWORD dwSize);
bool InsertItem(LPSTR value, DWORD dwSize, BYTE byISSub);
bool FindPosition(LPBYTE byMD5Buff, DWORD dwBufLen, DWORD &dwIndex, DWORD &dwVirusID);
bool Find(ExlcudeLSTNode *node, LPSTR value, DWORD dwBufLen, bool &bISSubFolderExcluded);
bool Find(LPSTR value, DWORD dwBufLen, bool &bISSubFolderExcluded);
bool RemoveAll(ListNode *node);
bool RemoveAll(ExlcudeLSTNode *node);
bool UnInitializeTree();
bool UnInitializeExcludeTree();
bool ISSubFilePath(LPSTR lpString, LPSTR lpSearchString);
bool InsertFileExtItem(LPSTR value, DWORD dwSize);
bool Find(LPSTR value, DWORD dwBufLen);
bool FindExt(ExlcudeExtLSTNode *node, LPSTR value, DWORD dwBufLen);
bool InsertScanByNameItem(LPSTR value, DWORD dwSize);
void Insert(ScanByNameListNode** head_ref, LPSTR value, DWORD dwSize);
bool FindScanByName(LPSTR value, DWORD dwBufLen);
bool FindScanByName(ScanByNameLSTNode *node, LPSTR value, DWORD dwBufLen);
bool FreeHeapAllocTreeMemory();
bool RemoveAll(ExlcudeExtListNode *node);
bool RemoveAll(ScanByNameListNode *node);

extern HANDLE					m_hHeapMem;
extern ListNodePtr				m_pRoot;
extern ExlcudeLSTNodePtr		m_pExcludeDBRoot;

