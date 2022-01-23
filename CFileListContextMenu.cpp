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

LPITEMIDLIST CopyPIDL(LPCITEMIDLIST pidl, int cb)
{
	if (cb == -1)
		cb = GetPIDLSize(pidl); // Calculate size of list.

	LPITEMIDLIST pidlRet = (LPITEMIDLIST)calloc(cb + sizeof(USHORT), sizeof(BYTE));
	if (pidlRet)
		CopyMemory(pidlRet, pidl, cb);
	return (pidlRet);
}

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

BOOL CFileListContextMenu::GetContextMenu(void** ppContextMenu)
{
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
			(HWND)m_pMenu->m_hMenu, //hWnd,
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
	if (!GetContextMenu((void**)&pContextMenu)) return 0;

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
	
	//BOOL b = IsClipboardFormatAvailable(CF_HDROP); 
	if (m_paPath->GetSize() == 0 && IsClipboardFormatAvailable(CF_HDROP) != FALSE)
	{
		CString strMenuString = L"붙여넣기";
		MENUITEMINFO mi;
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STRING | MIIM_STATE;
		mi.fType = MFT_STRING;
		mi.fState = MFS_ENABLED;
		mi.wID = IDM_PASTE_FILE;
		mi.dwTypeData = strMenuString.GetBuffer();
		m_pMenu->InsertMenuItem(0, &mi, TRUE);
		strMenuString.ReleaseBuffer();

		mi.fMask = MIIM_FTYPE;
		mi.fType = MFT_SEPARATOR;
		m_pMenu->InsertMenuItem(1, &mi, TRUE);
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
	CMINVOKECOMMANDINFOEX cmi = { 0 };
	cmi.cbSize = sizeof(CMINVOKECOMMANDINFOEX);
	cmi.fMask = 0x00004000;
	cmi.hwnd = m_pParent->GetSafeHwnd();
	cmi.lpVerb = (LPSTR)MAKEINTRESOURCE(idCommand);
	cmi.lpVerbW = (LPWSTR)MAKEINTRESOURCE(idCommand);;
	cmi.nShow = SW_SHOWNORMAL;
	pContextMenu->InvokeCommand((LPCMINVOKECOMMANDINFO)&cmi);
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
		free(aPIDL[i]);
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

LPITEMIDLIST CFileListContextMenu::CopyPIDL(LPCITEMIDLIST pidl, int cb)
{
	if (cb == -1) cb = GetPIDLSize(pidl); // Calculate size of list.
	LPITEMIDLIST pidlRet = (LPITEMIDLIST)calloc(cb + sizeof(USHORT), sizeof(BYTE));
	if (pidlRet) CopyMemory(pidlRet, pidl, cb);
	return pidlRet;
}


void CFileListContextMenu::SetPathArray(CString strFolder, CStringArray& aPath)
{
	CString strTemp;
	if (m_psfFolder) m_psfFolder->Release(); //(m_psfFolder && bDelete)
	m_psfFolder = NULL;
	HRESULT hr = S_OK;
	LPITEMIDLIST pidl = NULL;
	IShellFolder* psfDesktop = NULL;
	if (FAILED(SHGetDesktopFolder(&psfDesktop))) return; // Default IShellFolder to Call ParseDisplayName
	hr = psfDesktop->ParseDisplayName(NULL, 0, strFolder.GetBuffer(0), NULL, &pidl, NULL); // pidl = Absolute PIDL of the first path
	strFolder.ReleaseBuffer();
	if (FAILED(hr)) { psfDesktop->Release(); return; }
	hr = SHBindToObject(NULL, pidl, NULL, IID_IShellFolder, (void**)&m_psfFolder);
	CoTaskMemFree(pidl);
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
					m_aPIDL[i] = CopyPIDL(pidl_temp);
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