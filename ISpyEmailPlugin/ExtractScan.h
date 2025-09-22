#include "stdafx.h"
//#include <Windows.h>

#include "ISpyList.h"
#include "ISpyDataManager.h"
#include "iTinEmailContants.h"
#include "ISpyCommunicator.h"
#include "PipeConstants.h"


#ifndef __EXTRACT_H_
#define __EXTRACT_H_

struct VirusScanStruct
{
	CString	strEmailID ;
	CString	strSubject ;
	DWORD	strScanStatus ;
} ;

struct VirusFoundZip
{
	CString	strFilePath ;
	CString	strVirusName ;
	DWORD	dwRepairID ;
} ;


class CExtractAttchForScan
{
public:
	CExtractAttchForScan() ;
	~CExtractAttchForScan() ;

	bool bZipFileModified ;
	CString	strSenderMailID ;
	DWORD	dwRemovedZips ;

	bool GetWardWizPathFromRegistry( ) ;
	DWORD CreateFolder( TCHAR *pDestFol) ;
	void InitializeVariables() ;
	bool LoadDLLsANDSignatures() ;
	bool LoadScannerDLL() ;
	bool LoadSignatures(LPTSTR lpFilePath) ;
	bool UnLoadSignatures();
	void EnumZipFolderForScanning( TCHAR *pZipFol ) ;
	void ExtractZipForScanning( TCHAR *pZipFile ) ;
	void ScanZipFiles( TCHAR *pZipFile, bool &bModified, DWORD &dwCount ) ; ;

	DWORD RemoveFileOnly( TCHAR *pFile ) ;
	DWORD RemoveAllZipFiles( TCHAR *pFile ) ;
	DWORD RemoveFilesUsingSHFileOperation(TCHAR *pFolder ) ;

	bool AddToVirusScanEmailDB(CString &, CString & ) ;
	bool LoadVirusScanEmailDB() ;
	bool GetActionRegSetting(int &iValue);
	bool SendReLoadMessage2UI();
	bool SendEmailData2Service(DWORD dwAddEditDelType, DWORD dwType, CString csEntry,bool bVirusScanwait = false);
	bool SendEmailData2Service(DWORD dwAddEditDelType, DWORD dwType, CString csSendersAddress, CString csSubject, CString csStatus, bool bVirusScanwait);
	bool SendEmailData2Tray(int iMessage, CString csThreatName, CString csAttachmentName, CString csSenderAddr, DWORD dwAction,bool bWait = false);

	CStringArray	csaReAttachZipFiles ;
	CStringList		cslReAttachments ;
	CStringList		cslRemoveAttachments ;

protected:

	DWORD	dwVirusCount ;

	HMODULE	m_hModuleISpyScanDLL ;
	HMODULE	m_hModuleISpyRepairDLL ;
	HMODULE	m_hZip ;

	TCHAR	szModulePath[512] ;

	CString	strScanStatus ;
	CString	csVirusScanDBFile ;

	typedef std::map<DWORD, VirusScanStruct> VirusScanDataBase ;
	typedef std::map<DWORD, VirusFoundZip> VirusFoundZipDataBase ;
	

	VirusScanDataBase		VirusScanDB ;
	VirusFoundZipDataBase	VirusFoundDB ;

	typedef bool  (*LOADSIGNATURE)		(LPCTSTR);
	typedef bool  (*UNLOADSIGNATURES)	(void);
	typedef bool  (*SCANFILE)			(LPCTSTR, LPTSTR, DWORD&, bool);
	typedef bool  (*REPAIRFILE)			(LPCTSTR, DWORD);
	typedef DWORD (*UNZIPFILE )			(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount ) ;
	typedef DWORD (*MAKEZIPFILE)		(TCHAR *pFile, TCHAR *pZipPath ) ;

	LOADSIGNATURE		m_lpLoadSigProc;
	UNLOADSIGNATURES	m_lpUnLoadSigProc ;
	SCANFILE			m_lpScanFileProc ;
	REPAIRFILE			m_lpRepairFileProc ;
	UNZIPFILE			UnzipFile ;
	MAKEZIPFILE			MakeZipFile ;
public:
	bool				m_bSignaturesLoaded;
	CISpyCommunicator	m_objCom;
	FILETYPE GetFileType(LPCTSTR pszFilePath, DWORD &dwSubType, DWORD &dwVersion);
};

#endif