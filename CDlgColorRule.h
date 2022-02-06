#pragma once

#ifndef COLOR_RULE_TOTAL
#define COLOR_RULE_TOTAL 8
#define COLOR_RULE_EXT 0
#define COLOR_RULE_FOLDER 1
#define COLOR_RULE_NAME 2
#define COLOR_RULE_DATE 3
#define COLOR_RULE_COLNAME 4
#define COLOR_RULE_COLDATE 5
#define COLOR_RULE_COLSIZE 6
#define COLOR_RULE_COLTYPE 7
#endif

CString GetColorRuleName(int nRuleType);

// CDlgColorRule 대화 상자

class CDlgColorRule : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgColorRule)

public:
	CDlgColorRule(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgColorRule();
	void UpdateRuleType();

	COLORREF m_clrBk;
	COLORREF m_clrText;
	int m_nRuleType;
	CString m_strRuleOption;

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
};
