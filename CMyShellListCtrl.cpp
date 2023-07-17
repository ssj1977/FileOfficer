// CMyShellListCtrl.cpp: 구현 파일
//

#include "pch.h"
#include "FileOfficer.h"
#include "CMyShellListCtrl.h"


// CMyShellListCtrl

IMPLEMENT_DYNAMIC(CMyShellListCtrl, CMFCShellListCtrl)

CMyShellListCtrl::CMyShellListCtrl()
{

}

CMyShellListCtrl::~CMyShellListCtrl()
{
}


BEGIN_MESSAGE_MAP(CMyShellListCtrl, CMFCShellListCtrl)
END_MESSAGE_MAP()


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

void CMyShellListCtrl::InsertPath(int nItem, CString strPath)
{
	LVITEM lvItem;
	ZeroMemory(&lvItem, sizeof(lvItem));
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
	lvItem.pszText = strPath.GetBuffer();
	lvItem.iImage = 0; //GetShell
	lvItem.iItem = nItem;
	InsertItem(&lvItem);
	strPath.ReleaseBuffer();

}



BOOL CMyShellListCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_F5)
		{
			Refresh();
			//DisplayFolder_Start(m_strFolder);
			return TRUE;
		}
		if (pMsg->wParam == VK_RETURN)
		{
			int nItem = GetNextItem(-1, LVNI_SELECTED);
			LPAFX_SHELLITEMINFO pInfo = (LPAFX_SHELLITEMINFO)GetItemData(nItem);
			//OpenSelectedItem();
			//return TRUE;
		}
		//if (pMsg->wParam == VK_ESCAPE)
		//{
			//ClearSelected();
			//return TRUE;
		//}
		if (pMsg->wParam == VK_BACK)
		{
			DisplayParentFolder();
			///OpenParentFolder();
			return TRUE;
		}
		if (pMsg->wParam == VK_F2)
		{
			//RenameSelectedItem();
			return TRUE;
		}
		if (pMsg->wParam == VK_DELETE)
		{
			//if ((GetKeyState(VK_SHIFT) & 0xFF00) != 0) DeleteSelected(FALSE);
			//else DeleteSelected(TRUE);
		}
		if ((GetKeyState(VK_CONTROL) & 0xFF00) != 0)
		{
			//if (pMsg->wParam == _T('C')) { ClipBoardExport(FALSE); return TRUE; }
			//if (pMsg->wParam == _T('X')) { ClipBoardExport(TRUE); return TRUE; }
			//if (pMsg->wParam == _T('V')) { ClipBoardImport(); return TRUE; }
			if (pMsg->wParam == _T('A'))
			{
				//SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
				//return TRUE;
			}
		}
	}
	if (pMsg->message == WM_COMMAND)
	{
		//if (pMsg->wParam == CMD_DirWatch && m_strFolder.IsEmpty() == FALSE)
		//{
			//WatchFolder_Begin();
			//return TRUE;
		//}
	}
	if (pMsg->message == WM_XBUTTONUP)
	{
		//WORD w = HIWORD(pMsg->wParam);
		//if (w == XBUTTON2) //Back
		//{
			//BrowsePathHistory(TRUE);
		//}
		//else if (w == XBUTTON1) //Forward
		//{
			//BrowsePathHistory(FALSE);
		//}
		//return TRUE;
	}
	return CMFCShellListCtrl::PreTranslateMessage(pMsg);
}
