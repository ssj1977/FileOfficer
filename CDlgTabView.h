#pragma once
#include "CMyEditBrowseCtrl.h"
#include "CWndDragBar.h"
#include "CMyShellTreeCtrl.h"
#include "CMyShellListCtrl.h"
// CDlgTabView 대화 상자

class CDlgTabView : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTabView)

public:
	CDlgTabView(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgTabView();
	CMyEditBrowseCtrl m_editPath;
	CWndDragBar m_wndDragTab;
	CMyShellListCtrl m_listShell;
	int m_nDragBarPos;
	CTabCtrl m_tabPath;
	CImageList m_tabImgList;
	PathTabInfoArray m_aTabInfo;
	TabViewOption m_tvo;
	int m_nCurrentTab;
	int m_lfHeight;
	int m_nFocusedImage;
	//DWORD m_btnsize_text;
	//DWORD m_btnsize_icon;
	CToolBar* m_pTool;
	CToolBar m_toolIcon;
	CToolBar m_toolText;
	CFont m_font;
	BOOL m_bSelected;
	BOOL m_bFindMode;
	void ArrangeCtrl();
	void ResizeToolBar(int width, int height);
	void SetCurrentTab(int nTab);
	CWnd* CurrentList();
	int CurrentListType();
	void UpdateFromCurrentList();
	void UpdateSortInfo(CWnd* pWnd);
	void UpdateTabByPathEdit();
	void SetTabTitle(int nTab, CString strTitle);
	void SetSelected(BOOL bSelected);
	void AddFileListTab(CString strFolder);
	void CloseFileListTab(int nTab);
	void Clear();
	void SetCtrlColor(COLORREF crBk, COLORREF crText, BOOL bSetBk, BOOL bSetText);
	//void SetListColor(COLORREF crBk, COLORREF crText, BOOL bSetBk, BOOL bSetText);
	int GetIconType();
	void SetIconType(int nIconType);
	COLORREF GetMyClrText();
	COLORREF GetMyClrBk();
	void InitFont();
	void InitToolBar();
	void UpdateChildFont();
	void ConfigViewOption();
	void UpdateToolBar();
	void UpdateColWidths();
	void UpdateBkImg(CWnd* pWnd);
	void UpdateBkImgAll();
	void ToggleFindMode();
	void FindNext();

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
	afx_msg void OnBnClickedBtnFind();
//	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	CMyShellTreeCtrl m_wndFolderTree;
	afx_msg void OnDropFiles(HDROP hDropInfo);
};
