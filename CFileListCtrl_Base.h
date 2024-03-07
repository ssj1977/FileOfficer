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

//���� ���� ����Ʈ ��Ʈ���� �⺻���� �Լ��� �Ӽ��� ������ Ŭ����
//CFileListCtrl, CShortCutList � ����
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

	BOOL m_bUseFileIcon; //���� �������� ǥ������ ����, �Ϻ� �ý��� ������ ���� ���� ������
	int m_nIconType;
	HIMAGELIST m_hImageList;
	void SetIconType(int nIconType);
	static int GetFileImageIndex(CString strPath, DWORD dwAttribute);
	static int GetDriveImageIndex(int nDriveType);
	int GetFileImageIndexFromMap(CString strPath, DWORD dwAttribute);
	static CExtMap mapExt;  //Ȯ���ڿ� �ش��ϴ� �̹������� ��ȣ�� ���

	BOOL m_bUseFileType; //������ ������ �����ϴ� ������ �������� ����, FALSE �̸� Ȯ���ڷ� ��ü, �ӵ��鿡�� ���� ���̰� ����
	static CString GetPathType(CString strPath);
	CString GetPathTypeFromMap(CString strPath, BOOL bIsDirectory, BOOL bUseFileType);
	static CTypeMap mapType;  //Ȯ���ڿ� �ش��ϴ� ����Ÿ�� ���

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

