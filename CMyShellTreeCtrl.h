#pragma once


// CMyShellTreeCtrl

class CMyShellTreeCtrl : public CMFCShellTreeCtrl
{
	DECLARE_DYNAMIC(CMyShellTreeCtrl)

public:
	CMyShellTreeCtrl();
	virtual ~CMyShellTreeCtrl();
	int CMD_TreeSelChanged;
	HTREEITEM m_current_item;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTvnSelchanging(NMHDR* pNMHDR, LRESULT* pResult);
};


