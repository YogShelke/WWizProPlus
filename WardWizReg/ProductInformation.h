#pragma once
#include "afxwin.h"
#include "Resource.h"
#include "JpegDialog.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "WardWizRegistration.h"


// CProductInformation dialog

class CProductInformation : public CJpegDialog
{
	DECLARE_DYNAMIC(CProductInformation)

public:
	CProductInformation(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProductInformation();

// Dialog Data
	enum { IDD = IDD_DIALOG_PRODUCT_INFORMATION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CFont*					m_pBoldFont;
	CColorStatic			m_stProductInfo;
	CColorStatic			m_RegisterTo;
	CColorStatic			m_stEmailID;
	CColorStatic			m_stNoofDaysLeft;
	CColorStatic			m_stProductEdition;
	CColorStatic			m_stWholeName;
	CColorStatic			m_stRegisteredEmailID;
	CColorStatic			m_stNoofDays;
	CColorStatic			m_stInstalledEdition;
	CxSkinButton			m_btnProdClose;
	CColorStatic			m_stVersion;
	CColorStatic			m_stVersionNo;
	CString					m_csEmailID;
	CString					m_csNoofDays;
	CString					m_csWholeName;
	CString					m_csInstalledEdition;
	CString					m_csVersionNo;
	CString					m_csDataEncVersion;
	CxSkinButton			m_btnOk;
	CColorStatic			m_stProductKey;
	CColorStatic			m_stProductKeyNumber;
	CString					m_csProductKeyNumber;
	afx_msg void OnBnClickedButtonProdClose();
	BOOL RefreshString();

	afx_msg void OnBnClickedButtonOk();

	
	CColorStatic m_stDataEncText;
	CColorStatic m_stDataEncVersion;
};
