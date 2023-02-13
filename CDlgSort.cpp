// CDlgSort.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgSort.h"
#include "afxdialogex.h"
#include "CNameListCtrl.h"
#include "EtcFunctions.h"


// CDlgSort 대화 상자

IMPLEMENT_DYNAMIC(CDlgSort, CDialogEx)

CDlgSort::CDlgSort(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SORT, pParent)
{
	m_nSortCol = COL_OLDNAME;
	m_bSortAscend = TRUE;
	m_pSortWnd = NULL;
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

BOOL CDlgSort::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_SORTCOL);
	if (m_pSortWnd == NULL)
	{	//프리셋용 설정창
		pCB->AddString(IDSTR(IDS_COL_OLDNAME));
		pCB->AddString(IDSTR(IDS_COL_NEWNAME));
		pCB->AddString(IDSTR(IDS_COL_OLDFOLDER));
		pCB->AddString(IDSTR(IDS_COL_NEWFOLDER));
		pCB->AddString(IDSTR(IDS_COL_FILESIZE));
		pCB->AddString(IDSTR(IDS_COL_TIMEMODIFY));
		pCB->AddString(IDSTR(IDS_COL_TIMECREATE));
		pCB->AddString(IDSTR(IDS_COL_FULLPATH));
	}
	else
	{	//일반 설정창
		CMFCHeaderCtrl& header = ((CNameListCtrl*)m_pSortWnd)->GetHeaderCtrl();
		int nCount = header.GetItemCount();
		m_nSortCol = header.GetSortColumn();
		m_bSortAscend = header.IsAscending();
		HDITEM hdi;
		TCHAR buf[MY_MAX_PATH];
		hdi.mask = HDI_TEXT;
		hdi.cchTextMax = MY_MAX_PATH;
		hdi.pszText = buf;
		for (int i = 0; i < nCount; i++)
		{
			header.GetItem(i, &hdi);
			pCB->AddString(hdi.pszText);
		}
	}
	pCB->SetCurSel(m_nSortCol);
	if (m_bSortAscend == TRUE)	SetCheckByID(this, IDC_RADIO_SORT_ASCEND, TRUE);
	else						SetCheckByID(this, IDC_RADIO_SORT_DESCEND, TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgSort::OnOK()
{
	CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_SORTCOL);
	m_nSortCol = pCB->GetCurSel();
	m_bSortAscend = IsChecked(this, IDC_RADIO_SORT_ASCEND);
	CDialogEx::OnOK();
}


void CDlgSort::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}
