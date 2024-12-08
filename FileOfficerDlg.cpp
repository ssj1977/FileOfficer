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
#include "CDlgFileSearch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static CDlgFileSearch st_dlgSearch;
// CFileOfficerDlg 대화 상자



CFileOfficerDlg::CFileOfficerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILEOFFICER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pWndFocus = NULL;
	m_bShow2 = TRUE;
	m_nDefault_FontSize = -1;
	m_nDragMainPos = 350;
}

void CFileOfficerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFileOfficerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_CLIPBOARDUPDATE()
	ON_WM_ERASEBKGND()
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
	tvo.nFontWeight = lf.lfWeight;
	tvo.bFontItalic = lf.lfItalic;
}


BOOL CFileOfficerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	ModifyStyle(0, WS_CLIPCHILDREN);
	//Create Temporary List for Default Option Values
	m_wndDragMain.CreateDragBar(TRUE, this, 35000);
	m_wndDragMain.m_pBarPos = &(m_nDragMainPos);
	//Create Search Dialog (Modaless)
	if (APP()->m_aTabViewOption.GetSize() >= 3)	st_dlgSearch.m_tvoSearch = APP()->m_aTabViewOption.GetAt(2);
	st_dlgSearch.Create(IDD_FILE_SEARCH, this);
	st_dlgSearch.ShowWindow(SW_HIDE);

	if (APP()->m_rcMain.IsRectEmpty() == FALSE)
	{
		int nDefaultSize = 0;
		if (APP()->m_nLayoutType == LIST_LAYOUT_HORIZONTAL)
		{
			nDefaultSize = APP()->m_rcMain.Width();
		}
		else //if (APP()->m_nLayoutType == LIST_LAYOUT_VERTICAL)
		{
			nDefaultSize = APP()->m_rcMain.Height();
		}
		if (APP()->m_nLayoutSizeType == LIST_LAYOUT_SIZE_PERCENT)
		{
			nDefaultSize = (int)(nDefaultSize * (APP()->m_nLayoutSizePercent / 100.0));
		}
		else if (APP()->m_nLayoutSizeType == LIST_LAYOUT_SIZE_FIXED)
		{
			if (APP()->m_nLayoutSizeFixed > 0) nDefaultSize = APP()->m_nLayoutSizeFixed;
		}
		else if (APP()->m_nLayoutSizeType == LIST_LAYOUT_SIZE_DYNAMIC)
		{
			nDefaultSize = APP()->m_nLayoutSizeDynamic;
		}
		m_nDragMainPos = nDefaultSize;
	}

	//CFileListCtrl_Base::m_hSysImgList_SMALL = NULL;
	//CFileListCtrl_Base::m_hSysImgList_LARGE = NULL;
	//CFileListCtrl_Base::m_hSysImgList_EXTRALARGE = NULL;
	//CFileListCtrl_Base::m_hSysImgList_JUMBO = NULL;

	CFileListCtrl list;
	if (list.Create(WS_CHILD | LVS_REPORT | LVS_SHAREIMAGELISTS, CRect(0, 0, 0, 0), this, 0) == TRUE)
	{
		InitDefaultListOption(&list);
		list.DestroyWindow();
	}

	//ini가 비어있거나 잘못된 경우 대비, 빈 세개의 탭 정보 생성 (창1 + 창2 + 검색창)
	if (APP()->m_aTabViewOption.GetSize() < 3) APP()->m_aTabViewOption.SetSize(3);
	m_tv1.m_tvo = APP()->m_aTabViewOption.GetAt(0);
	m_tv2.m_tvo = APP()->m_aTabViewOption.GetAt(1);
	m_tv1.m_aTabInfo.Copy(APP()->m_aTab1);
	m_tv2.m_aTabInfo.Copy(APP()->m_aTab2);
	m_tv1.m_nCurrentTab = APP()->m_nCurrentTab1;
	m_tv2.m_nCurrentTab = APP()->m_nCurrentTab2;
	m_tv1.m_bViewShortCut = APP()->m_bViewShortCut1;
	m_tv2.m_bViewShortCut = APP()->m_bViewShortCut2;
	m_tv1.m_nDragBarPos = APP()->m_nDragBarPos1;
	m_tv2.m_nDragBarPos = APP()->m_nDragBarPos2;
	m_tv1.m_listShortCut.m_nIconType = APP()->m_nShortCutIconType1;
	m_tv2.m_listShortCut.m_nIconType = APP()->m_nShortCutIconType2;
	m_tv1.m_listShortCut.m_nViewType = APP()->m_nShortCutViewType1;
	m_tv2.m_listShortCut.m_nViewType = APP()->m_nShortCutViewType2;
	m_tv1.Create(IDD_TAB_VIEW, this);
	m_tv2.Create(IDD_TAB_VIEW, this);
	m_tv1.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	m_tv2.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	ArrangeTabLayout();
	if (APP()->m_rcMain.IsRectEmpty() == FALSE) MoveWindow(APP()->m_rcMain, TRUE);
	if (APP()->m_rcSearch.IsRectEmpty() == FALSE) st_dlgSearch.MoveWindow(APP()->m_rcSearch, TRUE);
	ArrangeCtrl();
	m_tv1.ShortCutImport(APP()->m_aShortCutPath1);
	m_tv2.ShortCutImport(APP()->m_aShortCutPath2);

	AddClipboardFormatListener(GetSafeHwnd());
	DragAcceptFiles(TRUE);
	if (APP()->m_nFocus == 1) { m_tv1.CurrentList()->SetFocus(); return FALSE; }
	else if (APP()->m_nFocus == 2) { m_tv2.CurrentList()->SetFocus(); return FALSE; }
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CFileOfficerDlg::ArrangeTabLayout()
{
	if (::IsWindow(m_wndDragMain.m_hWnd) == FALSE) return;
	if (::IsWindow(m_tv1.m_hWnd) == FALSE) return;
	if (::IsWindow(m_tv2.m_hWnd) == FALSE) return;

	if (APP()->m_nLayoutType == LIST_LAYOUT_SINGLE1)
	{
		m_wndDragMain.ShowWindow(SW_HIDE);
		m_tv1.ShowWindow(SW_SHOW);
		m_tv2.ShowWindow(SW_HIDE);
		m_tv1.SetCurrentTab(m_tv1.m_nCurrentTab);
	}
	else if (APP()->m_nLayoutType == LIST_LAYOUT_SINGLE2)
	{
		m_wndDragMain.ShowWindow(SW_HIDE);
		m_tv1.ShowWindow(SW_HIDE);
		m_tv2.ShowWindow(SW_SHOW);
		m_tv2.SetCurrentTab(m_tv2.m_nCurrentTab);
	}
	else
	{
		m_wndDragMain.ShowWindow(SW_SHOW);
		m_tv1.ShowWindow(SW_SHOW);
		m_tv2.ShowWindow(SW_SHOW);
		m_tv1.SetCurrentTab(m_tv1.m_nCurrentTab);
		m_tv2.SetCurrentTab(m_tv2.m_nCurrentTab);
	}
}

void CFileOfficerDlg::ArrangeCtrl()
{
	if (IsIconic() == TRUE) return;
	CRect rc; 
	GetClientRect(rc);
	int BW = 0; 
	int nBarSize = 6;
	if (APP()->m_nLayoutType == LIST_LAYOUT_HORIZONTAL)
	{
		CRect rcTab1 = CRect(0, 0, m_nDragMainPos - 1, rc.bottom);
		CRect rcTab2 = CRect(m_nDragMainPos + nBarSize, 0, rc.right, rc.bottom);
		m_wndDragMain.m_bVertical = FALSE;
		m_tv1.MoveWindow(rcTab1, FALSE);
		m_wndDragMain.MoveWindow(rcTab1.right + 1, rc.top, nBarSize, rc.Height(), FALSE);
		m_tv2.MoveWindow(rcTab2, FALSE);
	}
	else if (APP()->m_nLayoutType == LIST_LAYOUT_VERTICAL)
	{
		if (m_nDragMainPos < rc.top) m_nDragMainPos = rc.top + 100;
		if (m_nDragMainPos > rc.bottom) m_nDragMainPos = rc.bottom - 100;
		CRect rcTab1 = CRect(0, 0, rc.right, m_nDragMainPos - 1);
		CRect rcTab2 = CRect(0, m_nDragMainPos + nBarSize, rc.right, rc.bottom);
		m_wndDragMain.m_bVertical = TRUE;
		m_tv1.MoveWindow(rcTab1, FALSE);
		m_wndDragMain.MoveWindow(rc.left, rcTab1.bottom, rc.Width(), nBarSize, FALSE);
		m_tv2.MoveWindow(rcTab2, FALSE);
	}
	else if (APP()->m_nLayoutType == LIST_LAYOUT_SINGLE1)
	{
		m_wndDragMain.MoveWindow(0, 0, 0, 0, FALSE);
		m_tv1.MoveWindow(rc, FALSE);
		m_tv2.MoveWindow(0, 0, 0, 0, FALSE);
	}
	else if (APP()->m_nLayoutType == LIST_LAYOUT_SINGLE2)
	{
		m_wndDragMain.MoveWindow(0, 0, 0, 0, FALSE);
		m_tv1.MoveWindow(0, 0, 0, 0, FALSE);
		m_tv2.MoveWindow(rc, FALSE);
	}
	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
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
	case IDM_ARRANGECTRL:	ArrangeCtrl();		return TRUE;
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
			dlg.m_nLayoutType = APP()->m_nLayoutType;
			dlg.m_nLayoutSizeType = APP()->m_nLayoutSizeType;
			dlg.m_nLayoutSizePercent = APP()->m_nLayoutSizePercent;
			dlg.m_nLayoutSizeFixed = APP()->m_nLayoutSizeFixed;
			dlg.m_nToolBarButtonSize = APP()->m_nToolBarButtonSize;
			dlg.m_bToolBarVertical = APP()->m_bToolBarVertical;
			dlg.m_bViewShortCut1 = APP()->m_bViewShortCut1;
			dlg.m_bViewShortCut2 = APP()->m_bViewShortCut2;
			dlg.m_bUseFileIcon = APP()->m_bUseFileIcon;
			dlg.m_bUseFileType = APP()->m_bUseFileType;
			dlg.m_bCheckOpen = APP()->m_bCheckOpen;

			if (dlg.DoModal() == IDOK)
			{
				if (APP()->m_nLayoutType != dlg.m_nLayoutType)
				{
					APP()->m_nLayoutType = dlg.m_nLayoutType;
					ArrangeTabLayout();
				}
				APP()->m_nLayoutSizeType = dlg.m_nLayoutSizeType;
				APP()->m_nLayoutSizePercent = dlg.m_nLayoutSizePercent;
				APP()->m_nLayoutSizeFixed = dlg.m_nLayoutSizeFixed;
				BOOL bArrangeChild = FALSE;
				if (APP()->m_bViewShortCut1 != dlg.m_bViewShortCut1 || APP()->m_bViewShortCut2 != dlg.m_bViewShortCut2)
				{
					bArrangeChild = TRUE; 
					APP()->m_bViewShortCut1 = dlg.m_bViewShortCut1; m_tv1.m_bViewShortCut = APP()->m_bViewShortCut1;
					APP()->m_bViewShortCut2 = dlg.m_bViewShortCut2; m_tv2.m_bViewShortCut = APP()->m_bViewShortCut2;
				}
				if (APP()->m_nToolBarButtonSize != dlg.m_nToolBarButtonSize)
				{
					bArrangeChild = TRUE;
					APP()->m_nToolBarButtonSize = dlg.m_nToolBarButtonSize;
					m_tv1.ResizeToolBar(APP()->m_nToolBarButtonSize, APP()->m_nToolBarButtonSize);
					m_tv2.ResizeToolBar(APP()->m_nToolBarButtonSize, APP()->m_nToolBarButtonSize);
				}

				if (APP()->m_bToolBarVertical != dlg.m_bToolBarVertical)
				{
					bArrangeChild = TRUE;
					APP()->m_bToolBarVertical = dlg.m_bToolBarVertical;
				}
				if (bArrangeChild == TRUE)
				{
					m_tv1.ArrangeCtrl();
					m_tv2.ArrangeCtrl();
				}
				ArrangeCtrl();
				APP()->m_bUseFileIcon = dlg.m_bUseFileIcon;
				APP()->m_bUseFileType = dlg.m_bUseFileType;
				APP()->m_bCheckOpen = dlg.m_bCheckOpen;
			}
		}
		return TRUE;
		/*APP()->m_nViewMode += 1;
		if (APP()->m_nViewMode == 3) APP()->m_nViewMode = 0;
		ArrangeCtrl();*/
	case IDM_TOGGLE_SEARCHDLG:
		{
			if (::IsWindow(st_dlgSearch.GetSafeHwnd()))
			{
				if (st_dlgSearch.IsWindowVisible()) st_dlgSearch.ShowWindow(SW_HIDE);
				else st_dlgSearch.ShowWindow(SW_SHOW);
			}
		}
		return TRUE;
	case IDM_SEARCH_RESULT_VIEWTAB:
		ShowPath(APP()->m_strShowPath);
		return TRUE;
	default:
		return CDialogEx::OnCommand(wParam, lParam);
	}
	//UpdateMenu();
	return TRUE;
}

BOOL CFileOfficerDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
	}
	CWnd* pWnd = GetFocus();
	if (pWnd != NULL && ::IsWindow(pWnd->GetSafeHwnd()))
	{
		if (pWnd == &m_tv1 || pWnd->GetParent() == &m_tv1)
		{
			if (m_pWndFocus != m_tv1.CurrentList())
			{
				m_pWndFocus = m_tv1.CurrentList();
				m_tv1.SetSelected(TRUE);
				m_tv2.SetSelected(FALSE);
				//m_tv1.RedrawWindow();
				//m_tv2.RedrawWindow();
			}
		}
		else if (pWnd == &m_tv2 || pWnd->GetParent() == &m_tv2)
		{
			if (m_pWndFocus != m_tv2.CurrentList())
			{
				m_pWndFocus = m_tv2.CurrentList();
				m_tv1.SetSelected(FALSE);
				m_tv2.SetSelected(TRUE);
				//m_tv1.RedrawWindow();
				//m_tv2.RedrawWindow();
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
//	if (AfxMessageBox(IDSTR(IDS_CONFIRM_EXIT), MB_OKCANCEL) == IDCANCEL) return;

	CWnd* pWnd = GetFocus();
	if (pWnd == m_tv1.CurrentList() || pWnd == &m_tv1) APP()->m_nFocus = 1;
	else if (pWnd == m_tv2.CurrentList() || pWnd == &m_tv2) APP()->m_nFocus = 2;
	else APP()->m_nFocus = 1;

	ShowWindow(SW_SHOWNORMAL);
	GetWindowRect(APP()->m_rcMain);
	st_dlgSearch.GetWindowRect(APP()->m_rcSearch);
	m_tv1.UpdateColWidths();
	m_tv2.UpdateColWidths();
	APP()->m_nCurrentTab1 = m_tv1.m_nCurrentTab;
	APP()->m_nCurrentTab2 = m_tv2.m_nCurrentTab;
	APP()->m_aTab1.Copy(m_tv1.m_aTabInfo);
	APP()->m_aTab2.Copy(m_tv2.m_aTabInfo);
	APP()->m_aTabViewOption.SetAt(0, m_tv1.m_tvo);
	APP()->m_aTabViewOption.SetAt(1, m_tv2.m_tvo);
	APP()->m_aTabViewOption.SetAt(2, st_dlgSearch.m_tvoSearch);
	APP()->m_bViewShortCut1 = m_tv1.m_bViewShortCut;
	APP()->m_bViewShortCut2 = m_tv2.m_bViewShortCut;
	APP()->m_nShortCutIconType1 = m_tv1.m_listShortCut.m_nIconType;
	APP()->m_nShortCutIconType2 = m_tv2.m_listShortCut.m_nIconType;
	APP()->m_nShortCutViewType1 = m_tv1.m_listShortCut.m_nViewType;
	APP()->m_nShortCutViewType2 = m_tv2.m_listShortCut.m_nViewType;
	APP()->m_nDragBarPos1 = m_tv1.m_nDragBarPos;
	APP()->m_nDragBarPos2 = m_tv2.m_nDragBarPos;
	APP()->m_nLayoutSizeDynamic = m_nDragMainPos;
	m_tv1.ShortCutExport(APP()->m_aShortCutPath1);
	m_tv2.ShortCutExport(APP()->m_aShortCutPath2);
	m_tv1.Clear();
	m_tv2.Clear();
	if (::IsWindow(st_dlgSearch.GetSafeHwnd())) st_dlgSearch.DestroyWindow();
	CDialogEx::OnCancel();
}


void CFileOfficerDlg::OnOK()
{
	//CDialogEx::OnOK();
}



void CFileOfficerDlg::OnClipboardUpdate()
{
	CDialogEx::OnClipboardUpdate();
}


BOOL CFileOfficerDlg::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CFileOfficerDlg::ShowPath(CString strShow)
{
	CString strFolder = Get_Folder(strShow);
	BOOL bFound = FALSE;
	CFileListCtrl* pList = NULL;
	if (bFound == FALSE) // 첫번째 창에서 찾기
	{
		PathTabInfoArray& ta = m_tv1.m_aTabInfo;
		for (int i = 0; i < ta.GetCount(); i++)
		{
			if (strFolder.CompareNoCase(ta[i].strPath) == 0)
			{
				bFound = TRUE;
				m_tv1.SetCurrentTab(i);
				pList = (CFileListCtrl*)ta[i].pWnd;
			}
		}
	}
	if (bFound == FALSE) // 두번째 창에서 찾기
	{
		PathTabInfoArray& ta = m_tv2.m_aTabInfo;
		for (int i = 0; i < ta.GetCount(); i++)
		{
			if (strFolder.CompareNoCase(ta[i].strPath) == 0)
			{
				bFound = TRUE;
				m_tv2.SetCurrentTab(i);
				pList = (CFileListCtrl*)ta[i].pWnd;
			}
		}
	}
	if (bFound == FALSE) // 지금까지 못찾았으면 새로운 탭을 추가하기
	{
		if (APP()->m_nLayoutType == LIST_LAYOUT_SINGLE2)
		{
			m_tv2.AddFileListTabByPath(strFolder);
			pList = (CFileListCtrl*)m_tv2.CurrentList();
		}
		else
		{
			m_tv1.AddFileListTabByPath(strFolder);
			pList = (CFileListCtrl*)m_tv1.CurrentList();
		}
		bFound = TRUE;
	}

	if (bFound == TRUE) // 찾아낸 탭에서 해당 항목 선택해서 보여주기
	{
		CString strName = Get_Name(strShow, TRUE);
		pList->ClearSelected();
		for (int i = 0; i < pList->GetItemCount(); i++)
		{
			if (strName.CompareNoCase(pList->GetItemText(i, 0)) == 0)
			{
				pList->SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				pList->EnsureVisible(i, FALSE);
				break;// 다 찾았으므로 
			}
		}
	}
}