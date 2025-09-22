/****************************************************
*  Program Name: CRegistrationDlg.h
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 27th May 2016
*  Version No: 2.0.0.1
****************************************************/
#pragma once

#include "JpegDialog.h"
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "PictureEx.h"
#include "RegistrationSecondDlg.h"
#include "sciter-x-threads.h"

#define HTTP_RESOLVE_TIMEOUT		1000
#define HTTP_CONNECT_TIMEOUT		2000
#define HTTP_SEND_TIMEOUT			2000
#define HTTP_RECEIVE_TIMEOUT		3000

typedef DWORD(*GETREGISTRATIONDATA)	(LPBYTE, DWORD &, DWORD dwResType, TCHAR *pResName);
typedef DWORD(*GETINSTALLATIONCODE)(LPCTSTR lpSerialNumber, LPCTSTR lpMID, LPTSTR lpInstallationCode);
typedef DWORD(*VALIDATERESPONSE)(LPTSTR lpActivationCode, BYTE bProductID, SYSTEMTIME &ServetTime, WORD &wwDaysLeft);

class CSciterBase :
	public sciter::event_handler           // Sciter DOM event handling
{

};
class CRegistrationDlg : public CJpegDialog,
	public CSciterBase,
	public sciter::host<CRegistrationDlg> // Sciter host window primitives
{
	DECLARE_DYNAMIC(CRegistrationDlg)

public:
	CRegistrationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRegistrationDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_REGISTRATION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnBnClickedButtonRegistrationNext();
	afx_msg void OnBnClickedButtonRegistrationCancel();
	afx_msg void OnBnClickedButtonClose();
	void ShowHideRegistration(bool bEnable);
	bool GetRegistryValue( CString csValue, TCHAR * pszBIOSSerialNumber);
	afx_msg void OnBnClickedButtonMinimize();
	afx_msg void OnBnClickedCheckNewsletter();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	bool ValidationOfRegistrationForm();
	bool ValidateTitle();
	bool ValidateFirstName();
	bool ValidateLastName();
	bool ValidateTelephone();
	bool ValidateEmailID();
	bool ValidateRegistrationKey();
	void ReadRegistryOfNewsletter();
	DWORD GetRegistrationDateFromServer( LPSYSTEMTIME lpServerTime ) ;
	DWORD SendUserRegistrationDataToServer(TCHAR *pRegData, DWORD dwRegSize, LPSYSTEMTIME lpServerTime ) ;
	DWORD GetRegistrationDataFromRegistry( ) ;
	//DWORD AddRegistrationDataInRegistry() ;
	DWORD DecryptData( LPBYTE lpBuffer, DWORD dwSize ) ;
	DWORD GetCPUID( TCHAR *pszCPUID ) ;
	DWORD GetBIOSSerialNumber(TCHAR *pszBIOSSerialNumber ) ;
	DWORD RemoveCharsIfExists(TCHAR *pszBIOSSerialNumber, int iLen, int iSize, TCHAR chRemove ) ;
	BOOL RefreshString();
	void SetRegistrationHeader(CString csHeader);
	bool CheckSpecialCharInEmailID(CString csEmailID);
	bool HandleCloseButton();
	void ProductRenewalDetails();
	bool ConfirmEmailID();
public:
	HCURSOR			m_hButtonCursor;
	CPictureEx		m_stPreloaderImage;
	CColorStatic	m_stFirstName;
	CColorStatic	m_stTitle;
	CColorStatic	m_stLastName;
	CColorStatic	m_stEmailID;
	CColorStatic	m_stEmailIdStar;
	CColorStatic	m_stTelephoneNo;
	CColorStatic	m_stRequiredFields;
	CColorStatic	m_stPersonalInformation;
	CColorStatic	m_stRegistrationKey;
	CColorStatic	m_stNewLetterText;
	CColorStatic	m_stConnectToServer;
	CColorStatic	m_stSuccessMsg;
	CColorStatic	m_stRequestText;
	CColorStatic	m_stFailedText;
	CComboBox		m_comboTitle;
	CColorStatic	m_stForKeyExample;
	CButton			m_chkNewsLetter;
	CxSkinButton	m_btnRegistrationNext;
	CxSkinButton	m_btnMinimize;
	CxSkinButton	m_btnClose;
	CxSkinButton	m_btnRegistartionCancel;
	CEdit			m_edtForTitle;
	CEdit			m_edtFirstName;
	CEdit			m_edtEmailID;
	CEdit			m_edtRegistrationKey;
	CEdit			m_edtLastName;
	CEdit			m_edtTelephoneNo;
	CStatic			m_stPicCorrect;
	CBitmap			m_bmpPicCorrect;
	CStatic			m_stFailedPic;
	CBitmap			m_bmpPicFailed;
	CFont*			m_pBoldFont;
	CFont*			m_pNoteFont;
	CFont*			m_pTextFont;
	BOOL			m_bNewsLetter;
	bool            m_bIncorrectEmail;	
	TCHAR			m_szBIOSNumber[100];
	CColorStatic	m_stUserText;
	CStatic			m_stRegHeader;
	HBITMAP			m_bmpRegHeader;
	CColorStatic	m_stRegHeaderText;
	HBITMAP			m_bmpLogo;
	CStatic			m_stLogo;
	CColorStatic m_stRegistrationRights;
	CRegistrationSecondDlg m_objRegistrationSecondDlg;	
	CFont				 	m_BoldText;
	afx_msg void OnPaint();
	CColorStatic	 m_stContactStar;
	bool			 m_bIncorrectContact;
	bool			 m_bInvalidConact;
	bool			 m_bConfirmEmail;
	afx_msg void OnClose();
	CEdit m_edtReEnterEmailID;
	CColorStatic m_stConfirmEmail;
	CColorStatic m_stConfirmEmailStar;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CStatic m_stSplitImages;
	HBITMAP m_bmpSplitImage;

public:
	sciter::dom::element root_el;

	AVACTIVATIONINFO		m_ActInfo;

	HANDLE					m_hOfflineNextEvent;
	HANDLE					m_hSendRequestThread;

	HMODULE					m_hOfflineDLL;
	HMODULE					m_RegisterationDLL;

	bool					m_bTryProduct;
	bool					m_bActiveProduct;
	bool					m_bOnlineActivation;
	bool					m_bOfflineActivation;
	bool					m_bisThreadCompleted;

	DWORD					dwDaysRemain;

	TCHAR					m_szInstallCode[32];
	TCHAR					m_szMachineIDValue[0x80];
	TCHAR					m_szOfflineActivationCode[23];
	TCHAR					m_szDealerCode[64];
	TCHAR					m_szReferenceID[64];
	TCHAR					m_szCountry[512];
	TCHAR					m_szState[512];
	TCHAR					m_szCity[512];
	TCHAR					m_szPinCode[512];
	TCHAR					m_szEngineerName[512];
	TCHAR					m_szEngineerMobNo[512];
	TCHAR					m_szOSName[512];
	TCHAR					m_szRamSize[512];
	TCHAR					m_szHDSize[512];
	TCHAR					m_szProcessor[512];
	TCHAR					m_szCompName[512];
	TCHAR					m_szUserName[512];

	HWINDOW   get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	void HideAllElements();
	void DisplayFailureMessage(REGFAILUREMSGS dwRes);
	void PerformRegistration(SCITER_VALUE svArrUserDetails);
	void DisplayFailureMsgOnUI(CString csMessageType, CString strMessage);
	void DisplayFailureMessageForOfflineRegistration(REGFAILUREMSGS dwRes);

	bool CheckForMIDInRegistry();
	bool LoadWrdWizOfflineRegDll();
	bool ReadHostFileForWWRedirection();
	bool GetDomainName(LPTSTR pszDomainName, DWORD dwSize);
	bool CheckForMachineID(const AVACTIVATIONINFO	&actInfo);
	bool CopyFileToDestination(TCHAR *pszSource, TCHAR *pszDest);
	bool SendRegistrationInfo2Service(int iMessageInfo, DWORD dwType = 0, bool bEmailPluginWait = false);
	bool SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPBYTE byData, DWORD dwLen, bool bWait);
	bool CopyRegistrationFilesUsingService(DWORD dwType, CString csSrcFilePath, CString csDestFilePath, DWORD dwValue, bool bWait);
	bool SendRegisteredData2Service(DWORD dwType, LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName, bool bRegWait);

	CString GetModuleFilePath();

	DWORD GetRegisteredUserInfo(DWORD dwProdID);
	DWORD AddRegistrationDataInFile();
	DWORD SpreadRegistrationFilesInSystem();
	DWORD GetRegistrationDatafromFile();
	DWORD GetRegistrationDatafromFile(CString strUserRegFile);
	DWORD GetRegistrationDateFromServer(LPSYSTEMTIME lpServerTime, LPDWORD lpDaysRemaining);
	DWORD ExtractDate(TCHAR *pTime, LPSYSTEMTIME lpServerTime, LPDWORD lpdwServerDays);
	DWORD AddRegistrationDataInRegistry();	
	void AddProdRegInfoToLocal(DWORD m_dwType, AVACTIVATIONINFO	ActInfo , DWORD m_dwResSize, DWORD m_dwResType, TCHAR *m_pResName, bool m_bRegWait);
	void SystemInfoDetails();

public:
	SCITER_VALUE					m_svFunSetRegStatusCB;
	SCITER_VALUE					m_svFunSetUserDetailsCB;
	SCITER_VALUE					m_svFunSetOfflineActKeyCB;
	SCITER_VALUE					m_svFunSetNoInternetMsg;
	SCITER_VALUE					m_svSetSystemInfoCB;
	bool							bCheckCodeValue;
	CString							m_csServer;
	CString							m_csUserName;
	CString							m_csPassword;
	CString							m_csResponseData;

public:
	BEGIN_FUNCTION_MAP
		//Main DLG functions
		FUNCTION_0("OnClose", On_Close) // On_Close()
		
		//Registration related Functions
		FUNCTION_1("GetRegisteredUserDetails", GetRegisteredUserDetails)
		FUNCTION_4("OnClickTrialProduct", On_ClickTrialProduct)
		FUNCTION_4("OnClickActivateProduct", On_ClickActivateProduct)
		FUNCTION_3("GetOfflineActivationKey", GetOfflineActivationKey)
		FUNCTION_2("OnClickOfflineRegistration", On_ClickOfflineRegistration)		
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_3("onModalLoop", onModalLoop)
		FUNCTION_1("SetCheckForDealerRefCode", SetCheckForDealerRefCode)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
		FUNCTION_0("GetDBPath", GetDBPath)
		FUNCTION_0("FunGetMachineId", FunGetMachineId)
		FUNCTION_0("FunFunGetUUID", FunGetUUID)
		FUNCTION_0("GetInternetConnection", On_GetInternetConnection)
		FUNCTION_0("OnGetProxyRegistry", On_GetProxyRegistry)
		FUNCTION_0("GetHostFileForWWRedirectionFlag", GetHostFileForWWRedirectionFlag)
		FUNCTION_0("CheckForProductRenewal", CheckForProductRenewal)
		FUNCTION_1("GetSystemInformation", GetSystemInformation)
		FUNCTION_0("FunCheckInternetAccessBlock", FunCheckInternetAccessBlock)
	END_FUNCTION_MAP

public:
	//Main DLG functions
	json::value On_Close();

	//Registration related Functions
	json::value GetRegisteredUserDetails(SCITER_VALUE svFunSetUserDetailsCB);
	json::value On_ClickTrialProduct(SCITER_VALUE svArrUserDetails, SCITER_VALUE svFunSetRegStatusCB, SCITER_VALUE svFunSetNoInternetMsg, SCITER_VALUE svResponse);
	json::value On_ClickActivateProduct(SCITER_VALUE svArrUserDetails, SCITER_VALUE svFunSetRegStatusCB, SCITER_VALUE svFunSetNoInternetMsg, SCITER_VALUE svResponse);
	json::value GetOfflineActivationKey(SCITER_VALUE svArrUserDetails, SCITER_VALUE svFunSetOfflineActKeyCB, SCITER_VALUE svFunSetNoInternetMsg);
	json::value On_ClickOfflineRegistration(SCITER_VALUE svOfflineActKey, SCITER_VALUE svFunSetRegStatusCB);
	json::value On_GetProductID();
	json::value OnGetAppPath();
	json::value On_GetLanguageID();
	json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);
	json::value SetCheckForDealerRefCode(SCITER_VALUE svCheckDealerReferralCode);
	json::value On_GetThemeID();
	json::value GetDBPath();
	json::value FunGetMachineId();
	json::value FunGetUUID();
	json::value On_GetInternetConnection();
	json::value On_GetProxyRegistry();
	json::value GetHostFileForWWRedirectionFlag();
	json::value CheckForProductRenewal();
	json::value GetSystemInformation(SCITER_VALUE svSetSystemInfoCB);
	json::value FunCheckInternetAccessBlock();
	void SendRegInfo2Server();

	DWORD GetProxySettingsFromRegistry();
	bool GetProxyServerDetailsFromDB();
	CString GenerateGUIID();
};
