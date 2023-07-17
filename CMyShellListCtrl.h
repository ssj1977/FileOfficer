#pragma once


// CMyShellListCtrl

class CMyShellListCtrl : public CMFCShellListCtrl
{
	DECLARE_DYNAMIC(CMyShellListCtrl)

public:
	CMyShellListCtrl();
	virtual ~CMyShellListCtrl();

	CString GetItemFullPath(int nItem);
	void InsertPath(int nItem, CString strPath);

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


