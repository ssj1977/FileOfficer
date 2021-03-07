
// FileOfficerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "FileOfficer.h"
#include "FileOfficerDlg.h"
#include "afxdialogex.h"
#include "CFileListCtrl.h"
#include <CommonControls.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFileOfficerDlg 대화 상자



CFileOfficerDlg::CFileOfficerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILEOFFICER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFileOfficerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PATH_1, m_editPath1);
	DDX_Control(pDX, IDC_EDIT_PATH_2, m_editPath2);
	DDX_Control(pDX, IDC_TAB_PATH_1, m_tabPath1);
	DDX_Control(pDX, IDC_TAB_PATH_2, m_tabPath2);
}

BEGIN_MESSAGE_MAP(CFileOfficerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PATH_1, &CFileOfficerDlg::OnSelchangeTabPath1)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PATH_2, &CFileOfficerDlg::OnSelchangeTabPath2)
END_MESSAGE_MAP()

CString GetPathName(CString strPath);
// CFileOfficerDlg 메시지 처리기

BOOL CFileOfficerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	HRESULT hr = SHGetImageList(APP()->m_nIconType, IID_IImageList, (void**)&m_pSysImgList);
	// Init ToolBar
	m_toolMain.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_LEFT
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC | CCS_VERT);
	m_toolMain.LoadToolBar(IDR_TB_MAIN);

	m_tool1.CreateEx(this, TBSTYLE_TRANSPARENT, WS_CHILD | WS_VISIBLE | CBRS_BORDER_ANY);
	m_tool1.LoadToolBar(IDR_TB_TAB1);
	m_tool2.CreateEx(this, TBSTYLE_TRANSPARENT, WS_CHILD | WS_VISIBLE | CBRS_BORDER_ANY);
	m_tool2.LoadToolBar(IDR_TB_TAB2);
	// Init Tabs
	if (m_aTabInfo1.GetSize() == 0)
	{
		PathTabInfo tabInfo(L"", 0, TRUE);
		m_aTabInfo1.Add(tabInfo);
	}
	if (m_aTabInfo2.GetSize() == 0)
	{
		PathTabInfo tabInfo(L"", 0, TRUE);
		m_aTabInfo2.Add(tabInfo);
	}
	for (int i = 0; i < m_aTabInfo1.GetSize(); i++)
	{
		m_tabPath1.InsertItem(i, GetPathName(m_aTabInfo1[i].strPath));
	}
	SetCurrentTabItem(1, m_nCurrentTabItem1);
	for (int i = 0; i < m_aTabInfo2.GetSize(); i++)
	{
		m_tabPath2.InsertItem(i, GetPathName(m_aTabInfo2[i].strPath));
	}
	SetCurrentTabItem(2, m_nCurrentTabItem2);
	//Init Edit Browser
	m_editPath1.EnableFolderBrowseButton();
	m_editPath2.EnableFolderBrowseButton();
	//Set Default Size
	if (!m_rcMain.IsRectEmpty())
	{
		CRect rcWindow;
		CRect rcTemp;
		GetDesktopWindow()->GetClientRect(rcWindow);
		rcTemp.IntersectRect(rcWindow, m_rcMain);
		if (rcTemp.Width() < 100 || rcTemp.Height() < 100)
		{
			m_rcMain = CRect(0, 0, 400, 300);
		}
		MoveWindow(m_rcMain, TRUE);
	}
	ArrangeCtrl();
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CFileOfficerDlg::ArrangeCtrl()
{
	CRect rcBtnMain;
	m_toolMain.GetToolBarCtrl().GetItemRect(0, rcBtnMain);
	int TOOLMAIN_WIDTH = rcBtnMain.Width();
	CRect rc; 
	GetClientRect(rc);
	m_toolMain.MoveWindow(rc.right - TOOLMAIN_WIDTH, rc.top, TOOLMAIN_WIDTH, rc.Height());
	rc.DeflateRect(0, 0, TOOLMAIN_WIDTH, 0);
	int TABWIDTH = rc.Width() / 2;
	GetDlgItem(IDC_EDIT_PATH_1)->MoveWindow(rc.left, rc.top, TABWIDTH - 25, 20);
	m_tool1.MoveWindow(TABWIDTH - 25, rc.top, 20, 20);
	GetDlgItem(IDC_EDIT_PATH_2)->MoveWindow(TABWIDTH+5, rc.top, TABWIDTH - 25, 20);
	m_tool2.MoveWindow(rc.right - 20, rc.top, 20, 20);
	rc.top += 20;
	GetDlgItem(IDC_TAB_PATH_1)->MoveWindow(rc.left, rc.top, TABWIDTH-5, 20);
	GetDlgItem(IDC_TAB_PATH_2)->MoveWindow(TABWIDTH +5, rc.top, TABWIDTH - 5, 20);
	rc.top += 20;
	if (GetCurrentTabItem(1) != NULL) GetCurrentTabItem(1)->MoveWindow(rc.left, rc.top, TABWIDTH - 5, rc.Height());
	if (GetCurrentTabItem(2) != NULL) GetCurrentTabItem(2)->MoveWindow(TABWIDTH + 5, rc.top, TABWIDTH - 5, rc.Height());
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CFileOfficerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CFileOfficerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CFileOfficerDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDM_SET_PATH_1: UpdateTabItemByWnd(1, (CWnd*)lParam); break;
	case IDM_SET_PATH_2: UpdateTabItemByWnd(2, (CWnd*)lParam); break;
	default:
		return CDialogEx::OnCommand(wParam, lParam);
	}
	//UpdateMenu();
	return TRUE;
}


BOOL CFileOfficerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CFileOfficerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (::IsWindow(GetDlgItem(IDC_TAB_PATH_1)->GetSafeHwnd()) != FALSE) ArrangeCtrl();
}


void CFileOfficerDlg::OnSelchangeTabPath1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}


void CFileOfficerDlg::OnSelchangeTabPath2(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}


void CFileOfficerDlg::SetCurrentTab(int nTab)
{
	m_nCurrentTab = nTab;
}

#define IDC_LIST_FILE 50000

void CFileOfficerDlg::SetCurrentTabItem(int nTab, int nItem)
{
	if (nTab == 0) nTab = GetCurrentTab();
	CTabCtrl& tabPath = (nTab == 1) ? m_tabPath1 : m_tabPath2;
	PathTabInfoArray& aTabInfo = (nTab == 1) ? m_aTabInfo1 : m_aTabInfo2;
	//CMFCEditBrowseCtrl& editPath = (nTab == 1) ? m_editPath1 : m_editPath2;
	int& nCurrentItem = (nTab == 1) ? m_nCurrentTabItem1 : m_nCurrentTabItem2;

	PathTabInfo& pti = aTabInfo[nItem];
	CFileListCtrl* pList = (CFileListCtrl*)pti.pWnd;
	CRect rc = CRect(0,0, 40, 30); 
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
		pList->SetSortColumn(pti.iSortColumn, pti.bSortAscend);
		pList->CMD_UpdateTabCtrl = (nTab == 1) ? IDM_SET_PATH_1 : IDM_SET_PATH_2;
		if (m_pSysImgList) ListView_SetImageList(pList->GetSafeHwnd(), m_pSysImgList, LVSIL_SMALL);
		pti.pWnd = (CWnd*)pList;
		pList->DisplayFolder(pti.strPath);
		//SetTabTitle(nTab, nItem, GetPathName(pti.strPath));
	}
	CFileListCtrl* pListOld = (CFileListCtrl*)GetCurrentTabItem(nTab);
	if (pListOld !=NULL && ::IsWindow(pListOld->GetSafeHwnd())) pListOld->ShowWindow(SW_HIDE);
	pList->ShowWindow(SW_SHOW);
	nCurrentItem = nItem;
	ArrangeCtrl();
}
int CFileOfficerDlg::GetCurrentTab()
{
	return m_nCurrentTab;
}
CWnd* CFileOfficerDlg::GetCurrentTabItem(int nTab)
{
	if (nTab == 1)
	{
		if (m_aTabInfo1.GetSize() > m_nCurrentTabItem1 && m_nCurrentTabItem1 >= 0)
			return m_aTabInfo1[m_nCurrentTabItem1].pWnd;
	}
	else if (nTab == 2)
	{
		if (m_aTabInfo2.GetSize() > m_nCurrentTabItem2 && m_nCurrentTabItem2 >= 0)
			return m_aTabInfo2[m_nCurrentTabItem2].pWnd;
	}
	return NULL;
}
void CFileOfficerDlg::UpdateImageList()
{
	HRESULT hr = SHGetImageList(APP()->m_nIconType, IID_IImageList, (void**)&m_pSysImgList);
//	if (m_pSysImgList && SUCCEEDED(hr))
//	{
//		ListView_SetImageList(m_list.GetSafeHwnd(), m_pSysImgList, LVSIL_SMALL);
//	}
}

void CFileOfficerDlg::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CFileOfficerDlg::OnOK()
{
	//CDialogEx::OnOK();
}


BOOL CFileOfficerDlg::DestroyWindow()
{
	for (int i = 0; i < m_aTabInfo1.GetSize(); i++)
	{
		if (m_aTabInfo1[i].pWnd != NULL) delete m_aTabInfo1[i].pWnd;
	}
	for (int i = 0; i < m_aTabInfo2.GetSize(); i++)
	{
		if (m_aTabInfo2[i].pWnd != NULL) delete m_aTabInfo2[i].pWnd;
	}

	return CDialogEx::DestroyWindow();
}

void CFileOfficerDlg::SetTabTitle(int nTab, int nItem, CString strTitle)
{
	CTabCtrl& tabPath = (nTab == 1) ? m_tabPath1 : m_tabPath2;
	if (!strTitle.IsEmpty() && nItem < tabPath.GetItemCount())
	{
		TCITEM item;
		item.mask = TCIF_TEXT;
		item.pszText = strTitle.GetBuffer();
		tabPath.SetItem(nItem, &item);
		strTitle.ReleaseBuffer();
	}
}

void CFileOfficerDlg::UpdateTabItemByWnd(int nTab, CWnd* pWnd)
{
	if (pWnd == NULL || ::IsWindow(pWnd->GetSafeHwnd()) == FALSE) return;
	PathTabInfoArray& aTabInfo = (nTab == 1) ? m_aTabInfo1 : m_aTabInfo2;
	int nItem = -1;
	for (int i = 0; i < aTabInfo.GetSize(); i++)
	{
		if (aTabInfo[i].pWnd == pWnd)
		{
			nItem = i;
			break;
		}
	}
	if (nItem == -1) return;
	CTabCtrl& tabPath = (nTab == 1) ? m_tabPath1 : m_tabPath2;
	CMFCEditBrowseCtrl& editPath = (nTab == 1) ? m_editPath1 : m_editPath2;
	PathTabInfo& pti = aTabInfo[nItem];
	CFileListCtrl* pList = (CFileListCtrl*)pti.pWnd;
	pti.strPath = pList->m_strFolder;
	SetTabTitle(nTab, nItem, GetPathName(pti.strPath));
	editPath.SetWindowTextW(pti.strPath);
}