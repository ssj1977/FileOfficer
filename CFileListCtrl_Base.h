#pragma once
#include <afxlistctrl.h>
#include <map>
typedef std::map<CString, int> CExtMap;
typedef std::map<CString, CString> CTypeMap;

#ifndef COL_COMP_STR
#define COL_COMP_STR 0
#define COL_COMP_PATH 1
#define COL_COMP_SIZE 2
#define COL_COMP_DRIVE 3
#endif

//파일 보기 리스트 컨트롤의 기본적인 함수와 속성을 포함한 클래스
//CFileListCtrl, CShortCutList 등에 쓰임
class CFileListCtrl_Base :  public CMFCListCtrl
{
	DECLARE_DYNAMIC(CFileListCtrl_Base)
protected:
//	DECLARE_MESSAGE_MAP()

public:
	CFileListCtrl_Base();
	virtual ~CFileListCtrl_Base();

	static HIMAGELIST GetImageListByType(int nIconType);
	static HIMAGELIST m_hSysImgList_SMALL;
	static HIMAGELIST m_hSysImgList_LARGE;
	static HIMAGELIST m_hSysImgList_EXTRALARGE;
	static HIMAGELIST m_hSysImgList_JUMBO;

	BOOL m_bUseFileIcon; //파일 아이콘을 표시할지 구분, 일부 시스템 아이콘 오류 문제 대응용
	int m_nIconType;
	HIMAGELIST m_hImageList;
	void SetIconType(int nIconType);
	static int GetFileImageIndex(CString strPath, DWORD dwAttribute);
	static int GetDriveImageIndex(int nDriveType);
	int GetFileImageIndexFromMap(CString strPath, DWORD dwAttribute);
	static CExtMap mapExt;  //확장자에 해당하는 이미지맵의 번호를 기억

	BOOL m_bUseFileType; //파일의 종류를 설명하는 정보를 가져올지 구분, FALSE 이면 확장자로 대체, 속도면에서 많은 차이가 있음
	static CString GetPathType(CString strPath);
	CString GetPathTypeFromMap(CString strPath, BOOL bIsDirectory, BOOL bUseFileType);
	static CTypeMap mapType;  //확장자에 해당하는 파일타입 기억

	BOOL m_bCheckOpen;
	CString GetPathMemo(CString strPath, DWORD dwAttributes, BOOL bCheckOpen);
	static CString GetPathName(CString strPath);
	HRESULT CreateShellItemArrayFromPaths(CStringArray& aPath, IShellItemArray*& shi_array);
	static LPITEMIDLIST GetPIDLfromPath(CString strPath);

	void SetColTexts(int* pStringId, int* pColFmt, int size);
	CUIntArray m_aColWidth;
	CUIntArray m_aColCompareType;

	inline BOOL IsDir(int nItem) { return (GetItemData(nItem) & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE; };
	static CString GetDrivePathFromName(CString strPath);

	int CompareItemByType(LPARAM item1, LPARAM item2, int nCol, int nType);

//	DECLARE_MESSAGE_MAP()
};

