#ifndef __ETCFUNCTIONS__
#define __ETCFUNCTIONS__ 
BOOL FlagGET(int& nFlagType, int nField);
void FlagSET(int& nFlagType, int nField, BOOL bSet);
int GetLine(CString& strText, int nPos, CString& strLine, CString strToken);
BOOL WriteCStringToFile(CString strFile, CString& strContent);
BOOL ReadFileToCString(CString strFile, CString& strData);
CString IDSTR(int nID);
CString INTtoSTR(int n);
void GetToken(CString& strLine, CString& str1, CString& str2, TCHAR cSplit, BOOL bReverseFind);
CString Get_Folder(CString strFile, BOOL bIncludeSlash = FALSE);
CString Get_Name(CString strFile, BOOL bKeepExt = TRUE);
CString Get_Ext(CString strFile, BOOL bIsDirectory = FALSE);
#endif
