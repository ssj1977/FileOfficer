#include "pch.h"
#include "CMyDropTarget.h"


DROPEFFECT CMyDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	//HGLOBAL hDrag = pDataObject->GetGlobalData(CF_UNICODETEXT);
	//if (hDrag == NULL) return DROPEFFECT_NONE;
	return DROPEFFECT_MOVE;
	//return COleDropTarget::OnDragEnter(pWnd, pDataObject, dwKeyState, point);
}


DROPEFFECT CMyDropTarget::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return COleDropTarget::OnDragOver(pWnd, pDataObject, dwKeyState, point);
}
