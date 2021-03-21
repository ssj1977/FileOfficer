#include "pch.h"
#include "CFileListContextMenu.h"
#include <Shlobj.h>
#include <Objbase.h>

IContextMenu2* g_pIContext2 = NULL;
IContextMenu3* g_pIContext3 = NULL;

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

BOOL CFileListContextMenu::GetContextMenu(void** ppContextMenu, int& iMenuType)
{
	*ppContextMenu = NULL;
	LPCONTEXTMENU pMenu = NULL;
	HRESULT hr = m_psfFolder->GetUIObjectOf(NULL, (UINT)m_paPath->GetSize(), (LPCITEMIDLIST*)m_aPIDL, IID_IContextMenu, NULL, (void**)ppContextMenu);
	if (FAILED(hr)) return FALSE;
	return TRUE; // success
}

#define MIN_ID 1
#define MAX_ID 10000

UINT CFileListContextMenu::ShowContextMenu(CWnd* pWnd, CPoint pt)
{
	int iMenuType = 0;	// to know which version of IContextMenu is supported
	LPCONTEXTMENU pContextMenu;	// common pointer to IContextMenu and higher version interface

	if (!GetContextMenu((void**)&pContextMenu, iMenuType)) return 0;	// something went wrong

	if (!m_pMenu)
	{
		m_pMenu = new CMenu;
		m_pMenu->CreatePopupMenu();
	}
	// lets fill the our popupmenu  
	pContextMenu->QueryContextMenu(m_pMenu->m_hMenu, m_pMenu->GetMenuItemCount(), MIN_ID, MAX_ID, CMF_NORMAL | CMF_EXPLORE);

	// subclass window to handle menurelated messages in CShellContextMenu 
	WNDPROC OldWndProc;
	if (iMenuType > 1)	// only subclass if its version 2 or 3
	{
		OldWndProc = (WNDPROC)SetWindowLongPtr(pWnd->m_hWnd, GWLP_WNDPROC, (LONG_PTR)&HookWndProc);
		if (iMenuType == 2)	g_pIContext2 = (LPCONTEXTMENU2)pContextMenu;
		else g_pIContext3 = (LPCONTEXTMENU3)pContextMenu;
	}
	else OldWndProc = NULL;

	UINT idCommand = m_pMenu->TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN, pt.x, pt.y, pWnd);

	if (OldWndProc) // unsubclass
		SetWindowLong(pWnd->m_hWnd, GWLP_WNDPROC, (LONG)OldWndProc);

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
	CMINVOKECOMMANDINFO cmi = { 0 };
	cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
	cmi.lpVerb = (LPSTR)MAKEINTRESOURCE(idCommand);
	cmi.nShow = SW_SHOWNORMAL;

	pContextMenu->InvokeCommand(&cmi);
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
		if (g_pIContext2) g_pIContext2->HandleMenuMsg(message, wParam, lParam);
		else g_pIContext3->HandleMenuMsg(message, wParam, lParam);
		return (message == WM_INITMENUPOPUP ? 0 : TRUE); // inform caller that we handled WM_INITPOPUPMENU by ourself
		break;
	default:
		break;
	}
	// call original WndProc of window to prevent undefined bevhaviour of window
	return ::CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("OldWndProc")), hWnd, message, wParam, lParam);
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


void CFileListContextMenu::SetPathArray(CStringArray& aPath)
{
	if (aPath.GetSize() == 0) return;
	m_paPath = &aPath;
	if (m_psfFolder) m_psfFolder->Release(); //(m_psfFolder && bDelete)
	m_psfFolder = NULL;
	HRESULT hr = S_OK;
	LPITEMIDLIST pidl = NULL;
	IShellFolder* psfDesktop = NULL;
	if (FAILED(SHGetDesktopFolder(&psfDesktop))) return; // Default IShellFolder to Call ParseDisplayName
	CString strTemp = aPath.GetAt(0);
	hr = psfDesktop->ParseDisplayName(NULL, 0, strTemp.GetBuffer(0), NULL, &pidl, NULL); // pidl = Absolute PIDL of the first path
	strTemp.ReleaseBuffer();
	if (FAILED(hr)) { psfDesktop->Release(); return; }
	// m_psfFolder = IShellfolder of the parent of the first path ==> For ShowContextMenu
	hr = SHBindToParent(pidl, IID_IShellFolder, (void**)&m_psfFolder, NULL);
	CoTaskMemFree(pidl);

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

