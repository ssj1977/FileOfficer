
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
	int nRuleType;
	CString strRuleOption;
	BOOL bClrText;
	BOOL bClrBk;
	COLORREF clrText;
	COLORREF clrBk;
	CStringArray aRuleOptions;
	ColorRule();
	ColorRule(const ColorRule& cr);
	//CArray의 CArray를 만들때는 항상 복사 생성자를 오버로딩 해야 함
	void operator= (const ColorRule& cr) {	CopyColorRule(cr);	};
	void CopyColorRule(const ColorRule& cr);
	CString StringExport();
	void StringImport(CString strData);
	void ParseRuleOption();
};
typedef CArray<ColorRule> ColorRuleArray;
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
	BOOL bUseBkImage;
	CString strBkImagePath;
	ColorRuleArray aColorRules;
	TabViewOption();
	TabViewOption(const TabViewOption& tvo);
	//CArray의 CArray를 만들때는 항상 복사 생성자를 오버로딩 해야 함
	void operator= (const TabViewOption& tvo) {	CopyTabViewOption(tvo);	};
	void CopyTabViewOption(const TabViewOption& tvo);
	CString StringExport();
	void StringImport(CString strData);
};
typedef CArray<TabViewOption> TabViewOptionArray;
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

public:
	void INISave(CString strFile);
	void INILoad(CString strFile);
	DECLARE_MESSAGE_MAP()
	virtual BOOL InitInstance();
	virtual int ExitInstance();
};
inline CFileOfficerApp* APP() { return (CFileOfficerApp*)AfxGetApp(); };
extern CFileOfficerApp theApp;
