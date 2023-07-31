
// FileOfficer.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "FileOfficer.h"
#include "FileOfficerDlg.h"
#include <CommonControls.h>
#include "EtcFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFileOfficerApp

BEGIN_MESSAGE_MAP(CFileOfficerApp, CWinApp)
	//ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CFileOfficerApp 생성

CFileOfficerApp::CFileOfficerApp()
{
	m_pSysImgList_SMALL = NULL;
	m_pSysImgList_LARGE = NULL;
	m_pSysImgList_EXTRALARGE = NULL;
	m_pSysImgList_JUMBO = NULL;
	m_nSortCol_Default = 0;
	m_bSortAscend_Default = TRUE;
	m_strPath_Default.Empty();
	m_rcMain = CRect(0, 0, 0, 0);
	m_nCurrentTab1 = 0;
	m_nCurrentTab2 = 0;
	m_nFocus = 1;
	m_nLayoutType = 0;
	m_nLayoutSizeType = 0;
	m_nLayoutSizePercent = 50;
	m_nLayoutSizeFixed1 = 600;
	//m_nLayoutSizeFixed2 = 600;
	m_bUseFileType = FALSE;
	//m_bToolBarText = TRUE;
	m_nToolBarButtonSize = 20;
	m_bToolBarVertical = FALSE;
	m_hIcon = NULL;
	m_bUseFileIcon = TRUE;
	m_bUseFileType = TRUE;
	m_nListType = TABTYPE_SHELL_LIST;
}


// 유일한 CFileOfficerApp 개체입니다.

CFileOfficerApp theApp;


// CFileOfficerApp 초기화

BOOL CFileOfficerApp::InitInstance()
{
	TCHAR szBuff[MY_MAX_PATH];
	GetModuleFileName(m_hInstance, szBuff, MY_MAX_PATH);
	CString strExePath = szBuff;
	m_strINIPath = Get_Folder(strExePath, TRUE) + Get_Name(strExePath, FALSE) + L".ini";
	INILoad(m_strINIPath);
	m_hIcon = LoadIcon(IDR_MAINFRAME);
	//if (m_bEnglishUI == TRUE) SetLocale(LANG_ENGLISH);

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinApp::InitInstance();
	CShellManager *pShellManager = new CShellManager;
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
	if (!AfxOleInit())
	{
		AfxMessageBox(L"AfxOleInit Error!");
		return FALSE;
	}

	CFileOfficerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 여기에 [확인]을 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 여기에 [취소]를 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "경고: 대화 상자를 만들지 못했으므로 애플리케이션이 예기치 않게 종료됩니다.\n");
		TRACE(traceAppMsg, 0, "경고: 대화 상자에서 MFC 컨트롤을 사용하는 경우 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS를 수행할 수 없습니다.\n");
	}
	// 위에서 만든 셸 관리자를 삭제합니다.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif
	// 대화 상자가 닫혔으므로 응용 프로그램의 메시지 펌프를 시작하지 않고  응용 프로그램을 끝낼 수 있도록 FALSE를
	// 반환합니다.
	return FALSE;
}

HIMAGELIST* CFileOfficerApp::GetImageListByType(int nIconType)
{
	switch (nIconType)
	{
	case SHIL_SMALL:		
		if (m_pSysImgList_SMALL == NULL) SHGetImageList(nIconType, IID_IImageList, (void**)&m_pSysImgList_SMALL);
		return m_pSysImgList_SMALL;
	case SHIL_LARGE:
		if (m_pSysImgList_LARGE == NULL) SHGetImageList(nIconType, IID_IImageList, (void**)&m_pSysImgList_LARGE);
		return m_pSysImgList_LARGE;
	case SHIL_EXTRALARGE:
		if (m_pSysImgList_EXTRALARGE == NULL) SHGetImageList(nIconType, IID_IImageList, (void**)&m_pSysImgList_EXTRALARGE);
		return m_pSysImgList_EXTRALARGE;
	case SHIL_JUMBO:
		if (m_pSysImgList_JUMBO == NULL) SHGetImageList(nIconType, IID_IImageList, (void**)&m_pSysImgList_JUMBO);
		return m_pSysImgList_JUMBO;
	}
	return NULL;
}


int CFileOfficerApp::ExitInstance()
{
	//if (CMFCVisualManager::GetInstance() != NULL) delete CMFCVisualManager::GetInstance();
	INISave(m_strINIPath);
	CoUninitialize();
	return CWinApp::ExitInstance();
}

static CRect ConvertString2Rect(CString& str)
{
	CRect rc;
	CString strValue;
	int i = 0, nVal = 0;
	while (AfxExtractSubString(strValue, str, i, L','))
	{
		nVal = _ttoi(strValue);
		if (i == 0) rc.left = nVal;
		else if (i == 1) rc.top = nVal;
		else if (i == 2) rc.right = nVal;
		else if (i == 3) rc.bottom = nVal;
		i++;
	}
	return rc;
}

static CString UIntArray2String(CUIntArray& array)
{
	CString str;
	for (int i = 0; i < array.GetSize(); i++)
	{
		if (str.IsEmpty() == FALSE) str += L',';
		str += INTtoSTR((int)array[i]);
	}
	return str;
}

static void String2UIntArray(CString& str, CUIntArray& array)
{
	array.RemoveAll();
	if (str.IsEmpty()) return;
	CString strToken;
	int i = 0;
	while (AfxExtractSubString(strToken, str, i, L','))
	{
		array.Add((UINT)_ttoi(strToken));
		i++;
	}
}

void CFileOfficerApp::INISave(CString strFile)
{
	CString strData, strLine, str1, str2;
	if (m_rcMain.IsRectEmpty() == FALSE)
	{
		strLine.Format(_T("RectMain=%d,%d,%d,%d\r\n"), m_rcMain.left, m_rcMain.top, m_rcMain.right, m_rcMain.bottom);
		strData += strLine;;
	}
	strLine.Format(_T("UseFileType=%d\r\n"), m_bUseFileType);	strData += strLine;
	strLine.Format(_T("UseFileIcon=%d\r\n"), m_bUseFileIcon);	strData += strLine;
	strLine.Format(_T("CurrentTab1=%d\r\n"), m_nCurrentTab1);	strData += strLine;
	strLine.Format(_T("CurrentTab2=%d\r\n"), m_nCurrentTab2);	strData += strLine;
	strLine.Format(_T("Focused=%d\r\n"), m_nFocus);	strData += strLine;
	strLine.Format(_T("LayoutType=%d\r\n"), m_nLayoutType); strData += strLine;
	strLine.Format(_T("LayoutSizeType=%d\r\n"), m_nLayoutSizeType); strData += strLine;
	strLine.Format(_T("LayoutSizePercent=%d\r\n"), m_nLayoutSizePercent); strData += strLine;
	strLine.Format(_T("LayoutSizeFixed1=%d\r\n"), m_nLayoutSizeFixed1); strData += strLine;
	//strLine.Format(_T("LayoutSizeFixed2=%d\r\n"), m_nLayoutSizeFixed2); strData += strLine;
	//strLine.Format(_T("ToolBarText=%d\r\n"), m_bToolBarText); strData += strLine;
	strLine.Format(_T("ToolBarButtonSize=%d\r\n"), m_nToolBarButtonSize); strData += strLine;
	strLine.Format(_T("ToolBarVertical=%d\r\n"), m_bToolBarVertical); strData += strLine;
	//탭뷰설정값 저장
	for (int i = 0; i < m_aTabViewOption.GetSize(); i++)
	{
		strData += m_aTabViewOption.GetAt(i).StringExport();;
	}
	//열려있는 탭 정보1
	for (int i = 0; i < m_aTab1.GetSize(); i++)
	{
		strLine.Format(_T("Tab1_Path=%s\r\n"), (LPCTSTR)m_aTab1[i].strPath);	strData += strLine;
		strLine.Format(_T("Tab1_CtrlType=%d\r\n"), m_aTab1[i].nCtrlType); strData += strLine;
		strLine.Format(_T("Tab1_SortCol=%d\r\n"), m_aTab1[i].iSortColumn);	strData += strLine;
		strLine.Format(_T("Tab1_SortAscend=%d\r\n"), m_aTab1[i].bSortAscend); strData += strLine;
		strLine.Format(_T("Tab1_ColWidths=%s\r\n"), UIntArray2String(m_aTab1[i].aColWidth)); strData += strLine;
	}
	//열려있는 탭 정보2
	for (int i = 0; i < m_aTab2.GetSize(); i++)
	{
		strLine.Format(_T("Tab2_Path=%s\r\n"), (LPCTSTR)m_aTab2[i].strPath);	strData += strLine;
		strLine.Format(_T("Tab2_CtrlType=%d\r\n"), m_aTab2[i].nCtrlType); strData += strLine;
		strLine.Format(_T("Tab2_SortCol=%d\r\n"), m_aTab2[i].iSortColumn);	strData += strLine;
		strLine.Format(_T("Tab2_SortAscend=%d\r\n"), m_aTab2[i].bSortAscend);	strData += strLine;
		strLine.Format(_T("Tab2_ColWidths=%s\r\n"), UIntArray2String(m_aTab2[i].aColWidth)); strData += strLine;
	}
	WriteCStringToFile(strFile, strData);
}


void CFileOfficerApp::INILoad(CString strFile)
{
	CString strData, strLine, str1, str2, strTemp;
	m_aTabViewOption.RemoveAll();
	ReadFileToCString(strFile, strData);
	int nPos = 0;
	int nTabCount1 = -1, nTabCount2 = -1, nCRCount1 = -1, nCRCount2 = -1;
	CStringArray aTabViewOptionString;
	int nTabViewOptionIndex = -1;
	while (nPos != -1)
	{
		nPos = GetLine(strData, nPos, strLine, _T("\r\n"));
		GetToken(strLine, str1, str2, _T('='), FALSE);
		if (str1.CompareNoCase(_T("RectMain")) == 0) m_rcMain = ConvertString2Rect(str2);
		else if (str1.CompareNoCase(_T("UseFileType")) == 0) m_bUseFileType = _ttoi(str2);
		else if (str1.CompareNoCase(_T("UseFileIcon")) == 0) m_bUseFileIcon = _ttoi(str2);
		else if (str1.CompareNoCase(_T("CurrentTab1")) == 0) m_nCurrentTab1 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("CurrentTab2")) == 0) m_nCurrentTab2 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Focused")) == 0) m_nFocus = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutType")) == 0) m_nLayoutType = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutSizeType")) == 0) m_nLayoutSizeType = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutSizePercent")) == 0) m_nLayoutSizePercent = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutSizeFixed1")) == 0) m_nLayoutSizeFixed1 = _ttoi(str2);
		//else if (str1.CompareNoCase(_T("LayoutSizeFixed2")) == 0) m_nLayoutSizeFixed2 = _ttoi(str2);
		//else if (str1.CompareNoCase(_T("ToolBarText")) == 0) m_bToolBarText = _ttoi(str2);
		else if (str1.CompareNoCase(_T("ToolBarButtonSize")) == 0) m_nToolBarButtonSize = _ttoi(str2);
		else if (str1.CompareNoCase(_T("ToolBarVertical")) == 0) m_bToolBarVertical = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab1_Path")) == 0)	nTabCount1 = (int)m_aTab1.Add(PathTabInfo(PathBackSlash(str2, FALSE), 0, TRUE));
		else if (str1.CompareNoCase(_T("Tab1_CtrlType")) == 0 && nTabCount1 != -1) m_aTab1[nTabCount1].nCtrlType = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab1_SortCol")) == 0 && nTabCount1 != -1) m_aTab1[nTabCount1].iSortColumn = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab1_SortAscend")) == 0 && nTabCount1 != -1) m_aTab1[nTabCount1].bSortAscend = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab1_ColWidths")) == 0 && nTabCount1 != -1) String2UIntArray(str2, m_aTab1[nTabCount1].aColWidth);
		else if (str1.CompareNoCase(_T("Tab2_Path")) == 0) nTabCount2 = (int)m_aTab2.Add(PathTabInfo(str2, 0, TRUE));
		else if (str1.CompareNoCase(_T("Tab2_CtrlType")) == 0 && nTabCount2 != -1) m_aTab2[nTabCount2].nCtrlType = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab2_SortCol")) == 0 && nTabCount2 != -1) m_aTab2[nTabCount2].iSortColumn = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab2_SortAscend")) == 0 && nTabCount2 != -1) m_aTab2[nTabCount2].bSortAscend = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab2_ColWidths")) == 0 && nTabCount2 != -1) String2UIntArray(str2, m_aTab2[nTabCount2].aColWidth);
		else if (str1.CompareNoCase(_T("TabViewOption")) == 0)
		{   //사후 처리를 위해 배열에 저장(항목추가)
			nTabViewOptionIndex = (int)aTabViewOptionString.Add(strLine); 
		}
		else if (str1.Find(_T("TVO_")) == 0 && nTabViewOptionIndex != -1) 
		{	// TVO_로 시작하는 부분을 모두 모아서 더함
			aTabViewOptionString.SetAt(nTabViewOptionIndex, aTabViewOptionString.GetAt(nTabViewOptionIndex) + _T("\r\n") + strLine);
		}
	}
	//추출한 탭뷰설정값 저장 부분을 사후 처리 
	for (int i = 0; i < aTabViewOptionString.GetSize(); i++)
	{
		TabViewOption tvo;
		tvo.StringImport(aTabViewOptionString.GetAt(i));
		m_aTabViewOption.Add(tvo);
	}
}

////////////////////////////////////////////
//ColorRule 구조체 구현
////////////////////////////////////////////
ColorRule::ColorRule()
{
	nRuleType = 0;
	clrText = RGB(130, 180, 255);
	clrBk = RGB(0, 0, 0);
	bClrText = TRUE;
	bClrBk = FALSE;
}
ColorRule::ColorRule(const ColorRule& cr)
{
	CopyColorRule(cr);
}
void ColorRule::CopyColorRule(const ColorRule& cr)
{
	this->nRuleType = cr.nRuleType;
	this->strRuleOption = cr.strRuleOption;
	this->clrText = cr.clrText;
	this->clrBk = cr.clrBk;
	this->bClrText = cr.bClrText;
	this->bClrBk = cr.bClrBk;
	this->aRuleOptions.RemoveAll();
	this->aRuleOptions.Copy(cr.aRuleOptions);
}
void ColorRule::ParseRuleOption()
{
	aRuleOptions.RemoveAll();
	CString strToken;
	int nPos = 0, nNextPos = 0;
	int nCount = strRuleOption.GetLength();
	while (nPos < nCount)
	{
		nNextPos = strRuleOption.Find(L"/", nPos);
		if (nNextPos == -1) nNextPos = strRuleOption.GetLength();
		strToken = strRuleOption.Mid(nPos, nNextPos - nPos);
		strToken.Trim();
		nPos = nNextPos + 1;
		aRuleOptions.Add(strToken);
	}
}

CString ColorRule::StringExport()
{
	CString strData, strLine;
	strLine.Format(_T("TVO_ColorRule=%d,%d,%d,%d,%d\r\n"), nRuleType, bClrText, clrText, bClrBk, clrBk); strData += strLine;
	if (strRuleOption.IsEmpty() == FALSE)
	{
		strLine.Format(_T("TVO_ColorRuleOption=%s\r\n"), (LPCTSTR)strRuleOption); 
		strData += strLine;
	}
	return strData;
}

void ColorRule::StringImport(CString strData)
{
	CString strLine, str1, str2, strTemp;
	int nPos = 0;
	while (nPos != -1)
	{
		nPos = GetLine(strData, nPos, strLine, _T("\r\n"));
		GetToken(strLine, str1, str2, _T('='), FALSE);
		if (str1.CompareNoCase(_T("TVO_ColorRule")) == 0)
		{
			ColorRule cr;
			CString strValue;
			int i = 0, nVal = 0;
			while (AfxExtractSubString(strValue, str2, i, L','))
			{
				nVal = _ttoi(strValue);
				if (i == 0) nRuleType = nVal;
				else if (i == 1) bClrText = nVal;
				else if (i == 2) clrText = nVal;
				else if (i == 3) bClrBk = nVal;
				else if (i == 4) clrBk = nVal;
				i++;
			}
		}
		else if (str1.CompareNoCase(_T("TVO_ColorRuleOption")) == 0) strRuleOption = str2;
	}
	ParseRuleOption();
}

////////////////////////////////////////////
//TableViewOption 구조체 구현
////////////////////////////////////////////
TabViewOption::TabViewOption()
{
	clrText = RGB(0, 0, 0);
	clrBk = RGB(255, 255, 255);
	nFontSize = 11;
	nIconType = SHIL_SMALL;
	bUseDefaultColor = TRUE;
	bUseDefaultFont = TRUE;
	bUseBkImage = FALSE;
	nFontWeight = FW_NORMAL;
	bFontItalic = FALSE;
}
TabViewOption::TabViewOption(const TabViewOption& tvo)
{
	CopyTabViewOption(tvo);
}
void TabViewOption::CopyTabViewOption(const TabViewOption& tvo)
{
	this->clrText = tvo.clrText;
	this->clrBk = tvo.clrBk;
	this->nIconType = tvo.nIconType;
	this->nFontSize = tvo.nFontSize;
	this->nFontWeight = tvo.nFontWeight;
	this->bFontItalic = tvo.bFontItalic;
	this->strFontName = tvo.strFontName;
	this->bUseDefaultColor = tvo.bUseDefaultColor;
	this->bUseDefaultFont = tvo.bUseDefaultFont;
	this->bUseBkImage = tvo.bUseBkImage;
	this->strBkImagePath = tvo.strBkImagePath;
	this->aColorRules.Copy(tvo.aColorRules);
}
CString TabViewOption::StringExport()
{
	CString strData, strLine;
	strLine.Format(_T("TabViewOption=%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
		clrText, clrBk, nIconType, nFontSize, nFontWeight,
		bUseDefaultColor, bUseDefaultFont, bUseBkImage, bFontItalic);
	strData += strLine;
	if (strBkImagePath.IsEmpty() == FALSE)
	{
		strLine.Format(_T("TVO_BkImgPath=%s\r\n"), strBkImagePath);
		strData += strLine;
	}
	if (strFontName.IsEmpty() == FALSE)
	{
		strLine.Format(_T("TVO_FontName=%s\r\n"), strFontName);
		strData += strLine;
	}
	for (int i = 0; i < aColorRules.GetSize(); i++)
	{
		strData += aColorRules.GetAt(i).StringExport();
	}
	return strData;
}
void TabViewOption::StringImport(CString strData)
{
	CString strLine, str1, str2, strTemp;
	int nPos = 0;
	int nIndex = -1;
	CStringArray aColorRuleString;
	aColorRules.RemoveAll();
	while (nPos != -1)
	{
		nPos = GetLine(strData, nPos, strLine, _T("\r\n"));
		GetToken(strLine, str1, str2, _T('='), FALSE);
		if (str1.CompareNoCase(_T("TabViewOption")) == 0)
		{
			CString strValue;
			int i = 0, nVal = 0;
			while (AfxExtractSubString(strValue, str2, i, L','))
			{
				nVal = _ttoi(strValue);
				if (i == 0) clrText = nVal;
				else if (i == 1) clrBk = nVal;
				else if (i == 2) nIconType = nVal;
				else if (i == 3) nFontSize = nVal;
				else if (i == 4) nFontWeight = nVal;
				else if (i == 5) bUseDefaultColor = nVal;
				else if (i == 6) bUseDefaultFont = nVal;
				else if (i == 7) bUseBkImage = nVal;
				else if (i == 8) bFontItalic = nVal;
				i++;
			}
		}
		else if (str1.CompareNoCase(_T("TVO_FontName")) == 0) strFontName = str2;
		else if (str1.CompareNoCase(_T("TVO_BkImgPath")) == 0) strBkImagePath = str2;
		else if (str1.CompareNoCase(_T("TVO_ColorRule")) == 0) nIndex = (int)aColorRuleString.Add(strLine);
		else if (str1.CompareNoCase(_T("TVO_ColorRuleOption")) == 0 && nIndex != -1 && nIndex < aColorRuleString.GetSize())
		{
			aColorRuleString.SetAt(nIndex, aColorRuleString.GetAt(nIndex) + _T("\r\n") + strLine);
		}
	}
	for (int i = 0; i < aColorRuleString.GetSize(); i++)
	{
		ColorRule cr;
		cr.StringImport(aColorRuleString.GetAt(i));
		aColorRules.Add(cr);
	}
}