#include "pch.h"
#include "resource.h"
#include "FileOfficer.h"
#include "CSearchListCtrl.h"
#include "EtcFunctions.h"

//#ifndef NUM_OF_COLUMNS
#define NUM_OF_COLUMNS 6 // 일반적인 파일 목록과 다름에 주의
#define COL_SEARCH_NAME 0
#define COL_SEARCH_FOLDER 1
#define COL_SEARCH_DATE 2
#define COL_SEARCH_SIZE 3
#define COL_SEARCH_TYPE 4
#define COL_SEARCH_MEMO 5
//#endif

IMPLEMENT_DYNAMIC(CSearchListCtrl, CFileListCtrl_Base)
BEGIN_MESSAGE_MAP(CSearchListCtrl, CFileListCtrl_Base)
	//ON_NOTIFY(HDN_ITEMCLICKA, 0, &CSearchListCtrl::OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, &CSearchListCtrl::OnHdnItemclick)
END_MESSAGE_MAP()

CSearchListCtrl::CSearchListCtrl()
{
	//m_aNameMatch; // 이름 조건
	//m_aExtMatch; // 확장자 조건
	//m_dtFrom; // 일시 조건 (시작시점)
	//m_dtUntil; // 일시 조건 (종료시점)
	m_bSizeMin = FALSE; // 크기 조건 (최소)
	m_bSizeMax = FALSE; // 크기 조건 (최대)
	m_sizeMin = 0;
	m_sizeMax = 0;
	
	m_bUseFileType = TRUE;
	m_bCheckOpen = TRUE;
	//동작 관련 플래그들
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
	//여러개의 문자열 조건에 대한 토큰 만들기
	GetStringArray(m_SC.strName, L'/', m_aNameMatch);
	GetStringArray(m_SC.strExt, L'/', m_aExtMatch);
	//날짜+시간 조건 파싱
	m_dtFrom.ParseDateTime(m_SC.strDateTimeFrom);
	m_dtUntil.ParseDateTime(m_SC.strDateTimeUntil);
	//크기 조건 설정
	ULONGLONG sizeMin = Str2Size(m_SC.strSizeMin);
	ULONGLONG sizeMax = Str2Size(m_SC.strSizeMax);

	if (strMin.IsEmpty() == FALSE && strMax.IsEmpty() == FALSE && sizeMin > sizeMax)
	{
		AfxMessageBox(IDSTR(IDS_MSG_FILERANGE_ERROR));
		return FALSE;
	}
	else
	{
		if (strMin.IsEmpty() == FALSE)
		{
			if (strMin == L"0" && sizeMin == 0) m_listSearch.m_bSizeMin = TRUE;
			if (sizeMin > 0) m_listSearch.m_bSizeMin = TRUE;
			if (m_listSearch.m_bSizeMin) m_listSearch.m_sizeMin = sizeMin;
		}
		if (strMax.IsEmpty() == FALSE)
		{
			if (strMax == L"0" && sizeMax == 0) m_listSearch.m_bSizeMax = TRUE;
			if (sizeMax > 0) m_listSearch.m_bSizeMax = TRUE;
			if (m_listSearch.m_bSizeMax) m_listSearch.m_sizeMax = sizeMax;
		}
		sc.strSizeMin = strMin;
		sc.strSizeMax = strMax;
	}

	//찾기 시작
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
		pList->m_strMsg.Format(_T("검색 완료 : %d개 찾음"), pList->GetItemCount()); //리소스 처리 필요
	}
	else
	{
		pList->m_strMsg.Format(_T("검색 중단 : %d개 찾음"), pList->GetItemCount()); //리소스 처리 필요
	}
	pList->GetParent()->PostMessage(WM_COMMAND, IDM_SEARCH_MSG, 0);

	return 0;
}

void CSearchListCtrl::FileSearch_Do(CString strFolder)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	
	//먼저 해당 폴더가 존재하는 폴더인지 확인
	hFind = FindFirstFileExW(strFolder, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	if (hFind == INVALID_HANDLE_VALUE) return;
	FindClose(hFind); 
	hFind = NULL;
	if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) return;
	
	//존재하는 폴더라면 '*'를 붙여서 탐색 진행
	CString strFind = PathBackSlash(strFolder) + L'*';
	hFind = FindFirstFileExW(strFind, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	int nCount = 0;
	CString strSize, strDate, strType;
	DWORD dwItemData = 0;
	int nItem = -1;
	size_t nLen = 0;
	COleDateTime tTemp;
	BOOL b = TRUE, bIsDir = FALSE;
	CString fullpath;
	BOOL bIsDot = FALSE;
	ULARGE_INTEGER filesize;
	CStringArray aSubFolders; // 재귀호출용 하위폴더 저장

	//현재 보고 있는 폴더 위치를 표시해준다
	m_strMsg.Format(_T("검색중 : %s"), strFolder); //리소스 처리 필요
	GetParent()->PostMessage(WM_COMMAND, IDM_SEARCH_MSG, 0);

	while (b && !(m_bBreak))
	{
		dwItemData = fd.dwFileAttributes;
		bIsDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
		nLen = _tcsclen(fd.cFileName);
		// '.', '..' 인 경우를 식별하여 무시
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
			//파일 경로인 경우 저장해 두었다가 나중에 재귀호출
			if (bIsDir)
			{
				aSubFolders.Add(fullpath);
			}
			//일반 파일인 경우 
			else
			{
				BOOL bMatch = TRUE;
				//조건 검사 : 파일 상태
				if (m_bLocked || m_bHidden || m_bReadOnly || m_bEncrypted) bMatch = IsMatch_State(fd, fullpath);
				//조건 검사 : 파일 크기
				if (bMatch == TRUE && (m_bSizeMax || m_bSizeMin)) bMatch = IsMatch_Size(fd);
				//조건 검사 : 파일 변경 시점
				if (bMatch == TRUE && (m_bDateTimeFrom || m_bDateTimeUntil)) bMatch = IsMatch_Time(fd);
				//조건 검사 : 파일명
				if (bMatch == TRUE && m_aNameMatch.GetCount() > 0) bMatch = IsMatch_Name(fd);
				//조건 검사 : 확장자
				if (bMatch == TRUE && bIsDir == FALSE && m_aExtMatch.GetCount() > 0) bMatch = IsMatch_Ext(fd);
				// 조건이 맞는 파일만 표시
				if (bMatch == TRUE)
				{
					tTemp = COleDateTime(fd.ftLastWriteTime);
					strDate = tTemp.Format(_T("%Y-%m-%d %H:%M:%S"));
					filesize.HighPart = fd.nFileSizeHigh;
					filesize.LowPart = fd.nFileSizeLow;
					strSize = GetFileSizeString(filesize.QuadPart, 0);
					nItem = InsertItem(GetItemCount(), fd.cFileName, GetFileImageIndexFromMap(fullpath, fd.dwFileAttributes));
					SetItemData(nItem, dwItemData);
					SetItemText(nItem, COL_SEARCH_FOLDER, strFolder);
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
	if (m_bLocked) // 잠긴 파일인지 검사
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
	if (m_bHidden) bIsHidden = (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? TRUE : FALSE;
	if (m_bReadOnly) bIsReadOnly = (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? TRUE : FALSE;
	if (m_bEncrypted) bIsEncrypted = (fd.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) ? TRUE : FALSE;
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
		{	//포함되지 않음
			if (m_bNameAnd == TRUE) return FALSE; // AND 조건인 경우 바로 실패
			else bMatch = FALSE; // OR 이고 못찾았으면 다음 키워드로 재시도
		}
		else
		{	//포함됨
			if (m_bNameAnd == FALSE) return TRUE; // OR 조건인 경우 바로 성공
			else bMatch = TRUE; // AND 이고 찾았으면 다음 키워드로 추가 확인
		}
	}
	return bMatch; // AND 일때는 TRUE, OR 일때는 FALSE
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
	if (m_bDateTimeFrom == TRUE && m_dtFrom > tTemp) return FALSE;
	if (m_bDateTimeUntil == TRUE && m_dtUntil < tTemp) return FALSE;
	return TRUE;
}


BOOL CSearchListCtrl::IsMatch_Size(WIN32_FIND_DATA& fd) 
{
	// 조건 검사 : 파일 크기
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
	if (pMsg->message == WM_KEYUP && (GetKeyState(VK_CONTROL) & 0xFF00) != 0)
	{
		if (pMsg->wParam == _T('C'))
		{
			ClipBoardExport(FALSE); //Copy
		}
		else if (pMsg->wParam == _T('X'))
		{
			ClipBoardExport(TRUE); //Cut
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
		strData += strPath + _T("\r\n"); //텍스트로 복사할 경로
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