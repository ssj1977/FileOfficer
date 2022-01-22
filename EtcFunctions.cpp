#include "pch.h"
#include "EtcFunctions.h"

BOOL FlagGET(int& nFlagType, int nField)
{
	int i=0, flag=1;
	for (i=0;i<nField; i++) flag = flag * 2;
	
	if ((nFlagType & flag)!=0) return TRUE;
	return FALSE;
}

void FlagSET(int& nFlagType, int nField, BOOL bSet)
{
	int i=0, flag=1;
	for (i=0;i<nField; i++) flag = flag * 2;

	if (bSet==FALSE)
	{
		flag=0xFFFFFFFF^flag;
		nFlagType=nFlagType & flag;
	}
	if (bSet==TRUE)
	{
		nFlagType=nFlagType | flag;
	}
}


//���ϳ��� �ؽ�Ʈ ó���� ���� �Լ���
int GetLine(CString& strText, int nPos, CString& strLine, CString strToken)
{
	if (strText.IsEmpty()) {strLine.Empty();return -1;}
	int nPosNext = -1;
	
	if (strToken.GetLength()==1)
	{
		TCHAR c=strToken.GetAt(0);
		nPosNext=strText.Find(c,nPos);
	}
	else
	{
		nPosNext=strText.Find(strToken,nPos);
	}

	if (nPosNext!=-1)
	{
		strLine = strText.Mid(nPos,nPosNext-nPos);
		nPosNext+=strToken.GetLength();
	}
	else
	{
		nPosNext= strText.GetLength();
		strLine = strText.Mid(nPos,nPosNext-nPos);
		return -1;
	}
	return nPosNext;
}

void GetToken(CString& strLine, CString& str1, CString& str2, TCHAR cSplit, BOOL bReverseFind)
{
	int n;
	if (bReverseFind == FALSE)	n = strLine.Find(cSplit);
	else						n = strLine.ReverseFind(cSplit);
	if (n == -1)
	{
		str1 = strLine;
		str2.Empty();
	}
	else
	{
		str1 = strLine.Left(n);
		if ((strLine.GetLength() - n - 1) < 1) str2.Empty();
		else str2 = strLine.Right(strLine.GetLength() - n - 1);
	}
	str1.TrimLeft(); str1.TrimRight();
	str2.TrimLeft(); str2.TrimRight();
}

BOOL WriteCStringToFile(CString strFile, CString& strContent)
{
	try
	{
		CFile file;
		if (file.Open(strFile, CFile::modeCreate|CFile::modeWrite)==FALSE) return FALSE;
#ifdef _UNICODE
		BYTE UnicodeIdentifier[] = {0xff,0xfe};
		file.Write(UnicodeIdentifier, 2);
#endif 
		if (strContent.IsEmpty()==FALSE)
		{
			file.Write(strContent.GetBuffer(0), strContent.GetLength()*sizeof(TCHAR));
			strContent.ReleaseBuffer();
		}
		file.Flush();
		file.Close();
	}
	catch(CFileException* e)
	{
		e->Delete(); 
		return FALSE;
	}
	return TRUE;
}

BOOL ReadFileToCString(CString strFile, CString& strData)
{
	//Unicode �ĺ��ؼ� �б�
	try
	{
		CFile file;
		if (file.Open(strFile, CFile::modeRead)==FALSE) return FALSE;
		size_t filesize = (size_t)file.GetLength();
		if (filesize > 2)
		{
			BYTE uidf[2];
			file.Read(uidf, 2);
			if (uidf[0]==0xff && uidf[1]==0xfe)	
			{
				filesize-=2;
#ifdef _UNICODE
				int nStrLen = int( filesize / sizeof(TCHAR) ) + 1;
				TCHAR* pBuf = strData.GetBufferSetLength(nStrLen);
				memset(pBuf, 0, filesize + sizeof(TCHAR));
				file.Read(pBuf, (UINT)filesize);
				strData.ReleaseBuffer();
				file.Close();
#else
				int nStrLen = ( filesize / sizeof(WCHAR) ) + 1;
				WCHAR* pBuf=new WCHAR[nStrLen];
				memset(pBuf, 0, filesize + sizeof(WCHAR));
				file.Read(pBuf, filesize);
				file.Close();
				strData=pBuf;
				delete[] pBuf;
#endif 
			}
			else								
			{
				file.SeekToBegin();
				char* pBuf=new char[filesize + 1];
				memset(pBuf, 0, filesize + 1);
				file.Read(pBuf, (UINT)filesize);
				strData = pBuf;	
				file.Close();
				delete[] pBuf;
			}
		}
	}
	catch(CFileException* e)
	{
		e->Delete(); 
		return FALSE;
	}
	return TRUE;
}

CString IDSTR(int nID)
{
	CString strRet;
	strRet.LoadString(nID);
	return strRet;
}

CString INTtoSTR(int n)
{
	CString str; str.Format(_T("%d"), n);
	return str;
}

CString Get_Folder(CString strFile, BOOL bIncludeSlash)
{
	CString strReturn;
	int n = strFile.ReverseFind(_T('\\'));
	if (bIncludeSlash) n++;
	strReturn = strFile.Left(n);
	return strReturn;
}

CString Get_Name(CString strFile, BOOL bKeepExt)
{
	CString strReturn;
	int n1 = strFile.ReverseFind(_T('\\'));
	int n2 = -1;
	if (bKeepExt == FALSE)	n2 = strFile.ReverseFind(_T('.'));
	else					n2 = strFile.GetLength();
	if (n1 == -1) n1 = -1;
	if (n2 == -1) n2 = strFile.GetLength();
	strReturn = strFile.Mid(n1 + 1, n2 - n1 - 1);
	return strReturn;
}

CString Get_Ext(CString strFile, BOOL bIsDirectory, BOOL bIncludeDot)
{
	CString strReturn;
	int n = strFile.ReverseFind(_T('.'));
	if (n < 0 || bIsDirectory == TRUE) return _T("");
	if (bIncludeDot == FALSE) n++;
	strReturn = strFile.Right(strFile.GetLength() - n);
	return strReturn;
}