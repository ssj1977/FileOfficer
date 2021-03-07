
// FileOfficerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "FileOfficer.h"
#include "FileOfficerDlg.h"
#include "afxdialogex.h"
#include "CFileListCtrl.h"

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
}

BEGIN_MESSAGE_MAP(CFileOfficerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CFileOfficerDlg 메시지 처리기

BOOL CFileOfficerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	//
	m_tv1.Create(IDD_TAB_VIEW, this);
	m_tv2.Create(IDD_TAB_VIEW, this);
	m_tv1.ShowWindow(SW_SHOW);
	m_tv2.ShowWindow(SW_SHOW);
	// Init ToolBar
	m_toolMain.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_LEFT
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC | CCS_VERT);
	m_toolMain.LoadToolBar(IDR_TB_MAIN);
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
	m_tv1.MoveWindow(rc.left, rc.top, TABWIDTH-5, rc.Height());
	m_tv2.MoveWindow(rc.right-TABWIDTH + 5, rc.top, TABWIDTH-5, rc.Height());
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
	if (::IsWindow(m_tv1.GetSafeHwnd()) != FALSE) ArrangeCtrl();
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


void CFileOfficerDlg::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CFileOfficerDlg::OnOK()
{
	//CDialogEx::OnOK();
}



