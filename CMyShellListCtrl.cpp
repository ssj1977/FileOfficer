// CMyShellListCtrl.cpp: 구현 파일
//

#include "pch.h"
#include "FileOfficer.h"
#include "CMyShellListCtrl.h"
#include <vector>
#include <map>
#include "CDlgInput.h"
#include "EtcFunctions.h"

#ifndef WATCH_BUFFER_SIZE
#define WATCH_BUFFER_SIZE 32 * 1024 //네트워크 드라이브에서 버퍼 크기가 64KB 이상이 되면 오류발생(패킷 크기 제한 때문)
#define IDM_WATCHEVENT 55011
#endif 

//쓰레드의 상태를 관리하기 위한 static 변수
typedef std::map<CMyShellListCtrl*, BOOL> ThreadStatusMap;

static void SetThreadStatus(ThreadStatusMap& mythread, CMyShellListCtrl* pList, BOOL bLoading)
{
	ThreadStatusMap::iterator it = mythread.find(pList);
	if (it == mythread.end())	mythread.insert(ThreadStatusMap::value_type(pList, bLoading));
	else						mythread.at(pList) = bLoading;
}

static BOOL IsThreadOn(ThreadStatusMap& mythread, CMyShellListCtrl* pList)
{
	ThreadStatusMap::iterator it = mythread.find(pList);
	if (it == mythread.end())	return FALSE;
	else						return (*it).second;
}

static void DeleteThreadStatus(ThreadStatusMap& mythread, CMyShellListCtrl* pList)
{
	ThreadStatusMap::iterator it = mythread.find(pList);
	if (it != mythread.end())
	{
		mythread.erase(pList);
	}
}

static ThreadStatusMap st_mapWatching;

LPITEMIDLIST GetPIDLfromFullPath(CString strPath)
{
	//경로 길이가 MAX_PATH 보다 짧다면 간단히 끝난다
	if (strPath.GetLength() < MAX_PATH) return ILCreateFromPath(strPath);
	//경로 길이가 MAX_PATH 보다 길다면 파일명과 폴더명을 잘라서 처리 필요
	//상대 PIDL이 필요한 경우는 이것으로 처리
	LPITEMIDLIST pidl_result = NULL;
	CString strParent = Get_Folder(strPath);
	CString strChild = Get_Name(strPath);
	if (strChild.GetLength() < MAX_PATH)
	{
		IShellFolder* pisf = NULL;
		if ((SHGetDesktopFolder(&pisf)) == S_OK)
		{
			LPITEMIDLIST pidl_parent = GetPIDLfromFullPath(strParent);
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
			pisf->Release();
		}
	}
	return pidl_result;
}

HRESULT CreateShellItemArrayFromPaths(CStringArray& aPath, IShellItemArray*& shi_array)
{
	HRESULT hr = S_OK;
	std::vector<LPITEMIDLIST> pidl_items;
	for (int i = 0; i < aPath.GetSize(); i++)
	{
		LPITEMIDLIST pidl = GetPIDLfromFullPath(aPath.GetAt(i));
		if (pidl) pidl_items.push_back(pidl);
	}
	hr = SHCreateShellItemArrayFromIDLists((UINT)pidl_items.size(),
		(LPCITEMIDLIST*)pidl_items.data(), &shi_array);
	for (auto& pid : pidl_items) CoTaskMemFree(pid);
	pidl_items.clear();
	return hr;
}

// IFileOperation 에서 변경된 파일명을 받아오기 위한 IFileOperationProgressSink 구현

IFACEMETHODIMP CMyShellListProgress::PostCopyItem(DWORD dwFlags, IShellItem* psiItem,
	IShellItem* psiDestinationFolder, PCWSTR pwszNewName, HRESULT hrCopy,
	IShellItem* psiNewlyCreated)
{
	if (m_pList)
	{
		m_pList->AddPath(pwszNewName, TRUE, TRUE);
		//m_pList->UpdateMsgBar();
	}
	return S_OK;
}
IFACEMETHODIMP CMyShellListProgress::PostMoveItem(DWORD dwFlags, IShellItem* psiItem,
	IShellItem* psiDestinationFolder, PCWSTR pwszNewName, HRESULT hrCopy,
	IShellItem* psiNewlyCreated)
{
	if (m_pList)
	{
		m_pList->AddPath(pwszNewName, TRUE, TRUE);
		//m_pList->UpdateMsgBar();
	}
	return S_OK;
}
IFACEMETHODIMP CMyShellListProgress::PostRenameItem(DWORD dwFlags, IShellItem* psiItem,
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

IFACEMETHODIMP CMyShellListProgress::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit[] =
	{
		QITABENT(CMyShellListCtrl, IFileOperationProgressSink),
		{0},
	};
	return QISearch(this, qit, riid, ppv);
}
IFACEMETHODIMP_(ULONG) CMyShellListProgress::AddRef()
{
	return InterlockedIncrement(&_cRef);
}
IFACEMETHODIMP_(ULONG) CMyShellListProgress::Release()
{
	ULONG cRef = InterlockedDecrement(&_cRef);
	if (0 == cRef)	delete this;
	return cRef;
}


// CMyShellListCtrl

IMPLEMENT_DYNAMIC(CMyShellListCtrl, CMFCShellListCtrl)

CMyShellListCtrl::CMyShellListCtrl()
{
	CMD_OpenNewTab = 0;
	CMD_UpdateSortInfo = 0;
	CMD_UpdateFromList = 0;
	m_nIconType = SHIL_SMALL;

	m_hDirectory = NULL;
	m_hWatchBreak = NULL;
	m_pThreadWatch = NULL;
	m_pWatchBuffer = malloc(WATCH_BUFFER_SIZE);

	m_posPathHistory = NULL;


}

CMyShellListCtrl::~CMyShellListCtrl()
{
	CMyShellListCtrl::DeleteWatchingStatus(this);
	free(m_pWatchBuffer);
}


BEGIN_MESSAGE_MAP(CMyShellListCtrl, CMFCShellListCtrl)
	ON_WM_DROPFILES()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, &CMyShellListCtrl::OnLvnBegindrag)
	ON_WM_CLIPBOARDUPDATE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CMyShellListCtrl::OnNMDblclk)
END_MESSAGE_MAP()


void CMyShellListCtrl::ClearThread()
{
	if (IsWatching(this) != FALSE && m_pThreadWatch != NULL)
	{
		WatchFolder_End();
	}
}


CString CMyShellListCtrl::GetItemFullPath(int nItem)
{
	BOOL bSuccess = FALSE;
	CString strPath;
	bSuccess = GetItemPath(strPath, nItem); //Full Path 인지 테스트 필요
	if (bSuccess) return strPath;
	return _T("");
/*	if (m_nType == LIST_TYPE_FOLDER || m_nType == LIST_TYPE_UNCSERVER)
	{
		CString strPath;
		strPath = PathBackSlash(m_strFolder) + GetItemText(nItem, COL_NAME);
		return strPath;
	}
	else if (m_nType == LIST_TYPE_DRIVE)
	{
		return GetItemText(nItem, COL_DRIVEPATH);
	}
	return _T("");*/
}

HRESULT CMyShellListCtrl::GetShellItemInfoFromPath(CString strPath, BOOL bRelativePath, LPAFX_SHELLITEMINFO pItemInfo)
{
	if (pItemInfo == NULL || m_psfCurFolder == NULL || m_pidlCurFQ == NULL) return E_FAIL;
	CString strChild;
	if (bRelativePath == FALSE) strChild = Get_Name(strPath);
	else						strChild = strPath;
	if (strChild.GetLength() >= MAX_PATH) return E_FAIL;
	LPSHELLFOLDER pisf_parent = m_psfCurFolder;
	LPITEMIDLIST pidl_parent = m_pidlCurFQ;
	LPITEMIDLIST pidl_child = NULL;
	LPITEMIDLIST pidl_result = NULL;
	if (pisf_parent->ParseDisplayName(NULL, 0, strChild.GetBuffer(0), NULL, &pidl_child, NULL) == S_OK)
	{
		pidl_result = afxShellManager->ConcatenateItem(pidl_parent, pidl_child); //아래 주석처리 부분과 같은 역할
		/*UINT cb1 = ILGetSize(pidl_parent) - sizeof(pidl_parent->mkid.cb);
		UINT cb2 = ILGetSize(pidl_child);
		pidl_result = (LPITEMIDLIST)CoTaskMemAlloc(cb1 + cb2);
		if (pidl_result != NULL)
		{
			CopyMemory(pidl_result, pidl_parent, cb1);
			CopyMemory(((LPSTR)pidl_result) + cb1, pidl_child, cb2);
		}*/
		strChild.ReleaseBuffer();
		if (pItemInfo)
		{
			pItemInfo->pidlFQ = pidl_result; //전체경로 PIDL
			pItemInfo->pidlRel = pidl_child; //상대경로 PIDL
			pItemInfo->pParentFolder = pisf_parent; //IShellFolder
			pisf_parent->AddRef();
		}
		return S_OK;
	}
	return E_FAIL;
}

int CMyShellListCtrl::GetIndexByName(CString strName)
{
	int nItem = -1;
	for (int i = 0; i < GetItemCount(); i++)
	{
		if (GetItemText(i, AFX_ShellList_ColumnName).CompareNoCase(strName) == 0)
		{
			nItem = i;
			break;
		}
	}
	return nItem;
}

int CMyShellListCtrl::AddPath(CString strPath, BOOL bRelativePath, BOOL bCheckExist)
{
	// 존재하는지 미리 체크
	CString strName = bRelativePath ? strPath : Get_Name(strPath);
	if (bCheckExist == TRUE)
	{
		BOOL bExist = FALSE;
		if (GetIndexByName(strName) != -1) return -1;
	}
	return SetItemByName(GetItemCount(), strName, TRUE);
}

void CMyShellListCtrl::UpdateItemByPath(CString strOldPath, CString strNewPath, BOOL bRelativePath, BOOL bForceUpdate)
{
	CString strOldName = bRelativePath ? strOldPath : Get_Name(strOldPath);
	CString strNewName = bRelativePath ? strNewPath : Get_Name(strNewPath);
	int nItemNew = GetIndexByName(strNewName);
	int nItemOld = GetIndexByName(strOldName);
	if (nItemOld == -1) return; // 변경해야 할 아이템이 없는 경우
	if (nItemNew != -1 && nItemNew != nItemOld) //이미 변경하려는 이름이 다른 아이템에 있는 경우
	{
		if (bForceUpdate == FALSE) return; //강제갱신이 아니면 중단
	}
	nItemNew = SetItemByName(nItemOld, strNewName, FALSE);
}

void CMyShellListCtrl::DeleteInvalidName(CString strName)
{
	int nItem = GetIndexByName(strName);
	if (nItem == -1) return;
	CString strPath = GetItemFullPath(nItem);
	if (PathFileExists(strPath) == FALSE)
	{
		//ReleaseItemData(nItem);
		DeleteItem(nItem);
	}
}

void CMyShellListCtrl::ReleaseItemData(int nItem)
{
	LPAFX_SHELLITEMINFO pItemInfo = (LPAFX_SHELLITEMINFO)GetItemData(nItem);
	//free up the pidls that we allocated
	afxShellManager->FreeItem(pItemInfo->pidlFQ);
	afxShellManager->FreeItem(pItemInfo->pidlRel);
	//this may be NULL if this is the root item
	if (pItemInfo->pParentFolder != NULL)
	{
		pItemInfo->pParentFolder->Release();
		pItemInfo->pParentFolder = NULL;
	}
	//GlobalFree((HGLOBAL)pItemInfo);  //pItemInfo 메모리 자체는 재활용해야 하므로 지우지 않는다.
}

int CMyShellListCtrl::SetItemByName(int nItem, CString strName, BOOL bInsert)
{
	//lParam 에 아이템 값으로 LPAFX_SHELLITEMINFO 가 포함된다.
	LPITEMIDLIST pidlCurrent = NULL;
	LPAFX_SHELLITEMINFO pItemInfo = NULL;
	LPSHELLFOLDER pParentFolder = m_psfCurFolder;
	if (m_psfCurFolder == NULL) return -1;
	if (bInsert)
	{
		pItemInfo = (LPAFX_SHELLITEMINFO)GlobalAlloc(GPTR, sizeof(AFX_SHELLITEMINFO));
	}
	else
	{
		pItemInfo = (LPAFX_SHELLITEMINFO)GetItemData(nItem);
		ReleaseItemData(nItem);
	}
	if (pItemInfo == NULL || GetShellItemInfoFromPath(strName, TRUE, pItemInfo) != S_OK) return -1;
	LVITEM lvItem;
	ZeroMemory(&lvItem, sizeof(lvItem));
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvItem.pszText = _T("");
	lvItem.iImage = OnGetItemIcon(nItem, pItemInfo);
	lvItem.iItem = nItem;
	lvItem.lParam = (LPARAM)pItemInfo;

	//determine if the item is shared
	DWORD dwAttr = SFGAO_DISPLAYATTRMASK;
	if (pItemInfo) pParentFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&(pItemInfo->pidlRel), &dwAttr);
	else return -1;
	if (dwAttr & SFGAO_SHARE)
	{
		lvItem.mask |= LVIF_STATE;
		lvItem.stateMask |= LVIS_OVERLAYMASK;
		lvItem.state |= INDEXTOOVERLAYMASK(1); //1 is the index for the shared overlay image
	}
	if (dwAttr & SFGAO_GHOSTED)
	{
		lvItem.mask |= LVIF_STATE;
		lvItem.stateMask |= LVIS_CUT;
		lvItem.state |= LVIS_CUT;
	}
	if (bInsert)
	{
		nItem = InsertItem(&lvItem);
	}
	else
	{
		SetItem(&lvItem);
	}
	if (nItem >= 0 && nItem < GetItemCount()) // Set columns:
	{
		const int nColumns = m_wndHeader.GetItemCount();
		for (int iColumn = 0; iColumn < nColumns; iColumn++)
		{
			SetItemText(nItem, iColumn, OnGetItemText(nItem, iColumn, pItemInfo));
		}
	}
	return nItem;
}

void CMyShellListCtrl::OpenSelectedItem()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	BOOL bMulti = FALSE;
	if ((GetKeyState(VK_SHIFT) & 0xFF00) != 0) bMulti = TRUE;
	HRESULT hr = S_OK;

	while (nItem != -1)
	{
		LPAFX_SHELLITEMINFO pInfo = (LPAFX_SHELLITEMINFO)GetItemData(nItem);
		if (pInfo == NULL || pInfo->pidlFQ == NULL || pInfo->pidlRel == NULL)
		{
			ASSERT(FALSE);
			return;
		}
		IShellFolder* psfFolder = pInfo->pParentFolder;
		if (psfFolder == NULL)
		{
			HRESULT hr = SHGetDesktopFolder(&psfFolder);
			if (FAILED(hr))	return;
		}
		else
		{
			psfFolder->AddRef();
		}
		if (psfFolder == NULL) return;

		// 폴더인 경우에는 해당 폴더로 이동 또는 새 탭을 열고 끝낸다.
		ULONG ulAttrs = SFGAO_FOLDER;
		psfFolder->GetAttributesOf(1, (const struct _ITEMIDLIST**)&pInfo->pidlRel, &ulAttrs);

		if (ulAttrs & SFGAO_FOLDER)
		{
			if ((GetKeyState(VK_CONTROL) & 0xFF00) != 0)
			{   //Open in a new tab
				GetParent()->SendMessage(WM_COMMAND, CMD_OpenNewTab, (DWORD_PTR)this);
				psfFolder->Release();
			}
			else
			{
				psfFolder->Release();
				LoadFolder(GetItemFullPath(nItem));
			}
			break;
		}
		else
		{
			if (pInfo->pidlFQ != NULL)
			{
				SHELLEXECUTEINFO sei;
				memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
				sei.fMask = SEE_MASK_IDLIST;
				sei.cbSize = sizeof(SHELLEXECUTEINFO);
				sei.lpVerb = _T("open");
				sei.lpFile = NULL;
				sei.lpIDList = pInfo->pidlFQ;
				sei.nShow = SW_SHOW;
				if (ShellExecuteEx(&sei) == FALSE)
				{
					AfxMessageBox(L"Shell Execution Error"); //Resource
				}
			}
		}
		if (bMulti == TRUE) nItem = GetNextItem(nItem, LVNI_SELECTED);
		else nItem = -1;
		psfFolder->Release();
	}
}

void CMyShellListCtrl::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	OpenSelectedItem();
	//*pResult = 0;
}


void CMyShellListCtrl::DeleteSelected(BOOL bRecycle)
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
	CString strPath;
	while (nItem != -1)
	{
		if (PathFileExists(GetItemFullPath(nItem)) == FALSE)
		{
			//ReleaseItemData(nItem);
			bDeleted = DeleteItem(nItem);
		}
		else
		{
			bDeleted = FALSE;
		}
		if (bDeleted == FALSE) SetItemState(nItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
		nItem = GetNextItem(-1, LVNI_SELECTED);
	}
	//UpdateMsgBar();
	this->SetRedraw(TRUE);
}

void CMyShellListCtrl::RenameSelectedItem()
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

void CMyShellListCtrl::ConvertNFDNames()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	CStringArray aPath;
	while (nItem != -1)
	{
		int nIndex = (int)aPath.Add(GetItemFullPath(nItem));
		RenameFiles(aPath, ConvertNFD(GetItemText(nItem, 0))); //한개씩 호출
		nItem = GetNextItem(nItem, LVNI_SELECTED);
		aPath.RemoveAll();
	}
}

void CMyShellListCtrl::RenameFiles(CStringArray& aPath, CString strNewPath)
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
				::ATL::CComPtr<::CMyShellListProgress> pSink; //이름이 바뀌었을때 가져오기
				pSink.Attach(new CMyShellListProgress{});
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


void CMyShellListCtrl::AddPathHistory(CString strPath)
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

void CMyShellListCtrl::BrowsePathHistory(BOOL bPrevious)
{
	if (m_posPathHistory == NULL || m_aPathHistory.GetSize() < 2) return;
	//m_strFilterExclude.Empty();
	//m_strFilterInclude.Empty();
	if (bPrevious == TRUE && m_posPathHistory != m_aPathHistory.GetHeadPosition())
	{
		m_aPathHistory.GetPrev(m_posPathHistory);
		CString strFolder = m_aPathHistory.GetAt(m_posPathHistory);
		LoadFolder(strFolder, FALSE);
	}
	else if (bPrevious == FALSE && m_posPathHistory != m_aPathHistory.GetTailPosition())
	{
		m_aPathHistory.GetNext(m_posPathHistory);
		CString strFolder = m_aPathHistory.GetAt(m_posPathHistory);
		LoadFolder(strFolder, FALSE);
	}
}

BOOL CMyShellListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDM_FILE_DELETE:	DeleteSelected(((GetKeyState(VK_SHIFT) & 0xFF00) != 0) ? FALSE : TRUE);		break;
	case IDM_FILE_COPY:		ClipBoardExport(FALSE);		break;
	case IDM_FILE_CUT:		ClipBoardExport(TRUE);		break;
	case IDM_FILE_PASTE:	case IDM_PASTE_FILE:		ClipBoardImport();		break; //툴바 또는 메뉴
	//case IDM_CONVERT_NFD:	ConvertNFDNames();		break;
	case IDM_OPEN_PREV:		BrowsePathHistory(TRUE); break;
	case IDM_OPEN_NEXT:		BrowsePathHistory(FALSE); break;
	case IDM_PLAY_ITEM:		OpenSelectedItem(); break;
	case IDM_OPEN_PARENT:	OpenParentFolder(); break;
	case IDM_WATCHEVENT:	WatchEventHandler(); break;
	default:	return CMFCShellListCtrl::OnCommand(wParam, lParam); break;
	}
	return TRUE;
}

void CMyShellListCtrl::InitColumns(int nType)
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

BOOL CMyShellListCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_F5)
		{
			Refresh();
			return TRUE;
		}
		if (pMsg->wParam == VK_RETURN)
		{
			OpenSelectedItem();
			return TRUE;
		}
		if (pMsg->wParam == VK_ESCAPE)
		{
			// Clear Selected
			int nItem = GetNextItem(-1, LVNI_SELECTED);
			while (nItem != -1)
			{
				SetItemState(nItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
				nItem = GetNextItem(nItem, LVNI_SELECTED);
			}
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
		if (pMsg->wParam == VK_F4) // 임시로 단축키 지정, 나중에 변경 검토
		{
			ConvertNFDNames();
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
	return CMFCShellListCtrl::PreTranslateMessage(pMsg);
}

void CMyShellListCtrl::OnDropFiles(HDROP hDropInfo)
{
	BOOL bMove = TRUE;
	ProcessDropFiles(hDropInfo, bMove);
	DragFinish(hDropInfo); //실제로 마우스 드래그 메시지를 받은 경우에만 이 방식으로 메모리 해제
}


void CMyShellListCtrl::ProcessDropFiles(HDROP hDropInfo, BOOL bMove)
{
	if ( (GetItemTypes() & SHCONTF_FOLDERS) ==  0) return;
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
	if (nStart < nEnd && nEnd>1) EnsureVisible(nEnd - 1, FALSE);
	delete[] pszFilePath;
	//DragFinish(hDropInfo); //실제로 마우스 드래그 메시지를 받은 경우에만 이 방식으로 메모리 해제
	//CMFCListCtrl::OnDropFiles(hDropInfo);
}

void CMyShellListCtrl::PasteFiles(CStringArray& aSrcPath, BOOL bMove)
{
	if (m_pidlCurFQ == NULL) return;
	if (aSrcPath.GetSize() == 0) return;

	BOOL bIsSamePath = FALSE;
	CString strOldFolder = Get_Folder(aSrcPath[0], FALSE);
	CString strNewFolder; 
	if (GetCurrentFolder(strNewFolder) == FALSE) return;
	if (strOldFolder.CompareNoCase(strNewFolder) == 0) bIsSamePath = TRUE;

	IShellItemArray* shi_array = NULL;
	if (CreateShellItemArrayFromPaths(aSrcPath, shi_array) == S_OK)
	{
		IFileOperation* pifo = NULL;
		if (CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pifo)) == S_OK)
		{
			IShellItem* pisi = NULL;
			if (SHCreateShellItem(NULL, NULL, GetPIDLfromFullPath(strNewFolder), &pisi) == S_OK)
			{
				DWORD flag = FOFX_ADDUNDORECORD | FOF_ALLOWUNDO;
				if (bIsSamePath == TRUE) flag = flag | FOF_RENAMEONCOLLISION;
				if (pifo->SetOperationFlags(flag) == S_OK &&
					pifo->SetOwnerWindow(this->GetSafeHwnd()) == S_OK)
				{
					if (bMove)	pifo->MoveItems(shi_array, pisi);
					else		pifo->CopyItems(shi_array, pisi);
					//WatchFolder_Suspend();
					SetRedraw(FALSE);
					::ATL::CComPtr<::CMyShellListProgress> pSink; //이름이 바뀌었을때 가져오기
					pSink.Attach(new CMyShellListProgress{});
					pSink->m_pList = this;
					DWORD dwCookie;
					pifo->Advise(pSink, &dwCookie);
					pifo->PerformOperations();
					pifo->Unadvise(dwCookie);
					pSink.Release();
					SetRedraw(TRUE);
					//WatchFolder_Resume();
				}
				if (pisi) pisi->Release();
			}
			if (pifo) pifo->Release();
		}
		if (shi_array) shi_array->Release();
	}
	return;
}

void CMyShellListCtrl::OnLvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	NM_LISTVIEW* pNMListView = pNMLV;
	*pResult = 0;
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

struct CListItem
{
	CListItem() { pList = NULL; nIndex = -1; };
	CListItem(CListCtrl* p, int n) { pList = p; nIndex = n; };
	CListCtrl* pList;
	int nIndex;
};
static CArray<CListItem> st_selected;

void CMyShellListCtrl::ClearPreviousSelection()
{
	for (int i = 0; i < st_selected.GetCount(); i++)
	{
		st_selected[i].pList->SetItemState(st_selected[i].nIndex, 0, LVIS_CUT);
	}
	st_selected.RemoveAll();
}

HGLOBAL CMyShellListCtrl::GetOleDataForClipboard(int nState)
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


void CMyShellListCtrl::ClipBoardExport(BOOL bMove)
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
			if (pdw1 != NULL) (*pdw1) = effect;
			GlobalUnlock(hEffect);
			SetClipboardData(RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT), hEffect);
		}
		SetClipboardData(CF_HDROP, hgDrop);
		CloseClipboard();
	}
}

void CMyShellListCtrl::ClipBoardImport()
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


BOOL CMyShellListCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	BOOL b = CMFCShellListCtrl::Create(dwStyle, rect, pParentWnd, nID);
	if (b)
	{
		//m_DropTarget.m_bMFCShell = TRUE;
		//BOOL b = m_DropTarget.Register(this);
		DragAcceptFiles(TRUE);
	}
	return b;
}


void CMyShellListCtrl::OnClipboardUpdate()
{
	// 이 기능을 사용하려면 Windows Vista 이상이 있어야 합니다.
	// _WIN32_WINNT 기호는 0x0600보다 크거나 같아야 합니다.
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CMFCShellListCtrl::OnClipboardUpdate();
}

void CMyShellListCtrl::OpenParentFolder()
{
	WatchFolder_End();
	if (SUCCEEDED(DisplayParentFolder()))
	{
		if (GetCurrentFolder(m_strCurrentFolder))
		{
			AddPathHistory(m_strCurrentFolder);
			WatchFolder_Begin();
		}
		GetParent()->SendMessage(WM_COMMAND, CMD_UpdateFromList, (LPARAM)this);
	}
}

void CMyShellListCtrl::LoadFolder(CString strFolder, BOOL bAddHistory)
{
	WatchFolder_End();
	if (SUCCEEDED(DisplayFolder(strFolder)))
	{
		if (GetCurrentFolder(m_strCurrentFolder))
		{
			if (bAddHistory) AddPathHistory(strFolder);
			WatchFolder_Begin();
		}
		GetParent()->SendMessage(WM_COMMAND, CMD_UpdateFromList, (LPARAM)this);
	}
}

// 변경사항 모니터링 기능
void CMyShellListCtrl::WatchEventHandler()
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
			//쓰레드 내에서
			switch (pNotify->Action)
			{
			case FILE_ACTION_ADDED:
				TRACE(L"Added : %s\n", szFile);
				AddPath(szFile, TRUE, TRUE);
				//UpdateMsgBar();
				break;
			case FILE_ACTION_REMOVED:
				TRACE(L"Removed : %s\n", szFile);
				DeleteInvalidName(szFile);
				//UpdateMsgBar();
				break;
			case FILE_ACTION_MODIFIED:
				TRACE(L"Modified : %s\n", szFile);
				UpdateItemByPath(szFile, szFile, TRUE, TRUE);
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
					CString& strOldName = strPath;
					UpdateItemByPath(strOldName, szFile, TRUE, TRUE);
					bRename = FALSE;
				}
				break;
			}
		}
		offset += pNotify->NextEntryOffset;
	} while (pNotify->NextEntryOffset != 0);
}


void CMyShellListCtrl::WatchFolder_Suspend() // 별도 쓰레드 방식용
{
	if (m_pThreadWatch == NULL || IsWatching(this) == FALSE) return;
	DWORD dwCount = m_pThreadWatch->SuspendThread();
	if (dwCount > 1)
	{
		TRACE(_T("Too many suspended thread: %d\r\n"), dwCount);
	}
}

void CMyShellListCtrl::WatchFolder_Resume() // 별도 쓰레드 방식용
{
	if (m_pThreadWatch == NULL || IsWatching(this) == FALSE) return;
	DWORD dwCount = m_pThreadWatch->ResumeThread();
	if (dwCount > 1)
	{
		TRACE(_T("Too many suspended thread: %d\r\n"), dwCount);
	}
}


void CMyShellListCtrl::WatchFolder_End() // 별도 쓰레드 방식용
{
	if (IsWatching(this) == FALSE) return;
	HANDLE hThread = m_pThreadWatch->m_hThread;
	if (m_hWatchBreak != NULL) SetEvent(m_hWatchBreak);
	DWORD ret = WaitForSingleObject(hThread, 3000);
	if (ret != WAIT_OBJECT_0)
	{
		AfxMessageBox(L"Error:Watching thread is not cleared properly");
	}
	m_pThreadWatch = NULL;
}

void CMyShellListCtrl::WatchFolder_Work() // 별도 쓰레드 방식용
{
	if (IsWatching(this) == TRUE) return;
	//새로 생성된 쓰레드 내에서는 GetCurrentFolder 등 CMFCShellListCtrl의 함수를 호출할 수 없다.
	//이는 MFC 객체가 Thread safe 하지 않기 때문에 자체적으로 Assertion failure를 내도록 되어 있기 때문이다.
	//그래서 메시징 처리 등 다른 대안이 필요하다.
	CString strDir = m_strCurrentFolder;
	PathBackSlash(strDir);
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
				//쓰레드 safety를 위해 메시지 방식으로 처리 필요
				this->SendMessage(WM_COMMAND, IDM_WATCHEVENT, 0);
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

BOOL CMyShellListCtrl::IsWatchable() //모니터링 가능한 일반적인 폴더인 경우 TRUE 반환
{
	if (::IsWindow(GetSafeHwnd()) == FALSE) return FALSE;
	if (m_pidlCurFQ == NULL) return FALSE;
	if (IsDesktop()) return FALSE;
	if ((GetItemTypes() & SHCONTF_FOLDERS) == 0) return FALSE;
	if (GetItemCount() == 1 && GetItemData(0) == NULL) return FALSE;
	return TRUE;
}

void CMyShellListCtrl::WatchFolder_Begin()
{
	if (IsWatchable() == FALSE) return;
	if (IsWatching(this)) return;
	m_pThreadWatch = AfxBeginThread(WatchFolder_Thread, this);
}

UINT CMyShellListCtrl::WatchFolder_Thread(void* lParam)
{
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY))) return 0;
	CMyShellListCtrl* pList = (CMyShellListCtrl*)lParam;
	pList->WatchFolder_Work();
	CoUninitialize();
	return 0;
}

void CMyShellListCtrl::SetWatchingStatus(CMyShellListCtrl* pList, BOOL bLoading)
{
	SetThreadStatus(st_mapWatching, pList, bLoading);
}
BOOL CMyShellListCtrl::IsWatching(CMyShellListCtrl* pList)
{
	return IsThreadOn(st_mapWatching, pList);
}
void CMyShellListCtrl::DeleteWatchingStatus(CMyShellListCtrl* pList)
{
	DeleteThreadStatus(st_mapWatching, pList);
}