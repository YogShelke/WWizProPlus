#if !defined(AFX_COLORSTATIC_H__F35D88B3_A7BA_46D1_8FFF_AA0E973D9CC7__INCLUDED_)
#define AFX_COLORSTATIC_H__F35D88B3_A7BA_46D1_8FFF_AA0E973D9CC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CColorStatic : public CStatic
{
	bool m_SetTransparent;
// Construction
public:
	CColorStatic();

// Attributes
public:
    COLORREF m_clrText;
    COLORREF m_clrBack;
    CBrush m_brBkgnd;

// Operations
public:
    void SetTextColor (COLORREF clrText);
    void SetBkColor (COLORREF clrBack);
	void SetTransparent(bool bSetState);
	void setFont( CString csFontName, int iFontHeight, int iFontWidth,int iFontUnderline = 0,int iFontWeight = FW_NORMAL);
	void setUnderline(  int iFontUnderline );
	void CreateFontStyle( CFont& m_Font, CString csFontName, int iFontHeight, int iFontWidth = 0, 
						  int iFontUnderline = 0, int iFontWeight = FW_NORMAL);

	

// Implementation
public:
	virtual ~CColorStatic();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorStatic)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

#endif // !defined(AFX_COLORSTATIC_H__F35D88B3_A7BA_46D1_8FFF_AA0E973D9CC7__INCLUDED_)
