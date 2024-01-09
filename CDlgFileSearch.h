#pragma once
#include "afxdialogex.h"


// CDlgFileSearch 대화 상자

class CDlgFileSearch : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgFileSearch)

public:
	CDlgFileSearch(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgFileSearch();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILE_SEARCH };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	virtual void OnCancel();
	virtual void OnOK();
};
