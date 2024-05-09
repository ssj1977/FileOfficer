#pragma once
#include "CFileListCtrl_Base.h"

#ifndef IDM_LIST_MSG
#define IDM_SEARCH_MSG 55100
#endif

class CSearchListCtrl : public CFileListCtrl_Base
{
	DECLARE_DYNAMIC(CSearchListCtrl)

public:
	CSearchListCtrl();
	virtual ~CSearchListCtrl();
	void InitColumns();

	void FileSearch_Begin();
	void FileSearch_Do(CString strFolder);
	static UINT FileSearch_RunThread(void* lParam);

	BOOL IsMatch_State(WIN32_FIND_DATA& fd, CString& fullpath);
	BOOL IsMatch_Name(WIN32_FIND_DATA& fd);
	BOOL IsMatch_Ext(WIN32_FIND_DATA& fd);
	BOOL IsMatch_Time(WIN32_FIND_DATA& fd);
	BOOL IsMatch_Size(WIN32_FIND_DATA& fd);

	CString GetItemFullPath(int nItem);
	BOOL GetDataForClipBoard(int nState, HGLOBAL& hgDrop, CString& strData);
	void ClipBoardExport(BOOL bMove);

	virtual void Sort(int iColumn, BOOL bAscending = TRUE, BOOL bAdd = FALSE);

	CString m_strStartFolder; // ó�� �˻��� ������ ��ġ
	CStringArray m_aNameMatch; // �̸� ����
	CStringArray m_aExtMatch; // Ȯ���� ����
	BOOL m_bNameAnd; // �̸� ������ �������϶� AND�� �������� OR�� ��������
	COleDateTime m_dtFrom; // �Ͻ� ���� (���۽���)
	COleDateTime m_dtUntil; // �Ͻ� ���� (�������)
	BOOL m_bDateTimeFrom; // ���۽����� �������
	BOOL m_bDateTimeUntil; // ��������� �������W
	BOOL m_bSizeMin; // ũ�� ���� (�ּ�) ��� ����
	BOOL m_bSizeMax; // ũ�� ���� (�ִ�) ��� ����
	ULONGLONG m_sizeMin; // ũ�� ���� (�ּ�)
	ULONGLONG m_sizeMax; // ũ�� ���� (�ִ�)
	BOOL m_bLocked; // ��� ���� ����
	BOOL m_bHidden; // ������ ���� ����
	BOOL m_bReadOnly; // �б� ���� ���� ����
	BOOL m_bEncrypted; // ��ȣȭ ���� ����
	BOOL m_bWorking; // �˻� �����尡 �������϶� TRUE
	BOOL m_bBreak; //�˻��� �ߴ��Ҷ� ���� �÷���
	CString m_strMsg; //����â�� ǥ���� �޽���

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHdnItemclick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

