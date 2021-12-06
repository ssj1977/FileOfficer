// CDlgInput.cpp: 구현 파일
//

#include "pch.h"
#include "FileOfficer.h"
#include "CDlgInput.h"
#include "afxdialogex.h"


// CDlgInput 대화 상자

IMPLEMENT_DYNAMIC(CDlgInput, CDialogEx)

CDlgInput::CDlgInput(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INPUT, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nBH = 30;
	m_nMode = INPUT_MODE_FILENAME;
}

CDlgInput::~CDlgInput()
{
}

void CDlgInput::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INPUT, m_editInput);
}


BEGIN_MESSAGE_MAP(CDlgInput, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CDlgInput 메시지 처리기

BOOL CDlgInput::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	LOGFONT lf;
	GetFont()->GetLogFont(&lf);
	m_nBH = abs(lf.lfHeight) * 2;
	SetWindowText(m_strTitle);
	m_editInput.SetWindowText(m_strInput);

	ArrangeCtrl();
	if (m_nMode == INPUT_MODE_FILENAME)
	{
		int nPos = m_strInput.ReverseFind(L'.');
		if (nPos > 0)	m_editInput.SetSel(0, nPos);
		else 			m_editInput.SetSel(0, -1);
		m_editInput.SetFocus();
		return FALSE;
	}
	return TRUE; 
}



void CDlgInput::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	ArrangeCtrl();
}

void CDlgInput::ArrangeCtrl()
{
	if (IsWindow(m_editInput.GetSafeHwnd()) == FALSE) return;
	CRect rc;
	GetClientRect(rc);
	rc.DeflateRect(5, 5, 5, 5);
	m_editInput.MoveWindow(rc.left, rc.top, rc.Width(), rc.Height() - m_nBH - 3);
	int nBW = rc.Width() / 2;
	rc.top = rc.bottom - m_nBH;
	GetDlgItem(IDOK)->MoveWindow(rc.left, rc.top, nBW, m_nBH);
	rc.left += nBW+1;
	GetDlgItem(IDCANCEL)->MoveWindow(rc.left, rc.top, rc.Width(), m_nBH);
}



void CDlgInput::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CDlgInput::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_editInput.GetWindowText(m_strInput);
	CDialogEx::OnOK();
}



