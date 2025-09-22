#pragma once
#include "Resource.h"
#include "JpegDialog.h"
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "AVRegInfo.h"
#include "RegistrationThirdDlg.h"
#include "RegistrationForthDlg.h"

class CRegistrationSecondDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CRegistrationSecondDlg)

public:
	CRegistrationSecondDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRegistrationSecondDlg();

	enum { IDD = IDD_DIALOG_REGISTARTION_SECONGDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
protected:
	DWORD		dwDaysRemain;
public:
	afx_msg void OnBnClickedRadioTryProduct();
	BOOL PreTranslateMessage(MSG* pMsg);
	bool ValidateRegistrationKey();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonNext();
	afx_msg void OnBnClickedButtonBack();
	afx_msg void OnBnClickedRadioActiveproduct();
	DWORD GetRegisteredUserInfo(DWORD dwProdID);
	DWORD GetRegistrationDataFromRegistry( );
	DWORD AddRegistrationDataInRegistry();
	DWORD AddRegistrationDataInFile( );
	DWORD GetRegistrationDatafromFile( );
	DWORD GetRegistrationDateFromServer( LPSYSTEMTIME lpServerTime, LPDWORD );
	CString GetModuleFilePath();
	DWORD DecryptData( LPBYTE lpBuffer, DWORD dwSize );
	DWORD SendUserRegistrationDataToServer(TCHAR *pRegData, DWORD dwRegSize, LPSYSTEMTIME lpServerTime );
	DWORD ExtractDate(TCHAR *pTime, LPSYSTEMTIME lpServerTime, LPDWORD ) ;
	bool CheckForMachineID(const AVACTIVATIONINFO	&actInfo);
	void ShowHideSecondDlg(bool bEnable);
	void ShutdownThread();
	LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	bool SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPBYTE byData, DWORD dwLen, bool bWait);
	bool SendRegisteredData2Service(DWORD dwType, LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName,bool bRegWait =false);
	bool CopyFileToDestination( TCHAR *pszSource, TCHAR *pszDest ) ;
	bool GetGlobalIPAddrFromUrl(LPTSTR szResponse)	;
	DWORD GetRegistrationDatafromFile( CString strUserRegFile ) ;
	DWORD SpreadRegistrationFilesInSystem( );
	bool CopyRegistrationFilesUsingService(DWORD dwType, CString csSrcFilePath, CString csDestFilePath, DWORD dwValue, bool bWait);
	bool SendRegistrationInfo2Service(int iMessageInfo, DWORD dwType = 0,bool bEmailPluginWait = false);
	bool GetDomainName(LPTSTR szDomainName, DWORD dwSize);
	DWORD SendUserInformation2Server(int iInfoType);
	DWORD SendHTTPUpdateRequest(int iInfoType, LPSYSTEMTIME lpServerTime, LPDWORD lpDaysRemaining);
	bool UpdateUserInformationInLocalFiles(LPDWORD lpDaysRemaining);
	void DisplayFailureMessage(REGFAILUREMSGS dwRes);
	void DisplayFailureMessageForOfflineRegistration(REGFAILUREMSGS dwRes);
	bool CheckForMIDInRegistry();
	void ResetVariable();
public:
	bool				m_bMisMatchedEmailID;
	HCURSOR			    m_hButtonCursor;
	bool				m_bActiveProduct;
	bool				m_bTryProduct;
	bool					m_bOnlineActivation;
	bool					m_bOfflineActivation;
	CxSkinButton		m_btnCancel;
	CxSkinButton		m_btnNext;
	CxSkinButton		m_btnBack;
	CColorStatic		m_stInternetReqText;
	CColorStatic		m_stActiveProduct;
	CColorStatic		m_stTryProdText;
	CxSkinButton		m_btnActiveProduct;
	CxSkinButton		m_btnTryProduct;
	CEdit				m_edtRegistrationkey;
	CColorStatic		m_stClickTryText;
	CColorStatic		m_stClickActiveText;
	CColorStatic		m_stexample;
	CFont*				m_pTextFont;
	CFont*				m_pBoldFont;
	HANDLE				m_hSendRequestThread ;
	HMODULE				m_RegisterationDLL ;
	bool				m_bisThreadCompleted;
	AVACTIVATIONINFO	m_ActInfo ;
	CRegistrationThirdDlg	m_objRegistrationThirdDlg;
	CRegistrationForthDlg	m_objRegistrationForthDlg;
	TCHAR					m_szMachineIDValue[0x80];
	TCHAR					m_szInstallCode[32];
	HANDLE					m_hOfflineNextEvent;
	HMODULE					m_hOfflineDLL;

	BOOL	RefreshString();
	afx_msg void OnPaint();
	CxSkinButton m_btnOnlineActivation;
	CxSkinButton m_btnOfflineActivation;
	CColorStatic m_stOnlineActivation;
	CColorStatic m_stOfflineActivation;

	afx_msg void OnBnClickedRadioOnlineActivation();
	afx_msg void OnBnClickedRadioOfflineActivation();
	bool LoadWrdWizOfflineRegDll();
	bool ReadHostFileForWWRedirection();

	DECLARE_MESSAGE_MAP()

};
