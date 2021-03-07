// FileOfficerDlg.h
//
#pragma once

struct PathTabInfo
{
	CWnd* pWnd;
	CString strPath;
	int iSortColumn;
	BOOL bSortAscend;
	PathTabInfo()
	{
		pWnd = NULL;
		iSortColumn = 0;
		bSortAscend = TRUE;
	};
	PathTabInfo(CString strPath, int iSortColumn, BOOL bSortAscend)
	{
		SetTabInfo(strPath, iSortColumn, bSortAscend);
		pWnd = NULL;
	};
	void SetTabInfo(CString strPath, int iSortColumn, BOOL bSortAscend)
	{
		this->strPath = strPath;
		this->iSortColumn = iSortColumn;
		this->bSortAscend = bSortAscend;
	};
};

typedef CArray<PathTabInfo> PathTabInfoArray;

class CFileOfficerDlg : public CDialogEx
{
public:
	CFileOfficerDlg(CWnd* pParent = nullptr);
	CMFCEditBrowseCtrl m_editPath1;
	CMFCEditBrowseCtrl m_editPath2;
	CTabCtrl m_tabPath1;
	CTabCtrl m_tabPath2;
	PathTabInfoArray m_aTabInfo1;
	PathTabInfoArray m_aTabInfo2;
	int m_nCurrentTabItem1;
	int m_nCurrentTabItem2;
	int m_nCurrentTab;
	BOOL m_bShow2;
	CRect m_rcMain;
	CToolBar m_toolMain;
	CToolBar m_tool1;
	CToolBar m_tool2;

	CFont m_font;
	int m_lfHeight;
	HIMAGELIST* m_pSysImgList;
	void UpdateImageList();
	void UpdateFontSize();

	void UpdateBottomBar();
	void ArrangeCtrl();

	void SetCurrentTab(int nTab);
	void SetCurrentTabItem(int nTab, int nItem);
	int GetCurrentTab();
	CWnd* GetCurrentTabItem(int nTab);
	void SetTabTitle(int nTab, int nItem, CString strTitle);
	void UpdateTabItemByWnd(int nTab, CWnd* pWnd);

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
	virtual BOOL DestroyWindow();
};
