#pragma once
#include <afxlistctrl.h>
#include "CMyDropTarget.h"

/*using namespace std;
#include <set>
typedef set<CString> CPathSet; //�ߺ��̸� üũ�� �� �ش��ϴ� �̹������� ��ȣ�� ���*/

struct PathItem
{
	CString str0; //COL_NAME, COL_DRIVENAME
	CString str1; //COL_DATE, COL_DRIVEPATH
	CString str2; //COL_SIZE, COL_FREESPACE
	CString str3; //COL_TYPE, COL_TOTALSPACE
	DWORD dwData; 
	int nIconIndex;

	PathItem()
	{
		dwData = 0;
		nIconIndex = 0;
	};
	PathItem(DWORD _dwData, int _nIconIndex, CString _str0 = L"", CString _str1 = L"", CString _str2 = L"", CString _str3 = L"")
	{
		this->dwData = _dwData;
		this->nIconIndex = _nIconIndex;
		this->str0 = _str0;
		this->str1 = _str1;
		this->str2 = _str2;
		this->str3 = _str3;
	};
	PathItem(const PathItem& pti)
	{
		this->dwData = pti.dwData;
		this->nIconIndex = pti.nIconIndex;
		this->str0 = pti.str0;
		this->str1 = pti.str1;
		this->str2 = pti.str2;
		this->str3 = pti.str3;
	}
	PathItem& operator= (const PathItem& pti) //CArray�� CArray�� ���鶧�� �׻� ���� �����ڸ� �����ε� �ؾ� ��
	{
		this->dwData = pti.dwData;
		this->nIconIndex = pti.nIconIndex;
		this->str0 = pti.str0;
		this->str1 = pti.str1;
		this->str2 = pti.str2;
		this->str3 = pti.str3;
		return *this;
	};
};
typedef CArray<PathItem> PathItemArray;


class CFileListCtrl : public CMFCListCtrl
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
	void SetColTexts(int* pStringId, int* pColFmt, int size);
	void ShowContextMenu(CPoint* pPoint); //pPoint�� NULL�� ��� ���� ���콺 ��ġ�� ó��
	CString GetItemFullPath(int nItem);
	CString GetCurrentFolder();
	CString GetCurrentItemPath();
	CString GetBarString();
	void DeleteInvalidPath(CString strPath);
	void PasteFiles(CStringArray& aOldPath, BOOL bMove);
	void ClipBoardExport(BOOL bCut);
	void ClipBoardImport();
	HGLOBAL GetOleDataForClipboard(int nState);
	CString m_strFolder;
	CString m_strPrevFolder; //������ �̵��� ���� �������� ���������� �̵��ϴ� ��� �ڵ����� �ش� ���������� ��� �߿��� �����ϱ� ���� �̿�
	CString m_strFilterInclude;
	CString m_strFilterExclude;

	void* m_pColorRuleArray;
	void BrowsePathHistory(BOOL bPrevious);
	void AddPathHistory(CString strPath);
	BOOL IsDrive(int nItem);
	int GetNameColumnIndex();
	inline BOOL IsItemExist(int nItem) { return PathFileExists(GetItemFullPath(nItem)); };
	inline BOOL IsDir(int nItem) { return (GetItemData(nItem) & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE; };
	

	//��� �����丮 ����
	CList<CString> m_aPathHistory;
	POSITION m_posPathHistory;
	BOOL m_bUpdatePathHistory;
	inline BOOL IsFirstPath() { return m_posPathHistory == m_aPathHistory.GetHeadPosition(); };
	inline BOOL IsLastPath() { return m_posPathHistory == m_aPathHistory.GetTailPosition(); };
	inline BOOL IsRootPath() { return m_strFolder.IsEmpty(); };	
	
	//���� Ȯ��
	BOOL m_bAsc;
	BOOL m_bMenuOn; //���ؽ�Ʈ �޴��� ǥ�õǾ� �ִ����� üũ�ϴ� �÷���
	BOOL m_bUseFileType; //������ ������ �����ϴ� ������ �������� ����, FALSE �̸� Ȯ���ڷ� ��ü, �ӵ��鿡�� ���� ���̰� ����
	BOOL m_bUseFileIcon;
	CUIntArray m_aColWidth;
	int m_nSortCol;
	int m_nType;
	int m_nIconType;
	int CompareItemByType(LPARAM item1, LPARAM item2, int nCol, int nType);
	int CMD_UpdateSortInfo;
	int CMD_UpdateFromList;
	int CMD_UpdateBar;
	int CMD_OpenNewTabByList;
	CMyDropTarget m_DropTarget;
	void ProcessDropFiles(HDROP hDropInfo, BOOL bMove);
	void DeleteSelected(BOOL bRecycle);
	void RenameSelectedItem();
	void ConvertNFDNames();
	void RenameFiles(CStringArray& aPath, CString strNewPath);
	void ClearSelected();
	
	// ���ϸ�� �ҷ����� => �����带 ���� �ʴ� ���� �������� ���ǵ� �鿡�� ����
	//CWinThread* m_pThreadLoad;
	//static UINT DisplayFolder_Thread(void* lParam);
	//static void SetLoadingStatus(CFileListCtrl* pList, BOOL bLoading);
	//static BOOL IsLoading(CFileListCtrl* pList);
	//static void DeleteLoadingStatus(CFileListCtrl* pList);
	//HANDLE m_hLoadFinished; //���丮 �ε��� �����ų� �ߴܵɶ� �߻��ϴ� �̺�Ʈ �ڵ�
	//void DisplayFolder(CString strFolder, BOOL bUpdatePathHistory = TRUE);
	void DisplayFolder_Start(CString strFolder, BOOL bUpdatePathHistory = TRUE);
	BOOL m_bLoading; //�ε� ������ Ȯ�ο�
	void LoadFolder(CString strFolder, BOOL bUpdatePathHistory);
	void DisplayPathItems();
	PathItemArray m_aPathItem;

	// ������� ����͸��� ������ ó��
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
	HANDLE m_hWatchBreak; //���丮 ����͸��� �ߴ��Ҷ� �ʿ��� �̺�Ʈ
	CWinThread* m_pThreadWatch;
	OVERLAPPED	m_overlap_watch; //�񵿱� IO�� ���� OVERLAPPED��ü ��� ����ü, Callback�� ���� ��ü ������ ����
	HANDLE		m_hDirectory;	//���丮 ������� ����͸��� ���� I/O Handle
	LPVOID		m_pWatchBuffer; // ReadDirectoryChangesW �� ���� ����

	void ClearThread(); // ���� �۵����� ��������� �ߴ�
	void UpdateMsgBar(int nStringID = 0);
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
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};

class MyProgress : public IFileOperationProgressSink
{
public :
// IFileOperationSinkProgress ���� ����
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


