
// FileOfficer.h: PROJECT_NAME 애플리케이션에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.

#ifndef MY_MAX_PATH
#define MY_MAX_PATH 2048
#endif

#ifndef PathTabInfo
struct PathTabInfo
{
	CWnd* pWnd;
	CString strPath;
	int iSortColumn;
	BOOL bSortAscend;
	CUIntArray aColWidth;
	PathTabInfo()
	{
		pWnd = NULL;
		iSortColumn = 0;
		bSortAscend = TRUE;
	};
	PathTabInfo(CString strPath, int iSortColumn, BOOL bSortAscend)
	{
		this->strPath = strPath;
		this->iSortColumn = iSortColumn;
		this->bSortAscend = bSortAscend;
		pWnd = NULL;
	};
	void UpdateColWidth()
	{
		if (pWnd == NULL) return;
		CListCtrl* pList = (CListCtrl*)pWnd;
		aColWidth.RemoveAll();
		int nCount = pList->GetHeaderCtrl()->GetItemCount();
		for (int i = 0; i < nCount; i++)
		{
			aColWidth.Add((UINT)pList->GetColumnWidth(i));
		}
	};
	void operator= (const PathTabInfo& pti) //CArray의 CArray를 만들때는 항상 복사 생성자를 오버로딩 해야 함
	{
		this->strPath = pti.strPath;
		this->iSortColumn = pti.iSortColumn;
		this->bSortAscend = pti.bSortAscend;
		this->pWnd = pti.pWnd;
		this->aColWidth.RemoveAll();
		this->aColWidth.Copy(pti.aColWidth);
	};
};
typedef CArray<PathTabInfo> PathTabInfoArray;
#endif 

#ifndef TabViewOption
struct TabViewOption
{
	COLORREF clrText;
	COLORREF clrBk;
	int nFontSize;
	int nIconType;
	BOOL bBold;
	BOOL bUseDefaultColor;
	BOOL bUseDefaultFont;
	TabViewOption()
	{
		clrText = RGB(0,0,0);
		clrBk = RGB(255, 255, 255);
		nFontSize = 11;
		nIconType = SHIL_SMALL;
		bBold = FALSE;
		bUseDefaultColor = TRUE;
		bUseDefaultFont = TRUE;
	};
	TabViewOption(COLORREF clrText, COLORREF clrBk, int nIconType, int nFontSize, BOOL bBold)
	{
		SetTabViewOption(clrText, clrBk, nIconType, nFontSize, bBold);
	};
	void SetTabViewOption(COLORREF clrText, COLORREF clrBk, int nIconType, int nFontSize, BOOL bBold)
	{
		this->clrText = clrText;
		this->clrBk = clrBk;
		this->nIconType = nIconType;
		this->nFontSize = nFontSize;
		this->bBold = bBold;
	};
};
typedef CArray<TabViewOption> TabViewOptionArray;
#endif

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

#ifndef ColorRule
struct ColorRule
{
	int m_nRuleType;
	CString m_strRuleOption;
	BOOL m_bClrText;
	BOOL m_bClrBk;
	COLORREF m_clrText;
	COLORREF m_clrBk;
	CStringArray m_aRuleOptions;

	ColorRule()
	{
		m_nRuleType = 0;
		m_clrText = RGB(130, 180, 255);
		m_clrBk = RGB(0, 0, 0);
		m_bClrText = TRUE;
		m_bClrBk = FALSE;
	};
	ColorRule(const ColorRule& cr)
	{
		CopyColorRule(cr);
	};
	void operator= (const ColorRule& cr) //CArray의 CArray를 만들때는 항상 복사 생성자를 오버로딩 해야 함
	{
		CopyColorRule(cr);
	};
	void CopyColorRule(const ColorRule& cr)
	{
		this->m_nRuleType = cr.m_nRuleType;
		this->m_strRuleOption = cr.m_strRuleOption;
		this->m_clrText = cr.m_clrText;
		this->m_clrBk = cr.m_clrBk;
		this->m_bClrText = cr.m_bClrText;
		this->m_bClrBk = cr.m_bClrBk;
		this->m_aRuleOptions.RemoveAll();
		this->m_aRuleOptions.Copy(cr.m_aRuleOptions);
	}
	void ParseRuleOption()
	{
		m_aRuleOptions.RemoveAll();
		CString strToken;
		int nPos = 0, nNextPos = 0;
		int nCount = m_strRuleOption.GetLength();
		while (nPos < nCount)
 		{
			nNextPos = m_strRuleOption.Find(L"/", nPos);
			if (nNextPos == -1) nNextPos = m_strRuleOption.GetLength();
			strToken = m_strRuleOption.Mid(nPos, nNextPos - nPos);
			strToken.Trim();
			nPos = nNextPos + 1;
			m_aRuleOptions.Add(strToken);
		}
	};
};
typedef CArray<ColorRule> ColorRuleArray;
#endif 

// CFileOfficerApp:
// 이 클래스의 구현에 대해서는 FileOfficer.cpp을(를) 참조하세요.
//

class CFileOfficerApp : public CWinApp
{
public:
	CFileOfficerApp();
	int m_nSortCol_Default;
	int m_bSortAscend_Default;
	CString m_strPath_Default;
	CFont m_fontDefault;
	TabViewOption m_DefaultViewOption;
	TabViewOptionArray m_aTabViewOption;
	 
	HICON m_hIcon;
	CRect m_rcMain;
	CString m_strINIPath;
	HIMAGELIST* GetImageListByType(int nIconType);
	HIMAGELIST* m_pSysImgList_SMALL;
	HIMAGELIST* m_pSysImgList_LARGE;
	HIMAGELIST* m_pSysImgList_EXTRALARGE;
	HIMAGELIST* m_pSysImgList_JUMBO;

	PathTabInfoArray m_aTab1;
	PathTabInfoArray m_aTab2;
	int m_nCurrentTab1, m_nCurrentTab2, m_nFocus;
	int m_nLayoutType;
	int m_nLayoutSizeType;
	int m_nLayoutSizePercent;
	int m_nLayoutSizeFixed1;
	int m_nLayoutSizeFixed2;
	//BOOL m_bUseFileType; //map을 이용해서 속도를 향상시켰기 때문에 옵션 불필요
	BOOL m_bToolBarText;
	BOOL m_bBkImg1;
	BOOL m_bBkImg2;
	CString m_strBkImgPath1;
	CString m_strBkImgPath2;
	ColorRuleArray m_aCR_Tab1;
	ColorRuleArray m_aCR_Tab2;

public:
	void INISave(CString strFile);
	void INILoad(CString strFile);
	DECLARE_MESSAGE_MAP()
	virtual BOOL InitInstance();
	virtual int ExitInstance();
};
inline CFileOfficerApp* APP() { return (CFileOfficerApp*)AfxGetApp(); };
extern CFileOfficerApp theApp;
