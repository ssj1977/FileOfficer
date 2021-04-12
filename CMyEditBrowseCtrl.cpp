#include "pch.h"
#include "CMyEditBrowseCtrl.h"


BEGIN_MESSAGE_MAP(CMyEditBrowseCtrl, CMFCEditBrowseCtrl)
    ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()


void CMyEditBrowseCtrl::SetBkColor(COLORREF clr)
{
    m_clrBk = clr;
    m_brush.DeleteObject();
    m_brush.CreateSolidBrush(clr);
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
