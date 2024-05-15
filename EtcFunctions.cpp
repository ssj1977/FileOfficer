#include "pch.h"
#include "EtcFunctions.h"
#include <string>

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

int GetStringArray(CString strSource, TCHAR cSplit, CStringArray& aStr, BOOL bMakeLower)
{
	aStr.RemoveAll();
	if (strSource.IsEmpty()) return 0;
	CString strToken;
	int i = 0;
	while (AfxExtractSubString(strToken, strSource, i, cSplit))
	{
		if (bMakeLower) strToken.MakeLower();
		strToken.Trim();
		aStr.Add(strToken);
		i++;
	}
	return (int)aStr.GetCount();
}


BOOL WriteCStringToFile(CString strFile, CString& strContent, BOOL bUTF8, BOOL bBOM)
{
	try
	{
		CFile file;
		if (file.Open(strFile, CFile::modeCreate | CFile::modeWrite) == FALSE) return FALSE;
		if (bUTF8 == FALSE)
		{
			BYTE BOM[] = { 0xff,0xfe }; //UTF-16 LE
			file.Write(BOM, 2);
			if (strContent.IsEmpty() == FALSE)
			{
				file.Write(strContent.GetBuffer(0), strContent.GetLength() * sizeof(TCHAR));
				strContent.ReleaseBuffer();
			}
		}
		else
		{
			if (bBOM)
			{
				BYTE BOM[] = { 0xef,0xbb,0xbf }; //UTF-8
				file.Write(BOM, 3);
			}
			// Get the length of the wide character string
			int wideLength = strContent.GetLength();
			// Calculate the required buffer size for UTF-8 encoding
			int utf8Length = WideCharToMultiByte(CP_UTF8, 0, strContent, wideLength, NULL, 0, NULL, NULL);
			// Allocate buffer for UTF-8 string
			std::string utf8String(utf8Length, 0);
			// Perform the conversion
			WideCharToMultiByte(CP_UTF8, 0, strContent, wideLength, &utf8String[0], utf8Length, NULL, NULL);
			file.Write(utf8String.c_str(), static_cast<UINT>(utf8String.size()));
		}
		file.Flush();
		file.Close();
	}
	catch (CFileException* e)
	{
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

BOOL ReadFileToCString(CString strFile, CString& strData)
{
	try
	{
		CFile file;
		if (file.Open(strFile, CFile::modeRead)==FALSE) return FALSE;
		size_t filesize = (size_t)file.GetLength();
		if (filesize > 2)
		{
			BYTE uidf[2];
			file.Read(uidf, 2);
			if (uidf[0]==0xff && uidf[1]==0xfe)	 //UTF-16LE
			{
				filesize-=2;
				int nStrLen = int( filesize / sizeof(TCHAR) ) + 1;
				TCHAR* pBuf = strData.GetBufferSetLength(nStrLen);
				memset(pBuf, 0, filesize + sizeof(TCHAR));
				file.Read(pBuf, (UINT)filesize);
				strData.ReleaseBuffer();
				file.Close();
			}
			else //Others includng UTF-8							
			{
				file.SeekToBegin();
				char* pBuf=new char[filesize + 1];
				memset(pBuf, 0, filesize + 1);
				file.Read(pBuf, (UINT)filesize);
				int nBufSize_W = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, pBuf, -1, NULL, 0);
				if (nBufSize_W - 1 > 0)
				{
					WCHAR* pBufW = new WCHAR[nBufSize_W];
					MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, pBuf, -1, pBufW, nBufSize_W);
					strData = pBufW;
					delete[] pBufW;
				}
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
	if (strPath.GetAt(nPos) == _T('\\')) //���� '\' �϶�
	{
		if (bUseBackSlash == FALSE) strPath = strPath.Left(nPos);
	}
	else // ���� '\'�� �ƴҶ�
	{
		if (bUseBackSlash == TRUE) strPath += _T('\\');
	}
	return strPath;
}

////////////////////////////////////////////////////////
//Ǯ��� ����� �ѱ��� ��ƾ���� �ٲٱ� ���� �ڵ�
WCHAR Hangeul_NFDtoNFC(CString strNFD)
{
	// �ʼ� 19�� 0x1100 ~ 0x1112
	// �߼� 21�� 0x1161 ~ 0x1175
	// ���� 27�� 0x11A8 ~ 0x11C2  => ������ ���� ������ 28�� ���
	// �ϼ��� ���� �ڵ� : �� = 0xAC00 
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
	CString strNFD; //TCHAR NFD[3] = {}; //[0]=�ʼ�, [1]=�߼�, [2]=����
	for (int i = 0; i < strSrc.GetLength(); i++)
	{
		TCHAR c = strSrc.GetAt(i);
		if (0x1100 <= c && c <= 0x1112) //�ʼ� �߰�
		{
			if (strNFD.GetLength() == 0) //ó�� �߰�
			{
				strNFD = c; //��ū�� ���ۿ� ����
			}
			else if (strNFD.GetLength() == 1) // �߼��� ã�� �ִµ� �ʼ��� �߰ߵ� ���
			{
				strRet += strNFD; //���������� �״�� ���(���� �ʼ�)
				strNFD = c; //�ʼ��� ó�� �߰ߵ� ���� �ʱ�ȭ
			}
			else if (strNFD.GetLength() == 2) // ������ ã�� �ִµ� �ʼ��� �߰ߵ� ���
			{
				strRet += Hangeul_NFDtoNFC(strNFD); //���������� ������ ���� ���ڷ� �ؼ��Ͽ� ���
				strNFD = c; //�ʼ��� ó�� �߰ߵ� ���� �ʱ�ȭ
			}
		}
		else if (0x1161 <= c && c <= 0x1175) // �߼�(����) �߰�
		{
			if (strNFD.GetLength() == 0) //�ʼ��� ã�� �ִµ� �߼��� �߰ߵ� ���
			{
				strRet += c; //���� ��ū�� �״�� ���
			}
			else if (strNFD.GetLength() == 1) //���ϴ� ������ �߰ߵ� ���
			{
				strNFD += c; //��ū�� ���ۿ� ����
			}
			else if (strNFD.GetLength() == 2) //������ ã�� �ִµ� �߼��� �߰ߵ� ���
			{
				strRet += Hangeul_NFDtoNFC(strNFD); //���������� ������ ���� ���ڷ� �ؼ��Ͽ� ���
				strRet += c; //���� ��ū�� �״�� ���
			}
		}
		else if (0x11A8 <= c && c <= 0x11C2) // ���� �߰�
		{
			if (strNFD.GetLength() == 0) //�ʼ��� ã�� �ִµ� ������ �߰ߵ� ���
			{
				strRet += c; //���� ��ū�� �״�� ���
			}
			else if (strNFD.GetLength() == 1) //�߼��� ã�� �ִµ� ������ �߰ߵ� ���
			{
				strRet += strNFD; //����� ��ū(�ʼ��� �ִ� ���) �״�� ���
				strRet += c; //�߰ߵ� ���� �״�� ���
			}
			else if (strNFD.GetLength() == 2) //������ ������ �߰ߵ� ���
			{
				strNFD += c; //��ū�� ���ۿ� ����
				strRet += Hangeul_NFDtoNFC(strNFD); //��ȯ�Ͽ� ���
				strNFD.Empty(); // ���� �ʱ�ȭ
			}
		}
		else //������ �ѱ��� �ƴ� ���
		{
			if (strNFD.GetLength() < 2) //����� ��ū�� �ѱ��� ���϶��
			{
				strRet += strNFD; //�״�� ���
			}
			else //����� ��ū�� �α��� �̻��̶��
			{
				strRet += Hangeul_NFDtoNFC(strNFD);  //��ȯ�ؼ� ���
			}
			strNFD.Empty(); //���۸� ���
			strRet += c; //���� ��ū�� �״�� ���
		}
	}
	//������ ���� ��ū ó��
	if (strNFD.GetLength() == 1) //����� ��ū�� �ѱ��ڶ��
	{
		strRet += strNFD; //�״�� ���
	}
	else if (strNFD.GetLength() > 1) //����� ��ū�� �α��� �̻��̶��
	{
		strRet += Hangeul_NFDtoNFC(strNFD);  //��ȯ�ؼ� ���
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