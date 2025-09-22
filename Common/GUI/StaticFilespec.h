/*
 *  StaticFilespec.h
 *
 *  CStaticFilespec interface
 *    A simple class for displaying long filespecs in static text controls
 *
 *  Usage:        
 */

#ifndef _StaticFilespec_h
#define _StaticFilespec_h

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CStaticFilespec window

class CStaticFilespec : public CWnd
{
// Construction
	bool m_SetTransparent;
public:
	CStaticFilespec
    (DWORD  dwFormat = DT_LEFT | DT_NOPREFIX | DT_VCENTER,
     BOOL   bPathEllipsis = FALSE,
	 DWORD dwScanType = 0x01);

// Attributes
public:
	COLORREF m_clrText;
	COLORREF m_clrBack;
	CBrush m_brBkgnd;
	CFont* m_font;
	DWORD m_dwScanType;
// Operations
public:
  BOOL    IsPath() { return m_bPathEllipsis; }
  void    SetPath (BOOL bPathEllipsis)  { m_bPathEllipsis = bPathEllipsis; } 
  DWORD   GetFormat() { return m_dwFormat; } 
  void    SetFormat (DWORD dwFormat) { m_dwFormat = dwFormat; } 
  void SetTextColor(COLORREF clrText);
  void SetBkColor(COLORREF clrBack);
  void SetTransparent(bool bSetState);
  void setFont(CFont* specFont);//(CString csFontName, int iFontHeight, int iFontWidth, int iFontUnderline = 0, int iFontWeight = FW_NORMAL);
  void setUnderline(int iFontUnderline);
  void CreateFontStyle(CFont& m_Font, CString csFontName, int iFontHeight, int iFontWidth = 0,
	  int iFontUnderline = 0, int iFontWeight = FW_NORMAL);
  void setTextFormat(DWORD dwFormat);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaticFilespec)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CStaticFilespec();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStaticFilespec)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	

  BOOL    m_bPathEllipsis;    // flag: draw text as path
  DWORD   m_dwFormat;         // text format
protected:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif

// End StaticFilespec.h
