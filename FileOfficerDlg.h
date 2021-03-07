// FileOfficerDlg.h
//
#pragma once

#include "CDlgTabView.h"

class CFileOfficerDlg : public CDialogEx
{
public:
	CFileOfficerDlg(CWnd* pParent = nullptr);
	CDlgTabView m_tv1;
	CDlgTabView m_tv2;
	BOOL m_bShow2;
	CRect m_rcMain;
	CToolBar m_toolMain;
	CFont m_font;
	int m_lfHeight;
	void ArrangeCtrl();
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILEOFFICER_DIALOG };
#endif
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV
protected:
	HICON m_hIcon;
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeTabPath1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeTabPath2(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnCancel();
	virtual void OnOK();
};
