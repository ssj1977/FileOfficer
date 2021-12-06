﻿
// FileOfficerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "FileOfficer.h"
#include "FileOfficerDlg.h"
#include "afxdialogex.h"
#include "CFileListCtrl.h"
#include "EtcFunctions.h"
#include "CDlgCFG_Layout.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFileOfficerDlg 대화 상자



CFileOfficerDlg::CFileOfficerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILEOFFICER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//m_pWndFocus = NULL;
	m_bShow2 = TRUE;
	m_nDefault_FontSize = -1;
}

void CFileOfficerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFileOfficerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CFileOfficerDlg 메시지 처리기

void CFileOfficerDlg::InitDefaultListOption(CWnd* pWnd)
{
	CFileListCtrl* pList = (CFileListCtrl*)pWnd;
	LOGFONT lf;
	pList->GetFont()->GetLogFont(&lf);
	APP()->m_fontDefault.DeleteObject();
	APP()->m_fontDefault.CreateFontIndirect(&lf);
	TabViewOption& tvo = APP()->m_DefaultViewOption;
	tvo.nFontSize = MulDiv(-1 * lf.lfHeight, 72, GetDeviceCaps(GetDC()->GetSafeHdc(), LOGPIXELSY));
	tvo.clrText = pList->GetTextColor();
	tvo.clrBk = pList->GetBkColor();
}


BOOL CFileOfficerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	//Create Temporary List for Default Option Values
	CFileListCtrl list;
	if (list.Create(WS_CHILD | LVS_REPORT | LVS_SHAREIMAGELISTS, CRect(0, 0, 0, 0), this, 0) == TRUE)
	{
		InitDefaultListOption(&list);
		list.DestroyWindow();
	}
	//
	m_tv1.m_nViewOptionIndex = 0;
	m_tv2.m_nViewOptionIndex = 1;
	m_tv1.m_aTabInfo.Copy(APP()->m_aTab1);
	m_tv2.m_aTabInfo.Copy(APP()->m_aTab2);
	m_tv1.m_nCurrentTab = APP()->m_nCurrentTab1;
	m_tv2.m_nCurrentTab = APP()->m_nCurrentTab2;
	m_tv1.Create(IDD_TAB_VIEW, this);
	m_tv2.Create(IDD_TAB_VIEW, this);
	m_tv1.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	m_tv2.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	m_tv1.ShowWindow(SW_SHOW);
	m_tv2.ShowWindow(SW_SHOW);
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

	if (APP()->m_rcMain.IsRectEmpty() == FALSE)
	{
		CRect rcScreen;
		::GetWindowRect(::GetDesktopWindow(), &rcScreen);
		APP()->m_rcMain.NormalizeRect();
		CRect rcVisible;
		rcVisible.IntersectRect(APP()->m_rcMain, rcScreen);
		if (rcVisible.Width() > 200 && rcVisible.Height() > 100)
		{
			MoveWindow(APP()->m_rcMain, TRUE);
		}
	}
	ArrangeCtrl();
	if (APP()->m_nFocus == 1) { m_tv1.CurrentList()->SetFocus(); return FALSE; }
	else if (APP()->m_nFocus == 2) { m_tv2.CurrentList()->SetFocus(); return FALSE; }
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CFileOfficerDlg::ArrangeCtrl()
{
	CRect rc; 
	GetClientRect(rc);
	int BW = 0;
	if (APP()->m_nViewMode == 0)
	{
		m_tv1.ShowWindow(SW_SHOW);
		m_tv2.ShowWindow(SW_SHOW);
		int TABWIDTH = rc.Width() / 2;
		m_tv1.MoveWindow(rc.left, rc.top, TABWIDTH - BW, rc.Height());
		m_tv2.MoveWindow(rc.right - TABWIDTH + BW, rc.top, TABWIDTH - BW, rc.Height());
	}
	else if (APP()->m_nViewMode == 1)
	{
		m_tv1.ShowWindow(SW_SHOW);
		m_tv2.ShowWindow(SW_HIDE);
		int TABWIDTH = rc.Width();
		m_tv1.MoveWindow(rc.left, rc.top, TABWIDTH - BW, rc.Height());
		m_tv2.MoveWindow(0,0,0,0);
	}
	else if (APP()->m_nViewMode == 2)
	{
		m_tv1.ShowWindow(SW_HIDE);
		m_tv2.ShowWindow(SW_SHOW);
		int TABWIDTH = rc.Width();
		m_tv1.MoveWindow(0, 0, 0, 0);
		m_tv2.MoveWindow(rc.left, rc.top, TABWIDTH - BW, rc.Height());
	}
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
/*	case IDM_OPEN_PARENT:
	case IDM_REFRESH_LIST:
	case IDM_ADD_LIST:
	case IDM_CLOSE_LIST:
		if (m_pWndFocus != NULL && ::IsWindow(m_pWndFocus->GetSafeHwnd()))
		{
			m_pWndFocus->GetParent()->PostMessage(WM_COMMAND, wParam, lParam);
		}
		break;*/
	case IDM_CFG_LAYOUT:
		{
			CDlgCFG_Layout dlg;
			if (dlg.DoModal() == IDOK)
			{
				ArrangeCtrl();
			}
		}
		return TRUE;
		/*APP()->m_nViewMode += 1;
		if (APP()->m_nViewMode == 3) APP()->m_nViewMode = 0;
		ArrangeCtrl();*/
	default:
		return CDialogEx::OnCommand(wParam, lParam);
	}
	//UpdateMenu();
	return TRUE;
}

BOOL CFileOfficerDlg::PreTranslateMessage(MSG* pMsg)
{
/*	CWnd* pWnd = GetFocus();
	if (pWnd != NULL && ::IsWindow(pWnd->GetSafeHwnd()))
	{
		if (pWnd == &m_tv1 || pWnd->GetParent() == &m_tv1)
		{
			if (m_pWndFocus != m_tv1.CurrentList())
			{
				m_pWndFocus = m_tv1.CurrentList();
				m_tv1.SetSelected(TRUE);
				m_tv2.SetSelected(FALSE);
				m_tv1.RedrawWindow();
				m_tv2.RedrawWindow();
			}
		}
		else if (pWnd == &m_tv2 || pWnd->GetParent() == &m_tv2)
		{
			if (m_pWndFocus != m_tv2.CurrentList())
			{
				m_pWndFocus = m_tv2.CurrentList();
				m_tv1.SetSelected(FALSE);
				m_tv2.SetSelected(TRUE);
				m_tv1.RedrawWindow();
				m_tv2.RedrawWindow();
			}
		}
	}*/
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CFileOfficerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (::IsWindow(m_tv1.GetSafeHwnd()) != FALSE) ArrangeCtrl();
}


void CFileOfficerDlg::OnCancel()
{
	if (AfxMessageBox(IDSTR(IDS_CONFIRM_EXIT), MB_OKCANCEL) == IDCANCEL) return;

	CWnd* pWnd = GetFocus();
	if (pWnd == m_tv1.CurrentList() || pWnd == &m_tv1) APP()->m_nFocus = 1;
	else if (pWnd == m_tv2.CurrentList() || pWnd == &m_tv2) APP()->m_nFocus = 2;
	else APP()->m_nFocus = 1;

	ShowWindow(SW_SHOWNORMAL);
	GetWindowRect(APP()->m_rcMain);
	APP()->m_nCurrentTab1 = m_tv1.m_nCurrentTab;
	APP()->m_nCurrentTab2 = m_tv2.m_nCurrentTab;
	APP()->m_aTab1.Copy(m_tv1.m_aTabInfo);
	APP()->m_aTab2.Copy(m_tv2.m_aTabInfo);
	if (m_tv1.BreakThreads() == TRUE)
	{
		Sleep(500);
	}
	if (m_tv2.BreakThreads() == TRUE)
	{
		Sleep(500);
	}
	m_tv1.Clear();
	m_tv2.Clear();
	CDialogEx::OnCancel();
}


void CFileOfficerDlg::OnOK()
{
	//CDialogEx::OnOK();
}

