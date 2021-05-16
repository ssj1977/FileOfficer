// CDlgTabView.cpp: 구현 파일
//

#include "pch.h"
#include "FileOfficer.h"
#include "CDlgTabView.h"
#include "afxdialogex.h"
#include "CFileListCtrl.h"

#define IDC_LIST_FILE 50000
#define IDM_UPDATE_TAB 55000
#define IDM_UPDATE_SORTINFO 55001
#define IDM_UPDATE_BAR 55002
#define IDM_OPEN_NEWTAB 55003

CString GetPathName(CString strPath);
// CDlgTabView 대화 상자

IMPLEMENT_DYNAMIC(CDlgTabView, CDialogEx)

CDlgTabView::CDlgTabView(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TAB_VIEW, pParent)
{
	m_nCurrentTab = 0;
	m_pFont = NULL;
	m_bSelected = FALSE;
}

CDlgTabView::~CDlgTabView()
{
}

void CDlgTabView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PATH, m_editPath);
	DDX_Control(pDX, IDC_TAB_PATH, m_tabPath);
}


BEGIN_MESSAGE_MAP(CDlgTabView, CDialogEx)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PATH, &CDlgTabView::OnTcnSelchangeTabPath)
END_MESSAGE_MAP()


// CDlgTabView 메시지 처리기


void CDlgTabView::Clear()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			CFileListCtrl* pList = (CFileListCtrl*)m_aTabInfo[i].pWnd;;
			if (IsWindow(pList->GetSafeHwnd())) pList->DestroyWindow();
			delete pList;
		}
	}
}


BOOL CDlgTabView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDM_OPEN_PARENT: ((CFileListCtrl*)CurrentList())->OpenParentFolder(); break;
	case IDM_OPEN_NEWTAB: 
		{
			CFileListCtrl* pList = (CFileListCtrl*)lParam;
			CString strPath;
			int nItem = pList->GetNextItem(-1, LVNI_SELECTED);
			while (nItem != -1)
			{
				strPath = pList->GetItemFullPath(nItem);
				if (PathIsDirectory(strPath)) AddFileListTab(strPath);
				nItem = pList->GetNextItem(nItem, LVNI_SELECTED);
			}
		}
		break;
	case IDM_UPDATE_TAB: UpdateTabByWnd((CWnd*)lParam); break;
	case IDM_UPDATE_SORTINFO: UpdateSortInfo((CWnd*)lParam); break;
	case IDM_UPDATE_BAR: SetDlgItemText(IDC_ST_BAR, ((CFileListCtrl*)lParam)->m_strBarMsg); break;
	case IDM_REFRESH_LIST: UpdateTabByPathEdit(); break;
	case IDM_SET_PATH: UpdateTabByPathEdit(); break;
	case IDM_ADD_LIST: AddFileListTab(APP()->m_strPath_Default); break;
	case IDM_CLOSE_LIST: CloseFileListTab(m_nCurrentTab); break;
	case IDM_SET_FOCUS_ON: 
		//m_editPath.EnableWindow(TRUE);
		break;
	case IDM_SET_FOCUS_OFF:
		//m_editPath.EnableWindow(FALSE);
		break;
	default:
		return CDialogEx::OnCommand(wParam, lParam);
	}
	return TRUE;
}


void CDlgTabView::OnCancel() 
{	
	//CDialogEx::OnCancel();
}
void CDlgTabView::OnOK() 
{	
	//CDialogEx::OnOK();
}

BOOL CDlgTabView::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_tool.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_BORDER_ANY); // TBSTYLE_TRANSPARENT
	m_tool.LoadToolBar(IDR_TB_TAB);
	m_editPath.EnableFolderBrowseButton();
	m_tabImgList.Create(IDB_TABICON, 16, 2, RGB(255, 0, 255));
	//m_tabPath.SetExtendedStyle(TCS_EX_FLATSEPARATORS, TCS_EX_FLATSEPARATORS);
	m_tabPath.SetImageList(&m_tabImgList);
	// Init Tabs
	if (m_aTabInfo.GetSize() == 0)
	{
		PathTabInfo tabInfo(L"", 0, TRUE);
		m_aTabInfo.Add(tabInfo);
	}
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		m_tabPath.InsertItem(i, GetPathName(m_aTabInfo[i].strPath), 1);
	}
	SetCurrentTab(m_nCurrentTab);
	ArrangeCtrl();
	return TRUE;
}

void CDlgTabView::AddFileListTab(CString strPath)
{
	PathTabInfo tabInfo(strPath, APP()->m_nSortCol_Default, APP()->m_bSortAscend_Default);
	m_aTabInfo.Add(tabInfo);
	int nTab = m_tabPath.InsertItem((int)m_aTabInfo.GetSize(), GetPathName(strPath), 1);
	SetCurrentTab(nTab);
}

void CDlgTabView::CloseFileListTab(int nTab)
{
	if (m_aTabInfo.GetCount() == 1) return;
	PathTabInfo& pti = m_aTabInfo[nTab];
	CFileListCtrl* pList = (CFileListCtrl*)pti.pWnd;
	//if (pList->m_bLoading == TRUE) return;
	pList->DestroyWindow();
	delete pList;
	m_aTabInfo.RemoveAt(nTab);
	m_tabPath.DeleteItem(nTab);
	nTab--;
	if (nTab < 0) nTab = 0;
	m_tabPath.SetCurSel(0);
	SetCurrentTab(nTab);
}


void CDlgTabView::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (::IsWindow(m_tabPath.GetSafeHwnd())) ArrangeCtrl();
}


void CDlgTabView::SetCurrentTab(int nTab)
{
	TCITEM ti;
	ti.mask = TCIF_IMAGE;
	if (m_nCurrentTab != nTab)
	{
		ti.iImage = 1;
		m_tabPath.SetItem(m_nCurrentTab, &ti);
	}
	ti.iImage = 0;
	m_tabPath.SetItem(nTab, &ti);

	PathTabInfo& pti = m_aTabInfo[nTab];
	CFileListCtrl* pList = (CFileListCtrl*)pti.pWnd;
	CRect rc = CRect(0, 0, 40, 30);
	if (pList == NULL)
	{
		pList = new CFileListCtrl;
		pList->m_nIconType = APP()->m_nIconType;
		if (pList->Create(WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, rc, this, IDC_LIST_FILE) == FALSE)
		{
			delete pList;
			return;
		}
		if (m_pFont) pList->SetFont(m_pFont);
		pList->SetExtendedStyle(LVS_EX_FULLROWSELECT); //WS_EX_WINDOWEDGE , WS_EX_CLIENTEDGE
		if (APP()->m_bUseDefaultColor == FALSE)
		{
			pList->SetBkColor(APP()->m_clrBk);
			pList->SetTextColor(APP()->m_clrText);
		}
		else
		{
			pList->SetBkColor(APP()->m_clrDefault_Bk);
			pList->SetTextColor(APP()->m_clrDefault_Text);
		}
		pList->CMD_OpenNewTab = IDM_OPEN_NEWTAB;
		pList->CMD_UpdateTabCtrl = IDM_UPDATE_TAB;
		pList->CMD_UpdateSortInfo = IDM_UPDATE_SORTINFO;
		pList->CMD_UpdateBar = IDM_UPDATE_BAR;
		pList->m_nSortCol = pti.iSortColumn;
		pList->m_bAsc = pti.bSortAscend;
		pList->SetSortColumn(pti.iSortColumn, pti.bSortAscend);
		if (APP()->m_pSysImgList) ListView_SetImageList(pList->GetSafeHwnd(), APP()->m_pSysImgList, LVSIL_SMALL);
		pti.pWnd = (CWnd*)pList;
		pList->DisplayFolder_Start(pti.strPath);
	}
	CFileListCtrl* pListOld = (CFileListCtrl*)CurrentList();
	if (pListOld != NULL && ::IsWindow(pListOld->GetSafeHwnd())) pListOld->ShowWindow(SW_HIDE);
	pList->ShowWindow(SW_SHOW);
	m_nCurrentTab = nTab;
	m_tabPath.SetCurSel(nTab);
	pList->SetFocus();
	m_editPath.SetWindowText(pti.strPath);
	ArrangeCtrl();
}

CWnd* CDlgTabView::CurrentList()
{
	if (m_aTabInfo.GetSize() > m_nCurrentTab && m_nCurrentTab >= 0) return m_aTabInfo[m_nCurrentTab].pWnd;
	return NULL;
}

void CDlgTabView::UpdateTabByWnd(CWnd* pWnd)
{
	if (pWnd == NULL || ::IsWindow(pWnd->GetSafeHwnd()) == FALSE) return;
	int nTab = -1;
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd == pWnd)
		{
			nTab = i;
			break;
		}
	}
	if (nTab == -1) return;
	PathTabInfo& pti = m_aTabInfo[nTab];
	CFileListCtrl* pList = (CFileListCtrl*)pti.pWnd;
	pti.strPath = pList->m_strFolder;
	SetTabTitle(nTab, GetPathName(pti.strPath));
	m_editPath.SetWindowText(pti.strPath);
}

void CDlgTabView::UpdateSortInfo(CWnd* pWnd)
{
	if (pWnd == NULL || ::IsWindow(pWnd->GetSafeHwnd()) == FALSE) return;
	int nTab = -1;
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd == pWnd)
		{
			nTab = i;
			break;
		}
	}
	if (nTab == -1) return;
	PathTabInfo& pti = m_aTabInfo[nTab];
	CFileListCtrl* pList = (CFileListCtrl*)pti.pWnd;
	pti.bSortAscend = pList->GetHeaderCtrl().IsAscending();
	pti.iSortColumn = pList->GetHeaderCtrl().GetSortColumn();
}

void CDlgTabView::UpdateTabByPathEdit()
{
	CFileListCtrl* pList = (CFileListCtrl*)CurrentList();
	//pList->SetRedraw(FALSE);
	CString strPath, strName;
	GetDlgItemText(IDC_EDIT_PATH, strPath);
	pList->DisplayFolder_Start(strPath);
	strName = GetPathName(strPath);
	SetTabTitle(m_nCurrentTab, strName);
	//pList->SetRedraw(TRUE);
}

void CDlgTabView::SetTabTitle(int nTab, CString strTitle)
{
	if (!strTitle.IsEmpty() && nTab < m_tabPath.GetItemCount())
	{
		TCITEM item;
		item.mask = TCIF_TEXT;
		item.pszText = strTitle.GetBuffer();
		m_tabPath.SetItem(nTab, &item);
		strTitle.ReleaseBuffer();
	}
}

void CDlgTabView::ArrangeCtrl()
{
	CRect rc;
	GetClientRect(rc);
	rc.DeflateRect(3,3,3,3);
	int BH = APP()->m_lfHeight * 2;
	int TW = rc.Width();
	m_editPath.MoveWindow(rc.left, rc.top, TW - BH, BH);
	m_tool.GetToolBarCtrl().SetButtonSize(CSize(BH-6, BH));
	m_tool.MoveWindow(TW - BH + 6, rc.top, BH - 3, BH);
	rc.top += BH;
	rc.top += 3;
	m_tabPath.MoveWindow(rc.left, rc.top, TW, BH);
	rc.top += BH;
	CWnd* pWnd = CurrentList();
	if (pWnd != NULL && ::IsWindow(pWnd->GetSafeHwnd()))
	{
		pWnd->MoveWindow(rc.left, rc.top, TW, rc.Height()- BH);
	}
	GetDlgItem(IDC_ST_BAR)->MoveWindow(rc.left, rc.bottom - BH+2, TW, BH-2);
	GetDlgItem(IDC_ST_BAR)->RedrawWindow();
}



BOOL CDlgTabView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN && GetFocus() == &m_editPath)
		{
			UpdateTabByPathEdit();
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgTabView::SetSelected(BOOL bSelected)
{
	m_bSelected = bSelected;
	if (bSelected)
	{
		m_editPath.SetBkColor(APP()->m_clrBk);
		m_editPath.SetTextColor(APP()->m_clrText);
	}
	else
	{
		m_editPath.SetBkColor(APP()->m_clrDefault_Bk);
		m_editPath.SetTextColor(APP()->m_clrDefault_Text);
	}
	m_editPath.RedrawWindow();
}


BOOL CDlgTabView::OnEraseBkgnd(CDC* pDC)
{
	CRect rc;
	GetClientRect(rc);
/*	CWnd* pWnd = GetFocus();
	if (pWnd != NULL && ::IsWindow(pWnd->GetSafeHwnd()))
	{
		if (pWnd == this || pWnd->GetParent() == this)
		{
		}
	}*/
	if (m_bSelected == TRUE)
	{
		COLORREF clrBk = APP()->m_clrText; // RGB(130, 180, 255);
		pDC->SetBkColor(clrBk);
		pDC->FillSolidRect(rc, clrBk);
		return TRUE;
	}
	return CDialogEx::OnEraseBkgnd(pDC);
}


void CDlgTabView::OnTcnSelchangeTabPath(NMHDR* pNMHDR, LRESULT* pResult)
{
	SetCurrentTab(m_tabPath.GetCurSel());
	*pResult = 0;
}

void CDlgTabView::SetListColor(COLORREF crBk, COLORREF crText, BOOL bSetBk, BOOL bSetText)
{
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			CFileListCtrl* pList = (CFileListCtrl * )m_aTabInfo[i].pWnd;
			if (bSetBk)	pList->SetBkColor(crBk);
			if (bSetText) pList->SetTextColor(crText);
		}
	}

}

void CDlgTabView::UpdateImageList()
{
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			CFileListCtrl* pList = (CFileListCtrl*)m_aTabInfo[i].pWnd;
			ListView_SetImageList(pList->GetSafeHwnd(), APP()->m_pSysImgList, LVSIL_SMALL);
		}
	}
}

void CDlgTabView::UpdateFont(CFont* pFont)
{
	m_pFont = pFont;
	m_tabPath.SetFont(pFont);
	m_editPath.SetFont(pFont);
	GetDlgItem(IDC_ST_BAR)->SetFont(pFont);
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			CFileListCtrl* pList = (CFileListCtrl*)m_aTabInfo[i].pWnd;
			pList->SetFont(pFont);
		}
	}
	ArrangeCtrl();
}