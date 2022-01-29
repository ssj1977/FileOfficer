#pragma once
#include "CMyEditBrowseCtrl.h"
// CDlgTabView 대화 상자

class CDlgTabView : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTabView)

public:
	CDlgTabView(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgTabView();
	CMyEditBrowseCtrl m_editPath;
	CTabCtrl m_tabPath;
	CImageList m_tabImgList;
	PathTabInfoArray m_aTabInfo;
	int m_nCurrentTab;
	int m_nViewOptionIndex;
	int m_lfHeight;
	CToolBar m_tool;
	CFont m_font;
	BOOL m_bSelected;
	void ArrangeCtrl();
	void SetCurrentTab(int nTab);
	CWnd* CurrentList();
	void UpdateTabByWnd(CWnd* pWnd);
	void UpdateSortInfo(CWnd* pWnd);
	void UpdateTabByPathEdit();
	void SetTabTitle(int nTab, CString strTitle);
//	void SetSelected(BOOL bSelected);
	void AddFileListTab(CString strFolder);
	void CloseFileListTab(int nTab);
	void Clear();
	BOOL BreakThreads();
	void SetListColor(COLORREF crBk, COLORREF crText, BOOL bSetBk, BOOL bSetText);
	int GetIconType();
	void SetIconType(int nIconType);
	int GetFontSize();
	BOOL GetIsBold();
	COLORREF GetMyClrText();
	COLORREF GetMyClrBk();
	void InitFont();
	void InitToolBar();
	void UpdateChildFont();
	void ConfigViewOption();
	void UpdateToolBar();
	void UpdateColWidths();

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
	afx_msg void OnTcnSelchangeTabPath(NMHDR* pNMHDR, LRESULT* pResult);
};
