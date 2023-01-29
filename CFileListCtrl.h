#pragma once
#include <afxlistctrl.h>
#include "CMyDropTarget.h"

/*using namespace std;
#include <set>
typedef set<CString> CPathSet; //�ߺ��̸� üũ�� �� �ش��ϴ� �̹������� ��ȣ�� ���*/

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
	void ShowContextMenu(CPoint* pPoint); //pPoint�� NULL�� ��� ���� ���콺 ��ġ�� ó��
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
	CString m_strPrevFolder; //������ �̵��� ���� �������� ���������� �̵��ϴ� ��� �ڵ����� �ش� ���������� ��� �߿��� �����ϱ� ���� �̿�
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
	BOOL m_bMenuOn; //���ؽ�Ʈ �޴��� ǥ�õǾ� �ִ����� üũ�ϴ� �÷���
	//BOOL m_bLoading; //IsLoading�� �ߺ������� OnLvnItemchanged�� ���� ó���� ���� ���
	BOOL m_bUseFileType; //������ ������ �����ϴ� ������ �������� ����, FALSE �̸� Ȯ���ڷ� ��ü, �ӵ��鿡�� ���� ���̰� ����
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
	
	//��Ÿ static �Լ���
	static void ClearPreviousSelection(); // �߶󳻱� ����� ������ ���� ó�� �ʱ�ȭ
	static LPITEMIDLIST GetPIDLfromPath(CString strPath); //MAX_PATH�� �ʰ��ϴ� ��ο� ���ؼ��� ó���� �ش�.
	static HRESULT CreateShellItemArrayFromPaths(CStringArray& aPath, IShellItemArray*& shi_array);

	// ���ϸ�� �ҷ����� ������ ó��
	CWinThread* m_pThreadLoad;
	static UINT DisplayFolder_Thread(void* lParam);
	static void SetLoadingStatus(CFileListCtrl* pList, BOOL bLoading);
	static BOOL IsLoading(CFileListCtrl* pList);
	static void DeleteLoadingStatus(CFileListCtrl* pList);

	// ������� ����͸��� ������ ó��
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
	HANDLE m_hWatchBreak; //���丮 ����͸��� �ߴ��Ҷ� �ʿ��� �̺�Ʈ
	CWinThread* m_pThreadWatch;
	OVERLAPPED	m_overlap_watch; //�񵿱� IO�� ���� OVERLAPPED��ü ��� ����ü, Callback�� ���� ��ü ������ ����
	HANDLE		m_hDirectory;	//���丮 ������� ����͸��� ���� I/O Handle
	LPVOID		m_pWatchBuffer; // ReadDirectoryChangesW �� ���� ����

	HANDLE m_hLoadFinished; //���丮 ����͸��� �ߴ��Ҷ� �ʿ��� �̺�Ʈ
	void ClearThread(); // ���� �۵����� ��������� �ߴ�
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


