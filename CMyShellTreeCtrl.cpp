// CMyShellTreeCtrl.cpp: 구현 파일
//

#include "pch.h"
#include "FileOfficer.h"
#include "CMyShellTreeCtrl.h"


// CMyShellTreeCtrl

IMPLEMENT_DYNAMIC(CMyShellTreeCtrl, CMFCShellTreeCtrl)

CMyShellTreeCtrl::CMyShellTreeCtrl()
{
	m_current_item = NULL;
}

CMyShellTreeCtrl::~CMyShellTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CMyShellTreeCtrl, CMFCShellTreeCtrl)
	ON_NOTIFY_REFLECT(NM_CLICK, &CMyShellTreeCtrl::OnNMClick)
	ON_NOTIFY_REFLECT(TVN_SELCHANGING, &CMyShellTreeCtrl::OnTvnSelchanging)
END_MESSAGE_MAP()



void CMyShellTreeCtrl::OnNMClick(NMHDR* pNMHDR, LRESULT* pResult)
{
/*	HTREEITEM ti = GetSelectedItem();
	if (ti != m_current_item)
	{
		m_current_item = ti;
		GetParent()->SendMessage(WM_COMMAND, CMD_TreeSelChanged, 0);
	}*/
	*pResult = 0;
}


void CMyShellTreeCtrl::OnTvnSelchanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	if (pNMTreeView->itemNew.hItem != m_current_item)
	{
		m_current_item = pNMTreeView->itemNew.hItem;
		GetParent()->SendMessage(WM_COMMAND, CMD_TreeSelChanged,(LPARAM)m_current_item);
	}
	*pResult = 0;
}
