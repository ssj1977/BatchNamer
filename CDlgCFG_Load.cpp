// CDlgCFG_Load.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "BatchNamer.h"
#include "CDlgCFG_Load.h"
#include "EtcFunctions.h"
#include <afxdialogex.h>

// CDlgCFG_Load 대화 상자

IMPLEMENT_DYNAMIC(CDlgCFG_Load, CDialogEx)

CDlgCFG_Load::CDlgCFG_Load(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CFG_LOAD, pParent)
{
	m_nLoadType = 2; //목록 읽기 방법 : 0 = 폴더를 그대로 추가 / 1 = 폴더 안의 파일을 추가 / 2 = 폴더를 추가할때 물어보기
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

	if (m_nLoadType == 0)		((CButton*)GetDlgItem(IDC_RADIO_LOAD_0))->SetCheck(BST_CHECKED);
	else if (m_nLoadType == 1)	((CButton*)GetDlgItem(IDC_RADIO_LOAD_1))->SetCheck(BST_CHECKED);
	else if (m_nLoadType == 2)	((CButton*)GetDlgItem(IDC_RADIO_LOAD_2))->SetCheck(BST_CHECKED);

	if (m_bAutoSort == 0)		((CButton*)GetDlgItem(IDC_RADIO_AUTOSORT_0))->SetCheck(BST_CHECKED);
	else if (m_bAutoSort == 1)	((CButton*)GetDlgItem(IDC_RADIO_AUTOSORT_1))->SetCheck(BST_CHECKED);

	SetCheckByID(this, IDC_CHK_SHOWEVERYTIME, m_bShowEverytime);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}



void CDlgCFG_Load::OnOK()
{
	if (IsChecked(this, IDC_RADIO_LOAD_0) == TRUE) m_nLoadType = 0;
	else if (IsChecked(this, IDC_RADIO_LOAD_1) == TRUE) m_nLoadType = 1;
	else if (IsChecked(this, IDC_RADIO_LOAD_2) == TRUE) m_nLoadType = 2;

	if (IsChecked(this, IDC_RADIO_AUTOSORT_0) == TRUE) m_bAutoSort = FALSE;
	else if (IsChecked(this, IDC_RADIO_AUTOSORT_1) == TRUE) m_bAutoSort = TRUE;

	m_bShowEverytime = IsChecked(this, IDC_CHK_SHOWEVERYTIME);
	CDialogEx::OnOK();
}


void CDlgCFG_Load::OnCancel()
{
	CDialogEx::OnCancel();
}

