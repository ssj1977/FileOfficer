﻿// CDlgColorRule.cpp: 구현 파일
//

#include "pch.h"
#include "FileOfficer.h"
#include "CDlgColorRule.h"
#include "afxdialogex.h"
#include "EtcFunctions.h"

CString GetColorRuleName(int nRuleType)
{
	int aType[COLOR_RULE_TOTAL] = { COLOR_RULE_EXT, COLOR_RULE_FOLDER, COLOR_RULE_NAME, COLOR_RULE_DATE
		,COLOR_RULE_COLNAME,COLOR_RULE_COLDATE,COLOR_RULE_COLSIZE,COLOR_RULE_COLTYPE };
	int aTypeString[COLOR_RULE_TOTAL] = { IDS_CLR_RULE_EXT, IDS_CLR_RULE_FOLDER, IDS_CLR_RULE_NAME, IDS_CLR_RULE_DATE
		,IDS_CLR_RULE_COLNAME,IDS_CLR_RULE_COLDATE,IDS_CLR_RULE_COLSIZE,IDS_CLR_RULE_COLTYPE };
	CString strRuleName;
	for (int i = 0; i < COLOR_RULE_TOTAL; i++)
	{
		if (aType[i] == nRuleType)
		{
			strRuleName.LoadString(aTypeString[i]);
			break;
		}
	}
	return strRuleName;
}

// CDlgColorRule 대화 상자

IMPLEMENT_DYNAMIC(CDlgColorRule, CDialogEx)

CDlgColorRule::CDlgColorRule(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COLOR_RULE, pParent)
{
}

CDlgColorRule::~CDlgColorRule()
{
}

void CDlgColorRule::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CB_RULETYPE, m_cbRuleType);
}


BEGIN_MESSAGE_MAP(CDlgColorRule, CDialogEx)
	ON_CBN_SELCHANGE(IDC_CB_RULETYPE, &CDlgColorRule::OnSelchangeCbRuletype)
	ON_BN_CLICKED(IDC_CHK_COLOR_BK, &CDlgColorRule::OnBnClickedChkColorBk)
	ON_BN_CLICKED(IDC_CHK_COLOR_TEXT, &CDlgColorRule::OnBnClickedChkColorText)
END_MESSAGE_MAP()


// CDlgColorRule 메시지 처리기


BOOL CDlgColorRule::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CMFCColorButton* pColorText = (CMFCColorButton*)GetDlgItem(IDC_COLOR_TEXT);
	pColorText->SetColor(m_cr.m_clrText);
	pColorText->EnableWindow(m_cr.m_bClrText);
	((CButton*)GetDlgItem(IDC_CHK_COLOR_TEXT))->SetCheck(m_cr.m_bClrText ? BST_CHECKED : BST_UNCHECKED);
	CMFCColorButton* pColorBk = (CMFCColorButton*)GetDlgItem(IDC_COLOR_BK);
	pColorBk->SetColor(m_cr.m_clrBk);
	pColorBk->EnableWindow(m_cr.m_bClrBk);
	((CButton*)GetDlgItem(IDC_CHK_COLOR_BK))->SetCheck(m_cr.m_bClrBk ? BST_CHECKED : BST_UNCHECKED);
	int nItem = 0;
	int aType[COLOR_RULE_TOTAL] = { COLOR_RULE_EXT, COLOR_RULE_FOLDER, COLOR_RULE_NAME, COLOR_RULE_DATE
	,COLOR_RULE_COLNAME,COLOR_RULE_COLDATE,COLOR_RULE_COLSIZE,COLOR_RULE_COLTYPE };
	for (int i = 0; i < COLOR_RULE_TOTAL; i++)
	{   
		nItem = m_cbRuleType.AddString(GetColorRuleName(aType[i]));
		m_cbRuleType.SetItemData(nItem, (DWORD_PTR)(aType[i]));
	}
	SetDlgItemText(IDC_EDIT_COLOR_RULE, m_cr.m_strRuleOption);
	m_cbRuleType.SetCurSel(m_cr.m_nRuleType);
	UpdateRuleType();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgColorRule::OnOK()
{
	m_cr.m_bClrText = ((CButton*)GetDlgItem(IDC_CHK_COLOR_TEXT))->GetCheck() == BST_CHECKED ? TRUE : FALSE;
	CMFCColorButton* pColorText = (CMFCColorButton*)GetDlgItem(IDC_COLOR_TEXT);
	m_cr.m_clrText = pColorText->GetColor();
	m_cr.m_bClrBk = ((CButton*)GetDlgItem(IDC_CHK_COLOR_BK))->GetCheck() == BST_CHECKED ? TRUE : FALSE;
	CMFCColorButton* pColorBk = (CMFCColorButton*)GetDlgItem(IDC_COLOR_BK);
	m_cr.m_clrBk = pColorBk->GetColor();
	int nSel = m_cbRuleType.GetCurSel();
	if (nSel != -1)	m_cr.m_nRuleType = (int)m_cbRuleType.GetItemData(nSel);
	GetDlgItemText(IDC_EDIT_COLOR_RULE, m_cr.m_strRuleOption);

	BOOL bError = FALSE;
	switch (m_cr.m_nRuleType)
	{
	case COLOR_RULE_DATE:
		if (_ttoi(m_cr.m_strRuleOption) <= 0) bError = TRUE;
		break;
	case COLOR_RULE_EXT:
	case COLOR_RULE_NAME:
		if (m_cr.m_strRuleOption.IsEmpty()) bError = TRUE;
		break;
	}
	if (bError)
	{
		AfxMessageBox(IDSTR(IDS_MSG_COLORRULE_DATEERR));
		return;
	}
	CDialogEx::OnOK();
}


void CDlgColorRule::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CDlgColorRule::UpdateRuleType()
{
	int nType = m_cbRuleType.GetCurSel();
	CString strGuide;
	switch (nType)
	{
	case COLOR_RULE_EXT:
		strGuide.LoadString(IDS_CLR_RULE_EXT_GUIDE);
		break;
	case COLOR_RULE_FOLDER:
		break;
	case COLOR_RULE_NAME:
		strGuide.LoadString(IDS_CLR_RULE_NAME_GUIDE);
		break;
	case COLOR_RULE_DATE:
		strGuide.LoadString(IDS_CLR_RULE_DATE_GUIDE);
		break;
	case COLOR_RULE_COLNAME:
		break;
	case COLOR_RULE_COLDATE:
		break;
	case COLOR_RULE_COLSIZE:
		break;
	case COLOR_RULE_COLTYPE:
		break;
	default:
		break;
	}
	BOOL bShowOption = !(strGuide.IsEmpty());
	GetDlgItem(IDC_EDIT_COLOR_RULE)->ShowWindow(bShowOption ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_STATIC_COLOR_RULE)->ShowWindow(bShowOption ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_STATIC_INPUT_GUIDE)->ShowWindow(bShowOption ? SW_SHOW : SW_HIDE);
	SetDlgItemText(IDC_STATIC_INPUT_GUIDE, strGuide);
}



void CDlgColorRule::OnSelchangeCbRuletype()
{
	UpdateRuleType();
}


void CDlgColorRule::OnBnClickedChkColorText()
{
	m_cr.m_bClrText = ((CButton*)GetDlgItem(IDC_CHK_COLOR_TEXT))->GetCheck() == BST_CHECKED ? TRUE : FALSE;
	CMFCColorButton* pColorText = (CMFCColorButton*)GetDlgItem(IDC_COLOR_TEXT);
	pColorText->EnableWindow(m_cr.m_bClrText);
}


void CDlgColorRule::OnBnClickedChkColorBk()
{
	m_cr.m_bClrBk = ((CButton*)GetDlgItem(IDC_CHK_COLOR_BK))->GetCheck() == BST_CHECKED ? TRUE : FALSE;
	CMFCColorButton* pColorBk = (CMFCColorButton*)GetDlgItem(IDC_COLOR_BK);
	pColorBk->EnableWindow(m_cr.m_bClrBk);
}
