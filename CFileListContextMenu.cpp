#include "pch.h"
#include "CFileListContextMenu.h"
#include "resource.h"
#include <Shlobj.h>
#include <Objbase.h>

IContextMenu2* g_pIContext2 = NULL;
IContextMenu3* g_pIContext3 = NULL;
WNDPROC g_oldWndProc = NULL;

UINT GetPIDLSize(LPCITEMIDLIST pidl)
{
	if (!pidl)
		return 0;
	int nSize = 0;
	LPITEMIDLIST pidlTemp = (LPITEMIDLIST)pidl;
	while (pidlTemp->mkid.cb)
	{
		nSize += pidlTemp->mkid.cb;
		pidlTemp = (LPITEMIDLIST)(((LPBYTE)pidlTemp) + pidlTemp->mkid.cb);
	}
	return nSize;
}

/*LPITEMIDLIST CopyPIDL(LPCITEMIDLIST pidl, int cb)
{
	if (cb == -1)
		cb = GetPIDLSize(pidl); // Calculate size of list.

	LPITEMIDLIST pidlRet = (LPITEMIDLIST)calloc(cb + sizeof(USHORT), sizeof(BYTE));
	if (pidlRet)
		CopyMemory(pidlRet, pidl, cb);
	return (pidlRet);
}*/

CFileListContextMenu::CFileListContextMenu()
{
	m_pMenu = NULL;
	m_paPath = NULL;
	m_psfFolder = NULL; // To use GetUIObjectOf() to get a menu
	m_aPIDL = NULL;
	m_pParent = NULL;
}

CFileListContextMenu::~CFileListContextMenu()
{
	if (m_psfFolder) m_psfFolder->Release();
	m_psfFolder = NULL;
	FreePIDLArray(m_aPIDL);
	m_aPIDL = NULL;
	if (m_pMenu) delete m_pMenu;
	m_pMenu = NULL;
}

CMenu* CFileListContextMenu::GetMenu()
{
	return m_pMenu;
}

BOOL CFileListContextMenu::GetContextMenu(void** ppContextMenu, CWnd* pWnd)
{
	if (m_psfFolder == NULL) return FALSE;
	*ppContextMenu = NULL;
	LPCONTEXTMENU pMenu = NULL;
	if (m_paPath == NULL || m_paPath->GetSize() == 0)
	{
		HRESULT hr = m_psfFolder->CreateViewObject(m_pParent->GetSafeHwnd(), IID_IContextMenu, (void**)ppContextMenu);
	}
	else
	{
		DEFCONTEXTMENU dcm =
		{
			//(HWND)m_pMenu->m_hMenu, //hWnd, //이렇게 하면 디펜더 아이콘이 커지는 오류가 있어 아래와 같이 수정
			pWnd->m_hWnd,
			NULL, // contextMenuCB
			NULL, // pidlFolder,
			m_psfFolder, //IShellFolder 
			(UINT)m_paPath->GetSize(),    // Item Count cidl;
			(LPCITEMIDLIST*)m_aPIDL, //(LPCITEMIDLIST*)pidlChilds,
			NULL, // *punkAssociationInfo
			NULL, // cKeys
			NULL // *aKeys
		};
		HRESULT hr = SHCreateDefaultContextMenu(&dcm, IID_IContextMenu, (void**)ppContextMenu);
		if (FAILED(hr)) return FALSE;
		return TRUE;
	}
	//HRESULT hr = m_psfFolder->GetUIObjectOf(NULL, (UINT)m_paPath->GetSize(), (LPCITEMIDLIST*)m_aPIDL, IID_IContextMenu, NULL, (void**)ppContextMenu);
	//if (FAILED(hr)) return FALSE;
	return TRUE; // success
}

#define MIN_ID 1
#define MAX_ID 0x7FFF

UINT CFileListContextMenu::ShowContextMenu(CWnd* pWnd, CPoint pt)
{
	LPCONTEXTMENU pContextMenu;	// common pointer to IContextMenu and higher version interface

	if (!m_pMenu)
	{
		m_pMenu = new CMenu;
		m_pMenu->CreatePopupMenu();
	}
	//시스템 메뉴
	if (!GetContextMenu((void**)&pContextMenu, pWnd)) return 0;
	
	// lets fill the our popupmenu  
	pContextMenu->QueryContextMenu(m_pMenu->m_hMenu, m_pMenu->GetMenuItemCount(), MIN_ID, MAX_ID, CMF_NORMAL | CMF_EXTENDEDVERBS); //CMF_NORMAL | CMF_EXPLORE);
	// subclass window to handle menurelated messages in CShellContextMenu 
	pContextMenu->QueryInterface(IID_IContextMenu2, (void**)&g_pIContext2); //test
	pContextMenu->QueryInterface(IID_IContextMenu3, (void**)&g_pIContext3); //test

	if (g_pIContext2 != NULL || g_pIContext3 !=NULL)
	{
		g_oldWndProc = (WNDPROC)SetWindowLongPtr(pWnd->m_hWnd, GWLP_WNDPROC, (LONG_PTR)&HookWndProc);
	}
	else
	{
		g_oldWndProc = NULL;
	}
	//자체 커스텀 메뉴
	MENUITEMINFO mi;
	CString strMenuString;
	if (m_pMenu->GetMenuItemCount() > 0)
	{
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_FTYPE;
		mi.fType = MFT_SEPARATOR;
		m_pMenu->InsertMenuItem(m_pMenu->GetMenuItemCount(), &mi, TRUE);
		// 새 폴더 메뉴와 탐색기에서 열기 메뉴를 항상 하나 넣어준다
		strMenuString.LoadStringW(IDS_NEW_FOLDER);
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STRING | MIIM_STATE;
		mi.fType = MFT_STRING;
		mi.fState = MFS_ENABLED;
		mi.wID = IDM_NEW_FOLDER;
		mi.dwTypeData = strMenuString.GetBuffer();
		m_pMenu->InsertMenuItem(m_pMenu->GetMenuItemCount(), &mi, TRUE);
		strMenuString.ReleaseBuffer();

		strMenuString.LoadStringW(IDS_OPENFOLDER);
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STRING | MIIM_STATE;
		mi.fType = MFT_STRING;
		mi.fState = MFS_ENABLED;
		mi.wID = IDM_OPENFOLDER;
		mi.dwTypeData = strMenuString.GetBuffer();
		m_pMenu->InsertMenuItem(m_pMenu->GetMenuItemCount(), &mi, TRUE);
		strMenuString.ReleaseBuffer();
	}
	if (m_paPath->GetSize() == 0 && IsClipboardFormatAvailable(CF_HDROP) != FALSE)
	{
		strMenuString.LoadStringW(IDS_PASTE_FILE);
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STRING | MIIM_STATE;
		mi.fType = MFT_STRING;
		mi.fState = MFS_ENABLED;
		mi.wID = IDM_PASTE_FILE;
		mi.dwTypeData = strMenuString.GetBuffer();
		m_pMenu->InsertMenuItem(m_pMenu->GetMenuItemCount(), &mi, TRUE);
		strMenuString.ReleaseBuffer();
	}
	if (m_paPath->GetSize() > 0)
	{ 
		strMenuString.LoadStringW(IDS_RENAME);
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STRING | MIIM_STATE;
		mi.fType = MFT_STRING;
		mi.fState = MFS_ENABLED;
		mi.wID = IDM_RENAME_FILE;
		mi.dwTypeData = strMenuString.GetBuffer();
		m_pMenu->InsertMenuItem(m_pMenu->GetMenuItemCount(), &mi, TRUE);
		strMenuString.ReleaseBuffer();

		strMenuString.LoadStringW(IDS_CONVERT_NFD);
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STRING | MIIM_STATE;
		mi.fType = MFT_STRING;
		mi.fState = MFS_ENABLED;
		mi.wID = IDM_CONVERT_NFD;
		mi.dwTypeData = strMenuString.GetBuffer();
		m_pMenu->InsertMenuItem(m_pMenu->GetMenuItemCount(), &mi, TRUE);
		strMenuString.ReleaseBuffer();

		strMenuString.LoadStringW(IDS_CHECK_LOCKED);
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STRING | MIIM_STATE;
		mi.fType = MFT_STRING;
		mi.fState = MFS_ENABLED;
		mi.wID = IDM_CHECK_LOCKED;
		mi.dwTypeData = strMenuString.GetBuffer();
		m_pMenu->InsertMenuItem(m_pMenu->GetMenuItemCount(), &mi, TRUE);
		strMenuString.ReleaseBuffer();
	}


	UINT idCommand = m_pMenu->TrackPopupMenuEx(TPM_RETURNCMD | TPM_LEFTALIGN, pt.x, pt.y, pWnd, NULL);
	//test//
	if (g_pIContext2) 
	{
		g_pIContext2->Release();
		g_pIContext2 = NULL;
	}
	if (g_pIContext3) 
	{
		g_pIContext3->Release();
		g_pIContext3 = NULL;
	}
	//test//

	if (g_oldWndProc)
	{
		SetWindowLongPtr(pWnd->m_hWnd, GWLP_WNDPROC, (LONG_PTR)g_oldWndProc);  // unsubclass
		g_oldWndProc = NULL;
	}

	if (idCommand >= MIN_ID && idCommand <= MAX_ID)	// see if returned idCommand belongs to shell menu entries
	{
		InvokeCommand(pContextMenu, idCommand - MIN_ID);	// execute related command
		idCommand = 0;
	}
	pContextMenu->Release();
	g_pIContext2 = NULL;
	g_pIContext3 = NULL;
	return idCommand;
}

void CFileListContextMenu::InvokeCommand(LPCONTEXTMENU pContextMenu, UINT idCommand)
{
	CMINVOKECOMMANDINFOEX cmi = {};
	cmi.cbSize = sizeof(CMINVOKECOMMANDINFOEX);
	cmi.fMask = 0x00004000;
	cmi.hwnd = m_pParent->GetSafeHwnd();
	cmi.lpVerb = (LPSTR)MAKEINTRESOURCE(idCommand);
	cmi.lpVerbW = (LPWSTR)MAKEINTRESOURCE(idCommand);;
	cmi.nShow = SW_SHOWNORMAL;
	pContextMenu->InvokeCommand((LPCMINVOKECOMMANDINFO)&cmi);
}

// 윈도우 쉘의 기능을 바로 사용하는 방법으로 
// 콘텍스트 메뉴의 특정 항목을 바로 실행시킨다.
// 예를 들어 Ctrl+C 로 파일을 복사할때 클립보드에 pIDL도 복사
void CFileListContextMenu::RunShellMenuCommand(CWnd* pWnd, UINT idCommand)
{
	LPCONTEXTMENU pContextMenu;	// common pointer to IContextMenu and higher version interface
	if (!m_pMenu)
	{
		m_pMenu = new CMenu;
		m_pMenu->CreatePopupMenu();
	}
	//시스템 메뉴
	if (!GetContextMenu((void**)&pContextMenu, pWnd)) return;
	// lets fill the our popupmenu  
	pContextMenu->QueryContextMenu(m_pMenu->m_hMenu, m_pMenu->GetMenuItemCount(), MIN_ID, MAX_ID, CMF_NORMAL | CMF_EXTENDEDVERBS); //CMF_NORMAL | CMF_EXPLORE);
	// subclass window to handle menurelated messages in CShellContextMenu 
	pContextMenu->QueryInterface(IID_IContextMenu2, (void**)&g_pIContext2); //test
	pContextMenu->QueryInterface(IID_IContextMenu3, (void**)&g_pIContext3); //test
	//test//
	if (g_pIContext2)
	{
		g_pIContext2->Release();
		g_pIContext2 = NULL;
	}
	if (g_pIContext3)
	{
		g_pIContext3->Release();
		g_pIContext3 = NULL;
	}
	if (idCommand >= MIN_ID && idCommand <= MAX_ID)	// see if returned idCommand belongs to shell menu entries
	{
		InvokeCommand(pContextMenu, idCommand - MIN_ID);	// execute related command
		idCommand = 0;
	}
	pContextMenu->Release();
	g_pIContext2 = NULL;
	g_pIContext3 = NULL;
}



LRESULT CALLBACK CFileListContextMenu::HookWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MENUCHAR:	// only supported by IContextMenu3
		if (g_pIContext3)
		{
			LRESULT lResult = 0;
			g_pIContext3->HandleMenuMsg2(message, wParam, lParam, &lResult);
			return (lResult);
		}
		break;
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		if (wParam)	break; // if wParam != 0 then the message is not menu-related
	case WM_INITMENUPOPUP:
		if (g_pIContext3)
		{
			LRESULT lres = NULL;
			if (SUCCEEDED(g_pIContext3->HandleMenuMsg2(message, wParam, lParam, &lres)))
				return lres;
		}
		else if (g_pIContext2)
		{
			LRESULT lres = NULL;
			if (SUCCEEDED(g_pIContext2->HandleMenuMsg(message, wParam, lParam)))
				return lres;
		}
		break;
	default:
		break;
	}
	// call original WndProc of window to prevent undefined bevhaviour of window
	return ::CallWindowProc(g_oldWndProc, hWnd, message, wParam, lParam);
}

void CFileListContextMenu::FreePIDLArray(LPITEMIDLIST* aPIDL)
{
	if (!aPIDL) return;
	//int iSize = _msize(aPIDL) / sizeof(LPITEMIDLIST);
	for (int i = 0; i < m_paPath->GetSize(); i++)
	{
		//free(aPIDL[i]);
		CoTaskMemFree(aPIDL[i]);
	}
	CoTaskMemFree(aPIDL);
}

UINT CFileListContextMenu::GetPIDLSize(LPCITEMIDLIST pidl)
{
	if (!pidl) return 0;
	int nSize = 0;
	LPITEMIDLIST pidlTemp = (LPITEMIDLIST)pidl;
	while (pidlTemp->mkid.cb)
	{
		nSize += pidlTemp->mkid.cb;
		pidlTemp = (LPITEMIDLIST)(((LPBYTE)pidlTemp) + pidlTemp->mkid.cb);
	}
	return nSize;
}

/*LPITEMIDLIST CFileListContextMenu::CopyPIDL(LPCITEMIDLIST pidl, int cb)
{
	if (cb == -1) cb = GetPIDLSize(pidl); // Calculate size of list.
	LPITEMIDLIST pidlRet = (LPITEMIDLIST)calloc(cb + sizeof(USHORT), sizeof(BYTE));
	if (pidlRet) CopyMemory(pidlRet, pidl, cb);
	return pidlRet;
}*/


void CFileListContextMenu::SetPathArray(CString strFolder, CStringArray& aPath)
{
	CString strTemp;
	//m_psfFolder에 쉘 메뉴를 실행할 폴더에 대한 IShellFolder* 가 보관된다
	if (m_psfFolder) m_psfFolder->Release(); //(m_psfFolder && bDelete)
	m_psfFolder = NULL;
	HRESULT hr = S_OK;
	LPITEMIDLIST pidl = NULL;
	//데스크탑에 대한 IShellFolder* 를 구하고 이를 이용해서 폴더의 절대 식별자(pidl)를 구한다.
	IShellFolder* psfDesktop = NULL;
	if (FAILED(SHGetDesktopFolder(&psfDesktop))) return; // Default IShellFolder to Call ParseDisplayName
	hr = psfDesktop->ParseDisplayName(NULL, 0, strFolder.GetBuffer(0), NULL, &pidl, NULL); // pidl = Absolute PIDL of the first path
	strFolder.ReleaseBuffer();
	if (FAILED(hr)) { psfDesktop->Release(); return; }
	//현재 폴더
	hr = SHBindToObject(NULL, pidl, NULL, IID_IShellFolder, (void**)&m_psfFolder);
	m_paPath = &aPath;
	if (aPath.GetSize() == 0)
	{
		FreePIDLArray(m_aPIDL);
		return;
	}
	// m_psfFolder = IShellfolder of the parent of the first path ==> For ShowContextMenu
	//hr = SHBindToParent(pidl, IID_IShellFolder, (void**)&m_psfFolder, NULL);
	//CoTaskMemFree(pidl);

	// Enumerate Relative PIDL 
	IShellFolder* psfFolder = NULL;
	LPITEMIDLIST pidl_temp = NULL;	// relative pidl
	FreePIDLArray(m_aPIDL);
	INT_PTR nCount = aPath.GetSize();
	m_aPIDL = (LPITEMIDLIST*)CoTaskMemAlloc(nCount * sizeof(LPITEMIDLIST));
	if (m_aPIDL)
	{
		ZeroMemory(m_aPIDL, nCount * sizeof(LPITEMIDLIST));
		for (int i = 0; i < nCount; i++)
		{
			strTemp = aPath.GetAt(i);
			hr = psfDesktop->ParseDisplayName(NULL, 0, strTemp.GetBuffer(0), NULL, &pidl, NULL);
			strTemp.ReleaseBuffer();
			if (SUCCEEDED(hr))
			{
				// get relative pidl via SHBindToParent
				SHBindToParent(pidl, IID_IShellFolder, (void**)&psfFolder, (LPCITEMIDLIST*)&pidl_temp);
				if (pidl_temp)
				{
					//m_aPIDL[i] = CopyPIDL(pidl_temp);
					m_aPIDL[i] = ILClone(pidl_temp);
				}
				psfFolder->Release();
			}
		}
	}
	psfDesktop->Release();
}

void  CFileListContextMenu::SetParent(CWnd* pWnd)
{
	m_pParent = pWnd;
}