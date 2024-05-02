// CDlgTabView.cpp: 구현 파일
//

#include "pch.h"
#include "FileOfficer.h"
#include "CDlgTabView.h"
#include "afxdialogex.h"
#include "EtcFunctions.h"
#include "CFileListCtrl.h"
#include "CMyShellListCtrl.h"
#include "CDlgCFG_View.h"

#define IDC_LIST_FILE 50000
#define IDM_UPDATE_FROMLIST 55000
#define IDM_UPDATE_SORTINFO 55001
#define IDM_UPDATE_BAR 55002
#define IDM_OPEN_NEWTAB_BY_LIST 55003
#define IDM_OPEN_FOLDER_BY_SHORCUT 55004
//#define IDM_TREE_SELCHANGED 55101

CString GetParentFolder(CString strFolder);
CString GetActualPath(CString strPath);
void ResizeBitmap(CBitmap& bmp_src, CBitmap& bmp_dst, int dstW, int dstH);
// CDlgTabView 대화 상자

IMPLEMENT_DYNAMIC(CDlgTabView, CDialogEx)

CDlgTabView::CDlgTabView(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TAB_VIEW, pParent)
{
	m_nCurrentTab = 0;
	m_bSelected = FALSE;
	m_pTool = NULL;
	m_bFindMode = FALSE;
	m_nFocusedImage = 1;
	m_lfHeight = 12;
	m_nDragBarPos = 200;
	//m_btnsize_text = 0;
	//m_btnsize_icon = 0;
}

CDlgTabView::~CDlgTabView()
{
}


void CDlgTabView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PATH, m_editPath);
	DDX_Control(pDX, IDC_TAB_PATH, m_tabPath);
	DDX_Control(pDX, IDC_LIST_SHORTCUT, m_listShortCut);
}


BEGIN_MESSAGE_MAP(CDlgTabView, CDialogEx)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PATH, &CDlgTabView::OnTcnSelchangeTabPath)
	ON_BN_CLICKED(IDC_BTN_FIND, &CDlgTabView::OnBnClickedBtnFind)
//	ON_WM_SETFOCUS()
ON_WM_ERASEBKGND()
ON_WM_DROPFILES()
ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


// CDlgTabView 메시지 처리기
void CDlgTabView::Clear()
{
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			if (m_aTabInfo[i].nCtrlType == TABTYPE_CUSTOM_LIST)
			{
				CFileListCtrl* pList = (CFileListCtrl*)m_aTabInfo[i].pWnd;
				if (IsWindow(pList->GetSafeHwnd()))
				{
					pList->ClearThread();
					pList->DestroyWindow();
				}
				delete pList;
			}
			else
			{
				CMyShellListCtrl* pList = (CMyShellListCtrl*)m_aTabInfo[i].pWnd;
				if (IsWindow(pList->GetSafeHwnd()))
				{
					pList->ClearThread();
					pList->DestroyWindow();
				}
				delete pList;
			}

		}
	}
}


BOOL CDlgTabView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDM_FILE_DELETE:
	case IDM_FILE_COPY:
	case IDM_FILE_CUT:
	case IDM_FILE_PASTE: //툴바에서 오는 경우
	case IDM_PASTE_FILE:  //메뉴에서 오는 경우
	case IDM_CONVERT_NFD:
	case IDM_CHECK_LOCKED:
	case IDM_OPEN_PREV:
	case IDM_OPEN_NEXT:
	case IDM_PLAY_ITEM:
	case IDM_OPEN_PARENT:
		CurrentList()->SendMessage(WM_COMMAND, wParam, lParam);
		break;
	case IDM_OPEN_NEWTAB_BY_LIST:
		if (lParam && ::IsWindow(((CWnd*)lParam)->GetSafeHwnd()) )
		{
			if (APP()->m_nDefaultListType == TABTYPE_CUSTOM_LIST)
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
			else
			{
				CMyShellListCtrl* pList = (CMyShellListCtrl*)lParam;
				CString strPath;
				int nItem = pList->GetNextItem(-1, LVNI_SELECTED);
				while (nItem != -1)
				{
					strPath = pList->GetItemFullPath(nItem);
					if (PathIsDirectory(strPath)) AddFileListTab(strPath);
					nItem = pList->GetNextItem(nItem, LVNI_SELECTED);
				}
			}
		}
		break;
	case IDM_UPDATE_BAR:
		if (lParam != 0) SetDlgItemText(IDC_ST_BAR, IDSTR((int)lParam));
		else UpdateMsgBarFromList();
		break;
	case IDM_OPEN_FOLDER_BY_SHORCUT:
		OpenFolderByShortCut((int)lParam);
		break;
	case IDM_UPDATE_FROMLIST: 
		if (lParam != NULL && (CWnd*)lParam == CurrentList()) UpdateFromCurrentList();
		break; 
	case IDM_UPDATE_SORTINFO: UpdateSortInfo((CWnd*)lParam); break;
	case IDM_REFRESH_LIST: UpdateTabByPathEdit(); break;
	case IDM_TAB_ADD: case IDM_ADD_LIST: AddFileListTab(APP()->m_strPath_Default); break;
	case IDM_TAB_CLOSE:	case IDM_CLOSE_LIST: CloseFileListTab(m_nCurrentTab); break;
	case IDM_CONFIG: ConfigViewOption(); break;
	case IDM_TOGGLE_FIND: ToggleFindMode(); break;
	case IDM_TOGGLE_SEARCHDLG: GetParent()->PostMessage(WM_COMMAND, wParam, lParam); break;
	case IDM_CFG_LAYOUT: GetParent()->SendMessage(WM_COMMAND, wParam, lParam); break;
	case IDM_TAB_LEFT: MoveCurrentTab(FALSE); break;
	case IDM_TAB_RIGHT: MoveCurrentTab(TRUE); break;
	case IDM_ARRANGECTRL:	ArrangeCtrl();	return TRUE;  // 아래 처리 없이 바로 리턴
	default:
		return CDialogEx::OnCommand(wParam, lParam);
	}
	UpdateToolBar();
	return TRUE;
}

void CDlgTabView::OpenFolderByShortCut(int lParam)
{
	int nItem = m_listShortCut.GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	// LPARAM이 0 = 해당 폴더 내용을 열기 
	// LPARAM이 1 = 상위 폴더 열고 선택하기
	CString strPath = m_listShortCut.GetItemFullPath(nItem);
	CString strName, strParent;
	if (lParam == 1)
	{
		strName = m_listShortCut.GetItemText(nItem, 0);
		strParent = Get_Folder(strPath);
	}
	else
	{
		strName.Empty();
		strParent = m_listShortCut.GetItemFullPath(nItem);
	}
	//이미 열려있는 폴더이면 해당 탭을 열고
	BOOL bOpen = FALSE;
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		PathTabInfo& pti = m_aTabInfo.GetAt(i);
		if (strParent.CompareNoCase(pti.strPath) == 0)
		{
			SetCurrentTab(i);
			bOpen = TRUE;
		}
	}
	if (bOpen == FALSE)
	{
		//아니면 현재 탭에서 연다 
		m_editPath.SetWindowText(strParent);
		UpdateTabByPathEdit();
	}
	//상위 폴더를 연 경우 지정된 파일을 선택해 준다
	if (lParam == 1 && strName.IsEmpty() == FALSE)
	{
		int nNameColumnIndex = 0; //COL_NAME 
		CMFCListCtrl* pList = (CMFCListCtrl*)CurrentList();
		if (CurrentListType() == TABTYPE_CUSTOM_LIST)
		{
			nNameColumnIndex = ((CFileListCtrl*)pList)->GetNameColumnIndex();
		}
 		int nCount = pList->GetItemCount();
		for (int i = 0; i < nCount; i++)
		{
			if (strName.CompareNoCase(pList->GetItemText(i, nNameColumnIndex)) == 0)
			{
				pList->SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				pList->EnsureVisible(i, FALSE);
			}
			else
			{
				pList->SetItemState(i, 0, LVIS_SELECTED | LVIS_FOCUSED);
			}
		}
	}
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
	//툴바를 두개 만든다 텍스트 있는것/없는것
	//텍스트를 동적으로 넣었다 빼는 경우 빼더라도 버튼 크기가 완전히 줄어들지 않음(MFC의 버그)
	m_pTool = &m_toolIcon;
	m_toolIcon.LoadToolBar(IDR_TB_TAB);
	ResizeToolBar(APP()->m_nToolBarButtonSize, APP()->m_nToolBarButtonSize);
	//m_toolText.LoadToolBar(IDR_TB_TAB);
/*	UINT nStyle;
	int nCount = m_toolText.GetCount();
	int nTextIndex = 0;
	for (int i = 0; i < nCount; i++)
	{
		nStyle = m_toolText.GetButtonStyle(i);
		if (!(nStyle & TBBS_SEPARATOR))
		{
			m_toolText.SetButtonText(i, IDSTR(IDS_TB_00 + nTextIndex));
			nTextIndex++;
		}
	}
	m_btnsize_text = m_toolText.GetToolBarCtrl().GetButtonSize();*/
	//m_btnsize_icon = m_toolIcon.GetToolBarCtrl().GetButtonSize();
}

BOOL CDlgTabView::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	ModifyStyle(0, WS_CLIPCHILDREN);
	InitFont();
	m_toolIcon.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_WRAPABLE, WS_CHILD | WS_VISIBLE | CBRS_BORDER_ANY); // TBSTYLE_TRANSPARENT
	//m_toolText.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_WRAPABLE, WS_CHILD | WS_VISIBLE | CBRS_BORDER_ANY); // TBSTYLE_TRANSPARENT
	UpdateChildFont();
	InitToolBar();
	m_wndDragTab.CreateDragBar(FALSE, this, 35100);
	m_wndDragTab.m_pBarPos = &(m_nDragBarPos);
	m_editPath.EnableFolderBrowseButton();
	m_editPath.CMD_UpdateList = IDM_REFRESH_LIST;
	m_tabImgList.Create(IDB_TABICON, 16, 2, RGB(255, 0, 255));
	//m_tabPath.SetExtendedStyle(TCS_EX_FLATSEPARATORS, TCS_EX_FLATSEPARATORS);
	m_tabPath.SetImageList(&m_tabImgList);
	//ShortCut List
	m_listShortCut.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_listShortCut.CMD_OpenFolderByShortCut = IDM_OPEN_FOLDER_BY_SHORCUT;
	m_listShortCut.SetViewType(m_listShortCut.m_nViewType);
	m_listShortCut.SetIconType(m_listShortCut.m_nIconType);
	m_listShortCut.InsertColumn(0, IDSTR(IDS_COL_DRIVE_NAME), LVCFMT_LEFT, 200);
	m_listShortCut.InsertColumn(1, IDSTR(IDS_COL_DRIVE_PATH), LVCFMT_LEFT, 0);
	m_listShortCut.DragAcceptFiles(TRUE);

	//Set All Control Colors
	SetCtrlColor(GetMyClrBk(), GetMyClrText(), TRUE, TRUE);
	// Init Tabs
	if (m_aTabInfo.GetSize() == 0)
	{
		PathTabInfo tabInfo(L"", 0, TRUE);
		tabInfo.nCtrlType = APP()->m_nDefaultListType;
		m_aTabInfo.Add(tabInfo);
	}
	CString strTitle;
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		m_tabPath.InsertItem(i, CFileListCtrl::GetPathName(m_aTabInfo[i].strPath), 1);
	}
	if (m_aTabInfo.GetSize() <= m_nCurrentTab) m_nCurrentTab = 0;
	DragAcceptFiles(TRUE);
	//ArrangeCtrl(); //SetCurrentTab 안에 포함되어 있음
	return TRUE;
}

void CDlgTabView::AddFileListTab(CString strPath)
{
	PathTabInfo tabInfo(strPath, APP()->m_nSortCol_Default, APP()->m_bSortAscend_Default);
	tabInfo.nCtrlType = APP()->m_nDefaultListType;
	m_aTabInfo.Add(tabInfo);
	int nTab = m_tabPath.InsertItem((int)m_aTabInfo.GetSize(), CFileListCtrl::GetPathName(strPath), 1);
	SetCurrentTab(nTab);
}

void CDlgTabView::CloseFileListTab(int nTab)
{
	if (m_aTabInfo.GetCount() == 1) return;
	PathTabInfo& pti = m_aTabInfo[nTab];
	if (pti.nCtrlType == TABTYPE_CUSTOM_LIST)
	{
		CFileListCtrl* pList = (CFileListCtrl*)pti.pWnd;
		pList->ClearThread();
		pList->DestroyWindow();
		delete pList;
	}
	else
	{
		CMyShellListCtrl* pList = (CMyShellListCtrl*)pti.pWnd;
		pList->ClearThread();
		pList->DestroyWindow();
		delete pList;
	}
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

void CDlgTabView::SetSelected(BOOL bSelected)
{
	if (bSelected == TRUE)
	{
		m_nFocusedImage = 0;
	}
	else
	{
		m_nFocusedImage = 1;
	}
	TCITEM ti;
	ti.mask = TCIF_IMAGE;
	ti.iImage = m_nFocusedImage;
	m_tabPath.SetItem(m_nCurrentTab, &ti);
	m_tabPath.Invalidate();
}

void CDlgTabView::SetCurrentTab(int nTab)
{
	TCITEM ti;
	ti.mask = TCIF_IMAGE;
	if (m_nCurrentTab != nTab)
	{
		ti.iImage = 1; //Gray Icon
		m_tabPath.SetItem(m_nCurrentTab, &ti);
	}
	ti.iImage = m_nFocusedImage; //포커스 없을땐 Gray, 있을땐 Yellow
	m_tabPath.SetItem(nTab, &ti);
	PathTabInfo& pti = m_aTabInfo[nTab];
	CMFCListCtrl* pList = (CMFCListCtrl*)pti.pWnd;
	CRect rc = CRect(0, 0, 40, 30);
	if (pList == NULL)
	{
		if (pti.nCtrlType == TABTYPE_CUSTOM_LIST)
		{
			CFileListCtrl* pMyList = new CFileListCtrl;
			pMyList->m_aColWidth.Copy(pti.aColWidth);
			if (pMyList->Create(WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, rc, this, IDC_LIST_FILE) == FALSE)
			{
				delete pMyList;
				return;
			}
			pList = pMyList;
		}
		else
		{
			CMyShellListCtrl* pMyList = new CMyShellListCtrl;
			pMyList->m_aColWidth.Copy(pti.aColWidth);
			if (pMyList->Create(WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, rc, this, IDC_LIST_FILE) == FALSE)
			{
				delete pMyList;
				return;
			}
			pList = pMyList;
		}
		pList->SetFont(&m_font);
		pList->SetBkColor(GetMyClrBk());
		pList->SetTextColor(GetMyClrText());
		pList->SetSortColumn(pti.iSortColumn, pti.bSortAscend);
		pList->SetExtendedStyle(LVS_EX_FULLROWSELECT); //WS_EX_WINDOWEDGE , WS_EX_CLIENTEDGE
		pti.pWnd = (CWnd*)pList;
		if (pti.nCtrlType == TABTYPE_CUSTOM_LIST)
		{
			CFileListCtrl* pMyList = (CFileListCtrl*)pList;
			pMyList->m_pColorRuleArray = &m_tvo.aColorRules;
			pMyList->m_nIconType = GetIconType();
			pMyList->CMD_OpenNewTabByList = IDM_OPEN_NEWTAB_BY_LIST;
			pMyList->CMD_UpdateFromList = IDM_UPDATE_FROMLIST;
			pMyList->CMD_UpdateSortInfo = IDM_UPDATE_SORTINFO;
			pMyList->CMD_UpdateBar = IDM_UPDATE_BAR;
			pMyList->m_nSortCol = pti.iSortColumn;
			pMyList->m_bAsc = pti.bSortAscend;
			pMyList->m_bUseFileType = APP()->m_bUseFileType;
			pMyList->m_bUseFileIcon = APP()->m_bUseFileIcon;
			pMyList->m_bCheckOpen = APP()->m_bCheckOpen;
			pMyList->SetIconType(GetIconType());
			pMyList->DisplayFolder_Start(pti.strPath);
		}
		else
		{
			CMyShellListCtrl* pMyList = (CMyShellListCtrl*)pList;
			pMyList->CMD_OpenNewTabByList = IDM_OPEN_NEWTAB_BY_LIST;
			pMyList->CMD_UpdateFromList = IDM_UPDATE_FROMLIST;
			pMyList->CMD_UpdateSortInfo = IDM_UPDATE_SORTINFO;
			pMyList->CMD_UpdateBar = IDM_UPDATE_BAR;
			pMyList->m_nIconType = GetIconType();
			pMyList->m_pColorRuleArray = &m_tvo.aColorRules;
			pMyList->m_nSortColInit = pti.iSortColumn;
			pMyList->m_bAscInit = pti.bSortAscend;
			pMyList->LoadFolder(pti.strPath, TRUE);
		}
	}
	CMFCListCtrl* pListOld = (CMFCListCtrl*)CurrentList();
	if (pListOld != NULL && ::IsWindow(pListOld->GetSafeHwnd())) pListOld->ShowWindow(SW_HIDE);
	UpdateBkImg(pList);
	pList->ShowWindow(SW_SHOW);
	m_nCurrentTab = nTab;
	m_tabPath.SetCurSel(nTab);
	pList->SetFocus();
	m_editPath.SetWindowText(pti.strPath);
	if (pti.nCtrlType == TABTYPE_CUSTOM_LIST)
	{
		CFileListCtrl* pMyList = (CFileListCtrl*)pList;
		pMyList->UpdateMsgBar();
	}
	else
	{
		CMyShellListCtrl* pMyList = (CMyShellListCtrl*)pList;
		pMyList->UpdateMsgBar();
	}
	UpdateFromCurrentList();
	UpdateToolBar();
	ArrangeCtrl();
}

CWnd* CDlgTabView::CurrentList()
{
	if (m_aTabInfo.GetSize() > m_nCurrentTab && m_nCurrentTab >= 0) return m_aTabInfo[m_nCurrentTab].pWnd;
	return NULL;
}

int CDlgTabView::CurrentListType()
{
	if (m_aTabInfo.GetSize() > m_nCurrentTab && m_nCurrentTab >= 0) return m_aTabInfo[m_nCurrentTab].nCtrlType;
	return -1;
}

void CDlgTabView::UpdateFromCurrentList()
{
	CWnd* pWnd = CurrentList();
	if (pWnd == NULL || ::IsWindow(pWnd->GetSafeHwnd()) == FALSE) return;
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd == pWnd)
		{
			PathTabInfo& pti = m_aTabInfo[i];
			if (pti.nCtrlType == TABTYPE_CUSTOM_LIST)
			{
				CFileListCtrl* pList = (CFileListCtrl*)pti.pWnd;
				pti.strPath = GetActualPath(pList->m_strFolder);
				SetTabTitle(i, CFileListCtrl::GetPathName(pti.strPath));
				if (pList->m_strFilterInclude.IsEmpty() == FALSE && pList->m_strFilterInclude != L"*")
				{
					CString strPath;
					strPath = PathBackSlash(pList->m_strFolder) + pList->m_strFilterInclude;
					m_editPath.SetWindowText(strPath);
				}
				else
				{
					m_editPath.SetWindowText(pti.strPath);
				}
				m_editPath.SetSel(-1); //커서를 끝으로
			}
			else
			{
				CMyShellListCtrl* pList = (CMyShellListCtrl*)pti.pWnd;
				CString strFolder;
				if (pList->GetCurrentFolder(strFolder) == FALSE) strFolder = _T("");
				pti.strPath = GetActualPath(strFolder);
				SetTabTitle(i, CFileListCtrl::GetPathName(pti.strPath));
				m_editPath.SetWindowText(pti.strPath);
				m_editPath.SetSel(-1); //커서를 끝으로
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
	CMFCListCtrl* pList = (CMFCListCtrl*)pti.pWnd;
	if (pList && ::IsWindow(pList->GetSafeHwnd()))
	{
		pti.bSortAscend = pList->GetHeaderCtrl().IsAscending();
		pti.iSortColumn = pList->GetHeaderCtrl().GetSortColumn();
	}
}

CString GetActualPath(CString strPath)
{
	if (strPath.IsEmpty()) return strPath;
	if (strPath.GetAt(0) == L'\\') return strPath;
	CString strParent = GetParentFolder(strPath);
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	hFind = FindFirstFileExW(strPath, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	if (hFind == INVALID_HANDLE_VALUE) return strPath; //해당 경로가 존재하지 않는 경우 원래값 그대로 반환 
	CString strReturn;
	if (strParent.IsEmpty())
	{	//각 드라이브의 루트이므로 드라이브 문자는 대문자로
		strReturn = strPath.MakeUpper();
	}
	else
	{
		strReturn = PathBackSlash(GetActualPath(strParent)) + fd.cFileName;
	}
	FindClose(hFind);
	return strReturn;
}

void CDlgTabView::UpdateTabByPathEdit()
{
	CString strEdit, strPath, strName, strFilter;
	m_editPath.GetWindowText(strEdit);
	//끝 글자가 '\\' 인경우 잘라낸다
	while (strEdit.IsEmpty() == FALSE)
	{
		int nLast = strEdit.GetLength() - 1;
		if (strEdit.GetAt(nLast) == L'\\') strEdit.Delete(nLast);
		else break;
	}
	 
	//검색필터가 들어가 있는 경우 해당 필터를 잘라낸다
	int nTemp0 = strEdit.ReverseFind(L':');
	int nTemp1 = strEdit.ReverseFind(L'\\');
	int nTemp2 = strEdit.ReverseFind(L'*');
	int nTemp3 = strEdit.ReverseFind(L'?');
	if (nTemp0 == -1 && nTemp1 == -1)
	{	//경로에 '\\' 또는 ':'이 없다면  잘못된 입력이므로 빈 경로로 처리한다. 
		strPath.Empty();
		strFilter.Empty();
	}
	else if (nTemp1 != -1 && ((nTemp2 != -1 && nTemp1 < nTemp2) || (nTemp3 != -1 && nTemp1 < nTemp3)))
	{	//'\\' 뒤에 와일드카드가 붙어있는 모양인 경우 필터로 추출한다.
		strPath = strEdit.Left(nTemp1);
		if ((nTemp1 + 1) < strEdit.GetLength()) strFilter = strEdit.Mid(nTemp1 + 1);
		else strFilter.Empty();
	}
	else if (nTemp0 != -1 && nTemp1 == -1 && (nTemp2 != -1 || nTemp3 != -1))
	{	// D:*.jpg 처럼 ':'는 있는데 '\\'를 빼먹은 경우는 빈 경로로 처리
		strPath.Empty();
		strFilter.Empty();
	}
	else
	{	//와일드카드가 없다면 그냥 처리
		strPath = strEdit;
		strFilter.Empty();
	}
	strPath = GetActualPath(strPath);
	strName = CFileListCtrl::GetPathName(strPath);
	SetTabTitle(m_nCurrentTab, strName);

	if (CurrentListType() == TABTYPE_CUSTOM_LIST)
	{
		CFileListCtrl* pList = (CFileListCtrl*)CurrentList();
		if (strFilter.IsEmpty() == FALSE)
		{
			pList->DisplayFolder_Start(PathBackSlash(strPath) + strFilter);
		}
		else
		{
			pList->m_strFilterInclude.Empty();
			pList->m_strFilterExclude.Empty();
			pList->DisplayFolder_Start(strPath);
		}
	}
	else
	{
		CMyShellListCtrl* pList = (CMyShellListCtrl*)CurrentList();
		m_aTabInfo[m_nCurrentTab].strPath = strPath;
		pList->LoadFolder(strPath);
	}
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
	if (m_pTool == NULL) return;
	if (::IsWindow(m_wndDragTab.GetSafeHwnd()) == FALSE) return;
	CRect rc;
	GetClientRect(rc);
	rc.DeflateRect(3, 3, 3, 3);
	int BH = m_lfHeight * 2;
	int TW = rc.Width();
	//Edit Control
	m_editPath.MoveWindow(rc.left, rc.top, TW, BH, TRUE); 	//m_editPath.Invalidate(TRUE);
	rc.top += BH;
	rc.top += 2;
	//Toolbar
	//DWORD btnsize = (APP()->m_bToolBarText ? m_btnsize_text : m_btnsize_icon);
	DWORD btnsize = m_pTool->GetToolBarCtrl().GetButtonSize();
	int nHP = 0, nVP = 0; //Horizontal / Vertical
	m_pTool->GetToolBarCtrl().GetPadding(nHP, nVP);
	int nBtnW = LOWORD(btnsize);
	int nBtnH = HIWORD(btnsize);
	if (APP()->m_bToolBarVertical == FALSE)
	{
		//가로로 배치할때는 여러줄로 배치 가능하도록
		int nBtnLineCount = TW / nBtnW; if (nBtnLineCount == 0) nBtnLineCount = 1;
		int nBtnTotalCount = m_pTool->GetToolBarCtrl().GetButtonCount(); //Separator를 쓰지 않아야 함
		int nRow = (nBtnTotalCount % nBtnLineCount == 0) ? nBtnTotalCount / nBtnLineCount : (nBtnTotalCount / nBtnLineCount) + 1;
		int nToolH = nBtnH * nRow + nVP;
		m_pTool->MoveWindow(rc.left, rc.top, TW, nToolH, TRUE);
 		m_pTool->Invalidate(TRUE);
		rc.top += nToolH;
		rc.top += 2;
	}
	else
	{
		//세로로 배치할때는 그냥 한줄로
		int nToolW = nBtnW + nHP;
		m_pTool->MoveWindow(rc.left, rc.top, nToolW, rc.Height(), TRUE);
		m_pTool->Invalidate(TRUE);
		rc.left += nToolW;
		rc.left += 2;
		TW = TW - (nToolW + 2);
	}
	//찾기 기능 
	if (m_bFindMode == FALSE)
	{	
		GetDlgItem(IDC_EDIT_FIND)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_FIND)->ShowWindow(SW_HIDE);
	}
	else
	{
		int nFindBtnW = BH * 3;
		GetDlgItem(IDC_EDIT_FIND)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTN_FIND)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_FIND)->MoveWindow(rc.left, rc.top, TW - nFindBtnW, BH, TRUE);
		//GetDlgItem(IDC_EDIT_FIND)->Invalidate(TRUE);
		GetDlgItem(IDC_BTN_FIND)->MoveWindow(rc.left + TW - nFindBtnW + 1, rc.top, nFindBtnW - 1, BH, TRUE);
		//GetDlgItem(IDC_BTN_FIND)->Invalidate(TRUE);
		rc.top += BH;
		rc.top += 2;
	}
	//Tab Part
	if (m_nDragBarPos < 0) m_nDragBarPos = 0;
	CRect rcTab;
	if (m_bViewShortCut)
	{
		int nDragBarSize = 4;
		CRect rcSC = CRect(rc.left, rc.top, rc.left + m_nDragBarPos - 1, rc.bottom - BH);
		CRect rcBar = CRect(rcSC.right + 1, rc.top, rcSC.right + nDragBarSize, rc.bottom - BH);
		rcTab = CRect(rcBar.right + 1, rc.top, rc.right, rc.top + BH - 1);

		
		m_listShortCut.ShowWindow(SW_SHOW);
		m_wndDragTab.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ST_SHORTCUT)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ST_SHORTCUT)->MoveWindow(rcSC.left, rcSC.top, rcSC.Width(), BH - 1, TRUE);
		GetDlgItem(IDC_ST_SHORTCUT)->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
		rcSC.top += BH;
		m_listShortCut.MoveWindow(rcSC, TRUE); 
		//컬럼 폭 자동 조정
		CRect rcClient; m_listShortCut.GetClientRect(rcClient);
		if (m_listShortCut.GetColumnWidth(0) < rcClient.Width())
			m_listShortCut.SetColumnWidth(0, rcClient.Width());
		m_listShortCut.SetColumnWidth(1, 0);
		//////
		m_wndDragTab.MoveWindow(rcBar, TRUE);
		m_tabPath.MoveWindow(rcTab, TRUE); 
		rc.top += BH;
		CWnd* pWnd = CurrentList();
		if (pWnd != NULL && ::IsWindow(pWnd->GetSafeHwnd()))
		{
			CRect rcList = CRect(rcTab.left, rcTab.bottom + 1, rcTab.right, rc.bottom - BH);
			pWnd->MoveWindow(rcList, TRUE);
			//pWnd->Invalidate(TRUE);
		}
	}
	else
	{
		rcTab = CRect(rc.left, rc.top, rc.right, rc.top + BH - 1);
		GetDlgItem(IDC_ST_SHORTCUT)->ShowWindow(SW_HIDE);
		m_listShortCut.ShowWindow(SW_HIDE);
		m_wndDragTab.ShowWindow(SW_HIDE);
		m_tabPath.MoveWindow(rcTab, TRUE);
	}
	CWnd* pWnd = CurrentList();
	if (pWnd != NULL && ::IsWindow(pWnd->GetSafeHwnd()))
	{
		CRect rcList = CRect(rcTab.left, rcTab.bottom + 1, rcTab.right, rc.bottom - BH);
		pWnd->MoveWindow(rcList, TRUE);
		//pWnd->Invalidate(TRUE);
	}
	GetDlgItem(IDC_ST_BAR)->MoveWindow(rc.left, rc.bottom - BH + 4, TW, BH - 4, FALSE);
	GetDlgItem(IDC_ST_BAR)->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
}

void CDlgTabView::ResizeToolBar(int width, int height)
{
	if (m_pTool == NULL) return;
	//DWORD dwSize = m_toolIcon.GetToolBarCtrl().GetButtonSize();
	//int nHP = 0, nVP = 0; //Horizontal / Vertical
	//m_toolIcon.GetToolBarCtrl().GetPadding(nHP, nVP);
	//CSize btnsize(LOWORD(dwSize), HIWORD(dwSize));
	//btnsize.cx = width;
	//btnsize.cy = height;
	//m_pTool->GetToolBarCtrl().SetButtonSize(CSize(width, height));
	int nCount = m_pTool->GetToolBarCtrl().GetButtonCount();
	CImageList imgList;
	CBitmap bm_original, bm_resized;
	bm_original.LoadBitmap(IDR_TB_TAB);
	int size_x = width * nCount;
	int size_y = height;
	ResizeBitmap(bm_original, bm_resized, size_x, size_y);
	imgList.Create(width, height, ILC_COLORDDB | ILC_MASK, nCount, 0);
	imgList.Add(&bm_resized, RGB(255, 0, 255));
	m_pTool->SendMessage(TB_SETIMAGELIST, 0, (LPARAM)imgList.m_hImageList);
	imgList.Detach();
	bm_original.Detach();
	bm_resized.Detach();
	/*DWORD dwSize;
	dwSize = m_pTool->GetToolBarCtrl().GetButtonSize();
	int cx = LOWORD(dwSize);
	int cy = LOWORD(dwSize);*/
}



BOOL CDlgTabView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			CWnd* pWnd = GetFocus();
			if (pWnd == &m_editPath)
			{
				UpdateTabByPathEdit();
				return TRUE;
			}
			else if (pWnd == GetDlgItem(IDC_EDIT_FIND) || pWnd == GetDlgItem(IDC_BTN_FIND))
			{
				FindNext();
				return TRUE;
			}
		}
		if (pMsg->wParam == VK_F3)
		{
			ToggleFindMode();
			return TRUE;
		}
		if (pMsg->wParam == VK_F4) // Ctrl + F 로 바꿀지 생각해 볼것
		{
			GetParent()->PostMessageW(WM_COMMAND, IDM_TOGGLE_SEARCHDLG, 0);
			return TRUE;
		}
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
		if ((GetKeyState(VK_CONTROL) & 0xFF00) != 0)
		{
			if (pMsg->wParam == VK_LEFT) { OnCommand(IDM_TAB_LEFT, 0); return TRUE; }
			if (pMsg->wParam == VK_RIGHT) { OnCommand(IDM_TAB_RIGHT, 0); return TRUE; }
			if (pMsg->wParam == VK_F4) { OnCommand(IDM_TAB_CLOSE, 0); return TRUE; }
			if (pMsg->wParam == _T('O')) { OnCommand(IDM_TAB_ADD, 0); return TRUE; }
		}
	}
	/*	if (pMsg->message == WM_LBUTTONDOWN)
	{
		CurrentList()->SetFocus();
	}
	else if (pMsg->message == WM_RBUTTONUP)
	{
		if (CurrentListType() == TABTYPE_CUSTOM_LIST) 
			((CFileListCtrl*)CurrentList())->ShowContextMenu(NULL);
		else
			((CMyShellListCtrl*)CurrentList())->ShowContextMenu(NULL);
	}*/
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgTabView::OnTcnSelchangeTabPath(NMHDR* pNMHDR, LRESULT* pResult)
{
	SetCurrentTab(m_tabPath.GetCurSel());
	*pResult = 0;
}

void CDlgTabView::SetCtrlColor(COLORREF crBk, COLORREF crText, BOOL bSetBk, BOOL bSetText)
{
	if (bSetBk == FALSE && bSetText == FALSE) return;
	if (bSetBk)
	{
		m_editPath.SetBkColor(crBk);
		m_listShortCut.SetBkColor(crBk);
		m_listShortCut.SetTextBkColor(TRANSPARENT);
	}
	if (bSetText)
	{
		m_editPath.SetTextColor(crText);
		m_listShortCut.SetTextColor(crText);
	}
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL && ::IsWindow(m_aTabInfo[i].pWnd->GetSafeHwnd()) != FALSE)
		{
			CMFCListCtrl* pList = (CMFCListCtrl*)m_aTabInfo[i].pWnd;
			if (bSetBk)	pList->SetBkColor(crBk);
			if (bSetText) pList->SetTextColor(crText);
		}
	}
}

COLORREF CDlgTabView::GetMyClrText()
{
	return m_tvo.bUseDefaultColor ? APP()->m_DefaultViewOption.clrText : m_tvo.clrText;
}

COLORREF CDlgTabView::GetMyClrBk()
{
	return m_tvo.bUseDefaultColor ? APP()->m_DefaultViewOption.clrBk : m_tvo.clrBk;
}


int CDlgTabView::GetIconType()
{
	return m_tvo.nIconType;
}


void CDlgTabView::SetIconType(int nIconType)
{
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			CFileListCtrl* pList = (CFileListCtrl*)m_aTabInfo[i].pWnd;
			if (pList->m_nIconType != nIconType)
			{
				pList->SetIconType(nIconType);
				if (i == m_nCurrentTab) UpdateTabByPathEdit();
				else pList->RefreshList();
			}
		}
	}
	m_tvo.nIconType = nIconType;
}


void CDlgTabView::ConfigViewOption()
{
	TabViewOption& tvo = m_tvo;
	CDlgCFG_View dlg;
	dlg.m_tvo = tvo;
	m_font.GetLogFont(&dlg.m_lf);
	BOOL bUpdateClrBk = FALSE, bUpdateClrText = FALSE, bFontUpdated = FALSE;
	if (dlg.DoModal() == IDOK)
	{
		//Color
		if (tvo.bUseDefaultColor != dlg.m_tvo.bUseDefaultColor)
		{
			tvo.bUseDefaultColor = dlg.m_tvo.bUseDefaultColor;
			if (tvo.bUseDefaultColor == FALSE)
			{
				tvo.clrText = dlg.m_tvo.clrText;
				tvo.clrBk = dlg.m_tvo.clrBk;
			}
			bUpdateClrBk = TRUE;
			bUpdateClrText = TRUE;
		}
		if (tvo.clrBk != dlg.m_tvo.clrBk)
		{
			tvo.clrBk = dlg.m_tvo.clrBk;
			if (tvo.bUseDefaultColor == FALSE) bUpdateClrBk = TRUE;
		}
		if (tvo.clrText != dlg.m_tvo.clrText)
		{
			tvo.clrText = dlg.m_tvo.clrText;
			if (tvo.bUseDefaultColor == FALSE) bUpdateClrText = TRUE;
		}
		SetCtrlColor(GetMyClrBk(), GetMyClrText(), bUpdateClrBk, bUpdateClrText);
		//Font
		if (dlg.m_bUpdateFont != FALSE) // 폰트 선택 다이얼로그를 연 경우
		{
			CString strFontName = dlg.m_lf.lfFaceName;
			if (strFontName.IsEmpty() == FALSE)	tvo.strFontName = strFontName;
			tvo.nFontSize = dlg.m_tvo.nFontSize;
			tvo.nFontWeight = dlg.m_lf.lfWeight;
			tvo.bFontItalic = dlg.m_lf.lfItalic;
			if (tvo.bUseDefaultFont == FALSE) bFontUpdated = TRUE;
		}
		if (tvo.nFontSize != dlg.m_tvo.nFontSize) // 에디트 컨트롤에서 폰트 크기만 변경
		{
			tvo.nFontSize = dlg.m_tvo.nFontSize;
			if (tvo.bUseDefaultFont == FALSE) bFontUpdated = TRUE;
		}
		if (tvo.bUseDefaultFont != dlg.m_tvo.bUseDefaultFont) // 디폴트 폰트 사용여부 변경
		{
			tvo.bUseDefaultFont = dlg.m_tvo.bUseDefaultFont;
			bFontUpdated = TRUE;
		}
		if (bFontUpdated != FALSE)
		{
			InitFont();
			UpdateChildFont();
			ArrangeCtrl();
		}
		//Icon
		if (tvo.nIconType != dlg.m_tvo.nIconType)
		{
			SetIconType(dlg.m_tvo.nIconType);
		}
		//Background Image Path
		if (dlg.m_tvo.bUseBkImage != tvo.bUseBkImage || dlg.m_tvo.strBkImagePath != tvo.strBkImagePath)
		{
			tvo.bUseBkImage = dlg.m_tvo.bUseBkImage;
			tvo.strBkImagePath = dlg.m_tvo.strBkImagePath;
			UpdateBkImgAll();
		}
		//Color Rules
		tvo.aColorRules.Copy(dlg.m_tvo.aColorRules);
		RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}
}

void CDlgTabView::UpdateBkImgAll()
{
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			CMFCListCtrl* pList = (CMFCListCtrl*)m_aTabInfo[i].pWnd;;
			if (IsWindow(pList->GetSafeHwnd()))
			{
				UpdateBkImg(pList);
			}
		}
	}
}

void CDlgTabView::UpdateBkImg(CWnd* pWnd)
{
	CMFCListCtrl* pList = (CMFCListCtrl*)pWnd;;
	if (m_tvo.bUseBkImage)
	{
		pList->SetBkImage(m_tvo.strBkImagePath.GetBuffer(), FALSE, 0, 0);
		m_tvo.strBkImagePath.ReleaseBuffer();
	}
	else
	{
		LVBKIMAGE li;
		li.ulFlags = LVBKIF_SOURCE_NONE;
		pList->SetBkImage(&li);
	}
}

void CDlgTabView::InitFont()
{
	LOGFONT lf;
	if (m_tvo.bUseDefaultFont == FALSE)
	{
		if (m_tvo.strFontName.IsEmpty() == FALSE)
		{
			memset(&lf, 0, sizeof(LOGFONT));
			_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, m_tvo.strFontName, _TRUNCATE);
		}
		else //폰트 이름이 없으면 윈도우 기본 폰트를 가져와서 크기 등을 바꾼다
		{
			APP()->m_fontDefault.GetLogFont(&lf);
		}
		lf.lfHeight = m_tvo.nFontSize * 10;
		lf.lfWeight = m_tvo.nFontWeight;
		lf.lfItalic = m_tvo.bFontItalic;
		m_font.DeleteObject();
		m_font.CreatePointFontIndirect(&lf);
	}
	else  //윈도우 기본 폰트를 써야 할때
	{
		APP()->m_fontDefault.GetLogFont(&lf);
		m_font.DeleteObject();
		m_font.CreateFontIndirect(&lf);
	}
	//화면 확대 축소 비율을 반영한 컨트롤 배치용 초기값 구하기
	m_font.GetLogFont(&lf);
	m_lfHeight = abs(lf.lfHeight);
}

void CDlgTabView::UpdateChildFont()
{
/*	if (::IsWindow(m_toolText.GetSafeHwnd()))
	{
		m_toolIcon.SetFont(&m_font);
		m_toolText.SetFont(&m_font);
		InitToolBar();
	}*/
	if (::IsWindow(m_tabPath.GetSafeHwnd())) m_tabPath.SetFont(&m_font);
	if (::IsWindow(m_editPath.GetSafeHwnd())) m_editPath.SetFont(&m_font);
	if (::IsWindow(m_listShortCut.GetSafeHwnd())) m_listShortCut.SetFont(&m_font);
	GetDlgItem(IDC_ST_BAR)->SetFont(&m_font);
	GetDlgItem(IDC_ST_SHORTCUT)->SetFont(&m_font);
	GetDlgItem(IDC_EDIT_FIND)->SetFont(&m_font);
	GetDlgItem(IDC_BTN_FIND)->SetFont(&m_font);
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		if (m_aTabInfo[i].pWnd != NULL)
		{
			CMFCListCtrl* pList = (CMFCListCtrl*)m_aTabInfo[i].pWnd;
			pList->SetFont(&m_font);
		}
	}
}

void CDlgTabView::UpdateToolBar()
{
	//m_pTool = (APP()->m_bToolBarText) ? &m_toolText : &m_toolIcon;
	//m_toolText.ShowWindow((APP()->m_bToolBarText) ? SW_SHOW : SW_HIDE);
	//m_toolIcon.ShowWindow((APP()->m_bToolBarText) ? SW_HIDE : SW_SHOW);
	m_pTool = &m_toolIcon;
	if (CurrentListType() == TABTYPE_CUSTOM_LIST)
	{
		CFileListCtrl* pList = (CFileListCtrl*)CurrentList();
		if (pList)
		{
			m_pTool->GetToolBarCtrl().EnableButton(IDM_OPEN_PREV, (pList->IsFirstPath() == FALSE));
			m_pTool->GetToolBarCtrl().EnableButton(IDM_OPEN_NEXT, (pList->IsLastPath() == FALSE));
			m_pTool->GetToolBarCtrl().EnableButton(IDM_OPEN_PARENT, (pList->IsRootPath() == FALSE));
		}
	}
	else
	{
		CMyShellListCtrl* pList = (CMyShellListCtrl*)CurrentList();
	}
}

void CDlgTabView::UpdateColWidths()
{
	for (int i = 0; i < m_aTabInfo.GetSize(); i++)
	{
		m_aTabInfo[i].UpdateColWidth();
	}
}

void CDlgTabView::OnBnClickedBtnFind()
{
	FindNext();
}

void CDlgTabView::ToggleFindMode()
{
	m_bFindMode = (m_bFindMode == FALSE); 
	if (m_bFindMode == TRUE)
	{
		GetDlgItem(IDC_EDIT_FIND)->SetFocus();
	}
	else
	{
		CurrentList()->SetFocus();
	}
	ArrangeCtrl();
}

void CDlgTabView::FindNext()
{
	CMFCListCtrl* pList = (CMFCListCtrl*)CurrentList();
	int nStart = pList->GetNextItem(-1, LVNI_SELECTED);
	int nEnd = pList->GetItemCount() - 1;
	CString strFind, strName;
	GetDlgItemText(IDC_EDIT_FIND, strFind);
	strFind.MakeLower();

	int i = nStart; //탐색 시작 위치, 바로 ++ 되므로 nStart 다음 항목부터 검색
	if (i == nEnd) i = -1; //마지막 아이템인 경우 맨 앞부터 탐색
	do
	{
		i++;
		strName = pList->GetItemText(i, 0); //COL_NAME = 0
		if (strName.MakeLower().Find(strFind.MakeLower()) != -1)
		{
			pList->SetItemState(nStart, 0, LVIS_SELECTED | LVIS_FOCUSED);
			pList->SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			pList->EnsureVisible(i, FALSE);
			break;// 다 찾았으므로 
		}
		if (i == nEnd) i = -1; //끝을 넘어가면 맨 앞으로
	} 	
	while (i != nStart); //출발점으로 돌아오면 종료 
}


BOOL CDlgTabView::OnEraseBkgnd(CDC* pDC)
{
	/*CRect rc;
	GetClientRect(rc);
	pDC->FillSolidRect(rc, m_tvo.clrBk);
	return TRUE;*/
	return CDialogEx::OnEraseBkgnd(pDC);
}


void CDlgTabView::OnDropFiles(HDROP hDropInfo)
{
	CWnd* pList = CurrentList();
	pList->SendMessage(WM_DROPFILES, (WPARAM)hDropInfo, 0);
	/*if (CurrentListType() == TABTYPE_CUSTOM_LIST)
	{
		CurrentList()->SendMessage(WM_COMMAND, wParam, lParam);
	}
	else
	{
		CMyShellListCtrl* pList = (CMyShellListCtrl*)CurrentList();
		pList->DisplayParentFolder();
	}*/

	CDialogEx::OnDropFiles(hDropInfo);
}

void CDlgTabView::UpdateMsgBarFromList()
{
	if (::IsWindow(GetSafeHwnd()) == FALSE) return;
	CMFCListCtrl* pWnd = (CMFCListCtrl*)CurrentList();
	if (pWnd == NULL || ::IsWindow(pWnd->GetSafeHwnd()) == FALSE) return;

	CString strBarMsg;
	if (CurrentListType() == TABTYPE_CUSTOM_LIST)
	{
		CFileListCtrl* pList = (CFileListCtrl*)pWnd;
		strBarMsg = pList->GetBarString();
	}
	else
	{
		CMyShellListCtrl* pList = (CMyShellListCtrl*)pWnd;
		strBarMsg = pList->GetBarString();
	}
	SetDlgItemText(IDC_ST_BAR, strBarMsg);
}

void CDlgTabView::PathArrayImport(CStringArray& aPath)
{
	for (int i = 0; i < aPath.GetSize(); i++)
	{
		m_listShortCut.InsertPath(-1, aPath.GetAt(i));
	}
}

void CDlgTabView::PathArrayExport(CStringArray& aPath)
{
	aPath.RemoveAll();
	for (int i = 0; i < m_listShortCut.GetItemCount(); i++)
	{
		aPath.Add(m_listShortCut.GetItemText(i, 1));
	}
}


void CDlgTabView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_MENU_TAB);
	CMenu* pMenu = menu.GetSubMenu(0);
	pMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
}


//현재 탭을 한칸씩 이동한다.
void CDlgTabView::MoveCurrentTab(BOOL bRight)
{
	if (m_nCurrentTab < 0 || m_nCurrentTab >= m_aTabInfo.GetSize()) return;
	int nOldTab = -1, nNewTab = -1;
	if (bRight) //오른쪽으로
	{
		if (m_nCurrentTab == (m_aTabInfo.GetSize() - 1)) return; //제일 오른쪽인 경우
		nOldTab = m_nCurrentTab;
		nNewTab = m_nCurrentTab + 1;
	}
	else // 왼쪽으로
	{
		if (m_nCurrentTab == 0) return; //제일 왼인 경우
		nOldTab = m_nCurrentTab;
		nNewTab = m_nCurrentTab - 1;
	}
	if (nOldTab == -1 || nNewTab == -1) return;
	PathTabInfo ptiOld = m_aTabInfo.GetAt(nOldTab);
	PathTabInfo ptiNew = m_aTabInfo.GetAt(nNewTab);
	m_aTabInfo.SetAt(nOldTab, ptiNew);
	m_aTabInfo.SetAt(nNewTab, ptiOld);

	TCITEM tiOld, tiNew;
	TCHAR name1[MAX_PATH]; ZeroMemory(name1, MAX_PATH * sizeof(TCHAR));
	TCHAR name2[MAX_PATH]; ZeroMemory(name2, MAX_PATH * sizeof(TCHAR));
	tiOld.mask = TCIF_IMAGE | TCIF_TEXT;
	tiOld.pszText = name1;
	tiOld.cchTextMax = MAX_PATH;
	tiNew.mask = TCIF_IMAGE | TCIF_TEXT;
	tiNew.pszText = name2;
	tiNew.cchTextMax = MAX_PATH;
	m_tabPath.GetItem(nOldTab, &tiOld);
	m_tabPath.GetItem(nNewTab, &tiNew);
	m_tabPath.SetItem(nNewTab, &tiOld);
	m_tabPath.SetItem(nOldTab, &tiNew);
	SetCurrentTab(nNewTab);
}


