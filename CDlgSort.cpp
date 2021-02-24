// CDlgSort.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgSort.h"
#include "afxdialogex.h"
#include "CNameListCtrl.h"


// CDlgSort 대화 상자

IMPLEMENT_DYNAMIC(CDlgSort, CDialogEx)

CDlgSort::CDlgSort(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SORT, pParent)
{
	m_nSortCol = COL_OLDNAME;
	m_bAsc = TRUE;
}

CDlgSort::~CDlgSort()
{
}

void CDlgSort::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSort, CDialogEx)
END_MESSAGE_MAP()


// CDlgSort 메시지 처리기

void CDlgSort::SetCheckByID(int nID, BOOL bCheck)
{
	CButton* pBtn = (CButton*)GetDlgItem(nID);
	pBtn->SetCheck(bCheck);
}


BOOL CDlgSort::GetCheckByID(int nID)
{
	CButton* pBtn = (CButton*)GetDlgItem(nID);
	return pBtn->GetCheck();
}

BOOL CDlgSort::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	switch (m_nSortCol)
	{
		case COL_OLDNAME: SetCheckByID(IDC_RADIO_SORT_OLDNAME, TRUE); break;
		case COL_NEWNAME: SetCheckByID(IDC_RADIO_SORT_NEWNAME, TRUE); break;
		case COL_OLDFOLDER: SetCheckByID(IDC_RADIO_SORT_OLDFOLDER, TRUE); break;
		case COL_NEWFOLDER: SetCheckByID(IDC_RADIO_SORT_NEWFOLDER, TRUE); break;
		case COL_FILESIZE: SetCheckByID(IDC_RADIO_SORT_SIZE, TRUE); break;
		case COL_TIMEMODIFY: SetCheckByID(IDC_RADIO_SORT_TIMEMODIFY, TRUE); break;
		case COL_TIMECREATE: SetCheckByID(IDC_RADIO_SORT_TIMECREATE, TRUE); break;
		case COL_FULLPATH: SetCheckByID(IDC_RADIO_SORT_FULLPATH, TRUE); break;
	}
	if (m_bAsc == TRUE) SetCheckByID(IDC_RADIO_SORT_ASCEND, TRUE);
	else				SetCheckByID(IDC_RADIO_SORT_DESCEND, TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgSort::OnOK()
{
	if		(GetCheckByID(IDC_RADIO_SORT_OLDNAME)) m_nSortCol = COL_OLDNAME;
	else if (GetCheckByID(IDC_RADIO_SORT_NEWNAME)) m_nSortCol = COL_NEWNAME;
	else if (GetCheckByID(IDC_RADIO_SORT_OLDFOLDER)) m_nSortCol = COL_OLDFOLDER;
	else if (GetCheckByID(IDC_RADIO_SORT_NEWFOLDER)) m_nSortCol = COL_NEWFOLDER;
	else if (GetCheckByID(IDC_RADIO_SORT_SIZE)) m_nSortCol = COL_FILESIZE;
	else if (GetCheckByID(IDC_RADIO_SORT_TIMEMODIFY)) m_nSortCol = COL_TIMEMODIFY;
	else if (GetCheckByID(IDC_RADIO_SORT_TIMECREATE)) m_nSortCol = COL_TIMECREATE;
	else if (GetCheckByID(IDC_RADIO_SORT_FULLPATH)) m_nSortCol = COL_FULLPATH;


	if (GetCheckByID(IDC_RADIO_SORT_ASCEND)) m_bAsc = TRUE;
	else   m_bAsc = FALSE; //if (GetCheckByID(IDC_RADIO_SORT_DESCEND))

	CDialogEx::OnOK();
}


void CDlgSort::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}
