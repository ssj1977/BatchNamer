// CDlgCFG_Load.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "BatchNamer.h"
#include "CDlgCFG_Load.h"
#include <afxdialogex.h>

// CDlgCFG_Load 대화 상자

IMPLEMENT_DYNAMIC(CDlgCFG_Load, CDialogEx)

CDlgCFG_Load::CDlgCFG_Load(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CFG_LOAD, pParent)
{
	m_nLoadType = 0; // 0 = Add a Folder itself, 1 = Add Files in a Folder
	m_bShowEverytime = FALSE;
	m_bAutoSort = TRUE;
}

CDlgCFG_Load::~CDlgCFG_Load()
{

}

void CDlgCFG_Load::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgCFG_Load, CDialogEx)
END_MESSAGE_MAP()


// CDlgCFG_Load 메시지 처리기


BOOL CDlgCFG_Load::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (m_nLoadType == 0)		((CButton*)GetDlgItem(IDC_RADIO_LOAD_0))->SetCheck(TRUE);
	else if (m_nLoadType == 1)	((CButton*)GetDlgItem(IDC_RADIO_LOAD_1))->SetCheck(TRUE);

	if (m_bAutoSort == 0)		((CButton*)GetDlgItem(IDC_RADIO_AUTOSORT_0))->SetCheck(TRUE);
	else if (m_bAutoSort == 1)	((CButton*)GetDlgItem(IDC_RADIO_AUTOSORT_1))->SetCheck(TRUE);

	((CButton*)GetDlgItem(IDC_CHK_SHOWEVERYTIME))->SetCheck(m_bShowEverytime);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}



void CDlgCFG_Load::OnOK()
{
	if (((CButton*)GetDlgItem(IDC_RADIO_LOAD_0))->GetCheck()==TRUE) m_nLoadType = 0;
	else if (((CButton*)GetDlgItem(IDC_RADIO_LOAD_1))->GetCheck() == TRUE) m_nLoadType = 1;

	if (((CButton*)GetDlgItem(IDC_RADIO_AUTOSORT_0))->GetCheck() == TRUE) m_bAutoSort = FALSE;
	else if (((CButton*)GetDlgItem(IDC_RADIO_AUTOSORT_1))->GetCheck() == TRUE) m_bAutoSort = TRUE;

	m_bShowEverytime = ((CButton*)GetDlgItem(IDC_CHK_SHOWEVERYTIME))->GetCheck();
	CDialogEx::OnOK();
}


void CDlgCFG_Load::OnCancel()
{
	CDialogEx::OnCancel();
}
