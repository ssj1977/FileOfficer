
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
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
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
	m_nLayoutSizeFixed2 = 600;
	m_bUseFileType = FALSE;
	m_hIcon = NULL;
}


// 유일한 CFileOfficerApp 개체입니다.

CFileOfficerApp theApp;


// CFileOfficerApp 초기화

BOOL CFileOfficerApp::InitInstance()
{
	TCHAR szBuff[MAX_PATH];
	GetModuleFileName(m_hInstance, szBuff, MAX_PATH);
	CString strExePath = szBuff;
	m_strINIPath = Get_Folder(strExePath) + L"\\" + Get_Name(strExePath, FALSE) + L".ini";
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
	return CWinApp::ExitInstance();
}


void CFileOfficerApp::INISave(CString strFile)
{
	CString strData, strLine, str1, str2;
	if (m_rcMain.IsRectEmpty() == FALSE)
	{
		strLine.Format(_T("RectMain=%d,%d,%d,%d\r\n"), m_rcMain.left, m_rcMain.top, m_rcMain.right, m_rcMain.bottom);
		strData += strLine;;
	}
	strLine.Format(_T("CurrentTab1=%d\r\n"), m_nCurrentTab1);	strData += strLine;
	strLine.Format(_T("CurrentTab2=%d\r\n"), m_nCurrentTab2);	strData += strLine;
	strLine.Format(_T("Focused=%d\r\n"), m_nFocus);	strData += strLine;
	strLine.Format(_T("LayoutType=%d\r\n"), m_nLayoutType); strData += strLine;
	strLine.Format(_T("LayoutSizeType=%d\r\n"), m_nLayoutSizeType); strData += strLine;
	strLine.Format(_T("LayoutSizePercent=%d\r\n"), m_nLayoutSizePercent); strData += strLine;
	strLine.Format(_T("LayoutSizeFixed1=%d\r\n"), m_nLayoutSizeFixed1); strData += strLine;
	strLine.Format(_T("LayoutSizeFixed2=%d\r\n"), m_nLayoutSizeFixed2); strData += strLine;
	strLine.Format(_T("DefaultViewOption=%d,%d,%d,%d,%d,%d,%d\r\n"), 
		m_DefaultViewOption.clrText, m_DefaultViewOption.clrBk,
		m_DefaultViewOption.nIconType,
		m_DefaultViewOption.nFontSize, m_DefaultViewOption.bBold,
		m_DefaultViewOption.bUseDefaultColor, m_DefaultViewOption.bUseDefaultFont); strData += strLine;
	for (int i = 0; i < m_aTabViewOption.GetSize(); i++)
	{
		TabViewOption& tvo = m_aTabViewOption.GetAt(i);
		strLine.Format(_T("TabViewOption=%d,%d,%d,%d,%d,%d,%d\r\n"),
			tvo.clrText, tvo.clrBk,	tvo.nIconType, tvo.nFontSize, tvo.bBold,
			tvo.bUseDefaultColor, tvo.bUseDefaultFont); 
		strData += strLine;
	}
	for (int i = 0; i < m_aTab1.GetSize(); i++)
	{
		strLine.Format(_T("Tab1_Path=%s\r\n"), m_aTab1[i].strPath);	strData += strLine;
		strLine.Format(_T("Tab1_SortCol=%d\r\n"), m_aTab1[i].iSortColumn);	strData += strLine;
		strLine.Format(_T("Tab1_SortAscend=%d\r\n"), m_aTab1[i].bSortAscend);	strData += strLine;
	}
	for (int i = 0; i < m_aTab2.GetSize(); i++)
	{
		strLine.Format(_T("Tab2_Path=%s\r\n"), m_aTab2[i].strPath);	strData += strLine;
		strLine.Format(_T("Tab2_SortCol=%d\r\n"), m_aTab2[i].iSortColumn);	strData += strLine;
		strLine.Format(_T("Tab2_SortAscend=%d\r\n"), m_aTab2[i].bSortAscend);	strData += strLine;
	}
	WriteCStringToFile(strFile, strData);
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

static void ConvertString2ViewOption(CString& str, TabViewOption& tvo)
{
	CString strValue;
	int i = 0, nVal = 0;
	while (AfxExtractSubString(strValue, str, i, L','))
	{
		nVal = _ttoi(strValue);
		if (i == 0) tvo.clrText = nVal;
		else if (i == 1) tvo.clrBk = nVal;
		else if (i == 2) tvo.nIconType = nVal;
		else if (i == 3) tvo.nFontSize = nVal;
		else if (i == 4) tvo.bBold = nVal;
		else if (i == 5) tvo.bUseDefaultColor = nVal;
		else if (i == 6) tvo.bUseDefaultFont = nVal;
		i++;
	}
}

void CFileOfficerApp::INILoad(CString strFile)
{
	CString strData, strLine, str1, str2, strTemp;
	m_aTabViewOption.RemoveAll();
	ReadFileToCString(strFile, strData);
	int nPos = 0;
	int nTabCount = -1;
	while (nPos != -1)
	{
		nPos = GetLine(strData, nPos, strLine, _T("\r\n"));
		GetToken(strLine, str1, str2, _T('='), FALSE);
		if (str1.CompareNoCase(_T("RectMain")) == 0) m_rcMain = ConvertString2Rect(str2);
		else if (str1.CompareNoCase(_T("CurrentTab1")) == 0) m_nCurrentTab1 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("CurrentTab2")) == 0) m_nCurrentTab2 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Focused")) == 0) m_nFocus = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutType")) == 0) m_nLayoutType = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutSizeType")) == 0) m_nLayoutSizeType = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutSizePercent")) == 0) m_nLayoutSizePercent = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutSizeFixed1")) == 0) m_nLayoutSizeFixed1 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("LayoutSizeFixed2")) == 0) m_nLayoutSizeFixed2 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab1_Path")) == 0) nTabCount = (int)m_aTab1.Add(PathTabInfo(str2, 0, TRUE));
		else if (str1.CompareNoCase(_T("Tab1_SortCol")) == 0 && nTabCount != -1) m_aTab1[nTabCount].iSortColumn = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab1_SortAscend")) == 0 && nTabCount != -1) m_aTab1[nTabCount].bSortAscend = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab2_Path")) == 0) nTabCount = (int)m_aTab2.Add(PathTabInfo(str2, 0, TRUE));
		else if (str1.CompareNoCase(_T("Tab2_SortCol")) == 0 && nTabCount != -1) m_aTab2[nTabCount].iSortColumn = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab2_SortAscend")) == 0 && nTabCount != -1) m_aTab2[nTabCount].bSortAscend = _ttoi(str2);
		else if (str1.CompareNoCase(_T("DefaultViewOption")) == 0)
		{
			ConvertString2ViewOption(str2, m_DefaultViewOption);
		}
		else if (str1.CompareNoCase(_T("TabViewOption")) == 0)
		{
			TabViewOption tvo;
			ConvertString2ViewOption(str2, tvo);
			m_aTabViewOption.Add(tvo);
		}
	}
	if (m_aTabViewOption.GetSize() < 2) m_aTabViewOption.SetSize(2);
}