#pragma once

class CFileListContextMenu
{
public:
	CFileListContextMenu();
	virtual ~CFileListContextMenu();
	void SetPathArray(CString strFolder, CStringArray& aPath);
	void FreePIDLArray(LPITEMIDLIST* aPIDL);
	CMenu* GetMenu();
	BOOL GetContextMenu(void** ppContextMenu, int& iMenuType);
	UINT ShowContextMenu(CWnd* pWnd, CPoint pt);
	static LRESULT CALLBACK HookWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void InvokeCommand(LPCONTEXTMENU pContextMenu, UINT idCommand);
	UINT GetPIDLSize(LPCITEMIDLIST pidl);
	LPITEMIDLIST CopyPIDL(LPCITEMIDLIST pidl, int cb = -1);
	void SetParent(CWnd* pWnd);

private:
	CMenu* m_pMenu;
	CStringArray* m_paPath;
	IShellFolder* m_psfFolder;
	LPITEMIDLIST* m_aPIDL;
	CWnd* m_pParent;
};

