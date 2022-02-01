#pragma once
#include <afxlistctrl.h>
#include "CMyDropTarget.h"

/*using namespace std;
#include <set>
typedef set<CString> CPathSet; //중복이름 체크용 맵 해당하는 이미지맵의 번호를 기억*/

class CDirectoryChangeWatcher;
class CFileListCtrl;

// From https://www.codeproject.com/Articles/950/CDirectoryChangeWatcher-ReadDirectoryChangesW-all
#include "DirectoryChanges.h"
class CMyDirectoryChangeHandler : public CDirectoryChangeHandler
{
public:
	CMyDirectoryChangeHandler(CFileListCtrl* pList);
	CFileListCtrl* m_pList;
	virtual void On_FileAdded(const CString& strFileName);
	virtual void On_FileRemoved(const CString& strFileName);
	virtual void On_FileModified(const CString& strFileName);
	virtual void On_FileNameChanged(const CString& strOldFileName, const CString& strNewFileName);
	//virtual void On_ReadDirectoryChangesError(DWORD dwError);
	//virtual void On_WatchStarted(DWORD dwError, const CString& strDirectoryName);
	//virtual void On_WatchStopped(const CString& strDirectoryName);
	//Filter related:
	//virtual bool On_FilterNotification(DWORD dwNotifyAction, LPCTSTR szFileName, LPCTSTR szNewFileName);
};

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
	void AddItemByPath(CString strPath, BOOL bCheckExist = FALSE, BOOL bAllowBreak = TRUE, CString strSelectByName = _T(""));
	void UpdateItemByPath(CString strOldPath, CString strNewPath);
	void DisplayFolder(CString strFolder, BOOL bUpdatePathHistory = TRUE);
	void DisplayFolder_Start(CString strFolder, BOOL bUpdatePathHistory = TRUE);
	void InitColumns(int nType);
	void SetColTexts(int* pStringId, int* pColFmt, int size);
	void ShowContextMenu(CPoint pt);
	CString GetItemFullPath(int nItem);
	CString GetCurrentFolder();
	CString GetCurrentItemPath();
	BOOL DeleteInvalidItem(int nItem);
	void DeleteInvalidPath(CString strPath);
	BOOL IsItemExist(int nItem);
	void PasteFiles(CStringArray& aOldPath, BOOL bMove);
	void ClipBoardExport(BOOL bMove);
	void ClipBoardImport();
	HGLOBAL GetOleDataForClipboard();
	CString m_strFolder;
	CString m_strPrevFolder; //폴더간 이동시 하위 폴더에서 상위폴더로 이동하는 경우 자동으로 해당 하위폴더를 목록 중에서 선택하기 위해 이용
	CString m_strFilterInclude;
	CString m_strFilterExclude;
	CString m_strBarMsg;
	CList<CString> m_aPathHistory;
	POSITION m_posPathHistory;
	BOOL m_bUpdatePathHistory;
	void BrowsePathHistory(BOOL bPrevious);
	void AddPathHistory(CString strPath);
	BOOL IsFirstPath();
	BOOL IsLastPath();
	BOOL IsRootPath();
//	CPathSet m_setPath;
	BOOL m_bAsc;
	BOOL m_bMenuOn;
	//BOOL m_bUseFileType; //파일의 종류를 설명하는 정보를 가져올지 구분, FALSE 이면 확장자로 대체, 속도면에서 많은 차이가 있음
	CUIntArray m_aColWidth;
	int m_nSortCol;
	int m_nType;
	int m_nIconType;
	int CompareItemByType(LPARAM item1, LPARAM item2, int nCol, int nType);
	int CMD_UpdateSortInfo;
	int CMD_UpdateTabCtrl;
	int CMD_UpdateBar;
	int CMD_OpenNewTab;
	CMyDropTarget m_DropTarget;
	void ProcessDropFiles(HDROP hDropInfo, BOOL bMove);
	void DeleteSelected(BOOL bRecycle);
	BOOL RenameSelectedItem();
	static UINT DisplayFolder_Thread(void* lParam);
	static void SetLoadingStatus(CFileListCtrl* pList, BOOL bLoading);
	static BOOL IsLoading(CFileListCtrl* pList);
	static void DeleteLoadingStatus(CFileListCtrl* pList);

	void ClearThread();
	HANDLE m_hThreadLoad;
	CDirectoryChangeWatcher m_DirWatcher;
	CMyDirectoryChangeHandler m_DirHandler;
	void WatchCurrentDirectory(BOOL bOn);
	void SortCurrentList();
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
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	virtual COLORREF OnGetCellTextColor(int nRow, int nColumn);
	virtual COLORREF OnGetCellBkColor(int nRow, int nColumn);
};


