#include "pch.h"
#include "resource.h"
#include "CSearchListCtrl.h"
#include "EtcFunctions.h"

//#ifndef NUM_OF_COLUMNS
#define NUM_OF_COLUMNS 6 // �Ϲ����� ���� ��ϰ� �ٸ��� ����
#define COL_NAME 0
#define COL_FOLDER 1
#define COL_DATE 2
#define COL_SIZE 3
#define COL_TYPE 4
#define COL_MEMO 5
//#endif

IMPLEMENT_DYNAMIC(CSearchListCtrl, CFileListCtrl_Base)
BEGIN_MESSAGE_MAP(CSearchListCtrl, CFileListCtrl_Base)
	//ON_NOTIFY(HDN_ITEMCLICKA, 0, &CSearchListCtrl::OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, &CSearchListCtrl::OnHdnItemclick)
END_MESSAGE_MAP()

CSearchListCtrl::CSearchListCtrl()
{
	m_strStartFolder = L"C:"; // ó�� �˻��� ������ ��ġ
	//m_aNameMatch; // �̸� ����
	//m_aExtMatch; // Ȯ���� ����
	m_bNameAnd = FALSE; // �̸� ������ �������϶� AND�� �������� OR�� ��������
	//m_dtFrom; // �Ͻ� ���� (���۽���)
	//m_dtUntil; // �Ͻ� ���� (�������)
	m_bDateTimeFrom = FALSE; // ���۽����� �������
	m_bDateTimeUntil = FALSE; // ��������� �������
	m_bSizeMin = FALSE; // ũ�� ���� (�ּ�)
	m_bSizeMax = FALSE; // ũ�� ���� (�ִ�)
	m_sizeMin = 0;
	m_sizeMax = 0;
	m_bLocked = FALSE; // ��� ���� ����
	m_bHidden = FALSE; // ������ ���� ����
	m_bReadOnly = FALSE; // �б� ���� ���� ����
	m_bEncrypted = FALSE; // ��ȣȭ ���� ����
	
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
	//ã�� ����
	CWinThread* pThread = AfxBeginThread(FileSearch_RunThread, this);
	// if (m_iSortedColumn>=0 && m_iSortedColumn < GetHeaderCtrl().GetItemCount())Sort(m_iSortedColumn, m_bAscending);
}

UINT CSearchListCtrl::FileSearch_RunThread(void* lParam)
{
	CSearchListCtrl* pList = (CSearchListCtrl*)lParam;
	pList->m_bWorking = TRUE;
	pList->m_bBreak = FALSE;
	pList->FileSearch_Do(pList->m_strStartFolder);
	pList->m_bWorking = FALSE;
	if (pList->m_bBreak == FALSE)
	{
		pList->m_strMsg.Format(_T("�˻� �Ϸ� : %d�� ã��"), pList->GetItemCount()); //���ҽ� ó�� �ʿ�
	}
	else
	{
		pList->m_strMsg.Format(_T("�˻� �ߴ� : %d�� ã��"), pList->GetItemCount()); //���ҽ� ó�� �ʿ�
	}
	pList->GetParent()->PostMessage(WM_COMMAND, IDM_SEARCH_MSG, 0);

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
	BOOL b = TRUE, bIsDir = FALSE;
	CString fullpath;
	BOOL bIsDot = FALSE;
	ULARGE_INTEGER filesize;
	CStringArray aSubFolders; // ���ȣ��� �������� ����

	//���� ���� �ִ� ���� ��ġ�� ǥ�����ش�
	m_strMsg.Format(_T("�˻��� : %s"), strFolder); //���ҽ� ó�� �ʿ�
	GetParent()->PostMessage(WM_COMMAND, IDM_SEARCH_MSG, 0);

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
			//�Ϲ� ������ ��� 
			else
			{
				BOOL bMatch = TRUE;
				//���� �˻� : ���� ����
				if (m_bLocked || m_bHidden || m_bReadOnly || m_bEncrypted) bMatch = IsMatch_State(fd, fullpath);
				//���� �˻� : ���� ũ��
				if (bMatch == TRUE && (m_bSizeMax || m_bSizeMin)) bMatch = IsMatch_Size(fd);
				//���� �˻� : ���� ���� ����
				if (bMatch == TRUE && (m_bDateTimeFrom || m_bDateTimeUntil)) bMatch = IsMatch_Time(fd);
				//���� �˻� : ���ϸ�
				if (bMatch == TRUE && m_aNameMatch.GetCount() > 0) bMatch = IsMatch_Name(fd);
				//���� �˻� : Ȯ����
				if (bMatch == TRUE && bIsDir == FALSE && m_aExtMatch.GetCount() > 0) bMatch = IsMatch_Ext(fd);
				// ������ �´� ���ϸ� ǥ��
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
	if (m_bLocked) // ��� �������� �˻�
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
		{	//���Ե��� ����
			if (m_bNameAnd == TRUE) return FALSE; // AND ������ ��� �ٷ� ����
			else bMatch = FALSE; // OR �̰� ��ã������ ���� Ű����� ��õ�
		}
		else
		{	//���Ե�
			if (m_bNameAnd == FALSE) return TRUE; // OR ������ ��� �ٷ� ����
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
	if (m_bDateTimeFrom == TRUE && m_dtFrom > tTemp) return FALSE;
	if (m_bDateTimeUntil == TRUE && m_dtUntil < tTemp) return FALSE;
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
