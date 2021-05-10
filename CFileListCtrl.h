#pragma once
#include <afxlistctrl.h>
#include "CMyDropTarget.h"

#define IDM_SET_FOCUS_ON 50010
#define IDM_SET_FOCUS_OFF 50011

class CFileListCtrl : public CMFCListCtrl
{
	DECLARE_DYNAMIC(CFileListCtrl)

public:
	CFileListCtrl();
	virtual ~CFileListCtrl();
	void OpenSelectedItem();
	void OpenParentFolder();
	void ResizeColumns();
	void SetBarMsg(CString strMsg);
	void AddItemByPath(CString strPath, BOOL bCheckExist = FALSE);
	void DisplayFolder(CString strFolder);
	void DisplayFolder_Start(CString strFolder);
	void InitColumns(int nType);
	void ShowContextMenu(CPoint pt);
	CString GetItemFullPath(int nItem);
	CString GetCurrentFolder();
	CString GetCurrentItemPath();
	BOOL DeleteInvalidItem(int nItem);
	void DeleteInvalidPath(CString strPath);
	BOOL IsItemExist(int nItem);
	void PasteFile(CString strPath, BOOL bMove);
	void ClipBoardExport(BOOL bMove);
	void ClipBoardImport();
	HGLOBAL GetOleDataForClipboard();
	CString m_strFolder;
	CString m_strBarMsg;
	BOOL m_bAsc;
	BOOL m_bIsLoading;
	int m_nSortCol;
	int m_nType;
	int m_nIconType;
	int CompareItemByType(LPARAM item1, LPARAM item2, int nCol, int nType);
	int CMD_UpdateSortInfo;
	int CMD_UpdateTabCtrl;
	int CMD_UpdateBar;
	int CMD_OpenNewTab;
	CMyDropTarget m_DropTarget;
	void MyDropFiles(HDROP hDropInfo, BOOL bMove, CFileListCtrl* pListSrc = NULL);
	void DeleteSelected(BOOL bRecycle);
	BOOL RenameSelectedItem();
	static UINT DisplayFolder_Thread(void* lParam);
	static UINT SetChangeListner_Thread(void* lParam);
	void SetChangeListner();
	BOOL m_bListening;

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHdnItemclick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual int OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn);
	virtual void Sort(int iColumn, BOOL bAscending = TRUE, BOOL bAdd = FALSE);
	afx_msg void OnLvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnClipboardUpdate();
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
};

