#pragma once
#include "CFileListCtrl_Base.h"

class CShortCutList : public CFileListCtrl_Base
{
public:
    CShortCutList();
    virtual ~CShortCutList();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual COLORREF OnGetCellTextColor(int nRow, int nColumn);
    virtual COLORREF OnGetCellBkColor(int nRow, int nColumn);
    COLORREF ApplyColorRule(int nRow, int nColumn, BOOL bBk);
    void OpenSelectedItem();
    void RemoveSelectedItem();
    void ViewSelectedItemFolder();
    void DeleteSelectedItem();
    void Refresh();
    void MoveSelectedItem(BOOL bUp);
    void SwapItem(int n1, int n2);
    void InsertPath(int nItem, CString strPath);
    int m_nViewType;
    void SetViewType(int nType);
    inline BOOL IsDir(int nItem) { return (GetItemData(nItem) & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE; };
    inline BOOL IsItemExist(int nItem) { return PathFileExists(GetItemFullPath(nItem)); };
    inline CString GetItemFullPath(int nItem) { return GetItemText(nItem, 1); };
    int GetFileImageIndex(CString strPath, DWORD dwAttribute);

    //클립보드 관련 코드
    void ClipBoardExport(BOOL bCut);
    HGLOBAL GetOleDataForClipboard(int nState);

    //메시지 전달용 
    int CMD_OpenFolderByShortCut;

public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnLvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

