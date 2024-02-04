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


//파일내의 텍스트 처리를 위한 함수들
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
	//Unicode 식별해서 읽기
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

CString PathBackSlash(CString strPath, BOOL bUseBackSlash)
{
	if (strPath.IsEmpty()) return strPath;
	int nPos = strPath.GetLength() - 1;
	if (strPath.GetAt(nPos) == _T('\\')) //끝이 '\' 일때
	{
		if (bUseBackSlash == FALSE) strPath = strPath.Left(nPos);
	}
	else // 끝이 '\'가 아닐때
	{
		if (bUseBackSlash == TRUE) strPath += _T('\\');
	}
	return strPath;
}

////////////////////////////////////////////////////////
//풀어쓰기 방식의 한글을 모아쓰기로 바꾸기 위한 코드
WCHAR Hangeul_NFDtoNFC(CString strNFD)
{
	// 초성 19개 0x1100 ~ 0x1112
	// 중성 21개 0x1161 ~ 0x1175
	// 종성 27개 0x11A8 ~ 0x11C2  => 종성이 없는 경우까지 28개 경우
	// 완성형 시작 코드 : 가 = 0xAC00 
	WCHAR cNFC = L'\0';
	if (strNFD.GetLength() >= 2)
	{
		cNFC = (strNFD.GetAt(0) - 0x1100) * 28 * 21 + (strNFD.GetAt(1) - 0x1161) * 28;
		if (strNFD.GetLength() == 3) cNFC += (strNFD.GetAt(2) - 0x11A7);
		cNFC += 0xAC00;
	}
	return cNFC;
}
CString ConvertNFD(CString strSrc)
{
	CString strRet, strTemp;
	CString strNFD; //TCHAR NFD[3] = {}; //[0]=초성, [1]=중성, [2]=종성
	for (int i = 0; i < strSrc.GetLength(); i++)
	{
		TCHAR c = strSrc.GetAt(i);
		if (0x1100 <= c && c <= 0x1112) //초성 발견
		{
			if (strNFD.GetLength() == 0) //처음 발견
			{
				strNFD = c; //토큰을 버퍼에 저장
			}
			else if (strNFD.GetLength() == 1) // 중성을 찾고 있는데 초성이 발견된 경우
			{
				strRet += strNFD; //이전까지를 그대로 출력(단일 초성)
				strNFD = c; //초성이 처음 발견된 경우로 초기화
			}
			else if (strNFD.GetLength() == 2) // 종성을 찾고 있는데 초성이 발견된 경우
			{
				strRet += Hangeul_NFDtoNFC(strNFD); //이전까지를 종성이 없는 문자로 해석하여 출력
				strNFD = c; //초성이 처음 발견된 경우로 초기화
			}
		}
		else if (0x1161 <= c && c <= 0x1175) // 중성(모음) 발견
		{
			if (strNFD.GetLength() == 0) //초성을 찾고 있는데 중성이 발견된 경우
			{
				strRet += c; //현재 토큰을 그대로 출력
			}
			else if (strNFD.GetLength() == 1) //원하는 시점에 발견된 경우
			{
				strNFD += c; //토큰을 버퍼에 저장
			}
			else if (strNFD.GetLength() == 2) //종성을 찾고 있는데 중성이 발견된 경우
			{
				strRet += Hangeul_NFDtoNFC(strNFD); //이전까지를 종성이 없는 문자로 해석하여 출력
				strRet += c; //현재 토큰을 그대로 출력
			}
		}
		else if (0x11A8 <= c && c <= 0x11C2) // 종성 발견
		{
			if (strNFD.GetLength() == 0) //초성을 찾고 있는데 종성이 발견된 경우
			{
				strRet += c; //현재 토큰을 그대로 출력
			}
			else if (strNFD.GetLength() == 1) //중성을 찾고 있는데 종성이 발견된 경우
			{
				strRet += strNFD; //저장된 토큰(초성만 있는 경우) 그대로 출력
				strRet += c; //발견된 종성 그대로 출력
			}
			else if (strNFD.GetLength() == 2) //적합한 시점에 발견된 경우
			{
				strNFD += c; //토큰을 버퍼에 저장
				strRet += Hangeul_NFDtoNFC(strNFD); //변환하여 출력
				strNFD.Empty(); // 버퍼 초기화
			}
		}
		else //조합형 한글이 아닌 경우
		{
			if (strNFD.GetLength() < 2) //저장된 토큰이 한글자 이하라면
			{
				strRet += strNFD; //그대로 출력
			}
			else //저장된 토큰이 두글자 이상이라면
			{
				strRet += Hangeul_NFDtoNFC(strNFD);  //변환해서 출력
			}
			strNFD.Empty(); //버퍼를 비움
			strRet += c; //현재 토큰을 그대로 출력
		}
	}
	//끝나고 남은 토큰 처리
	if (strNFD.GetLength() == 1) //저장된 토큰이 한글자라면
	{
		strRet += strNFD; //그대로 출력
	}
	else if (strNFD.GetLength() > 1) //저장된 토큰이 두글자 이상이라면
	{
		strRet += Hangeul_NFDtoNFC(strNFD);  //변환해서 출력
	}
	return strRet;
}
////////////////////////////////////////////////////////



ULONGLONG Str2Size(CString str)
{
	str.Remove(_T(','));
	ULONGLONG size = _wcstoui64(str, NULL, 10);
	if (str.GetLength() > 2)
	{
		CString strUnit = str.Right(2);
		if (strUnit == _T("GB")) size = size * 1073741824;
		else if (strUnit == _T("MB")) size = size * 1048576;
		else if (strUnit == _T("KB")) size = size * 1024;
	}
	return size;
}

CString GetFileSizeString(ULONGLONG nSize, int nUnit)
{
	TCHAR pBuf[135];
	ZeroMemory(pBuf, 135);
	CString strSize, strReturn;
	if (nUnit > 4) nUnit = 4;
	if (nUnit < 0) nUnit = 0;
	nSize = nSize / (int)pow(2, 10 * nUnit);
	strSize.Format(_T("%I64u"), nSize);
	int nLen = strSize.GetLength();
	if (nLen < 100)
	{
		int nPos = 0;
		for (int i = 0; i < nLen; i++)
		{
			pBuf[nPos] = strSize.GetAt(i);
			nPos += 1;
			if (i < nLen - 3 && (nLen - i - 1) % 3 == 0)
			{
				pBuf[nPos] = _T(',');
				nPos += 1;
			}
		}
		pBuf[99] = _T('\0');
		strReturn = (LPCTSTR)pBuf;
		//if (nUnit == 0) strReturn += "B";
		if (nUnit == 1) strReturn += "KB";
		else if (nUnit == 2) strReturn += "MB";
		else if (nUnit == 3) strReturn += "GB";
		else if (nUnit == 4) strReturn += "TB";
	}
	else
	{
		strReturn = _T("The size is too large.");
	}
	return strReturn;
}