#pragma once

//#include "CMyDropTarget.h"
// CMyShellListCtrl

class CMyShellListCtrl : public CMFCShellListCtrl
{
	DECLARE_DYNAMIC(CMyShellListCtrl)

public:
	CMyShellListCtrl();
	virtual ~CMyShellListCtrl();
	void ClearThread();
	HRESULT DisplayFolder_CustomSort(LPAFX_SHELLITEMINFO pItemInfo);
	HRESULT DisplayFolder_CustomSort(LPCTSTR lpszPath);
	void LoadFolder(CString strFolder, BOOL bAddHistory = TRUE);
	void OpenParentFolder();
	void OpenSelectedItem();
	void DeleteSelected(BOOL bRecycle);
	void RenameSelectedItem();
	void ConvertNFDNames();
	void RenameFiles(CStringArray& aPath, CString strNewPath);
	int CMD_UpdateBar;
	int CMD_OpenNewTabByList;
	int CMD_UpdateSortInfo;
	int CMD_UpdateFromList;
	CString m_strCurrentFolder;
	int m_nIconType;
	CString GetBarString();
	void UpdateMsgBar(int nStringID = 0);
	BOOL m_bLoading;
	
	//컬럼별 폭 및 정렬 기준 기억
	void InitColumns(int nType);
	//void ResizeColumns();
	CUIntArray m_aColWidth;
	int m_nSortColInit; // 처음 초기화 용도로만 사용
	BOOL m_bAscInit; //처음 초기화 용도로만 사용 

	//히스토리 기능
	CList<CString> m_aPathHistory;
	POSITION m_posPathHistory;
	void BrowsePathHistory(BOOL bPrevious);
	void AddPathHistory(CString strPath);
	inline BOOL IsFirstPath() { return m_posPathHistory == m_aPathHistory.GetHeadPosition(); };
	inline BOOL IsLastPath() { return m_posPathHistory == m_aPathHistory.GetTailPosition(); };
	inline BOOL IsRootPath() { return IsDesktop(); };

	//폴더 내 변경사항의 업데이트를 위한 추가 함수
	CString GetItemFullPath(int nItem);
	HRESULT GetShellItemInfoFromPath(CString strPath, BOOL bRelativePath, LPAFX_SHELLITEMINFO pItemInfo);
	void PasteFiles(CStringArray& aOldPath, BOOL bMove);
	int AddPath(CString strPath, BOOL bRelativePath = FALSE, BOOL bCheckExist = TRUE);
	int SetItemByName(int nItem, CString strName, BOOL bInsert = FALSE);
	void ReleaseItemData(int nItem);
	void UpdateItemByPath(CString strOldPath, CString strNewPath, BOOL bRelativePath = FALSE, BOOL bForceUpdate = FALSE);
	int GetIndexByName(CString strName);
	void DeleteInvalidName(CString strName);
	//

	// 탐색기와 상호 Copy & Paste
	void ClipBoardExport(BOOL bCut);
	void ClipBoardImport();
	HGLOBAL GetOleDataForClipboard(int nState);
	void ProcessDropFiles(HDROP hDropInfo, BOOL bMove);
	//

	// 변경사항 모니터링용 쓰레드 처리
	BOOL IsWatchable();
	void WatchFolder_Begin();
	void WatchFolder_Work();
	void WatchFolder_End();
	void WatchFolder_Resume();
	void WatchFolder_Suspend();
	void WatchEventHandler();
	static UINT WatchFolder_Thread(void* lParam);
	static void SetWatchingStatus(CMyShellListCtrl* pList, BOOL bLoading);
	static BOOL IsWatching(CMyShellListCtrl* pList);
	static void DeleteWatchingStatus(CMyShellListCtrl* pList);
	HANDLE m_hWatchBreak; //디렉토리 모니터링을 중단할때 필요한 이벤트
	CWinThread* m_pThreadWatch;
	OVERLAPPED	m_overlap_watch; //비동기 IO를 위한 OVERLAPPED개체 상속 구조체, Callback을 위한 객체 포인터 포함
	HANDLE		m_hDirectory;	//디렉토리 변경사항 모니터링을 위한 I/O Handle
	LPVOID		m_pWatchBuffer; // ReadDirectoryChangesW 를 위한 버퍼
	//
	
	// 색깔 넣기
	void* m_pColorRuleArray;
	COLORREF ApplyColorRule(int nRow, int nColumn, BOOL bBk);
	virtual COLORREF OnGetCellTextColor(int nRow, int nColumn);
	virtual COLORREF OnGetCellBkColor(int nRow, int nColumn);
	//

	//팝업 메뉴
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	void ShowContextMenu(CPoint* pPoint); //pPoint가 NULL인 경우 현재 마우스 위치로 처리
	BOOL m_bMenuOn;
	//
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnLvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	afx_msg void OnClipboardUpdate();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHdnItemclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
};

class CMyShellListProgress : public IFileOperationProgressSink
{
public:
	// IFileOperationSinkProgress 관련 구현
	IFACEMETHODIMP PostCopyItem(DWORD dwFlags, IShellItem* psiItem,
		IShellItem* psiDestinationFolder, PCWSTR pwszNewName, HRESULT hrCopy,
		IShellItem* psiNewlyCreated);
	IFACEMETHODIMP PostMoveItem(DWORD dwFlags, IShellItem* psiItem,
		IShellItem* psiDestinationFolder, PCWSTR pwszNewName, HRESULT hrCopy,
		IShellItem* psiNewlyCreated);
	IFACEMETHODIMP PostRenameItem(DWORD dwFlags, IShellItem*, PCWSTR, HRESULT, IShellItem*);
	long   _cRef;
	CMyShellListCtrl* m_pList;
	CMyShellListProgress() { _cRef = 1; m_pList = NULL; };
	IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();
	IFACEMETHODIMP StartOperations() { return S_OK; };
	IFACEMETHODIMP FinishOperations(HRESULT) { return S_OK; };
	IFACEMETHODIMP PreRenameItem(DWORD, IShellItem*, PCWSTR) { return S_OK; };
	IFACEMETHODIMP PreMoveItem(DWORD, IShellItem*, IShellItem*, PCWSTR) { return S_OK; };
	IFACEMETHODIMP PreCopyItem(DWORD, IShellItem*, IShellItem*, PCWSTR) { return S_OK; };
	IFACEMETHODIMP PreDeleteItem(DWORD, IShellItem*) { return S_OK; };
	IFACEMETHODIMP PostDeleteItem(DWORD, IShellItem*, HRESULT, IShellItem*) { return S_OK; };
	IFACEMETHODIMP PreNewItem(DWORD, IShellItem*, PCWSTR) { return S_OK; };
	IFACEMETHODIMP PostNewItem(DWORD, IShellItem*, PCWSTR, PCWSTR, DWORD, HRESULT, IShellItem*) { return S_OK; };
	IFACEMETHODIMP UpdateProgress(UINT, UINT) { return S_OK; };
	IFACEMETHODIMP ResetTimer() { return S_OK; };
	IFACEMETHODIMP PauseTimer() { return S_OK; };
	IFACEMETHODIMP ResumeTimer() { return S_OK; };
};