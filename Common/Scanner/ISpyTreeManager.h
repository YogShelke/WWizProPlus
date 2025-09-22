#pragma once
#include "TreeConstants.h"
#include "WrdwizEncDecManager.h"

class CISpyTreeManager
{
public:
	CISpyTreeManager(void);
	virtual ~CISpyTreeManager(void);
public:
	//Our vector of data found
	typedef std::vector<std::string> MatchedStringFoundVect;
public:
	bool AddString(char * szSignature);
	bool AddStringSEH(char * szSignature);
	bool ClearTree();
	bool DeleteNode(Node* pNode)const;
	bool DeleteNodeSEH(Node* pNode)const;
	bool BuildTreeIndex();
	bool BuildIndex(unsigned char * szSig, Node* pNode);
	Node* SearchNode(unsigned char * szString, Node* pNode);
	bool ScanBuffer(unsigned char *szBuffer, unsigned int iBufferLen, char * pszVirusName, DWORD &dwSpyID, bool bRescan = false);
	bool ScanBufferSEH(unsigned char *szBuffer, unsigned int iBufferLen, char * pszVirusName, DWORD &dwSpyID, bool bRescan);
	int  GetMatchedSignatureCount(MatchedStringFoundVect &vtMatchedString);
	bool IsPartsMatched(MatchedStringFoundVect &vtMatchedString, char * pszVirusName, DWORD &dwSpyID, bool bRescan = false);
	bool SetPESignatureFilePath(LPTSTR pszFilePath);
	bool CopyDataByRef(CDataManager *objrdwizEncDecManager);
	bool IsReScanEntry(char * pszVirusName, DWORD dwSpyID);
public:
	//Our root node
	Node 				m_aRoot;
	TCHAR 				m_szDBFilePath[MAX_PATH];
	CDataManager        *m_objWrdwizEncDecManager;
	bool				m_bReScan;
};
