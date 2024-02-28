#include "pch.h"
#include "resource.h"
#include "CSearchListCtrl.h"
#include "EtcFunctions.h"

#ifndef NUM_OF_COLUMNS
#define NUM_OF_COLUMNS 6 // 일반적인 파일 목록과 다름에 주의
#define COL_NAME 0
#define COL_FOLDER 1
#define COL_DATE 2
#define COL_SIZE 3
#define COL_TYPE 4
#define COL_MEMO 5
#endif

IMPLEMENT_DYNAMIC(CSearchListCtrl, CFileListCtrl_Base)

CSearchListCtrl::CSearchListCtrl()
{
	m_strStartFolder = L"C:"; // 처음 검색을 시작할 위치
	//m_aNameMatch; // 이름 조건
	//m_aExtMatch; // 확장자 조건
	m_bNameAnd = FALSE; // 이름 조건이 여러개일때 AND로 적용할지 OR로 적용할지
	m_bExtAnd = FALSE; // 확장자 조건이 여러개일때 AND로 적용할지 OR로 적용할지
	//m_dtFrom; // 일시 조건 (시작시점)
	//m_dtUntil; // 일시 조건 (종료시점)
	m_bDateTimeFrom = FALSE; // 시작시점을 사용할지
	m_bDateTimeUntil = FALSE; // 종료시점을 사용할지
	m_bSizeMin = FALSE; // 크기 조건 (최소)
	m_bSizeMax = FALSE; // 크기 조건 (최대)
	m_sizeMin = 0;
	m_sizeMax = 0;
	m_bLocked = FALSE; // 잠긴 파일 여부
	m_bHidden = FALSE; // 숨겨진 파일 여부
	m_bReadOnly = FALSE; // 읽기 전용 파일 여부
	m_bEncrypted = FALSE; // 암호화 파일 여부
	
	m_bUseFileType = TRUE;
	m_bCheckOpen = TRUE;
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
	SetColTexts(string_id, col_fmt, NUM_OF_COLUMNS);

}



void CSearchListCtrl::FileSearch_Begin()
{
	DeleteAllItems();
	// 이름 및 확장자 검색 키워드 초기화
	CString strNames, strExts;
	GetDlgItemText(IDC_EDIT_FILENAME, strNames);
	GetDlgItemText(IDC_EDIT_FILEEXT, strExts);
	GetStringArray(strNames, L'/', m_aNameMatch);
	GetStringArray(strExts, L'/', m_aExtMatch);
	//찾기 시작
	FileSearch_Do(m_strStartFolder);
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

	while (b)
	{
		dwItemData = fd.dwFileAttributes;
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
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
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
				if (m_bSizeMax || m_bSizeMin) bMatch = IsMatch_Size(fd);
				//조건 검사 : 파일 변경 시점d
				if (m_bDateTimeFrom || m_bDateTimeUntil) bMatch = IsMatch_Time(fd);
				//조건 검사 : 파일명
				if (m_aNameMatch.GetCount() > 0) bMatch = IsMatch_Name(fd);
				//조건 검사 : 확장자
				if (m_aExtMatch.GetCount() > 0) bMatch = IsMatch_Ext(fd);
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
					SetItemText(nItem, COL_FOLDER, strFolder);
					SetItemText(nItem, COL_DATE, strDate);
					SetItemText(nItem, COL_SIZE, strSize);
					SetItemText(nItem, COL_TYPE, GetPathTypeFromMap(fullpath, bIsDir, m_bUseFileType));
					SetItemText(nItem, COL_MEMO, GetPathMemo(fullpath, dwItemData, m_bCheckOpen));
				}
			}
		}
		b = FindNextFileW(hFind, &fd);
	}
	FindClose(hFind);
	for (int i = 0; i < aSubFolders.GetSize(); i++)
	{
		FileSearch_Do(aSubFolders[i]);
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
	return TRUE;
}


BOOL CSearchListCtrl::IsMatch_Ext(WIN32_FIND_DATA& fd)
{
	return TRUE;
}


BOOL CSearchListCtrl::IsMatch_Time(WIN32_FIND_DATA& fd)
{
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