#pragma once
#include <afxlistctrl.h>
#include "CMyDropTarget.h"

/*using namespace std;
#include <set>
typedef set<CString> CPathSet; //중복이름 체크용 맵 해당하는 이미지맵의 번호를 기억*/

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
	int AddItemByPath(CString strPath, BOOL bCheckExist = FALSE, BOOL bAllowBreak = TRUE, CString strSelectByName = _T(""));
	void UpdateItem(int nItem, CString strPath, BOOL bUpdateIcon);
	void UpdateItemByPath(CString strOldPath, CString strNewPath, BOOL bRelativePath = FALSE, BOOL bForceUpdate = FALSE);
	void DisplayFolder(CString strFolder, BOOL bUpdatePathHistory = TRUE);
	void DisplayFolder_Start(CString strFolder, BOOL bUpdatePathHistory = TRUE);
	void InitColumns(int nType);
	void SetColTexts(int* pStringId, int* pColFmt, int size);
	void ShowContextMenu(CPoint* pPoint); //pPoint가 NULL인 경우 현재 마우스 위치로 처리
	CString GetItemFullPath(int nItem);
	CString GetCurrentFolder();
	CString GetCurrentItemPath();
	BOOL DeleteInvalidItem(int nItem);
	void DeleteInvalidPath(CString strPath);
	BOOL IsItemExist(int nItem);
	void PasteFiles(CStringArray& aOldPath, BOOL bMove);
	void ClipBoardExport(BOOL bCut);
	void ClipBoardImport();
	HGLOBAL GetOleDataForClipboard(int nState);
	CString m_strFolder;
	CString m_strPrevFolder; //폴더간 이동시 하위 폴더에서 상위폴더로 이동하는 경우 자동으로 해당 하위폴더를 목록 중에서 선택하기 위해 이용
	CString m_strFilterInclude;
	CString m_strFilterExclude;
	CString m_strBarMsg;
	CList<CString> m_aPathHistory;
	POSITION m_posPathHistory;
	BOOL m_bUpdatePathHistory;
	void* m_pColorRuleArray;
	void BrowsePathHistory(BOOL bPrevious);
	void AddPathHistory(CString strPath);
	BOOL IsFirstPath();
	BOOL IsLastPath();
	BOOL IsRootPath();
	BOOL IsWatchable();
	//	CPathSet m_setPath;
	BOOL m_bAsc;
	BOOL m_bMenuOn; //컨텍스트 메뉴가 표시되어 있는지를 체크하는 플래그
	//BOOL m_bLoading; //IsLoading과 중복되지만 OnLvnItemchanged의 빠른 처리를 위해 사용
	BOOL m_bUseFileType; //파일의 종류를 설명하는 정보를 가져올지 구분, FALSE 이면 확장자로 대체, 속도면에서 많은 차이가 있음
	BOOL m_bUseFileIcon;
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
	void RenameSelectedItem();
	void ConvertNFDNames();
	void RenameFiles(CStringArray& aPath, CString strNewPath);
	void ClearSelected();
	
	//기타 static 함수들
	static void ClearPreviousSelection(); // 잘라내기 기능의 아이콘 음영 처리 초기화
	static LPITEMIDLIST GetPIDLfromPath(CString strPath); //MAX_PATH를 초과하는 경로에 대해서도 처리해 준다.
	static HRESULT CreateShellItemArrayFromPaths(CStringArray& aPath, IShellItemArray*& shi_array);

	// 파일목록 불러오기 쓰레드 처리
	CWinThread* m_pThreadLoad;
	static UINT DisplayFolder_Thread(void* lParam);
	static void SetLoadingStatus(CFileListCtrl* pList, BOOL bLoading);
	static BOOL IsLoading(CFileListCtrl* pList);
	static void DeleteLoadingStatus(CFileListCtrl* pList);

	// 변경사항 모니터링용 쓰레드 처리
	void WatchFolder_Begin();
	void WatchFolder_Work();
	void WatchFolder_End();
	void WatchFolder_Resume();
	void WatchFolder_Suspend();
	void WatchEventHandler();
	static UINT WatchFolder_Thread(void* lParam);
	static void SetWatchingStatus(CFileListCtrl* pList, BOOL bLoading);
	static BOOL IsWatching(CFileListCtrl* pList);
	static void DeleteWatchingStatus(CFileListCtrl* pList);
	HANDLE m_hWatchBreak; //디렉토리 모니터링을 중단할때 필요한 이벤트
	CWinThread* m_pThreadWatch;
	OVERLAPPED	m_overlap_watch; //비동기 IO를 위한 OVERLAPPED개체 상속 구조체, Callback을 위한 객체 포인터 포함
	HANDLE		m_hDirectory;	//디렉토리 변경사항 모니터링을 위한 I/O Handle
	LPVOID		m_pWatchBuffer; // ReadDirectoryChangesW 를 위한 버퍼

	HANDLE m_hLoadFinished; //디렉토리 모니터링을 중단할때 필요한 이벤트
	void ClearThread(); // 현재 작동중인 쓰레드들을 중단
	void UpdateMsgBar();
	void SortCurrentList();
	COLORREF ApplyColorRule(int nRow, int nColumn, BOOL bBk);

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
	afx_msg void OnLvnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
};

class MyProgress : public IFileOperationProgressSink
{
public :
// IFileOperationSinkProgress 관련 구현
	IFACEMETHODIMP PostCopyItem(DWORD dwFlags, IShellItem* psiItem,
		IShellItem* psiDestinationFolder, PCWSTR pwszNewName, HRESULT hrCopy,
		IShellItem* psiNewlyCreated);
	IFACEMETHODIMP PostMoveItem(DWORD dwFlags, IShellItem* psiItem,
		IShellItem* psiDestinationFolder, PCWSTR pwszNewName, HRESULT hrCopy,
		IShellItem* psiNewlyCreated);
	IFACEMETHODIMP PostRenameItem(DWORD dwFlags, IShellItem*, PCWSTR, HRESULT, IShellItem*);
	long   _cRef;
	CFileListCtrl* m_pList;
	MyProgress() { _cRef = 1; m_pList = NULL; };
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


