#pragma once

class CToolTipCtrlEx : public CToolTipCtrl
{
public:
	CToolTipCtrlEx();
	virtual ~CToolTipCtrlEx();
protected:
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
private:
	DECLARE_DYNAMIC(CToolTipCtrlEx)
	enum	Orientations
	{
		NW=1,
		NE,
		SW,
		SE,
	};
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	COLORREF	m_bkColor;//=RGB(255,255,255);
	COLORREF	m_leftColor;//=RGB(255, 210, 83);
	COLORREF	m_frameColor;//=RGB(155, 110, 53);
	COLORREF	m_textColor;//=RGB(0,0,0);
	COLORREF	m_arrowColor;//=RGB(0,0,0);
};