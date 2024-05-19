// CDlgFileSearch.cpp: 구현 파일
//

#include "pch.h"
#include "FileOfficer.h"
#include "afxdialogex.h"
#include "CDlgFileSearch.h"
#include "CDlgCFG_View.h"
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
	m_lfHeight = 12;
	m_hIcon = NULL;
//	m_brush_ItemBk.CreateSolidBrush(RGB(0,0,0));
}

CDlgFileSearch::~CDlgFileSearch()
{
//	m_brush_ItemBk.DeleteObject();
	m_font.DeleteObject();
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
//	ON_WM_CTLCOLOR()
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

	InitFont();
	m_toolSearch.CreateEx(this, TBSTYLE_LIST | TBSTYLE_WRAPABLE, WS_CHILD | WS_VISIBLE | CBRS_BORDER_ANY); // TBSTYLE_TRANSPARENT
	InitToolBar();
	UpdateChildFont();
	m_editFilePath.EnableFolderBrowseButton();
	m_listSearch.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_listSearch.SetIconType(m_nIconType);
	m_listSearch.InitColumns();

	((CDateTimeCtrl*)GetDlgItem(IDC_FILE_TIME_FROM))->SetFormat(L"HH:mm:ss"); //24h
	((CDateTimeCtrl*)GetDlgItem(IDC_FILE_TIME_UNTIL))->SetFormat(L"HH:mm:ss"); //24h
	
	TabViewOption& tvo = m_tvoSearch;
	COLORREF clrBK = tvo.bUseDefaultColor ? APP()->m_DefaultViewOption.clrBk : tvo.clrBk;
	COLORREF clrText = tvo.bUseDefaultColor ? APP()->m_DefaultViewOption.clrText : tvo.clrText;
	SetCtrlColor(clrBK, clrText, TRUE, TRUE);

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
	else if (pMsg->message == WM_KEYUP)
	{
		if ((GetKeyState(VK_CONTROL) & 0xFF00) != 0)
		{
			if (pMsg->wParam == _T('S'))
			{
				ResultExport();
				return TRUE;
			}
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgFileSearch::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (::IsWindow(m_editFilePath.m_hWnd)) ArrangeCtrl();
}

inline int MoveHorizontal(CWnd* pWnd, int x, int y, int w, int h, double fSizeRatio)
{
	int new_width = (int)((double)w * fSizeRatio);
	pWnd->MoveWindow(x, y, new_width, h);
	return new_width;
}

void CDlgFileSearch::ArrangeCtrl()
{
	double fSizeRatio = (double)(m_lfHeight) / 20.0F; //폰트 크기에 따른 조정 비율
	if (fSizeRatio < 1.0F) fSizeRatio = 1;
	int h_line = (int)(30.0F * fSizeRatio); //높이
	int w_static = (int)(120.0F * fSizeRatio); // 항목명(좌측) 부분 기본 폭
	int w_combo = (int)(160.0F * fSizeRatio); // 콤보박스(우측) 부분 기본 폭

	CRect rcDlg, rcST, rcCB, rcTemp;
	GetClientRect(rcDlg);
	int cx = 0, cy = 0, tx = 0, sw = 5, sh=5;
	cx = sw; cy = sw;
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
	//int w_edit = rcDlg.Width() - w_static - (sw * 3); //남는 공간 계산
	int w_edit = rcDlg.Width() - w_static - (sw * 3) - w_combo - 2; //남는 공간 계산

	tx = cx;
	tx += MoveHorizontal(GetDlgItem(IDC_ST_FILEPATH), tx, cy, w_static, h_line, 1.0F) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_EDIT_FILEPATH), tx, cy, w_edit, h_line, 1.0F) + 2;
	tx += MoveHorizontal(GetDlgItem(IDC_CB_SEARCH_TARGET), tx, cy, w_combo, h_line, 1.0F) + sw;
	cy += h_line + sh;

	//w_edit = w_edit - w_combo - 2; //남는 공간 계산
	tx = cx;
	tx += MoveHorizontal(GetDlgItem(IDC_ST_FILENAME), tx, cy, w_static, h_line, 1.0F) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_EDIT_FILENAME), tx, cy, w_edit, h_line, 1.0F) + 2; 
	tx += MoveHorizontal(GetDlgItem(IDC_CB_NAME), tx, cy, w_combo, h_line, 1.0F) + sw;
	cy += h_line + sh;;

	tx = cx;
	tx += MoveHorizontal(GetDlgItem(IDC_ST_FILEEXT), tx, cy, w_static, h_line, 1.0F) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_EDIT_FILEEXT), tx, cy, w_edit, h_line, 1.0F) + 2;
	tx += MoveHorizontal(GetDlgItem(IDC_ST_GUIDE), tx, cy, w_combo, h_line, 1.0F) + sw;
	cy += h_line + sh;;

	tx = cx;
	tx += MoveHorizontal(GetDlgItem(IDC_ST_FILESIZE), tx, cy, w_static, h_line, 1.0F) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_EDIT_FILESIZE_MIN), tx, cy, 180, h_line, fSizeRatio) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_ST_SIZERANGE), tx, cy, 20, h_line, fSizeRatio) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_EDIT_FILESIZE_MAX), tx, cy, 180, h_line, fSizeRatio) + sw;
	cy += h_line + sh;;

	tx = cx;
	tx += MoveHorizontal(GetDlgItem(IDC_ST_FILEDATETIME), tx, cy, w_static, h_line, 1.0F) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_CB_TIMERANGE), tx, cy, 100, h_line, fSizeRatio) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_CHK_DATETIME_FROM), tx, cy, 20, h_line, 1.0F) + sw; //체크박스 버튼은 크기가 조정되지 않음
	tx += MoveHorizontal(GetDlgItem(IDC_FILE_DATE_FROM), tx, cy, 150, h_line, fSizeRatio) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_FILE_TIME_FROM), tx, cy, 120, h_line, fSizeRatio) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_ST_DATETIMERANGE), tx, cy, 20, h_line, fSizeRatio) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_CHK_DATETIME_UNTIL), tx, cy, 20, h_line, 1.0F) + sw;  //체크박스 버튼은 크기가 조정되지 않음
	tx += MoveHorizontal(GetDlgItem(IDC_FILE_DATE_UNTIL), tx, cy, 150, h_line, fSizeRatio) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_FILE_TIME_UNTIL), tx, cy, 120, h_line, fSizeRatio) + sw;
	cy += h_line + sh;

	tx = cx;
	tx += MoveHorizontal(GetDlgItem(IDC_ST_FILESTATE), tx, cy, w_static, h_line, 1.0F) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_CHK_FILESTATE_LOCKED), tx, cy, 100, h_line, fSizeRatio) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_CHK_FILESTATE_HIDDEN), tx, cy, 100, h_line, fSizeRatio) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_CHK_FILESTATE_READONLY), tx, cy, 100, h_line, fSizeRatio) + sw;
	tx += MoveHorizontal(GetDlgItem(IDC_CHK_FILESTATE_ENCRYPTED), tx, cy, 100, h_line, fSizeRatio) + sw;
	cy += h_line + sh;

	int nListHeight = rcDlg.Height() - cy - sh - h_line;
	GetDlgItem(IDC_LIST_SEARCH)->MoveWindow(cx, cy, rcDlg.Width() - (cx * 2), rcDlg.Height() - cy - sh - h_line);
	cy += nListHeight;
	GetDlgItem(IDC_EDIT_SEARCH_MSG)->MoveWindow(cx, cy, rcDlg.Width() - (cx * 2), h_line);
}


void CDlgFileSearch::SearchStart()
{
	if (CriteriaReadFromUI() == FALSE) return;
	m_listSearch.FileSearch_Begin();
	UpdateToolBar(TRUE);
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
		SetDlgItemText(IDC_EDIT_SEARCH_MSG, _T(""));
		UpdateToolBar(FALSE);
		return TRUE;
	case IDM_SEARCH_RESULT_EXPORT:		ResultExport();	return TRUE;
	case IDM_PLAY_ITEM:		m_listSearch.OpenSelectedItem(TRUE);	return TRUE;
	case IDM_SEARCH_RESULT_COPY: m_listSearch.ClipBoardExport(FALSE); 	return TRUE;
	case IDM_SEARCH_RESULT_CUT: m_listSearch.ClipBoardExport(TRUE);		return TRUE;
	case IDM_SEARCH_CRITERIA_EXPORT:	CriteriaExport();	return TRUE;
	case IDM_SEARCH_CRITERIA_IMPORT:	CriteriaImport();	return TRUE;
	case IDM_SEARCH_CRITERIA_CLEAR:		CriteriaClear();	return TRUE;
	case IDM_SEARCH_RESULT_SELECTALL:	m_listSearch.SelectAllItems();	return TRUE;
	case IDM_SEARCH_RESULT_OPENFOLDER: m_listSearch.OpenSelectedParent(FALSE); return TRUE;
	case IDM_SEARCH_RESULT_VIEWTAB: m_listSearch.OpenSelectedParent(TRUE); return TRUE;
	case IDM_SEARCH_RESULT_REMOVE: m_listSearch.RemoveSelected();  return TRUE;
	case IDM_SEARCH_RESULT_DELETE: m_listSearch.DeleteSelected();  return TRUE;
	case IDM_SEARCH_SETTING: ConfigViewOption(); return TRUE;
	case IDM_SEARCH_MSG: 
		SetDlgItemText(IDC_EDIT_SEARCH_MSG, m_listSearch.m_strMsg);	
		if (lParam == 1) UpdateToolBar(FALSE); //마지막 메시지일때
		return TRUE;

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
	UpdateToolBar(FALSE);
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
	//검색 대상 종류
	sc.nTargetType = ((CComboBox*)GetDlgItem(IDC_CB_SEARCH_TARGET))->GetCurSel();
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
	OPENFILENAME ofn = {};
	CString strTitle;
	if (strTitle.LoadString(IDS_SEARCH_CRITERIA_EXPORT) == FALSE) strTitle.Empty();
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->GetSafeHwnd();
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_ENABLESIZING;
	ofn.lpstrTitle = strTitle;
	ofn.lpstrFilter = _T("FileOfficer Search Criteria(*.fsc)\0*.fsc\0\0");
	ofn.lpstrDefExt = _T("fsc");
	ofn.nMaxFile = MY_MAX_PATH;
	TCHAR* pBuf = new TCHAR[MY_MAX_PATH]; pBuf[0] = _T('\0');
	ofn.lpstrFile = pBuf;
	if (GetSaveFileName(&ofn) != FALSE)
	{
		CString strData = m_listSearch.m_SC.StringExport();
		WriteCStringToFile(ofn.lpstrFile, strData, FALSE, FALSE);
	}
	delete[] pBuf;
}

void CDlgFileSearch::CriteriaImport()
{
	CString strImportData, strName;
	OPENFILENAME ofn = {};
	CString strTitle;
	if (strTitle.LoadString(IDS_SEARCH_CRITERIA_IMPORT) == FALSE) strTitle.Empty();
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->GetSafeHwnd();
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ENABLESIZING;
	ofn.lpstrTitle = strTitle;
	ofn.lpstrFilter = _T("FileOfficer Search Criteria(*.fsc)\0*.fsc\0\0");
	ofn.lpstrDefExt = _T("fsc");
	ofn.nMaxFile = MY_MAX_PATH;
	TCHAR* pBuf = new TCHAR[MY_MAX_PATH]; pBuf[0] = _T('\0');
	ofn.lpstrFile = pBuf;
	if (GetOpenFileName(&ofn) != FALSE)
	{
		ReadFileToCString(ofn.lpstrFile, strImportData);
		m_listSearch.m_SC.StringImport(strImportData);
		CriteriaInit(m_listSearch.m_SC);
	}
	delete[] pBuf;
}

void CDlgFileSearch::CriteriaInit(SearchCriteria& sc)
{
	m_editFilePath.SetWindowTextW(sc.strStartPath);
	((CComboBox*)GetDlgItem(IDC_CB_SEARCH_TARGET))->SetCurSel(sc.nTargetType);
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
	if (m_listSearch.GetItemCount() == 0) return;
	OPENFILENAME ofn = {};
	CString strTitle;
	BOOL b = strTitle.LoadString(IDS_SEARCH_RESULT_EXPORT);
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->GetSafeHwnd();
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_ENABLESIZING;
	ofn.lpstrTitle = strTitle;
	ofn.lpstrFilter = _T("CSV Files(*.csv)\0*.csv\0Text Files(*.txt)\0*.txt\0\0");
	ofn.lpstrDefExt = _T("csv");
	ofn.nMaxFile = MY_MAX_PATH;
	TCHAR* pBuf = new TCHAR[MY_MAX_PATH]; pBuf[0] = _T('\0');
	ofn.lpstrFile = pBuf;
	if (GetSaveFileName(&ofn) != FALSE)
	{
		CString strExt = Get_Ext(ofn.lpstrFile, FALSE, FALSE);
		CString strData, strLine, strToken;
		int nRowCount = m_listSearch.GetItemCount();
		int nColCount = m_listSearch.GetHeaderCtrl().GetItemCount();
		BOOL bCSV = FALSE;
		if (strExt.CompareNoCase(_T("csv")) == 0) bCSV = TRUE;
		for (int i = 0; i < nRowCount; i++)
		{ 
			if (bCSV == TRUE)
			{
				strLine.Empty();
				for (int j = 0; j < nColCount; j++)
				{
					strToken = m_listSearch.GetItemText(i, j);
					strToken.Replace(_T("\""), _T("\"\""));
					if (strLine.IsEmpty() == FALSE) strLine += _T(",");
					strLine += _T("\"") + strToken + _T("\"");
				}
			}
			else
			{
				strLine = m_listSearch.GetItemFullPath(i);
			}
			strData += strLine + _T("\r\n");
		}
		WriteCStringToFile(ofn.lpstrFile, strData, bCSV, bCSV);
	}
	delete[] pBuf;
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
	if (m_listSearch.m_bWorking) pMenu->EnableMenuItem(IDM_SEARCH_BEGIN, MF_BYCOMMAND | MF_GRAYED);
	if (m_listSearch.GetItemCount() == 0 || m_listSearch.m_bWorking == TRUE)
	{
		pMenu->EnableMenuItem(IDM_SEARCH_RESULT_CLEAR, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SEARCH_RESULT_EXPORT, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SEARCH_RESULT_SELECTALL, MF_BYCOMMAND | MF_GRAYED);
	}
	int nSelected = m_listSearch.GetSelectedCount();
	if (nSelected == 0)
	{
		pMenu->EnableMenuItem(IDM_PLAY_ITEM, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SEARCH_RESULT_COPY, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SEARCH_RESULT_CUT, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SEARCH_RESULT_REMOVE, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SEARCH_RESULT_DELETE, MF_BYCOMMAND | MF_GRAYED);
	}
	if (nSelected != 1)
	{
		pMenu->EnableMenuItem(IDM_SEARCH_RESULT_VIEWTAB, MF_BYCOMMAND | MF_GRAYED);
		pMenu->EnableMenuItem(IDM_SEARCH_RESULT_OPENFOLDER, MF_BYCOMMAND | MF_GRAYED);
	}
	pMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
}

void CDlgFileSearch::UpdateToolBar(BOOL bWorking)
{
	m_toolSearch.GetToolBarCtrl().HideButton(IDM_SEARCH_BEGIN, bWorking);
	m_toolSearch.GetToolBarCtrl().HideButton(IDM_SEARCH_STOP, !bWorking);
	BOOL bEnable = !bWorking;
	if (bEnable == TRUE && m_listSearch.GetItemCount() == 0) bEnable = FALSE;
	m_toolSearch.GetToolBarCtrl().EnableButton(IDM_SEARCH_RESULT_CLEAR,  bEnable);
	m_toolSearch.GetToolBarCtrl().EnableButton(IDM_SEARCH_RESULT_EXPORT, bEnable);
}

void CDlgFileSearch::InitFont()
{
	TabViewOption& tvo = m_tvoSearch;
	LOGFONT lf;
	if (tvo.bUseDefaultFont == FALSE)
	{
		if (tvo.strFontName.IsEmpty() == FALSE)
		{
			memset(&lf, 0, sizeof(LOGFONT));
			_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, tvo.strFontName, _TRUNCATE);
		}
		else //폰트 이름이 없으면 다이얼로그의 기본 폰트를 가져와서 크기 등을 바꾼다
		{
			GetFont()->GetLogFont(&lf);
		}
		lf.lfHeight = tvo.nFontSize * 10;
		lf.lfWeight = tvo.nFontWeight;
		lf.lfItalic = tvo.bFontItalic;
		m_font.DeleteObject();
		m_font.CreatePointFontIndirect(&lf);
	}
	else  //윈도우 기본 폰트를 써야 할때
	{
		GetFont()->GetLogFont(&lf);
		m_font.DeleteObject();
		m_font.CreateFontIndirect(&lf);
	}
	//화면 확대 축소 비율을 반영한 컨트롤 배치용 초기값 구하기
	m_font.GetLogFont(&lf);
	m_lfHeight = abs(lf.lfHeight);
}

void CDlgFileSearch::UpdateChildFont()
{
	if (::IsWindow(GetSafeHwnd()) == FALSE) return;
	CWnd* pWnd = GetWindow(GW_CHILD);
	while (pWnd)
	{
		if (::IsWindow(pWnd->GetSafeHwnd())) pWnd->SetFont(&m_font);
		pWnd = pWnd->GetWindow(GW_HWNDNEXT);
	}
	InitToolBar();
	//if (::IsWindow(m_toolSearch.GetSafeHwnd()))
	//{
	//	m_toolSearch.SetFont(&m_font);
	//	InitToolBar();
	//}
	/*if (::IsWindow(m_tabPath.GetSafeHwnd())) m_tabPath.SetFont(&m_font);
	if (::IsWindow(m_editPath.GetSafeHwnd())) m_editPath.SetFont(&m_font);
	if (::IsWindow(m_listSearch.GetSafeHwnd())) m_listSearch.SetFont(&m_font);
	GetDlgItem(IDC_ST_BAR)->SetFont(&m_font);
	GetDlgItem(IDC_ST_SHORTCUT)->SetFont(&m_font);
	GetDlgItem(IDC_EDIT_FIND)->SetFont(&m_font);
	GetDlgItem(IDC_BTN_FIND)->SetFont(&m_font);
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			CMFCListCtrl* pList = (CMFCListCtrl*)m_aTabInfo[i].pWnd;
			pList->SetFont(&m_font);
		}
	}*/
}

void CDlgFileSearch::ConfigViewOption()
{
	TabViewOption& tvo = m_tvoSearch;
	CDlgCFG_View dlg;
	dlg.m_tvo = tvo;
	dlg.m_bUseColorRule = FALSE;
	m_font.GetLogFont(&dlg.m_lf);
	BOOL bUpdateClrBk = FALSE, bUpdateClrText = FALSE, bFontUpdated = FALSE;
	if (dlg.DoModal() == IDOK)
	{
		//Color
		if (tvo.bUseDefaultColor != dlg.m_tvo.bUseDefaultColor)
		{
			tvo.bUseDefaultColor = dlg.m_tvo.bUseDefaultColor;
			if (tvo.bUseDefaultColor == FALSE)
			{
				tvo.clrText = dlg.m_tvo.clrText;
				tvo.clrBk = dlg.m_tvo.clrBk;
			}
			bUpdateClrBk = TRUE;
			bUpdateClrText = TRUE;
		}
		if (tvo.clrBk != dlg.m_tvo.clrBk)
		{
			tvo.clrBk = dlg.m_tvo.clrBk;
			if (tvo.bUseDefaultColor == FALSE) bUpdateClrBk = TRUE;
		}
		if (tvo.clrText != dlg.m_tvo.clrText)
		{
			tvo.clrText = dlg.m_tvo.clrText;
			if (tvo.bUseDefaultColor == FALSE) bUpdateClrText = TRUE;
		}
		COLORREF clrBK = tvo.bUseDefaultColor ? APP()->m_DefaultViewOption.clrBk : tvo.clrBk;
		COLORREF clrText = tvo.bUseDefaultColor ? APP()->m_DefaultViewOption.clrText : tvo.clrText;
		SetCtrlColor(clrBK, clrText, bUpdateClrBk, bUpdateClrText);
		//Font
		if (dlg.m_bUpdateFont != FALSE) // 폰트 선택 다이얼로그를 연 경우
		{
			CString strFontName = dlg.m_lf.lfFaceName;
			if (strFontName.IsEmpty() == FALSE)	tvo.strFontName = strFontName;
			tvo.nFontSize = dlg.m_tvo.nFontSize;
			tvo.nFontWeight = dlg.m_lf.lfWeight;
			tvo.bFontItalic = dlg.m_lf.lfItalic;
			if (tvo.bUseDefaultFont == FALSE) bFontUpdated = TRUE;
		}
		if (tvo.nFontSize != dlg.m_tvo.nFontSize) // 에디트 컨트롤에서 폰트 크기만 변경
		{
			tvo.nFontSize = dlg.m_tvo.nFontSize;
			if (tvo.bUseDefaultFont == FALSE) bFontUpdated = TRUE;
		}
		if (tvo.bUseDefaultFont != dlg.m_tvo.bUseDefaultFont) // 디폴트 폰트 사용여부 변경
		{
			tvo.bUseDefaultFont = dlg.m_tvo.bUseDefaultFont;
			bFontUpdated = TRUE;
		}
		if (bFontUpdated != FALSE)
		{
			InitFont();
			UpdateChildFont();
			ArrangeCtrl();
		}
		//Icon
		if (tvo.nIconType != dlg.m_tvo.nIconType)
		{
			m_listSearch.SetIconType(dlg.m_tvo.nIconType);
		}
		//Background Image Path
		if (dlg.m_tvo.bUseBkImage != tvo.bUseBkImage || dlg.m_tvo.strBkImagePath != tvo.strBkImagePath)
		{
			tvo.bUseBkImage = dlg.m_tvo.bUseBkImage;
			tvo.strBkImagePath = dlg.m_tvo.strBkImagePath;
			if (tvo.bUseBkImage)
			{
				m_listSearch.SetBkImage(tvo.strBkImagePath.GetBuffer(), FALSE, 0, 0);
				tvo.strBkImagePath.ReleaseBuffer();
			}
			else
			{
				LVBKIMAGE li;
				li.ulFlags = LVBKIF_SOURCE_NONE;
				m_listSearch.SetBkImage(&li);
			}
		}
		//Color Rules
		tvo.aColorRules.Copy(dlg.m_tvo.aColorRules);
		RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}
}

void CDlgFileSearch::SetCtrlColor(COLORREF clrBk, COLORREF clrText, BOOL bSetBk, BOOL bSetText)
{
	if (bSetBk == FALSE && bSetText == FALSE) return;
	if (bSetBk)
	{
		m_editFilePath.SetBkColor(clrBk);
		m_listSearch.SetBkColor(clrBk);
		m_listSearch.SetTextBkColor(TRANSPARENT);
//		m_brush_ItemBk.DeleteObject();
//		m_brush_ItemBk.CreateSolidBrush(clrBk);
	}
	if (bSetText)
	{
		m_editFilePath.SetTextColor(clrText);
		m_listSearch.SetTextColor(clrText);
	}
}


/*HBRUSH CDlgFileSearch::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_EDIT)
	{
		TabViewOption& tvo = m_tvoSearch;
		pDC->SetTextColor(tvo.clrText);
		pDC->SetBkColor(tvo.clrBk);
		return m_brush_ItemBk;
	}
	return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}*/

