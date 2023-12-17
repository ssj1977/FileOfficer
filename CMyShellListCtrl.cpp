// CMyShellListCtrl.cpp: 구현 파일
//

#include "pch.h"
#include "FileOfficer.h"
#include "CMyShellListCtrl.h"
#include <vector>
#include <map>
#include "CDlgInput.h"
#include "EtcFunctions.h"
#include "CFileListContextMenu.h"

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

// IFileOperation 에서 변경된 파일명을 받아오기 위한 IFileOperationProgressSink 구현
LPITEMIDLIST GetPIDLfromPath(CString strPath)
{
	if (strPath.GetLength() < MAX_PATH) return ILCreateFromPath(strPath);
	LPITEMIDLIST pidl_result = NULL;
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

HRESULT CreateShellItemArrayFromPaths(CStringArray& aPath, IShellItemArray*& shi_array)
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

IFACEMETHODIMP CMyShellListProgress::PostCopyItem(DWORD dwFlags, IShellItem* psiItem,
	IShellItem* psiDestinationFolder, PCWSTR pwszNewName, HRESULT hrCopy,
	IShellItem* psiNewlyCreated)
{
	if (m_pList)
	{
		m_pList->AddPath(pwszNewName, TRUE, TRUE);
		m_pList->UpdateMsgBar();
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
		m_pList->UpdateMsgBar();
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
	CMD_OpenNewTabByList = 0;
	CMD_UpdateSortInfo = 0;
	CMD_UpdateFromList = 0;
	CMD_UpdateBar = 0;
	m_nIconType = SHIL_SMALL;
	m_bAscInit = TRUE;
	m_nSortColInit = 0;
	m_hDirectory = NULL;
	m_hWatchBreak = NULL;
	m_pThreadWatch = NULL;
	m_pWatchBuffer = malloc(WATCH_BUFFER_SIZE);
	m_posPathHistory = NULL;
	m_bLoading = FALSE;
	m_pColorRuleArray = NULL;
	m_bMenuOn = FALSE;
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
	ON_NOTIFY(HDN_ITEMCLICKA, 0, &CMyShellListCtrl::OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, &CMyShellListCtrl::OnHdnItemclick)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CMyShellListCtrl::OnLvnItemchanged)
	ON_WM_CONTEXTMENU()
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
	if (pItemInfo)
	{
		//this may be NULL if this is the root item
		if (pItemInfo->pParentFolder != NULL)
		{
			if (pItemInfo->pidlFQ) afxShellManager->FreeItem(pItemInfo->pidlFQ);
			if (pItemInfo->pidlRel)	afxShellManager->FreeItem(pItemInfo->pidlRel);
			pItemInfo->pParentFolder->Release();
			pItemInfo->pParentFolder = NULL;
		}
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
				GetParent()->SendMessage(WM_COMMAND, CMD_OpenNewTabByList, (DWORD_PTR)this);
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
	UpdateMsgBar();
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
	int nWidth = 0;
	for (int i = 0; i < nCount; i++)
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
		SetColumnWidth(i, nWidth);
	}
}

BOOL CMyShellListCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_F5)
		{
			CString strPath;
			if (GetCurrentFolder(strPath)) LoadFolder(strPath, FALSE);
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
	}
}

HGLOBAL CMyShellListCtrl::GetOleDataForClipboard(int nState)
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
		APP()->ClearPreviousCutItems();
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
		InitColumns(m_nIconType);
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
	UpdateMsgBar(IDS_NOW_LOADING);
	m_bLoading = TRUE;
	clock_t s = clock();
	if (SUCCEEDED(DisplayFolder_CustomSort(strFolder)))
	{
		if (GetCurrentFolder(m_strCurrentFolder))
		{
			if (bAddHistory) AddPathHistory(strFolder);
			WatchFolder_Begin();
		}
		GetParent()->SendMessage(WM_COMMAND, CMD_UpdateFromList, (LPARAM)this);
	}
	m_bLoading = FALSE;
	clock_t e = clock();
	TRACE(L"%s / Display Time : %d\n", strFolder, e - s);
	UpdateMsgBar();
}

void CMyShellListCtrl::UpdateMsgBar(int nStringID)
{
	if (CMD_UpdateBar != 0 && GetParent() != NULL) GetParent()->SendMessage(WM_COMMAND, CMD_UpdateBar, nStringID);
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
				UpdateMsgBar();
				break;
			case FILE_ACTION_REMOVED:
				TRACE(L"Removed : %s\n", szFile);
				DeleteInvalidName(szFile);
				UpdateMsgBar();
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

void CMyShellListCtrl::OnHdnItemclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	Default();
	m_bAscInit = GetHeaderCtrl().IsAscending();
	m_nSortColInit = GetHeaderCtrl().GetSortColumn();
	GetParent()->PostMessageW(WM_COMMAND, CMD_UpdateSortInfo, (DWORD_PTR)this);
	*pResult = 0;
}


//처음 읽을때 정렬작업이 두번 발생하는 경우를 방지하기 위해 CMFCShellListCtrl의 코드를 일부 수정하여 사용
HRESULT CMyShellListCtrl::DisplayFolder_CustomSort(LPAFX_SHELLITEMINFO pItemInfo)
{
	HRESULT hr = E_FAIL;

	if (afxShellManager == NULL)
	{
		ASSERT(FALSE);
		return hr;
	}

	if (pItemInfo != NULL)
	{
		ReleaseCurrFolder();
		hr = LockCurrentFolder(pItemInfo);

		if (FAILED(hr))
		{
			return hr;
		}
	}

	DeleteAllItems();

	if (m_psfCurFolder != NULL)
	{
		CWaitCursor wait;
		SetRedraw(FALSE);

		hr = EnumObjects(m_psfCurFolder, m_pidlCurFQ);

		if (GetStyle() & LVS_REPORT)
		{
			//이부분이 다르다
			//Sort(AFX_ShellList_ColumnName);
			Sort(m_nSortColInit, m_bAscInit);
		}

		SetRedraw(TRUE);
		RedrawWindow();
	}

	if (SUCCEEDED(hr) && pItemInfo != NULL)
	{
		CMFCShellTreeCtrl* pTree = GetRelatedTree();
		if (pTree != NULL && !m_bNoNotify)
		{
			ASSERT_VALID(pTree);
			pTree->SelectPath(m_pidlCurFQ);
		}

		if (GetParent() != NULL)
		{
			GetParent()->SendMessage(AFX_WM_CHANGE_CURRENT_FOLDER);
		}
	}

	return hr;
}

HRESULT CMyShellListCtrl::DisplayFolder_CustomSort(LPCTSTR lpszPath)
{
	if (afxShellManager == NULL)
	{
		ASSERT(FALSE);
		return E_FAIL;
	}

	ENSURE(lpszPath != NULL);
	ASSERT_VALID(afxShellManager);

	AFX_SHELLITEMINFO info;
	HRESULT hr = afxShellManager->ItemFromPath(lpszPath, info.pidlRel);

	if (FAILED(hr))
	{
		return hr;
	}

	LPSHELLFOLDER pDesktopFolder;
	hr = SHGetDesktopFolder(&pDesktopFolder);

	if (SUCCEEDED(hr))
	{
		info.pParentFolder = pDesktopFolder;
		info.pidlFQ = info.pidlRel;

		hr = DisplayFolder_CustomSort(&info);
		pDesktopFolder->Release();
	}

	afxShellManager->FreeItem(info.pidlFQ);
	return hr;
}

ULONGLONG Str2Size(CString str);
CString GetFileSizeString(ULONGLONG nSize, int nUnit);

CString CMyShellListCtrl::GetBarString()
{
	if (::IsWindow(GetSafeHwnd()) == FALSE) return _T("");
	CString strReturn, strInfo;

	int nSelected = GetSelectedCount();

	if (nSelected == 0) //선택 항목이 하나도 없으면 항목 개수만 표시
	{
		strReturn.Format(_T("%d%s"), GetItemCount(), (LPCTSTR)IDSTR(IDS_ITEM_COUNT));
		return strReturn;
	}

	int nItem = GetNextItem(-1, LVNI_SELECTED);
	CString strSize = GetItemText(nItem, AFX_ShellList_ColumnSize);
	CString strModified = GetItemText(nItem, AFX_ShellList_ColumnModified);

	if (nSelected == 1)  // 선택 항목이 한개인 경우
	{
		if (strSize.IsEmpty() == FALSE && strModified.IsEmpty() == FALSE)
		{
			strInfo.Format(_T(" / %s / %s"), strModified, strSize);
		}
		else if (strSize.IsEmpty() != FALSE && strModified.IsEmpty() == FALSE)
		{
			strInfo.Format(_T(" / %s"), strModified);
		}
		else if (strSize.IsEmpty() == FALSE && strModified.IsEmpty() != FALSE)
		{
			strInfo.Format(_T(" / %s"), strSize);
		}
	}
	else if (nSelected > 1) // 선택 항목이 여러개인 경우
	{
		ULONGLONG total_size = 0;
		while (nItem != -1)
		{
			// 선택이 여러개인 경우 크기 합산하여 출력
			total_size += Str2Size(GetItemText(nItem, AFX_ShellList_ColumnSize));
			nItem = GetNextItem(nItem, LVNI_SELECTED);
		}
		if (total_size > 0) strInfo.Format(_T(" / %s"), GetFileSizeString(total_size, 1));
	}

	strReturn.Format(_T("%d%s / %d%s%s"), GetItemCount(), (LPCTSTR)IDSTR(IDS_ITEM_COUNT),
		 GetSelectedCount(), (LPCTSTR)IDSTR(IDS_SELECTED_COUNT), strInfo);
	return strReturn;
}

void CMyShellListCtrl::OnLvnItemchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_bLoading == TRUE) return;
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	UpdateMsgBar();
	*pResult = 0;
}

COLORREF CMyShellListCtrl::OnGetCellTextColor(int nRow, int nColumn)
{
	return ApplyColorRule(nRow, nColumn, FALSE);
}

COLORREF CMyShellListCtrl::OnGetCellBkColor(int nRow, int nColumn)
{
	return ApplyColorRule(nRow, nColumn, TRUE);
}



COLORREF CMyShellListCtrl::ApplyColorRule(int nRow, int nColumn, BOOL bBk)
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
		LPAFX_SHELLITEMINFO pItemInfo = (LPAFX_SHELLITEMINFO)GetItemData(nRow);
		BOOL bIsDir = FALSE; 		//속도를 감안하여 한번만 처리하도록 관련 값은 미리 세팅
		//		SHFILEINFO sfi;
		//if (SHGetFileInfo((LPCTSTR)pItemInfo->pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_ATTRIBUTES))
		//{
			//if (sfi.dwAttributes & SFGAO_FOLDER) bIsDir = TRUE;
		//}
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
						strName = GetItemText(nRow, AFX_ShellList_ColumnName);
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
					strName = GetItemText(nRow, AFX_ShellList_ColumnName);
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
					strDate = GetItemText(nRow, AFX_ShellList_ColumnModified);
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
				if (nColumn == AFX_ShellList_ColumnName) bMatch = TRUE;
				break;
			case COLOR_RULE_COLDATE: //변경일시 컬럼 전체 
				if (nColumn == AFX_ShellList_ColumnModified) bMatch = TRUE;
				break;
			case COLOR_RULE_COLSIZE: //크기 컬럼 전체 
				if (nColumn == AFX_ShellList_ColumnSize) bMatch = TRUE;
				break;
			case COLOR_RULE_COLTYPE: //파일 종류 컬럼 전체 
				if (nColumn == AFX_ShellList_ColumnType) bMatch = TRUE;
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


void CMyShellListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (m_bMenuOn == TRUE) return; 		//메뉴가 이미 떠있는지 체크하고 표시	
	if (GetNextItem(-1, LVNI_SELECTED) == -1 || pWnd == NULL || point == CPoint(0,0) )
	{
		//빈 공간용 기본 메뉴 표시
		ShowContextMenu(NULL);
	}
	else
	{
		CMFCShellListCtrl::OnContextMenu(pWnd, point);
	}
}

void CMyShellListCtrl::ShowContextMenu(CPoint* pPoint)
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
	context_menu.SetPathArray(m_strCurrentFolder, aSelectedPath);
	UINT idCommand = context_menu.ShowContextMenu(this, pt);
	if (idCommand) GetParent()->PostMessage(WM_COMMAND, idCommand, 0);
	m_bMenuOn = FALSE;
}