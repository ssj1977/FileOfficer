#include "pch.h"
#include "CWndDragBar.h"


CWndDragBar::CWndDragBar()
{
	m_bDragging = FALSE;
	m_pBarPos = NULL;
	m_bVertical = TRUE;
	m_nParentCommand = IDM_ARRANGECTRL;
	m_crBackGround = GetSysColor(COLOR_3DFACE);
	//m_crBackGround = RGB(255, 0, 0);
}

CWndDragBar::~CWndDragBar()
{
}

BOOL CWndDragBar::CreateDragBar(BOOL bVertical, CWnd* pParentWnd, UINT nID)
{
	//DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_DLGFRAME | SS_NOTIFY | SS_ETCHEDFRAME;
	DWORD dwStyle = WS_VISIBLE | WS_CHILD;
	RECT rect = CRect (0,0,0,0);
	m_bVertical = bVertical;
	return CWnd::Create(NULL, NULL, dwStyle, rect, pParentWnd, nID);
}


BEGIN_MESSAGE_MAP(CWndDragBar, CWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void CWndDragBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	m_bDragging = TRUE;
	CWnd::OnLButtonDown(nFlags, point);
}


void CWndDragBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	m_bDragging = FALSE;
	CWnd::OnLButtonUp(nFlags, point);
}

BOOL CWndDragBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
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


void CWndDragBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDragging && m_pBarPos)
	{
		int& nBarPos = (*m_pBarPos);
		int nGap = m_bVertical ? point.y : point.x;
		int nNewBarPos = nBarPos + nGap;
		CRect rcWnd; 
		GetWindowRect(rcWnd);
		int nSize = m_bVertical ? rcWnd.Height() : rcWnd.Width();
		if (abs(nGap) > (nSize/2) && nNewBarPos > nSize)
		{
			nBarPos = nNewBarPos;
			GetParent()->SendMessage(WM_COMMAND, m_nParentCommand);
			//TRACE(L"BarPos: %d\n", nBarPos);
		}
	}
	CWnd::OnMouseMove(nFlags, point);
}


BOOL CWndDragBar::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CRect rc;
	GetClientRect(rc);
	pDC->FillSolidRect(rc, m_crBackGround);
	return TRUE;
	//return CWnd::OnEraseBkgnd(pDC);
}
