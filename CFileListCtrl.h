#pragma once
#include "CFileListCtrl_Base.h"

/*using namespace std;
#include <set>
typedef set<CString> CPathSet; //중복이름 체크용 맵 해당하는 이미지맵의 번호를 기억*/

struct PathItem
{
	CString str0; //COL_NAME, COL_DRIVENAME
	CString str1; //COL_DATE, COL_DRIVEPATH
	CString str2; //COL_SIZE, COL_FREESPACE
	CString str3; //COL_TYPE, COL_TOTALSPACE
	CString str4; //COL_MEMO
	DWORD dwData;
	int nIconIndex;
	LPITEMIDLIST pidl;

	PathItem()
	{
		dwData = 0;
		nIconIndex = 0;
		pidl = NULL;
	};
	PathItem(DWORD _dwData, int _nIconIndex, CString _str0 = L"", CString _str1 = L"", CString _str2 = L"", CString _str3 = L"", CString _str4=L"")
	{
		this->dwData = _dwData;
		this->nIconIndex = _nIconIndex;
		this->str0 = _str0;
		this->str1 = _str1;
		this->str2 = _str2;
		this->str3 = _str3;
		this->str4 = _str4;
		this->pidl = NULL;
	};
	~PathItem()
	{
		if (pidl) CoTaskMemFree(pidl);
	}
	PathItem(const PathItem& pti)
	{
		this->dwData = pti.dwData;
		this->nIconIndex = pti.nIconIndex;
		this->str0 = pti.str0;
		this->str1 = pti.str1;
		this->str2 = pti.str2;
		this->str3 = pti.str3;
		this->str4 = pti.str4;
		this->pidl = NULL;
		if (pti.pidl) this->pidl = ILClone(pti.pidl);
	}
	PathItem& operator= (const PathItem& pti) //CArray의 CArray를 만들때는 항상 복사 생성자를 오버로딩 해야 함
	{
		this->dwData = pti.dwData;
		this->nIconIndex = pti.nIconIndex;
		this->str0 = pti.str0;
		this->str1 = pti.str1;
		this->str2 = pti.str2;
		this->str3 = pti.str3;
		this->str4 = pti.str4;
		this->pidl = NULL;
		if (pti.pidl) this->pidl = ILClone(pti.pidl);
		return *this;
	};
};
typedef CArray<PathItem> PathItemArray;


class CFileListCtrl : public CFileListCtrl_Base
{
	DECLARE_DYNAMIC(CFileListCtrl)

public:
	CFileListCtrl();
	virtual ~CFileListCtrl();
	void OpenSelectedItem();
	void OpenParentFolder();
	int AddItemByPath(CString strPath, BOOL bCheckExist = FALSE, BOOL bAllowBreak = TRUE, CString strSelectByName = _T(""));
	void UpdateItem(int nItem, CString strPath, BOOL bUpdateIcon);
	void UpdateItemByPath(CString strOldPath, CString strNewPath, BOOL bRelativePath = FALSE, BOOL bForceUpdate = FALSE);
	void InitColumns(int nType);
	CString GetItemFullPath(int nItem);
	CString GetCurrentFolder();
	CString GetCurrentItemPath();
	CString GetBarString();
	void CreateNewFolder();
	void DeleteInvalidPath(CString strPath);
	void PasteFiles(CStringArray& aOldPath, BOOL bMove);
	void ClipBoardExport(BOOL bMove, BOOL bUseShell = FALSE); //Move = Cut 
	void ClipBoardImport();
	HGLOBAL GetOleDataForClipboard(int nState);
	CString m_strFolder;
	CString m_strPrevFolder; //폴더간 이동시 하위 폴더에서 상위폴더로 이동하는 경우 자동으로 해당 하위폴더를 목록 중에서 선택하기 위해 이용
	CString m_strFilterInclude;

	void* m_pColorRuleArray;
	int GetNameColumnIndex();
	inline BOOL IsItemExist(int nItem) { return PathFileExists(GetItemFullPath(nItem)); };
	
	//경로 히스토리 관련
	BOOL m_bUpdateHistory;
	void BrowseHistory(BOOL bPrevious);
	CList<CString> m_aPathHistory;
	POSITION m_posPathHistory;
	void AddPathHistory(CString strPath);
	inline BOOL IsFirstPath() { return m_posPathHistory == m_aPathHistory.GetHeadPosition(); };
	inline BOOL IsLastPath() { return m_posPathHistory == m_aPathHistory.GetTailPosition(); };
	inline BOOL IsRootPath() { return m_strFolder.IsEmpty(); };	

	//상태 확인
	BOOL m_bAsc;
	int m_nSortCol;
	int m_nListType;
	int CMD_UpdateSortInfo;
	int CMD_UpdateFromList;
	int CMD_UpdateBar;
	int CMD_OpenNewTabByList;
	void ProcessDropFiles(HDROP hDropInfo, BOOL bMove);
	void DeleteSelected(BOOL bRecycle);
	void RenameSelectedItem();
	void ConvertNFDNames();
	void UpdateMemo();
	void RenameFiles(CStringArray& aPath, CString strNewPath);
	void ClearSelected();
	
	// 파일목록 불러오기 => 쓰레드를 쓰지 않는 쪽이 안정성과 스피드 면에서 유리
	//CWinThread* m_pThreadLoad;
	//static UINT DisplayFolder_Thread(void* lParam);
	//static void SetLoadingStatus(CFileListCtrl* pList, BOOL bLoading);
	//static BOOL IsLoading(CFileListCtrl* pList);
	//static void DeleteLoadingStatus(CFileListCtrl* pList);
	//HANDLE m_hLoadFinished; //디렉토리 로딩이 끝나거나 중단될때 발생하는 이벤트 핸들
	void RefreshList();
	BOOL m_bLoading; //로딩 중인지 확인용
	void DisplayFolderByPath(CString strFolder, BOOL bUpdateHistory = TRUE);
	void LoadFolder(CString strFolder, BOOL bUpdateHistory);
	void DisplayPathItems();
	PathItemArray m_aPathItem; //처음 추가할때는 배열의 형태로 추가, 추가할때의 배열의 인덱스를 아이템데이터에 할당

	// 변경사항 모니터링용 쓰레드 처리
	BOOL IsWatchable();
	void WatchFolder_Begin();
	void WatchFolder_Work();
	void WatchFolder_End();
	void WatchFolder_Resume();
	void WatchFolder_Suspend();
	void WatchEventHandler();
	static UINT WatchFolder_Thread(void* lParam);
	static void SetWatchingStatus(CFileListCtrl* pList, BOOL Watching);
	static BOOL IsWatching(CFileListCtrl* pList);
	static void DeleteWatchingStatus(CFileListCtrl* pList);
	HANDLE m_hWatchBreak; //디렉토리 모니터링을 중단할때 필요한 이벤트
	CWinThread* m_pThreadWatch;
	OVERLAPPED	m_overlap_watch; //비동기 IO를 위한 OVERLAPPED개체 상속 구조체, Callback을 위한 객체 포인터 포함
	HANDLE		m_hDirectory;	//디렉토리 변경사항 모니터링을 위한 I/O Handle
	LPVOID		m_pWatchBuffer; // ReadDirectoryChangesW 를 위한 버퍼

	void ClearThread(); // 현재 작동중인 쓰레드들을 중단
	void UpdateMsgBar(int nStringID = 0);
	COLORREF ApplyColorRule(int nRow, int nColumn, BOOL bBk);

	//컨텍스트 메뉴 처리용
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	void ShowContextMenu(CPoint* pPoint); //pPoint가 NULL인 경우 현재 마우스 위치로 처리
	void RunShellMenuCommand(UINT idCommand);
	BOOL m_bMenuOn; //컨텍스트 메뉴가 표시되어 있는지를 체크하는 플래그

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHdnItemclick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//virtual int OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn);
	virtual void Sort(int iColumn, BOOL bAscending = TRUE, BOOL bAdd = FALSE);
	afx_msg void OnLvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	afx_msg void OnDestroy();
	virtual COLORREF OnGetCellTextColor(int nRow, int nColumn);
	virtual COLORREF OnGetCellBkColor(int nRow, int nColumn);
	afx_msg void OnLvnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	afx_msg void OnDropFiles(HDROP hDropInfo);
	//afx_msg DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
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


