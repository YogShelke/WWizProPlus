#if !defined(AFX_ColorEdit_H__E889B47D_AF6B_4066_B055_967508314A88__INCLUDED_)
#define AFX_ColorEdit_H__E889B47D_AF6B_4066_B055_967508314A88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorEdit.h : header file
//
#define RED        RGB(127,  0,  0)
#define GREEN      RGB(  0,127,  0)
#define BLUE       RGB(  0,  0,127)
#define LIGHTRED   RGB(255,  0,  0)
#define LIGHTGREEN RGB(  0,255,  0)
#define LIGHTBLUE  RGB(  0,  0,255)
#define BLACK      RGB(  0,  0,  0)
#define WHITE      RGB(255,255,255)
#define GRAY       RGB(192,192,192)

/////////////////////////////////////////////////////////////////////////////
// CColorEdit window

class CColorEdit : public CEdit
{
// Construction
public:
	CColorEdit();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorEdit)
	//}}AFX_VIRTUAL

	void SetBkColor(COLORREF crColor); // This Function is to set the BackGround Color for the Text and the Edit Box.
	void SetTextColor(COLORREF crColor); // This Function is to set the Color for the Text.
	BOOL SetReadOnly(BOOL flag = TRUE);
	virtual ~CColorEdit();

	// Generated message map functions
protected:
	CBrush m_brBkgnd; // Holds Brush Color for the Edit Box
	COLORREF m_crBkColor; // Holds the Background Color for the Text
	COLORREF m_crTextColor; // Holds the Color for the Text
	//{{AFX_MSG(CColorEdit)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor); // This Function Gets Called Every Time Your Window Gets Redrawn.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ColorEdit_H__E889B47D_AF6B_4066_B055_967508314A88__INCLUDED_)
