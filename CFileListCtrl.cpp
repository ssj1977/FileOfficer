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

/////////////////////////////////////////////////
//파일 아이콘 처리
#include <map>
#include <vector>
typedef std::map<CString, int> CExtMap; //확장자에 해당하는 이미지맵의 번호를 기억
static CExtMap mapExt;
typedef std::map<CString, CString> CTypeMap;
static CTypeMap mapType;

//기본 이미지맵 번호
#define SI_UNKNOWN 0 //Unknown File Type
#define SI_DEF_DOCUMENT	1 //Default document
#define SI_DEF_APPLICATION 2//Default application
#define SI_FOLDER_CLOSED 3	//Closed folder
#define SI_FOLDER_OPEN 4	//Open folder
#define SI_FLOPPY_514 5	//5 1/4 floppy
#define SI_FLOPPY_35 6	//3 1/2 floppy
#define SI_REMOVABLE 7	//Removable drive
#define SI_HDD 8	//Hard disk drive
#define SI_NETWORKDRIVE 9	//Network drive
#define SI_NETWORKDRIVE_DISCONNECTED 10 //network drive offline
#define SI_CDROM 11	//CD drive
#define SI_RAMDISK 12	//RAM disk
#define SI_NETWORK 13	//Entire network
#define SI_MOUSEGLOBE //14		?
#define SI_MYCOMPUTER 15	//My Computer
#define SI_PRINTMANAGER 16	//Printer Manager
#define SI_NETWORK_NEIGHBORHOOD	17//Network Neighborhood
#define SI_NETWORK_WORKGROUP 18	//Network Workgroup
#define SI_STARTMENU_PROGRAMS 19	//Start Menu Programs
#define SI_STARTMENU_DOCUMENTS 20	//Start Menu Documents
#define SI_STARTMENU_SETTINGS 21	//Start Menu Settings
#define SI_STARTMENU_FIND 22	//Start Menu Find
#define SI_STARTMENU_HELP 23	//Start Menu Help
#define SI_STARTMENU_RUN 24	//Start Menu Run
#define SI_STARTMENU_SUSPEND 25	//Start Menu Suspend
#define SI_STARTMENU_DOCKING 26	//Start Menu Docking
#define SI_STARTMENU_SHUTDOWN 27	//Start Menu Shutdown
#define SI_SHARE 28	//Sharing overlay (hand)
#define SI_SHORTCUT 29	//Shortcut overlay (small arrow)
#define SI_PRINTER_DEFAULT 30	//Default printer overlay (small tick)
#define SI_RECYCLEBIN_EMPTY 31	//Recycle bin empty
#define SI_RECYCLEBIN_FULL 32	//Recycle bin full
#define SI_DUN 33	//Dial-up Network Folder
#define SI_DESKTOP 34	//Desktop
#define SI_CONTROLPANEL 35	//Control Panel
#define SI_PROGRAMGROUPS 36	//Program Group
#define SI_PRINTER 37	//Printer
#define SI_FONT 38	//Font Folder
#define SI_TASKBAR 39	//Taskbar
#define SI_AUDIO_CD 40	//Audio CD
#define SI_TREE 41		//?
#define SI_PCFOLDER 42	//?
#define SI_FAVORITES 43	//IE favorites
#define SI_LOGOFF 44	//Start Menu Logoff
#define SI_FOLDERUPLOAD 45		?
#define SI_SCREENREFRESH 46		?
#define SI_LOCK 47	//Lock
#define SI_HIBERNATE 48	//Hibernate

//For directory change listner
#define IDM_START_DIRWATCH 55010

//해당 파일의 아이콘 정보를 가져온다
int GetFileImageIndex(CString strPath)
{
	SHFILEINFO sfi;
	memset(&sfi, 0x00, sizeof(sfi));
	if (strPath.GetLength() < MAX_PATH)
	{
		SHGetFileInfo((LPCTSTR)strPath, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
	}
	else
	{
		LPITEMIDLIST pidl = CFileListCtrl::GetPIDLfromPath(strPath);
		SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_PIDL);
		CoTaskMemFree(pidl);
	}
	return sfi.iIcon;
}
int GetFileImageIndexFromMap(CString strPath, BOOL bIsDirectory)
{
	if (bIsDirectory) return SI_FOLDER_CLOSED; 
	if (APP()->m_bUseFileIcon == FALSE) return SI_UNKNOWN;

	CString strExt = Get_Ext(strPath, bIsDirectory, TRUE);
	if (strExt.CompareNoCase(_T(".exe")) == 0
		|| strExt.CompareNoCase(_T(".ico")) == 0
		|| strExt.CompareNoCase(_T(".lnk")) == 0)
	{
		//확장자가 같아도 아이콘이 다를 수 있는 파일들은 바로 조회
		return GetFileImageIndex(strPath);
	}
	//나머지 파일에 대해서는 맵에서 우선 찾아서 속도 향상
	CExtMap::iterator it = mapExt.find(strExt);
	int nImage = 0;
	if (it == mapExt.end())
	{
		//미디어 파일의 경우 네트워크 드라이브에서 경우에 따라 오류가 나는 경우가 많음
		//위의 exe등을 제외하고는 모두 로컬에서 임시파일을 만들어서 처리하는 방식으로 변경
		//	if (strExt.CompareNoCase(_T(".jpg")) == 0
		//	|| strExt.CompareNoCase(_T(".jpeg")) == 0	
		//{} else nImage = GetFileImageIndex(strPath);
		//맵에 없으면 현재 폴더에 임시 파일을 만들어서 아이콘 식별
		TCHAR dir[256];
		GetCurrentDirectory(256, dir);
		CString strTempPath = L"_tmp00_" + strExt; // 현재 폴더가 리모트일 경우가 있는지 미확인
		HANDLE hFile = CreateFile(strTempPath, GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		nImage = GetFileImageIndex(strTempPath);
		if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
		DeleteFile(strTempPath);
		mapExt.insert(CExtMap::value_type(strExt, nImage));
	}
	else
	{
		nImage = (*it).second;
	}
	return nImage;
}
/////////////////////////////////////////////////

CString GetPathName(CString strPath)
{
	CString strReturn;
	SHFILEINFO sfi = {};
	LPITEMIDLIST pidl = NULL;
	if (strPath.IsEmpty())
	{
		SHGetKnownFolderIDList(FOLDERID_ComputerFolder, 0, NULL, &pidl); //'내 컴퓨터' 이름을 가져올떄
	}
	else //if (strPath.GetLength() < MAX_PATH)
	{
		pidl = CFileListCtrl::GetPIDLfromPath(strPath);
	}
	if (pidl == NULL)
	{
		if (SHGetFileInfo((LPCTSTR)strPath, -1, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME))
			strReturn = sfi.szDisplayName;
	}
	else
	{
		if (SHGetFileInfo((LPCTSTR)pidl, -1, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME))
			strReturn = sfi.szDisplayName;
		CoTaskMemFree(pidl);
	}
	if (strReturn.IsEmpty()) strReturn = strPath;
	return strReturn;
}

CString GetPathType(CString strPath)
{
	CString strReturn;
	SHFILEINFO sfi = {};
	if (strPath.GetLength() < MAX_PATH)
	{
		if (SHGetFileInfo(strPath, -1, &sfi, sizeof(sfi), SHGFI_TYPENAME))
			strReturn = sfi.szTypeName;
	}
	else
	{
		LPITEMIDLIST pidl = NULL;
		if (strPath.IsEmpty()) SHGetFolderLocation(NULL, CSIDL_DRIVES, NULL, 0, &pidl);
		else pidl = CFileListCtrl::GetPIDLfromPath(strPath);
		if (SHGetFileInfo((LPCTSTR)pidl, -1, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_TYPENAME))
			strReturn = sfi.szTypeName;
		CoTaskMemFree(pidl);
	}
	return strReturn;
}

CString GetPathTypeFromMap(CString strPath, BOOL bIsDirectory)
{
	if (bIsDirectory) return _T("");
	CString strType;
	CString strExt = Get_Ext(strPath, FALSE, FALSE);
	CTypeMap::iterator it = mapType.find(strExt);
	if (it == mapType.end())
	{
		strType = GetPathType(strPath);
		mapType.insert(CTypeMap::value_type(strExt, strType));
		return strType;
	}
	return (*it).second;
}


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

IMPLEMENT_DYNAMIC(CFileListCtrl, CMFCListCtrl)

#define COL_NAME 0
#define COL_DRIVENAME 0
#define COL_DATE 1
#define COL_DRIVEPATH 1
#define COL_SIZE 2
#define COL_FREESPACE 2
#define COL_TYPE 3
#define COL_TOTALSPACE 3

#define ITEM_TYPE_DOTS 0
#define ITEM_TYPE_DIRECTORY 1
#define ITEM_TYPE_FILE 2
#define ITEM_TYPE_DRIVE 3
#define ITEM_TYPE_UNC 4
#define ITEM_TYPE_INVALID 5

#define LIST_TYPE_DRIVE 0
#define LIST_TYPE_FOLDER 1
#define LIST_TYPE_UNCSERVER 2

#define COL_COMP_STR 0
#define COL_COMP_PATH 1
#define COL_COMP_SIZE 2

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

static ThreadStatusMap st_mapLoading;
static ThreadStatusMap st_mapWatching;

void CFileListCtrl::SetLoadingStatus(CFileListCtrl* pList, BOOL bLoading)
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
}
void CFileListCtrl::SetWatchingStatus(CFileListCtrl* pList, BOOL bLoading)
{
	SetThreadStatus(st_mapWatching, pList, bLoading);
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

CFileListCtrl::CFileListCtrl()
{
	m_strFolder = L"";
	m_nType = LIST_TYPE_DRIVE;
	CMD_UpdateSortInfo = 0;
	CMD_UpdateTabCtrl = 0;
	CMD_UpdateBar = 0;
	CMD_OpenNewTab = 0;
	m_bAsc = TRUE;
	m_nSortCol = 0 ;
	m_nIconType = SHIL_SMALL;
	m_pThreadLoad = NULL;
	m_pThreadWatch = NULL;
	m_posPathHistory = NULL;
	m_bUpdatePathHistory = TRUE;
	m_bMenuOn = FALSE;
	m_pColorRuleArray = NULL;
//	m_bLoading = FALSE;
	m_bUseFileType = TRUE;
	m_bUseFileIcon = TRUE;
	m_pWatchBuffer = malloc(WATCH_BUFFER_SIZE);
	m_hDirectory = NULL;
	m_hWatchBreak = NULL;
	m_hLoadFinished = NULL;
}

CFileListCtrl::~CFileListCtrl()
{
	CFileListCtrl::DeleteWatchingStatus(this);
	CFileListCtrl::DeleteLoadingStatus(this);
	free(m_pWatchBuffer);
	CloseHandle(m_hLoadFinished);
}

static int CMD_DirWatch = IDM_START_DIRWATCH;

BEGIN_MESSAGE_MAP(CFileListCtrl, CMFCListCtrl)
	ON_WM_SIZE()
	//ON_NOTIFY(HDN_ITEMCLICKA, 0, &CFileListCtrl::OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, &CFileListCtrl::OnHdnItemclick)
	ON_WM_DROPFILES()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, &CFileListCtrl::OnLvnBegindrag)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CFileListCtrl::OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CFileListCtrl::OnLvnItemchanged)
	ON_NOTIFY_REFLECT(NM_RCLICK, &CFileListCtrl::OnNMRClick)
	ON_WM_CLIPBOARDUPDATE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

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

void CFileListCtrl::SetColTexts(int* pStringId, int* pColFmt, int size)
{
	CString strText;
	LVCOLUMN col;
	col.mask = LVCF_TEXT | LVCF_FMT;
	for (int i = 0; i < size; i++)
	{
		BOOL b = strText.LoadString(*(pStringId+i));
		if (b)
		{
			col.pszText = strText.GetBuffer();
			col.fmt = *(pColFmt + i);
			SetColumn(i, &col);
			strText.ReleaseBuffer();
		}
	}

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
		for (int i = 0; i < 4; i++)
		{
			if (m_aColWidth.GetSize() > i) nWidth = m_aColWidth[i];
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
		int string_id[] = { IDS_COL_DRIVE_NAME, IDS_COL_DRIVE_PATH, IDS_COL_FREESPACE_DRIVE, IDS_COL_TOTALSPACE_DRIVE };
		int col_fmt[] = { LVCFMT_LEFT , LVCFMT_LEFT , LVCFMT_RIGHT, LVCFMT_RIGHT };
		SetColTexts(string_id, col_fmt, 4);
	}
	else if (nType == LIST_TYPE_FOLDER)
	{
		int string_id[] = { IDS_COL_NAME_FOLDER, IDS_COL_DATE_FOLDER, IDS_COL_SIZE_FOLDER, IDS_COL_TYPE_FOLDER };
		int col_fmt[] = { LVCFMT_LEFT , LVCFMT_RIGHT , LVCFMT_RIGHT, LVCFMT_LEFT };
		SetColTexts(string_id, col_fmt, 4);
	}
	else if (nType == LIST_TYPE_UNCSERVER)
	{
		int string_id[] = { IDS_COL_NAME_UNC, IDS_COL_EMPTY, IDS_COL_EMPTY, IDS_COL_EMPTY };
		int col_fmt[] = { LVCFMT_LEFT , LVCFMT_LEFT , LVCFMT_LEFT, LVCFMT_LEFT };
		SetColTexts(string_id, col_fmt, 4);
	}
	m_nType = nType;
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

LPITEMIDLIST CFileListCtrl::GetPIDLfromPath(CString strPath)
{
	//경로 길이가 MAX_PATH 보다 짧다면 간단히 끝난다
	if (strPath.GetLength() < MAX_PATH) return ILCreateFromPath(strPath);

	LPITEMIDLIST pidl_result = NULL;
/*	IShellFolder* pisf = NULL;		//테스트 결과 ILCreateFromPath와 속도차는 거의 없었음
	if ((SHGetDesktopFolder(&pisf)) != S_OK) return NULL;
	if (strPath.GetLength() < MAX_PATH)
	{
		if (pisf->ParseDisplayName(NULL, 0, strPath.GetBuffer(0), NULL, &pidl_result, NULL) == S_OK)
		{	
			strPath.ReleaseBuffer();
			pisf->Release();
			return pidl_result;
		}
	}*/
	//경로 길이가 MAX_PATH 이상인 경우
	//상위(폴더)경로 PIDL과 상대경로 PIDL로 쪼개서 만든 후 다시 합친다.
	//이때 상위 폴더 경로에 대해서는 재귀적 호출로 만든다
	CString strParent = Get_Folder(strPath);
	CString strChild = Get_Name(strPath);
	if (strChild.GetLength() < MAX_PATH)
	{
		IShellFolder* pisf = NULL;
		if ((SHGetDesktopFolder(&pisf)) == S_OK)
		{
			LPITEMIDLIST pidl_parent = GetPIDLfromPath(strParent);
			LPITEMIDLIST pidl_child = NULL;
			if (pidl_parent)
			{
				if (pisf->BindToObject(pidl_parent, NULL, IID_IShellFolder, (void**)&pisf) == S_OK)
				{
					if (pisf->ParseDisplayName(NULL, 0, strChild.GetBuffer(0), NULL, &pidl_child, NULL) == S_OK)
					{
						UINT cb1 = ILGetSize(pidl_parent) - sizeof(pidl_parent->mkid.cb);
						UINT cb2 = ILGetSize(pidl_child);
						pidl_result = (LPITEMIDLIST)CoTaskMemAlloc(cb1 + cb2);
						if (pidl_result != NULL)
						{
							CopyMemory(pidl_result, pidl_parent, cb1);
							CopyMemory(((LPSTR)pidl_result) + cb1, pidl_child, cb2);
						}
						CoTaskMemFree(pidl_child);
					}
					strChild.ReleaseBuffer();
				}
				CoTaskMemFree(pidl_parent);
			}
		}
		pisf->Release();
	}
	return pidl_result;
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
		INT_PTR nType = GetItemData(nItem);
		if (nType == ITEM_TYPE_FILE)
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
		else //Folder
		{
			if (bMulti == TRUE || (GetKeyState(VK_CONTROL) & 0xFF00) != 0)
			{   //Open in a new tab
				GetParent()->PostMessage(WM_COMMAND, CMD_OpenNewTab, (DWORD_PTR)this);
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

void CFileListCtrl::ResizeColumns()
{
	CRect rcThis;
	GetClientRect(rcThis);

	/*int nColWidthSum = 0;
	for (int i = 0; i < GetHeaderCtrl().GetItemCount(); i++)
	{
		nColWidthSum += GetColumnWidth(i);
	}
	if (nColWidthSum < rcThis.Width())
	{
		int nW = GetColumnWidth(0);
		SetColumnWidth(0, nW + rcThis.Width() - nColWidthSum);
	}*/
}

CString GetFileSizeString(ULONGLONG nSize)
{
	TCHAR pBuf[100];
	ZeroMemory(pBuf, 100);
	CString strSize, strReturn;
	strSize.Format(_T("%I64u"), nSize);
	int nLen = strSize.GetLength();
	if (nLen < 100)
	{
		int nPos = 0;
		for (int i = 0; i < nLen; i++)
		{
			pBuf[nPos] = strSize.GetAt(i);
			nPos += 1;
			if (i < nLen - 3 && (nLen - i - 1) % 3 == 0)
			{
				pBuf[nPos] = _T(',');
				nPos += 1;
			}
		}
		pBuf[99] = _T('\0');
		strReturn = (LPCTSTR)pBuf;
	}
	else
	{
		strReturn = _T("The size is too large.");
	}
	return strReturn;
}

CString GetDriveSizeString(ULARGE_INTEGER size)
{
	CString str;
	ULONGLONG nSize = size.QuadPart;
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

ULONGLONG Str2Size(CString str)
{
	str.Remove(_T(','));
	ULONGLONG size = _wcstoui64(str, NULL, 10);
	if (str.GetLength() > 2)
	{
		CString strUnit = str.Right(2);
		if (strUnit == _T("GB")) size = size * 1073741824;
		else if (strUnit == _T("MB")) size = size * 1048576;
		else if (strUnit == _T("KB")) size = size * 1024;
	}
	return size;
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


void CFileListCtrl::DisplayFolder_Start(CString strFolder, BOOL bUpdatePathHistory)
{
	if (::IsWindow(m_hWnd) == FALSE) return;
	ClearThread();
	if (IsLoading(this) == TRUE || IsWatching(this) == TRUE) return;
	m_strPrevFolder = m_strFolder;
	m_strFolder = strFolder;
	m_bUpdatePathHistory = bUpdatePathHistory;
	//	if (GetParent()!=NULL && ::IsWindow(GetParent()->GetSafeHwnd()))
	//		GetParent()->PostMessage(WM_COMMAND, CMD_UpdateTabCtrl, (DWORD_PTR)this);
	m_pThreadLoad = AfxBeginThread(DisplayFolder_Thread, this);
}

UINT CFileListCtrl::DisplayFolder_Thread(void* lParam)
{
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY))) return 0;
	//APP()->UpdateThreadLocale();
	CFileListCtrl* pList = (CFileListCtrl*)lParam;
	SetLoadingStatus(pList, TRUE); //외부에서 쓰레드 작동 여부 검사용
	ResetEvent(pList->m_hLoadFinished);
	pList->SetBarMsg(IDSTR(IDS_NOW_LOADING));
	pList->DisplayFolder(pList->m_strFolder, pList->m_bUpdatePathHistory);
	if (IsLoading(pList) == TRUE)
	{   // 중단 없이 정상적으로 끝난 경우 
		// 일반 폴더인 경우 변경사항 모니터링 쓰레드 시작
		if (pList->IsWatchable()) pList->PostMessageW(WM_COMMAND, CMD_DirWatch, 0);
		SetLoadingStatus(pList, FALSE);
	}
	else
	{	//로딩이 중단된 경우
		TRACE(_T("[[[ break loading ]]]\r\n"));
	}
	SetEvent(pList->m_hLoadFinished);
	CoUninitialize();
	return 0;
}

void CFileListCtrl::DisplayFolder(CString strFolder, BOOL bUpdatePathHistory)
{
	//m_bLoading = TRUE;
	//clock_t startTime, endTime;
	// startTime = clock();
	DeleteAllItems();
	//m_setPath.clear();
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
		if (GetParent() != NULL && ::IsWindow(GetParent()->GetSafeHwnd())) GetParent()->PostMessage(WM_COMMAND, CMD_UpdateTabCtrl, (DWORD_PTR)this);
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
				if (nType == DRIVE_REMOVABLE) nImage = SI_REMOVABLE;
				else if (nType == DRIVE_CDROM) nImage = SI_CDROM;
				else if (nType == DRIVE_RAMDISK) nImage = SI_RAMDISK;
				else if (nType == DRIVE_REMOTE) nImage = SI_NETWORKDRIVE;
				else nImage = SI_HDD;
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
				SetItemData(nItem, ITEM_TYPE_DRIVE);
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
			nItem = InsertItem(GetItemCount(), GetPathName(path), GetFileImageIndex(path));
			SetItemText(nItem, COL_DRIVEPATH, path);
			CoTaskMemFree(path);
		}
	}
	else if (PathIsUNCServerW(strFolder))
	{
		m_strFolder = strFolder;
		if (GetParent() != NULL && ::IsWindow(GetParent()->GetSafeHwnd())) GetParent()->PostMessage(WM_COMMAND, CMD_UpdateTabCtrl, (DWORD_PTR)this);
		
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
						SetItemData(nItem, ITEM_TYPE_UNC);
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
		//AddItemByPath으로 로딩 시작 전에 경로 에디트 박스 갱신
		if (GetParent() != NULL && ::IsWindow(GetParent()->GetSafeHwnd())) GetParent()->PostMessage(WM_COMMAND, CMD_UpdateTabCtrl, (DWORD_PTR)this);
		if (AddItemByPath(strFind, FALSE, TRUE, strSelectedFolder) == -1)
		{ // 해당 경로가 존재하지 않는 오류가 발생한 경우
			InsertItem(0, IDSTR(IDS_INVALIDPATH));
			SetItemData(0, ITEM_TYPE_INVALID);
			//m_bValid = FALSE;
		}
		else
		{
			SortCurrentList();
		}
	}
	int nSelected = GetNextItem(-1, LVNI_SELECTED);
	if (nSelected != -1) EnsureVisible(nSelected, FALSE);
	UpdateMsgBar();
	/*
	endTime = clock();
	CString strTemp;
	strTemp.Format(_T("%s %d"), IDSTR(IDS_LOADING_TIME), endTime - startTime);
	SetBarMsg(strTemp);
	*/
	SetBkColor(clrBk);
	SetTextColor(clrText);
	RedrawWindow();
	//m_bLoading = FALSE;
}

void CFileListCtrl::OnSize(UINT nType, int cx, int cy)
{
	CMFCListCtrl::OnSize(nType, cx, cy);
	ResizeColumns();
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
	if (pMsg->message == WM_COMMAND)
	{
		if (pMsg->wParam == CMD_DirWatch && m_strFolder.IsEmpty() == FALSE)
		{
			WatchFolder_Begin();
			return TRUE;
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
	DWORD itemData = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) ? ITEM_TYPE_FILE : ITEM_TYPE_DIRECTORY;
	//시각
	COleDateTime tTemp = COleDateTime(fd.ftLastWriteTime);
	CString strTime = tTemp.Format(_T("%Y-%m-%d %H:%M:%S"));
	//크기
	if (itemData == ITEM_TYPE_FILE)
	{
		ULARGE_INTEGER filesize;
		filesize.HighPart = fd.nFileSizeHigh;
		filesize.LowPart = fd.nFileSizeLow;
		CString strSize = GetFileSizeString(filesize.QuadPart);
		SetItemText(nItem, COL_SIZE, strSize);
		if (bUpdateIcon) //시간이 걸릴수 있고 확장자가 바뀌지 않은 경우 필요가 없으므로 옵션 처리
		{
			//종류
			CString strType = GetPathTypeFromMap(strPath, (itemData == ITEM_TYPE_DIRECTORY));
			SetItemText(nItem, COL_TYPE, strType);
			//아이콘
			int nImage = GetFileImageIndexFromMap(strPath, (itemData == ITEM_TYPE_DIRECTORY));
			SetItem(nItem, COL_NAME, LVIF_IMAGE, NULL, nImage, 0, 0, 0);
		}
	}
	SetItemText(nItem, COL_NAME, Get_Name(strPath));
	SetItemText(nItem, COL_DATE, strTime);
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
	DWORD itemData = 0;
	int nItem = -1;
	size_t nLen = 0;
	ULARGE_INTEGER filesize;
	COleDateTime tTemp;
	BOOL b = TRUE, bIsDir = FALSE;
	CString fullpath;
	CString strDir = Get_Folder(strPath);
	BOOL bSelect = !(strSelectByName.IsEmpty());
	while (b)
	{
		if (bAllowBreak == TRUE && IsLoading(this) == FALSE) // && m_bLoading == FALSE)
		{
			break;
		}
		itemData = ITEM_TYPE_FILE;
		nLen = _tcsclen(fd.cFileName);
		if (nLen == 1 && fd.cFileName[0] == _T('.')) itemData = ITEM_TYPE_DOTS; //Dots
		else if (nLen == 2 && fd.cFileName[0] == _T('.') && fd.cFileName[1] == _T('.')) itemData = ITEM_TYPE_DOTS; //Dots
		if (itemData != ITEM_TYPE_DOTS)
		{
			tTemp = COleDateTime(fd.ftLastWriteTime);
			strDate = tTemp.Format(_T("%Y-%m-%d %H:%M:%S"));
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				itemData = ITEM_TYPE_DIRECTORY;  //Directory
				bIsDir = TRUE;
				strSize.Empty();
			}
			else
			{
				bIsDir = FALSE;
				filesize.HighPart = fd.nFileSizeHigh;
				filesize.LowPart = fd.nFileSizeLow;
				strSize = GetFileSizeString(filesize.QuadPart);
			}
			fullpath = PathBackSlash(strDir) + fd.cFileName;
			BOOL bExist = FALSE;
			if (bCheckExist == TRUE)
			{
				/*CPathSet::iterator it = m_setPath.find(fd.cFileName);
				if (it == m_setPath.end()) m_setPath.insert(fd.cFileName);
				else bExist = TRUE; // 7000개 수준에서도 set을 쓰나 안쓰나 속도에 큰 차이가 없음
				if (bExist == TRUE) // 존재하는 경우 인덱스 찾기*/
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
			}
			if (bExist == FALSE)
			{
				nItem = InsertItem(GetItemCount(), fd.cFileName, GetFileImageIndexFromMap(fullpath, bIsDir));
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
				SetItemData(nItem, itemData);
				SetItemText(nItem, COL_DATE, strDate);
				SetItemText(nItem, COL_SIZE, strSize);
				if (m_bUseFileType == TRUE) SetItemText(nItem, COL_TYPE, GetPathTypeFromMap(fullpath, bIsDir));
				else SetItemText(nItem, COL_TYPE, Get_Ext(fd.cFileName, bIsDir, FALSE));
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
		else
		{
/*			BOOL bDeleted = FALSE;
			int nItem = GetNextItem(-1, LVNI_SELECTED);
			while (nItem != -1)
			{
				bDeleted = DeleteInvalidItem(nItem);
				if (bDeleted == TRUE) nItem -= 1;
				nItem = GetNextItem(nItem, LVNI_SELECTED);
			}*/
		}
	}
}

BOOL CFileListCtrl::IsItemExist(int nItem)
{
	return PathFileExists(GetItemFullPath(nItem));
}

BOOL CFileListCtrl::DeleteInvalidItem(int nItem)
{
	BOOL bDeleted = FALSE;
	if (IsItemExist(nItem) == FALSE)
	{
		bDeleted = DeleteItem(nItem);
//		if (bDeleted == TRUE) m_setPath.erase(GetItemText(nItem, COL_NAME));
	}
	return bDeleted;
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
//				m_setPath.erase(GetItemText(i, COL_NAME));
			}
			else
			{
				TRACE(_T("Delete Failed: %s\n"), strPath);
			}
			return;
		}
	}
}

int CFileListCtrl::CompareItemByType(LPARAM item1, LPARAM item2, int nCol, int nType)
{
	int nRet = 0;
	CString str1 = GetItemText((int)item1, nCol);
	CString str2 = GetItemText((int)item2, nCol);
	if (nType == COL_COMP_STR)
	{
		nRet = StrCmp(str1, str2);
	}
	else if (nType == COL_COMP_PATH)
	{
		DWORD_PTR type1, type2;
		type1 = GetItemData((int)item1);
		type2 = GetItemData((int)item2);
		if (type1 != type2)
		{
			nRet = int(type1 - type2);
		}
		else
		{
			nRet = StrCmpLogicalW(str1.GetBuffer(), str2.GetBuffer());
			str1.ReleaseBuffer();
			str2.ReleaseBuffer();
		}
	}
	else if (nType == COL_COMP_SIZE)
	{
		ULONGLONG size1 = Str2Size(str1);
		ULONGLONG size2 = Str2Size(str2);
		if (size1 == size2) nRet = 0;
		else if (size1 > size2) nRet = 1;
		else if (size1 < size2) nRet = -1;
	}
	return nRet;
}

int CFileListCtrl::OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn)
{
	int nRet = 0;
	if (m_nType == LIST_TYPE_FOLDER)
	{
		if (iColumn == COL_NAME) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_PATH);
		else if (iColumn == COL_DATE) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_STR);
		else if (iColumn == COL_SIZE) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_SIZE);
		else if (iColumn == COL_TYPE) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_STR);
	}
	else if (m_nType == LIST_TYPE_DRIVE)
	{
		if (iColumn == COL_DRIVENAME) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_STR);
		else if (iColumn == COL_DRIVEPATH) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_STR);
		else if (iColumn == COL_FREESPACE) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_SIZE);
		else if (iColumn == COL_TOTALSPACE) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_SIZE);
	}
	else if (m_nType == LIST_TYPE_UNCSERVER)
	{
		if (iColumn == COL_NAME) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_STR);
	}

	return nRet;
}

void CFileListCtrl::SortCurrentList()
{
	Sort(m_nSortCol, m_bAsc);
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


BOOL CFileListCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	BOOL b = CMFCListCtrl::Create(dwStyle, rect, pParentWnd, nID);
	m_hLoadFinished = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (b)
	{
		BOOL bd = m_DropTarget.Register(this);
	}
	return b;
}

struct CListItem
{
	CListItem() { pList = NULL; nIndex = -1; };
	CListItem(CListCtrl* p, int n) { pList = p; nIndex = n; };
	CListCtrl* pList;
	int nIndex;
};
static CArray<CListItem> st_selected;

void CFileListCtrl::ClearPreviousSelection()
{
	for (int i = 0; i < st_selected.GetCount(); i++)
	{
		st_selected[i].pList->SetItemState(st_selected[i].nIndex, 0, LVIS_CUT);
	}
	st_selected.RemoveAll();
}

HGLOBAL CFileListCtrl::GetOleDataForClipboard(int nState)
{
	CStringList aFiles;
	CString strPath;
	size_t uBuffSize = 0;
	ClearPreviousSelection();
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return NULL;
	while (nItem != -1)
	{
		strPath = GetItemFullPath(nItem);
		aFiles.AddTail(strPath);
		SetItemState(nItem, nState, LVIS_CUT);
		st_selected.Add(CListItem(this, nItem));
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
		SetClipboardData(CF_HDROP, hgDrop);
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
	}
}

HRESULT CFileListCtrl::CreateShellItemArrayFromPaths(CStringArray& aPath, IShellItemArray*& shi_array)
{
	HRESULT hr = S_OK;
	std::vector<LPITEMIDLIST> pidl_items;
	for (int i = 0; i < aPath.GetSize(); i++)
	{
		LPITEMIDLIST pidl = CFileListCtrl::GetPIDLfromPath(aPath.GetAt(i));
		if (pidl) pidl_items.push_back(pidl);
	}
	hr = SHCreateShellItemArrayFromIDLists((UINT)pidl_items.size(),
		(LPCITEMIDLIST*)pidl_items.data(), &shi_array);
	for (auto& pid : pidl_items) CoTaskMemFree(pid);
	pidl_items.clear();
	return hr;
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
	BOOL bDeleted = FALSE;
	this->SetRedraw(FALSE);
	while (nItem != -1)
	{
		bDeleted = DeleteInvalidItem(nItem); //실제로 지워졌는지 확인
		if (bDeleted == TRUE) nItem -= 1;
		nItem = GetNextItem(nItem, LVNI_SELECTED);
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
	if (IsLoading(this) == TRUE)
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
	}
}

void CFileListCtrl::OnDestroy()
{ 
	ClearThread();
//	m_setPath.clear();
	CMFCListCtrl::OnDestroy();
}


void CFileListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	//메뉴가 이미 떠있는지 체크하고 표시	
	if (m_bMenuOn == TRUE) return;
	ShowContextMenu(&point);
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

BOOL CFileListCtrl::IsFirstPath()
{
	return m_posPathHistory == m_aPathHistory.GetHeadPosition();
}

BOOL CFileListCtrl::IsLastPath()
{
	return m_posPathHistory == m_aPathHistory.GetTailPosition();
}

BOOL CFileListCtrl::IsRootPath()
{
	return m_strFolder.IsEmpty();
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
		BOOL bIsDir = (GetItemData(nRow) == ITEM_TYPE_FILE) ? FALSE : TRUE;
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

void CFileListCtrl::UpdateMsgBar()
{
	//if (::IsWindow(m_hWnd) == FALSE) return;
	CString strTemp, strInfo;
	int nTotal = GetItemCount();
	int nSelected = GetSelectedCount();
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (m_nType == LIST_TYPE_FOLDER)
	{
		if (nSelected == 1 && GetItemData(nItem) == ITEM_TYPE_FILE)
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
			GetFileSizeString(total_size);
			if (total_size > 0) strInfo.Format(_T(" / %s"), GetFileSizeString(total_size));
		}
	}
	strTemp.Format(_T("%d%s / %d%s%s"), GetItemCount(), (LPCTSTR)IDSTR(IDS_ITEM_COUNT), GetSelectedCount(), (LPCTSTR)IDSTR(IDS_SELECTED_COUNT), strInfo); // , IDSTR(IDS_LOADING_TIME), endTime - startTime);
	SetBarMsg(strTemp);
}

void CFileListCtrl::SetBarMsg(CString strMsg)
{
	m_strBarMsg = strMsg;
	if (CMD_UpdateBar != 0) GetParent()->PostMessage(WM_COMMAND, CMD_UpdateBar, (DWORD_PTR)this);
}

void CFileListCtrl::OnLvnItemchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (IsLoading(this)) return;
	//if (!m_bLoading) return;
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	UpdateMsgBar();
	*pResult = 0;
}

BOOL CFileListCtrl::IsWatchable() //모니터링 가능한 일반적인 폴더인 경우 TRUE 반환
{
	if (m_strFolder.IsEmpty()) return FALSE;
	if (::IsWindow(GetSafeHwnd()) == FALSE) return FALSE;
	if (m_nType != LIST_TYPE_FOLDER) return FALSE;
	if (GetItemCount() == 1 && GetItemData(0) == ITEM_TYPE_INVALID) return FALSE;
	return TRUE;
}
