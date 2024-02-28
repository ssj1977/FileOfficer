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

	BOOL IsMatch_State(WIN32_FIND_DATA& fd, CString& fullpath);
	BOOL IsMatch_Name(WIN32_FIND_DATA& fd);
	BOOL IsMatch_Ext(WIN32_FIND_DATA& fd);
	BOOL IsMatch_Time(WIN32_FIND_DATA& fd);
	BOOL IsMatch_Size(WIN32_FIND_DATA& fd);

	CString m_strStartFolder; // 처음 검색을 시작할 위치
	CStringArray m_aNameMatch; // 이름 조건
	CStringArray m_aExtMatch; // 확장자 조건
	BOOL m_bNameAnd; // 이름 조건이 여러개일때 AND로 적용할지 OR로 적용할지
	BOOL m_bExtAnd; // 확장자 조건이 여러개일때 AND로 적용할지 OR로 적용할지
	COleDateTime m_dtFrom; // 일시 조건 (시작시점)
	COleDateTime m_dtUntil; // 일시 조건 (종료시점)
	BOOL m_bDateTimeFrom; // 시작시점을 사용할지
	BOOL m_bDateTimeUntil; // 종료시점을 사용할지
	BOOL m_bSizeMin; // 크기 조건 (최소) 사용 여부
	BOOL m_bSizeMax; // 크기 조건 (최대) 사용 여부
	ULONGLONG m_sizeMin; // 크기 조건 (최소)
	ULONGLONG m_sizeMax; // 크기 조건 (최대)
	BOOL m_bLocked; // 잠긴 파일 여부
	BOOL m_bHidden; // 숨겨진 파일 여부
	BOOL m_bReadOnly; // 읽기 전용 파일 여부
	BOOL m_bEncrypted; // 암호화 파일 여부
};

