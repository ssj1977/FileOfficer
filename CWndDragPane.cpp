#include "pch.h"
#include "CWndDragPane.h"

CWndDragPane::CWndDragPane()
{
	m_pWnd1 = NULL;
	m_pWnd2 = NULL;
	m_bDragging = FALSE;
	m_nBarPos = 0;
	m_nBarSize = 8;
	m_bVertical = TRUE;
	m_crBackGround = GetSysColor(COLOR_WINDOW);
	m_crBar = GetSysColor(COLOR_3DFACE);
	//m_crBackGround = RGB(255, 0, 0);
}

CWndDragPane::~CWndDragPane()
{
}

BOOL CWndDragPane::CreateWnd(BOOL bVertical, CWnd* pParentWnd, UINT nID)
{
	DWORD dwStyle = WS_VISIBLE | WS_CHILD;
	RECT rect = CRect(0, 0, 0, 0);
	m_bVertical = bVertical;
	return CWnd::Create(NULL, NULL, dwStyle, rect, pParentWnd, nID);
}

void CWndDragPane::SetPaneWnd(CWnd* pWnd1, CWnd* pWnd2)
{
	m_pWnd1 = pWnd1;
	m_pWnd2 = pWnd2;
	if (m_pWnd1 && ::IsWindow(m_pWnd1->GetSafeHwnd()))
	{
		m_pWnd1->SetParent(this);
	}
	if (m_pWnd2 && ::IsWindow(m_pWnd2->GetSafeHwnd()))
	{
		m_pWnd2->SetParent(this);
	}
}


BEGIN_MESSAGE_MAP(CWndDragPane, CWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void CWndDragPane::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (GetBarRect().PtInRect(point))
	{
		SetCapture();
		m_bDragging = TRUE;
	}
	CWnd::OnLButtonDown(nFlags, point);
}


void CWndDragPane::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDragging = TRUE)
	{
		ReleaseCapture();
		m_bDragging = FALSE;
	}
	CWnd::OnLButtonUp(nFlags, point);
}

BOOL CWndDragPane::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nHitTest == HTCLIENT || nHitTest == HTBORDER)
	{
		if (m_bVertical)	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
		else				::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
		return TRUE;
	}
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}


void CWndDragPane::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDragging)
	{
		int nNewBarPos = m_bVertical ? point.y : point.x;
		int nGap = nNewBarPos - m_nBarPos;
		CRect rc;
		GetClientRect(rc);
		if (abs(nGap) > (m_nBarSize / 2) && nNewBarPos > 0 && nNewBarPos < (rc.bottom - m_nBarSize))
		{
			m_nBarPos = nNewBarPos;
			ArrangeCtrl();
		}
	}
	CWnd::OnMouseMove(nFlags, point);
}

CRect CWndDragPane::GetBarRect()
{
	CRect rcBar;
	GetClientRect(rcBar);
	if (m_bVertical)
	{
		if (m_nBarPos < rcBar.left || m_nBarPos > (rcBar.right - m_nBarSize))
		{
			m_nBarPos = (rcBar.left + rcBar.right - m_nBarSize) / 2;
		}
		rcBar.left = m_nBarPos;
		rcBar.right = rcBar.left + m_nBarSize - 1;
	}
	else
	{
		if (m_nBarPos < rcBar.top || m_nBarPos > (rcBar.bottom - m_nBarSize))
		{
			m_nBarPos = (rcBar.top + rcBar.bottom - m_nBarSize) / 2;
		}
		rcBar.top = m_nBarPos;
		rcBar.bottom = rcBar.top + m_nBarSize - 1;
	}
	return rcBar;
}

BOOL CWndDragPane::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CRect rc;
	GetClientRect(rc);
	pDC->FillSolidRect(rc, m_crBackGround);
	pDC->FillSolidRect(GetBarRect(), m_crBar);
	return TRUE;
	//return CWnd::OnEraseBkgnd(pDC);
}

void CWndDragPane::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	ArrangeCtrl();
}


void CWndDragPane::ArrangeCtrl()
{
	CRect rc1, rc2;
	GetClientRect(rc1); rc2 = rc1;
	if (m_bVertical)
	{
		rc1.right = m_nBarPos - 1;
		rc2.left = m_nBarPos + m_nBarSize;
	}
	else
	{
		rc1.bottom = m_nBarPos - 1;
		rc2.top = m_nBarPos + m_nBarSize;
	}
	if (m_pWnd1 && ::IsWindow(m_pWnd1->GetSafeHwnd()))
	{
		m_pWnd1->MoveWindow(rc1);
	}
	if (m_pWnd2 && ::IsWindow(m_pWnd2->GetSafeHwnd()))
	{
		m_pWnd2->MoveWindow(rc1);
	}
}