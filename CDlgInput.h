#pragma once

#ifndef INPUT_MODE_FILENAME
#define INPUT_MODE_FILENAME 0
#endif 

// CDlgInput 대화 상자

class CDlgInput : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgInput)

public:
	CDlgInput(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgInput();
	void ArrangeCtrl();
	CString m_strInput;
	CString m_strTitle;
	int m_nBH;
	int m_nMode;
	HICON m_hIcon;
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INPUT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnCancel();
	virtual void OnOK();
	CEdit m_editInput;
	virtual BOOL OnInitDialog();
};
