#include "pch.h"
#include "CMyDropTarget.h"
#include "CFileListCtrl.h"


DROPEFFECT CMyDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	//HGLOBAL hDrag = pDataObject->GetGlobalData(CF_UNICODETEXT);
	//if (hDrag == NULL) return DROPEFFECT_NONE;
	if ((dwKeyState & MK_CONTROL) == MK_CONTROL) return DROPEFFECT_COPY;
	return DROPEFFECT_MOVE;
	//return COleDropTarget::OnDragEnter(pWnd, pDataObject, dwKeyState, point);
}


DROPEFFECT CMyDropTarget::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if ((dwKeyState & MK_CONTROL) == MK_CONTROL) return DROPEFFECT_COPY;
	return DROPEFFECT_MOVE;
	//return COleDropTarget::OnDragOver(pWnd, pDataObject, dwKeyState, point);
}


void CMyDropTarget::OnDragLeave(CWnd* pWnd)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	COleDropTarget::OnDragLeave(pWnd);
}


BOOL CMyDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	CFileListCtrl* pList = (CFileListCtrl*)pWnd;
	FORMATETC etc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	HGLOBAL hg = pDataObject->GetGlobalData(CF_HDROP, &etc);
	if (hg == NULL) return FALSE;
	HDROP hdrop = (HDROP)GlobalLock(hg);
	if (hdrop == NULL)
	{
		GlobalUnlock(hg);
		return FALSE;
	}
	if ((dropEffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE)
	{
		pList->MyDropFiles(hdrop, TRUE);
	}
	else if ((dropEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY)
	{
		pList->MyDropFiles(hdrop, FALSE);
	}
	GlobalUnlock(hg);
	return TRUE;
	//return COleDropTarget::OnDrop(pWnd, pDataObject, dropEffect, point);
}


DROPEFFECT CMyDropTarget::OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if ((dwKeyState & MK_CONTROL) == MK_CONTROL) return DROPEFFECT_COPY;
	return DROPEFFECT_MOVE;
	//return COleDropTarget::OnDragScroll(pWnd, dwKeyState, point);
}
