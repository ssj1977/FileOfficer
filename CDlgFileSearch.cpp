// CDlgFileSearch.cpp: 구현 파일
//

#include "pch.h"
#include "FileOfficer.h"
#include "afxdialogex.h"
#include "CDlgFileSearch.h"
#include "EtcFunctions.h"


// CDlgFileSearch 대화 상자

IMPLEMENT_DYNAMIC(CDlgFileSearch, CDialogEx)

CDlgFileSearch::CDlgFileSearch(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILE_SEARCH, pParent)
	, m_dateFrom(COleDateTime::GetCurrentTime())
	, m_dateUntil(COleDateTime::GetCurrentTime())
	, m_timeFrom(COleDateTime::GetCurrentTime())
	, m_timeUntil(COleDateTime::GetCurrentTime())
{
	m_nIconType = SHIL_LARGE;
}

CDlgFileSearch::~CDlgFileSearch()
{
}

void CDlgFileSearch::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FILEPATH, m_editFilePath);
	DDX_Control(pDX, IDC_LIST_SEARCH, m_listSearch);
	DDX_DateTimeCtrl(pDX, IDC_FILE_DATE_FROM, m_dateFrom);
	DDX_DateTimeCtrl(pDX, IDC_FILE_DATE_UNTIL, m_dateUntil);
	DDX_DateTimeCtrl(pDX, IDC_FILE_TIME_FROM, m_timeFrom);
	DDX_DateTimeCtrl(pDX, IDC_FILE_TIME_UNTIL, m_timeUntil);
}


BEGIN_MESSAGE_MAP(CDlgFileSearch, CDialogEx)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CHK_DATETIME_FROM, &CDlgFileSearch::OnBnClickedChkDatetimeFrom)
	ON_BN_CLICKED(IDC_CHK_DATETIME_UNTIL, &CDlgFileSearch::OnBnClickedChkDatetimeUntil)
	ON_CBN_SELCHANGE(IDC_CB_TIMERANGE, &CDlgFileSearch::OnCbnSelchangeCbTimerange)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


// CDlgFileSearch 메시지 처리기


void CDlgFileSearch::OnCancel()
{
	GetParent()->PostMessage(WM_COMMAND, IDM_TOGGLE_SEARCHDLG, 0);
	/*if (m_listSearch.GetItemCount() > 0)
	{
		if (AfxMessageBox(IDSTR(IDS_CONFIRM_CLOSE_SEARCH), MB_YESNO) == IDNO) return;
	}
	CDialogEx::OnCancel();*/
}


void CDlgFileSearch::OnOK()
{
	//SearchStart();
}


BOOL CDlgFileSearch::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_toolSearch.CreateEx(this, TBSTYLE_LIST | TBSTYLE_WRAPABLE, WS_CHILD | WS_VISIBLE | CBRS_BORDER_ANY); // TBSTYLE_TRANSPARENT
	InitToolBar();

	m_editFilePath.EnableFolderBrowseButton();
	m_listSearch.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_listSearch.SetIconType(m_nIconType);
	m_listSearch.InitColumns();

	((CDateTimeCtrl*)GetDlgItem(IDC_FILE_TIME_FROM))->SetFormat(L"HH:mm:ss"); //24h
	((CDateTimeCtrl*)GetDlgItem(IDC_FILE_TIME_UNTIL))->SetFormat(L"HH:mm:ss"); //24h

	CriteriaInit(APP()->m_defaultSC);
	ArrangeCtrl();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


BOOL CDlgFileSearch::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_F5)
		{
			OnCommand(IDM_SEARCH_BEGIN, 0);
			return TRUE;
		}
	}
	/*if (GetFocus() == this)
	{
		if (pMsg->message == WM_KEYUP && (GetKeyState(VK_CONTROL) & 0xFF00) != 0)
		{
			if (pMsg->wParam == _T('A'))
			{
				m_listSearch.SelectAllItems();
				return TRUE;
			}
		}
	}*/
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgFileSearch::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (::IsWindow(m_editFilePath.m_hWnd)) ArrangeCtrl();
}

inline int MoveKeepWidth(CWnd* pWnd, int x, int y, int h)
{
	CRect rc;
	pWnd->GetWindowRect(rc);
	pWnd->MoveWindow(x, y, rc.Width(), h);
	return rc.Width();
}

void CDlgFileSearch::ArrangeCtrl()
{
	CRect rcDlg, rcST, rcCB, rcTemp;
	GetClientRect(rcDlg);
	int cx = 0, cy = 0, tx = 0, tw = 0, tx2 = 0, sw = 5, sh=5;
	GetDlgItem(IDC_ST_FILEPATH)->GetWindowRect(rcST);
	GetDlgItem(IDC_CB_NAME)->GetWindowRect(rcCB);

	cx = sw; cy = sw;
	tx = cx + rcST.Width() + sw; 
	tw = rcDlg.Width() - tx - sw ;

	int nDH = rcCB.Height();

	//Toolbar
	DWORD btnsize = m_toolSearch.GetToolBarCtrl().GetButtonSize();
	int nHP = 0, nVP = 0; //Horizontal / Vertical
	m_toolSearch.GetToolBarCtrl().GetPadding(nHP, nVP);
	int nBtnW = LOWORD(btnsize);
	int nBtnH = HIWORD(btnsize);
	int nToolH = nBtnH + nVP;
	m_toolSearch.MoveWindow(cx, cy, rcDlg.Width() - (cx * 2), nToolH, TRUE);
	cy += nToolH + sh;

	//Conditions
	GetDlgItem(IDC_ST_FILEPATH)->MoveWindow(cx, cy, rcST.Width(), nDH);
	GetDlgItem(IDC_EDIT_FILEPATH)->MoveWindow(tx, cy, tw, nDH);
	cy += nDH + sh;

	GetDlgItem(IDC_ST_FILENAME)->MoveWindow(cx, cy, rcST.Width(), nDH);
	GetDlgItem(IDC_EDIT_FILENAME)->MoveWindow(tx, cy, tw - rcCB.Width() - 2, nDH);
	GetDlgItem(IDC_CB_NAME)->MoveWindow(tx + (tw - rcCB.Width()), cy, rcCB.Width(), nDH);
	cy += nDH + sh;;

	GetDlgItem(IDC_ST_FILEEXT)->MoveWindow(cx, cy, rcST.Width(), nDH);
	GetDlgItem(IDC_EDIT_FILEEXT)->MoveWindow(tx, cy, tw - rcCB.Width() - 2, nDH);
	GetDlgItem(IDC_ST_GUIDE)->MoveWindow(tx + (tw - rcCB.Width()), cy, rcCB.Width(), nDH);
	cy += nDH + sh;;

	GetDlgItem(IDC_ST_FILESIZE)->MoveWindow(cx, cy, rcST.Width(), nDH);
	tx2 = tx;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_EDIT_FILESIZE_MIN), tx2, cy, nDH) + sw;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_ST_SIZERANGE), tx2, cy, nDH) + sw;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_EDIT_FILESIZE_MAX), tx2, cy, nDH) + sw;
	cy += nDH + sh;;

	GetDlgItem(IDC_ST_FILEDATETIME)->MoveWindow(cx, cy, rcST.Width(), nDH);
	tx2 = tx;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_CB_TIMERANGE), tx2, cy, nDH) + sw;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_CHK_DATETIME_FROM), tx2, cy, nDH) + sw;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_FILE_DATE_FROM), tx2, cy, nDH) + sw;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_FILE_TIME_FROM), tx2, cy, nDH) + sw;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_ST_DATETIMERANGE), tx2, cy, nDH) + sw;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_CHK_DATETIME_UNTIL), tx2, cy, nDH) + sw;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_FILE_DATE_UNTIL), tx2, cy, nDH) + sw;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_FILE_TIME_UNTIL), tx2, cy, nDH) + sw;
	cy += nDH + sh;

	GetDlgItem(IDC_ST_FILESTATE)->MoveWindow(cx, cy, rcST.Width(), nDH);
	tx2 = tx;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_CHK_FILESTATE_LOCKED), tx2, cy, nDH) + sw;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_CHK_FILESTATE_HIDDEN), tx2, cy, nDH) + sw;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_CHK_FILESTATE_READONLY), tx2, cy, nDH) + sw;
	tx2 += MoveKeepWidth(GetDlgItem(IDC_CHK_FILESTATE_ENCRYPTED), tx2, cy, nDH) + sw;
	cy += nDH + sh;

	int nListHeight = rcDlg.Height() - cy - sh - nDH;
	GetDlgItem(IDC_LIST_SEARCH)->MoveWindow(cx, cy, rcDlg.Width() - (cx * 2), rcDlg.Height() - cy - sh - nDH);
	cy += nListHeight;
	GetDlgItem(IDC_EDIT_SEARCH_MSG)->MoveWindow(cx, cy, rcDlg.Width() - (cx * 2), nDH);
}


void CDlgFileSearch::SearchStart()
{
	if (CriteriaReadFromUI() == FALSE) return;
	m_listSearch.FileSearch_Begin();
}


void CDlgFileSearch::OnBnClickedChkDatetimeFrom()
{
	SearchCriteria& sc = m_listSearch.m_SC;
	sc.bDateTimeFrom = (((CButton*)GetDlgItem(IDC_CHK_DATETIME_FROM))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	GetDlgItem(IDC_FILE_DATE_FROM)->EnableWindow(sc.bDateTimeFrom);
	GetDlgItem(IDC_FILE_TIME_FROM)->EnableWindow(sc.bDateTimeFrom);
}


void CDlgFileSearch::OnBnClickedChkDatetimeUntil()
{
	SearchCriteria& sc = m_listSearch.m_SC;
	sc.bDateTimeUntil = (((CButton*)GetDlgItem(IDC_CHK_DATETIME_UNTIL))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	GetDlgItem(IDC_FILE_DATE_UNTIL)->EnableWindow(sc.bDateTimeUntil);
	GetDlgItem(IDC_FILE_TIME_UNTIL)->EnableWindow(sc.bDateTimeUntil);
}


BOOL CDlgFileSearch::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDM_SEARCH_BEGIN:
		if (m_listSearch.m_bWorking == FALSE) SearchStart();
		return TRUE;
	case IDM_SEARCH_STOP:
		if (m_listSearch.m_bWorking == TRUE) m_listSearch.m_bBreak = TRUE;
		return TRUE;
	case IDM_SEARCH_RESULT_CLEAR:		
		if (m_listSearch.GetItemCount() > 0)
		{
			if (AfxMessageBox(IDSTR(IDS_SEARCH_CONFIRM_CLEAR), MB_YESNO) == IDNO) return TRUE;
		}
		m_listSearch.DeleteAllItems();		
		return TRUE;
	case IDM_SEARCH_RESULT_EXPORT:		ResultExport();	return TRUE;
	case IDM_PLAY_ITEM:		m_listSearch.OpenSelectedItem(TRUE);	return TRUE;
	case IDM_SEARCH_RESULT_COPY: m_listSearch.ClipBoardExport(FALSE); 	return TRUE;
	case IDM_SEARCH_RESULT_CUT: m_listSearch.ClipBoardExport(TRUE);		return TRUE;
	case IDM_SEARCH_CRITERIA_EXPORT:	CriteriaImport();	return TRUE;
	case IDM_SEARCH_CRITERIA_IMPORT:	CriteriaExport();	return TRUE;
	case IDM_SEARCH_CRITERIA_CLEAR:		CriteriaClear();	return TRUE;
	case IDM_SEARCH_RESULT_SELECTALL:	m_listSearch.SelectAllItems();	return TRUE;
	case IDM_SEARCH_RESULT_OPENFOLDER: m_listSearch.OpenSelectedParent(FALSE); return TRUE;
	case IDM_SEARCH_RESULT_VIEWTAB: m_listSearch.OpenSelectedParent(TRUE); return TRUE;

	case IDM_SEARCH_MSG: SetDlgItemText(IDC_EDIT_SEARCH_MSG, m_listSearch.m_strMsg);	return TRUE;

	}

	return CDialogEx::OnCommand(wParam, lParam);
}


void CDlgFileSearch::InitToolBar()
{
	m_toolSearch.LoadToolBar(IDR_TB_SEARCH);
	UINT nStyle;
	int nCount = m_toolSearch.GetCount();
	int nTextIndex = 0;
	for (int i = 0; i < nCount; i++)
	{
		nStyle = m_toolSearch.GetButtonStyle(i);
		if (!(nStyle & TBBS_SEPARATOR))
		{
			m_toolSearch.SetButtonText(i, IDSTR(IDS_TB_SEARCH_00 + nTextIndex));
			nTextIndex++;
		}
	}
	ResizeToolBar(APP()->m_nToolBarButtonSize, APP()->m_nToolBarButtonSize);
}

void ResizeBitmap(CBitmap& bmp_src, CBitmap& bmp_dst, int dstW, int dstH);

void CDlgFileSearch::ResizeToolBar(int width, int height)
{
	if (::IsWindow(m_toolSearch.GetSafeHwnd()) == FALSE) return;
	int nCount = m_toolSearch.GetToolBarCtrl().GetButtonCount();
	CImageList imgList;
	CBitmap bm_original, bm_resized;
	bm_original.LoadBitmap(IDR_TB_SEARCH);
	int size_x = width * nCount;
	int size_y = height;
	ResizeBitmap(bm_original, bm_resized, size_x, size_y);
	imgList.Create(width, height, ILC_COLORDDB | ILC_MASK, nCount, 0);
	imgList.Add(&bm_resized, RGB(255, 0, 255));
	m_toolSearch.SendMessage(TB_SETIMAGELIST, 0, (LPARAM)imgList.m_hImageList);
	imgList.Detach();
	bm_original.Detach();
	bm_resized.Detach();
}

BOOL CDlgFileSearch::CriteriaReadFromUI()
{
	SearchCriteria& sc = m_listSearch.m_SC;
	//검색 경로
	m_editFilePath.GetWindowText(sc.strStartPath);
	// 파일 상태 조건
	sc.bLocked = (((CButton*)GetDlgItem(IDC_CHK_FILESTATE_LOCKED))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	sc.bHidden = (((CButton*)GetDlgItem(IDC_CHK_FILESTATE_HIDDEN))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	sc.bReadOnly = (((CButton*)GetDlgItem(IDC_CHK_FILESTATE_READONLY))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	sc.bEncrypted = (((CButton*)GetDlgItem(IDC_CHK_FILESTATE_ENCRYPTED))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	// 크기 조건
	GetDlgItemText(IDC_EDIT_FILESIZE_MIN, sc.strSizeMin);
	GetDlgItemText(IDC_EDIT_FILESIZE_MAX, sc.strSizeMax);
	if (sc.ValidateCriteriaSize() == FALSE)
	{
		AfxMessageBox(IDSTR(IDS_MSG_FILERANGE_ERROR));
		return FALSE;
	}
	// 날짜와 시간 조건
	sc.nDateTimeType = ((CComboBox*)GetDlgItem(IDC_CB_TIMERANGE))->GetCurSel();
	sc.bDateTimeFrom = (((CButton*)GetDlgItem(IDC_CHK_DATETIME_FROM))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	sc.bDateTimeUntil = (((CButton*)GetDlgItem(IDC_CHK_DATETIME_UNTIL))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	UpdateData(TRUE);
	COleDateTime dtFrom, dtUntil;
	dtFrom.SetDateTime(m_dateFrom.GetYear(), m_dateFrom.GetMonth(), m_dateFrom.GetDay(),
		m_timeFrom.GetHour(), m_timeFrom.GetMinute(), m_dateFrom.GetSecond());
	dtUntil.SetDateTime(m_dateUntil.GetYear(), m_dateUntil.GetMonth(), m_dateUntil.GetDay(),
		m_timeUntil.GetHour(), m_timeUntil.GetMinute(), m_timeUntil.GetSecond());
	sc.strDateTimeFrom = dtFrom.Format(_T("%Y-%m-%d %H:%M:%S"));
	sc.strDateTimeUntil = dtUntil.Format(_T("%Y-%m-%d %H:%M:%S"));
	if (sc.ValidateCriteriaDateTime() == FALSE)
	{
		AfxMessageBox(IDSTR(IDS_MSG_TIMERANGE_ERROR));
		sc.strDateTimeFrom.Empty();
		sc.strDateTimeUntil.Empty();
		return FALSE;
	}
	//이름, 확장자 조건
	GetDlgItemText(IDC_EDIT_FILENAME, sc.strName);
	GetDlgItemText(IDC_EDIT_FILEEXT, sc.strExt);
	sc.bNameAnd = (((CComboBox*)GetDlgItem(IDC_CB_NAME))->GetCurSel() == 0) ? FALSE : TRUE;
	//기본값에 복사 
	APP()->m_defaultSC = sc;
	return TRUE;
}

void CDlgFileSearch::CriteriaClear()
{
	SearchCriteria& sc = APP()->m_defaultSC;
	sc.Empty();
	CriteriaInit(sc);
}

void CDlgFileSearch::CriteriaExport()
{
	if (CriteriaReadFromUI() == FALSE) return;
}

void CDlgFileSearch::CriteriaImport()
{
	
}

void CDlgFileSearch::CriteriaInit(SearchCriteria& sc)
{
	m_editFilePath.SetWindowTextW(sc.strStartPath);
	SetDlgItemText(IDC_EDIT_FILENAME, sc.strName);
	((CComboBox*)GetDlgItem(IDC_CB_NAME))->SetCurSel(sc.bNameAnd);
	SetDlgItemText(IDC_EDIT_FILEEXT, sc.strExt);
	SetDlgItemText(IDC_EDIT_FILESIZE_MIN, sc.strSizeMin);
	SetDlgItemText(IDC_EDIT_FILESIZE_MAX, sc.strSizeMax);
	((CComboBox*)GetDlgItem(IDC_CB_TIMERANGE))->SetCurSel(sc.nDateTimeType);
	CriteriaInitDateTime(sc);
	((CButton*)GetDlgItem(IDC_CHK_FILESTATE_LOCKED))->SetCheck(sc.bLocked ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHK_FILESTATE_HIDDEN))->SetCheck(sc.bHidden ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHK_FILESTATE_READONLY))->SetCheck(sc.bReadOnly ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHK_FILESTATE_ENCRYPTED))->SetCheck(sc.bEncrypted ? BST_CHECKED : BST_UNCHECKED);
}

void CDlgFileSearch::ResultExport()
{

}

void CDlgFileSearch::CriteriaInitDateTime(SearchCriteria& sc)
{
	COleDateTime dtFrom; //= COleDateTime::GetCurrentTime();
	COleDateTime dtUntil; //= COleDateTime::GetCurrentTime();
	if (dtFrom.ParseDateTime(sc.strDateTimeFrom) == FALSE)
	{
		dtFrom = COleDateTime::GetCurrentTime();
		dtFrom.SetDateTime(dtFrom.GetYear(), dtFrom.GetMonth(), dtFrom.GetDay(), 0, 0, 0);
	}
	if (dtUntil.ParseDateTime(sc.strDateTimeUntil) == FALSE)
	{
		dtUntil = COleDateTime::GetCurrentTime();
		dtUntil.SetDateTime(dtUntil.GetYear(), dtUntil.GetMonth(), dtUntil.GetDay(), 23, 59, 59);
	}

	m_dateFrom.SetDate(dtFrom.GetYear(), dtFrom.GetMonth(), dtFrom.GetDay());
	m_dateUntil.SetDate(dtUntil.GetYear(), dtUntil.GetMonth(), dtUntil.GetDay());
	m_timeFrom.SetTime(dtFrom.GetHour(), dtFrom.GetMinute(), dtFrom.GetSecond());
	m_timeUntil.SetTime(dtUntil.GetHour(), dtUntil.GetMinute(), dtUntil.GetSecond());
	UpdateData(FALSE);

	((CButton*)GetDlgItem(IDC_CHK_DATETIME_FROM))->SetCheck(sc.bDateTimeFrom ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHK_DATETIME_UNTIL))->SetCheck(sc.bDateTimeUntil ? BST_CHECKED : BST_UNCHECKED);
	if (sc.nDateTimeType == 1) // 사용자 지정 기간인 경우
	{
		GetDlgItem(IDC_CHK_DATETIME_FROM)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHK_DATETIME_UNTIL)->EnableWindow(TRUE);
		GetDlgItem(IDC_FILE_DATE_FROM)->EnableWindow(sc.bDateTimeFrom);
		GetDlgItem(IDC_FILE_TIME_FROM)->EnableWindow(sc.bDateTimeFrom);
		GetDlgItem(IDC_FILE_DATE_UNTIL)->EnableWindow(sc.bDateTimeUntil);
		GetDlgItem(IDC_FILE_TIME_UNTIL)->EnableWindow(sc.bDateTimeUntil);
	}
	else
	{
		GetDlgItem(IDC_CHK_DATETIME_FROM)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHK_DATETIME_UNTIL)->EnableWindow(FALSE);
		GetDlgItem(IDC_FILE_DATE_FROM)->EnableWindow(FALSE);
		GetDlgItem(IDC_FILE_TIME_FROM)->EnableWindow(FALSE);
		GetDlgItem(IDC_FILE_DATE_UNTIL)->EnableWindow(FALSE);
		GetDlgItem(IDC_FILE_TIME_UNTIL)->EnableWindow(FALSE);
	}
}


void CDlgFileSearch::OnCbnSelchangeCbTimerange()
{
	SearchCriteria& sc = m_listSearch.m_SC;
	int nType = ((CComboBox*)GetDlgItem(IDC_CB_TIMERANGE))->GetCurSel();
	sc.nDateTimeType = nType;
	if (sc.ValidateCriteriaDateTime() == FALSE) return;
	CriteriaInitDateTime(sc);
}

void CDlgFileSearch::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_MENU_SEARCH);
	CMenu* pMenu = menu.GetSubMenu(0);
	pMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
}