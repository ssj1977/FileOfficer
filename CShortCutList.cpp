#include "pch.h"
#include "FileOfficer.h"
#include "CShortCutList.h"
#include "EtcFunctions.h"

LPITEMIDLIST GetPIDLfromPath(CString strPath);

BEGIN_MESSAGE_MAP(CShortCutList, CMFCListCtrl)
	ON_WM_DROPFILES()
	ON_WM_CTLCOLOR()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, &CShortCutList::OnLvnBegindrag)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CShortCutList::OnNMDblclk)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

CShortCutList::CShortCutList()
{
	CMD_OpenFolderByShortCut = 0;
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
		if (pMsg->wParam == VK_RETURN)
		{
			OpenSelectedItem();
			return TRUE;
		}
		else if (pMsg->wParam == VK_DELETE)
		{
			int nItem = GetNextItem(-1, LVNI_SELECTED);
			if (nItem != -1) DeleteItem(nItem);
		}
	}
	return CMFCListCtrl::PreTranslateMessage(pMsg);
}

BOOL CShortCutList::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return CMFCListCtrl::OnCommand(wParam, lParam);
}

int GetFileImageIndexFromMap(CString strPath, DWORD dwAttribute);
void CShortCutList::OnDropFiles(HDROP hDropInfo)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	//TCHAR szFilePath[MY_MAX_PATH];
	TCHAR* pszFilePath = new TCHAR[MY_MAX_PATH];
	size_t bufsize = sizeof(TCHAR) * MY_MAX_PATH;
	if (bufsize > 0) ZeroMemory(pszFilePath, bufsize);
	WORD cFiles = DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	int nStart = GetItemCount();
	//CStringArray aPath;
	for (int i = 0; i < cFiles; i++)
	{
		DragQueryFile(hDropInfo, i, pszFilePath, MY_MAX_PATH);
		InsertPath(i, pszFilePath);
	}
	delete[] pszFilePath;
}

void CShortCutList::InsertPath(int nItem, CString strPath)
{
	DWORD dw = GetFileAttributes(strPath);
	if (dw != INVALID_FILE_ATTRIBUTES)
	{
		nItem = InsertItem(nItem, Get_Name(strPath), GetFileImageIndexFromMap(strPath, dw));
		SetItemText(nItem, 1, strPath);
		SetItemData(nItem, dw);
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
	OpenSelectedItem();
	*pResult = 0;
}

void CShortCutList::OpenSelectedItem()
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	IShellFolder* pisf = NULL;
	//��Ʈ(����ũž)�� IShellFolder �������̽� ������
	if (FAILED(SHGetDesktopFolder(&pisf))) return;
	if (IsDir(nItem) == FALSE) //�Ϲ� ����
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
		GetParent()->SendMessage(WM_COMMAND, CMD_OpenFolderByShortCut, (DWORD_PTR)this);
	}
	pisf->Release();
}


void CShortCutList::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	return;
}
