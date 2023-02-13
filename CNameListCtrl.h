#pragma once
#include <afxlistctrl.h>
using namespace std;

#include <set>
typedef set<CString> CPathSet; //중복이름 체크용 맵 해당하는 이미지맵의 번호를 기억

#ifndef COL_OLDNAME
#define COL_OLDNAME 0
#define COL_NEWNAME 1
#define COL_OLDFOLDER 2
#define COL_NEWFOLDER 3
#define COL_FILESIZE 4
#define COL_TIMEMODIFY 5
#define COL_TIMECREATE 6
#define COL_FULLPATH 7
#define COL_TOTAL 8
#endif

class CNameListCtrl : public CMFCListCtrl
{
public:
	CPathSet m_setPath;
	afx_msg void OnHdnItemclick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual int OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn);
	virtual void Sort(int iColumn, BOOL bAscending = TRUE, BOOL bAdd = FALSE);
	int CompareItemByType(LPARAM item1, LPARAM item2, int nCol, int nType);
	CString GetOldPath(int nItem);
	CString GetNewPath(int nItem);
	void DeleteListItem(int nItem);
};

