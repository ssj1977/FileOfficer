#pragma once
#include "CFileListCtrl_Base.h"

#ifndef IDM_LIST_MSG
#define IDM_SEARCH_MSG 55100
#endif

#ifndef SearchCriteria
struct SearchCriteria;
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
	void SelectAllItems();
	void OpenSelectedItem(BOOL bMulti = TRUE);
	void OpenSelectedParent(BOOL bUseTab);
	void RemoveSelected();
	void DeleteSelected(BOOL bRecycle = TRUE);
	virtual void Sort(int iColumn, BOOL bAscending = TRUE, BOOL bAdd = FALSE);
	SearchCriteria m_SC;
	//SearchCriteria 에서 추출 및 변환되는 정보
	CStringArray m_aNameMatch; // 이름 조건 토큰
	CStringArray m_aExtMatch; // 확장자 조건 토큰
	COleDateTime m_dtFrom; // 일시 조건 (시작시점)
	COleDateTime m_dtUntil; // 일시 조건 (종료시점)
	BOOL m_bSizeMin; // 크기 조건 (최소)
	BOOL m_bSizeMax; // 크기 조건 (최대)
	ULONGLONG m_sizeMin;
	ULONGLONG m_sizeMax;

	//작동 상태 처리용 정보
	BOOL m_bWorking; // 검색 쓰레드가 동작중일때 TRUE
	BOOL m_bBreak; //검색을 중단할때 쓰는 플래그
	CString m_strMsg; //상태창에 표시할 메시지

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHdnItemclick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
};

