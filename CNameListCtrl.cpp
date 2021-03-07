#include "pch.h"
#include "CNameListCtrl.h"
#include <shlwapi.h>
#include <atlpath.h>

#define COL_COMP_STR 0
#define COL_COMP_PATH 1
#define COL_COMP_SIZE 2

CString GetFileSizeString(ULONGLONG nSize)
{
	TCHAR pBuf[100];
	ZeroMemory(pBuf, 100);
	CString strSize;
	strSize.Format(_T("%I64u"), nSize);
	int nLen = strSize.GetLength();
	if (nLen > 74) return (_T("Overflow"));
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
	return pBuf;
}

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

int CNameListCtrl::OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn)
{
	int nRet = 0;
	if (iColumn == COL_OLDNAME || iColumn == COL_NEWNAME || 
		iColumn == COL_OLDFOLDER || iColumn == COL_NEWFOLDER || 
		iColumn == COL_FULLPATH)
		nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_PATH);
	else if (iColumn == COL_TIMECREATE || iColumn == COL_TIMEMODIFY) 
		nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_STR);
	else if (iColumn == COL_FILESIZE)
		nRet = CompareItemByType(lParam1, lParam2, iColumn, COL_COMP_SIZE);
	return nRet;
}

int CNameListCtrl::CompareItemByType(LPARAM item1, LPARAM item2, int nCol, int nType)
{
	int nRet = 0;
	CString str1 = GetItemText((int)item1, nCol);
	CString str2 = GetItemText((int)item2, nCol);
	if (nType == COL_COMP_STR)
	{
		nRet = StrCmp(str1, str2);
	}
	else if (nType == COL_COMP_PATH)
	{
		DWORD_PTR type1, type2;
		type1 = GetItemData((int)item1);
		type2 = GetItemData((int)item2);
		if (type1 != type2)
		{
			nRet = int(type1 - type2);
		}
		else
		{
			nRet = StrCmpLogicalW(str1.GetBuffer(), str2.GetBuffer());
			str1.ReleaseBuffer();
			str2.ReleaseBuffer();
		}
	}
	else if (nType == COL_COMP_SIZE)
	{
		ULONGLONG size1 = Str2Size(str1);
		ULONGLONG size2 = Str2Size(str2);
		if (size1 == size2) nRet = 0;
		else if (size1 > size2) nRet = 1;
		else if (size1 < size2) nRet = -1;
	}
	return nRet;
}

void CNameListCtrl::Sort(int iColumn, BOOL bAscending, BOOL bAdd)
{
	/*if (iColumn == 1) // Using Item Data : File size
	{
		CMFCListCtrl::Sort(iColumn, bAscending, bAdd);
		return;
	}*/
	CWaitCursor wait;
	GetHeaderCtrl().SetSortColumn(iColumn, bAscending, bAdd);
	m_iSortedColumn = iColumn;
	m_bAscending = bAscending;
	SortItemsEx(CompareProc, (LPARAM)this);
}

void CNameListCtrl::OnHdnItemclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	Default();
	*pResult = 0;
}

CString CNameListCtrl::GetOldPath(int nItem)
{
	CString strPath;
	strPath = GetItemText(nItem, COL_FULLPATH);
	if (strPath.IsEmpty())
		strPath = GetItemText(nItem, COL_OLDFOLDER) + _T("\\") + GetItemText(nItem, COL_OLDNAME);
	return strPath;
}

CString CNameListCtrl::GetNewPath(int nItem)
{
	CString strPath;
	strPath = GetItemText(nItem, COL_NEWFOLDER) + _T("\\") + GetItemText(nItem, COL_NEWNAME);
	return strPath;
}

void CNameListCtrl::DeleteListItem(int nItem)
{
	m_setPath.erase(GetOldPath(nItem));
	DeleteItem(nItem);
}
