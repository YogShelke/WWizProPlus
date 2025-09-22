#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "stdafx.h"

typedef struct __tagstructMD5Data
{
	BYTE byMD5Hex[0x04];
}STRUCTMD5Data;

typedef DWORD(*GetByteStringHashFunc)(LPBYTE, DWORD, LPBYTE);

class CMD5ScanDLLApp : public CWinApp
{
public:
	CMD5ScanDLLApp();
	~CMD5ScanDLLApp();
// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	bool ScanForMD5DataSEH(LPBYTE lpBuffer, DWORD dwBufferSize, DWORD &dwVirusID, bool Rescan, FILETYPE filetype);
	DWORD CalculateMD5(LPBYTE lpBuffer, DWORD dwBufferSize, LPBYTE lpMD5Buffer);
	void LoadReqdLibrary();
	void UnloadLibrary();
	bool LoadMD5DatabaseSEH(FILETYPE filetype, DWORD &dwSigCount);
	bool UnLoadSignatures();
	bool FindPosition(vector<STRUCTMD5Data> & m_vMD5Collection, BYTE *MD5Data, DWORD & dwIndex, DWORD & dwVirusID);
	bool LoadContaintFromFile(LPTSTR szFilePath, FILETYPE filetype, DWORD dwVersionLength, DWORD &dwSigCount);
	bool LoadContaintFromFileSEH(LPTSTR szFilePath, FILETYPE filetype, DWORD dwVersionLength, DWORD &dwSigCount);
	DWORD GetDBDwordName(FILETYPE filetype, DWORD dwIndex);
	bool MatchSecondarySig(FILETYPE filetype, LPBYTE lpFileBuffer, DWORD dwIndex, DWORD &dwVirusID);
	bool MatchElementUsingIndex(vector<STRUCTMD5Data> & vMD5Collection, BYTE *byMD5Buffer, BYTE *byDBSigBuffer, DWORD dwIndex, DWORD & dwVirusID);
	DWORD GetActualFileOffsetFromIndex(FILETYPE filetype, DWORD dwIndex);
	void SetSigCountAsPerListID(FILETYPE filetype, DWORD dwListID, DWORD dwCount);
	DWORD GetSigCountAsPerListID(FILETYPE filetype, DWORD dwListID);
public:
	HINSTANCE					m_hInstLibrary;
	STRUCTMD5Data				m_stMD5Item;
	vector<STRUCTMD5Data>		m_vMD5PECollection;
	vector<STRUCTMD5Data>		m_vMD5JPEGCollection;
	vector<STRUCTMD5Data>		m_vMD5DOCCollection;
	vector<STRUCTMD5Data>		m_vMD5HTMLCollection;
	vector<STRUCTMD5Data>		m_vMD5XMLCollection;
	vector<STRUCTMD5Data>		m_vMD5PDFCollection;
	vector<STRUCTMD5Data>		m_vMD5PHPCollection;

	GetByteStringHashFunc		m_pGetbyteStrHash;
	DWORD						m_dwArrPESigCountPerDB[0x200];
	DWORD						m_dwArrJPGSigCountPerDB[0x32];
	DWORD						m_dwArrDOCSigCountPerDB[0x32];
	DWORD						m_dwArrHTMLSigCountPerDB[0x32];
	DWORD						m_dwArrXMLigCountPerDB[0x32];
	DWORD						m_dwArrPDFSigCountPerDB[0x32];
	DWORD						m_dwArrPHPSigCountPerDB[0x32];
	DWORD						m_dwProductID;

	DECLARE_MESSAGE_MAP()
};
