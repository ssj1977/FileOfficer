#pragma once
#include <afxeditbrowsectrl.h>
class CMyEditBrowseCtrl : public CMFCEditBrowseCtrl
{
public:
    COLORREF m_clrBk;
    COLORREF m_clrText;
    CBrush m_brush;
    int CMD_UpdateList;
    void SetBkColor(COLORREF clr);
    void SetTextColor(COLORREF clr);
    DECLARE_MESSAGE_MAP()
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT /*nCtlColor*/);
    virtual void OnAfterUpdate();
};

