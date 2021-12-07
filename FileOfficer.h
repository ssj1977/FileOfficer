
// FileOfficer.h: PROJECT_NAME 애플리케이션에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.

#ifndef PathTabInfo
struct PathTabInfo
{
	CWnd* pWnd;
	CString strPath;
	int iSortColumn;
	BOOL bSortAscend;
	PathTabInfo()
	{
		pWnd = NULL;
		iSortColumn = 0;
		bSortAscend = TRUE;
	};
	PathTabInfo(CString strPath, int iSortColumn, BOOL bSortAscend)
	{
		SetTabInfo(strPath, iSortColumn, bSortAscend);
		pWnd = NULL;
	};
	void SetTabInfo(CString strPath, int iSortColumn, BOOL bSortAscend)
	{
		this->strPath = strPath;
		this->iSortColumn = iSortColumn;
		this->bSortAscend = bSortAscend;
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
public:
	void INISave(CString strFile);
	void INILoad(CString strFile);
	DECLARE_MESSAGE_MAP()
	virtual BOOL InitInstance();
	virtual int ExitInstance();
};
inline CFileOfficerApp* APP() { return (CFileOfficerApp*)AfxGetApp(); };
extern CFileOfficerApp theApp;
