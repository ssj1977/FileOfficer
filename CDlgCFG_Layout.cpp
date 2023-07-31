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
	m_nLayoutSizeFixed1 = 600;
//	m_nLayoutSizeFixed2 = 600;
	m_nToolBarButtonSize = 20;
	m_bToolBarVertical = FALSE;
//	m_bToolBarText = TRUE;
}

CDlgCFG_Layout::~CDlgCFG_Layout()
{
}

void CDlgCFG_Layout::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgCFG_Layout, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_HORIZONTAL, &CDlgCFG_Layout::OnBnClickedRadioLayoutHorizontal)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_VERTICAL, &CDlgCFG_Layout::OnBnClickedRadioLayoutVertical)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_SINGLE_1, &CDlgCFG_Layout::OnBnClickedRadioLayoutSingle1)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_SINGLE_2, &CDlgCFG_Layout::OnBnClickedRadioLayoutSingle2)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_PERCENT, &CDlgCFG_Layout::OnBnClickedRadioLayoutPercent)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_FIXED_1, &CDlgCFG_Layout::OnBnClickedRadioLayoutFixed1)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_FIXED_2, &CDlgCFG_Layout::OnBnClickedRadioLayoutFixed2)
END_MESSAGE_MAP()


// CDlgCFG_Layout 메시지 처리기


BOOL CDlgCFG_Layout::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	int nID = 0;
	switch (m_nLayoutType)
	{
	case LIST_LAYOUT_HORIZONTAL: nID = IDC_RADIO_LAYOUT_HORIZONTAL; OnBnClickedRadioLayoutHorizontal();  break;
	case LIST_LAYOUT_VERTICAL: nID = IDC_RADIO_LAYOUT_VERTICAL; OnBnClickedRadioLayoutVertical(); break;
	case LIST_LAYOUT_SINGLE1: nID = IDC_RADIO_LAYOUT_SINGLE_1; OnBnClickedRadioLayoutSingle1(); break;
	case LIST_LAYOUT_SINGLE2: nID = IDC_RADIO_LAYOUT_SINGLE_2; OnBnClickedRadioLayoutSingle2(); break;
	}
	((CButton*)GetDlgItem(nID))->SetCheck(BST_CHECKED);

	switch (m_nLayoutSizeType)
	{
	case LIST_LAYOUT_SIZE_PERCENT: nID = IDC_RADIO_LAYOUT_PERCENT; OnBnClickedRadioLayoutPercent(); break;
	case LIST_LAYOUT_SIZE_FIXED_1: nID = IDC_RADIO_LAYOUT_FIXED_1; OnBnClickedRadioLayoutFixed1(); break;
	case LIST_LAYOUT_SIZE_FIXED_2: nID = IDC_RADIO_LAYOUT_FIXED_2; OnBnClickedRadioLayoutFixed2(); break;
	}
	((CButton*)GetDlgItem(nID))->SetCheck(BST_CHECKED);

	CString strTemp;
	strTemp.Format(L"%d", m_nLayoutSizePercent);
	((CEdit*)GetDlgItem(IDC_EDIT_LAYOUT_PERCENT))->SetWindowText(strTemp);
	strTemp.Format(L"%d", m_nLayoutSizeFixed1);
	((CEdit*)GetDlgItem(IDC_EDIT_LAYOUT_FIXED_1))->SetWindowText(strTemp);
	//strTemp.Format(L"%d", m_nLayoutSizeFixed2);
	//((CEdit*)GetDlgItem(IDC_EDIT_LAYOUT_FIXED_2))->SetWindowText(strTemp);
	strTemp.Format(L"%d", m_nToolBarButtonSize);
	((CEdit*)GetDlgItem(IDC_EDIT_LAYOUT_BTNSIZE))->SetWindowText(strTemp);
	((CButton*)GetDlgItem(IDC_CHECK_TOOLBAR_VERTICAL))->SetCheck(m_bToolBarVertical ? BST_CHECKED : BST_UNCHECKED);
	//((CButton*)GetDlgItem(IDC_CHECK_TOOLBARTEXT))->SetCheck(m_bToolBarText ? BST_CHECKED : BST_UNCHECKED);

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
	else if (((CButton*)GetDlgItem(IDC_RADIO_LAYOUT_FIXED_1))->GetCheck() == BST_CHECKED) m_nLayoutSizeType = LIST_LAYOUT_SIZE_FIXED_1;
	else if (((CButton*)GetDlgItem(IDC_RADIO_LAYOUT_FIXED_2))->GetCheck() == BST_CHECKED) m_nLayoutSizeType = LIST_LAYOUT_SIZE_FIXED_2;
	
	CString strTemp;
	GetDlgItemText(IDC_EDIT_LAYOUT_PERCENT, strTemp); m_nLayoutSizePercent = _ttoi(strTemp);
	GetDlgItemText(IDC_EDIT_LAYOUT_FIXED_1, strTemp); m_nLayoutSizeFixed1 = _ttoi(strTemp);
	//GetDlgItemText(IDC_EDIT_LAYOUT_FIXED_2, strTemp); m_nLayoutSizeFixed2 = _ttoi(strTemp);
	GetDlgItemText(IDC_EDIT_LAYOUT_BTNSIZE, strTemp); m_nToolBarButtonSize = _ttoi(strTemp);
	m_bToolBarVertical = (((CButton*)GetDlgItem(IDC_CHECK_TOOLBAR_VERTICAL))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
//	m_bToolBarText = (((CButton*)GetDlgItem(IDC_CHECK_TOOLBARTEXT))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	CDialogEx::OnOK();
}


void CDlgCFG_Layout::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CDlgCFG_Layout::OnBnClickedRadioLayoutHorizontal()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CDlgCFG_Layout::OnBnClickedRadioLayoutVertical()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CDlgCFG_Layout::OnBnClickedRadioLayoutSingle1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CDlgCFG_Layout::OnBnClickedRadioLayoutSingle2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CDlgCFG_Layout::OnBnClickedRadioLayoutPercent()
{
	GetDlgItem(IDC_EDIT_LAYOUT_PERCENT)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_LAYOUT_FIXED_1)->EnableWindow(FALSE);
}


void CDlgCFG_Layout::OnBnClickedRadioLayoutFixed1()
{
	GetDlgItem(IDC_EDIT_LAYOUT_PERCENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_LAYOUT_FIXED_1)->EnableWindow(TRUE);
//	GetDlgItem(IDC_EDIT_LAYOUT_FIXED_2)->EnableWindow(FALSE);
}


void CDlgCFG_Layout::OnBnClickedRadioLayoutFixed2()
{
	GetDlgItem(IDC_EDIT_LAYOUT_PERCENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_LAYOUT_FIXED_1)->EnableWindow(FALSE);
//	GetDlgItem(IDC_EDIT_LAYOUT_FIXED_2)->EnableWindow(TRUE);
}
