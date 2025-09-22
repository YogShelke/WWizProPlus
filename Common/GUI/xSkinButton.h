#if !defined(AFX_MYB_H__3832DDEF_0C12_11D5_B6BE_00E07D8144D0__INCLUDED_)
#define AFX_MYB_H__3832DDEF_0C12_11D5_B6BE_00E07D8144D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxcmn.h>
#include "ColorStatic.h"
#define WM_CXSHADE_RADIO WM_USER+0x100

#define DT_BOTTOM_SD                  0x00000000
/////////////////////////////////////////////////////////////////////////////
// CxSkinButton window
class CxSkinButton : public CButton
{
	// Construction
public:
	CxSkinButton();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void SetResourceHandle(HANDLE hResHandle, HWND hWnd);
	void SetToolTipText(CString s);
	COLORREF SetTextColor(COLORREF new_color);
	COLORREF SetTextColorA(COLORREF normal_color,COLORREF down_color = 0,COLORREF over_color = 0);
	COLORREF SetFocusTextColor(COLORREF new_color);
	void SetSkin(UINT normal,UINT down, UINT over=0, UINT disabled=0, UINT focus=0,UINT mask=0,
		short drawmode=1,short border=1,short margin=4);
	void SetSkin(HMODULE hinstDLL, UINT normal,UINT down, UINT over=0, UINT disabled=0, UINT focus=0,UINT mask=0,
		short drawmode=1,short border=1,short margin=4);
	void SetSkin(LPCTSTR normal,LPCTSTR down =0, LPCTSTR over=0, LPCTSTR disabled=0, LPCTSTR focus=0,LPCTSTR mask=0,
		short drawmode=1,short border=1,short margin=4);
	virtual ~CxSkinButton();
	// Generated message map functions
protected:
	virtual void PreSubclassWindow();
	void RelayEvent(UINT message, WPARAM wParam, LPARAM lParam);
	HRGN	CreateRgnFromBitmap(HBITMAP hBmp, COLORREF color);
	void	FillWithBitmap(CDC* dc, HBITMAP hbmp, RECT r);
	void	DrawBitmap(CDC* dc, HBITMAP hbmp, RECT r, int DrawMode);
	int		GetBitmapWidth (HBITMAP hBitmap);
	int		GetBitmapHeight (HBITMAP hBitmap);


	//{{AFX_MSG(CxSkinButton)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnClicked();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
	afx_msg LRESULT OnRadioInfo(WPARAM, LPARAM);
	afx_msg LRESULT OnBMSetCheck(WPARAM, LPARAM);
	afx_msg LRESULT OnBMGetCheck(WPARAM, LPARAM);

	bool	m_Checked;					//radio & check buttons
	DWORD	m_Style;					//radio & check buttons
	DWORD	m_AlignStyle;				//Alignment
	bool m_bBtnRadio_down; 
	bool m_tracking;
	bool m_button_down;
	CToolTipCtrl m_tooltip;
	CBitmap m_bNormal,m_bDown,m_bDisabled,m_bMask,m_bOver,m_bFocus; //skin bitmaps
	short	m_FocusRectMargin;		//dotted margin offset
	COLORREF m_TextColor;			//button text color
	COLORREF m_DownTextColor;			//button text color
	COLORREF m_OverTextColor;			//button text color
	COLORREF m_FocusTextColor;          //button text colour on focus

	HRGN	hClipRgn;				//clipping region
	BOOL	m_Border;				//0=flat; 1=3D;
	short	m_DrawMode;				//0=normal; 1=stretch; 2=tiled;
	DECLARE_MESSAGE_MAP()
public:
	bool	m_bShowText;
	bool	m_bShiftClickText;
	bool	m_bShowMultiline;	//To supprt multiline text : mrudula
	bool	m_bShowTopline;
	void	SetTextAlignment(DWORD dwAlignment, bool bBottom = false);
	void	ShowMultilineText(bool bMultiline);	//To supprt multiline text : mrudula
	void    ShowToplineText(bool bTopline);
	void    CreateFontStyle(CFont& m_Font, CString csFontName, int iFontHeight, int iFontWidth = 0,
		int iFontUnderline = 0, int iFontWeight = FW_NORMAL);

	CColorStatic *m_pstBoost;
	bool m_bBottom;
};
/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#endif // !defined(AFX_MYB_H__3832DDEF_0C12_11D5_B6BE_00E07D8144D0__INCLUDED_)
