﻿
// FileOfficer.h: PROJECT_NAME 애플리케이션에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.

#ifndef MY_MAX_PATH
#define MY_MAX_PATH 32768
#endif

#ifndef PathTabInfo
struct PathTabInfo
{
	CWnd* pWnd;
	CString strPath;
	LPITEMIDLIST pidl;
	int iSortColumn;
	BOOL bSortAscend;
	CUIntArray aColWidth;
	PathTabInfo()
	{
		pidl = NULL;
		pWnd = NULL;
		iSortColumn = 0;
		bSortAscend = TRUE;
	};
	~PathTabInfo()
	{
		if (pidl) CoTaskMemFree(pidl);
	}
	PathTabInfo(CString _strPath, LPITEMIDLIST _pidl, int _iSortColumn, BOOL _bSortAscend)
	{
		this->strPath = _strPath;
		this->pidl = NULL;
		if (pidl) this->pidl = ILClone(_pidl);
		this->iSortColumn = _iSortColumn;
		this->bSortAscend = _bSortAscend;
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
	PathTabInfo(const PathTabInfo& pti)
	{
		this->strPath = pti.strPath;
		this->pidl = NULL;
		if (pti.pidl) this->pidl = ILClone(pti.pidl);
		this->iSortColumn = pti.iSortColumn;
		this->bSortAscend = pti.bSortAscend;
		this->pWnd = pti.pWnd;
		this->aColWidth.RemoveAll();
		this->aColWidth.Copy(pti.aColWidth);
	}
	PathTabInfo& operator= (const PathTabInfo& pti) //CArray의 CArray를 만들때는 항상 복사 생성자를 오버로딩 해야 함
	{
		this->strPath = pti.strPath;
		if (pti.pidl) this->pidl = ILClone(pti.pidl);
		this->iSortColumn = pti.iSortColumn;
		this->bSortAscend = pti.bSortAscend;
		this->pWnd = pti.pWnd;
		this->aColWidth.RemoveAll();
		this->aColWidth.Copy(pti.aColWidth);
		return *this;
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
	int nIconType;
	BOOL bUseDefaultColor;
	BOOL bUseBkImage;
	CString strBkImagePath;
	BOOL bUseDefaultFont;
	CString strFontName;
	int nFontWeight;
	int nFontSize;
	BOOL bFontItalic;
	ColorRuleArray aColorRules;
	TabViewOption();
	TabViewOption(const TabViewOption& tvo);
	//CArray의 CArray를 만들때는 항상 복사 생성자를 오버로딩 해야 함
	TabViewOption& operator= (const TabViewOption& tvo) { CopyTabViewOption(tvo); return *this; };
	void CopyTabViewOption(const TabViewOption& tvo);
	CString StringExport();
	void StringImport(CString strData);
};
typedef CArray<TabViewOption> TabViewOptionArray;
#endif

#ifndef CListItem
struct CListItem
{
	CListItem() { pList = NULL; nIndex = -1; };
	CListItem(CListCtrl* p, int n) { pList = p; nIndex = n; };
	CListCtrl* pList;
	int nIndex;
};
typedef CArray<CListItem> ListItemArray;
#endif

#ifndef SearchCriteria
struct SearchCriteria
{
	CString strStartPath;
	int nTargetType; // 0=파일만, 1=폴더만, 2=둘다
	CString strName;
	BOOL bNameAnd;
	CString strExt;
	CString strSizeMax; //KB 등의 단위도 인식 가능하도록 문자열 사용
	CString strSizeMin; //KB 등의 단위도 인식 가능하도록 문자열 사용
	BOOL bLocked;
	BOOL bHidden;
	BOOL bEncrypted;
	BOOL bReadOnly;
	BOOL bDateTimeFrom;
	BOOL bDateTimeUntil;
	CString strDateTimeFrom;
	CString strDateTimeUntil;
	int nDateTimeType; 

	SearchCriteria();
	BOOL ValidateCriteriaSize();
	BOOL ValidateCriteriaDateTime();
	void Empty();
	CString StringExport();
	void StringImport(CString strData);
	void operator= (const SearchCriteria& sc) { CopySearchCriteria(sc); };
	void CopySearchCriteria(const SearchCriteria& sc);

};
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
	TabViewOption m_DefaultViewOption; //현재 Windows 설정상 기본 색, 글꼴 등을 저장하는 곳
	TabViewOptionArray m_aTabViewOption;

	HICON m_hIcon;
	CRect m_rcMain;
	CRect m_rcSearch;
	CString m_strINIPath;
	
	PathTabInfoArray m_aTab1;
	PathTabInfoArray m_aTab2;
	int m_nCurrentTab1, m_nCurrentTab2, m_nFocus;
	int m_nLayoutType;
	int m_nLayoutSizeType;
	int m_nLayoutSizePercent;
	int m_nLayoutSizeFixed;
	int m_nLayoutSizeDynamic;
	int m_nDragBarPos1;
	int m_nDragBarPos2;
	BOOL m_bUseFileType; //map을 이용해서 속도를 향상시켰기 때문에 옵션 불필요? 
	BOOL m_bUseFileIcon;
	BOOL m_bCheckOpen;
	int m_nToolBarButtonSize; //BOOL m_bToolBarText;
	BOOL m_bToolBarVertical;
	BOOL m_bViewShortCut1;
	BOOL m_bViewShortCut2;
	CStringArray m_aShortCutPath1;
	CStringArray m_aShortCutPath2;
	int m_nShortCutViewType1; 
	int m_nShortCutViewType2;
	int m_nShortCutIconType1;
	int m_nShortCutIconType2;
	//검색창 조건 저장용 
	SearchCriteria m_defaultSC;
	CString m_strShowPath; //폴더 열기 인자 전달용

	//선택항목 관리, CUT 기능에서 음영 처리의 일관성 유지에 필요
	ListItemArray m_aCutItem;
	void ClearPreviousCutItems();

public:
	void INISave(CString strFile);
	void INILoad(CString strFile);
	DECLARE_MESSAGE_MAP()
	virtual BOOL InitInstance();
	virtual int ExitInstance();
};
inline CFileOfficerApp* APP() { return (CFileOfficerApp*)AfxGetApp(); };
extern CFileOfficerApp theApp;
