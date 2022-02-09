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
	SHGetFileInfo((LPCTSTR)strPath, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
	return sfi.iIcon;
}
int GetFileImageIndexFromMap(CString strPath, BOOL bIsDirectory)
{
	if (bIsDirectory)
	{
		//return GetFileImageIndex(_T(""));
		return 3;		// SI_FOLDER_CLOSE
	}
	//CPath path = CPath(strPath);
	//CString strExt = path.GetExtension();
	CString strExt = Get_Ext(strPath, bIsDirectory, TRUE);
	if (strExt.CompareNoCase(_T(".exe")) == 0
		|| strExt.CompareNoCase(_T(".ico")) == 0
		|| strExt.CompareNoCase(_T(".lnk")) == 0
		) return GetFileImageIndex(strPath);
	CExtMap::iterator it = mapExt.find(strExt);
	if (it == mapExt.end())
	{
		int nImage = GetFileImageIndex(strPath);
		mapExt.insert(CExtMap::value_type(strExt, nImage));
		return nImage;
	}
	return (*it).second;
}
/////////////////////////////////////////////////

CString GetPathName(CString strPath)
{
	CString strReturn;
	SHFILEINFO sfi = { 0 };
	LPITEMIDLIST pidl = NULL;
	if (strPath.IsEmpty()) SHGetFolderLocation(NULL, CSIDL_DRIVES, NULL, 0, &pidl);
	else pidl = ILCreateFromPath(strPath);

	if (SHGetFileInfo((LPCTSTR)pidl, -1, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME))
	{
		strReturn = sfi.szDisplayName;
	}
	ILFree(pidl);
	return strReturn;
}

CString GetPathType(CString strPath)
{
	CString strReturn;
	SHFILEINFO sfi = { 0 };
	LPITEMIDLIST pidl = NULL;
	if (strPath.IsEmpty()) SHGetFolderLocation(NULL, CSIDL_DRIVES, NULL, 0, &pidl);
	else pidl = ILCreateFromPath(strPath);

	if (SHGetFileInfo((LPCTSTR)pidl, -1, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_TYPENAME))
	{
		strReturn = sfi.szTypeName;
	}
	ILFree(pidl);
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


void GetPathInfo(CString strPath, CString& strDisplayName, CString& strTypeName)
{
	SHFILEINFO sfi = { 0 };
	LPITEMIDLIST pidl = NULL;
	if (strPath.IsEmpty()) SHGetFolderLocation(NULL, CSIDL_DRIVES, NULL, 0, &pidl);
	else pidl = ILCreateFromPath(strPath);
	if (SHGetFileInfo((LPCTSTR)pidl, -1, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_TYPENAME))
	{
		strDisplayName = sfi.szDisplayName;
		strTypeName = sfi.szTypeName;
	}
	ILFree(pidl);

}


void StringArray2szzBuffer(CStringArray& aPath, TCHAR*& pszzBuf)
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
}

// From https://www.codeproject.com/Articles/950/CDirectoryChangeWatcher-ReadDirectoryChangesW-all

CMyDirectoryChangeHandler::CMyDirectoryChangeHandler(CFileListCtrl* pList)
{
	m_pList = pList;
}

void CMyDirectoryChangeHandler::On_FileAdded(const CString& strFileName)
{
	m_pList->AddItemByPath(strFileName, TRUE, FALSE);
}

void CMyDirectoryChangeHandler::On_FileRemoved(const CString& strFileName)
{
	m_pList->DeleteInvalidPath(strFileName);
}

void CMyDirectoryChangeHandler::On_FileModified(const CString& strFileName)
{
	m_pList->UpdateItemByPath(strFileName, strFileName);
}

void CMyDirectoryChangeHandler::On_FileNameChanged(const CString& strOldFileName, const CString& strNewFileName)
{
	m_pList->UpdateItemByPath(strOldFileName, strNewFileName);
}

void ClearAllNotification();

// CFileListCtrl

IMPLEMENT_DYNAMIC(CFileListCtrl, CMFCListCtrl)

#define COL_NAME 0
#define COL_DATE 1
#define COL_ALIAS 1
#define COL_SIZE 2
#define COL_FREESPACE 2
#define COL_TYPE 3
#define COL_TOTALSPACE 3

#define ITEM_TYPE_DOTS 0
#define ITEM_TYPE_DIRECTORY 1
#define ITEM_TYPE_FILE 2
#define ITEM_TYPE_DRIVE 3
#define ITEM_TYPE_UNC 4

#define LIST_TYPE_DRIVE 0
#define LIST_TYPE_FOLDER 1
#define LIST_TYPE_UNCSERVER 2

#define COL_COMP_STR 0
#define COL_COMP_PATH 1
#define COL_COMP_SIZE 2

typedef std::map<CFileListCtrl*, BOOL> CLoadingMap;
static CLoadingMap st_mapLoading;

void CFileListCtrl::SetLoadingStatus(CFileListCtrl* pList, BOOL bLoading)
{
	CLoadingMap::iterator it = st_mapLoading.find(pList);
	if (it == st_mapLoading.end())
	{
		st_mapLoading.insert(CLoadingMap::value_type(pList, bLoading));
	}
	else
	{
		st_mapLoading.at(pList) = bLoading;
	}
}

BOOL CFileListCtrl::IsLoading(CFileListCtrl* pList)
{
	CLoadingMap::iterator it = st_mapLoading.find(pList);
	if (it == st_mapLoading.end())
	{
		return FALSE;
	}
	else
	{
		return (*it).second;
	}
}

void CFileListCtrl::DeleteLoadingStatus(CFileListCtrl* pList)
{
	CLoadingMap::iterator it = st_mapLoading.find(pList);
	if (it != st_mapLoading.end())
	{
		st_mapLoading.erase(pList);
	}
}

CFileListCtrl::CFileListCtrl()
: m_DirHandler(this) , m_DirWatcher(true)
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
	m_hThreadLoad = NULL;
	m_posPathHistory = NULL;
	m_bUpdatePathHistory = TRUE;
	m_bMenuOn = FALSE;
	//m_bUseFileType = FALSE;
}

CFileListCtrl::~CFileListCtrl()
{
	ClearAllNotification();
}

static int CMD_DirWatch = IDM_START_DIRWATCH;

BEGIN_MESSAGE_MAP(CFileListCtrl, CMFCListCtrl)
	ON_WM_SIZE()
	//ON_NOTIFY(HDN_ITEMCLICKA, 0, &CFileListCtrl::OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, &CFileListCtrl::OnHdnItemclick)
	ON_WM_DROPFILES()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, &CFileListCtrl::OnLvnBegindrag)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CFileListCtrl::OnNMDblclk)
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
		strText.LoadString(*(pStringId+i));
		col.pszText = strText.GetBuffer();
		col.fmt = *(pColFmt+i);
		SetColumn(i, &col);
		strText.ReleaseBuffer();
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
		int string_id[] = { IDS_COL_NAME_DRIVE, IDS_COL_ALIAS_DRIVE, IDS_COL_FREESPACE_DRIVE, IDS_COL_TOTALSPACE_DRIVE };
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
		TCHAR path[MY_MAX_PATH] = {};
		PathCombineW(path, m_strFolder, GetItemText(nItem, COL_NAME));
		return path;
	}
	else if (m_nType == LIST_TYPE_DRIVE)
	{
		return GetItemText(nItem, COL_NAME);
	}
	return _T("");
}

void CFileListCtrl::OpenSelectedItem()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	BOOL bMulti = FALSE;
	if ((GetKeyState(VK_SHIFT) & 0xFF00) != 0) bMulti = TRUE;
	
	while (nItem != -1)
	{
		INT_PTR nType = GetItemData(nItem);
		if (nType == ITEM_TYPE_FILE)
		{
			HINSTANCE hr = ShellExecute(NULL, NULL, GetItemFullPath(nItem), NULL, NULL, SW_SHOW);
			if ((INT_PTR)hr <= 32) AfxMessageBox(L"Shell Execution Error"); //Resource
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
	CPath path = CPath(m_strFolder);
	if (path.IsUNCServer())
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
	CString strSize;
	strSize.Format(_T("%I64u"), nSize);
	int nLen = strSize.GetLength();
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
	return (LPCTSTR)pBuf;
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

void CFileListCtrl::DisplayFolder_Start(CString strFolder, BOOL bUpdatePathHistory)
{
	if (IsLoading(this) == TRUE) return;
	if (::IsWindow(m_hWnd) == FALSE) return;
	ClearThread();
	m_strPrevFolder = m_strFolder;
	m_strFolder = strFolder;
	m_bUpdatePathHistory = bUpdatePathHistory;
//	if (GetParent()!=NULL && ::IsWindow(GetParent()->GetSafeHwnd()))
//		GetParent()->PostMessage(WM_COMMAND, CMD_UpdateTabCtrl, (DWORD_PTR)this);
	AfxBeginThread(DisplayFolder_Thread, this);
}

UINT CFileListCtrl::DisplayFolder_Thread(void* lParam)
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
	//APP()->UpdateThreadLocale();
	CFileListCtrl* pList = (CFileListCtrl*)lParam;
	SetLoadingStatus(pList, TRUE);
	ResetEvent(pList->m_hThreadLoad);
	pList->SetBarMsg(_T("Now Loading..."));
	pList->DisplayFolder(pList->m_strFolder, pList->m_bUpdatePathHistory);
	if (IsLoading(pList) == TRUE)
	{   // 정상적으로 끝난 경우, IsLoading이 FALSE면 중단된 경우
		SetLoadingStatus(pList, FALSE);
		pList->PostMessageW(WM_COMMAND, CMD_DirWatch, 0);
	}
	SetEvent(pList->m_hThreadLoad);
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

void CFileListCtrl::DisplayFolder(CString strFolder, BOOL bUpdatePathHistory)
{
	clock_t startTime, endTime;
	startTime = clock();
	CPath path = CPath(strFolder);
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
	if (strFolder.IsEmpty()) //strFolder가 빈 값 = 루트이므로 모든 드라이브 표시
	{
		m_strFolder = (CString)path;
		if (GetParent() != NULL && ::IsWindow(GetParent()->GetSafeHwnd())) GetParent()->PostMessage(WM_COMMAND, CMD_UpdateTabCtrl, (DWORD_PTR)this);

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
				nItem = InsertItem(GetItemCount(), strDrive, nImage); //GetFileImageIndexFromMap(L"", TRUE)
				SetItemText(nItem, COL_ALIAS, GetPathName(strDrive));
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
	}
	else if (path.IsUNCServer())
	{
		m_strFolder = (CString)path;
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
			m_strFolder = (CString)path;
			path.AddBackslash();
			strFind = path + _T("*");
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
		AddItemByPath(strFind, FALSE, TRUE, strSelectedFolder);
		SortCurrentList();
	}
	int nSelected = GetNextItem(-1, LVNI_SELECTED);
	if (nSelected != -1) EnsureVisible(nSelected, FALSE);
	endTime = clock();
	CString strTemp;
	strTemp.Format(_T("%d Item(s) / Loading Time : %d"), GetItemCount(), endTime - startTime);
	SetBarMsg(strTemp);
	SetBkColor(clrBk);
	SetTextColor(clrText);
	RedrawWindow();
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
		}
	}
	if (pMsg->message == WM_COMMAND)
	{
		if (pMsg->wParam == CMD_DirWatch && m_strFolder.IsEmpty() == FALSE)
		{
			WatchCurrentDirectory(TRUE);
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

void CFileListCtrl::WatchCurrentDirectory(BOOL bOn)
{
	CString strDirectory = PathBackSlash(m_strFolder, TRUE); // "D:" 의 경우 오동작, "D:\"로 하여야 함
	if (bOn == FALSE)
	{
		m_DirWatcher.UnwatchDirectory(strDirectory);
	}
	else
	{
		DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
			FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;
		if (m_DirWatcher.IsWatchingDirectory(strDirectory)) m_DirWatcher.UnwatchDirectory(strDirectory);
		m_DirWatcher.WatchDirectory(strDirectory, dwNotifyFilter, &m_DirHandler, FALSE, m_strFilterInclude, m_strFilterExclude);
	}
}

void CFileListCtrl::ProcessDropFiles(HDROP hDropInfo, BOOL bMove)
{
	if (m_nType != LIST_TYPE_FOLDER) return;
	TCHAR szFilePath[MY_MAX_PATH];
	size_t bufsize = sizeof(TCHAR) * MY_MAX_PATH;
	memset(szFilePath, 0, bufsize);
	WORD cFiles = DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	int nStart = GetItemCount();
	CStringArray aPath;
	for (int i = 0; i < cFiles; i++)
	{
		DragQueryFile(hDropInfo, i, szFilePath, MY_MAX_PATH);
		aPath.Add(szFilePath);
	}
	PasteFiles(aPath, bMove);
	int nEnd = GetItemCount();
	for (int i = nStart; i < nEnd; i++)
	{
		SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	if (nStart<nEnd && nEnd>1) EnsureVisible(nEnd-1, FALSE);
	//DragFinish(hDropInfo); //실제로 마우스 드래그 메시지를 받은 경우에만 이 방식으로 메모리 해제
	//CMFCListCtrl::OnDropFiles(hDropInfo);
}

struct HANDLETOMAPPINGS
{
	UINT              uNumberOfMappings;  // Number of mappings in the array.
	LPSHNAMEMAPPING   lpSHNameMapping;    // Pointer to the array of mappings.
};

void CFileListCtrl::PasteFiles(CStringArray& aOldPath, BOOL bMove)
{
	if (m_strFolder.IsEmpty()) return;
	if (aOldPath.GetSize() == 0) return;

	BOOL bIsSamePath = FALSE;
	CString strOldFolder = Get_Folder(aOldPath[0], TRUE); //'\'를 붙여서 추출
	CString strNewFolder = m_strFolder;
	strNewFolder = PathBackSlash(strNewFolder, TRUE); //뒤에 '\'를 붙여 준다.
	if (strOldFolder.CompareNoCase(strNewFolder) == 0) bIsSamePath = TRUE;

	CStringArray aNewPath;
	aNewPath.SetSize(aOldPath.GetSize());
	for (int i = 0; i < aOldPath.GetSize(); i++)
	{
		aNewPath[i] = strNewFolder + Get_Name(aOldPath[i]);
	}
	TCHAR* pszzBuf_OldPath = NULL;
	TCHAR* pszzBuf_NewPath = NULL;
	StringArray2szzBuffer(aOldPath, pszzBuf_OldPath);
	if (pszzBuf_OldPath == NULL) return;
	StringArray2szzBuffer(aNewPath, pszzBuf_NewPath);
	if (pszzBuf_NewPath == NULL) return;

	SHFILEOPSTRUCT FileOp = { 0 };
	FileOp.hwnd = NULL;
	FileOp.wFunc = bMove ? FO_MOVE : FO_COPY;
	FileOp.pFrom = pszzBuf_OldPath;
	FileOp.pTo = pszzBuf_NewPath;
	FileOp.fFlags = FOF_MULTIDESTFILES | FOF_ALLOWUNDO | FOF_WANTMAPPINGHANDLE;
	if (bIsSamePath == TRUE) FileOp.fFlags = FileOp.fFlags | FOF_RENAMEONCOLLISION;
	FileOp.fAnyOperationsAborted = false;
	FileOp.hNameMappings = NULL;
	FileOp.lpszProgressTitle = NULL;
	WatchCurrentDirectory(FALSE);
	int nRet = SHFileOperation(&FileOp);
	if (FileOp.hNameMappings)
	{
		HANDLETOMAPPINGS* phtm = (HANDLETOMAPPINGS*)FileOp.hNameMappings;
		SHNAMEMAPPING* pnm = phtm->lpSHNameMapping;
		aNewPath.RemoveAll();
		int nCount = (int)phtm->uNumberOfMappings;
		for (int i = 0; i < nCount; i++)
		{
			aNewPath.Add(pnm->pszNewPath);
			pnm++;
		}
		SHFreeNameMappings(FileOp.hNameMappings);
	}
	delete[] pszzBuf_OldPath;
	delete[] pszzBuf_NewPath;
	for (int i=0; i<aNewPath.GetSize(); i++) AddItemByPath(aNewPath[i], TRUE, FALSE); 
	WatchCurrentDirectory(TRUE);
}


void CFileListCtrl::UpdateItemByPath(CString strOldPath, CString strNewPath)
{
	CString strOldFolder = Get_Folder(strOldPath, TRUE);
	CString strNewFolder = Get_Folder(strNewPath, TRUE);
	CString strOldName = Get_Name(strOldPath);
	CString strNewName = Get_Name(strNewPath);
	if (strOldFolder.CompareNoCase(m_strFolder) != 0) return;
	if (strOldFolder.CompareNoCase(strNewFolder) != 0) return;
	int nItem = -1;
	for (int i = 0; i < GetItemCount(); i++)
	{
		CString strTemp = GetItemText(i, 0);
		if (GetItemText(i, 0).CompareNoCase(strOldName) == 0)
		{
			nItem = i;
			break;
		}
	}
	if (nItem == -1) return;
	HANDLE hFile = CreateFile(strNewPath, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hFile == INVALID_HANDLE_VALUE || hFile == NULL)	return;
	DWORD itemData = PathIsDirectory(strNewPath) ? ITEM_TYPE_DIRECTORY : ITEM_TYPE_FILE;
	CString strSize;
	if (itemData == ITEM_TYPE_FILE)
	{
		LARGE_INTEGER filesize;
		if (GetFileSizeEx(hFile, &filesize)) strSize = GetFileSizeString(filesize.QuadPart);
	}
	FILETIME ftWrite;
	CString strWriteTime;
	if (GetFileTime(hFile, NULL, NULL, &ftWrite))
	{
		CTime tTemp = CTime(ftWrite);
		strWriteTime = tTemp.Format(_T("%Y-%m-%d %H:%M:%S"));
	}
	int nImage = GetFileImageIndexFromMap(strNewPath, (itemData == ITEM_TYPE_DIRECTORY) );
	SetItem(nItem, 0, LVIF_IMAGE | LVIF_TEXT, strNewName, nImage, 0, 0, 0);
	SetItemText(nItem, COL_DATE, strWriteTime);
	SetItemText(nItem, COL_SIZE, strSize);
	CloseHandle(hFile);
}


void CFileListCtrl::AddItemByPath(CString strPath, BOOL bCheckExist, BOOL bAllowBreak, CString strSelectByName)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	hFind = FindFirstFileExW(strPath, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	if (hFind == INVALID_HANDLE_VALUE) return;
	CString strSize, strDate, strType;
	DWORD itemData = 0;
	int nItem = -1;
	size_t nLen = 0;
	ULARGE_INTEGER filesize;
	CTime tTemp;
	BOOL b = TRUE, bIsDir = FALSE;
	TCHAR fullpath[MY_MAX_PATH];
	CString strDir = Get_Folder(strPath);
	BOOL bSelect = !(strSelectByName.IsEmpty());
	while (b)
	{
		if (bAllowBreak == TRUE && IsLoading(this) == FALSE)
		{
			break;
		}
		itemData = ITEM_TYPE_FILE;
		nLen = _tcsclen(fd.cFileName);
		if (nLen == 1 && fd.cFileName[0] == _T('.')) itemData = ITEM_TYPE_DOTS; //Dots
		else if (nLen == 2 && fd.cFileName[0] == _T('.') && fd.cFileName[1] == _T('.')) itemData = ITEM_TYPE_DOTS; //Dots
		if (itemData != ITEM_TYPE_DOTS)
		{
			tTemp = CTime(fd.ftLastWriteTime);
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
			PathCombineW(fullpath, strDir, fd.cFileName);
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
						if (GetItemText(i, 0).CompareNoCase(fd.cFileName) == 0)
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
				//if (m_bUseFileType == TRUE) SetItemText(nItem, COL_TYPE, GetPathType(fullpath));
				//else SetItemText(nItem, COL_TYPE, Get_Ext(fd.cFileName, bIsDir, FALSE));
				SetItemText(nItem, COL_TYPE, GetPathTypeFromMap(fullpath, bIsDir));
			}
		}
		b = FindNextFileW(hFind, &fd);
	}
	FindClose(hFind);
}

void CFileListCtrl::OnLvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	NM_LISTVIEW* pNMListView = pNMLV;
	* pResult = 0;

	HGLOBAL hgDrop = GetOleDataForClipboard();
	if (hgDrop != NULL)
	{
		COleDataSource datasrc;
		FORMATETC etc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		datasrc.CacheGlobalData(CF_HDROP, hgDrop, &etc);
		DROPEFFECT dwEffect = datasrc.DoDragDrop(DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK);
		//if ((dwEffect & DROPEFFECT_LINK) == DROPEFFECT_LINK || (dwEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY) ;
		//else if ((dwEffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE)
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
//				m_setPath.erase(GetItemText(i, COL_NAME));
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
		if (iColumn == COL_NAME) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_STR);
		else if (iColumn == COL_ALIAS) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_STR);
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

void CFileListCtrl::ShowContextMenu(CPoint pt)
{
	int nIndex = GetNextItem(-1, LVNI_SELECTED);

	CStringArray aSelectedPath;

	while (nIndex != -1)
	{
		aSelectedPath.Add(GetItemFullPath(nIndex));
		nIndex = GetNextItem(nIndex, LVNI_SELECTED);
	}
	/*if (aSelectedPath.GetSize() == 0)
	{
		aSelectedPath.Add(m_strFolder);
	}*/
	m_bMenuOn = TRUE;
	CFileListContextMenu context_menu;
	context_menu.SetParent(this);
	context_menu.SetPathArray(m_strFolder, aSelectedPath);
	UINT idCommand = context_menu.ShowContextMenu(this, pt);
	if (idCommand) GetParent()->PostMessage(WM_COMMAND, idCommand, 0);
	m_bMenuOn = FALSE;
}


void CFileListCtrl::SetBarMsg(CString strMsg)
{
	m_strBarMsg = strMsg;
	if (CMD_UpdateBar!=0) GetParent()->PostMessage(WM_COMMAND, CMD_UpdateBar, (DWORD_PTR)this);
}

BOOL CFileListCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	BOOL b = CMFCListCtrl::Create(dwStyle, rect, pParentWnd, nID);
	m_hThreadLoad = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (b)
	{
		BOOL bd = m_DropTarget.Register(this);
	}
	return b;
}

HGLOBAL CFileListCtrl::GetOleDataForClipboard()
{
	CStringList aFiles;
	CString strPath;
	size_t uBuffSize = 0;
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return NULL;
	while (nItem != -1)
	{
		strPath = GetItemFullPath(nItem);
		aFiles.AddTail(strPath);
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
		TCHAR* pszBuff;
		POSITION pos = aFiles.GetHeadPosition();
		pszBuff = (TCHAR*)(LPBYTE(pDrop) + sizeof(DROPFILES));
		while (NULL != pos)
		{
			lstrcpy(pszBuff, (LPCTSTR)aFiles.GetNext(pos));
			pszBuff = 1 + _tcschr(pszBuff, _T('\0'));
		}
		GlobalUnlock(hgDrop);
	}
	return hgDrop;
}


void CFileListCtrl::ClipBoardExport(BOOL bMove)
{
	HGLOBAL hgDrop = GetOleDataForClipboard();
	if (hgDrop == NULL) return;
	if (OpenClipboard())
	{
		EmptyClipboard();
		HGLOBAL hEffect = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(DWORD));
		if (hEffect != NULL)
		{
			DROPEFFECT effect = bMove ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
			DWORD* pdw1 = (DWORD*)GlobalLock(hEffect);
			(*pdw1) = effect;
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
			if (((*pEffect) & DROPEFFECT_MOVE) != 0) bMove = TRUE;
			GlobalUnlock(hMemEffect);
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


void CFileListCtrl::DeleteSelected(BOOL bRecycle)
{
	CStringArray aPath;
	CString strPath;
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	while (nItem != -1)
	{
		strPath = GetItemFullPath(nItem);
		aPath.Add(strPath);
		nItem = GetNextItem(nItem, LVNI_SELECTED);
	}
	TCHAR* pszBuf_Delete;
	StringArray2szzBuffer(aPath, pszBuf_Delete);

	SHFILEOPSTRUCT FileOp = { 0 };
	FileOp.hwnd = NULL;
	FileOp.wFunc = FO_DELETE;
	FileOp.pFrom = pszBuf_Delete;
	FileOp.pTo = NULL;
	FileOp.fFlags = bRecycle ? FOF_ALLOWUNDO : 0;
	FileOp.fAnyOperationsAborted = false;
	FileOp.hNameMappings = NULL;
	FileOp.lpszProgressTitle = NULL;
	WatchCurrentDirectory(FALSE);
	int nRet = SHFileOperation(&FileOp);
	delete[] pszBuf_Delete;
	nItem = GetNextItem(-1, LVNI_SELECTED);
	BOOL bDeleted = FALSE;
	while (nItem != -1)
	{
		bDeleted = DeleteInvalidItem(nItem);
		if (bDeleted == TRUE) nItem -= 1;
		nItem = GetNextItem(nItem, LVNI_SELECTED);
	}
	WatchCurrentDirectory(TRUE);
}

BOOL CFileListCtrl::RenameSelectedItem()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return FALSE;
	CDlgInput dlg;
	dlg.m_strTitle = _T("이름 바꾸기"); // 리소스 처리
	dlg.m_strInput = GetItemText(nItem, 0);
	dlg.m_nMode = INPUT_MODE_FILENAME;
	if (dlg.DoModal() == IDOK)
	{
		CString strPath = GetItemFullPath(nItem);
		TCHAR szOldPath[MY_MAX_PATH];
		TCHAR szNewPath[MY_MAX_PATH];
		memset(szOldPath, 0, MY_MAX_PATH * sizeof(TCHAR));
		memset(szNewPath, 0, MY_MAX_PATH * sizeof(TCHAR));
		lstrcpy(szOldPath, (LPCTSTR)strPath);
		PathCombineW(szNewPath, m_strFolder, dlg.m_strInput);

		BOOL bIsSamePath = FALSE;
		if (strPath.CompareNoCase(szNewPath) == 0) bIsSamePath = TRUE;

		SHFILEOPSTRUCT FileOp = { 0 };
		FileOp.hwnd = NULL;
		FileOp.wFunc = FO_RENAME;
		FileOp.pFrom = szOldPath;
		FileOp.pTo = szNewPath;
		FileOp.fFlags = FOF_RENAMEONCOLLISION | FOF_WANTMAPPINGHANDLE | FOF_ALLOWUNDO;
		FileOp.fAnyOperationsAborted = false;
		FileOp.hNameMappings = NULL;
		FileOp.lpszProgressTitle = NULL;
		WatchCurrentDirectory(FALSE);
		int nRet = SHFileOperation(&FileOp);
		if (FileOp.hNameMappings)
		{
			HANDLETOMAPPINGS* phtm = (HANDLETOMAPPINGS*)FileOp.hNameMappings;
			if (phtm->uNumberOfMappings > 0)
			{
				SHNAMEMAPPING* pnm = phtm->lpSHNameMapping;
				lstrcpy(szNewPath, pnm->pszNewPath);
			}
			SHFreeNameMappings(FileOp.hNameMappings);
		}
		SetItemText(nItem, 0, Get_Name(szNewPath));
		WatchCurrentDirectory(TRUE);
	}
	return TRUE;
}


void CFileListCtrl::ClearThread()
{
	if (m_strFolder.IsEmpty() == FALSE) //&& m_nType == LIST_TYPE_FOLDER)
	{
		m_DirWatcher.UnwatchAllDirectories();
		//if (m_DirWatcher.IsWatchingDirectory(m_strFolder)) m_DirWatcher.UnwatchDirectory(m_strFolder);
	}
	if (IsLoading(this) == TRUE)
	{
		SetLoadingStatus(this, FALSE);
		DWORD ret = WaitForSingleObject(m_hThreadLoad, 10000);
		if (ret != WAIT_OBJECT_0)
		{
			AfxMessageBox(L"Error:Loading thread is not cleared properly");
		}
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
	if (m_bMenuOn == FALSE)	ShowContextMenu(point);
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

COLORREF CFileListCtrl::OnGetCellTextColor(int nRow, int nColumn)
{
	//if (nColumn == COL_SIZE) return RGB(255, 0, 0);
	return GetTextColor();
}

COLORREF CFileListCtrl::OnGetCellBkColor(int nRow, int nColumn)
{
	//if (nColumn == COL_SIZE) return RGB(50, 22, 22);
	return GetBkColor();
}