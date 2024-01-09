// CDlgFileSearch.cpp: 구현 파일
//

#include "pch.h"
#include "FileOfficer.h"
#include "afxdialogex.h"
#include "CDlgFileSearch.h"


// CDlgFileSearch 대화 상자

IMPLEMENT_DYNAMIC(CDlgFileSearch, CDialogEx)

CDlgFileSearch::CDlgFileSearch(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILE_SEARCH, pParent)
{

}

CDlgFileSearch::~CDlgFileSearch()
{
}

void CDlgFileSearch::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgFileSearch, CDialogEx)
END_MESSAGE_MAP()


// CDlgFileSearch 메시지 처리기


void CDlgFileSearch::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CDlgFileSearch::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnOK();
}
