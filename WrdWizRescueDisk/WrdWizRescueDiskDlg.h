/*********************************************************************
*  Program Name		: WrdWizRescueDiskDlg.h
*  Description		: CWrdWizRescueDiskDlg Implementation.
*  Author Name		: NITIN SHELAR
*  Date Of Creation	: 26 feb 2019
**********************************************************************/

#pragma once

#include <winioctl.h>
#include <winerror.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <imapi2fs.h>
#include <imapi2.h>
#include <atlbase.h>
#include <atlcom.h>
#include <afxctl.h>
#include "DiskInfo.h"
#include "PipeConstants.h"
#include "iTINRegWrapper.h"
#include "ISpyCommunicator.h"

/***************************************************************************************************
*  Class Name	  : CSciterBase
*  Description    : Sciterbase class for sciter DOM event handling
*  Author Name    :	NITIN SHELAR
*  SR_NO		  :
*  Date			  : 11/02/2019
****************************************************************************************************/
class CSciterBase :
	public sciter::event_handler           // Sciter DOM event handling
{

};

/***************************************************************************************************
*  Class Name	  : CWrdWizRescueDiskDlg
*  Description    : CWrdWizRescueDiskDlg class for sciter DOM event, function map and method declaration.
*  Author Name    :	NITIN SHELAR
*  SR_NO		  :
*  Date			  : 11/02/2019
****************************************************************************************************/
class CWrdWizRescueDiskDlg : public CDialogEx,
	public CSciterBase,
	sciter::behavior_factory,
	public sciter::host <CWrdWizRescueDiskDlg>
{
// Construction
public:
	CWrdWizRescueDiskDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WRDWIZRESCUEDISK_DIALOG };

	~CWrdWizRescueDiskDlg();

	virtual bool subscription(HELEMENT he, UINT& event_groups)
	{
		event_groups = UINT(-1);
		return true;
	}
	// the only behavior_factory method:
	virtual event_handler* create(HELEMENT he) { return this; }

	virtual void attached(HELEMENT he) {
		self = he;
	}
	virtual void detached(HELEMENT he) {
		self = NULL;
	}

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnImapiUpdate(WPARAM, LPARAM);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedCancel();

public:
	BEGIN_FUNCTION_MAP
		FUNCTION_0("OnClose", On_Close)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnMinimize", On_Minimize)
		FUNCTION_3("onModalLoop", onModalLoop)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
		FUNCTION_2("OnGetDriveList", On_GetDriveList)
		FUNCTION_0("OnGetIsoFile", On_GetIsoFile)
		FUNCTION_3("OnGetResourceData", On_GetResourceData)
		FUNCTION_3("OnStartCDThread", On_StartCDThread)
		FUNCTION_0("FunCheckInternetAccessBlock", FunCheckInternetAccessBlock)
	END_FUNCTION_MAP

	json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);
	json::value On_GetResourceData(SCITER_VALUE svIsoFilePath, SCITER_VALUE svDrivePath, SCITER_VALUE svStatus);
	json::value On_StartCDThread(SCITER_VALUE svIsoFilePath, SCITER_VALUE svDrivePath, SCITER_VALUE svStatus);
	json::value On_GetThemeID();
	json::value On_Minimize();
	json::value On_GetProductID();
	json::value On_GetLanguageID();
	json::value On_Close();
	json::value OnGetAppPath();
	json::value On_GetDriveList(SCITER_VALUE svDriveList, SCITER_VALUE svDriveType);
	json::value On_GetIsoFile();
	json::value FunCheckInternetAccessBlock();

public:
	HWINDOW   get_hwnd();
	HINSTANCE get_resource_instance();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	
	void FunFreeReource(HANDLE hThread);
	void GetCheckZipReg();
	void UpdateProgressEvent();
	void FunUIMessages(int dwFlagVal);
	HRESULT CreateStreamISO(IDiscFormat2Data* IDataFormat2);
	HRESULT GetExtractData();

	int64_t WriteBootSectors(HANDLE hDrive, uint64_t SectorSize, uint64_t StartSector, uint64_t nSectors, const void *pBuf);
	int64_t ReadSectors(HANDLE hDrive, uint64_t SectorSize, uint64_t StartSector, uint64_t nSectors, void *pBuf);
	
	int GetDrivePartitionData(DWORD DriveIndex, char* FileSystemName, DWORD FileSystemNameSize);
	int WriteData(FILE *fp, uint64_t Position, const void *pData, uint64_t Len);
	int WriteZeroMBR(FILE *fp);
	int WriteWinMBR(FILE *fp);

	char* GetPhysicalName(DWORD DriveIndex);
	char* GetLogicalName(DWORD DriveIndex, BOOL bKeepTrailingBackslash, BOOL bSilent);

	HANDLE GetPhysicalDriveHandle(DWORD DriveIndex, BOOL bWriteAccess, BOOL bLockDrive);
	HANDLE GetGenericHandle(char* szPath, BOOL bWriteAccess, BOOL bLockDrive);
	HANDLE GetLogicalDriveHandle(DWORD DriveIndex, BOOL bWriteAccess, BOOL bLockDrive);

	HRESULT GetDiscRecorder(__in ULONG index, __out IDiscRecorder2 ** recorder);
	HRESULT ListAllRecorders();

	BOOL GetExtractIso();
	BOOL GetRMDrivesList();
	BOOL GetDriveIndex(int * iDriveIndex);
	BOOL WaitForLogical(DWORD DriveIndex);
	BOOL UnmountVolume(HANDLE hDrive);
	BOOL MountVolume(char* drive_name, char *drive_guid);
	BOOL WriteNTFSBootR(HANDLE hLogicalVolume);
	BOOL FormatDrive(DWORD DriveIndex);
	BOOL WriteMBR(HANDLE hPhysicalDrive);
	BOOL FlushDrive(char drive_letter);
	BOOL RemountVolume(char* drive_name);

	BOOL GetDrivePathCheck(CString csDriveName);
	IMAPI_MEDIA_PHYSICAL_TYPE MediaValue;

	inline void SetHwnd(HWND hWnd){ m_hNotifyWnd = hWnd; }

	static CWrdWizRescueDiskDlg* CreateEventSink();
	bool ConnectDiscFormatData(IDiscFormat2Data* IDsikObj);
	void ConnectionUnadvise();
public:
	CString							m_csDrivePath;
	CString							m_csIsoFilePath;
	HANDLE							m_hFormatThread;
	HANDLE							m_hFormatCDThread;
	DWORD							FormatStatus;
	DWORD							m_dwPercentVal;
	BOOL							m_bFlagProgress;

	SCITER_VALUE					m_svAddDriveEntry;
	SCITER_VALUE					m_status;
	SCITER_VALUE					m_svDiskType;
	
	HELEMENT						self;
	static CWrdWizRescueDiskDlg		*m_pThis;
	sciter::dom::element			m_root_el;
	RM_DRIVE_INFO					SelectedDrive;

	LPTYPEINFO  m_ptinfo;
	DWORD		m_dwCookie;
	LPUNKNOWN	m_pUnkSink;
	LPUNKNOWN	m_pUnkSrc;
	HWND		m_hNotifyWnd;

	IDiscRecorder2*     m_IDiskRecorder2;
	
public:
	
	DECLARE_INTERFACE_MAP()
	BEGIN_INTERFACE_PART(FormatDataEvents, DDiscFormat2DataEvents)
		
		STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);
		STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
		STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames,UINT cNames, LCID lcid, DISPID FAR* rgdispid);
		STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, 
			WORD wFlags, DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,
			EXCEPINFO FAR* pexcepinfo, UINT FAR* puArgErr);
		STDMETHOD_(HRESULT, Update)(LPDISPATCH, LPDISPATCH);
	END_INTERFACE_PART(FormatDataEvents)
	
}; 
