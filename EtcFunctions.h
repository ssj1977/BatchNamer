#ifndef __ETCFUNCTIONS__
#define __ETCFUNCTIONS__ 
BOOL FlagGET(int& nFlagType, int nField);
void FlagSET(int& nFlagType, int nField, BOOL bSet);
int GetFileImageIndex(CString strPath, BOOL bIsDirectory);
int CompareFileName(const void* left, const void* right);
//int CompareFileName(TCHAR* name1, TCHAR* name2);
//int CompareFileName(CString& name1, CString& name2);
int GetLine(CString& strText, int nPos, CString& strLine, CString strToken);
BOOL WriteCStringToFile(CString strFile, CString& strContent);
BOOL ReadFileToCString(CString strFile, CString& strData);
CString IDSTR(int nID);
CString INTtoSTR(int n);
void GetToken(CString& strLine, CString& str1, CString& str2, TCHAR cSplit, BOOL bReverseFind);
CString Get_Folder(CString strFile);
CString Get_Name(CString strFile, BOOL bKeepExt = TRUE);
CString Get_Ext(CString strFile, BOOL bIsDirectory = FALSE);
CString ConvertKeyCodeToName(DWORD code);
BOOL CString2BOOL(CString str);
#endif
