#include "pch.h"
#include "FileOfficer.h"
#include "CFileListCtrl.h"
#include <shlwapi.h>
#include <shellapi.h>
#include <lm.h>
#include <atlpath.h>
#include <strsafe.h>
#include "CFileListContextMenu.h"
#include "CDlgInput.h"
#include "EtcFunctions.h"
#include "resource.h"

#pragma comment(lib, "Netapi32.lib")

//For directory change listner
#define IDM_START_DIRWATCH 55010
#define IDM_DISPLAY_PATH 55011


/*void StringArray2szzBuffer(CStringArray& aPath, TCHAR*& pszzBuf)
{
	if (aPath.GetSize() == 0)
	{
		pszzBuf = NULL;
		return;
	}
	//Caculate Buffer Size
	size_t uBufSize = 0;
	for (int i = 0; i < aPath.GetSize(); i++)
	{
		uBufSize += aPath[i].GetLength() + 1; //String + '\0'
	}
	uBufSize += 1; //For the last '\0'
	//Copy into buffer
	pszzBuf = new TCHAR[uBufSize];
	memset(pszzBuf, 0, uBufSize * sizeof(TCHAR));
	TCHAR* pBufPos = pszzBuf;
	for (int i = 0; i < aPath.GetSize(); i++)
	{
		lstrcpy(pBufPos, (LPCTSTR)aPath[i]);
		pBufPos = 1 + _tcschr(pBufPos, _T('\0'));
	}
}*/

// IFileOperation 에서 변경된 파일명을 받아오기 위한 IFileOperationProgressSink 구현

IFACEMETHODIMP MyProgress::PostCopyItem(DWORD dwFlags, IShellItem* psiItem,
	IShellItem* psiDestinationFolder, PCWSTR pwszNewName, HRESULT hrCopy,
	IShellItem* psiNewlyCreated)
{
	CString strPath = PathBackSlash(m_pList->m_strFolder, TRUE) + pwszNewName;
	if (m_pList)
	{
		m_pList->AddItemByPath(strPath, TRUE, FALSE);
		m_pList->UpdateMsgBar();
	}
	return S_OK;
}
IFACEMETHODIMP MyProgress::PostMoveItem(DWORD dwFlags, IShellItem* psiItem,
	IShellItem* psiDestinationFolder, PCWSTR pwszNewName, HRESULT hrCopy,
	IShellItem* psiNewlyCreated)
{
	CString strPath = PathBackSlash(m_pList->m_strFolder, TRUE) + pwszNewName;
	if (m_pList) m_pList->AddItemByPath(strPath, TRUE, FALSE);
	return S_OK;
}
IFACEMETHODIMP MyProgress::PostRenameItem(DWORD dwFlags, IShellItem* psiItem, 
	PCWSTR pszNewName, HRESULT hrRename, IShellItem* psiNewlyCreated)
{ 
	if (m_pList == NULL) return S_OK;
	LPWSTR pszOldName = NULL;
	if (SUCCEEDED(psiItem->GetDisplayName(SIGDN_PARENTRELATIVEEDITING, &pszOldName)))
	{
		if (pszOldName != NULL)
		{
			size_t nOldSize = _tcslen(pszOldName);
			size_t nNewSize = _tcslen(pszNewName);
			size_t nCompareSize = (nOldSize > nNewSize) ? nOldSize : nNewSize;
			if (_tcsnicmp(pszOldName, pszNewName, nCompareSize) != 0)
			{
				m_pList->UpdateItemByPath(pszOldName, pszNewName, TRUE);
			}
			CoTaskMemFree(pszOldName);
		}
	}
	return S_OK;
}

IFACEMETHODIMP MyProgress::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit[] =
	{
		QITABENT(CFileListCtrl, IFileOperationProgressSink),
		{0},
	};
	return QISearch(this, qit, riid, ppv);
}
IFACEMETHODIMP_(ULONG) MyProgress::AddRef()
{ 
	return InterlockedIncrement(&_cRef); 
}
IFACEMETHODIMP_(ULONG) MyProgress::Release()
{
	ULONG cRef = InterlockedDecrement(&_cRef);
	if (0 == cRef)	delete this;
	return cRef;
}

// CFileListCtrl

#define NUM_OF_COLUMNS 5
#define COL_NAME 0
#define COL_DRIVENAME 0
#define COL_DATE 1
#define COL_DRIVEPATH 1
#define COL_SIZE 2
#define COL_FREESPACE 2
#define COL_TYPE 3
#define COL_TOTALSPACE 3
#define COL_MEMO 4

#define LIST_TYPE_INVALID -1
#define LIST_TYPE_DRIVE 0
#define LIST_TYPE_FOLDER 1
#define LIST_TYPE_UNCSERVER 2


//쓰레드의 상태를 관리하기 위한 static 변수
typedef std::map<CFileListCtrl*, BOOL> ThreadStatusMap;

static void SetThreadStatus(ThreadStatusMap& mythread, CFileListCtrl* pList, BOOL bLoading)
{
	ThreadStatusMap::iterator it = mythread.find(pList);
	if (it == mythread.end())	mythread.insert(ThreadStatusMap::value_type(pList, bLoading));
	else						mythread.at(pList) = bLoading;
}

static BOOL IsThreadOn(ThreadStatusMap& mythread, CFileListCtrl* pList)
{
	ThreadStatusMap::iterator it = mythread.find(pList);
	if (it == mythread.end())	return FALSE;
	else						return (*it).second;
}

static void DeleteThreadStatus(ThreadStatusMap& mythread, CFileListCtrl* pList)
{
	ThreadStatusMap::iterator it = mythread.find(pList);
	if (it != mythread.end())
	{
		mythread.erase(pList);
	}
}

//static ThreadStatusMap st_mapLoading;
static ThreadStatusMap st_mapWatching;

/*void CFileListCtrl::SetLoadingStatus(CFileListCtrl* pList, BOOL bLoading)
{
	SetThreadStatus(st_mapLoading, pList, bLoading);
	//pList->m_bLoading = bLoading;
}
BOOL CFileListCtrl::IsLoading(CFileListCtrl* pList)
{
	return IsThreadOn(st_mapLoading, pList);
}
void CFileListCtrl::DeleteLoadingStatus(CFileListCtrl* pList)
{
	DeleteThreadStatus(st_mapLoading, pList);
}*/

void CFileListCtrl::SetWatchingStatus(CFileListCtrl* pList, BOOL bWatching)
{
	SetThreadStatus(st_mapWatching, pList, bWatching);
}
BOOL CFileListCtrl::IsWatching(CFileListCtrl* pList)
{
	return IsThreadOn(st_mapWatching, pList);
}
void CFileListCtrl::DeleteWatchingStatus(CFileListCtrl* pList)
{
	DeleteThreadStatus(st_mapWatching, pList);
}

#define WATCH_BUFFER_SIZE 32 * 1024 //네트워크 드라이브에서 버퍼 크기가 64KB 이상이 되면 오류발생(패킷 크기 제한 때문)

IMPLEMENT_DYNAMIC(CFileListCtrl, CFileListCtrl_Base)
BEGIN_MESSAGE_MAP(CFileListCtrl, CFileListCtrl_Base)
	ON_WM_SIZE()
	//ON_NOTIFY(HDN_ITEMCLICKA, 0, &CFileListCtrl::OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, &CFileListCtrl::OnHdnItemclick)
	ON_WM_DROPFILES()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, &CFileListCtrl::OnLvnBegindrag)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CFileListCtrl::OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CFileListCtrl::OnLvnItemchanged)
	ON_NOTIFY_REFLECT(NM_RCLICK, &CFileListCtrl::OnNMRClick)
	ON_WM_CLIPBOARDUPDATE()
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


CFileListCtrl::CFileListCtrl()
{
	m_strFolder = L"";
	m_nType = LIST_TYPE_DRIVE;
	CMD_UpdateSortInfo = 0;
	CMD_UpdateFromList = 0;
	CMD_UpdateBar = 0;
	CMD_OpenNewTabByList = 0;
	m_bAsc = TRUE;
	m_nSortCol = 0 ;
	//m_pThreadLoad = NULL;
	m_pThreadWatch = NULL;
	m_posPathHistory = NULL;
	m_bUpdatePathHistory = TRUE;
	m_pColorRuleArray = NULL;
	m_bLoading = FALSE;
	m_pWatchBuffer = malloc(WATCH_BUFFER_SIZE);
	m_hDirectory = NULL;
	m_hWatchBreak = NULL;
	m_bMenuOn = FALSE;
	//m_hLoadFinished = NULL;
}

CFileListCtrl::~CFileListCtrl()
{
	CFileListCtrl::DeleteWatchingStatus(this);
	//CFileListCtrl::DeleteLoadingStatus(this);
	free(m_pWatchBuffer);
	//CloseHandle(m_hLoadFinished);
}

//static int CMD_DirWatch = IDM_START_DIRWATCH;

CString CFileListCtrl::GetCurrentFolder()
{
	return m_strFolder;
}

CString CFileListCtrl::GetCurrentItemPath()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return _T("");
	return GetItemFullPath(nItem);
}

int CFileListCtrl::GetNameColumnIndex()
{
	return (m_nType == LIST_TYPE_DRIVE) ? COL_DRIVEPATH : COL_NAME;
}

void CFileListCtrl::InitColumns(int nType)
{
	int nIconWidth = 0;
	switch (m_nIconType)
	{
	case SHIL_SMALL: nIconWidth = 16; break;
	case SHIL_LARGE: nIconWidth = 32; break;
	case SHIL_EXTRALARGE: nIconWidth = 48; break;
	case SHIL_JUMBO: nIconWidth = 256; break;
	}
	int nCount = GetHeaderCtrl().GetItemCount();
	if (nCount == 0)
	{
		int nWidth = 0;
		for (int i = 0; i < NUM_OF_COLUMNS; i++)
		{
			if (m_aColWidth.GetSize() > i)
			{
				nWidth = m_aColWidth[i];
			}
			else
			{
				if (i == 0) nWidth = nIconWidth + 400;
				else nWidth = 200;
			}
			InsertColumn(i, _T(""), LVCFMT_LEFT, nWidth);
		}
	}
	//for (int i = nCount - 1; i >= 0; i--) DeleteColumn(i);
	if (nType == LIST_TYPE_DRIVE)
	{
		int string_id[] = { IDS_COL_DRIVE_NAME, IDS_COL_DRIVE_PATH, IDS_COL_FREESPACE_DRIVE, IDS_COL_TOTALSPACE_DRIVE, IDS_COL_EMPTY };
		int col_fmt[] = { LVCFMT_LEFT , LVCFMT_LEFT , LVCFMT_RIGHT, LVCFMT_RIGHT, LVCFMT_LEFT };
		int sort_type[] = { COL_COMP_DRIVE , COL_COMP_STR , COL_COMP_SIZE, COL_COMP_SIZE, COL_COMP_STR };
		SetColTexts(string_id, col_fmt, sort_type, NUM_OF_COLUMNS);
	}
	else if (nType == LIST_TYPE_FOLDER)
	{
		int string_id[] = { IDS_COL_NAME_FOLDER, IDS_COL_DATE_FOLDER, IDS_COL_SIZE_FOLDER, IDS_COL_TYPE_FOLDER, IDS_COL_MEMO };
		int col_fmt[] = { LVCFMT_LEFT , LVCFMT_RIGHT , LVCFMT_RIGHT, LVCFMT_LEFT, LVCFMT_LEFT };
		int sort_type[] = { COL_COMP_PATH, COL_COMP_STR , COL_COMP_SIZE, COL_COMP_STR, COL_COMP_STR };
		SetColTexts(string_id, col_fmt, sort_type, NUM_OF_COLUMNS);
	}
	else if (nType == LIST_TYPE_UNCSERVER)
	{
		int string_id[] = { IDS_COL_NAME_UNC, IDS_COL_EMPTY, IDS_COL_EMPTY, IDS_COL_EMPTY, IDS_COL_EMPTY };
		int col_fmt[] = { LVCFMT_LEFT , LVCFMT_LEFT , LVCFMT_LEFT, LVCFMT_LEFT, LVCFMT_LEFT };
		int sort_type[] = { COL_COMP_STR , COL_COMP_STR , COL_COMP_STR, COL_COMP_STR, COL_COMP_STR };
		SetColTexts(string_id, col_fmt, sort_type, NUM_OF_COLUMNS);
	}
}

CString CFileListCtrl::GetItemFullPath(int nItem)
{
	if (m_nType == LIST_TYPE_FOLDER || m_nType == LIST_TYPE_UNCSERVER)
	{
		CString strPath;
		strPath = PathBackSlash(m_strFolder) + GetItemText(nItem, COL_NAME);
		return strPath;
	}
	else if (m_nType == LIST_TYPE_DRIVE)
	{
		return GetItemText(nItem, COL_DRIVEPATH);
	}
	return _T("");
}

void CFileListCtrl::OpenSelectedItem()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	BOOL bMulti = FALSE;
	if ((GetKeyState(VK_SHIFT) & 0xFF00) != 0) bMulti = TRUE;
	HRESULT hr = S_OK;
	IShellFolder* pisf = NULL;
	//루트(데스크탑)의 IShellFolder 인터페이스 얻어오기
	if (FAILED(SHGetDesktopFolder(&pisf))) return;
	while (nItem != -1)
	{
		if (m_nType == LIST_TYPE_FOLDER && IsDir(nItem) == FALSE) //일반 파일
		{
			LPITEMIDLIST pidl = GetPIDLfromPath(GetItemFullPath(nItem));
			if (pidl != NULL)
			{
				SHELLEXECUTEINFO sei;
				memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
				sei.fMask = SEE_MASK_IDLIST;
				sei.cbSize = sizeof(SHELLEXECUTEINFO);
				sei.lpVerb = _T("open");
				sei.lpFile = NULL;
				sei.lpIDList = pidl;
				sei.nShow = SW_SHOW;
				if (ShellExecuteEx(&sei) == FALSE)
				{
					AfxMessageBox(L"Shell Execution Error"); //Resource
				}
				CoTaskMemFree(pidl);
			}
		}
		else //Folder or Drive
		{
			if (bMulti == TRUE || (GetKeyState(VK_CONTROL) & 0xFF00) != 0)
			{   //Open in a new tab
				GetParent()->SendMessage(WM_COMMAND, CMD_OpenNewTabByList, (DWORD_PTR)this);
			}
			else
			{
				DisplayFolder_Start(GetItemFullPath(nItem));
			}
		}
		if (bMulti == TRUE) nItem = GetNextItem(nItem, LVNI_SELECTED);
		else nItem = -1;
	}
	pisf->Release();
}


CString GetParentFolder(CString strFolder)
{
	if (strFolder.IsEmpty()) return strFolder;
	strFolder = PathBackSlash(strFolder, FALSE);
	int nPos = strFolder.ReverseFind(_T('\\'));
	if (nPos <= 0) return _T("");
	return strFolder.Left(nPos);
}

void CFileListCtrl::OpenParentFolder()
{
	if (m_strFolder.IsEmpty()) return;
	if (PathIsUNCServerW(m_strFolder))
	{
		return;
	}
	else
	{
		m_strFilterExclude.Empty();
		m_strFilterInclude.Empty();
		DisplayFolder_Start(GetParentFolder(m_strFolder));
	}
}

CString GetFileSizeString2(ULONGLONG nSize)
{
	CString str;
	if (nSize > 1073741824)  // 2^30 = GB
	{
		nSize = nSize / 1073741824;
		str.Format(_T("%I64uGB"), nSize);
	}
	else if (nSize > 1048576) // 2^20 = MB
	{
		nSize = nSize / 1048576;
		str.Format(_T("%I64uMB"), nSize);
	}
	else if (nSize > 1024) // 2^10 = KB
	{
		nSize = nSize / 1024;
		str.Format(_T("%I64uKB"), nSize);
	}
	else
	{
		str.Format(_T("%I64uB"), nSize);
	}
	return str;
}

CString GetDriveSizeString(ULARGE_INTEGER size)
{
	ULONGLONG nSize = size.QuadPart;
	return GetFileSizeString2(nSize);
}


void CFileListCtrl::WatchEventHandler()
{
	TCHAR szFile[MAX_PATH];
	VOID* pBuffer = m_pWatchBuffer;
	if (pBuffer == NULL) return;
	FILE_NOTIFY_INFORMATION* pNotify = (FILE_NOTIFY_INFORMATION*)pBuffer;
	size_t offset = 0; //버퍼 iteration 용 offset;
	CString strPath;
	BOOL bRename = FALSE;
	do
	{
		pNotify = (FILE_NOTIFY_INFORMATION*)((BYTE*)pBuffer + offset);
		if (lstrcpynW(szFile, pNotify->FileName, min(MAX_PATH, pNotify->FileNameLength / sizeof(WCHAR) + 1)) != NULL)
		{
			switch (pNotify->Action)
			{
			case FILE_ACTION_ADDED:
				TRACE(L"Added : %s\n", szFile);
				strPath = PathBackSlash(m_strFolder, TRUE) + szFile;
				AddItemByPath(strPath, TRUE, FALSE);
				UpdateMsgBar();
				break;
			case FILE_ACTION_REMOVED:
				TRACE(L"Removed : %s\n", szFile);
				strPath = PathBackSlash(m_strFolder, TRUE) + szFile;
				DeleteInvalidPath(strPath);
				UpdateMsgBar();
				break;
			case FILE_ACTION_MODIFIED:
				strPath = PathBackSlash(m_strFolder, TRUE) + szFile;
				if (PathFileExists(strPath))
				{
					TRACE(L"Modified : %s\n", szFile);
					UpdateItemByPath(strPath, strPath, TRUE);
				}
				break;
			case FILE_ACTION_RENAMED_OLD_NAME:
				TRACE(L"Renamed_Old : %s\n", szFile);
				strPath = szFile;
				bRename = TRUE;
				break;
			case FILE_ACTION_RENAMED_NEW_NAME:
				if (bRename == TRUE)
				{
					TRACE(L"Renamed_New : %s\n", szFile);
					int nItem = -1;
					//오피스 파일의 저장과정 등 임시파일의 생성과 삭제가 빨리 진행되면 제대로 처리되지 않는 문제
					//해당 문제를 해결하기 위해 일단 이름을 먼저 바꾸고 나머지 업데이트
					CString& strOldName = strPath;
					CString strNewName = szFile;
					for (int i = 0; i < GetItemCount(); i++)
					{
						CString strTemp = GetItemText(i, 0);
						if (GetItemText(i, 0).CompareNoCase(strOldName) == 0)
						{
							nItem = i;
							break;
						}
					}
					if (nItem != -1)
					{
						SetItemText(nItem, COL_NAME, strNewName); //이름부터
						if (Get_Ext(strOldName).CompareNoCase(Get_Ext(strNewName)) != 0)
						{	//확장자가 바뀐 경우에 대한 처리, 이미 목록을 갱신했으므로 강제 업데이트
							UpdateItem(nItem,_T(""), TRUE);
						}
					}
					bRename = FALSE;
				}
				break;
			}
		}
		offset += pNotify->NextEntryOffset;
	} while (pNotify->NextEntryOffset != 0);
}


void CFileListCtrl::WatchFolder_Suspend() // 별도 쓰레드 방식용
{
	if (m_pThreadWatch == NULL || IsWatching(this) == FALSE) return;
	DWORD dwCount = m_pThreadWatch->SuspendThread();
	if (dwCount > 1)
	{
		TRACE(_T("Too many suspended thread: %d\r\n"), dwCount);
	}
}

void CFileListCtrl::WatchFolder_Resume() // 별도 쓰레드 방식용
{
	if (m_pThreadWatch == NULL || IsWatching(this) == FALSE) return;
	DWORD dwCount = m_pThreadWatch->ResumeThread();
	if (dwCount > 1)
	{
		TRACE(_T("Too many suspended thread: %d\r\n"), dwCount);
	}
}


void CFileListCtrl::WatchFolder_End() // 별도 쓰레드 방식용
{
	if (IsWatching(this) == FALSE) return;
	HANDLE hThread = m_pThreadWatch->m_hThread;
	if (m_hWatchBreak != NULL ) SetEvent(m_hWatchBreak);
	DWORD ret = WaitForSingleObject(hThread, 3000);
	if (ret != WAIT_OBJECT_0)
	{
		AfxMessageBox(L"Error:Watching thread is not cleared properly");
	}
	m_pThreadWatch = NULL;
}

void CFileListCtrl::WatchFolder_Work() // 별도 쓰레드 방식용
{
	if (IsWatching(this) == TRUE) return;
	CString strDir = PathBackSlash(m_strFolder);
	m_hDirectory = CreateFile(strDir, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
		OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);
	DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
		FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
		FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;
	OVERLAPPED* pOverlapped = &m_overlap_watch;
	if (pOverlapped->hEvent == NULL) pOverlapped->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hWatchBreak == NULL) m_hWatchBreak = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (pOverlapped->hEvent == NULL || m_hWatchBreak == NULL) return;
	HANDLE hEvents[2] = { pOverlapped->hEvent, m_hWatchBreak };
	
	SetWatchingStatus(this, TRUE);
	FILE_NOTIFY_INFORMATION* pNotify = NULL;
	while (TRUE)
	{
		if (ReadDirectoryChangesW(m_hDirectory, m_pWatchBuffer, WATCH_BUFFER_SIZE
			, FALSE, dwNotifyFilter, NULL, pOverlapped, NULL) != FALSE)
		{
			DWORD dwWait = WaitForMultipleObjectsEx(2, hEvents, FALSE, INFINITE, TRUE);
			if (dwWait == WAIT_OBJECT_0)
			{
				WatchEventHandler();
				/*DWORD NumberOfBytesTransferred = 0;
				BOOL bOK = GetOverlappedResult(m_hDirectory, pOverlapped, &NumberOfBytesTransferred, FALSE);
				if (bOK != FALSE) WatchEventHandler();
				else
				{
					DWORD err = GetLastError();
					if (err == ERROR_IO_INCOMPLETE)	TRACE(_T("ERROR_IO_INCOMPLETE: %d\r\n"), err);
					else				TRACE(_T("GetOverlappedResult: %d\r\n"), err);
				}*/
			}
			else if (dwWait == (WAIT_OBJECT_0 + 1))
			{
				ResetEvent(m_hWatchBreak);
				break;
			}
		}
		else break;
	}
	if (m_hDirectory != NULL)
	{
		CancelIo(m_hDirectory);
		if (!HasOverlappedIoCompleted(&m_overlap_watch))
		{
			SleepEx(5, TRUE);
		}
		if (m_overlap_watch.hEvent)
		{
			CloseHandle(m_overlap_watch.hEvent);
			m_overlap_watch.hEvent = NULL;
			CloseHandle(m_hWatchBreak);
			m_hWatchBreak = NULL;
		}
		CloseHandle(m_hDirectory);
		m_hDirectory = NULL;
	}
	SetWatchingStatus(this, FALSE);
}

void CFileListCtrl::WatchFolder_Begin()
{
	if (IsWatchable() == FALSE) return;
	if (IsWatching(this)) return;
	m_pThreadWatch = AfxBeginThread(WatchFolder_Thread, this);
}

UINT CFileListCtrl::WatchFolder_Thread(void* lParam)
{
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY))) return 0;
	CFileListCtrl* pList = (CFileListCtrl*)lParam;
	pList->WatchFolder_Work();
	CoUninitialize();
	return 0;
}

COLORREF GetDimColor(COLORREF clr)
{
	COLORREF clrDim = clr;
	BYTE R = (BYTE)(clr);
	BYTE G = (BYTE)(((WORD)(clr)) >> 8);
	BYTE B = (BYTE)((clr) >> 16);
	if (R > 100) R = int((float)R * 0.7);
	else R = R + 50;
	if (G > 100) G = int((float)G * 0.7);
	else G = G + 50;
	if (B > 100) B = int((float)B * 0.7);
	else B = B + 50;
	return RGB(R, G, B);
}

void CFileListCtrl::RefreshList()
{
	DisplayFolder_Start(m_strFolder, FALSE);
}

void CFileListCtrl::DisplayFolder_Start(CString strFolder, BOOL bUpdatePathHistory)
{
	if (::IsWindow(m_hWnd) == FALSE) return;
	//ClearThread();
	WatchFolder_End();
	if (m_bLoading == TRUE || IsWatching(this) == TRUE) return;
	m_strPrevFolder = m_strFolder;
	m_strFolder = strFolder;
	m_bUpdatePathHistory = bUpdatePathHistory;
	UpdateMsgBar(IDS_NOW_LOADING);
	COLORREF crBk = GetBkColor(); //나중에 복구하기 위해 저장
	COLORREF crText = GetTextColor(); //나중에 복구하기 위해 저장
	SetBkColor(GetDimColor(crBk));
	SetTextColor(GetDimColor(crText));
	//m_pThreadLoad = AfxBeginThread(DisplayFolder_Thread, this);
#ifdef _DEBUG
	clock_t s = clock();
#endif
	m_bLoading = TRUE;
	LoadFolder(m_strFolder, m_bUpdatePathHistory);
	DisplayPathItems();
	m_bLoading = FALSE;
#ifdef _DEBUG
	clock_t e = clock();
	TRACE(L"%s / Display Time : %d\n", m_strFolder, e - s);
#endif
	UpdateMsgBar();
	SetBkColor(crBk);
	SetTextColor(crText);
	RedrawWindow();
	WatchFolder_Begin();	//if (IsWatchable()) PostMessageW(WM_COMMAND, CMD_DirWatch, 0);
}

/*UINT CFileListCtrl::DisplayFolder_Thread(void* lParam)
{
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY))) return 0;
	//APP()->UpdateThreadLocale();
	CFileListCtrl* pList = (CFileListCtrl*)lParam;
	SetLoadingStatus(pList, TRUE); //외부에서 쓰레드 작동 여부 검사용
	ResetEvent(pList->m_hLoadFinished);
	pList->LoadFolder(pList->m_strFolder, pList->m_bUpdatePathHistory);
	pList->DisplayPathItems();
	SetEvent(pList->m_hLoadFinished);
	//pList->PostMessageW(WM_COMMAND, IDM_DISPLAY_PATH, 0);
	CoUninitialize();
	//pList->DisplayFolder(pList->m_strFolder, pList->m_bUpdatePathHistory);
	//SetEvent(pList->m_hLoadFinished);
	//SetLoadingStatus(pList, FALSE);
	//if (pList->IsWatchable()) pList->PostMessageW(WM_COMMAND, CMD_DirWatch, 0);
	//CoUninitialize();
	return 0;
}*/

//로딩과 표시를 구분할 필요가 있음, 로딩은 쓰레드로, 표시는 일반 작업으로
void CFileListCtrl::LoadFolder(CString strFolder, BOOL bUpdatePathHistory)
{
	if (bUpdatePathHistory) AddPathHistory(strFolder);
	m_aPathItem.RemoveAll();
	//필터 초기화
	m_strFilterInclude = L"";
	m_strFilterExclude = L"";
	if (strFolder.IsEmpty())
	{
		m_nType = LIST_TYPE_DRIVE;
		//strFolder가 빈 값 = 루트이므로 모든 드라이브와 특수 폴더(다운로드 등) 표시
		m_strFolder = strFolder;
		//드라이브 
		DWORD drives = GetLogicalDrives();
		DWORD flag = 1;
		int nItem = 0, nImage = 0;
		UINT nType = 0;
		TCHAR c = _T('A');
		CString strDrive;
		ULARGE_INTEGER space_free, space_total;
		for (int i = 0; i < 32; i++)
		{
			if (drives & flag)
			{
				strDrive = (TCHAR)(c + i);
				strDrive += _T(":");
				nType = GetDriveType(strDrive);
				nImage = GetDriveImageIndex(nType);
				//nItem = InsertItem(GetItemCount(), strDrive, nImage);
				//SetItemText(nItem, COL_ALIAS, GetPathName(strDrive));
				// LIST_TYPE_DRIVE 인 경우에는 첫번째 컬럼에 이름 / 두번째 컬럼에 경로
				PathItem pi;
				pi.dwData = nType;
				pi.nIconIndex = nImage;
				pi.str0 = GetPathName(strDrive);
				pi.str1 = strDrive;
				if (GetDiskFreeSpaceEx(strDrive, NULL, &space_total, &space_free))
				{
					pi.str2 = GetDriveSizeString(space_free);
					pi.str3 = GetDriveSizeString(space_total);
				}
				m_aPathItem.Add(pi);
			}
			flag = flag * 2;
		}
		//특수폴더 표시
		KNOWNFOLDERID folder_ids[] = {
			FOLDERID_Desktop, FOLDERID_Downloads, FOLDERID_Documents,
			FOLDERID_Pictures, FOLDERID_Music, FOLDERID_Videos };
		int nCount = sizeof(folder_ids) / sizeof(KNOWNFOLDERID);
		for (int i = 0; i < nCount; i++)
		{
			TCHAR* path = NULL;
			SHGetKnownFolderPath(folder_ids[i], 0, NULL, &path); //바탕화면 경로 가져오기.
			DWORD dwAttribute = GetFileAttributes(path);
			int nImageIndex = GetFileImageIndex(path, INVALID_FILE_ATTRIBUTES); //특수폴더이므로 아이콘을 직접 가져온다
			m_aPathItem.Add(PathItem(dwAttribute, nImageIndex, GetPathName(path), path));
			CoTaskMemFree(path);
		}
	}
	else if (PathIsUNCServerW(strFolder))
	{
		m_strFolder = strFolder;
		m_nType = LIST_TYPE_UNCSERVER;
		//if (GetParent() != NULL && ::IsWindow(GetParent()->GetSafeHwnd())) GetParent()->PostMessage(WM_COMMAND, CMD_UpdateFromList, (DWORD_PTR)this);
		PSHARE_INFO_0 pBuffer, pTemp;
		NET_API_STATUS res;
		DWORD er = 0, tr = 0, resume = 0, i;
		LPTSTR lpszServer = strFolder.GetBuffer();
		int nItem = 0;
		do
		{
			res = NetShareEnum(lpszServer, 0, (LPBYTE*)&pBuffer, MAX_PREFERRED_LENGTH, &er, &tr, &resume);
			if (res == ERROR_SUCCESS || res == ERROR_MORE_DATA)
			{
				pTemp = pBuffer;
				for (i = 1; i <= er; i++)
				{
					CString strTemp = pTemp->shi0_netname;
					if (strTemp != "IPC$")
					{
						m_aPathItem.Add(PathItem(0, GetDriveImageIndex(DRIVE_REMOTE), strTemp));
					}
					pTemp++;
				}
				NetApiBufferFree(pBuffer);
			}
		} while (res == ERROR_MORE_DATA);
		strFolder.ReleaseBuffer();
	}
	else
	{
		m_nType = LIST_TYPE_FOLDER;
		CString strFind = strFolder;
		if (strFind.Find(L'*') == -1 && strFind.Find(L'?') == -1)
		{
			m_strFolder = strFolder;
			strFind = PathBackSlash(strFolder) + _T("*");
			m_strFilterInclude = L"*";
			m_strFilterExclude = L"";
		}
		else
		{
			strFind = strFolder;
			m_strFolder = Get_Folder(strFolder, TRUE);
			m_strFilterInclude = Get_Name(strFolder);
			m_strFilterExclude = L"";
		}
		//////////////////////////////////////////
		WIN32_FIND_DATA fd;
		HANDLE hFind;
		clock_t starttime = clock();
		hFind = FindFirstFileExW(strFind, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			int nCount = 0;
			CString strSize, strDate, strType, strMemo;
			DWORD dwItemData = 0;
			size_t nLen = 0;
			ULARGE_INTEGER filesize;
			COleDateTime tTemp;
			BOOL b = TRUE, bIsDir = FALSE;
			CString fullpath;
			CString strDir = Get_Folder(strFind);
			BOOL bIsDot = FALSE;
			while (b)
			{
				if (m_bLoading == FALSE) break;
				dwItemData = fd.dwFileAttributes;
				nLen = _tcsclen(fd.cFileName);
				if ((nLen == 1 && fd.cFileName[0] == _T('.'))
					|| (nLen == 2 && fd.cFileName[0] == _T('.') && fd.cFileName[1] == _T('.')))
				{
					bIsDot = TRUE; //Dots
				}
				else
				{
					bIsDot = FALSE;
				}
				if (bIsDot == FALSE)
				{
					tTemp = COleDateTime(fd.ftLastWriteTime);
					strDate = tTemp.Format(_T("%Y-%m-%d %H:%M:%S"));
					if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						bIsDir = TRUE;
						strSize.Empty();
					}
					else
					{
						bIsDir = FALSE;
						filesize.HighPart = fd.nFileSizeHigh;
						filesize.LowPart = fd.nFileSizeLow;
						strSize = GetFileSizeString(filesize.QuadPart, 0);
					}
					fullpath = PathBackSlash(strDir) + fd.cFileName;
					int nIconIndex = GetFileImageIndexFromMap(fullpath, fd.dwFileAttributes);
					strType = GetPathTypeFromMap(fullpath, bIsDir, m_bUseFileType);
					strMemo = GetPathMemo(fullpath, dwItemData, m_bCheckOpen);
					m_aPathItem.Add(PathItem(fd.dwFileAttributes, nIconIndex, fd.cFileName, strDate, strSize, strType, strMemo));
				}
				b = FindNextFileW(hFind, &fd);
			}
			FindClose(hFind);
		}
		clock_t endtime = clock();
		TRACE(L"%s(%d) Loading Time : %d\n", strFolder, m_aPathItem.GetSize(), endtime - starttime);
	}
}

void CFileListCtrl::DisplayPathItems()
{
	DeleteAllItems();
	if (m_nType == LIST_TYPE_INVALID) return;
	//새로운 폴더가 기존 폴더의 상위 폴더인지 체크
	CString strSelectedFolder; //새로운 폴더가 기존 폴더의 상위 폴더라면 여기에 기본으로 선택할 기존 폴더 저장
	if (m_strPrevFolder.GetLength() > m_strFolder.GetLength()) //예) c:\test\temp => c:\test
	{
		if (m_strPrevFolder.MakeLower().Find(m_strFolder.MakeLower()) != -1) //새로운 폴더 경로가 기존 폴더에 포함
		{	//기본 선택할 항목 찾기 
			strSelectedFolder = m_strPrevFolder;
			CString strParent = GetParentFolder(m_strPrevFolder);
			//여러 단계 상위 폴더로 바로 이동하는 경우에도 찾을 수 있도록 탐색
			while (strParent.IsEmpty() == FALSE)
			{
				if (strParent.CompareNoCase(m_strFolder) == 0)
				{
					strSelectedFolder = Get_Name(strSelectedFolder);
					break;
				}
				strSelectedFolder = strParent;
				strParent = GetParentFolder(strParent);
			}
		}
	}
	//경로 에디트 박스 갱신
	if (GetParent() != NULL && ::IsWindow(GetParent()->GetSafeHwnd())) GetParent()->PostMessage(WM_COMMAND, CMD_UpdateFromList, (DWORD_PTR)this);
	//필터 초기화
	InitColumns(m_nType);
	int nCount = (int)m_aPathItem.GetSize();
	int nItem = -1;
	BOOL bSelect = strSelectedFolder.IsEmpty() ? FALSE : TRUE ;
	for (int i = 0; i < nCount; i++)
	{
		PathItem& pi = m_aPathItem.GetAt(i);
		nItem = InsertItem(i, pi.str0, pi.nIconIndex);
		if (pi.str1.IsEmpty() == FALSE) SetItemText(nItem, 1, pi.str1);
		if (pi.str2.IsEmpty() == FALSE) SetItemText(nItem, 2, pi.str2);
		if (pi.str3.IsEmpty() == FALSE) SetItemText(nItem, 3, pi.str3);
		if (pi.str4.IsEmpty() == FALSE) SetItemText(nItem, 4, pi.str4);
		SetItemData(nItem, pi.dwData);
		if (bSelect == TRUE) 
		{
			if (strSelectedFolder.CompareNoCase(pi.str0) == 0)
			{
				SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				bSelect = FALSE; //한번만 선택
			}
		}
	}
	Sort(m_nSortCol, m_bAsc);
	int nSelected = GetNextItem(-1, LVNI_SELECTED);
	if (nSelected != -1) EnsureVisible(nSelected, FALSE);
}

/*
void CFileListCtrl::DisplayFolder(CString strFolder, BOOL bUpdatePathHistory)
{
	//m_bLoading = TRUE;
	//clock_t startTime, endTime;
	// startTime = clock();
	DeleteAllItems();
	COLORREF clrBk = GetBkColor();
	COLORREF clrText = GetTextColor();
	COLORREF clrBk2 = GetDimColor(clrBk);
	COLORREF clrText2 = GetDimColor(clrText);
	SetBkColor(clrBk2);
	SetTextColor(clrText2);
	RedrawWindow();
	if (bUpdatePathHistory) AddPathHistory(strFolder);
	//새로운 폴더가 기존 폴더의 상위 폴더인지 체크
	CString strSelectedFolder; //새로운 폴더가 기존 폴더의 상위 폴더라면 여기에 기본으로 선택할 기존 폴더 저장
	if (m_strPrevFolder.GetLength() > m_strFolder.GetLength()) //예) c:\test\temp => c:\test
	{
		if (m_strPrevFolder.MakeLower().Find(m_strFolder.MakeLower()) != -1) //새로운 폴더 경로가 기존 폴더에 포함
		{	//기본 선택할 항목 찾기 
			strSelectedFolder = m_strPrevFolder;
			CString strParent = GetParentFolder(m_strPrevFolder);
			//여러 단계 상위 폴더로 바로 이동하는 경우에도 찾을 수 있도록 탐색
			while (strParent.IsEmpty() == FALSE)
			{
				if (strParent.CompareNoCase(m_strFolder) == 0)
				{
					strSelectedFolder = Get_Name(strSelectedFolder);
					break; 
				}
				strSelectedFolder = strParent; 
				strParent = GetParentFolder(strParent);
			}
		}
	}
	//필터 초기화
	m_strFilterInclude = L"";
	m_strFilterExclude = L"";
	if (strFolder.IsEmpty()) 
	{
		//strFolder가 빈 값 = 루트이므로 모든 드라이브와 특수 폴더(다운로드 등) 표시
		m_strFolder = strFolder;
		if (GetParent() != NULL && ::IsWindow(GetParent()->GetSafeHwnd())) GetParent()->PostMessage(WM_COMMAND, CMD_UpdateFromList, (DWORD_PTR)this);
		//드라이브 
		InitColumns(LIST_TYPE_DRIVE);
		DWORD drives = GetLogicalDrives();
		DWORD flag = 1;
		int nItem = 0, nImage = 0;
		UINT nType = 0;
		TCHAR c = _T('A');
		CString strDrive;
	 	ULARGE_INTEGER space_free, space_total;
		for (int i = 0; i < 32; i++)
		{
			if (drives & flag)
			{
				strDrive = (TCHAR)(c + i);
				strDrive += _T(":");
				nType = GetDriveType(strDrive);
				nImage = GetDriveImageIndex(nType);
				//nItem = InsertItem(GetItemCount(), strDrive, nImage);
				//SetItemText(nItem, COL_ALIAS, GetPathName(strDrive));
				// LIST_TYPE_DRIVE 인 경우에는 첫번째 컬럼에 이름 / 두번째 컬럼에 경로
				nItem = InsertItem(GetItemCount(), GetPathName(strDrive), nImage);
				SetItemText(nItem, COL_DRIVEPATH, strDrive);
				if (GetDiskFreeSpaceEx(strDrive, NULL, &space_total, &space_free))
				{
					SetItemText(nItem, COL_FREESPACE, GetDriveSizeString(space_free));
					SetItemText(nItem, COL_TOTALSPACE, GetDriveSizeString(space_total));
				}
				//SetItemData(nItem, ITEM_TYPE_DRIVE);
				if (strSelectedFolder.CompareNoCase(strDrive) == 0)
				{
					SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}

			}
			flag = flag * 2;
		}
		//특수폴더 표시
		KNOWNFOLDERID folder_ids[] = { 
			FOLDERID_Desktop, FOLDERID_Downloads, FOLDERID_Documents, 
			FOLDERID_Pictures, FOLDERID_Music, FOLDERID_Videos};
		int nCount = sizeof(folder_ids) / sizeof (KNOWNFOLDERID);
		for (int i=0; i<nCount; i++)
		{ 
			TCHAR* path = NULL;
			SHGetKnownFolderPath(folder_ids[i], 0, NULL, &path); //바탕화면 경로 가져오기.
			nItem = InsertItem(GetItemCount(), GetPathName(path), GetFileImageIndex(path, GetFileAttributes(path)));
			SetItemText(nItem, COL_DRIVEPATH, path);
			CoTaskMemFree(path);
		}
	}
	else if (PathIsUNCServerW(strFolder))
	{
		m_strFolder = strFolder;
		if (GetParent() != NULL && ::IsWindow(GetParent()->GetSafeHwnd())) GetParent()->PostMessage(WM_COMMAND, CMD_UpdateFromList, (DWORD_PTR)this);
		
		InitColumns(LIST_TYPE_UNCSERVER);
		PSHARE_INFO_0 pBuffer, pTemp;
		NET_API_STATUS res;
		DWORD er = 0, tr = 0, resume = 0, i;
		LPTSTR lpszServer = strFolder.GetBuffer();
		int nItem = 0;
		do
		{
			res = NetShareEnum(lpszServer, 0, (LPBYTE*)&pBuffer, MAX_PREFERRED_LENGTH, &er, &tr, &resume);
			if (res == ERROR_SUCCESS || res == ERROR_MORE_DATA)
			{
				pTemp = pBuffer;
				for (i = 1; i <= er; i++)
				{
					CString strTemp = pTemp->shi0_netname;
					if (strTemp != "IPC$")
					{
						nItem = InsertItem(GetItemCount(), strTemp, SI_NETWORKDRIVE);
						// SetItemData(nItem, ITEM_TYPE_UNC);
					}
					pTemp++;
				}
				NetApiBufferFree(pBuffer);
			}
		} while (res == ERROR_MORE_DATA);
		strFolder.ReleaseBuffer();
	}
	else
	{
		InitColumns(LIST_TYPE_FOLDER);
		CString strFind = strFolder;
		if (strFind.Find(L'*') == -1 && strFind.Find(L'?') == -1)
		{
			m_strFolder = strFolder;
			strFind = PathBackSlash(strFolder) + _T("*");
			m_strFilterInclude = L"*";
			m_strFilterExclude = L"";
		}
		else
		{
			strFind = strFolder;
			m_strFolder = Get_Folder(strFolder, TRUE);
			m_strFilterInclude = Get_Name(strFolder);
			m_strFilterExclude = L"";
		}
		//로딩 시작 전에 경로 에디트 박스 갱신
		if (GetParent() != NULL && ::IsWindow(GetParent()->GetSafeHwnd())) GetParent()->PostMessage(WM_COMMAND, CMD_UpdateFromList, (DWORD_PTR)this);
		if (AddItemByPath(strFind, FALSE, TRUE, strSelectedFolder) == -1)
		{ // 해당 경로가 존재하지 않는 오류가 발생한 경우
			InsertItem(0, IDSTR(IDS_INVALIDPATH));
			m_nType = LIST_TYPE_INVALID;
		}
		else
		{
			Sort(m_nSortCol, m_bAsc);
		}
	}
	int nSelected = GetNextItem(-1, LVNI_SELECTED);
	if (nSelected != -1) EnsureVisible(nSelected, FALSE);
	UpdateMsgBar();
	SetBkColor(clrBk);
	SetTextColor(clrText);
	RedrawWindow();
	//m_bLoading = FALSE;
}*/

void CFileListCtrl::OnSize(UINT nType, int cx, int cy)
{
	CMFCListCtrl::OnSize(nType, cx, cy);
}

void CFileListCtrl::OnHdnItemclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	Default();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//CDFilesDlg* pParent = (CDFilesDlg*)GetParent();
	//pParent->UpdateSortColumn(GetHeaderCtrl().GetSortColumn(), GetHeaderCtrl().IsAscending());
	SetSortColumn(m_iSortedColumn, m_bAscending);
	m_bAsc = m_bAscending;
	m_nSortCol = m_iSortedColumn;
	GetParent()->PostMessageW(WM_COMMAND, CMD_UpdateSortInfo, (DWORD_PTR)this);
	*pResult = 0;
}


BOOL CFileListCtrl::PreTranslateMessage(MSG* pMsg)
{
	/*if (pMsg->message == WM_LBUTTONDBLCLK)
	{
		if (GetNextItem(-1, LVNI_SELECTED) == -1)
		{
			OpenParentFolder();
			return TRUE;
		}
	}*/
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_F5)
		{
			DisplayFolder_Start(m_strFolder);
			return TRUE;
		}
		if (pMsg->wParam == VK_RETURN)
		{
			OpenSelectedItem();
			return TRUE;
		}
		if (pMsg->wParam == VK_ESCAPE)
		{
			ClearSelected();
			return TRUE;
		}
		if (pMsg->wParam == VK_BACK)
		{
			OpenParentFolder();
			return TRUE;
		}
		if (pMsg->wParam == VK_F2)
		{
			RenameSelectedItem();
			return TRUE;
		}
		if (pMsg->wParam == VK_DELETE)
		{
			if ((GetKeyState(VK_SHIFT) & 0xFF00) != 0) DeleteSelected(FALSE);
			else DeleteSelected(TRUE);
		}
		if ((GetKeyState(VK_CONTROL) & 0xFF00) != 0)
		{
			if (pMsg->wParam == _T('C')) { ClipBoardExport(FALSE); return TRUE; }
			if (pMsg->wParam == _T('X')) { ClipBoardExport(TRUE); return TRUE; }
			if (pMsg->wParam == _T('V')) { ClipBoardImport(); return TRUE; }
			if (pMsg->wParam == _T('A')) 
			{ 
				SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
				return TRUE;
			}
		}
	}
	if (pMsg->message == WM_XBUTTONUP)
	{
		WORD w = HIWORD(pMsg->wParam);
		if (w == XBUTTON2) //Back
		{
			BrowsePathHistory(TRUE);
		}
		else if (w == XBUTTON1) //Forward
		{
			BrowsePathHistory(FALSE);
		}
		return TRUE;
	}
	return CMFCListCtrl::PreTranslateMessage(pMsg);
}

void CFileListCtrl::ProcessDropFiles(HDROP hDropInfo, BOOL bMove)
{
	if (m_nType != LIST_TYPE_FOLDER) return;
	//TCHAR szFilePath[MY_MAX_PATH];
	TCHAR* pszFilePath = new TCHAR[MY_MAX_PATH];
	size_t bufsize = sizeof(TCHAR) * MY_MAX_PATH;
	if (bufsize > 0) ZeroMemory(pszFilePath, bufsize);
	WORD cFiles = DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	int nStart = GetItemCount();
	CStringArray aPath;
	for (int i = 0; i < cFiles; i++)
	{
		DragQueryFile(hDropInfo, i, pszFilePath, MY_MAX_PATH);
		aPath.Add(pszFilePath);
	}
	PasteFiles(aPath, bMove);
	int nEnd = GetItemCount();
	SetItemState(-1, 0, LVIS_SELECTED | LVIS_FOCUSED); // 기존 선택된 항목들 초기화
	for (int i = nStart; i < nEnd; i++)
	{
		SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	if (nStart<nEnd && nEnd>1) EnsureVisible(nEnd-1, FALSE);
	delete[] pszFilePath;
	//DragFinish(hDropInfo); //실제로 마우스 드래그 메시지를 받은 경우에만 이 방식으로 메모리 해제
	//CMFCListCtrl::OnDropFiles(hDropInfo);
}


void CFileListCtrl::PasteFiles(CStringArray& aSrcPath, BOOL bMove)
{
	if (m_strFolder.IsEmpty()) return;
	if (aSrcPath.GetSize() == 0) return;

	BOOL bIsSamePath = FALSE;
	CString strOldFolder = Get_Folder(aSrcPath[0], FALSE); //'\'를 붙여서 추출
	CString strNewFolder = PathBackSlash(m_strFolder, FALSE);
	if (strOldFolder.CompareNoCase(strNewFolder) == 0) bIsSamePath = TRUE;

	IShellItemArray* shi_array = NULL;
	if (CreateShellItemArrayFromPaths(aSrcPath, shi_array) == S_OK)
	{
		IFileOperation* pifo = NULL;
		if (CoCreateInstance(CLSID_FileOperation, NULL,	CLSCTX_ALL, IID_PPV_ARGS(&pifo)) == S_OK)
		{
			IShellItem* pisi = NULL;
			if (SHCreateShellItem(NULL, NULL, GetPIDLfromPath(strNewFolder), &pisi) == S_OK)
			{
				DWORD flag = FOFX_ADDUNDORECORD | FOF_ALLOWUNDO;
				if (bIsSamePath == TRUE) flag = flag | FOF_RENAMEONCOLLISION;
				if (pifo->SetOperationFlags(flag) == S_OK &&
					pifo->SetOwnerWindow(this->GetSafeHwnd()) == S_OK)
				{
					if (bMove)	pifo->MoveItems(shi_array, pisi);
					else		pifo->CopyItems(shi_array, pisi);
					WatchFolder_Suspend();
					SetRedraw(FALSE);
					::ATL::CComPtr<::MyProgress> pSink; //이름이 바뀌었을때 가져오기
					pSink.Attach(new MyProgress{});
					pSink->m_pList = this;
					DWORD dwCookie;
					pifo->Advise(pSink, &dwCookie);
					pifo->PerformOperations();
					pifo->Unadvise(dwCookie);
					pSink.Release();
					SetRedraw(TRUE);
					WatchFolder_Resume();
				}
				if (pisi) pisi->Release();
			}
			if (pifo) pifo->Release();
		}
		if (shi_array) shi_array->Release();
	}
	return;
}

void CFileListCtrl::UpdateItem(int nItem, CString strPath, BOOL bUpdateIcon)
{
	if (strPath.IsEmpty()) strPath = GetItemFullPath(nItem); //strPath는 전체경로
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	hFind = FindFirstFileExW(strPath, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	if (hFind == INVALID_HANDLE_VALUE) return;
	//시각
	COleDateTime tTemp = COleDateTime(fd.ftLastWriteTime);
	CString strTime = tTemp.Format(_T("%Y-%m-%d %H:%M:%S"));
	//크기
	BOOL bIsDir = IsDir(nItem);
	if (bIsDir == FALSE)
	{
		ULARGE_INTEGER filesize;
		filesize.HighPart = fd.nFileSizeHigh;
		filesize.LowPart = fd.nFileSizeLow;
		CString strSize = GetFileSizeString(filesize.QuadPart, 0);
		SetItemText(nItem, COL_SIZE, strSize);
		if (bUpdateIcon) //시간이 걸릴수 있고 확장자가 바뀌지 않은 경우 필요가 없으므로 옵션 처리
		{
			//종류
			CString strType = GetPathTypeFromMap(strPath, bIsDir, m_bUseFileType);
			SetItemText(nItem, COL_TYPE, strType);
			//아이콘
			int nImage = GetFileImageIndexFromMap(strPath, fd.dwFileAttributes);
			SetItem(nItem, COL_NAME, LVIF_IMAGE, NULL, nImage, 0, 0, 0);
		}
	}
	SetItemText(nItem, COL_NAME, Get_Name(strPath));
	SetItemText(nItem, COL_DATE, strTime);
	SetItemText(nItem, COL_MEMO, GetPathMemo(strPath, fd.dwFileAttributes, m_bCheckOpen));
	FindClose(hFind);
}


void CFileListCtrl::UpdateItemByPath(CString strOldPath, CString strNewPath, BOOL bRelativePath, BOOL bForceUpdate)
{
	if (m_nType != LIST_TYPE_FOLDER) return;
	BOOL bSame = FALSE;
	if (strOldPath.Compare(strNewPath) == 0) //대소문자도 구분하여 같을때
	{
		bSame = TRUE;
	}
	CString strOldName = bRelativePath ? strOldPath : Get_Name(strOldPath);
	CString strNewName = bRelativePath ? strNewPath : Get_Name(strNewPath);
	if (bRelativePath == FALSE)
	{
		CString strOldFolder = Get_Folder(strOldPath, TRUE);
		CString strNewFolder = Get_Folder(strNewPath, TRUE);
		if (strOldFolder.CompareNoCase(PathBackSlash(m_strFolder, TRUE)) != 0) return;
		if (strOldFolder.CompareNoCase(strNewFolder) != 0) return;
	}
	else
	{
		strOldPath = PathBackSlash(m_strFolder, TRUE) + strOldName;
		strNewPath = PathBackSlash(m_strFolder, TRUE) + strNewName;
	}
	int nItem = -1, nItemNew = -1;
	for (int i = 0; i < GetItemCount(); i++)
	{
		CString strTemp = GetItemText(i, 0);
		if (strTemp.CompareNoCase(strOldName) == 0)
		{
			nItem = i;
			if (bSame) break; //이름 변경이 없다면 그만 찾는다
		}
		if (bSame == FALSE) //이름 변경이 있다면
		{	//새로운 파일명이 이미 목록에 있는 경우 식별
			if (strTemp.CompareNoCase(strNewName) == 0)
			{
				nItemNew = i;
				if (nItem != -1) break; //양쪽 다 찾은 경우 중단
			}
		}
	}
	if (nItemNew != -1)
	{
		if (nItem == -1)
		{ //원래 파일명은 없고 새로운 파일명은 있는 경우
			nItem = nItemNew;
		}
		else if (nItemNew != nItem)
		{ //원래 파일명과 새로운 파일명이 모두 존재하는 경우
			if (bForceUpdate == FALSE) return; //강제갱신이 아니면 중단
			nItem = nItemNew; //강제갱신인 경우 새로운 아이템을 갱신
		}
		//else  //원래 파일명과 새로운 파일명이 같은 항목인 경우
	}
	if (nItem == -1) return;
	BOOL bUpdateIcon = FALSE;
	if (Get_Ext(strOldName).CompareNoCase(Get_Ext(strNewName)) != 0) bUpdateIcon = TRUE;
	UpdateItem(nItem, strNewPath, bUpdateIcon);
}


int CFileListCtrl::AddItemByPath(CString strPath, BOOL bCheckExist, BOOL bAllowBreak, CString strSelectByName)
{
	//////////////////////////////////////////
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	hFind = FindFirstFileExW(strPath, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	int nCount = 0;
	CString strSize, strDate, strType;
	DWORD dwItemData = 0;
	int nItem = -1;
	size_t nLen = 0;
	ULARGE_INTEGER filesize;
	COleDateTime tTemp;
	BOOL b = TRUE, bIsDir = FALSE;
	CString fullpath;
	CString strDir = Get_Folder(strPath);
	BOOL bSelect = !(strSelectByName.IsEmpty());
	BOOL bIsDot = FALSE;
	while (b)
	{
		if (bAllowBreak == TRUE && m_bLoading == FALSE)
		{
			break;
		}
		dwItemData = fd.dwFileAttributes;
		nLen = _tcsclen(fd.cFileName);
		if ( (nLen == 1 && fd.cFileName[0] == _T('.')) 
			|| (nLen == 2 && fd.cFileName[0] == _T('.') && fd.cFileName[1] == _T('.')))
		{
			bIsDot = TRUE; //Dots
		}
		else
		{
			bIsDot = FALSE;
		}
		if (bIsDot == FALSE)
		{
			tTemp = COleDateTime(fd.ftLastWriteTime);
			strDate = tTemp.Format(_T("%Y-%m-%d %H:%M:%S"));
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				bIsDir = TRUE;
				strSize.Empty();
			}
			else
			{
				bIsDir = FALSE;
				filesize.HighPart = fd.nFileSizeHigh;
				filesize.LowPart = fd.nFileSizeLow;
				strSize = GetFileSizeString(filesize.QuadPart, 0);
			}
			fullpath = PathBackSlash(strDir) + fd.cFileName;
			BOOL bExist = FALSE;
			if (bCheckExist == TRUE)
			{
				for (int i = 0; i < GetItemCount(); i++)
				{
					if (GetItemText(i, COL_NAME).CompareNoCase(fd.cFileName) == 0)
					{
						bExist = TRUE;
						nItem = i;
						break;
					}
				}
			}
			if (bExist == FALSE)
			{
				nItem = InsertItem(GetItemCount(), fd.cFileName, GetFileImageIndexFromMap(fullpath, fd.dwFileAttributes));
				nCount++;
			}
			if (nItem != -1)
			{
				if (bSelect == TRUE)
				{
					if (strSelectByName.CompareNoCase(fd.cFileName) == 0)
					{
						SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						bSelect = FALSE; //한번만
					}
				}
				SetItemData(nItem, dwItemData);
				SetItemText(nItem, COL_DATE, strDate);
				SetItemText(nItem, COL_SIZE, strSize);
				SetItemText(nItem, COL_TYPE, GetPathTypeFromMap(fullpath, bIsDir, m_bUseFileType));
				SetItemText(nItem, COL_MEMO, GetPathMemo(fullpath, dwItemData, m_bCheckOpen));
			}
		}
		b = FindNextFileW(hFind, &fd);
	}
	FindClose(hFind);
	return nCount;
}

void CFileListCtrl::OnLvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	NM_LISTVIEW* pNMListView = pNMLV;
	* pResult = 0;

	HGLOBAL hgDrop = GetOleDataForClipboard(LVIS_CUT);
	if (hgDrop != NULL)
	{
		COleDataSource datasrc;
		FORMATETC etc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		datasrc.CacheGlobalData(CF_HDROP, hgDrop, &etc);
		DROPEFFECT dwEffect = datasrc.DoDragDrop(DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK);
		//if ((dwEffect & DROPEFFECT_LINK) == DROPEFFECT_LINK || (dwEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY) ;
		//else if ((dwEffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE)
		SetItemState(-1, 0, LVIS_CUT);
		if (dwEffect == DROPEFFECT_NONE)
		{
			GlobalFree(hgDrop);
		}
	}
}


void CFileListCtrl::DeleteInvalidPath(CString strPath)
{
	int nCount = GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		if (strPath.CompareNoCase(GetItemFullPath(i)) == 0)
		{
			if (PathFileExists(strPath) == FALSE)
			{
				DeleteItem(i);
				TRACE(_T("Delete Succeeded: %s\n"), strPath);
			}
			else
			{
				TRACE(_T("Delete Failed: %s\n"), strPath);
			}
			return;
		}
	}
}



void CFileListCtrl::Sort(int iColumn, BOOL bAscending, BOOL bAdd)
{
	//virtual 함수, 컬럼 클릭 시에도 호출된다.
	/*if (iColumn == 1) // Using Item Data : File size
	{
		CMFCListCtrl::Sort(iColumn, bAscending, bAdd);
		return;
	}*/
	CWaitCursor wait;
	//SetSortColumn(iColumn, bAscending, bAdd);
	m_nSortCol = iColumn;
	m_bAsc = bAscending;
	m_iSortedColumn = iColumn;
	m_bAscending = bAscending;
	SortItemsEx(CompareProc, (LPARAM)this);
}

void CFileListCtrl::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	OpenSelectedItem();
	*pResult = 0;
}

void CFileListCtrl::OnNMRClick(NMHDR* pNMHDR, LRESULT* pResult)
{
/*	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint pt(pNMItemActivate->ptAction);
	ClientToScreen(&pt);
	ShowContextMenu(pt);*/
	*pResult = 0;
}


BOOL CFileListCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	BOOL b = CMFCListCtrl::Create(dwStyle, rect, pParentWnd, nID);
	//m_hLoadFinished = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (b)
	{
		BOOL b = m_DropTarget.Register(this);
	}
	return b;
}


HGLOBAL CFileListCtrl::GetOleDataForClipboard(int nState)
{
	ListItemArray& aCut = APP()->m_aCutItem;
	CStringList aFiles;
	CString strPath;
	size_t uBuffSize = 0;
	APP()->ClearPreviousCutItems();
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return NULL;
	while (nItem != -1)
	{
		strPath = GetItemFullPath(nItem);
		aFiles.AddTail(strPath);
		SetItemState(nItem, nState, LVIS_CUT);
		aCut.Add(CListItem(this, nItem));
		nItem = GetNextItem(nItem, LVNI_SELECTED);
		uBuffSize += strPath.GetLength() + 1;
	}
	uBuffSize = sizeof(DROPFILES) + sizeof(TCHAR) * (uBuffSize + 1);
	HGLOBAL hgDrop = ::GlobalAlloc(GHND | GMEM_SHARE, uBuffSize);
	if (hgDrop != NULL)
	{
		DROPFILES* pDrop = (DROPFILES*)GlobalLock(hgDrop);;
		if (NULL == pDrop)
		{
			GlobalFree(hgDrop);
			return NULL;
		}
		pDrop->pFiles = sizeof(DROPFILES);
		pDrop->fWide = TRUE;
		TCHAR* pszBuff = NULL;
		POSITION pos = aFiles.GetHeadPosition();
		pszBuff = (TCHAR*)(LPBYTE(pDrop) + sizeof(DROPFILES));
		while (NULL != pos && pszBuff != NULL)
		{
			lstrcpy(pszBuff, (LPCTSTR)aFiles.GetNext(pos));
			pszBuff = _tcschr(pszBuff, _T('\0')) + 1;
		}
		GlobalUnlock(hgDrop);
	}
	return hgDrop;
}


void CFileListCtrl::ClipBoardExport(BOOL bMove)
{
	HGLOBAL hgDrop = GetOleDataForClipboard(bMove ? LVIS_CUT : 0);
	if (hgDrop == NULL) return;
	if (OpenClipboard())
	{
		EmptyClipboard();
		HGLOBAL hEffect = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(DWORD));
		if (hEffect != NULL)
		{
			DROPEFFECT effect = bMove ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
			DWORD* pdw1 = (DWORD*)GlobalLock(hEffect);
			if  (pdw1 != NULL) (*pdw1) = effect;
			GlobalUnlock(hEffect);
			SetClipboardData(RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT), hEffect);
		}
		//텍스트 경로 방식의 클립보드 정보
		SetClipboardData(CF_HDROP, hgDrop);

/*		//쉘ID 방식의 클립보드 정보
		IShellItemArray* shi_array = NULL;
		CStringArray aSrcPath;
		int nItem = GetNextItem(-1, LVNI_SELECTED);
		while (nItem != -1)
		{
			aSrcPath.Add(GetItemFullPath(nItem));
			nItem = GetNextItem(nItem, LVNI_SELECTED);
		}
		// Allocate memory for the shell item identifiers
		const int numItems = aSrcPath.GetSize();
		const int totalSize = (numItems + 1) * sizeof(DWORD); // Include the null terminator
		HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, totalSize);
		LPITEMIDLIST* pShellIDs = (LPITEMIDLIST*)hGlob;

		// Convert file paths to shell item identifiers
		for (int i = 0; i < numItems; ++i)
		{
			SHParseDisplayName(aSrcPath[i], nullptr, &pShellIDs[i], 0, nullptr);
		}

		// Set the Clipboard data format (CFSTR_SHELLIDLIST)
		if (SetClipboardData(RegisterClipboardFormat(CFSTR_SHELLIDLIST), hGlob) == NULL)
		{
			AfxMessageBox(_T("Unable to set Clipboard data"));
			CloseClipboard();
			GlobalFree(hGlob);
			return;
		}*/

		CloseClipboard();
	}


}

void CFileListCtrl::ClipBoardImport()
{
	UINT DROP_EFFECT = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	BOOL bMove = FALSE;
	if (OpenClipboard())
	{
		DWORD* pEffect = NULL;
		HANDLE hMemEffect = NULL;
		HANDLE hMemDropFiles = NULL;
		HDROP hDropInfo = NULL;
		hMemEffect = GetClipboardData(DROP_EFFECT);
		if (hMemEffect)
		{
			pEffect = (DWORD*)GlobalLock(hMemEffect);
			if (pEffect != NULL)
			{
				if (((*pEffect) & DROPEFFECT_MOVE) != 0) bMove = TRUE;
				GlobalUnlock(hMemEffect);
			}
		}
		HGLOBAL hGlobal = (HGLOBAL)GetClipboardData(CF_HDROP);
		if (hGlobal)
		{
			HDROP hDropInfo = (HDROP)GlobalLock(hGlobal);
			ProcessDropFiles(hDropInfo, bMove);
			GlobalUnlock(hGlobal);
		}
		EmptyClipboard();
		CloseClipboard();
		APP()->ClearPreviousCutItems();
	}
}


void CFileListCtrl::DeleteSelected(BOOL bRecycle)
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	CStringArray aPath;
	while (nItem != -1)
	{
		aPath.Add(GetItemFullPath(nItem));
		nItem = GetNextItem(nItem, LVNI_SELECTED);
	}
	IShellItemArray* shi_array = NULL;
	if (CreateShellItemArrayFromPaths(aPath, shi_array) == S_OK)
	{
		IFileOperation* pifo = NULL;
		if (CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pifo)) == S_OK)
		{
			DWORD flag = 0;
			if (bRecycle) flag = flag | FOFX_RECYCLEONDELETE | FOFX_ADDUNDORECORD | FOF_ALLOWUNDO;
			if (pifo->SetOperationFlags(flag) == S_OK &&
				pifo->SetOwnerWindow(this->GetSafeHwnd()) == S_OK)
			{
				pifo->DeleteItems(shi_array);
				WatchFolder_Suspend();
				pifo->PerformOperations();
				WatchFolder_Resume();
			}
			if (pifo) pifo->Release();
		}
		if (shi_array) shi_array->Release();
	}
	//UI에서 삭제하기
	nItem = GetNextItem(-1, LVNI_SELECTED);
	this->SetRedraw(FALSE);
	int nFirstSelected = nItem;
	while (nItem != -1)
	{
		if (IsItemExist(nItem))
		{	//실제로는 안지워진 경우 => 다음 아이템부터 다음 지울 것을 찾는다.
			nItem = GetNextItem(nItem, LVNI_SELECTED);
		}
		else
		{	//지워진 경우 => 해당 아이템부터 다음 지울 것을 찾는다.
			DeleteItem(nItem);
			nItem = GetNextItem(nItem -1 , LVNI_SELECTED);
		}
	}
	//모든 항목이 다 지워진 경우 스크롤이 위로 튀지 않도록 해당 위치를 다시 선택해준다.
	nItem = GetNextItem(- 1, LVNI_SELECTED);
	if (nItem == -1)
	{
		if (nFirstSelected >= GetItemCount()) nFirstSelected = GetItemCount() - 1;
		SetItemState(nFirstSelected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	UpdateMsgBar();
	this->SetRedraw(TRUE);
}

void CFileListCtrl::ClearSelected()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	while (nItem != -1)
	{
		SetItemState(nItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
		nItem = GetNextItem(nItem, LVNI_SELECTED);
	}
}

void CFileListCtrl::RenameSelectedItem()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	CDlgInput dlg;
	BOOL b = dlg.m_strTitle.LoadString(IDS_RENAME);
	dlg.m_strInput = GetItemText(nItem, 0);
	dlg.m_nMode = INPUT_MODE_FILENAME;
	if (dlg.DoModal() != IDOK) return;
	CStringArray aPath;
	while (nItem != -1)
	{
		aPath.Add(GetItemFullPath(nItem));
		nItem = GetNextItem(nItem, LVNI_SELECTED);
	}
	RenameFiles(aPath, dlg.m_strInput); //한번에 다 호출
}

void CFileListCtrl::ConvertNFDNames()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	CStringArray aPath;
	while (nItem != -1)
	{
		int nIndex= (int) aPath.Add(GetItemFullPath(nItem));
		RenameFiles(aPath, ConvertNFD(GetItemText(nItem, 0))); //한개씩 호출
		nItem = GetNextItem(nItem, LVNI_SELECTED);
		aPath.RemoveAll(); 
	}
}

void CFileListCtrl::RenameFiles(CStringArray& aPath, CString strNewPath)
{
	IShellItemArray* shi_array = NULL;
	if (CreateShellItemArrayFromPaths(aPath, shi_array) == S_OK)
	{
		IFileOperation* pifo = NULL;
		if (CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pifo)) == S_OK)
		{
			DWORD flag = FOF_RENAMEONCOLLISION | FOFX_ADDUNDORECORD | FOF_ALLOWUNDO;
			if (pifo->SetOperationFlags(flag) == S_OK &&
				pifo->SetOwnerWindow(this->GetSafeHwnd()) == S_OK)
			{
				pifo->RenameItems(shi_array, strNewPath);
				WatchFolder_Suspend();
				SetRedraw(FALSE);
				::ATL::CComPtr<::MyProgress> pSink; //이름이 바뀌었을때 가져오기
				pSink.Attach(new MyProgress{});
				pSink->m_pList = this;
				DWORD dwCookie;
				pifo->Advise(pSink, &dwCookie);
				pifo->PerformOperations();
				pifo->Unadvise(dwCookie);
				pSink.Release();
				SetRedraw(TRUE);
				WatchFolder_Resume();
			}
			if (pifo) pifo->Release();
		}
		if (shi_array) shi_array->Release();
	}
}

void CFileListCtrl::ClearThread()
{
	if (IsWatching(this) != FALSE && m_pThreadWatch != NULL)
	{
		WatchFolder_End();
	}
/*	if (IsLoading(this) == TRUE)
	{
		SetLoadingStatus(this, FALSE);
		//로딩 쓰레드의 경우 UI를 건드리므로 단순 WaitSingleObject로 하면 UI 메시지 Deadlock 발생
		BOOL bLoop = TRUE;
		MSG msg;
		while (bLoop)
		{
			//UI 메시지를 수동으로 처리하면서 루프를 돈다
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			//CWinThread의 Handle로는 정상작동하지 않으므로 별도의 이벤트 Handle(m_hLoadFinished) 필요
			DWORD dwWait = MsgWaitForMultipleObjects(1, &m_hLoadFinished, FALSE, 3000, QS_ALLEVENTS);
			if (dwWait == WAIT_OBJECT_0)
			{
				bLoop = FALSE; 
			}
			else if (dwWait == WAIT_TIMEOUT)
			{
				AfxMessageBox(L"Timeout:Loading thread was not cleared properly.");
			}
		}
		m_pThreadLoad = NULL;
	}*/
}

void CFileListCtrl::OnDestroy()
{ 
	ClearThread();
	CMFCListCtrl::OnDestroy();
}



void CFileListCtrl::BrowsePathHistory(BOOL bPrevious)
{
	if (m_posPathHistory == NULL || m_aPathHistory.GetSize() < 2) return;
	m_strFilterExclude.Empty();
	m_strFilterInclude.Empty();
	if (bPrevious == TRUE && m_posPathHistory != m_aPathHistory.GetHeadPosition())
	{
		m_aPathHistory.GetPrev(m_posPathHistory);
		CString strFolder =m_aPathHistory.GetAt(m_posPathHistory);
		DisplayFolder_Start(strFolder, FALSE);
	}
	else if (bPrevious == FALSE && m_posPathHistory != m_aPathHistory.GetTailPosition())
	{
		m_aPathHistory.GetNext(m_posPathHistory);
		CString strFolder = m_aPathHistory.GetAt(m_posPathHistory);
		DisplayFolder_Start(strFolder, FALSE);
	}
}

void CFileListCtrl::AddPathHistory(CString strPath)
{
	if (m_aPathHistory.GetSize() > 0)
	{
		while (m_posPathHistory != m_aPathHistory.GetTailPosition())
		{
			m_aPathHistory.RemoveTail();
			if (m_aPathHistory.GetTailPosition() == NULL) break;
		}
	}
	m_posPathHistory = m_aPathHistory.AddTail(strPath);
}

COLORREF CFileListCtrl::ApplyColorRule(int nRow, int nColumn, BOOL bBk)
{
	COLORREF color = bBk ? GetBkColor() : GetTextColor();
//	return color;
	if (m_pColorRuleArray)
	{
		int nCount = (int)((ColorRuleArray*)m_pColorRuleArray)->GetSize();
		BOOL bMatch = FALSE;
		BOOL bSetName = FALSE, bSetExt = FALSE, bSetDate = FALSE; // 처음 한번만 값을 세팅하기 위해 사용
		BOOL bValidDate = FALSE; //날짜값 검증결과 기억용
		CString strName, strExt, strDate;
		COleDateTime dt;
		COleDateTime today = COleDateTime::GetCurrentTime();
		COleDateTimeSpan differ;
		BOOL bIsDir = IsDir(nRow);
		//속도를 감안하여 한번만 처리하도록 관련 값은 미리 세팅
		for (int i = 0; i < nCount; i++)
		{
			ColorRule& cr = ((ColorRuleArray*)m_pColorRuleArray)->GetAt(i);
			bMatch = FALSE;
			switch (cr.nRuleType)
			{
			case COLOR_RULE_EXT: //확장자 조건 (대소문자 구분 없음)
				if (bIsDir == FALSE)
				{
					if (bSetExt == FALSE)
					{
						strName = GetItemText(nRow, COL_NAME);
						if (strName.IsEmpty() == FALSE)	strExt = Get_Ext(strName, bIsDir, FALSE);
						bSetExt = TRUE;
					}
					if (strExt.IsEmpty() == FALSE)
					{
						for (int j = 0; j < cr.aRuleOptions.GetSize(); j++)
						{
							if (strExt.CompareNoCase(cr.aRuleOptions.GetAt(j)) == 0)
							{
								bMatch = TRUE;
								break;
							}
						}
					}
				}
				break;
			case COLOR_RULE_FOLDER: //폴더일때
				if (bIsDir != FALSE) bMatch = TRUE;
				break;
			case COLOR_RULE_NAME: //이름에 포함된 문자열(확장자도 포함, 대소문자 구분 있음)
				if (bSetName == FALSE)
				{
					strName = GetItemText(nRow, COL_NAME);
					bSetName = TRUE;
				}
				if (strName.IsEmpty() == FALSE)
				{
					for (int j = 0; j < cr.aRuleOptions.GetSize(); j++)
					{
						if (strName.Find(cr.aRuleOptions.GetAt(j)) != -1)
						{
							bMatch = TRUE;
							break;
						}
					}
				}
				break;
			case COLOR_RULE_DATE: //날짜 범위
				if (bSetDate == FALSE)
				{
					strDate = GetItemText(nRow, COL_DATE);
					if (strDate.IsEmpty() == FALSE)
					{
						if (dt.ParseDateTime(strDate) != FALSE)
						{
							differ = today - dt;
							bValidDate = TRUE;
						}
					}
					bSetDate = TRUE;
				}
				if (bValidDate != FALSE)
				{
					if (differ.GetTotalDays() <= (_ttoi(cr.strRuleOption) - 1)) bMatch = TRUE;
				}
				break;
			case COLOR_RULE_COLNAME: //이를 컬럼 전체 
				if (nColumn == COL_NAME) bMatch = TRUE;
				break;
			case COLOR_RULE_COLDATE: //변경일시 컬럼 전체 
				if (nColumn == COL_DATE) bMatch = TRUE;
				break;
			case COLOR_RULE_COLSIZE: //크기 컬럼 전체 
				if (nColumn == COL_SIZE) bMatch = TRUE;
				break;
			case COLOR_RULE_COLTYPE: //파일 종류 컬럼 전체 
				if (nColumn == COL_TYPE) bMatch = TRUE;
				break;
			}
			if (bMatch != FALSE)
			{
				if (bBk == FALSE && cr.bClrText != FALSE) color = cr.clrText;
				else if (bBk != FALSE && cr.bClrBk != FALSE) color = cr.clrBk;
			}
		}
	}
	return color;
}


COLORREF CFileListCtrl::OnGetCellTextColor(int nRow, int nColumn)
{
	return ApplyColorRule(nRow, nColumn, FALSE);
}

COLORREF CFileListCtrl::OnGetCellBkColor(int nRow, int nColumn)
{
	return ApplyColorRule(nRow, nColumn, TRUE);
}


void CFileListCtrl::UpdateMsgBar(int nStringID)
{
	if (CMD_UpdateBar != 0 && GetParent() != NULL) GetParent()->SendMessage(WM_COMMAND, CMD_UpdateBar, nStringID);
}

void CFileListCtrl::OnLvnItemchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_bLoading) return;
	//if (!m_bLoading) return;
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	UpdateMsgBar();
	*pResult = 0;
}

BOOL CFileListCtrl::IsWatchable() //모니터링 가능한 일반적인 폴더인 경우 TRUE 반환
{
	if (::IsWindow(GetSafeHwnd()) == FALSE) return FALSE;
	if (m_nType != LIST_TYPE_FOLDER) return FALSE;
	if (m_strFolder.IsEmpty()) return FALSE;
	return TRUE;
}

BOOL CFileListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDM_FILE_DELETE:	DeleteSelected(((GetKeyState(VK_SHIFT) & 0xFF00) != 0) ? FALSE : TRUE);		break;
	case IDM_FILE_COPY:		ClipBoardExport(FALSE);		break;
	case IDM_FILE_CUT:		ClipBoardExport(TRUE);		break;
	case IDM_FILE_PASTE:	case IDM_PASTE_FILE:		ClipBoardImport();		break; //툴바 또는 메뉴
	case IDM_CONVERT_NFD:	ConvertNFDNames();		break;
	case IDM_CHECK_LOCKED: UpdateMemo();		break;
	case IDM_OPEN_PREV:		BrowsePathHistory(TRUE); break;
	case IDM_OPEN_NEXT:		BrowsePathHistory(FALSE); break;
	case IDM_PLAY_ITEM:		OpenSelectedItem(); break;
	case IDM_OPEN_PARENT:	OpenParentFolder(); break;
	case IDM_DISPLAY_PATH:	DisplayPathItems(); break;
	//case IDM_START_DIRWATCH: WatchFolder_Begin(); break;
	default:	
		return CMFCListCtrl::OnCommand(wParam, lParam); break;
	}
	return TRUE;
}

CString CFileListCtrl::GetBarString()
{
	if (::IsWindow(GetSafeHwnd()) == FALSE) return _T("");
	CString strReturn, strInfo;
	if (m_nType == LIST_TYPE_FOLDER)
	{
		int nSelected = GetSelectedCount();
		int nItem = GetNextItem(-1, LVNI_SELECTED);
		if (nSelected == 1 && IsDir(nItem) == FALSE)
		{
			// 선택이 한개인 경우 최종 갱신 시점, 크기 표시
			strInfo.Format(_T(" / %s / %s"), GetItemText(nItem, COL_DATE), GetItemText(nItem, COL_SIZE));
		}
		else if (nSelected > 1)
		{
			ULONGLONG total_size = 0;
			while (nItem != -1)
			{
				// 선택이 여러개인 경우 크기 합산하여 출력
				total_size += Str2Size(GetItemText(nItem, COL_SIZE));
				nItem = GetNextItem(nItem, LVNI_SELECTED);
			}
			if (total_size > 0) strInfo.Format(_T(" / %s"), GetFileSizeString(total_size, 0));
		}
		strReturn.Format(_T("%d%s / %d%s%s"), GetItemCount(),
			(LPCTSTR)IDSTR(IDS_ITEM_COUNT), nSelected,
			(LPCTSTR)IDSTR(IDS_SELECTED_COUNT), strInfo);
	}
	else
	{
		strReturn.Format(_T("%d%s"), GetItemCount(), (LPCTSTR)IDSTR(IDS_ITEM_COUNT));
	}
	return strReturn;
}


void CFileListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	//메뉴가 이미 떠있는지 체크하고 표시	
	if (m_bMenuOn == TRUE) return;
	ShowContextMenu(&point);
}

void CFileListCtrl::ShowContextMenu(CPoint* pPoint)
{
	CPoint pt;
	CStringArray aSelectedPath;
	if (pPoint == NULL)
	{	//pPoint가 NULL인 경우 무조건 빈 공간 클릭시 나오는 메뉴로 처리
		GetCursorPos(&pt);
	}
	else
	{	//pPoint가 NULL이 아니라면 현재 선택된 항목을 확인하여 처리
		int nIndex = GetNextItem(-1, LVNI_SELECTED);
		while (nIndex != -1)
		{
			aSelectedPath.Add(GetItemFullPath(nIndex));
			nIndex = GetNextItem(nIndex, LVNI_SELECTED);
		}
		//현재 마우스의 좌표와 pPoint의 좌표를 비교하여 다른 경우
		//주로 키보드의 메뉴 키를 누른 경우에 해당
		//이 경우 pPoint 값이 (-1, -1)로 나오므로 좌표를 다시 계산해야 함
		GetCursorPos(&pt);
		if (pPoint->x != pt.x || pPoint->y != pt.y)
		{
			if (aSelectedPath.GetSize() > 0)
			{	//선택된 항목이 있는 경우에는 첫 항목의 좌표 이용
				nIndex = GetNextItem(-1, LVNI_SELECTED);
				if (nIndex != -1)
				{
					CRect rc;
					GetItemRect(nIndex, rc, LVIR_LABEL);
					ClientToScreen(rc);
					pt.SetPoint(rc.left + 5, rc.bottom - 3);
				}
			}
			//else 선택된 항목이 없는 경우에는 그냥 마우스 좌표 이용
		}
	}
	//현재 마우스의 좌표와 point의 좌표를 비교
	m_bMenuOn = TRUE;
	CFileListContextMenu context_menu;
	context_menu.SetParent(this);
	context_menu.SetPathArray(m_strFolder, aSelectedPath);
	UINT idCommand = context_menu.ShowContextMenu(this, pt);
	if (idCommand) GetParent()->PostMessage(WM_COMMAND, idCommand, 0);
	m_bMenuOn = FALSE;
}

void CFileListCtrl::UpdateMemo()
{
	BOOL bBackup = m_bCheckOpen;
	m_bCheckOpen = TRUE;
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	while (nItem != -1)
	{
		UpdateItem(nItem, _T(""), FALSE);
		nItem = GetNextItem(nItem, LVNI_SELECTED);
	}
	m_bCheckOpen = bBackup;
}