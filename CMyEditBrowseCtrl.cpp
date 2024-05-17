#include "pch.h"
#include "CMyEditBrowseCtrl.h"


BEGIN_MESSAGE_MAP(CMyEditBrowseCtrl, CMFCEditBrowseCtrl)
    ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()


CMyEditBrowseCtrl::CMyEditBrowseCtrl() : CMFCEditBrowseCtrl()
{
    m_clrBk = RGB(0,0,0);
    m_clrText = RGB(255, 255, 255);
    m_brush.CreateSolidBrush(m_clrBk);
    CMD_UpdateList = 0;
}

CMyEditBrowseCtrl::~CMyEditBrowseCtrl()
{
    m_brush.DeleteObject();
}

void CMyEditBrowseCtrl::SetBkColor(COLORREF clr)
{
    m_clrBk = clr;
    m_brush.DeleteObject();
    m_brush.CreateSolidBrush(clr);
    CMD_UpdateList = 0;
}

void CMyEditBrowseCtrl::SetTextColor(COLORREF clr)
{
    m_clrText = clr;
}

HBRUSH CMyEditBrowseCtrl::CtlColor(CDC* pDC, UINT)
{
    if (!m_brush.GetSafeHandle()) return GetSysColorBrush(COLOR_WINDOW);
    pDC->SetBkColor(m_clrBk);
    pDC->SetTextColor(m_clrText);
    return m_brush;
}

void CMyEditBrowseCtrl::OnAfterUpdate()
{
    GetParent()->PostMessageW(WM_COMMAND, CMD_UpdateList, 0);
}