﻿#pragma once
#include "CMyEditBrowseCtrl.h"
#include "CWndDragBar.h"
#include "CShortCutList.h"
// CDlgTabView 대화 상자

class CDlgTabView : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTabView)

public:
	CDlgTabView(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgTabView();
	CMyEditBrowseCtrl m_editPath;
	CWndDragBar m_wndDragTab;
	int m_nDragBarPos;
	CTabCtrl m_tabPath;
	CImageList m_tabImgList;
	PathTabInfoArray m_aTabInfo;
	TabViewOption m_tvo;
	int m_nCurrentTab;
	int m_lfHeight;
	int m_nFocusedImage;
	CStringArray* m_pShortCutPathArray;
	//DWORD m_btnsize_text;
	//DWORD m_btnsize_icon;
	CToolBar* m_pTool;
	CToolBar m_toolIcon;
	CToolBar m_toolText;
	CFont m_font;
	BOOL m_bSelected;
	BOOL m_bFindMode;
	BOOL m_bViewShortCut;
	void ArrangeCtrl();
	void ResizeToolBar(int width, int height);
	void SetCurrentTab(int nTab);
	CWnd* CurrentList();
	void UpdateFromCurrentList();
	void UpdateSortInfo(CWnd* pWnd);
	void UpdateTabByPathEdit();
	void SetTabTitle(int nTab, CString strTitle);
	void SetSelected(BOOL bSelected);
	void AddFileListTabByPath(CString strFolder);
	void CloseFileListTab(int nTab);
	void Clear();
	void SetCtrlColor(COLORREF clrBk, COLORREF clrText, BOOL bSetBk, BOOL bSetText);
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
	void UpdateMsgBarFromList();
	void OpenFolderByShortCut(int lParam);
	void ShortCutImport(CStringArray& aPath);
	void ShortCutExport(CStringArray& aPath);
	void ShortCutRefresh();
	void MoveCurrentTab(BOOL bRight);

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
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	CShortCutList m_listShortCut;
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};
