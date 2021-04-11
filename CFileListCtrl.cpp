﻿#include "pch.h"
#include "CFileListCtrl.h"
#include <shlwapi.h>
#include <shellapi.h>
#include <lm.h>
#include <atlpath.h>
#include "CFileListContextMenu.h"
#include "EtcFunctions.h"

#pragma comment(lib, "Netapi32.lib")

/////////////////////////////////////////////////
//파일 아이콘 처리
#include <map>
typedef std::map<CString, int> CExtMap; //확장자에 해당하는 이미지맵의 번호를 기억
static CExtMap mapExt;

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
		return GetFileImageIndex(_T(""));
		//return SI_FOLDER_OPEN;
	}
	CPath path = CPath(strPath);
	CString strExt = path.GetExtension();
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



// CFileListCtrl

IMPLEMENT_DYNAMIC(CFileListCtrl, CMFCListCtrl)

#define COL_NAME 0
#define COL_DATE 1
#define COL_ALIAS 1
#define COL_SIZE 2
#define COL_FREESPACE 2
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

CFileListCtrl::CFileListCtrl()
{
	m_strFolder = L"";
	m_nType = LIST_TYPE_DRIVE;
	CMD_UpdateSortInfo = 0;
	CMD_UpdateTabCtrl = 0;
	CMD_UpdateBar = 0;
	m_bAsc = TRUE;
	m_nSortCol = 0 ;
	m_bIsThreadWorking = FALSE;
	m_nIconType = SHIL_SMALL;
}

CFileListCtrl::~CFileListCtrl()
{
}


BEGIN_MESSAGE_MAP(CFileListCtrl, CMFCListCtrl)
	ON_WM_SIZE()
	//ON_NOTIFY(HDN_ITEMCLICKA, 0, &CFileListCtrl::OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, &CFileListCtrl::OnHdnItemclick)
	ON_WM_DROPFILES()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, &CFileListCtrl::OnLvnBegindrag)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CFileListCtrl::OnNMDblclk)
	ON_NOTIFY_REFLECT(NM_RCLICK, &CFileListCtrl::OnNMRClick)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

CString CFileListCtrl::GetCurrentFolder()
{
	return m_strFolder;
}

void CFileListCtrl::InitColumns(int nType)
{
	int nCount = GetHeaderCtrl().GetItemCount();
	for (int i = nCount - 1; i >= 0; i--) DeleteColumn(i);
	int nIconWidth = 0;
	switch (m_nIconType)
	{
		case SHIL_SMALL: nIconWidth = 16; break;
		case SHIL_LARGE: nIconWidth = 32; break;
		case SHIL_EXTRALARGE: nIconWidth = 48; break;
		case SHIL_JUMBO: nIconWidth = 256; break;
	}
	if (nType == LIST_TYPE_DRIVE)
	{
		InsertColumn(COL_NAME, _T("Drive"), LVCFMT_LEFT, nIconWidth + 150);
		InsertColumn(COL_ALIAS, _T("Description"), LVCFMT_LEFT, 300);
		InsertColumn(COL_FREESPACE, _T("Free"), LVCFMT_LEFT, 150);
		InsertColumn(COL_TOTALSPACE, _T("Total"), LVCFMT_LEFT, 150);
	}
	else if (nType == LIST_TYPE_FOLDER)
	{
		InsertColumn(COL_NAME, _T("Name"), LVCFMT_LEFT, nIconWidth + 300);
		InsertColumn(COL_DATE, _T("Date"), LVCFMT_RIGHT, 200);
		InsertColumn(COL_SIZE, _T("Size"), LVCFMT_RIGHT, 150);
	}
	else if (nType == LIST_TYPE_UNCSERVER)
	{
		InsertColumn(COL_NAME, _T("Folder"), LVCFMT_LEFT, nIconWidth + 300);
	}
	m_nType = nType;
}

CString CFileListCtrl::GetItemFullPath(int nItem)
{
	CPath path = CPath(m_strFolder);
	path.AddBackslash();
	path.Append(GetItemText(nItem, COL_NAME));
	return CString(path);
}

void CFileListCtrl::OpenSelectedItem()
{
	int nIndex = GetNextItem(-1, LVNI_SELECTED);
	if (nIndex == -1) return;

	INT_PTR nType = GetItemData(nIndex);
	if (nType == ITEM_TYPE_FILE)
	{
		HINSTANCE hr = ShellExecute(NULL, NULL, GetItemFullPath(nIndex), NULL, NULL, SW_SHOW);
		if ((INT_PTR)hr <= 32) AfxMessageBox(L"Error");
	}
	else
	{
		DisplayFolder_Start(GetItemFullPath(nIndex));
	}
}

void CFileListCtrl::OpenParentFolder()
{
	if (m_strFolder.IsEmpty()) return;
	CPath path = CPath(m_strFolder);
	if (path.IsUNCServer())
	{
		return;
	}
	else if (path.IsRoot())
	{
		DisplayFolder_Start(_T(""));
	}
	else
	{
		CString strParent = m_strFolder;
		int nPos = strParent.ReverseFind(_T('\\'));
		if (nPos = strParent.GetLength() - 1)
		{
			strParent = strParent.Left(nPos);
			nPos = strParent.ReverseFind(_T('\\'));
		}
		if (nPos <= 0) return;
		strParent = strParent.Left(nPos);
		DisplayFolder_Start(strParent);
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

void CFileListCtrl::DisplayFolder_Start(CString strFolder)
{
	if (m_bIsThreadWorking == TRUE) return;
	m_strFolder = strFolder;
	if (GetParent()!=NULL && ::IsWindow(GetParent()->GetSafeHwnd()))
		GetParent()->PostMessage(WM_COMMAND, CMD_UpdateTabCtrl, (DWORD_PTR)this);
	AfxBeginThread(DisplayFolder_Thread, this);
}
UINT CFileListCtrl::DisplayFolder_Thread(void* lParam)
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
	//APP()->UpdateThreadLocale();
	CFileListCtrl* pList = (CFileListCtrl*)lParam;
	pList->m_bIsThreadWorking = TRUE;
	pList->SetBarMsg(_T("Thread Working..."));
	pList->DisplayFolder(pList->m_strFolder);
	pList->m_bIsThreadWorking = FALSE;
	return 0;
}

void CFileListCtrl::DisplayFolder(CString strFolder)
{
	clock_t startTime, endTime;
	startTime = clock();

	CPath path = CPath(strFolder);
	DeleteAllItems();
	if (strFolder.IsEmpty())
	{
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
				strDrive += _T(":\\");
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
			}
			flag = flag * 2;
		}
	}
	else if (path.IsUNCServer())
	{
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
		path.AddBackslash();
		CString strFind = path + _T("*");
		AddItemByPath(strFind);
		Sort(m_nSortCol, m_bAsc);
	}
	m_strFolder = (CString)path;

	endTime = clock();
	CString strTemp;
	strTemp.Format(_T("Loading Time : %d"), endTime - startTime);
	SetBarMsg(strTemp);
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
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_LBUTTONDBLCLK)
	{
		if (GetNextItem(-1, LVNI_SELECTED) == -1)
		{
			OpenParentFolder();
			return TRUE;
		}
	}
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
	}
	if (pMsg->message == WM_KEYUP && (GetKeyState(VK_CONTROL) & 0xFF00) != 0)
	{
		if (pMsg->wParam == _T('C')) { ClipBoardExport(FALSE); return TRUE; }
		if (pMsg->wParam == _T('X')) { ClipBoardExport(TRUE); return TRUE; }
		if (pMsg->wParam == _T('V')) { ClipBoardImport(); return TRUE; }
	}
	if (pMsg->message == WM_KEYUP && pMsg->wParam == VK_DELETE)
	{
		if ((GetKeyState(VK_SHIFT) & 0xFF00) != 0) DeleteSelected(FALSE);
		else DeleteSelected(TRUE);
	}
	return CMFCListCtrl::PreTranslateMessage(pMsg);
}


void CFileListCtrl::MyDropFiles(HDROP hDropInfo, BOOL bMove)
{
	if (m_nType != LIST_TYPE_FOLDER) return;
	TCHAR szFilePath[MAX_PATH];
	size_t bufsize = sizeof(TCHAR) * MAX_PATH;
	memset(szFilePath, 0, bufsize);
	WORD cFiles = DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	for (int i = 0; i < cFiles; i++)
	{
		DragQueryFile(hDropInfo, i, szFilePath, MAX_PATH);
		PasteFile(szFilePath, bMove);
	}
	DragFinish(hDropInfo);
	//CMFCListCtrl::OnDropFiles(hDropInfo);
}

void CFileListCtrl::PasteFile(CString strPath, BOOL bMove)
{
	TCHAR szNewPath[MAX_PATH];
	PathCombineW(szNewPath, m_strFolder, Get_Name(strPath));
	if (strPath.CompareNoCase(szNewPath) == 0) return;
	BOOL bRet = FALSE;
	BOOL bCancel = FALSE;
	if (bMove == TRUE) bRet = MoveFileExW(strPath, szNewPath, MOVEFILE_COPY_ALLOWED);
	else bRet= CopyFileExW(strPath, szNewPath, NULL,NULL, &bCancel, COPY_FILE_FAIL_IF_EXISTS | COPY_FILE_RESTARTABLE);
	if (bRet == FALSE)
	{
		LPVOID lpMsgBuf;
		DWORD err = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf, 0, NULL);
		CString strErr = (LPCTSTR)lpMsgBuf;
		LocalFree(lpMsgBuf);
		if (strErr.IsEmpty() == FALSE) AfxMessageBox(strErr);
		return;
	}
	AddItemByPath(szNewPath);
}

void CFileListCtrl::AddItemByPath(CString strPath)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	hFind = FindFirstFileExW(strPath, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	if (hFind == INVALID_HANDLE_VALUE) return;
	CString strSize, strDate, strType;
	DWORD itemData = 0;
	int nItem = 0;
	size_t nLen = 0;
	ULARGE_INTEGER filesize;
	CTime tTemp;
	BOOL b = TRUE, bIsDir = FALSE;
	TCHAR fullpath[MAX_PATH];
	CString strDir = Get_Folder(strPath);
	while (b)
	{
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
			nItem = InsertItem(GetItemCount(), fd.cFileName, GetFileImageIndexFromMap(fullpath, bIsDir));
			SetItemData(nItem, itemData);
			SetItemText(nItem, COL_DATE, strDate);
			if (!strSize.IsEmpty()) SetItemText(nItem, COL_SIZE, strSize);
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
		if ((dwEffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE)
		{
			int nItem = GetNextItem(-1, LVNI_SELECTED);
			while (nItem != -1)
			{
				//DeleteInvaildItem(nItem);
				DeleteItem(nItem);
				nItem = GetNextItem(-1, LVNI_SELECTED);
			}
		}
		else if (dwEffect == DROPEFFECT_NONE)
		{
			GlobalFree(hgDrop);
		}
	}
}

BOOL CFileListCtrl::DeleteInvaildItem(int nItem)
{
	BOOL bDeleted = FALSE;
	CString strPath = GetItemFullPath(nItem);
	CFileFind find;
	if (find.FindFile(strPath) == FALSE)
	{
		bDeleted = DeleteItem(nItem);
	}
	find.Close();
	return bDeleted;
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

void CFileListCtrl::Sort(int iColumn, BOOL bAscending, BOOL bAdd)
{
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
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	CPoint pt(pNMItemActivate->ptAction);
	ClientToScreen(&pt);
	ShowContextMenu(pt);
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
	if (aSelectedPath.GetSize() == 0)
	{
		aSelectedPath.Add(m_strFolder);
	}
	CFileListContextMenu context_menu;
	context_menu.SetPathArray(aSelectedPath);
	UINT idCommand = context_menu.ShowContextMenu(this, pt);
	if (idCommand) GetParent()->PostMessage(WM_COMMAND, idCommand, 0);
}


void CFileListCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CMFCListCtrl::OnSetFocus(pOldWnd);
	GetParent()->PostMessage(WM_COMMAND, IDM_SET_FOCUS_ON, 0);
}


void CFileListCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CMFCListCtrl::OnKillFocus(pNewWnd);
	GetParent()->PostMessage(WM_COMMAND, IDM_SET_FOCUS_OFF, 0);
}

void CFileListCtrl::SetBarMsg(CString strMsg)
{
	m_strBarMsg = strMsg;
	if (CMD_UpdateBar!=0) GetParent()->PostMessage(WM_COMMAND, CMD_UpdateBar, (DWORD_PTR)this);
}

BOOL CFileListCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	BOOL b = CMFCListCtrl::Create(dwStyle, rect, pParentWnd, nID);
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
		SetClipboardData(CF_HDROP, hgDrop);
		CloseClipboard();
	}
}

void CFileListCtrl::ClipBoardImport()
{
	COleDataObject odj;
	if (OpenClipboard())
	{
		HDROP hDropInfo = (HDROP)GetClipboardData(CF_HDROP);
		MyDropFiles(hDropInfo, FALSE);
		CloseClipboard();
		GlobalFree((HGLOBAL)hDropInfo);
	}
/*	if (odj.AttachClipboard())
	{
		if (odj.IsDataAvailable(CF_HDROP))
		{
			STGMEDIUM StgMed;
			FORMATETC fmte = { CF_HDROP, (DVTARGETDEVICE FAR*)NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			if (odj.GetData(CF_HDROP, &StgMed, &fmte))
			{
				HDROP hDropInfo = (HDROP)StgMed.hGlobal;
				OnDropFiles(hDropInfo);
			}
			if (StgMed.pUnkForRelease) StgMed.pUnkForRelease->Release();
			else GlobalFree(StgMed.hGlobal);
		}
	}*/
}


void CFileListCtrl::DeleteSelected(BOOL bRecycle)
{
	CStringList aFiles;
	CString strPath;
	size_t uBufSize = 0;
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	while (nItem != -1)
	{
		strPath = GetItemFullPath(nItem);
		aFiles.AddTail(strPath);
		nItem = GetNextItem(nItem, LVNI_SELECTED);
		uBufSize += strPath.GetLength() + 1;
	}
	uBufSize += 1; //For an additional NULL character
	TCHAR* pszzBuff = new TCHAR[uBufSize];
	memset(pszzBuff, 0, uBufSize * sizeof(TCHAR));
	POSITION pos = aFiles.GetHeadPosition();
	TCHAR* pBufPos = pszzBuff;
	while (NULL != pos)
	{
		lstrcpy(pBufPos, (LPCTSTR)aFiles.GetNext(pos));
		pBufPos = 1 + _tcschr(pBufPos, _T('\0'));
	}
	SHFILEOPSTRUCT FileOp = { 0 };
	FileOp.hwnd = NULL;
	FileOp.wFunc = FO_DELETE;
	FileOp.pFrom = pszzBuff;
	FileOp.pTo = NULL;
	FileOp.fFlags = bRecycle ? FOF_ALLOWUNDO : 0;
	FileOp.fAnyOperationsAborted = false;
	FileOp.hNameMappings = NULL;
	FileOp.lpszProgressTitle = NULL;
	int nRet = SHFileOperation(&FileOp);
	delete[] pszzBuff;
	nItem = GetNextItem(-1, LVNI_SELECTED);
	BOOL bDeleted = FALSE;
	while (nItem != -1)
	{
		bDeleted = DeleteInvaildItem(nItem);
		if (bDeleted == TRUE) nItem -= 1;
		nItem = GetNextItem(nItem, LVNI_SELECTED);
	}
}