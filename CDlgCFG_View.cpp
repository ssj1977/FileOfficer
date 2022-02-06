// CDlgCFG_View.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "FileOfficer.h"
#include "CDlgCFG_View.h"
#include "CDlgColorRule.h"
#include "EtcFunctions.h"
#include <afxdialogex.h>
#include <afxcolorbutton.h>

// CDlgCFG_View 대화 상자

IMPLEMENT_DYNAMIC(CDlgCFG_View, CDialogEx)

CDlgCFG_View::CDlgCFG_View(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CFG_VIEW, pParent)
{
	m_clrText = RGB(130, 180, 255);
	m_clrBk = RGB(0, 0, 0);
	m_bUseDefaultColor = TRUE;
	m_nFontSize = 12;
	m_bUseDefaultFont = TRUE;
	m_nIconType = SHIL_SMALL;
	m_bBold = FALSE;
	m_bBkImg = FALSE;
}

CDlgCFG_View::~CDlgCFG_View()
{
}

void CDlgCFG_View::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_COLOR_RULE, m_listColorRule);
}


BEGIN_MESSAGE_MAP(CDlgCFG_View, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT_COLOR, &CDlgCFG_View::OnBnClickedCheckDefaultColor)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT_FONT, &CDlgCFG_View::OnBnClickedCheckDefaultFont)
	ON_BN_CLICKED(IDC_BTN_COLOR_RULE_ADD, &CDlgCFG_View::OnBnClickedBtnColorRuleAdd)
	ON_BN_CLICKED(IDC_BTN_COLOR_RULE_EDIT, &CDlgCFG_View::OnBnClickedBtnColorRuleEdit)
	ON_BN_CLICKED(IDC_BTN_COLOR_RULE_DEL, &CDlgCFG_View::OnBnClickedBtnColorRuleDel)
	ON_BN_CLICKED(IDC_BTN_COLOR_RULE_UP, &CDlgCFG_View::OnBnClickedBtnColorRuleUp)
	ON_BN_CLICKED(IDC_BTN_COLOR_RULE_DOWN, &CDlgCFG_View::OnBnClickedBtnColorRuleDown)
	ON_BN_CLICKED(IDC_BTN_BKIMG_PATH, &CDlgCFG_View::OnBnClickedBtnBkimgPath)
	ON_BN_CLICKED(IDC_CHK_BKIMG, &CDlgCFG_View::OnBnClickedChkBkimg)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_COLOR_RULE, &CDlgCFG_View::OnDblclkListColorRule)
END_MESSAGE_MAP()


// CDlgCFG_View 메시지 처리기


BOOL CDlgCFG_View::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_listColorRule.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	((CButton*)GetDlgItem(IDC_CHECK_DEFAULT_COLOR))->SetCheck(m_bUseDefaultColor ? BST_CHECKED : BST_UNCHECKED);
	((CMFCColorButton*)GetDlgItem(IDC_COLOR_BK))->SetColor(m_clrBk);
	((CMFCColorButton*)GetDlgItem(IDC_COLOR_TEXT))->SetColor(m_clrText);
	((CButton*)GetDlgItem(IDC_CHECK_DEFAULT_FONT))->SetCheck(m_bUseDefaultFont ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHK_BOLD))->SetCheck(m_bBold ? BST_CHECKED : BST_UNCHECKED);
	CString strTemp;
	strTemp.Format(_T("%d"), m_nFontSize);
	GetDlgItem(IDC_EDIT_FONTSIZE)->SetWindowText(strTemp);

	CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_ICONSIZE);
	int nIndex = 0;
	nIndex = pCB->AddString(IDSTR(IDS_ICON_SMALL));
	pCB->SetItemData(nIndex, SHIL_SMALL);
	nIndex = pCB->AddString(IDSTR(IDS_ICON_LARGE));
	pCB->SetItemData(nIndex, SHIL_LARGE);
	nIndex = pCB->AddString(IDSTR(IDS_ICON_EXLARGE));
	pCB->SetItemData(nIndex, SHIL_EXTRALARGE);
	nIndex = pCB->AddString(IDSTR(IDS_ICON_JUMBO));
	pCB->SetItemData(nIndex, SHIL_JUMBO);

	for (int i = 0; pCB->GetCount(); i++)
	{
		if (pCB->GetItemData(i) == m_nIconType)
		{
			pCB->SetCurSel(i);
			break;
		}
	}
	//추가 컬러 설정 목록 초기화
	m_listColorRule.InsertColumn(0, IDSTR(IDS_COL_CLR_RULETYPE), LVCFMT_LEFT, 190);
	m_listColorRule.InsertColumn(1, IDSTR(IDS_COL_CLR_RULETEXT), LVCFMT_RIGHT, 90);
	m_listColorRule.InsertColumn(2, IDSTR(IDS_COL_CLR_RULEBK), LVCFMT_RIGHT, 90);
	m_listColorRule.InsertColumn(3, IDSTR(IDS_COL_CLR_RULEOPTION), LVCFMT_LEFT, 130);

	((CButton*)GetDlgItem(IDC_CHK_BKIMG))->SetCheck(m_bBkImg ? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemText(IDC_EDIT_BKIMG_PATH, m_strBkImgPath);
	UpdateControl();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgCFG_View::UpdateControl()
{
	GetDlgItem(IDC_COLOR_BK)->EnableWindow(!m_bUseDefaultColor);
	GetDlgItem(IDC_ST_COLOR_BK)->EnableWindow(!m_bUseDefaultColor);
	GetDlgItem(IDC_COLOR_TEXT)->EnableWindow(!m_bUseDefaultColor);
	GetDlgItem(IDC_ST_COLOR_TEXT)->EnableWindow(!m_bUseDefaultColor);
	GetDlgItem(IDC_EDIT_FONTSIZE)->EnableWindow(!m_bUseDefaultFont);
	GetDlgItem(IDC_ST_FONTSIZE)->EnableWindow(!m_bUseDefaultFont);
	GetDlgItem(IDC_EDIT_BKIMG_PATH)->EnableWindow(m_bBkImg);
	GetDlgItem(IDC_BTN_BKIMG_PATH)->EnableWindow(m_bBkImg);
}


void CDlgCFG_View::OnOK()
{
	m_clrBk = ((CMFCColorButton*)GetDlgItem(IDC_COLOR_BK))->GetColor();
	m_clrText = ((CMFCColorButton*)GetDlgItem(IDC_COLOR_TEXT))->GetColor();
	m_bUseDefaultColor = (((CButton*)GetDlgItem(IDC_CHECK_DEFAULT_COLOR))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	m_bUseDefaultFont = (((CButton*)GetDlgItem(IDC_CHECK_DEFAULT_FONT))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	m_bBold = (((CButton*)GetDlgItem(IDC_CHK_BOLD))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	CString strTemp;
	GetDlgItem(IDC_EDIT_FONTSIZE)->GetWindowText(strTemp);
	m_nFontSize = _ttoi(strTemp);
	if (m_nFontSize < 1 || m_nFontSize > 100)
	{
		AfxMessageBox(IDSTR(IDS_MSG_INVALIDFONTSIZE));
		return;
	}
	CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_ICONSIZE);
	m_nIconType = (int)pCB->GetItemData(pCB->GetCurSel());
	m_bBkImg = (((CButton*)GetDlgItem(IDC_CHK_BKIMG))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	GetDlgItemText(IDC_EDIT_BKIMG_PATH, m_strBkImgPath);

	CDialogEx::OnOK();
}


void CDlgCFG_View::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CDlgCFG_View::OnBnClickedCheckDefaultColor()
{
	m_bUseDefaultColor = !m_bUseDefaultColor;
	UpdateControl();
}


void CDlgCFG_View::OnBnClickedCheckDefaultFont()
{
	m_bUseDefaultFont = !m_bUseDefaultFont;
	UpdateControl();
}


CString RGB2String(COLORREF cr)
{
	DWORD dwR = GetRValue(cr);
	DWORD dwG = GetGValue(cr);
	DWORD dwB = GetBValue(cr);
	CString str;
	str.Format(_T("#%02X%02X%02X"), dwR, dwG, dwB);
	return str;
}

void CDlgCFG_View::OnBnClickedBtnColorRuleAdd()
{
	CDlgColorRule dlg;
	dlg.m_clrBk = ((CMFCColorButton*)GetDlgItem(IDC_COLOR_BK))->GetColor();
	dlg.m_clrText = ((CMFCColorButton*)GetDlgItem(IDC_COLOR_TEXT))->GetColor();
	if (dlg.DoModal() == IDOK)
	{
		ColorRule cr;
		cr.m_nRuleType = dlg.m_nRuleType;
		cr.m_strRuleOption = dlg.m_strRuleOption;
		cr.m_clrText = dlg.m_clrText;
		cr.m_clrBk = dlg.m_clrBk;
		APP()->m_aColorRules.Add(cr);
		int nItem = m_listColorRule.InsertItem(m_listColorRule.GetItemCount(), GetColorRuleName(cr.m_nRuleType));
		m_listColorRule.SetItemText(nItem, 1, RGB2String(cr.m_clrText));
		m_listColorRule.SetItemText(nItem, 2, RGB2String(cr.m_clrBk));
		m_listColorRule.SetItemText(nItem, 3, cr.m_strRuleOption);
		m_listColorRule.EnsureVisible(nItem, FALSE);
	}
}


void CDlgCFG_View::OnBnClickedBtnColorRuleEdit()
{
	CDlgColorRule dlg;
	int nItem = m_listColorRule.GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	ColorRule& cr = APP()->m_aColorRules[nItem];
	dlg.m_nRuleType = cr.m_nRuleType;
	dlg.m_strRuleOption = cr.m_strRuleOption;
	dlg.m_clrText = cr.m_clrText;
	dlg.m_clrBk = cr.m_clrBk;
	if (dlg.DoModal() == IDOK)
	{
		cr.m_nRuleType = dlg.m_nRuleType;
		cr.m_strRuleOption = dlg.m_strRuleOption;
		cr.m_clrText = dlg.m_clrText;
		cr.m_clrBk = dlg.m_clrBk;
		m_listColorRule.SetItemText(nItem, 0, GetColorRuleName(cr.m_nRuleType));
		m_listColorRule.SetItemText(nItem, 1, RGB2String(cr.m_clrText));
		m_listColorRule.SetItemText(nItem, 2, RGB2String(cr.m_clrBk));
		m_listColorRule.SetItemText(nItem, 3, cr.m_strRuleOption);
		m_listColorRule.EnsureVisible(nItem, FALSE);
	}
}


void CDlgCFG_View::OnBnClickedBtnColorRuleDel()
{
	if (AfxMessageBox(IDSTR(IDS_CONFIRM_DELETE), MB_OKCANCEL) == IDCANCEL) return;

	int nItem = m_listColorRule.GetNextItem(-1, LVNI_SELECTED);
	while (nItem != -1)
	{
		m_listColorRule.DeleteItem(nItem);
		APP()->m_aColorRules.RemoveAt(nItem);
		nItem = m_listColorRule.GetNextItem(-1, LVNI_SELECTED);
	}
}


void CDlgCFG_View::OnBnClickedBtnColorRuleUp()
{
	int n1 = m_listColorRule.GetNextItem(-1, LVNI_SELECTED);
	if (n1 <= 0) return;
	int n2 = n1 - 1;
	ColorRule crTemp = APP()->m_aColorRules.GetAt(n1);
	APP()->m_aColorRules.SetAt(n1, APP()->m_aColorRules.GetAt(n2));
	APP()->m_aColorRules.SetAt(n2, crTemp);
	CString strTemp;
	for (int i=0; i<m_listColorRule.GetHeaderCtrl()->GetItemCount(); i++)
	{ 
		strTemp = m_listColorRule.GetItemText(n1, i);
		m_listColorRule.SetItemText(n1, i, m_listColorRule.GetItemText(n2, i));
		m_listColorRule.SetItemText(n2, i, strTemp);
	}
	m_listColorRule.SetItemState(n1, 0, LVIS_SELECTED | LVIS_FOCUSED);
	m_listColorRule.SetItemState(n2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_listColorRule.EnsureVisible(n2, FALSE);
}


void CDlgCFG_View::OnBnClickedBtnColorRuleDown()
{
	int n1 = m_listColorRule.GetNextItem(-1, LVNI_SELECTED);
	if (n1 >= (m_listColorRule.GetItemCount() - 1)) return;
	int n2 = n1 + 1;
	ColorRule crTemp = APP()->m_aColorRules.GetAt(n1);
	APP()->m_aColorRules.SetAt(n1, APP()->m_aColorRules.GetAt(n2));
	APP()->m_aColorRules.SetAt(n2, crTemp);
	CString strTemp;
	for (int i = 0; i < m_listColorRule.GetHeaderCtrl()->GetItemCount(); i++)
	{
		strTemp = m_listColorRule.GetItemText(n1, i);
		m_listColorRule.SetItemText(n1, i, m_listColorRule.GetItemText(n2, i));
		m_listColorRule.SetItemText(n2, i, strTemp);
	}
	m_listColorRule.SetItemState(n1, 0, LVIS_SELECTED | LVIS_FOCUSED);
	m_listColorRule.SetItemState(n2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_listColorRule.EnsureVisible(n2, FALSE);
}


void CDlgCFG_View::OnBnClickedBtnBkimgPath()
{
	OPENFILENAME ofn = { 0 };
	CString strTitle;
	if (strTitle.LoadString(IDS_BKIMG_PATH) == FALSE) strTitle.Empty();
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->GetSafeHwnd();
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ENABLESIZING;
	ofn.lpstrTitle = strTitle;
	ofn.lpstrFilter = _T("Image Files(BMP,GIF,JPG,PNG)\0*.BMP;*.GIF;*.JPG;*.JPEG;*.PNG\0All Files(*.*)\0*.*\0\0");
	ofn.nMaxFile = MY_MAX_PATH;
	TCHAR* pBuf = new TCHAR[ofn.nMaxFile];
	memset(pBuf, 0, sizeof(TCHAR) * ofn.nMaxFile);
	ofn.lpstrFile = pBuf;
	if (GetOpenFileName(&ofn) != FALSE)
	{
		SetDlgItemText(IDC_EDIT_BKIMG_PATH, ofn.lpstrFile);
	}
}


void CDlgCFG_View::OnBnClickedChkBkimg()
{
	m_bBkImg = (((CButton*)GetDlgItem(IDC_CHK_BKIMG))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	UpdateControl();
}


void CDlgCFG_View::OnDblclkListColorRule(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	OnBnClickedBtnColorRuleEdit();
	*pResult = 0;
}
