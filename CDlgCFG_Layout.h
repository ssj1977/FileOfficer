#pragma once

#ifndef LAYOUT_HORIZONTAL
//Layout Type
#define LIST_LAYOUT_HORIZONTAL 0
#define LIST_LAYOUT_VERTICAL 1
#define LIST_LAYOUT_SINGLE1 2
#define LIST_LAYOUT_SINGLE2 3
//Layout Size Type
#define LIST_LAYOUT_SIZE_PERCENT 0
#define LIST_LAYOUT_SIZE_FIXED_1 1
#define LIST_LAYOUT_SIZE_FIXED_2 2
#endif
// CDlgCFG_Layout 대화 상자

class CDlgCFG_Layout : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCFG_Layout)

public:
	CDlgCFG_Layout(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgCFG_Layout();

	int m_nLayoutType;
	int m_nLayoutSizeType;
	int m_nLayoutSizePercent;
	int m_nLayoutSizeFixed1;
	int m_nLayoutSizeFixed2;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CFG_LAYOUT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBnClickedRadioLayoutHorizontal();
	afx_msg void OnBnClickedRadioLayoutVertical();
	afx_msg void OnBnClickedRadioLayoutSingle1();
	afx_msg void OnBnClickedRadioLayoutSingle2();
	afx_msg void OnBnClickedRadioLayoutPercent();
	afx_msg void OnBnClickedRadioLayoutFixed1();
	afx_msg void OnBnClickedRadioLayoutFixed2();
};
