
// FileOfficerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "FileOfficer.h"
#include "FileOfficerDlg.h"
#include "afxdialogex.h"
#include "CFileListCtrl.h"
#include "CDlgCFG_View.h"
#include "EtcFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFileOfficerDlg 대화 상자



CFileOfficerDlg::CFileOfficerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILEOFFICER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pWndFocus = NULL;
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
	UpdateFontSize();
	ArrangeCtrl();
	m_tv2.PostMessageW(WM_COMMAND, IDM_SET_FOCUS_OFF, 0);
	m_tv1.PostMessageW(WM_COMMAND, IDM_SET_FOCUS_ON, 0);
	if (APP()->m_nFocus == 1) { m_tv1.CurrentList()->SetFocus(); return FALSE; }
	else if (APP()->m_nFocus == 2) { m_tv2.CurrentList()->SetFocus(); return FALSE; }
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CFileOfficerDlg::ArrangeCtrl()
{
	CRect rcBtnMain;
	m_toolMain.GetToolBarCtrl().GetItemRect(0, rcBtnMain);
	int TOOLMAIN_WIDTH = rcBtnMain.Width();
	CRect rc; 
	GetClientRect(rc);
	int BW = 0;
	m_toolMain.MoveWindow(rc.right - TOOLMAIN_WIDTH, rc.top, TOOLMAIN_WIDTH, rc.Height());
	rc.DeflateRect(0, 0, TOOLMAIN_WIDTH, 0);

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
	case IDM_OPEN_PARENT:
	case IDM_REFRESH_LIST:
	case IDM_ADD_LIST:
	case IDM_CLOSE_LIST:
		if (m_pWndFocus != NULL && ::IsWindow(m_pWndFocus->GetSafeHwnd()))
		{
			m_pWndFocus->PostMessage(WM_COMMAND, wParam, lParam);
		}
		break;
	case IDM_TOGGLE_VIEW:
		APP()->m_nViewMode += 1;
		if (APP()->m_nViewMode == 3) APP()->m_nViewMode = 0;
		ArrangeCtrl();
		break;
	case IDM_CONFIG: ConfigViewOption(); break;

	default:
		return CDialogEx::OnCommand(wParam, lParam);
	}
	//UpdateMenu();
	return TRUE;
}

BOOL CFileOfficerDlg::PreTranslateMessage(MSG* pMsg)
{
	CWnd* pWnd = GetFocus();
	if (pWnd != NULL && ::IsWindow(pWnd->GetSafeHwnd()))
	{
		if (pWnd == &m_tv1 || pWnd->GetParent() == &m_tv1)
		{
			if (m_pWndFocus != &m_tv1)
			{
				m_pWndFocus = &m_tv1;
				m_tv1.SetSelected(TRUE);
				m_tv2.SetSelected(FALSE);
				m_tv1.RedrawWindow();
				m_tv2.RedrawWindow();
			}
		}
		else if (pWnd == &m_tv2 || pWnd->GetParent() == &m_tv2)
		{
			if (m_pWndFocus != &m_tv2)
			{
				m_pWndFocus = &m_tv2;
				m_tv1.SetSelected(FALSE);
				m_tv2.SetSelected(TRUE);
				m_tv1.RedrawWindow();
				m_tv2.RedrawWindow();
			}
		}
	}
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
	m_tv1.Clear();
	m_tv2.Clear();
	CDialogEx::OnCancel();
}


void CFileOfficerDlg::OnOK()
{
	//CDialogEx::OnOK();
}


void CFileOfficerDlg::ConfigViewOption()
{
	CDlgCFG_View dlg;
	dlg.m_clrText = APP()->m_clrText;
	dlg.m_clrBk = APP()->m_clrBk;
	dlg.m_bUseDefaultColor = APP()->m_bUseDefaultColor;
	dlg.m_nFontSize = APP()->m_nFontSize;
	dlg.m_bUseDefaultFont = APP()->m_bUseDefaultFont;
	dlg.m_nIconType = APP()->m_nIconType;
	dlg.m_bBold = APP()->m_bBold;
	BOOL bUpdateClrBk = FALSE, bUpdateClrText = FALSE;
	if (dlg.DoModal() == IDOK)
	{
		//Color
		if (APP()->m_bUseDefaultColor != dlg.m_bUseDefaultColor)
		{
			APP()->m_bUseDefaultColor = dlg.m_bUseDefaultColor;
			if (APP()->m_bUseDefaultColor == FALSE)
			{
				APP()->m_clrText = dlg.m_clrText;
				APP()->m_clrBk = dlg.m_clrBk;
				bUpdateClrBk = TRUE;
				bUpdateClrText = TRUE;
			}
		}
		if (APP()->m_clrBk != dlg.m_clrBk)
		{
			APP()->m_clrBk = dlg.m_clrBk;
			if (APP()->m_bUseDefaultColor == FALSE) bUpdateClrBk = TRUE;
		}
		if (APP()->m_clrText != dlg.m_clrText)
		{
			APP()->m_clrText = dlg.m_clrText;
			if (APP()->m_bUseDefaultColor == FALSE) bUpdateClrText = TRUE;
		}
		m_tv1.SetListColor(APP()->GetMyClrBk(), APP()->GetMyClrText(), bUpdateClrBk, bUpdateClrText);
		m_tv2.SetListColor(APP()->GetMyClrBk(), APP()->GetMyClrText(), bUpdateClrBk, bUpdateClrText);
		//Font
		if (APP()->m_nFontSize != dlg.m_nFontSize || APP()->m_bBold != dlg.m_bBold)
		{
			APP()->m_nFontSize = dlg.m_nFontSize;
			APP()->m_bBold = dlg.m_bBold;
			if (APP()->m_bUseDefaultFont == FALSE)
			{
				UpdateFontSize();
			}
		}
		if (APP()->m_bUseDefaultFont != dlg.m_bUseDefaultFont)
		{
			APP()->m_bUseDefaultFont = dlg.m_bUseDefaultFont;
			APP()->m_bBold = dlg.m_bBold;
			{
				UpdateFontSize();
			}
		}
		if (APP()->m_nIconType != dlg.m_nIconType)
		{
			APP()->LoadImageList(dlg.m_nIconType);
			m_tv1.UpdateImageList();
			m_tv2.UpdateImageList();
		}
		RedrawWindow(NULL,NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
		m_tv1.SetSelected(m_tv1.m_bSelected);
		m_tv2.SetSelected(m_tv2.m_bSelected);
		//m_toolMain.RedrawWindow();
	}
}

void CFileOfficerDlg::InitDefaultListOption(CWnd* pWnd)
{
	CFileListCtrl* pList = (CFileListCtrl*)pWnd;
	LOGFONT lf;
	pList->GetFont()->GetLogFont(&lf);
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&lf);
	m_nDefault_FontSize = MulDiv(-1 * lf.lfHeight, 72, GetDeviceCaps(GetDC()->GetSafeHdc(), LOGPIXELSY));
	APP()->m_clrDefault_Bk = pList->GetBkColor();
	APP()->m_clrDefault_Text = pList->GetTextColor();
	APP()->m_lfHeight = abs(lf.lfHeight);
}

void CFileOfficerDlg::UpdateFontSize()
{
	int nFontSize = APP()->m_nFontSize;
	if (APP()->m_bUseDefaultFont == TRUE) nFontSize = m_nDefault_FontSize;
	LOGFONT lf;
	m_font.GetLogFont(&lf);
	lf.lfHeight = -1 * MulDiv(nFontSize, GetDeviceCaps(GetDC()->GetSafeHdc(), LOGPIXELSY), 72);
	APP()->m_lfHeight = abs(lf.lfHeight);
	if (APP()->m_bBold == TRUE) lf.lfWeight = FW_BOLD;
	else lf.lfWeight = FW_NORMAL;
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&lf); //자동 소멸되지 않도록 멤버 변수 사용
	m_tv1.UpdateFont(&m_font);
	m_tv2.UpdateFont(&m_font);
}


