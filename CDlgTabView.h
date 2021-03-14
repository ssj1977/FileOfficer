#pragma once

#ifndef PathTabInfo
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
#endif 
// CDlgTabView 대화 상자

class CDlgTabView : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTabView)

public:
	CDlgTabView(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgTabView();
	CMFCEditBrowseCtrl m_editPath;
	CTabCtrl m_tabPath;
	PathTabInfoArray m_aTabInfo;
	int m_nCurrentTab;
	CToolBar m_tool;
	int m_lfHeight;

	void ArrangeCtrl();
	void SetCurrentTab(int nTab);
	CWnd* CurrentList();
	void UpdateTabByWnd(CWnd* pWnd);
	void UpdateSortInfo(CWnd* pWnd);
	void UpdateTabByPathEdit();
	void SetTabTitle(int nTab, CString strTitle);
	void UpdateIconType();
	void Clear();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_VIEW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
