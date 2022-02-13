#pragma once

CString GetColorRuleName(int nRuleType);
// CDlgColorRule 대화 상자

class CDlgColorRule : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgColorRule)

public:
	CDlgColorRule(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgColorRule();
	void UpdateRuleType();

	ColorRule m_cr;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COLOR_RULE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	CComboBox m_cbRuleType;
	afx_msg void OnSelchangeCbRuletype();
	afx_msg void OnBnClickedChkColorText();
	afx_msg void OnBnClickedChkColorBk();
};
