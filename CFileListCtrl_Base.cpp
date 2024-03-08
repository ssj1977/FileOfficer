#include "pch.h"
#include "CFileListCtrl_Base.h"
#include "EtcFunctions.h"
#include "resource.h"
#include <vector>
#include <CommonControls.h> //For IID_IImageList

/////////////////////////////////////////////////
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
/////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CFileListCtrl_Base, CMFCListCtrl)

HIMAGELIST CFileListCtrl_Base::m_hSysImgList_SMALL = NULL;
HIMAGELIST CFileListCtrl_Base::m_hSysImgList_LARGE = NULL;
HIMAGELIST CFileListCtrl_Base::m_hSysImgList_EXTRALARGE = NULL;
HIMAGELIST CFileListCtrl_Base::m_hSysImgList_JUMBO = NULL;

CExtMap CFileListCtrl_Base::mapExt;  //확장자에 해당하는 이미지맵의 번호를 기억
CTypeMap CFileListCtrl_Base::mapType;  //확장자에 해당하는 파일타입 기억

CFileListCtrl_Base::CFileListCtrl_Base()
{
	m_hImageList = NULL;
	m_nIconType = SHIL_SMALL;
	m_bUseFileType = TRUE;
	m_bUseFileIcon = TRUE;
	m_bCheckOpen = FALSE;
}

CFileListCtrl_Base::~CFileListCtrl_Base()
{

}

//BEGIN_MESSAGE_MAP(CFileListCtrl_Base, CMFCListCtrl)
//END_MESSAGE_MAP()

void CFileListCtrl_Base::SetIconType(int nIconType)
{
	m_nIconType = nIconType;
	m_hImageList = GetImageListByType(nIconType);
	ListView_SetImageList(this->GetSafeHwnd(), m_hImageList, LVSIL_NORMAL);
	ListView_SetImageList(this->GetSafeHwnd(), m_hImageList, LVSIL_SMALL);
}

HIMAGELIST CFileListCtrl_Base::GetImageListByType(int nIconType)
{
	HIMAGELIST* himl_ptr = NULL;
	switch (nIconType)
	{
	case SHIL_SMALL:  himl_ptr = &m_hSysImgList_SMALL; break;
	case SHIL_LARGE:  himl_ptr = &m_hSysImgList_LARGE; break;
	case SHIL_EXTRALARGE:  himl_ptr = &m_hSysImgList_EXTRALARGE; break;
	case SHIL_JUMBO:  himl_ptr = &m_hSysImgList_JUMBO; break;
	default: return NULL;
	}
	if ((*himl_ptr) == NULL)
	{
		if (FAILED(SHGetImageList(nIconType, IID_IImageList, (void**)himl_ptr))) *himl_ptr = NULL;
	}
	return *himl_ptr;
}

//해당 파일의 아이콘 정보를 가져온다
int CFileListCtrl_Base::GetFileImageIndex(CString strPath, DWORD dwAttribute)
{
	SHFILEINFO sfi;
	memset(&sfi, 0x00, sizeof(sfi));
	//오버레이 처리는 네트워크 드라이브 속도 저하의 원인이 되어 하지 않기로
	//대신 Read Only 처리는 별도로 수행해서 보여주는 식으로 구현
	DWORD flag = SHGFI_ICON | SHGFI_LARGEICON; //| SHGFI_OVERLAYINDEX;
	//속성값이 
	if (dwAttribute != INVALID_FILE_ATTRIBUTES) flag = flag | SHGFI_USEFILEATTRIBUTES;

	if (strPath.GetLength() < MAX_PATH)
	{
		SHGetFileInfo(strPath, dwAttribute, &sfi, sizeof(sfi), flag);
	}
	else
	{
		LPITEMIDLIST pidl = GetPIDLfromPath(strPath);
		SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi), flag | SHGFI_PIDL);
		CoTaskMemFree(pidl);
	}
	//int nIconIndex = LOWORD(sfi.iIcon);
	//return nIconIndex;
	return sfi.iIcon;
}

int CFileListCtrl_Base::GetFileImageIndexFromMap(CString strPath, DWORD dwAttribute)
{
	BOOL bIsDir = (dwAttribute & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
	if (bIsDir) return SI_FOLDER_CLOSED;
	if (m_bUseFileIcon == FALSE) return SI_UNKNOWN;

	CString strExt = Get_Ext(strPath, bIsDir, TRUE);
	if (strExt.CompareNoCase(_T(".exe")) == 0
		|| strExt.CompareNoCase(_T(".ico")) == 0
		|| strExt.CompareNoCase(_T(".lnk")) == 0)
	{
		//확장자가 같아도 아이콘이 다를 수 있는 파일들은 바로 조회
		return GetFileImageIndex(strPath, dwAttribute);
	}
	CString strKey;
	strKey.Format(_T("%s%d"), strExt, (int)dwAttribute); //속성과 경로를 합쳐서 키 생성
	//나머지 파일에 대해서는 맵에서 우선 찾아서 속도 향상
	CExtMap::iterator it = mapExt.find(strKey);
	int nImage = 0;
	if (it == mapExt.end())
	{
		//dwAttribute를 사용하도록 하면 파일에 직접 액세스 하지 않고 가능
		nImage = GetFileImageIndex(strPath, dwAttribute);
		mapExt.insert(CExtMap::value_type(strKey, nImage));
	}
	else
	{
		nImage = (*it).second;
	}
	return nImage;
}

int CFileListCtrl_Base::GetDriveImageIndex(int nDriveType)
{
	switch (nDriveType)
	{
	case DRIVE_UNKNOWN: return SI_UNKNOWN;
	case DRIVE_NO_ROOT_DIR: return SI_UNKNOWN;
	case DRIVE_REMOVABLE: return SI_REMOVABLE;
	case DRIVE_FIXED:  return SI_HDD;
	case DRIVE_REMOTE: return SI_NETWORKDRIVE;
	case DRIVE_CDROM: return SI_CDROM;
	case DRIVE_RAMDISK: return SI_RAMDISK;
	}
	return SI_HDD;
}
/////////////////////////////////////////////////

static CString strFolderTypeText = _T("");

CString CFileListCtrl_Base::GetPathTypeFromMap(CString strPath, BOOL bIsDirectory, BOOL bUseFileType)
{
	if (bIsDirectory)
	{
		if (strFolderTypeText.IsEmpty())
		{
			strFolderTypeText = GetPathType(strPath);
		}
		return strFolderTypeText;
	}
	CString strType;
	CString strExt = Get_Ext(strPath, FALSE, FALSE);
	if (bUseFileType == FALSE)
	{
		return strExt;
	}
	CTypeMap::iterator it = mapType.find(strExt);
	if (it == mapType.end())
	{
		strType = GetPathType(strPath);
		mapType.insert(CTypeMap::value_type(strExt, strType));
		return strType;
	}
	return (*it).second;
}

LPITEMIDLIST CFileListCtrl_Base::GetPIDLfromPath(CString strPath)
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


CString CFileListCtrl_Base::GetPathName(CString strPath)
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
		pidl = GetPIDLfromPath(strPath);
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

CString CFileListCtrl_Base::GetPathType(CString strPath)
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
		else pidl = GetPIDLfromPath(strPath);
		if (SHGetFileInfo((LPCTSTR)pidl, -1, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_TYPENAME))
			strReturn = sfi.szTypeName;
		CoTaskMemFree(pidl);
	}
	return strReturn;
}

HRESULT CFileListCtrl_Base::CreateShellItemArrayFromPaths(CStringArray& aPath, IShellItemArray*& shi_array)
{
	HRESULT hr = S_OK;
	std::vector<LPITEMIDLIST> pidl_items;
	for (int i = 0; i < aPath.GetSize(); i++)
	{
		LPITEMIDLIST pidl = GetPIDLfromPath(aPath.GetAt(i));
		if (pidl) pidl_items.push_back(pidl);
	}
	hr = SHCreateShellItemArrayFromIDLists((UINT)pidl_items.size(),
		(LPCITEMIDLIST*)pidl_items.data(), &shi_array);
	for (auto& pid : pidl_items) CoTaskMemFree(pid);
	pidl_items.clear();
	return hr;
}

CString CFileListCtrl_Base::GetPathMemo(CString strPath, DWORD dwAttributes, BOOL bCheckOpen)
{
	CString strMemo;

	if (bCheckOpen == TRUE && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		HANDLE hFile = CreateFile(strPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
		{
			strMemo += IDSTR(IDS_MEMO_LOCKED);
		}
		else
		{
			CloseHandle(hFile);
		}
	}

	if (dwAttributes & FILE_ATTRIBUTE_HIDDEN) strMemo += IDSTR(IDS_MEMO_HIDDEN);
	if (dwAttributes & FILE_ATTRIBUTE_READONLY) strMemo += IDSTR(IDS_MEMO_READONLY);
	if (dwAttributes & FILE_ATTRIBUTE_COMPRESSED) strMemo += IDSTR(IDS_MEMO_COMPRESSED);
	if (dwAttributes & FILE_ATTRIBUTE_ENCRYPTED) strMemo += IDSTR(IDS_MEMO_ENCRYPTED);
	if (bCheckOpen == TRUE && strMemo.IsEmpty()) strMemo = L"-";

	return strMemo;
}


void CFileListCtrl_Base::SetColTexts(int* pStringId, int* pColFmt, int* pColSortType, int size)
{
	CString strText;
	LVCOLUMN col;
	col.mask = LVCF_TEXT | LVCF_FMT;
	m_aColSortType.RemoveAll();
	for (int i = 0; i < size; i++)
	{
		BOOL b = strText.LoadString(*(pStringId + i));
		if (b)
		{
			col.pszText = strText.GetBuffer();
			col.fmt = *(pColFmt + i);
			SetColumn(i, &col);
			strText.ReleaseBuffer();
		}
		m_aColSortType.Add(*(pColSortType + i));
	}
}

int CFileListCtrl_Base::CompareItemByType(LPARAM item1, LPARAM item2, int nCol, int nType)
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
		BOOL bIsDir1 = IsDir((int)item1);
		BOOL bIsDir2 = IsDir((int)item2);
		if (bIsDir1 != bIsDir2)
		{
			nRet = int(bIsDir2 - bIsDir1);
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
	else if (nType == COL_COMP_DRIVE)
	{
		// 드라이브 라벨 컬럼 클릭시 드라이브끼리의 정렬은 경로 순서로 해야 함 
		// 라벨에서 경로 정보를 추출하여 처리
		CString strPath1 = GetDrivePathFromName(str1);
		CString strPath2 = GetDrivePathFromName(str2);
		BOOL bIsDrive1 = !(strPath1.IsEmpty());
		BOOL bIsDrive2 = !(strPath2.IsEmpty());
		if (bIsDrive1 != bIsDrive2) //드라이브가 항상 드라이브 아닌 것 보다 우선
		{
			nRet = int(bIsDrive2 - bIsDrive1);
		}
		else
		{
			if (bIsDrive1) // 드라이브 끼리 비교할때는 경로로
			{
				nRet = StrCmpLogicalW(strPath1.GetBuffer(), strPath2.GetBuffer());
				strPath1.ReleaseBuffer();
				strPath2.ReleaseBuffer();
			}
			else // 드라이브 아닌 것 끼리 비교할때는 라벨로
			{
				nRet = StrCmpLogicalW(str1.GetBuffer(), str2.GetBuffer());
				str1.ReleaseBuffer();
				str2.ReleaseBuffer();
			}
		}
	}
	return nRet;
}

CString CFileListCtrl_Base::GetDrivePathFromName(CString strPath)
{
	//드라이브 라벨에는 항상 괄호 안에 실제 드라이브 경로가 포함되어 있음
	int nFind = strPath.Find(_T(':'));
	if (nFind < 1) return _T("");
	return strPath.Mid(nFind - 1, 2); //콜론 기준 앞의 한글자와 콜론을 반환
}


int CFileListCtrl_Base::OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn)
{
	if (m_aColSortType.GetCount() <= iColumn) return 0;
	/*	int nRet = 0;
	if (m_nType == LIST_TYPE_FOLDER)
	{
		if (iColumn == COL_NAME) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_PATH);
		else if (iColumn == COL_DATE) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_STR);
		else if (iColumn == COL_SIZE) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_SIZE);
		else if (iColumn == COL_TYPE) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_STR);
	}
	else if (m_nType == LIST_TYPE_DRIVE)
	{
		if (iColumn == COL_DRIVENAME) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_DRIVE);
		else if (iColumn == COL_DRIVEPATH) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_STR);
		else if (iColumn == COL_FREESPACE) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_SIZE);
		else if (iColumn == COL_TOTALSPACE) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_SIZE);
	}
	else if (m_nType == LIST_TYPE_UNCSERVER)
	{
		if (iColumn == COL_NAME) nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_STR);
	}
		//nRet = CompareItemByType(lParam1, lParam2, iColumn, m_aColSortType[iColumn]);
	}*/
	return CompareItemByType(lParam1, lParam2, iColumn, m_aColSortType[iColumn]);
}