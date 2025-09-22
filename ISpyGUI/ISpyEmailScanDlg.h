#pragma once
#include <string>
#include <Msi.h>
#include "JpegDialog.h"
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "ISpyGUI.h"
#include "ISpyList.h"
#include "ISpyDataManager.h"
#include "iTinEmailContants.h"
#include "ISpyCommunicator.h"

class CISpyEmailScanDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CISpyEmailScanDlg)
private:
	CISpyEmailScanDlg(CWnd* pParent = NULL);   // standard constructor
public:
	static CISpyEmailScanDlg* GetEmailScanDlgInstance(CWnd* pParent);
	static void ResetInstance();
	virtual ~CISpyEmailScanDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_EMAILSCAN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CRect					m_rect;
	BOOL					m_bSignature;
	HCURSOR					m_hButtonCursor;
	CxSkinButton			m_btnQuarantine;
	CxSkinButton			m_btnOk;
	CxSkinButton			m_btnApply;
	CxSkinButton			m_btnDelete;
	CStatic					m_stEmailHeaderBitmap;
	HBITMAP					m_bmpEmailHeaderBitmap;
	CListCtrl				m_lstVirusScan;
	CxSkinButton			m_btnRemove;
	CColorStatic			m_stForExample;
	CColorStatic			m_stQuarantineText;
	CColorStatic			m_stRemoveText;
	CListCtrl				m_lstSpamFilter;
	CStatic					m_stSpamFilterHeaderPic;
	CBitmap					m_bmpSpamFilterHeader;
	CListCtrl				m_lstContentFilter;
	CStatic					m_contentFilterHeaderPic;
	CBitmap					m_bmpContentfilterHeader;
	CStatic					m_stSignatureHeaderPic;
	CBitmap					m_bmpSignatureHeader;
	CFont*                  m_pBoldFont;
	CFont*                  m_pHeaderFont;
	CStatic					m_stSettingHeaderPic;
	CBitmap					m_bmpSettingHeader;
	CButton					m_chkAllowSignature;
	CColorStatic			m_stAllowSignatureMsg;
	CButton					m_chkSpamFilter;
	CColorStatic			m_stEnableSpamFilter;
	CButton					m_chkVirusScan;
	CColorStatic			m_stEnableVirusScan;
	CButton					m_chkContentfilter;
	CColorStatic			m_stEnableContentFilter;
	CEdit					m_edtSigDesMsg;
	CStatic      			m_stGrpBoxForRadioBtn;
	CStatic					m_stGrpAddRule;
	CColorStatic			m_stAddRule;
	CStatic					m_stAddRuleHeaderPic;
	CBitmap					m_bmpAddRuleHeader;
	CStatic					m_stAddaRuleForContent;
	CBitmap					m_bmpAddaRuleForContent;
	CxSkinButton			m_btnEdit;
	CEdit					m_edtEmailAddress;
	CComboBox				m_DropList;
	CComboBox				m_DroplistContentFilter;
	CDataManager			m_objSpamFilterdb;
	CDataManager			m_objContentFilterdb;
	CDataManager			m_objSignaturedb;
	CDataManager			m_objVirusScandb;
	BOOL					m_bSpamFilter;
	BOOL					m_bVirusScan;
	BOOL					m_bContentFilter;
	int						m_nContactVersion;
	bool					m_bEditFlag;
	bool					m_bDeleteFlag;
	BOOL					m_bSignatureFlag;
	DWORD					m_dwnitem;
	DWORD					m_dwnDeleteItem;
	bool					m_bsuccess;
	BOOL					m_bUpdate;
	BOOL					m_bEnableQuarantine;
	BOOL					m_bEnableRemove;
	BOOL					m_bReloadSetting;
	DWORD					m_dwEnableContentFilter;
	DWORD					m_dwEnableSpamFilter;
	DWORD					m_dwEnableVirusScan;
	DWORD					m_dwEnableSignature;
	DWORD					m_dwAttachScanAction;
	DWORD					m_dwReloadSettings;
	DWORD					m_dwEnableVirusPopUp;
	CImageList 				m_ImageList;
	CImageList 				m_VirusImageList;
	bool					m_bEnableVirusPopUp;
	CButton					m_chkAllowVirusPopUp;
	CColorStatic			m_stAllowVirusPopdlg;
	CxSkinButton			m_btnDefaultSig;
	bool					m_bDefaultSigSelected;
	int						m_iCountEmailVScan;
	int						m_iPrevCountEmailVScan;

	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	bool OnBnClickedButtonVirusscan();
	bool OnBnClickedButtonSpamfilter();
	bool OnBnClickedButtonContentfilter();
	bool OnBnClickedButtonSignature();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedRadioQuarantine();
	LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBnClickedRadioRemove();
	//afx_msg void OnBnClickedButtonBack();
	afx_msg void OnBnClickedButtonEdit();
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnBnClickedCheckSignatureCheckbox();
	afx_msg void OnBnClickedCheckVirusscan();
	afx_msg void OnBnClickedCheckSpamfilter();
	afx_msg void OnBnClickedCheckEnablecontent();
	afx_msg void OnEnChangeEditSigDescriptionmsg();
	afx_msg void OnPaint();
	afx_msg void OnBnClickedCheckAllowviruspopup();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void RefreshString();

	void ShowHideChildEmailScan();
	void DisableEnableAllFunctions(bool bEnable);
	bool isEmail(CString inputEmail);
	void InsertItem(CString strEmailAddr, CString strType);
	void InsertItem(CString strEmailAddr, CString strType,CString strRuleType);
	BOOL StoredataContentToFile(CString csPathName);
	void LoadDataContentFromFile(CString csPathName);
	void PopulateList();
	void LoadExistingSpamFilterFile();
	void LoadExistingContentFilterFile();
	void LoadExistingSignatureFile();
	void LoadExistingVirusScanFile();
	bool isExtension(CString inputExtension);
	void WriteRegistryEntryofEmailScan(DWORD dwChangeValue);
	void ReadRegistryEntryofEmailScan();
	BOOL InitImageList();
	bool GetExistingPathofOutlook();
	HRESULT GetOutlookVersionString(LPTSTR* ppszVer, BOOL* pf64Bit);
	void HideAllControls();
	bool SendEmailData2Service(DWORD dwAddEditDelType, DWORD dwType, CString csEntry,bool bEmailScanWait = false);
	bool SendEmailData2Service(DWORD dwAddEditDelType, DWORD dwType, CString csFirstParam, CString csSecondParam, CString csThirdParam, bool bEmailScanWait = false);
	void HideORShowButton(bool bEnableAdd , bool bEnableEdit , bool bEnableDelete , bool bEnableApply);
	bool SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait = true);
	afx_msg void OnBnClickedBtnDefaultsig();
	BOOL WriteRegistryEntryOfMenuItem(CString strKey, DWORD dwChangeValue);
	void AppyRegistrySetting();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	bool CheckForOutllookExe();

	CColorStatic				m_stVirusscan_HeaderName;
	CColorStatic				m_stVirusscan_HeaderDes;
	CColorStatic				m_stVirusSettingHeader;
	CColorStatic				m_stSpamHeaderName;
	CColorStatic				m_stSpamHeaderDes;
	CColorStatic				m_stSpamSettingHeadername;
	CColorStatic				m_stContentHeaderName;
	CColorStatic				m_stContentHeaderDes;
	CColorStatic				m_stContentSettingHeader;
	CColorStatic				m_stSignture_HeaderName;
	CColorStatic				m_stSignature_HeaderDes;
	CISpyCommunicator			m_objEmailScanCom;
	bool						m_bIsPopUpDisplayed;
private:
	static bool					m_bEmailInstanceFlag;
	static CISpyEmailScanDlg	*m_pobjEmailScanDlg;
};
