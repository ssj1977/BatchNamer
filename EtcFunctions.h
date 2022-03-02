#ifndef __ETCFUNCTIONS__
#define __ETCFUNCTIONS__ 
BOOL FlagGET(int& nFlagType, int nField);
void FlagSET(int& nFlagType, int nField, BOOL bSet);
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
CString Get_Ext(CString strFile, BOOL bIsDirectory = FALSE, BOOL bIncludeDot = TRUE);
CString ConvertKeyCodeToName(DWORD code);
BOOL CString2BOOL(CString str);
CString PathBackSlash(CString strPath, BOOL bUseBackSlash = TRUE);

inline BOOL IsChecked(CWnd* pWnd, int nID)
{
	return (((CButton*)pWnd->GetDlgItem(nID))->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
}
inline void SetCheckByID(CWnd* pWnd, int nID, BOOL bCheck)
{
	((CButton*)pWnd->GetDlgItem(nID))->SetCheck(bCheck ? BST_CHECKED : BST_UNCHECKED);
}
#endif

