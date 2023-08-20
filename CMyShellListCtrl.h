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
	int CMD_OpenNewTab;
	int CMD_UpdateSortInfo;
	int CMD_UpdateFromList;
	CString m_strCurrentFolder;
	int m_nIconType;
	CString GetBarString();
	void UpdateMsgBar(int nStringID = 0);
	BOOL m_bLoading;
	
	//�÷��� �� �� ���� ���� ���
	void InitColumns(int nType);
	//void ResizeColumns();
	CUIntArray m_aColWidth;
	int m_nSortColInit; // ó�� �ʱ�ȭ �뵵�θ� ���
	BOOL m_bAscInit; //ó�� �ʱ�ȭ �뵵�θ� ��� 

	//�����丮 ���
	CList<CString> m_aPathHistory;
	POSITION m_posPathHistory;
	void BrowsePathHistory(BOOL bPrevious);
	void AddPathHistory(CString strPath);
	inline BOOL IsFirstPath() { return m_posPathHistory == m_aPathHistory.GetHeadPosition(); };
	inline BOOL IsLastPath() { return m_posPathHistory == m_aPathHistory.GetTailPosition(); };
	inline BOOL IsRootPath() { return IsDesktop(); };

	//���� �� ��������� ������Ʈ�� ���� �߰� �Լ�
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

	// Ž����� ��ȣ Copy & Paste
	void ClearPreviousSelection();
	void ClipBoardExport(BOOL bCut);
	void ClipBoardImport();
	HGLOBAL GetOleDataForClipboard(int nState);
	void ProcessDropFiles(HDROP hDropInfo, BOOL bMove);
	//

	// ������� ����͸��� ������ ó��
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
	HANDLE m_hWatchBreak; //���丮 ����͸��� �ߴ��Ҷ� �ʿ��� �̺�Ʈ
	CWinThread* m_pThreadWatch;
	OVERLAPPED	m_overlap_watch; //�񵿱� IO�� ���� OVERLAPPED��ü ��� ����ü, Callback�� ���� ��ü ������ ����
	HANDLE		m_hDirectory;	//���丮 ������� ����͸��� ���� I/O Handle
	LPVOID		m_pWatchBuffer; // ReadDirectoryChangesW �� ���� ����
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
	// IFileOperationSinkProgress ���� ����
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