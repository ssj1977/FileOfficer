// CDlgCFG_Layout.cpp: 구현 파일
//

#include "pch.h"
#include "FileOfficer.h"
#include "CDlgCFG_Layout.h"
#include "afxdialogex.h"


// CDlgCFG_Layout 대화 상자

IMPLEMENT_DYNAMIC(CDlgCFG_Layout, CDialogEx)

CDlgCFG_Layout::CDlgCFG_Layout(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CFG_LAYOUT, pParent)
{
	m_nLayoutType = LIST_LAYOUT_HORIZONTAL;
	m_nLayoutSizeType = LIST_LAYOUT_SIZE_PERCENT;
	m_nLayoutSizePercent = 50;
	m_nLayoutSizeFixed = 600;
	m_nToolBarButtonSize = 20;
	m_bToolBarVertical = FALSE;
	m_bViewShortCut1 = FALSE;
	m_bViewShortCut2 = FALSE;
	m_bUseFileIcon = TRUE;
	m_bUseFileType = FALSE;
	m_bCheckOpen = FALSE;
}

CDlgCFG_Layout::~CDlgCFG_Layout()
{
}

void CDlgCFG_Layout::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgCFG_Layout, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_PERCENT, &CDlgCFG_Layout::OnBnClickedRadioLayoutPercent)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_FIXED, &CDlgCFG_Layout::OnBnClickedRadioLayoutFixed)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_DYNAMIC, &CDlgCFG_Layout::OnBnClickedRadioLayoutDynamic)
//	ON_WM_CREATE()
END_MESSAGE_MAP()


// CDlgCFG_Layout 메시지 처리기


BOOL CDlgCFG_Layout::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	int nID = 0;
	switch (m_nLayoutType)
	{
	case LIST_LAYOUT_HORIZONTAL: nID = IDC_RADIO_LAYOUT_HORIZONTAL; break;
	case LIST_LAYOUT_VERTICAL: nID = IDC_RADIO_LAYOUT_VERTICAL; break;
	case LIST_LAYOUT_SINGLE1: nID = IDC_RADIO_LAYOUT_SINGLE_1; break;
	case LIST_LAYOUT_SINGLE2: nID = IDC_RADIO_LAYOUT_SINGLE_2; break;
	}
	((CButton*)GetDlgItem(nID))->SetCheck(BST_CHECKED);

	switch (m_nLayoutSizeType)
	{
	case LIST_LAYOUT_SIZE_PERCENT: nID = IDC_RADIO_LAYOUT_PERCENT; OnBnClickedRadioLayoutPercent(); break;
	case LIST_LAYOUT_SIZE_FIXED: nID = IDC_RADIO_LAYOUT_FIXED; OnBnClickedRadioLayoutFixed(); break;
	case LIST_LAYOUT_SIZE_DYNAMIC: nID = IDC_RADIO_LAYOUT_DYNAMIC; OnBnClickedRadioLayoutDynamic(); break;
	}
	((CButton*)GetDlgItem(nID))->SetCheck(BST_CHECKED);

	CString strTemp;
	strTemp.Format(L"%d", m_nLayoutSizePercent);
	((CEdit*)GetDlgItem(IDC_EDIT_LAYOUT_PERCENT))->SetWindowText(strTemp);
	strTemp.Format(L"%d", m_nLayoutSizeFixed);
	((CEdit*)GetDlgItem(IDC_EDIT_LAYOUT_FIXED_1))->SetWindowText(strTemp);
	strTemp.Format(L"%d", m_nToolBarButtonSize);
	((CEdit*)GetDlgItem(IDC_EDIT_LAYOUT_BTNSIZE))->SetWindowText(strTemp);
	((CButton*)GetDlgItem(IDC_CHECK_TOOLBAR_VERTICAL))->SetCheck(m_bToolBarVertical ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_VIEW_SHORTCUT_1))->SetCheck(m_bViewShortCut1 ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_VIEW_SHORTCUT_2))->SetCheck(m_bViewShortCut2 ? BST_CHECKED : BST_UNCHECKED);

	((CButton*)GetDlgItem(IDC_CHECK_USEFILEICON))->SetCheck(m_bUseFileIcon ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_USEFILETYPE))->SetCheck(m_bUseFileType ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_CHECKOPEN))->SetCheck(m_bCheckOpen ? BST_CHECKED : BST_UNCHECKED);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgCFG_Layout::OnOK()
{
	if (((CButton*)GetDlgItem(IDC_RADIO_LAYOUT_HORIZONTAL))->GetCheck() == BST_CHECKED) m_nLayoutType = LIST_LAYOUT_HORIZONTAL;
	else if (((CButton*)GetDlgItem(IDC_RADIO_LAYOUT_VERTICAL))->GetCheck() == BST_CHECKED) m_nLayoutType = LIST_LAYOUT_VERTICAL;
	else if (((CButton*)GetDlgItem(IDC_RADIO_LAYOUT_SINGLE_1))->GetCheck() == BST_CHECKED) m_nLayoutType = LIST_LAYOUT_SINGLE1;
	else if (((CButton*)GetDlgItem(IDC_RADIO_LAYOUT_SINGLE_2))->GetCheck() == BST_CHECKED) m_nLayoutType = LIST_LAYOUT_SINGLE2;

	if (((CButton*)GetDlgItem(IDC_RADIO_LAYOUT_PERCENT))->GetCheck() == BST_CHECKED) m_nLayoutSizeType = LIST_LAYOUT_SIZE_PERCENT;
	else if (((CButton*)GetDlgItem(IDC_RADIO_LAYOUT_FIXED))->GetCheck() == BST_CHECKED) m_nLayoutSizeType = LIST_LAYOUT_SIZE_FIXED;
	else if (((CButton*)GetDlgItem(IDC_RADIO_LAYOUT_DYNAMIC))->GetCheck() == BST_CHECKED) m_nLayoutSizeType = LIST_LAYOUT_SIZE_DYNAMIC;
	
	CString strTemp;
	GetDlgItemText(IDC_EDIT_LAYOUT_PERCENT, strTemp); m_nLayoutSizePercent = _ttoi(strTemp);
	GetDlgItemText(IDC_EDIT_LAYOUT_FIXED_1, strTemp); m_nLayoutSizeFixed = _ttoi(strTemp);
	GetDlgItemText(IDC_EDIT_LAYOUT_BTNSIZE, strTemp); m_nToolBarButtonSize = _ttoi(strTemp);
	m_bToolBarVertical = (((CButton*)GetDlgItem(IDC_CHECK_TOOLBAR_VERTICAL))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;

	m_bViewShortCut1 = (((CButton*)GetDlgItem(IDC_CHECK_VIEW_SHORTCUT_1))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	m_bViewShortCut2 = (((CButton*)GetDlgItem(IDC_CHECK_VIEW_SHORTCUT_2))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;

	m_bUseFileIcon = (((CButton*)GetDlgItem(IDC_CHECK_USEFILEICON))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	m_bUseFileType = (((CButton*)GetDlgItem(IDC_CHECK_USEFILETYPE))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	m_bCheckOpen = (((CButton*)GetDlgItem(IDC_CHECK_CHECKOPEN))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;

	CDialogEx::OnOK();
}


void CDlgCFG_Layout::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CDlgCFG_Layout::OnBnClickedRadioLayoutPercent()
{
	GetDlgItem(IDC_EDIT_LAYOUT_PERCENT)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_LAYOUT_FIXED_1)->EnableWindow(FALSE);
}


void CDlgCFG_Layout::OnBnClickedRadioLayoutFixed()
{
	GetDlgItem(IDC_EDIT_LAYOUT_PERCENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_LAYOUT_FIXED_1)->EnableWindow(TRUE);
}


void CDlgCFG_Layout::OnBnClickedRadioLayoutDynamic()
{
	GetDlgItem(IDC_EDIT_LAYOUT_PERCENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_LAYOUT_FIXED_1)->EnableWindow(FALSE);
}
