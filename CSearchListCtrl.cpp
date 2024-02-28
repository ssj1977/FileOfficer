#include "pch.h"
#include "resource.h"
#include "CSearchListCtrl.h"
#include "EtcFunctions.h"

#ifndef NUM_OF_COLUMNS
#define NUM_OF_COLUMNS 6 // �Ϲ����� ���� ��ϰ� �ٸ��� ����
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
	m_strStartFolder = L"C:"; // ó�� �˻��� ������ ��ġ
	//m_aNameMatch; // �̸� ����
	//m_aExtMatch; // Ȯ���� ����
	m_bNameAnd = FALSE; // �̸� ������ �������϶� AND�� �������� OR�� ��������
	m_bExtAnd = FALSE; // Ȯ���� ������ �������϶� AND�� �������� OR�� ��������
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
	// �̸� �� Ȯ���� �˻� Ű���� �ʱ�ȭ
	CString strNames, strExts;
	GetDlgItemText(IDC_EDIT_FILENAME, strNames);
	GetDlgItemText(IDC_EDIT_FILEEXT, strExts);
	GetStringArray(strNames, L'/', m_aNameMatch);
	GetStringArray(strExts, L'/', m_aExtMatch);
	//ã�� ����
	FileSearch_Do(m_strStartFolder);
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

	while (b)
	{
		dwItemData = fd.dwFileAttributes;
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
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
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
				if (m_bSizeMax || m_bSizeMin) bMatch = IsMatch_Size(fd);
				//���� �˻� : ���� ���� ����d
				if (m_bDateTimeFrom || m_bDateTimeUntil) bMatch = IsMatch_Time(fd);
				//���� �˻� : ���ϸ�
				if (m_aNameMatch.GetCount() > 0) bMatch = IsMatch_Name(fd);
				//���� �˻� : Ȯ����
				if (m_aExtMatch.GetCount() > 0) bMatch = IsMatch_Ext(fd);
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
	for (int i = 0; i < aSubFolders.GetSize(); i++)
	{
		FileSearch_Do(aSubFolders[i]);
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