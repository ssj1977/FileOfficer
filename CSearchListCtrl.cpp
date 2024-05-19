#include "pch.h"
#include "resource.h"
#include "FileOfficer.h"
#include "CSearchListCtrl.h"
#include "EtcFunctions.h"

//#ifndef NUM_OF_COLUMNS
#define NUM_OF_COLUMNS 6 // �Ϲ����� ���� ��ϰ� �ٸ��� ����
#define COL_SEARCH_NAME 0
#define COL_SEARCH_FOLDER 1
#define COL_SEARCH_DATE 2
#define COL_SEARCH_SIZE 3
#define COL_SEARCH_TYPE 4
#define COL_SEARCH_MEMO 5
//#endif

#define ID_SHELL_MENU_BASE 10000

IMPLEMENT_DYNAMIC(CSearchListCtrl, CFileListCtrl_Base)
BEGIN_MESSAGE_MAP(CSearchListCtrl, CFileListCtrl_Base)
	//ON_NOTIFY(HDN_ITEMCLICKA, 0, &CSearchListCtrl::OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, &CSearchListCtrl::OnHdnItemclick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CSearchListCtrl::OnNMDblclk)
	//ON_COMMAND_RANGE(ID_SHELL_MENU_BASE, ID_SHELL_MENU_BASE + 0x7FFF, &CYourView::OnShellMenuCommand)
END_MESSAGE_MAP()

CSearchListCtrl::CSearchListCtrl()
{
	//m_aNameMatch; // �̸� ����
	//m_aExtMatch; // Ȯ���� ����
	//m_dtFrom; // �Ͻ� ���� (���۽���)
	//m_dtUntil; // �Ͻ� ���� (�������)
	m_bSizeMin = FALSE; // ũ�� ���� (�ּ�)
	m_bSizeMax = FALSE; // ũ�� ���� (�ִ�)
	m_sizeMin = 0;
	m_sizeMax = 0;
	
	m_bUseFileType = TRUE;
	m_bCheckOpen = TRUE;
	//���� ���� �÷��׵�
	m_bWorking = FALSE;
	m_bBreak = FALSE;
}

CSearchListCtrl::~CSearchListCtrl()
{

}

void CSearchListCtrl::InitColumns()
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
	int string_id[] = { IDS_COL_NAME_FOLDER, IDS_COL_DRIVE_PATH, IDS_COL_DATE_FOLDER, IDS_COL_SIZE_FOLDER, IDS_COL_TYPE_FOLDER, IDS_COL_MEMO };
	int col_fmt[] = { LVCFMT_LEFT , LVCFMT_LEFT , LVCFMT_RIGHT, LVCFMT_RIGHT, LVCFMT_LEFT, LVCFMT_LEFT };
	int sort_type[] = { COL_COMP_PATH, COL_COMP_STR, COL_COMP_STR, COL_COMP_SIZE, COL_COMP_STR, COL_COMP_STR, COL_COMP_STR };
	SetColTexts(string_id, col_fmt, sort_type, NUM_OF_COLUMNS);
}



void CSearchListCtrl::FileSearch_Begin()
{
	DeleteAllItems();
	SetSortColumn(-1, m_bAscending);
	//�������� ���ڿ� ���ǿ� ���� ��ū �����
	GetStringArray(m_SC.strName, L'/', m_aNameMatch);
	GetStringArray(m_SC.strExt, L'/', m_aExtMatch);
	//��¥+�ð� ���� �Ľ�
	m_dtFrom.ParseDateTime(m_SC.strDateTimeFrom);
	m_dtUntil.ParseDateTime(m_SC.strDateTimeUntil);
	//ũ�� ���� ����
	m_bSizeMax = FALSE;
	m_bSizeMin = FALSE;
	if (m_SC.ValidateCriteriaSize() == TRUE)
	{
		if (m_SC.strSizeMin.IsEmpty() == FALSE)
		{
			m_sizeMin = Str2Size(m_SC.strSizeMin);
			m_bSizeMin = TRUE;
		}
		if (m_SC.strSizeMax.IsEmpty() == FALSE)
		{
			m_sizeMax = Str2Size(m_SC.strSizeMax);
			m_bSizeMax = TRUE;
		}
	}
	//ã�� ����
	CWinThread* pThread = AfxBeginThread(FileSearch_RunThread, this);
	// if (m_iSortedColumn>=0 && m_iSortedColumn < GetHeaderCtrl().GetItemCount())Sort(m_iSortedColumn, m_bAscending);
}

UINT CSearchListCtrl::FileSearch_RunThread(void* lParam)
{
	CSearchListCtrl* pList = (CSearchListCtrl*)lParam;
	pList->m_bWorking = TRUE; 
	pList->m_bBreak = FALSE;
	pList->FileSearch_Do(pList->m_SC.strStartPath);
	pList->m_bWorking = FALSE;
	if (pList->m_bBreak == FALSE)
	{
		pList->m_strMsg.Format(IDSTR(IDS_SEARCH_MSG_FINISHED), pList->GetItemCount());
	}
	else
	{
		pList->m_strMsg.Format(IDSTR(IDS_SEARCH_MSG_STOPPED), pList->GetItemCount());
	}
	pList->GetParent()->PostMessage(WM_COMMAND, IDM_SEARCH_MSG, 1); //lParam = 1 �̸� ����

	return 0;
}

void CSearchListCtrl::FileSearch_Do(CString strFolder)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	
	//���� �ش� ������ �����ϴ� �������� Ȯ��
	hFind = FindFirstFileExW(strFolder, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	if (hFind == INVALID_HANDLE_VALUE) return;
	FindClose(hFind); 
	hFind = NULL;
	if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) return;
	
	//�����ϴ� ������� '*'�� �ٿ��� Ž�� ����
	CString strFind = PathBackSlash(strFolder) + L'*';
	hFind = FindFirstFileExW(strFind, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	int nCount = 0;
	CString strSize, strDate, strType;
	DWORD dwItemData = 0;
	int nItem = -1;
	size_t nLen = 0;
	COleDateTime tTemp;
	BOOL b = TRUE, bIsDir = FALSE, bDoSearch = FALSE;
	CString fullpath;
	BOOL bIsDot = FALSE;
	ULARGE_INTEGER filesize;
	CStringArray aSubFolders; // ���ȣ��� �������� ����

	//���� ���� �ִ� ���� ��ġ�� ǥ�����ش�
	m_strMsg.Format(_T("�˻��� : %s"), (LPCTSTR)strFolder); //���ҽ� ó�� �ʿ�
	GetParent()->PostMessage(WM_COMMAND, IDM_SEARCH_MSG, 0); //lParam = 0 �̸� �Ϲ�

	while (b && !(m_bBreak))
	{
		dwItemData = fd.dwFileAttributes;
		bIsDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
		nLen = _tcsclen(fd.cFileName);
		// '.', '..' �� ��츦 �ĺ��Ͽ� ����
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
			fullpath = PathBackSlash(strFolder) + fd.cFileName;
			//���� ����� ��� ������ �ξ��ٰ� ���߿� ���ȣ��
			if (bIsDir)
			{
				aSubFolders.Add(fullpath);
			}
			// 0 : ���ϸ� / 1 : ������ / 2 : ��� �˻�
			bDoSearch = FALSE;
			if (m_SC.nTargetType == 0 && bIsDir == FALSE) bDoSearch = TRUE;
			else if (m_SC.nTargetType == 1 && bIsDir == TRUE) bDoSearch = TRUE;
			else if (m_SC.nTargetType == 2) bDoSearch = TRUE;
			if (bDoSearch)
			{
				BOOL bMatch = TRUE;
				//���� �˻� : ���� ����
				if (m_SC.bLocked || m_SC.bHidden || m_SC.bReadOnly || m_SC.bEncrypted) bMatch = IsMatch_State(fd, fullpath);
				//���� �˻� : ���� ũ�� (�������� ������� ����)
				if (bMatch == TRUE && bIsDir == FALSE && (m_bSizeMax || m_bSizeMin)) bMatch = IsMatch_Size(fd);
				//���� �˻� : ���� ���� ����
				if (bMatch == TRUE && (m_SC.bDateTimeFrom || m_SC.bDateTimeUntil)) bMatch = IsMatch_Time(fd);
				//���� �˻� : ���ϸ�
				if (bMatch == TRUE && m_aNameMatch.GetCount() > 0) bMatch = IsMatch_Name(fd);
				//���� �˻� : Ȯ���� (�������� ������� ����)
				if (bMatch == TRUE && bIsDir == FALSE && m_aExtMatch.GetCount() > 0) bMatch = IsMatch_Ext(fd);
				// ������ �´� ���ϸ� ǥ��
				if (bMatch == TRUE)
				{
					tTemp = COleDateTime(fd.ftLastWriteTime);
					strDate = tTemp.Format(_T("%Y-%m-%d %H:%M:%S"));
					if (bIsDir == FALSE)
					{
						filesize.HighPart = fd.nFileSizeHigh;
						filesize.LowPart = fd.nFileSizeLow;
						strSize = GetFileSizeString(filesize.QuadPart, 0);
					}
					else strSize.Empty();
					nItem = InsertItem(GetItemCount(), fd.cFileName, GetFileImageIndexFromMap(fullpath, fd.dwFileAttributes));
					SetItemData(nItem, dwItemData);
					if (bIsDir == FALSE)
					{
						SetItemText(nItem, COL_SEARCH_FOLDER, strFolder);
					}
					else
					{
						SetItemText(nItem, COL_SEARCH_FOLDER, Get_Folder(strFolder));
					}
					SetItemText(nItem, COL_SEARCH_DATE, strDate);
					SetItemText(nItem, COL_SEARCH_SIZE, strSize);
					SetItemText(nItem, COL_SEARCH_TYPE, GetPathTypeFromMap(fullpath, bIsDir, m_bUseFileType));
					SetItemText(nItem, COL_SEARCH_MEMO, GetPathMemo(fullpath, dwItemData, m_bCheckOpen));
				}
			}
		}
		b = FindNextFileW(hFind, &fd);
	}
	FindClose(hFind);
	if (m_bBreak == FALSE)
	{
		for (int i = 0; i < aSubFolders.GetSize(); i++)
		{
			FileSearch_Do(aSubFolders[i]);
		}
	}
}


BOOL CSearchListCtrl::IsMatch_State(WIN32_FIND_DATA& fd, CString& fullpath)
{
	BOOL bIsLocked = TRUE, bIsHidden = TRUE, bIsReadOnly = TRUE, bIsEncrypted = TRUE;
	BOOL bIsDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
	if (bIsDir == FALSE && m_SC.bLocked == TRUE) // ��� �������� �˻�
	{
		HANDLE hFile = CreateFile(fullpath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
		{
			bIsLocked = TRUE;
		}
		else
		{
			bIsLocked = FALSE;
			CloseHandle(hFile);
		}
	}
	if (m_SC.bHidden) bIsHidden = (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? TRUE : FALSE;
	if (m_SC.bReadOnly) bIsReadOnly = (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? TRUE : FALSE;
	if (m_SC.bEncrypted) bIsEncrypted = (fd.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) ? TRUE : FALSE;
	return bIsLocked && bIsHidden && bIsReadOnly && bIsEncrypted;
}


BOOL CSearchListCtrl::IsMatch_Name(WIN32_FIND_DATA& fd)
{
	int nCount = (int)m_aNameMatch.GetCount();
	if (m_aNameMatch.GetCount() <= 0) return TRUE;
	CString strName = Get_Name(fd.cFileName, FALSE);
	strName.MakeLower();
	BOOL bMatch = TRUE;
	for (int i = 0; i < nCount; i++)
	{
		if (strName.Find(m_aNameMatch[i]) == -1)
		{	//���Ե��� ����
			if (m_SC.bNameAnd == TRUE) return FALSE; // AND ������ ��� �ٷ� ����
			else bMatch = FALSE; // OR �̰� ��ã������ ���� Ű����� ��õ�
		}
		else
		{	//���Ե�
			if (m_SC.bNameAnd == FALSE) return TRUE; // OR ������ ��� �ٷ� ����
			else bMatch = TRUE; // AND �̰� ã������ ���� Ű����� �߰� Ȯ��
		}
	}
	return bMatch; // AND �϶��� TRUE, OR �϶��� FALSE
}


BOOL CSearchListCtrl::IsMatch_Ext(WIN32_FIND_DATA& fd)
{
	int nCount = (int)m_aExtMatch.GetCount();
	if (m_aExtMatch.GetCount() <= 0) return TRUE;
	CString strExt = Get_Ext(fd.cFileName, FALSE, FALSE);
	for (int i = 0; i < nCount; i++)
	{
		if (strExt.CompareNoCase(m_aExtMatch[i]) == 0) return TRUE;
	}
	return FALSE;
}


BOOL CSearchListCtrl::IsMatch_Time(WIN32_FIND_DATA& fd)
{
	COleDateTime tTemp = COleDateTime(fd.ftLastWriteTime);
	if (m_SC.bDateTimeFrom == TRUE && m_dtFrom > tTemp) return FALSE;
	if (m_SC.bDateTimeUntil == TRUE && m_dtUntil < tTemp) return FALSE;
	return TRUE;
}


BOOL CSearchListCtrl::IsMatch_Size(WIN32_FIND_DATA& fd) 
{
	// ���� �˻� : ���� ũ��
	BOOL bMatch = TRUE;
	ULARGE_INTEGER filesize;
	filesize.HighPart = fd.nFileSizeHigh;
	filesize.LowPart = fd.nFileSizeLow;
	if (m_bSizeMax && bMatch)
	{
		if (filesize.QuadPart > m_sizeMax) bMatch = FALSE;
	}
	if (m_bSizeMin && bMatch)
	{
		if (filesize.QuadPart < m_sizeMin) bMatch = FALSE;
	}
	return bMatch;
}

void CSearchListCtrl::Sort(int iColumn, BOOL bAscending, BOOL bAdd)
{
	m_iSortedColumn = iColumn;
	m_bAscending = bAscending;
	SortItemsEx(CompareProc, (LPARAM)this);
}


void CSearchListCtrl::OnHdnItemclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	Default();
	SetSortColumn(m_iSortedColumn, m_bAscending);
	//m_bAsc = m_bAscending;
	//m_nSortCol = m_iSortedColumn;
	//GetParent()->PostMessageW(WM_COMMAND, CMD_UpdateSortInfo, (DWORD_PTR)this);
	*pResult = 0;
}


BOOL CSearchListCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			BOOL bMulti = TRUE;
			if ((GetKeyState(VK_SHIFT) & 0xFF00) != 0) bMulti = FALSE;
			OpenSelectedItem(bMulti);
			return TRUE;
		}
	}
	else if (pMsg->message == WM_KEYUP)
	{
		if ((GetKeyState(VK_CONTROL) & 0xFF00) != 0)
		{
			if (pMsg->wParam == _T('C'))
			{
				ClipBoardExport(FALSE); //Copy
				return TRUE;
			}
			else if (pMsg->wParam == _T('X'))
			{
				ClipBoardExport(TRUE); //Cut
				return TRUE;
			}
			else if (pMsg->wParam == _T('A'))
			{
				SelectAllItems();
				return TRUE;
			}
		}
		if (pMsg->wParam == VK_DELETE)
		{
			if ((GetKeyState(VK_SHIFT) & 0xFF00) != 0) DeleteSelected();
			else RemoveSelected();
		}
	}
	return CFileListCtrl_Base::PreTranslateMessage(pMsg);
}

CString CSearchListCtrl::GetItemFullPath(int nItem)
{
	if (nItem < 0 || nItem >= GetItemCount()) 	return _T("");
	CString strPath;
	strPath = PathBackSlash(GetItemText(nItem, COL_SEARCH_FOLDER)) + GetItemText(nItem, COL_SEARCH_NAME);
	return strPath;
}

void CSearchListCtrl::SelectAllItems()
{
	int nCount = GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
}


BOOL CSearchListCtrl::GetDataForClipBoard(int nState, HGLOBAL& hgDrop, CString& strData)
{
	ListItemArray& aCut = APP()->m_aCutItem;
	CStringList aFiles;
	CString strPath;
	strData.Empty();
	size_t uBuffSize = 0;
	APP()->ClearPreviousCutItems();
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return FALSE;
	while (nItem != -1)
	{
		strPath = GetItemFullPath(nItem);
		aFiles.AddTail(strPath);
		strData += strPath + _T("\r\n"); //�ؽ�Ʈ�� ������ ���
		SetItemState(nItem, nState, LVIS_CUT);
		aCut.Add(CListItem(this, nItem));
		nItem = GetNextItem(nItem, LVNI_SELECTED);
		uBuffSize += strPath.GetLength() + 1;
	}
	uBuffSize = sizeof(DROPFILES) + sizeof(TCHAR) * (uBuffSize + 1);
	hgDrop = ::GlobalAlloc(GHND | GMEM_SHARE, uBuffSize);
	if (hgDrop != NULL)
	{
		DROPFILES* pDrop = (DROPFILES*)GlobalLock(hgDrop);;
		if (NULL == pDrop)
		{
			GlobalFree(hgDrop);
			return FALSE;
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
	return TRUE;
}

void CSearchListCtrl::ClipBoardExport(BOOL bMove)
{
	HGLOBAL hgDrop = NULL;
	CString strText; 
	if (GetDataForClipBoard((bMove ? LVIS_CUT : 0), hgDrop, strText) == FALSE) return;
	if (hgDrop == NULL) return;
	if (OpenClipboard())
	{
		EmptyClipboard();

		SetClipboardData(CF_HDROP, hgDrop);

		HGLOBAL hEffect = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(DWORD));
		if (hEffect != NULL)
		{
			DROPEFFECT effect = bMove ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
			DWORD* pdw1 = (DWORD*)GlobalLock(hEffect);
			if (pdw1 != NULL) (*pdw1) = effect;
			GlobalUnlock(hEffect);
			SetClipboardData(RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT), hEffect);
		}
		HGLOBAL hClipboardData = GlobalAlloc(GMEM_MOVEABLE, (strText.GetLength() + 1) * sizeof(TCHAR));
		if (hClipboardData) 
		{
			LPTSTR pchData = static_cast<LPTSTR>(GlobalLock(hClipboardData));
			if (pchData) 
			{
				_tcscpy_s(pchData, strText.GetLength() + 1, strText);
				GlobalUnlock(hClipboardData);
				SetClipboardData(CF_UNICODETEXT, hClipboardData);
			}
		}
		CloseClipboard();
	}
}

void CSearchListCtrl::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	OpenSelectedItem(FALSE);
	*pResult = 0;
}

void CSearchListCtrl::OpenSelectedItem(BOOL bMulti)
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	HRESULT hr = S_OK;
	IShellFolder* pisf = NULL;
	//��Ʈ(����ũž)�� IShellFolder �������̽� ������
	if (FAILED(SHGetDesktopFolder(&pisf))) return;
	while (nItem != -1)
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
		if (bMulti == TRUE) nItem = GetNextItem(nItem, LVNI_SELECTED);
		else nItem = -1;
	}
	pisf->Release();
}

void CSearchListCtrl::OpenSelectedParent(BOOL bUseTab)
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	HRESULT hr = S_OK;
	CString strFolder = GetItemText(nItem, COL_SEARCH_FOLDER);
	if (bUseTab == FALSE)
	{
		ShellExecute(NULL, _T("open"), _T("explorer"), strFolder, NULL, SW_SHOWNORMAL);
	}
	else
	{
		APP()->m_strShowPath = GetItemFullPath(nItem);
		GetParent()->GetParent()->PostMessage(WM_COMMAND, IDM_SEARCH_RESULT_VIEWTAB, 0);
	}
}

void CSearchListCtrl::RemoveSelected()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	int nFirstSelected = nItem;
	//�����׸��� ��� ã�� �Ŀ� �ڿ������� �ѹ��� �����
	CUIntArray aIndex;
	while (nItem != -1)
	{
		aIndex.Add(nItem);
		nItem = GetNextItem(nItem, LVNI_SELECTED);
	}
	int nCount = (int)aIndex.GetCount();
	for (int i = (nCount - 1); i >= 0; i--)	DeleteItem(aIndex[i]);
	//��� �׸��� �� ������ ��� ��ũ���� ���� Ƣ�� �ʵ��� �ش� ��ġ�� �ٽ� �������ش�.
	nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1)
	{
		if (nFirstSelected >= GetItemCount()) nFirstSelected = GetItemCount() - 1;
		SetItemState(nFirstSelected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	//���� ǥ�� ������Ʈ
	m_strMsg.Format(IDSTR(IDS_SEARCH_MSG_REMOVE), nCount, GetItemCount());
	GetParent()->PostMessage(WM_COMMAND, IDM_SEARCH_MSG, 1); //lParam = 1 �̸� ����
}

void CSearchListCtrl::DeleteSelected(BOOL bRecycle)
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	int nFirstSelected = nItem;
	CStringArray aPath;
	CUIntArray aIndex;
	while (nItem != -1)
	{
		aPath.Add(GetItemFullPath(nItem));
		aIndex.Add(nItem);
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
	//UI���� �����ϱ�
	//this->SetRedraw(FALSE);
	int nCount = (int)aIndex.GetCount();
	for (int i = (nCount - 1); i >= 0; i--)
	{
		if (PathFileExists(aPath[i]) == FALSE)	DeleteItem(aIndex[i]);
	}
	//this->SetRedraw(TRUE);
	//��� �׸��� �� ������ ��� ��ũ���� ���� Ƣ�� �ʵ��� �ش� ��ġ�� �ٽ� �������ش�.
	nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1)
	{
		if (nFirstSelected >= GetItemCount()) nFirstSelected = GetItemCount() - 1;
		SetItemState(nFirstSelected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	//���� ǥ�� ������Ʈ
	m_strMsg.Format(IDSTR(IDS_SEARCH_MSG_DELETE), nCount, GetItemCount());
	GetParent()->PostMessage(WM_COMMAND, IDM_SEARCH_MSG, 1); //lParam = 1 �̸� ����
	//this->SetRedraw(TRUE);
}


