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


//해당 파일의 아이콘 정보를 가져온다
int GetFileImageIndex(CString strPath, BOOL bIsDirectory)
{
	SHFILEINFO sfi;
	SHGetFileInfo((LPCTSTR)strPath, bIsDirectory ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL,
		&sfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES|SHGFI_SYSICONINDEX|SHGFI_ICON|SHGFI_SMALLICON);
//	return bIsDirectory ? 4 : 3;
	return sfi.iIcon;
}

//두 파일명 비교하기 : Locale 고려 비교 기능
int CompareFileName(TCHAR* name1, TCHAR* name2)
{
	int len1=(int)_tcslen(name1);
	int len2=(int)_tcslen(name2);
	int nRet=CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, name1, len1, name2, len2);
	if (nRet==CSTR_LESS_THAN) return -1;
	if (nRet==CSTR_GREATER_THAN) return 1;
	return 0; //nRet==CSTR_EQUAL
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

CString Get_Folder(CString strFile)
{
	CString strReturn;
	int n = strFile.ReverseFind(_T('\\'));
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

CString Get_Ext(CString strFile, BOOL bIsDirectory)
{
	CString strReturn;
	int n = strFile.ReverseFind(_T('.'));
	if (n == -1 || bIsDirectory == TRUE) return _T("");
	strReturn = strFile.Right(strFile.GetLength() - n);
	return strReturn;
}

CString ConvertKeyCodeToName(DWORD code)
{
	CString strKey;

	if (_T('0') <= code && code <= _T('9')) strKey = (TCHAR)code;
	else if (_T('A') <= code && code <= _T('Z')) strKey = (TCHAR)code;
	else if (code == _T(' ')) strKey = _T("Space");
	else if (code == VK_RETURN) strKey = _T("Enter");
	else if (code == VK_DELETE) strKey = _T("Delete");
	else if (code == VK_BACK)	strKey = _T("Backspace");
	else if (code == VK_INSERT) strKey = _T("Insert");
	else if (code == VK_TAB) strKey = _T("Tab");
	else if (code == VK_SHIFT) strKey = _T("Shift");
	else if (code == VK_LSHIFT) strKey = _T("Left Shift");
	else if (code == VK_RSHIFT) strKey = _T("Right Shift");
	else if (code == VK_CONTROL) strKey = _T("NULL");//strKey=_T("Ctrl");
	else if (code == VK_LCONTROL) strKey = _T("NULL");//strKey=_T("Left Ctrl");
	else if (code == VK_RCONTROL) strKey = _T("NULL");//strKey=_T("Right Ctrl");
	else if (code == VK_UP) strKey = _T("Up Arrow");
	else if (code == VK_DOWN) strKey = _T("Down Arrow");
	else if (code == VK_LEFT) strKey = _T("Left Arrow");
	else if (code == VK_RIGHT) strKey = _T("Right Arrow");
	else if (code == VK_HOME) strKey = _T("Home");
	else if (code == VK_END) strKey = _T("End");
	else if (code == VK_PRIOR) strKey = _T("Page Up");
	else if (code == VK_NEXT) strKey = _T("Page Down");
	else if (code == VK_F1) strKey = _T("F1");
	else if (code == VK_F2) strKey = _T("F2");
	else if (code == VK_F3) strKey = _T("F3");
	else if (code == VK_F4) strKey = _T("F4");
	else if (code == VK_F5) strKey = _T("F5");
	else if (code == VK_F6) strKey = _T("F6");
	else if (code == VK_F7) strKey = _T("F7");
	else if (code == VK_F8) strKey = _T("F8");
	else if (code == VK_F9) strKey = _T("F9");
	else if (code == VK_F10) strKey = _T("F10");
	else if (code == VK_F11) strKey = _T("F11");
	else if (code == VK_F12) strKey = _T("F12");
	else if (code == VK_F12) strKey = _T("F12");
	else if (code == VK_F12) strKey = _T("F12");
	else if (code == VK_NUMPAD0) strKey = _T("Numpad 0");
	else if (code == VK_NUMPAD1) strKey = _T("Numpad 1");
	else if (code == VK_NUMPAD2) strKey = _T("Numpad 2");
	else if (code == VK_NUMPAD3) strKey = _T("Numpad 3");
	else if (code == VK_NUMPAD4) strKey = _T("Numpad 4");
	else if (code == VK_NUMPAD5) strKey = _T("Numpad 5");
	else if (code == VK_NUMPAD6) strKey = _T("Numpad 6");
	else if (code == VK_NUMPAD7) strKey = _T("Numpad 7");
	else if (code == VK_NUMPAD8) strKey = _T("Numpad 8");
	else if (code == VK_NUMPAD9) strKey = _T("Numpad 9");
	else if (code == VK_MULTIPLY) strKey = _T("Numpad *");
	else if (code == VK_DIVIDE)	strKey = _T("Numpad /");
	else if (code == VK_ADD)		strKey = _T("Numpad +");
	else if (code == VK_SUBTRACT) strKey = _T("Numpad -");
	else if (code == VK_DECIMAL)	strKey = _T("Numpad .");

	else if (code == 0xbb) strKey = _T("=");
	else if (code == 0xbd) strKey = _T("-");
	else if (code == 0xc0) strKey = _T("~");
	else if (code == 0xdb) strKey = _T("[");
	else if (code == 0xdc) strKey = _T("\\");
	else if (code == 0xdd) strKey = _T("]");
	else if (code == 0xba) strKey = _T(";");
	else if (code == 0xde) strKey = _T("'");
	else if (code == 0xbc) strKey = _T("<");
	else if (code == 0xbe) strKey = _T(">");
	else if (code == 0xbf) strKey = _T("?");
	else if (code == VK_ESCAPE)	strKey = _T("NULL");
	else if (code == VK_CAPITAL)	strKey = _T("NULL");
	else if (code == VK_NUMLOCK)	strKey = _T("NULL");
	else if (code == VK_SCROLL)	strKey = _T("NULL");
	else if (code == VK_CLEAR)	strKey = _T("NULL");
	else if (code == VK_PAUSE)	strKey = _T("NULL");
	else if (code == VK_SNAPSHOT) strKey = _T("NULL");
	else if (code == -1) strKey = _T("지정 안함");
	else strKey.Format(_T("0x%x"), code);

	return strKey;

}
