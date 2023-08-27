#pragma once
#include <afxlistctrl.h>
class CShortCutList : public CMFCListCtrl
{
public:
    CShortCutList();
    virtual ~CShortCutList();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual COLORREF OnGetCellTextColor(int nRow, int nColumn);
    virtual COLORREF OnGetCellBkColor(int nRow, int nColumn);
    COLORREF ApplyColorRule(int nRow, int nColumn, BOOL bBk);
    void OpenSelectedItem();
    void InsertPath(int nItem, CString strPath);
    inline BOOL IsDir(int nItem) { return (GetItemData(nItem) & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE; };
    inline CString GetItemFullPath(int nItem) { return GetItemText(nItem, 1); };

    int CMD_OpenFolderByShortCut;

public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnLvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};

