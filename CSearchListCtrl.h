#pragma once
#include "CFileListCtrl_Base.h"
class CSearchListCtrl : public CFileListCtrl_Base
{
	DECLARE_DYNAMIC(CSearchListCtrl)

public:
	CSearchListCtrl();
	virtual ~CSearchListCtrl();
	void InitColumns();

	void FileSearch_Begin();
	void FileSearch_Do(CString strFolder);

	CString m_strStartFolder; // ó�� �˻��� ������ ��ġ
	CStringArray m_aNameMatch; // �̸� ����
	CStringArray m_aExtMatch; // Ȯ���� ����
	BOOL m_bNameAnd; // �̸� ������ �������϶� AND�� �������� OR�� ��������
	BOOL m_bExtAnd; // Ȯ���� ������ �������϶� AND�� �������� OR�� ��������
	COleDateTime m_dtFrom; // �Ͻ� ���� (���۽���)
	COleDateTime m_dtUntil; // �Ͻ� ���� (�������)
	BOOL m_bDateTimeFrom; // ���۽����� �������
	BOOL m_bDateTimeUntil; // ��������� �������
	BOOL m_bSizeMin; // ũ�� ���� (�ּ�) ��� ����
	BOOL m_bSizeMax; // ũ�� ���� (�ִ�) ��� ����
	ULONGLONG m_sizeMin; // ũ�� ���� (�ּ�)
	ULONGLONG m_sizeMax; // ũ�� ���� (�ִ�)
	BOOL m_bLocked; // ��� ���� ����
	BOOL m_bHidden; // ������ ���� ����
	BOOL m_bReadOnly; // �б� ���� ���� ����
	BOOL m_bEncrypted; // ��ȣȭ ���� ����
};

