#pragma once
#include "afxwin.h"

class CWndDragPane : public CWnd
{
public:
	CWndDragPane();
	virtual ~CWndDragPane();

	BOOL CreateWnd(BOOL bVertical, CWnd* pParentWnd, UINT nID);
	void SetPaneWnd(CWnd* pWnd1, CWnd* pWnd2);
	BOOL m_bVertical;
	BOOL m_bDragging;
	int m_nBarPos;
	int m_nBarSize;
	COLORREF m_crBackGround;
	COLORREF m_crBar;
	CWnd* m_pWnd1;
	CWnd* m_pWnd2;
	void ArrangeCtrl();
	CRect GetBarRect();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

