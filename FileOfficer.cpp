
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
	m_nIconType = SHIL_EXTRALARGE;
	m_pSysImgList = NULL;
	m_clrDefault_Bk = RGB(255,255,2555);
	m_clrDefault_Text = RGB(0, 0, 0);
	m_nSortCol_Default = 0;
	m_bSortAscend_Default = TRUE;
	m_strPath_Default.Empty();
	m_rcMain = CRect(0, 0, 0, 0);
	m_nCurrentTab1 = 0;
	m_nCurrentTab2 = 0;
	m_nFocus = 1;
	m_nViewMode = 0;

	m_clrText = RGB(130, 180, 255);;
	m_clrBk = RGB(0, 0, 0);
	m_bUseDefaultColor = TRUE;
	m_nFontSize = 12;
	m_bUseDefaultFont = TRUE;
	m_bBold = FALSE;
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
	LoadImageList(m_nIconType);

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

void CFileOfficerApp::LoadImageList(int nIconType)
{
	m_nIconType = nIconType;
	HRESULT hr = SHGetImageList(m_nIconType, IID_IImageList, (void**)&m_pSysImgList);
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
	strLine.Format(_T("MainRectLeft=%d\r\n"), m_rcMain.left);		strData += strLine;
	strLine.Format(_T("MainRectTop=%d\r\n"), m_rcMain.top);		strData += strLine;
	strLine.Format(_T("MainRectRight=%d\r\n"), m_rcMain.right);	strData += strLine;
	strLine.Format(_T("MainRectBottom=%d\r\n"), m_rcMain.bottom);	strData += strLine;
	strLine.Format(_T("CurrentTab1=%d\r\n"), m_nCurrentTab1);	strData += strLine;
	strLine.Format(_T("CurrentTab2=%d\r\n"), m_nCurrentTab2);	strData += strLine;
	strLine.Format(_T("Focused=%d\r\n"), m_nFocus);	strData += strLine;
	strLine.Format(_T("ViewMode=%d\r\n"), m_nViewMode); strData += strLine;
	strLine.Format(_T("UseDefaultColor=%d\r\n"), m_bUseDefaultColor);	strData += strLine;
	strLine.Format(_T("ColorBk=%d\r\n"), m_clrBk);	strData += strLine;
	strLine.Format(_T("ColorText=%d\r\n"), m_clrText);	strData += strLine;
	strLine.Format(_T("UseDefaultFont=%d\r\n"), m_bUseDefaultFont);	strData += strLine;
	strLine.Format(_T("UseBoldFont=%d\r\n"), m_bBold);	strData += strLine;
	strLine.Format(_T("FontSize=%d\r\n"), m_nFontSize);	strData += strLine;
	strLine.Format(_T("IconType=%d\r\n"), m_nIconType);	strData += strLine;

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


void CFileOfficerApp::INILoad(CString strFile)
{
	CString strData, strLine, str1, str2, strTemp;
	//m_aTabInfo.RemoveAll();
	ReadFileToCString(strFile, strData);
	int nPos = 0;
	int nTabCount = -1;
	while (nPos != -1)
	{
		nPos = GetLine(strData, nPos, strLine, _T("\r\n"));
		GetToken(strLine, str1, str2, _T('='), FALSE);
		if (str1.CompareNoCase(_T("MainRectLeft")) == 0) m_rcMain.left = _ttoi(str2);
		else if (str1.CompareNoCase(_T("MainRectTop")) == 0) m_rcMain.top = _ttoi(str2);
		else if (str1.CompareNoCase(_T("MainRectRight")) == 0) m_rcMain.right = _ttoi(str2);
		else if (str1.CompareNoCase(_T("MainRectBottom")) == 0) m_rcMain.bottom = _ttoi(str2);
		else if (str1.CompareNoCase(_T("CurrentTab1")) == 0) m_nCurrentTab1 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("CurrentTab2")) == 0) m_nCurrentTab2 = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Focused")) == 0) m_nFocus = _ttoi(str2);
		else if (str1.CompareNoCase(_T("ViewMode")) == 0) m_nViewMode = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab1_Path")) == 0) nTabCount = (int)m_aTab1.Add(PathTabInfo(str2, 0, TRUE));
		else if (str1.CompareNoCase(_T("Tab1_SortCol")) == 0 && nTabCount != -1) m_aTab1[nTabCount].iSortColumn = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab1_SortAscend")) == 0 && nTabCount != -1) m_aTab1[nTabCount].bSortAscend = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab2_Path")) == 0) nTabCount = (int)m_aTab2.Add(PathTabInfo(str2, 0, TRUE));
		else if (str1.CompareNoCase(_T("Tab2_SortCol")) == 0 && nTabCount != -1) m_aTab2[nTabCount].iSortColumn = _ttoi(str2);
		else if (str1.CompareNoCase(_T("Tab2_SortAscend")) == 0 && nTabCount != -1) m_aTab2[nTabCount].bSortAscend = _ttoi(str2);
		else if (str1.CompareNoCase(_T("UseDefaultColor")) == 0) m_bUseDefaultColor = _ttoi(str2);
		else if (str1.CompareNoCase(_T("ColorBk")) == 0) m_clrBk = _ttoi(str2);
		else if (str1.CompareNoCase(_T("ColorText")) == 0) m_clrText = _ttoi(str2);
		else if (str1.CompareNoCase(_T("UseDefaultFont")) == 0) m_bUseDefaultFont = _ttoi(str2);
		else if (str1.CompareNoCase(_T("UseBoldFont")) == 0) m_bBold = _ttoi(str2);
		else if (str1.CompareNoCase(_T("FontSize")) == 0) m_nFontSize = _ttoi(str2);
		else if (str1.CompareNoCase(_T("IconType")) == 0) m_nIconType = _ttoi(str2);
	}
}

COLORREF CFileOfficerApp::GetMyClrText()
{
	return m_bUseDefaultColor ? m_clrDefault_Text : m_clrText;
}

COLORREF CFileOfficerApp::GetMyClrBk()
{
	// TODO: 여기에 구현 코드 추가.
	return m_bUseDefaultColor ? m_clrDefault_Bk : m_clrBk;
}
