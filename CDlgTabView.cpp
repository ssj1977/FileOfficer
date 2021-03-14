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

CString GetPathName(CString strPath);
// CDlgTabView 대화 상자

IMPLEMENT_DYNAMIC(CDlgTabView, CDialogEx)

CDlgTabView::CDlgTabView(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TAB_VIEW, pParent)
{
	m_nCurrentTab = 0;
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
END_MESSAGE_MAP()


// CDlgTabView 메시지 처리기


void CDlgTabView::Clear()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL) delete m_aTabInfo[i].pWnd;
	}
}


BOOL CDlgTabView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDM_OPEN_PARENT: ((CFileListCtrl*)CurrentList())->OpenParentFolder(); break;
	case IDM_UPDATE_TAB: UpdateTabByWnd((CWnd*)lParam); break;
	case IDM_UPDATE_SORTINFO: UpdateSortInfo((CWnd*)lParam); break;
	case IDM_UPDATE_BAR: SetDlgItemText(IDC_ST_BAR, ((CFileListCtrl*)lParam)->m_strBarMsg); break;
	case IDM_SET_PATH: UpdateTabByPathEdit(); break;
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
	m_tool.CreateEx(this, TBSTYLE_TRANSPARENT, WS_CHILD | WS_VISIBLE | CBRS_BORDER_ANY);
	m_tool.LoadToolBar(IDR_TB_TAB);
	m_editPath.EnableFolderBrowseButton();
	// Init Tabs
	if (m_aTabInfo.GetSize() == 0)
	{
		PathTabInfo tabInfo(L"", 0, TRUE);
		m_aTabInfo.Add(tabInfo);
	}
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		m_tabPath.InsertItem(i, GetPathName(m_aTabInfo[i].strPath));
	}
	SetCurrentTab(m_nCurrentTab);
	CFileListCtrl* pList = (CFileListCtrl*)CurrentList();
	if (pList != NULL && ::IsWindow(pList->GetSafeHwnd()))
	{
		LOGFONT lf;
		pList->GetFont()->GetLogFont(&lf);
		if (APP()->m_nDefault_FontSize == -1)
		{
			APP()->m_nDefault_FontSize = MulDiv(-1 * lf.lfHeight, 72, GetDeviceCaps(GetDC()->GetSafeHdc(), LOGPIXELSY));
			APP()->m_clrDefault_Bk = pList->GetBkColor();
			APP()->m_clrDefault_Text = pList->GetTextColor();
		}
		m_lfHeight = abs(lf.lfHeight);
	}
	ArrangeCtrl();
	return TRUE;
}


void CDlgTabView::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (::IsWindow(m_tabPath.GetSafeHwnd())) ArrangeCtrl();
}


void CDlgTabView::UpdateIconType()
{
	
}

void CDlgTabView::SetCurrentTab(int nTab)
{
	PathTabInfo& pti = m_aTabInfo[nTab];
	CFileListCtrl* pList = (CFileListCtrl*)pti.pWnd;
	CRect rc = CRect(0, 0, 40, 30);
	if (pList == NULL)
	{
		pList = new CFileListCtrl;
		if (pList->Create(WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, rc, this, IDC_LIST_FILE) == FALSE)
		{
			delete pList;
			return;
		}
		pList->SetExtendedStyle(LVS_EX_FULLROWSELECT | WS_EX_CLIENTEDGE);
		//pList->SetFont(&m_font);
		pList->CMD_UpdateTabCtrl = IDM_UPDATE_TAB;
		pList->CMD_UpdateSortInfo = IDM_UPDATE_SORTINFO;
		pList->CMD_UpdateBar = IDM_UPDATE_BAR;
		pList->m_nSortCol = pti.iSortColumn;
		pList->m_bAsc = pti.bSortAscend;
		pList->SetSortColumn(pti.iSortColumn, pti.bSortAscend);
		if (APP()->m_pSysImgList) ListView_SetImageList(pList->GetSafeHwnd(), APP()->m_pSysImgList, LVSIL_SMALL);
		pti.pWnd = (CWnd*)pList;
		//pList->DisplayFolder(pti.strPath);
		pList->DisplayFolder_Start(pti.strPath);
	}
	CFileListCtrl* pListOld = (CFileListCtrl*)CurrentList();
	if (pListOld != NULL && ::IsWindow(pListOld->GetSafeHwnd())) pListOld->ShowWindow(SW_HIDE);
	pList->ShowWindow(SW_SHOW);
	m_nCurrentTab = nTab;
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
	m_editPath.SetWindowTextW(pti.strPath);
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
	int BH = m_lfHeight * 2;
	CRect rc;
	GetClientRect(rc);
	int TW = rc.Width();
	m_editPath.MoveWindow(rc.left, rc.top, TW - BH, BH);
	m_tool.MoveWindow(TW - BH, rc.top, BH, BH);
	rc.top += BH;
	m_tabPath.MoveWindow(rc.left, rc.top, TW, BH);
	rc.top += BH;
	CWnd* pWnd = CurrentList();
	if (pWnd != NULL && ::IsWindow(pWnd->GetSafeHwnd()))
	{
		pWnd->MoveWindow(rc.left, rc.top, TW, rc.Height()- BH);
	}
	GetDlgItem(IDC_ST_BAR)->MoveWindow(rc.left, rc.bottom - BH+1, TW, BH);
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


BOOL CDlgTabView::OnEraseBkgnd(CDC* pDC)
{
	CRect rc;
	GetClientRect(rc);
	CWnd* pWnd = GetFocus();
	if (pWnd != NULL && ::IsWindow(pWnd->GetSafeHwnd()))
	{
		if (pWnd == this || pWnd->GetParent() == this)
		{
			COLORREF clrBk = RGB(255, 0, 0);
			pDC->SetBkColor(clrBk);
			pDC->FillSolidRect(rc, clrBk);
			return TRUE;
		}
	}
	return CDialogEx::OnEraseBkgnd(pDC);
}
