// CDlgTabView.cpp: 구현 파일
//

#include "pch.h"
#include "FileOfficer.h"
#include "CDlgTabView.h"
#include "afxdialogex.h"
#include "EtcFunctions.h"
#include "CFileListCtrl.h"
#include "CDlgCFG_View.h"
#include "CDlgCFG_Layout.h"

#define IDC_LIST_FILE 50000
#define IDM_UPDATE_TAB 55000
#define IDM_UPDATE_SORTINFO 55001
#define IDM_UPDATE_BAR 55002
#define IDM_OPEN_NEWTAB 55003

CString GetPathName(CString strPath);
CString GetParentFolder(CString strFolder);
CString GetActualPath(CString strPath);
// CDlgTabView 대화 상자

IMPLEMENT_DYNAMIC(CDlgTabView, CDialogEx)

CDlgTabView::CDlgTabView(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TAB_VIEW, pParent)
{
	m_nCurrentTab = 0;
	m_bSelected = FALSE;
	m_nViewOptionIndex = -1;
}

CDlgTabView::~CDlgTabView()
{
}

void CDlgTabView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PATH, m_editPath);
	DDX_Control(pDX, IDC_TAB_PATH, m_tabPath);
}


BEGIN_MESSAGE_MAP(CDlgTabView, CDialogEx)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PATH, &CDlgTabView::OnTcnSelchangeTabPath)
END_MESSAGE_MAP()


// CDlgTabView 메시지 처리기
BOOL CDlgTabView::BreakThreads()
{
	BOOL bRet = FALSE;
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			CFileListCtrl* pList = (CFileListCtrl*)m_aTabInfo[i].pWnd;;
			if (IsWindow(pList->GetSafeHwnd()))
			{
				if (CFileListCtrl::IsLoading(pList) == TRUE)
				{
					CFileListCtrl::SetLoadingStatus(pList, FALSE);
					bRet = TRUE;
				}
			}
		}
	}
	return bRet;
}

void CDlgTabView::Clear()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			CFileListCtrl* pList = (CFileListCtrl*)m_aTabInfo[i].pWnd;;
			if (IsWindow(pList->GetSafeHwnd()))
			{
				pList->ClearThread();
				pList->DestroyWindow();
			}
			delete pList;
		}
	}
}


BOOL CDlgTabView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDM_OPEN_PREV: ((CFileListCtrl*)CurrentList())->BrowsePathHistory(TRUE); break;
	case IDM_OPEN_NEXT:((CFileListCtrl*)CurrentList())->BrowsePathHistory(FALSE); break;
	case IDM_PLAY_ITEM: ((CFileListCtrl*)CurrentList())->OpenSelectedItem(); break;
	case IDM_OPEN_PARENT: ((CFileListCtrl*)CurrentList())->OpenParentFolder(); break;
	case IDM_OPEN_NEWTAB: 
		{
			CFileListCtrl* pList = (CFileListCtrl*)lParam;
			CString strPath;
			int nItem = pList->GetNextItem(-1, LVNI_SELECTED);
			while (nItem != -1)
			{
				strPath = pList->GetItemFullPath(nItem);
				if (PathIsDirectory(strPath)) AddFileListTab(strPath);
				nItem = pList->GetNextItem(nItem, LVNI_SELECTED);
			}
		}
		break;
	case IDM_UPDATE_TAB: UpdateTabByWnd((CWnd*)lParam); break;
	case IDM_UPDATE_SORTINFO: UpdateSortInfo((CWnd*)lParam); break;
	case IDM_UPDATE_BAR: SetDlgItemText(IDC_ST_BAR, ((CFileListCtrl*)lParam)->m_strBarMsg); break;
	case IDM_REFRESH_LIST: UpdateTabByPathEdit(); break;
	case IDM_SET_PATH: UpdateTabByPathEdit(); break;
	case IDM_ADD_LIST: AddFileListTab(APP()->m_strPath_Default); break;
	case IDM_CLOSE_LIST: CloseFileListTab(m_nCurrentTab); break;
	case IDM_CFG_LAYOUT: GetParent()->PostMessage(WM_COMMAND, wParam, lParam); break;
	case IDM_CONFIG: ConfigViewOption(); break;
	default:
		return CDialogEx::OnCommand(wParam, lParam);
	}
	UpdateToolBar();
	return TRUE;
}


void CDlgTabView::OnCancel() 
{	
	//CDialogEx::OnCancel();
}
void CDlgTabView::OnOK() 
{	
	//CDialogEx::OnOK();
}

void CDlgTabView::InitToolBar()
{
	m_tool.LoadToolBar(IDR_TB_TAB);
	UINT nStyle;
	int nCount = m_tool.GetCount();
	int nTextIndex = 0;
	for (int i = 0; i < nCount; i++)
	{
		nStyle = m_tool.GetButtonStyle(i);
		if (!(nStyle & TBBS_SEPARATOR))
		{
			m_tool.SetButtonText(i, IDSTR(IDS_TB_00 + nTextIndex));
			nTextIndex += 1;
		}
	}
}

BOOL CDlgTabView::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	InitFont();
	m_tool.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_WRAPABLE, WS_CHILD | WS_VISIBLE | CBRS_BORDER_ANY); // TBSTYLE_TRANSPARENT
	UpdateChildFont();
	InitToolBar();
	m_editPath.EnableFolderBrowseButton();
	m_editPath.CMD_UpdateList = IDM_REFRESH_LIST;
	m_tabImgList.Create(IDB_TABICON, 16, 2, RGB(255, 0, 255));
	//m_tabPath.SetExtendedStyle(TCS_EX_FLATSEPARATORS, TCS_EX_FLATSEPARATORS);
	m_tabPath.SetImageList(&m_tabImgList);

	m_editPath.SetBkColor(GetMyClrBk());
	m_editPath.SetTextColor(GetMyClrText());
	// Init Tabs
	if (m_aTabInfo.GetSize() == 0)
	{
		PathTabInfo tabInfo(L"", 0, TRUE);
		m_aTabInfo.Add(tabInfo);
	}
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		m_tabPath.InsertItem(i, GetPathName(m_aTabInfo[i].strPath), 1);
	}
	SetCurrentTab(m_nCurrentTab);
	ArrangeCtrl();
	return TRUE;
}

void CDlgTabView::AddFileListTab(CString strPath)
{
	PathTabInfo tabInfo(strPath, APP()->m_nSortCol_Default, APP()->m_bSortAscend_Default);
	m_aTabInfo.Add(tabInfo);
	int nTab = m_tabPath.InsertItem((int)m_aTabInfo.GetSize(), GetPathName(strPath), 1);
	SetCurrentTab(nTab);
}

void CDlgTabView::CloseFileListTab(int nTab)
{
	if (m_aTabInfo.GetCount() == 1) return;
	PathTabInfo& pti = m_aTabInfo[nTab];
	CFileListCtrl* pList = (CFileListCtrl*)pti.pWnd;
	//if (pList->m_bLoading == TRUE) return;
	pList->DestroyWindow();
	CFileListCtrl::DeleteLoadingStatus(pList);
	delete pList;
	m_aTabInfo.RemoveAt(nTab);
	m_tabPath.DeleteItem(nTab);
	nTab--;
	if (nTab < 0) nTab = 0;
	m_tabPath.SetCurSel(0);
	SetCurrentTab(nTab);
}


void CDlgTabView::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (::IsWindow(m_tabPath.GetSafeHwnd())) ArrangeCtrl();
}


void CDlgTabView::SetCurrentTab(int nTab)
{
	TCITEM ti;
	ti.mask = TCIF_IMAGE;
	if (m_nCurrentTab != nTab)
	{
		ti.iImage = 1;
		m_tabPath.SetItem(m_nCurrentTab, &ti);
	}
	ti.iImage = 0;
	m_tabPath.SetItem(nTab, &ti);

	PathTabInfo& pti = m_aTabInfo[nTab];
	CFileListCtrl* pList = (CFileListCtrl*)pti.pWnd;
	CRect rc = CRect(0, 0, 40, 30);
	if (pList == NULL)
	{
		pList = new CFileListCtrl;
		pList->m_nIconType = GetIconType();
		if (pList->Create(WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, rc, this, IDC_LIST_FILE) == FALSE)
		{
			delete pList;
			return;
		}
		pList->SetFont(&m_font);
		pList->SetExtendedStyle(LVS_EX_FULLROWSELECT); //WS_EX_WINDOWEDGE , WS_EX_CLIENTEDGE
		pList->SetBkColor(GetMyClrBk());
		pList->SetTextColor(GetMyClrText());
		pList->CMD_OpenNewTab = IDM_OPEN_NEWTAB;
		pList->CMD_UpdateTabCtrl = IDM_UPDATE_TAB;
		pList->CMD_UpdateSortInfo = IDM_UPDATE_SORTINFO;
		pList->CMD_UpdateBar = IDM_UPDATE_BAR;
		pList->m_nSortCol = pti.iSortColumn;
		pList->m_bAsc = pti.bSortAscend;
		pList->SetSortColumn(pti.iSortColumn, pti.bSortAscend);
		ListView_SetImageList(pList->GetSafeHwnd(), APP()->GetImageListByType(pList->m_nIconType) , LVSIL_SMALL);
		pti.pWnd = (CWnd*)pList;
		pList->DisplayFolder_Start(pti.strPath);
	}
	CFileListCtrl* pListOld = (CFileListCtrl*)CurrentList();
	if (pListOld != NULL && ::IsWindow(pListOld->GetSafeHwnd())) pListOld->ShowWindow(SW_HIDE);
	pList->ShowWindow(SW_SHOW);
	m_nCurrentTab = nTab;
	m_tabPath.SetCurSel(nTab);
	pList->SetFocus();
	m_editPath.SetWindowText(pti.strPath);
	SetDlgItemText(IDC_ST_BAR, pList->m_strBarMsg);
	ArrangeCtrl();
}

CWnd* CDlgTabView::CurrentList()
{
	if (m_aTabInfo.GetSize() > m_nCurrentTab && m_nCurrentTab >= 0) return m_aTabInfo[m_nCurrentTab].pWnd;
	return NULL;
}

void CDlgTabView::UpdateTabByWnd(CWnd* pWnd)
{
	if (pWnd == NULL || ::IsWindow(pWnd->GetSafeHwnd()) == FALSE) return;
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd == pWnd)
		{
			PathTabInfo& pti = m_aTabInfo[i];
			CFileListCtrl* pList = (CFileListCtrl*)pti.pWnd;
			pti.strPath = GetActualPath(pList->m_strFolder);
			//pti.strFilterInclude = pList->m_strFilterInclude; //필터정보의 저장 및 관리는 추후 검토
			SetTabTitle(i, GetPathName(pti.strPath));
			if (pList->m_strFilterInclude.IsEmpty() == FALSE && pList->m_strFilterInclude != L"*")
			{
				TCHAR path[MAX_PATH];
				PathCombineW(path, pList->m_strFolder, pList->m_strFilterInclude);
				m_editPath.SetWindowText(path);
			}
			else
			{
				m_editPath.SetWindowText(pti.strPath);
			}
			break;
		}
	}
}

void CDlgTabView::UpdateSortInfo(CWnd* pWnd)
{
	if (pWnd == NULL || ::IsWindow(pWnd->GetSafeHwnd()) == FALSE) return;
	int nTab = -1;
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd == pWnd)
		{
			nTab = i;
			break;
		}
	}
	if (nTab == -1) return;
	PathTabInfo& pti = m_aTabInfo[nTab];
	CFileListCtrl* pList = (CFileListCtrl*)pti.pWnd;
	pti.bSortAscend = pList->GetHeaderCtrl().IsAscending();
	pti.iSortColumn = pList->GetHeaderCtrl().GetSortColumn();
}

CString GetActualPath(CString strPath)
{
	if (strPath.IsEmpty()) return strPath;
	if (strPath.GetAt(0) == L'\\') return strPath;
	TCHAR path[MAX_PATH] = {};
	CString strParent = GetParentFolder(strPath);
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	hFind = FindFirstFileExW(strPath, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	CString strReturn;
	if (strParent.IsEmpty())
	{
		strReturn = strPath.MakeUpper();
	}
	else
	{
		//strReturn = GetActualPath(strParent) + L"\\" + fd.cFileName;
		PathCombine(path, GetActualPath(strParent), fd.cFileName);
		strReturn = path;
	}
	FindClose(hFind);
	return strReturn;
}

void CDlgTabView::UpdateTabByPathEdit()
{
	CFileListCtrl* pList = (CFileListCtrl*)CurrentList();
	CString strEdit, strPath, strName, strFilter;
	GetDlgItemText(IDC_EDIT_PATH, strEdit);
	//검색필터가 들어가 있는 경우 해당 필터를 잘라낸다
	int nTemp1 = strEdit.ReverseFind(L'\\');
	int nTemp2 = strEdit.ReverseFind(L'*');
	int nTemp3 = strEdit.ReverseFind(L'?');
	if ((nTemp2 != -1 && nTemp1 < nTemp2) || (nTemp3 != -1 && nTemp1 < nTemp3))
	{
		strPath = strEdit.Left(nTemp1);
		if ((nTemp1 + 1) < strEdit.GetLength()) strFilter = strEdit.Mid(nTemp1 + 1);
	}
	else
	{
		strPath = strEdit;
		strFilter.Empty();
	}
	TCHAR path_with_filter[MAX_PATH] = {};
	strPath = GetActualPath(strPath);
	PathCombine(path_with_filter, strPath, strFilter);
	SetDlgItemText(IDC_EDIT_PATH, strPath);
	pList->DisplayFolder_Start(path_with_filter);
	strName = GetPathName(strPath);
	SetTabTitle(m_nCurrentTab, strName);
}

void CDlgTabView::SetTabTitle(int nTab, CString strTitle)
{
	if (nTab < m_tabPath.GetItemCount())
	{
		TCITEM item;
		item.mask = TCIF_TEXT;
		item.pszText = strTitle.GetBuffer();
		m_tabPath.SetItem(nTab, &item);
		strTitle.ReleaseBuffer();
	}
}

void CDlgTabView::ArrangeCtrl()
{
	CRect rc;
	GetClientRect(rc);
	rc.DeflateRect(3, 3, 3, 3);
	int BH = m_lfHeight * 2;
	int TW = rc.Width();
	//Edit Control
	m_editPath.MoveWindow(rc.left, rc.top, TW, BH);
	rc.top += BH;
	rc.top += 2;
	//Toolbar
	DWORD btnsize = m_tool.GetToolBarCtrl().GetButtonSize();
	int nHP = 0, nVP = 0; //Horizontal / Vertical
	m_tool.GetToolBarCtrl().GetPadding(nHP, nVP);
	int nBtnW = (LOWORD(btnsize) + nHP);
	int nBtnH = (HIWORD(btnsize) + nVP);
	int nBtnLineCount = TW / nBtnW; if (nBtnLineCount == 0) nBtnLineCount = 1;
	int nBtnTotalCount = m_tool.GetToolBarCtrl().GetButtonCount() / 2;
	int nRow = (nBtnTotalCount / nBtnLineCount) + 1;
	int nH = nBtnH * nRow;
	m_tool.MoveWindow(rc.left, rc.top, TW, nH);
	m_tool.Invalidate();
	rc.top += nH;
	rc.top += 2;
	//Tab Part
	m_tabPath.MoveWindow(rc.left, rc.top, TW, BH);
	rc.top += BH;
	CWnd* pWnd = CurrentList();
	if (pWnd != NULL && ::IsWindow(pWnd->GetSafeHwnd()))
	{
		pWnd->MoveWindow(rc.left, rc.top, TW, rc.Height() - BH);
		pWnd->Invalidate();
	}
	GetDlgItem(IDC_ST_BAR)->MoveWindow(rc.left, rc.bottom - BH + 2, TW, BH - 2);
	GetDlgItem(IDC_ST_BAR)->Invalidate();
}




BOOL CDlgTabView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN && GetFocus() == &m_editPath)
		{
			UpdateTabByPathEdit();
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgTabView::OnTcnSelchangeTabPath(NMHDR* pNMHDR, LRESULT* pResult)
{
	SetCurrentTab(m_tabPath.GetCurSel());
	*pResult = 0;
}

void CDlgTabView::SetListColor(COLORREF crBk, COLORREF crText, BOOL bSetBk, BOOL bSetText)
{
	if (bSetBk == FALSE && bSetText == FALSE) return;
	if (bSetBk)	m_editPath.SetBkColor(crBk);
	if (bSetText) m_editPath.SetTextColor(crText);
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			CFileListCtrl* pList = (CFileListCtrl * )m_aTabInfo[i].pWnd;
			if (bSetBk)	pList->SetBkColor(crBk);
			if (bSetText) pList->SetTextColor(crText);
		}
	}

}

COLORREF CDlgTabView::GetMyClrText()
{
	if (m_nViewOptionIndex < 0 || m_nViewOptionIndex >= APP()->m_aTabViewOption.GetSize())
		return APP()->m_DefaultViewOption.clrText;
	TabViewOption& tvo = APP()->m_aTabViewOption.GetAt(m_nViewOptionIndex);
	return tvo.bUseDefaultColor ? APP()->m_DefaultViewOption.clrText : tvo.clrText;
}

COLORREF CDlgTabView::GetMyClrBk()
{
	if (m_nViewOptionIndex < 0 || m_nViewOptionIndex >= APP()->m_aTabViewOption.GetSize())
		return APP()->m_DefaultViewOption.clrBk;
	TabViewOption& tvo = APP()->m_aTabViewOption.GetAt(m_nViewOptionIndex);
	return tvo.bUseDefaultColor ? APP()->m_DefaultViewOption.clrBk : tvo.clrBk;
}


int CDlgTabView::GetIconType()
{
	if (m_nViewOptionIndex >= 0 && m_nViewOptionIndex < APP()->m_aTabViewOption.GetSize())
	{
		return APP()->m_aTabViewOption.GetAt(m_nViewOptionIndex).nIconType;
	}
	return SHIL_SMALL;
}


int CDlgTabView::GetFontSize()
{
	if (m_nViewOptionIndex < 0 || m_nViewOptionIndex >= APP()->m_aTabViewOption.GetSize())
		return APP()->m_DefaultViewOption.nFontSize;
	TabViewOption& tvo = APP()->m_aTabViewOption.GetAt(m_nViewOptionIndex);
	return tvo.bUseDefaultFont ? APP()->m_DefaultViewOption.nFontSize : tvo.nFontSize;
}

BOOL CDlgTabView::GetIsBold()
{
	if (m_nViewOptionIndex < 0 || m_nViewOptionIndex >= APP()->m_aTabViewOption.GetSize())
		return APP()->m_DefaultViewOption.bBold;
	TabViewOption& tvo = APP()->m_aTabViewOption.GetAt(m_nViewOptionIndex);
	return tvo.bUseDefaultFont ? APP()->m_DefaultViewOption.bBold : tvo.bBold;
}



void CDlgTabView::SetIconType(int nIconType)
{
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			CFileListCtrl* pList = (CFileListCtrl*)m_aTabInfo[i].pWnd;
			ListView_SetImageList(pList->GetSafeHwnd(), APP()->GetImageListByType(nIconType), LVSIL_SMALL);
		}
	}
	if (m_nViewOptionIndex >= 0 && m_nViewOptionIndex < APP()->m_aTabViewOption.GetSize())
	{
		APP()->m_aTabViewOption.GetAt(m_nViewOptionIndex).nIconType = nIconType;
	}
}


void CDlgTabView::ConfigViewOption()
{
	if (m_nViewOptionIndex < 0 || m_nViewOptionIndex >= APP()->m_aTabViewOption.GetSize())	return;
	TabViewOption& tvo = APP()->m_aTabViewOption.GetAt(m_nViewOptionIndex);
	CDlgCFG_View dlg;
	dlg.m_clrText = tvo.clrText;
	dlg.m_clrBk = tvo.clrBk;
	dlg.m_nIconType = tvo.nIconType;
	dlg.m_nFontSize = tvo.nFontSize;
	dlg.m_bBold = tvo.bBold;
	dlg.m_bUseDefaultColor = tvo.bUseDefaultColor;
	dlg.m_bUseDefaultFont = tvo.bUseDefaultFont;
	BOOL bUpdateClrBk = FALSE, bUpdateClrText = FALSE;
	if (dlg.DoModal() == IDOK)
	{
		//Color
		if (tvo.bUseDefaultColor != dlg.m_bUseDefaultColor)
		{
			tvo.bUseDefaultColor = dlg.m_bUseDefaultColor;
			if (tvo.bUseDefaultColor == FALSE)
			{
				tvo.clrText = dlg.m_clrText;
				tvo.clrBk = dlg.m_clrBk;
			}
			bUpdateClrBk = TRUE;
			bUpdateClrText = TRUE;
		}
		if (tvo.clrBk != dlg.m_clrBk)
		{
			tvo.clrBk = dlg.m_clrBk;
			if (tvo.bUseDefaultColor == FALSE) bUpdateClrBk = TRUE;
		}
		if (tvo.clrText != dlg.m_clrText)
		{
			tvo.clrText = dlg.m_clrText;
			if (tvo.bUseDefaultColor == FALSE) bUpdateClrText = TRUE;
		}
		SetListColor(GetMyClrBk(), GetMyClrText(), bUpdateClrBk, bUpdateClrText);
		//Font
		if (tvo.nFontSize != dlg.m_nFontSize || tvo.bBold != dlg.m_bBold)
		{
			tvo.nFontSize = dlg.m_nFontSize;
			tvo.bBold = dlg.m_bBold;
			if (tvo.bUseDefaultFont == FALSE)
			{
				InitFont();
				UpdateChildFont();
				ArrangeCtrl();
			}
		}
		if (tvo.bUseDefaultFont != dlg.m_bUseDefaultFont)
		{
			tvo.bUseDefaultFont = dlg.m_bUseDefaultFont;
			tvo.bBold = dlg.m_bBold;
			{
				InitFont();
				UpdateChildFont();
				ArrangeCtrl();
			}
		}
		if (tvo.nIconType != dlg.m_nIconType)
		{
			SetIconType(dlg.m_nIconType);
		}
		RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}
}

void CDlgTabView::InitFont()
{
	int nFontSize = GetFontSize();
	BOOL bBold = GetIsBold();
	LOGFONT lf;
	APP()->m_fontDefault.GetLogFont(&lf);
	lf.lfHeight = -1 * MulDiv(nFontSize, GetDeviceCaps(GetDC()->GetSafeHdc(), LOGPIXELSY), 72);
	m_lfHeight = abs(lf.lfHeight);
	if (bBold == TRUE)	lf.lfWeight = FW_BOLD;
	else				lf.lfWeight = FW_NORMAL;
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&lf); //자동 소멸되지 않도록 멤버 변수 사용
}

void CDlgTabView::UpdateChildFont()
{
	if (::IsWindow(m_tool.GetSafeHwnd()))
	{
		m_tool.SetFont(&m_font);
		InitToolBar();
		//m_tool.GetToolBarCtrl().AutoSize();
	}
	if (::IsWindow(m_tabPath.GetSafeHwnd())) m_tabPath.SetFont(&m_font);
	if (::IsWindow(m_editPath.GetSafeHwnd())) m_editPath.SetFont(&m_font);
	GetDlgItem(IDC_ST_BAR)->SetFont(&m_font);
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			CFileListCtrl* pList = (CFileListCtrl*)m_aTabInfo[i].pWnd;
			pList->SetFont(&m_font);
		}
	}
}

void CDlgTabView::UpdateToolBar()
{
	CFileListCtrl* pList = (CFileListCtrl*)CurrentList();
	m_tool.GetToolBarCtrl().EnableButton(IDM_OPEN_PREV, !(pList->IsFirstPath()));
	m_tool.GetToolBarCtrl().EnableButton(IDM_OPEN_NEXT, !(pList->IsLastPath()));
	m_tool.GetToolBarCtrl().EnableButton(IDM_OPEN_PARENT, !(pList->IsRootPath()));
}