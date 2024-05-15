
// FileOfficer.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "FileOfficer.h"
#include "FileOfficerDlg.h"
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
	m_nSortCol_Default = 0;
	m_bSortAscend_Default = TRUE;
	m_strPath_Default.Empty();
	m_rcMain = CRect(10, 10, 780, 580);
	m_rcSearch.SetRectEmpty();
	m_nCurrentTab1 = 0;
	m_nCurrentTab2 = 0;
	m_nFocus = 1;
	m_nLayoutType = 0;
	m_nLayoutSizeType = 0;
	m_nLayoutSizePercent = 50;
	m_nLayoutSizeFixed = 300;
	m_bViewShortCut1 = TRUE;
	m_bViewShortCut2 = TRUE;
	m_nDragBarPos1 = 100;
	m_nDragBarPos2 = 100;
	m_nToolBarButtonSize = 20;
	m_bToolBarVertical = FALSE;
	m_hIcon = NULL;
	m_bUseFileIcon = TRUE;
	m_bUseFileType = FALSE;
	m_nShortCutViewType1 = LVS_ICON;
	m_nShortCutViewType2 = LVS_ICON;
	m_nShortCutIconType1 = SHIL_EXTRALARGE;
	m_nShortCutIconType2 = SHIL_EXTRALARGE;
}


// 유일한 CFileOfficerApp 개체입니다.

CFileOfficerApp theApp;


// CFileOfficerApp 초기화

BOOL CFileOfficerApp::InitInstance()
{
	TCHAR* pBuf = new TCHAR[MY_MAX_PATH];
	GetModuleFileName(m_hInstance, pBuf, MY_MAX_PATH);
	CString strExePath = (LPCTSTR)pBuf;
	delete[] pBuf;
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
	if (S_OK != CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY))
	{
		AfxMessageBox(L"CoInitializeEx Error!");
		return FALSE;
	}
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

void CFileOfficerApp::ClearPreviousCutItems()
{
	ListItemArray& aCut = APP()->m_aCutItem;
	CListCtrl* pList = NULL;
	for (int i = 0; i < aCut.GetCount(); i++)
	{
		pList = aCut[i].pList;
		if (pList != NULL && ::IsWindow(pList->GetSafeHwnd()) != FALSE)
		{
			int nItem = aCut[i].nIndex;
			if (nItem >= 0 && nItem < pList->GetItemCount())
			{
				pList->SetItemState(aCut[i].nIndex, 0, LVIS_CUT);
			}
		}
	}
	aCut.RemoveAll();
}

void CFileOfficerApp::INISave(CString strFile)
{
	CString strData, strLine, str1, str2;
	if (m_rcMain.IsRectEmpty() == FALSE)
	{
		strLine.Format(_T("RectMain=%d,%d,%d,%d\r\n"), m_rcMain.left, m_rcMain.top, m_rcMain.right, m_rcMain.bottom);
		strData += strLine;;
	}
	if (m_rcSearch.IsRectEmpty() == FALSE)
	{
		strLine.Format(_T("RectSearch=%d,%d,%d,%d\r\n"), m_rcSearch.left, m_rcSearch.top, m_rcSearch.right, m_rcSearch.bottom);
		strData += strLine;;
	}
	strLine.Format(_T("UseFileType=%d\r\n"), m_bUseFileType);	strData += strLine;
	strLine.Format(_T("UseFileIcon=%d\r\n"), m_bUseFileIcon);	strData += strLine;
	strLine.Format(_T("CheckOpen=%d\r\n"), m_bCheckOpen);	strData += strLine;
	strLine.Format(_T("CurrentTab1=%d\r\n"), m_nCurrentTab1);	strData += strLine;
	strLine.Format(_T("CurrentTab2=%d\r\n"), m_nCurrentTab2);	strData += strLine;
	strLine.Format(_T("Focused=%d\r\n"), m_nFocus);	strData += strLine;
	strLine.Format(_T("LayoutType=%d\r\n"), m_nLayoutType); strData += strLine;
	strLine.Format(_T("LayoutSizeType=%d\r\n"), m_nLayoutSizeType); strData += strLine;
	strLine.Format(_T("LayoutSizePercent=%d\r\n"), m_nLayoutSizePercent); strData += strLine;
	strLine.Format(_T("LayoutSizeFixed=%d\r\n"), m_nLayoutSizeFixed); strData += strLine;
	strLine.Format(_T("LayoutSizeDynamic=%d\r\n"), m_nLayoutSizeDynamic); strData += strLine;
	strLine.Format(_T("ViewShortCut1=%d\r\n"), m_bViewShortCut1); strData += strLine;
	strLine.Format(_T("ViewShortCut2=%d\r\n"), m_bViewShortCut2); strData += strLine;
	strLine.Format(_T("ShortCutViewType1=%d\r\n"), m_nShortCutViewType1); strData += strLine;
	strLine.Format(_T("ShortCutViewType2=%d\r\n"), m_nShortCutViewType2); strData += strLine;
	strLine.Format(_T("ShortCutIconType1=%d\r\n"), m_nShortCutIconType1); strData += strLine;
	strLine.Format(_T("ShortCutIconType2=%d\r\n"), m_nShortCutIconType2); strData += strLine;
	strLine.Format(_T("DragBarPos1=%d\r\n"), m_nDragBarPos1); strData += strLine;
	strLine.Format(_T("DragBarPos2=%d\r\n"), m_nDragBarPos2); strData += strLine;
	strLine.Format(_T("ToolBarButtonSize=%d\r\n"), m_nToolBarButtonSize); strData += strLine;
	strLine.Format(_T("ToolBarVertical=%d\r\n"), m_bToolBarVertical); strData += strLine;
	//검색창 조건값 저장
	strData += m_defaultSC.ExportString();
	//탭뷰설정값 저장
	for (int i = 0; i < m_aTabViewOption.GetSize(); i++)
	{
		strData += m_aTabViewOption.GetAt(i).StringExport();;
	}
	//열려있는 탭 정보1
	for (int i = 0; i < m_aTab1.GetSize(); i++)
	{
		strLine.Format(_T("Tab1_Path=%s\r\n"), (LPCTSTR)m_aTab1[i].strPath);	strData += strLine;
		strLine.Format(_T("Tab1_SortCol=%d\r\n"), m_aTab1[i].iSortColumn);	strData += strLine;
		strLine.Format(_T("Tab1_SortAscend=%d\r\n"), m_aTab1[i].bSortAscend); strData += strLine;
		strLine.Format(_T("Tab1_ColWidths=%s\r\n"), UIntArray2String(m_aTab1[i].aColWidth)); strData += strLine;
	}
	//열려있는 탭 정보2
	for (int i = 0; i < m_aTab2.GetSize(); i++)
	{
		strLine.Format(_T("Tab2_Path=%s\r\n"), (LPCTSTR)m_aTab2[i].strPath);	strData += strLine;
		strLine.Format(_T("Tab2_SortCol=%d\r\n"), m_aTab2[i].iSortColumn);	strData += strLine;
		strLine.Format(_T("Tab2_SortAscend=%d\r\n"), m_aTab2[i].bSortAscend);	strData += strLine;
		strLine.Format(_T("Tab2_ColWidths=%s\r\n"), UIntArray2String(m_aTab2[i].aColWidth)); strData += strLine;
	}
	//숏컷 정보
	for (int i = 0; i < m_aShortCutPath1.GetSize(); i++)
	{
		strLine.Format(_T("ShortCutPath1=%s\r\n"), m_aShortCutPath1.GetAt(i));	
		strData += strLine;
	}
	for (int i = 0; i < m_aShortCutPath2.GetSize(); i++)
	{
		strLine.Format(_T("ShortCutPath2=%s\r\n"), m_aShortCutPath2.GetAt(i));
		strData += strLine;
	}
	WriteCStringToFile(strFile, strData);
}


void CFileOfficerApp::INILoad(CString strFile)
{
	CString strData, strLine, str1, str2, strTemp;
	m_aTabViewOption.RemoveAll();
	m_aShortCutPath1.RemoveAll();
	m_aShortCutPath2.RemoveAll();
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
		else if (str1.CompareNoCase(_T("RectSearch")) == 0) m_rcSearch = ConvertString2Rect(str2);
		else if (str1.CompareNoCase(_T("UseFileType")) == 0) m_bUseFileType = _ttoi(str2);
		else if (str1.CompareNoCase(_T("UseFileIcon")) == 0) m_bUseFileIcon = _ttoi(str2);
		else if (str1.CompareNoCase(_T("CheckOpen")) == 0) m_bCheckOpen = _ttoi(str2);
		else if (str1.CompareNoCase(_T("CurrentTab1")) == 0) m_nCurrentTab1 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("CurrentTab2")) == 0) m_nCurrentTab2 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Focused")) == 0) m_nFocus = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutType")) == 0) m_nLayoutType = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutSizeType")) == 0) m_nLayoutSizeType = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutSizePercent")) == 0) m_nLayoutSizePercent = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutSizeFixed")) == 0) m_nLayoutSizeFixed = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutSizeDynamic")) == 0) m_nLayoutSizeDynamic = _ttoi(str2);
		else if (str1.CompareNoCase(_T("ViewShortCut1")) == 0) m_bViewShortCut1 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("ViewShortCut2")) == 0) m_bViewShortCut2 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("ShortCutViewType1")) == 0) m_nShortCutViewType1 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("ShortCutViewType2")) == 0) m_nShortCutViewType2 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("ShortCutIconType1")) == 0) m_nShortCutIconType1 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("ShortCutIconType2")) == 0) m_nShortCutIconType2 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("DragBarPos1")) == 0) m_nDragBarPos1 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("DragBarPos2")) == 0) m_nDragBarPos2 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("ToolBarButtonSize")) == 0) m_nToolBarButtonSize = _ttoi(str2);
		else if (str1.CompareNoCase(_T("ToolBarVertical")) == 0) m_bToolBarVertical = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab1_Path")) == 0)	nTabCount1 = (int)m_aTab1.Add(PathTabInfo(PathBackSlash(str2, FALSE), 0, TRUE));
		else if (str1.CompareNoCase(_T("Tab1_SortCol")) == 0 && nTabCount1 != -1) m_aTab1[nTabCount1].iSortColumn = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab1_SortAscend")) == 0 && nTabCount1 != -1) m_aTab1[nTabCount1].bSortAscend = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab1_ColWidths")) == 0 && nTabCount1 != -1) String2UIntArray(str2, m_aTab1[nTabCount1].aColWidth);
		else if (str1.CompareNoCase(_T("Tab2_Path")) == 0) nTabCount2 = (int)m_aTab2.Add(PathTabInfo(str2, 0, TRUE));
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
		else if (str1.CompareNoCase(_T("ShortCutPath1")) == 0)	m_aShortCutPath1.Add(str2);
		else if (str1.CompareNoCase(_T("ShortCutPath2")) == 0)	m_aShortCutPath2.Add(str2);
		//검색조건 읽어오기
		else if (str1.CompareNoCase(_T("SearchPath")) == 0)	m_defaultSC.strStartPath = str2;
		else if (str1.CompareNoCase(_T("SearchCriteria_Name")) == 0)	m_defaultSC.strName = str2;
		else if (str1.CompareNoCase(_T("SearchCriteria_NameAnd")) == 0)	m_defaultSC.bNameAnd = _ttoi(str2);
		else if (str1.CompareNoCase(_T("SearchCriteria_Ext")) == 0)		m_defaultSC.strExt = str2;
		else if (str1.CompareNoCase(_T("SearchCriteria_SizeMin")) == 0)	m_defaultSC.strSizeMin = str2;
		else if (str1.CompareNoCase(_T("SearchCriteria_SizeMax")) == 0)	m_defaultSC.strSizeMax = str2;
		else if (str1.CompareNoCase(_T("SearchCriteria_Locked")) == 0)		m_defaultSC.bLocked = _ttoi(str2);
		else if (str1.CompareNoCase(_T("SearchCriteria_Hidden")) == 0)		m_defaultSC.bHidden = _ttoi(str2);
		else if (str1.CompareNoCase(_T("SearchCriteria_Encrypted")) == 0)	m_defaultSC.bEncrypted = _ttoi(str2);
		else if (str1.CompareNoCase(_T("SearchCriteria_ReadOnly")) == 0)	m_defaultSC.bReadOnly = _ttoi(str2);
		else if (str1.CompareNoCase(_T("SearchCriteria_UseDateTimeFrom")) == 0)		m_defaultSC.bDateTimeFrom = _ttoi(str2);
		else if (str1.CompareNoCase(_T("SearchCriteria_UseDateTimeUntil")) == 0)	m_defaultSC.bDateTimeUntil = _ttoi(str2);
		else if (str1.CompareNoCase(_T("SearchCriteria_DateTimeFromString")) == 0)	m_defaultSC.strDateTimeFrom = str2;
		else if (str1.CompareNoCase(_T("SearchCriteria_DateTimeUntilString")) == 0)	m_defaultSC.strDateTimeUntil = str2;
		else if (str1.CompareNoCase(_T("SearchCriteria_DateTimeType")) == 0)		m_defaultSC.nDateTimeType = _ttoi(str2);
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
	clrText = RGB(255, 255, 255); //반전으로
	clrBk = RGB(0, 0, 0);
	nFontSize = 12; //폰트도 크게
	nIconType = SHIL_LARGE;
	bUseDefaultColor = FALSE;
	bUseDefaultFont = FALSE;
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

void ResizeBitmap(CBitmap& bmp_src, CBitmap& bmp_dst, int dstW, int dstH)
{
	//출처: https://stackoverflow.com/questions/2770855/how-do-you-scale-a-cbitmap-object
	BITMAP bm = { 0 };
	bmp_src.GetBitmap(&bm);
	auto size = CSize(bm.bmWidth, bm.bmHeight);
	CWindowDC wndDC(NULL);
	CDC srcDC;
	srcDC.CreateCompatibleDC(&wndDC);
	auto oldSrcBmp = srcDC.SelectObject(&bmp_src);

	CDC destDC;
	destDC.CreateCompatibleDC(&wndDC);
	bmp_dst.CreateCompatibleBitmap(&wndDC, dstW, dstH);
	auto oldDestBmp = destDC.SelectObject(&bmp_dst);
	destDC.StretchBlt(0, 0, dstW, dstH, &srcDC, 0, 0, size.cx, size.cy, SRCCOPY);
}

SearchCriteria::SearchCriteria()
{
	Empty();
}

void SearchCriteria::Empty()
{
	strStartPath.Empty();
	strName.Empty();
	bNameAnd = FALSE;
	strExt.Empty();
	strSizeMax.Empty();
	strSizeMin.Empty();
	bLocked = FALSE;
	bHidden = FALSE;
	bEncrypted = FALSE;
	bReadOnly = FALSE;
	bDateTimeFrom = FALSE;
	bDateTimeUntil = FALSE;
	strDateTimeFrom.Empty();
	strDateTimeUntil.Empty();
	nDateTimeType = 0;
}

CString SearchCriteria::ExportString()
{
	CString strData, strLine;
	strLine.Format(_T("SearchPath=%s\r\n"), strStartPath);	strData += strLine;
	strLine.Format(_T("SearchCriteria_Name=%s\r\n"), strName);	strData += strLine;
	strLine.Format(_T("SearchCriteria_NameAnd=%d\r\n"), bNameAnd);	strData += strLine;
	strLine.Format(_T("SearchCriteria_Ext=%s\r\n"), strExt);	strData += strLine;
	strLine.Format(_T("SearchCriteria_SizeMin=%s\r\n"), strSizeMin);	strData += strLine;
	strLine.Format(_T("SearchCriteria_SizeMax=%s\r\n"), strSizeMax);	strData += strLine;
	strLine.Format(_T("SearchCriteria_Locked=%d\r\n"), bLocked);	strData += strLine;
	strLine.Format(_T("SearchCriteria_Hidden=%d\r\n"), bHidden);	strData += strLine;
	strLine.Format(_T("SearchCriteria_Encrypted=%d\r\n"), bEncrypted);	strData += strLine;
	strLine.Format(_T("SearchCriteria_ReadOnly=%d\r\n"), bReadOnly);	strData += strLine;
	strLine.Format(_T("SearchCriteria_UseDateTimeFrom=%d\r\n"), bDateTimeFrom);	strData += strLine;
	strLine.Format(_T("SearchCriteria_UseDateTimeUntil=%d\r\n"), bDateTimeUntil);	strData += strLine;
	strLine.Format(_T("SearchCriteria_DateTimeFromString=%s\r\n"), strDateTimeFrom);	strData += strLine;
	strLine.Format(_T("SearchCriteria_DateTimeUntilString=%s\r\n"),strDateTimeUntil);	strData += strLine;
	strLine.Format(_T("SearchCriteria_DateTimeType=%d\r\n"), nDateTimeType);	strData += strLine;
	return strData;
}

BOOL SearchCriteria::ValidateCriteriaSize()
{
	ULONGLONG sizeMin = Str2Size(strSizeMin);
	ULONGLONG sizeMax = Str2Size(strSizeMax);
	if (strSizeMin.IsEmpty() == FALSE)
	{
		if (strSizeMin.GetAt(0) != L'0' && sizeMin == 0) return FALSE;
		if (sizeMin < 0) return FALSE;
	}
	if (strSizeMax.IsEmpty() == FALSE && sizeMax < 0) return FALSE;
	if (strSizeMin.IsEmpty() == FALSE && strSizeMax.IsEmpty() == FALSE && sizeMin > sizeMax) return FALSE;
	return TRUE;
}


void SearchCriteria::CopySearchCriteria(const SearchCriteria& sc)
{
	this->bDateTimeFrom = sc.bDateTimeFrom;
	this->bDateTimeUntil = sc.bDateTimeUntil;
	this->bEncrypted = sc.bEncrypted;
	this->bHidden = sc.bHidden;
	this->bLocked = sc.bLocked;
	this->bNameAnd = sc.bNameAnd;
	this->bReadOnly = sc.bReadOnly;
	this->nDateTimeType = sc.nDateTimeType;
	this->strDateTimeFrom = sc.strDateTimeFrom;
	this->strDateTimeUntil = sc.strDateTimeUntil;
	this->strExt = sc.strExt;
	this->strName = sc.strName;
	this->strSizeMax = sc.strSizeMax;
	this->strSizeMin = sc.strSizeMin;
	this->strStartPath = sc.strStartPath;
}

BOOL SearchCriteria::ValidateCriteriaDateTime()
{
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	COleDateTime dtFrom, dtUntil;
	BOOL bRet = TRUE;
	switch (nDateTimeType)
	{
	case 0: // 날짜시간 조건 사용하지 않음
		bDateTimeFrom = FALSE;
		bDateTimeUntil = FALSE;
		break;
	case 1: // 사용자 지정 기간
		if (bDateTimeFrom == TRUE && dtFrom.ParseDateTime(strDateTimeFrom) == FALSE) bRet = FALSE;
		if (bDateTimeUntil == TRUE && dtUntil.ParseDateTime(strDateTimeUntil) == FALSE) bRet = FALSE;
		if (bDateTimeFrom == TRUE && bDateTimeUntil == TRUE && dtFrom > dtUntil) bRet = FALSE;
		break;
	case 2: //오늘
		dtFrom.SetDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0, 0, 0);
		dtUntil.SetDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 23, 59, 59);
		break;
	case 3: //이번주
		{
			int nDayOfWeek = dtNow.GetDayOfWeek();
			// Calculate the start of the week (Sunday)
			COleDateTimeSpan spanFromSunday(nDayOfWeek - 1, 0, 0, 0);
			dtFrom = dtNow - spanFromSunday;
			COleDateTimeSpan spanToEndOfWeek(6 - nDayOfWeek, 23, 59, 59);
			dtUntil = dtNow + spanToEndOfWeek;
		}
		break;
	case 4: //이번달
		{
			int nYear = dtNow.GetYear();
			int nMonth = dtNow.GetMonth();
			dtFrom.SetDateTime(nYear, nMonth, 1, 0, 0, 0);
			if (nMonth < 12)	dtUntil.SetDateTime(nYear, nMonth + 1, 1, 23, 59, 59);
			else				dtUntil.SetDateTime(nYear + 1, 1, 1, 23, 59, 59);
			COleDateTimeSpan oneday(1, 0, 0, 0);
			dtUntil = dtUntil - oneday;
		}
		break;
	case 5: //올해
		dtFrom.SetDateTime(dtNow.GetYear(), 1, 1, 0, 0, 0);
		dtUntil.SetDateTime(dtNow.GetYear(), 12, 31, 23, 59, 59);
		break;
	}
	if (bRet == TRUE && nDateTimeType > 1)
	{
		strDateTimeFrom = dtFrom.Format(_T("%Y-%m-%d %H:%M:%S"));
		strDateTimeUntil = dtUntil.Format(_T("%Y-%m-%d %H:%M:%S"));
		bDateTimeFrom = TRUE;
		bDateTimeUntil = TRUE;
	}
	return bRet;
}