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
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	return COleDropTarget::OnDragOver(pWnd, pDataObject, dwKeyState, point);
}
