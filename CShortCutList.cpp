#include "pch.h"
#include "FileOfficer.h"
#include "CShortCutList.h"
#include "EtcFunctions.h"

BEGIN_MESSAGE_MAP(CShortCutList, CMFCListCtrl)
	ON_WM_DROPFILES()
	ON_WM_CTLCOLOR()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, &CShortCutList::OnLvnBegindrag)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CShortCutList::OnNMDblclk)
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

CShortCutList::CShortCutList()
{
	CMD_OpenFolderByShortCut = 0;
	m_nViewType = LVS_ICON;
	m_nIconType = SHIL_EXTRALARGE;
}

CShortCutList::~CShortCutList()
{

}

COLORREF CShortCutList::OnGetCellTextColor(int nRow, int nColumn)
{
	return ApplyColorRule(nRow, nColumn, FALSE);
}

COLORREF CShortCutList::OnGetCellBkColor(int nRow, int nColumn)
{
	return ApplyColorRule(nRow, nColumn, TRUE);
}

COLORREF CShortCutList::ApplyColorRule(int nRow, int nColumn, BOOL bBk)
{
	COLORREF color = bBk ? GetBkColor() : GetTextColor();
	return color;
}

BOOL CShortCutList::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_F5)
		{
			Refresh();
			return TRUE;
		}
		else if (pMsg->wParam == VK_RETURN)
		{
			if ((GetKeyState(VK_CONTROL) & 0xFF00) != 0) ViewSelectedItemFolder();
			else OpenSelectedItem();
			return TRUE;
		}
		else if (pMsg->wParam == VK_DELETE)
		{
			if ((GetKeyState(VK_CONTROL) & 0xFF00) != 0) DeleteSelectedItem();
			else RemoveSelectedItem();
			return TRUE;
		}
		if ((GetKeyState(VK_CONTROL) & 0xFF00) != 0)
		{
			if (pMsg->wParam == _T('C')) { ClipBoardExport(FALSE); return TRUE; }
			if (pMsg->wParam == _T('X')) { ClipBoardExport(TRUE); return TRUE; }
			//if (pMsg->wParam == _T('V')) { ClipBoardImport(); return TRUE; }
			if (pMsg->wParam == _T('A'))
			{
				SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
				return TRUE;
			}
			if (pMsg->wParam == VK_UP) { MoveSelectedItem(TRUE); return TRUE; }
			if (pMsg->wParam == VK_DOWN) { MoveSelectedItem(FALSE); return TRUE; }
		}
	}
	return CMFCListCtrl::PreTranslateMessage(pMsg);
}

BOOL CShortCutList::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDM_SHORTCUT_OPEN:	OpenSelectedItem(); break;
	case IDM_SHORTCUT_REMOVEFROMLIST:	RemoveSelectedItem(); break;
	//항목이 속한 폴더 보여주기, 폴더인 경우에는 해당 폴더의 상위 폴더 보여주기
	case IDM_SHORTCUT_VIEWFOLDER: ViewSelectedItemFolder();  break;
	//파일 복사/잘라내기/삭제 (붙여넣기는 해당 없음)
	case IDM_SHORTCUT_COPY: ClipBoardExport(FALSE); break;
	case IDM_SHORTCUT_CUT: ClipBoardExport(TRUE); break;
	case IDM_SHORTCUT_DELETE: DeleteSelectedItem();  break;
	case IDM_SHORTCUT_REFRESH: Refresh();  break;
	case IDM_SHORTCUT_UP: MoveSelectedItem(TRUE);  break;
	case IDM_SHORTCUT_DOWN: MoveSelectedItem(FALSE);  break;
	case IDM_SHORTCUT_ICONVIEW: SetViewType(LVS_ICON); break;
	case IDM_SHORTCUT_LISTVIEW: SetViewType(LVS_LIST);	break;
	case IDM_SHORTCUT_REPORTVIEW: SetViewType(LVS_REPORT);	break;
	case IDM_SHORTCUT_ICONSIZE_16: SetIconType(SHIL_SMALL);	break;
	case IDM_SHORTCUT_ICONSIZE_32: SetIconType(SHIL_LARGE);	break;
	case IDM_SHORTCUT_ICONSIZE_48: SetIconType(SHIL_EXTRALARGE);	break;
	case IDM_SHORTCUT_ICONSIZE_256: SetIconType(SHIL_JUMBO);	break;
	default:
		return CMFCListCtrl::OnCommand(wParam, lParam);
	}
	return TRUE;
}

int CShortCutList::GetFileImageIndex(CString strPath, DWORD dwAttribute)
{
	SHFILEINFO sfi;
	memset(&sfi, 0x00, sizeof(sfi));
	sfi.dwAttributes = dwAttribute;
	DWORD flag = SHGFI_ICON | SHGFI_OVERLAYINDEX;
//	if (strPath.GetLength() < MAX_PATH && dwAttribute != INVALID_FILE_ATTRIBUTES)
//	{
//		SHGetFileInfo(strPath, dwAttribute, &sfi, sizeof(sfi), flag | SHGFI_USEFILEATTRIBUTES);
//	}
//	else
//	{
		LPITEMIDLIST pidl = GetPIDLfromPath(strPath);
		SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi), flag | SHGFI_PIDL);
		CoTaskMemFree(pidl);
//	}
	int nIconIndex = LOWORD(sfi.iIcon);
	return nIconIndex;
}

void CShortCutList::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	TCHAR* pszFilePath = new TCHAR[MY_MAX_PATH]; pszFilePath[0] = _T('\0');
	size_t bufsize = sizeof(TCHAR) * MY_MAX_PATH;
	if (bufsize > 0) ZeroMemory(pszFilePath, bufsize);
	WORD cFiles = DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	for (int i = 0; i < cFiles; i++)
	{
		DragQueryFile(hDropInfo, i, pszFilePath, MY_MAX_PATH);
		InsertPath(-1, pszFilePath);
	}
	delete[] pszFilePath;
}

void CShortCutList::InsertPath(int nItem, CString strPath)
{
	if (nItem == -1) nItem = GetItemCount();
	DWORD dw = GetFileAttributes(strPath);
	if (dw != INVALID_FILE_ATTRIBUTES)
	{
		int nCount = GetItemCount();
		CString strTemp;
		BOOL bExist = FALSE;
		for (int i = 0; i < nCount; i++)
		{
			if (strPath.CompareNoCase(GetItemFullPath(i)) == 0)
			{
				bExist = TRUE;
				break;
			}
		}
		if (bExist == FALSE)
		{
			nItem = InsertItem(nItem, Get_Name(strPath), GetFileImageIndex(strPath, dw));
			SetItemText(nItem, 1, strPath);
			SetItemData(nItem, dw);
		}
	}
}


void CShortCutList::OnLvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	//
	*pResult = 0;

}


void CShortCutList::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if ((GetKeyState(VK_CONTROL) & 0xFF00) != 0) ViewSelectedItemFolder();
	else OpenSelectedItem();
	*pResult = 0;
}

void CShortCutList::RemoveSelectedItem()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	while(nItem != -1)
	{
		DeleteItem(nItem);
		nItem = GetNextItem(-1, LVNI_SELECTED);
	}
}

void CShortCutList::DeleteSelectedItem()
{
	BOOL bRecycle = TRUE; //일단은 휴지통을 항상 사용
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
				pifo->PerformOperations();
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
		if (IsItemExist(nItem))
		{	//실제로는 안지워진 경우
			SetItemState(nItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
		}
		else
		{	//지워진 경우
			DeleteItem(nItem);
		}
		nItem = GetNextItem(-1, LVNI_SELECTED);
	}
	this->SetRedraw(TRUE);
}

void CShortCutList::ViewSelectedItemFolder()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	//맨 첫번째 한개를 기준으로 한다.
	//LPARAM이 0 = 해당 폴더 내용을 열기 / LPARAM이 1 = 상위 폴더 열고 선택하기
	GetParent()->SendMessage(WM_COMMAND, CMD_OpenFolderByShortCut, 1);

	/*if (IsDir(nItem))
	{
		GetParent()->SendMessage(WM_COMMAND, CMD_OpenFolderByShortCut, 0); 
	}
	else 
	{
		GetParent()->SendMessage(WM_COMMAND, CMD_OpenFolderByShortCut, 1); 
	}*/
}

void CShortCutList::OpenSelectedItem()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	IShellFolder* pisf = NULL;
	//루트(데스크탑)의 IShellFolder 인터페이스 얻어오기
	if (FAILED(SHGetDesktopFolder(&pisf))) return;
	if (IsDir(nItem) == FALSE) //일반 파일
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
		//LPARAM이 0 = 해당 폴더 내용을 열기 / LPARAM이 1 = 상위 폴더 열고 선택하기
		GetParent()->SendMessage(WM_COMMAND, CMD_OpenFolderByShortCut, 0);
	}
	pisf->Release();
}


void CShortCutList::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_MENU_SHORTCUT);
	CMenu* pMenu = menu.GetSubMenu(0);
	if (GetNextItem(-1, LVNI_SELECTED) == -1)
	{
		pMenu->EnableMenuItem(IDM_SHORTCUT_OPEN, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SHORTCUT_COPY, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SHORTCUT_CUT, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SHORTCUT_DELETE, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SHORTCUT_DOWN, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SHORTCUT_REMOVEFROMLIST, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SHORTCUT_UP, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SHORTCUT_VIEWFOLDER, MF_BYCOMMAND | MF_GRAYED);
	}
	switch (m_nIconType)
	{
	case SHIL_SMALL: pMenu->CheckMenuItem(IDM_SHORTCUT_ICONSIZE_16, MF_BYCOMMAND | MF_CHECKED);  break;
	case SHIL_LARGE: pMenu->CheckMenuItem(IDM_SHORTCUT_ICONSIZE_32, MF_BYCOMMAND | MF_CHECKED);  break;
	case SHIL_EXTRALARGE: pMenu->CheckMenuItem(IDM_SHORTCUT_ICONSIZE_48, MF_BYCOMMAND | MF_CHECKED);  break;
	case SHIL_JUMBO: pMenu->CheckMenuItem(IDM_SHORTCUT_ICONSIZE_256, MF_BYCOMMAND | MF_CHECKED);  break;
	}
	switch (m_nViewType)
	{
	case LVS_ICON: pMenu->CheckMenuItem(IDM_SHORTCUT_ICONVIEW, MF_BYCOMMAND | MF_CHECKED);  break;
	case LVS_LIST: pMenu->CheckMenuItem(IDM_SHORTCUT_LISTVIEW, MF_BYCOMMAND | MF_CHECKED);  break;
	case LVS_REPORT: pMenu->CheckMenuItem(IDM_SHORTCUT_REPORTVIEW, MF_BYCOMMAND | MF_CHECKED);  break;
	}
	pMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
}


void CShortCutList::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CMFCListCtrl::OnLButtonUp(nFlags, point);
}


void CShortCutList::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CMFCListCtrl::OnMouseMove(nFlags, point);
}


HGLOBAL CShortCutList::GetOleDataForClipboard(int nState)
{
	ListItemArray& aCut = APP()->m_aCutItem;
	CStringList aFiles;
	CString strPath;
	size_t uBuffSize = 0;
	APP()->ClearPreviousCutItems(); // CUT으로 인한 음영처리를 위한 부분
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return NULL;
	while (nItem != -1)
	{
		strPath = GetItemFullPath(nItem);
		aFiles.AddTail(strPath);
		SetItemState(nItem, nState, LVIS_CUT);
		aCut.Add(CListItem(this, nItem));  // CUT으로 인한 음영처리를 위한 부분
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


void CShortCutList::ClipBoardExport(BOOL bMove)
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

void CShortCutList::Refresh()
{
	int nLast = GetItemCount() - 1 ;
	for (int i = nLast; i >= 0; i--)
	{
		if (IsItemExist(i) == FALSE) DeleteItem(i);
	}
}


//선택된 리스트 아이템을 한칸씩 이동한다.
void CShortCutList::MoveSelectedItem(BOOL bUp)
{
	int nLastItem = -1;
	if (bUp) //위로
	{
		int nItem = GetNextItem(-1, LVNI_SELECTED);
		if (nItem == 0) return; //맨 위가 선택된 경우
		int nLast = GetItemCount() - 1;
		for (int i = 1; i <= nLast; i++)
		{
			if (GetItemState(i, LVIS_SELECTED) != 0)
			{
				SwapItem(i, i - 1);
				nLastItem = i - 1;
			}
		}
	}
	else // 아래로
	{
		int nItem = GetNextItem(GetItemCount() - 2, LVNI_SELECTED);
		if (nItem != -1) return; //맨 아래가 선택된 경우
		int nLast = GetItemCount() - 2;
		for (int i = nLast; i >= 0; i--)
		{
			if (GetItemState(i, LVIS_SELECTED) != 0)
			{
				SwapItem(i, i + 1);
				nLastItem = i + 1;
			}
		}
	}
	EnsureVisible(nLastItem, FALSE);
}


//두개의 아이템 교환
void CShortCutList::SwapItem(int n1, int n2)
{
	CString str1, str2;
	int nCol = 2, i = 0;

	//아이템 데이타 교환
	DWORD_PTR dw1 = GetItemData(n1);
	DWORD_PTR dw2 = GetItemData(n2);
	SetItemData(n1, dw2);
	SetItemData(n2, dw1);

	//아이콘 이미지 교환
	LVITEM li;
	memset(&li, 0, sizeof(LVITEM));
	li.mask = LVIF_IMAGE;
	li.iItem = n1;
	GetItem(&li);
	int img1 = li.iImage;
	li.iItem = n2;
	GetItem(&li);
	int img2 = li.iImage;

	SetItem(n1, 0, LVIF_IMAGE, NULL, img2, 0, 0, 0);
	SetItem(n2, 0, LVIF_IMAGE, NULL, img1, 0, 0, 0);

	//칼럼 정보 교환 
	for (i = 0; i < nCol; i++)
	{
		str1 = GetItemText(n1, i);
		str2 = GetItemText(n2, i);
		SetItemText(n1, i, str2);
		SetItemText(n2, i, str1);
	}

	//아이템 상태 교환
	DWORD d1 = GetItemState(n1, LVIS_SELECTED | LVIS_FOCUSED);
	DWORD d2 = GetItemState(n2, LVIS_SELECTED | LVIS_FOCUSED);
	SetItemState(n1, d2, LVIS_SELECTED | LVIS_FOCUSED);
	SetItemState(n2, d1, LVIS_SELECTED | LVIS_FOCUSED);
}

void CShortCutList::SetViewType(int nType)
{
	m_nViewType = nType;
	BOOL bReport = ((nType & LVS_REPORT) != 0) ? TRUE : FALSE;
	if (bReport) nType = nType | LVS_NOSORTHEADER;
	ModifyStyle(LVS_ICON | LVS_REPORT | LVS_SMALLICON | LVS_LIST, nType);
	if (bReport)
	{
		CRect rc;
		GetClientRect(rc);
		SetColumnWidth(0, rc.Width());
		SetColumnWidth(1, 0);
	}
	RedrawWindow();
}