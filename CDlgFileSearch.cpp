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
	SearchStart();
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

	((CButton*)GetDlgItem(IDC_CHK_DATETIME_FROM))->SetCheck(m_listSearch.m_bDateTimeFrom ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHK_DATETIME_UNTIL))->SetCheck(m_listSearch.m_bDateTimeUntil ? BST_CHECKED : BST_UNCHECKED);
	GetDlgItem(IDC_FILE_DATE_FROM)->EnableWindow(m_listSearch.m_bDateTimeFrom);
	GetDlgItem(IDC_FILE_TIME_FROM)->EnableWindow(m_listSearch.m_bDateTimeFrom);
	GetDlgItem(IDC_FILE_DATE_UNTIL)->EnableWindow(m_listSearch.m_bDateTimeUntil);
	GetDlgItem(IDC_FILE_TIME_UNTIL)->EnableWindow(m_listSearch.m_bDateTimeUntil);

	CriteriaClear();
	CriteriaInit();
	ArrangeCtrl();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


BOOL CDlgFileSearch::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

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
	m_listSearch.m_bDateTimeFrom = (((CButton*)GetDlgItem(IDC_CHK_DATETIME_FROM))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	GetDlgItem(IDC_FILE_DATE_FROM)->EnableWindow(m_listSearch.m_bDateTimeFrom);
	GetDlgItem(IDC_FILE_TIME_FROM)->EnableWindow(m_listSearch.m_bDateTimeFrom);
}


void CDlgFileSearch::OnBnClickedChkDatetimeUntil()
{
	m_listSearch.m_bDateTimeUntil = (((CButton*)GetDlgItem(IDC_CHK_DATETIME_UNTIL))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	GetDlgItem(IDC_FILE_DATE_UNTIL)->EnableWindow(m_listSearch.m_bDateTimeUntil);
	GetDlgItem(IDC_FILE_TIME_UNTIL)->EnableWindow(m_listSearch.m_bDateTimeUntil);
}


BOOL CDlgFileSearch::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDM_SEARCH_BEGIN:
		if (m_listSearch.m_bWorking == FALSE) SearchStart();
		break;
	case IDM_SEARCH_STOP:
		if (m_listSearch.m_bWorking == TRUE) m_listSearch.m_bBreak = TRUE;
		break;
	case IDM_SEARCH_RESULT_CLEAR:		m_listSearch.DeleteAllItems();		break;
	case IDM_SEARCH_RESULT_EXPORT:		ResultExport();		break;
	case IDM_SEARCH_CRITERIA_EXPORT:	CriteriaImport();	break;
	case IDM_SEARCH_CRITERIA_IMPORT:	CriteriaExport();	break;
	case IDM_SEARCH_CRITERIA_CLEAR:		CriteriaClear();	break;
	case IDM_SEARCH_MSG:
		SetDlgItemText(IDC_EDIT_SEARCH_MSG, m_listSearch.m_strMsg);
		break;
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
	m_editFilePath.GetWindowText(m_listSearch.m_strStartFolder);
	m_listSearch.m_bLocked = (((CButton*)GetDlgItem(IDC_CHK_FILESTATE_LOCKED))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	m_listSearch.m_bHidden = (((CButton*)GetDlgItem(IDC_CHK_FILESTATE_HIDDEN))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	m_listSearch.m_bReadOnly = (((CButton*)GetDlgItem(IDC_CHK_FILESTATE_READONLY))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	m_listSearch.m_bEncrypted = (((CButton*)GetDlgItem(IDC_CHK_FILESTATE_ENCRYPTED))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;

	CString strMin, strMax;
	GetDlgItemText(IDC_EDIT_FILESIZE_MIN, strMin);
	GetDlgItemText(IDC_EDIT_FILESIZE_MAX, strMax);
	ULONGLONG sizeMin = Str2Size(strMin);
	ULONGLONG sizeMax = Str2Size(strMax);
	m_listSearch.m_bSizeMin = FALSE;
	m_listSearch.m_bSizeMax = FALSE;
	if (strMin.IsEmpty() == FALSE && strMax.IsEmpty() == FALSE && sizeMin > sizeMax)
	{
		AfxMessageBox(IDSTR(IDS_MSG_FILERANGE_ERROR));
		return FALSE;
	}
	else
	{
		if (strMin.IsEmpty() == FALSE)
		{
			if (strMin == L"0" && sizeMin == 0) m_listSearch.m_bSizeMin = TRUE;
			if (sizeMin > 0) m_listSearch.m_bSizeMin = TRUE;
			if (m_listSearch.m_bSizeMin) m_listSearch.m_sizeMin = sizeMin;
		}
		if (strMax.IsEmpty() == FALSE)
		{
			if (strMax == L"0" && sizeMax == 0) m_listSearch.m_bSizeMax = TRUE;
			if (sizeMax > 0) m_listSearch.m_bSizeMax = TRUE;
			if (m_listSearch.m_bSizeMax) m_listSearch.m_sizeMax = sizeMax;
		}
	}

	m_listSearch.m_bDateTimeFrom = (((CButton*)GetDlgItem(IDC_CHK_DATETIME_FROM))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	m_listSearch.m_bDateTimeUntil = (((CButton*)GetDlgItem(IDC_CHK_DATETIME_UNTIL))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	UpdateData(TRUE);
	m_listSearch.m_dtFrom.SetDateTime(m_dateFrom.GetYear(), m_dateFrom.GetMonth(), m_dateFrom.GetDay(),
		m_timeFrom.GetHour(), m_timeFrom.GetMinute(), m_dateFrom.GetSecond());
	m_listSearch.m_dtUntil.SetDateTime(m_dateUntil.GetYear(), m_dateUntil.GetMonth(), m_dateUntil.GetDay(),
		m_timeUntil.GetHour(), m_timeUntil.GetMinute(), m_timeUntil.GetSecond());
	if (m_listSearch.m_bDateTimeFrom && m_listSearch.m_bDateTimeUntil
		&& m_listSearch.m_dtFrom > m_listSearch.m_dtUntil)
	{
		AfxMessageBox(IDSTR(IDS_MSG_TIMERANGE_ERROR));
		return FALSE;
	}
	CString strNames, strExts;
	GetDlgItemText(IDC_EDIT_FILENAME, strNames);
	GetDlgItemText(IDC_EDIT_FILEEXT, strExts);
	GetStringArray(strNames, L'/', m_listSearch.m_aNameMatch);
	GetStringArray(strExts, L'/', m_listSearch.m_aExtMatch);
	m_listSearch.m_bNameAnd = (((CComboBox*)GetDlgItem(IDC_CB_NAME))->GetCurSel() == 0) ? FALSE : TRUE;
	return TRUE;
}

void CDlgFileSearch::CriteriaClear()
{
	CSearchCriteria& sc = APP()->m_defaultSC;
	sc.Empty();
	CriteriaInit();




	((CButton*)GetDlgItem(IDC_CHK_DATETIME_FROM))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHK_DATETIME_UNTIL))->SetCheck(BST_UNCHECKED);
	((CComboBox*)GetDlgItem(IDC_CB_TIMERANGE))->SetCurSel(0);

	m_listSearch.m_bDateTimeFrom = (((CButton*)GetDlgItem(IDC_CHK_DATETIME_FROM))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	m_listSearch.m_bDateTimeUntil = (((CButton*)GetDlgItem(IDC_CHK_DATETIME_UNTIL))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	m_dateFrom = COleDateTime::GetCurrentTime(); m_timeFrom.SetTime(0, 0, 0);
	m_dateUntil = COleDateTime::GetCurrentTime(); m_timeUntil.SetTime(23, 59, 59);
	UpdateData(FALSE);

	SetDlgItemText(IDC_EDIT_FILENAME, _T(""));
	SetDlgItemText(IDC_EDIT_FILEEXT, _T(""));
}

void CDlgFileSearch::CriteriaExport()
{
	if (CriteriaReadFromUI() == FALSE) return;
}

void CDlgFileSearch::CriteriaImport()
{
	
}

void CDlgFileSearch::CriteriaInit()
{
	CSearchCriteria& sc = APP()->m_defaultSC;
	m_editFilePath.SetWindowTextW(sc.strPath);
	SetDlgItemText(IDC_EDIT_FILENAME, sc.strName);
	((CComboBox*)GetDlgItem(IDC_CB_NAME))->SetCurSel(sc.bNameAnd);
	SetDlgItemText(IDC_EDIT_FILEEXT, sc.strExt);

	SetDlgItemText(IDC_EDIT_FILESIZE_MIN, sc.GetSizeMinString());
	SetDlgItemText(IDC_EDIT_FILESIZE_MAX, sc.GetSizeMaxString());



	((CButton*)GetDlgItem(IDC_CHK_DATETIME_FROM))->SetCheck(sc.bDateTimeFrom ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHK_DATETIME_UNTIL))->SetCheck(sc.bDateTimeUntil ? BST_CHECKED : BST_UNCHECKED);
	((CComboBox*)GetDlgItem(IDC_CB_TIMERANGE))->SetCurSel(sc.nDateTimeType);


	((CButton*)GetDlgItem(IDC_CHK_FILESTATE_LOCKED))->SetCheck(sc.bLocked ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHK_FILESTATE_HIDDEN))->SetCheck(sc.bHidden ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHK_FILESTATE_READONLY))->SetCheck(sc.bReadOnly ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHK_FILESTATE_ENCRYPTED))->SetCheck(sc.bEncrypted ? BST_CHECKED : BST_UNCHECKED);

}

void CDlgFileSearch::ResultExport()
{

}