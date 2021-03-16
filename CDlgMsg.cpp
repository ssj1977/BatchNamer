// CDlgMsg.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgMsg.h"
#include "afxdialogex.h"


// CDlgMsg 대화 상자

IMPLEMENT_DYNAMIC(CDlgMsg, CDialogEx)

CDlgMsg::CDlgMsg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MSG, pParent)
{

}

CDlgMsg::~CDlgMsg()
{
}

void CDlgMsg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgMsg, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CDlgMsg 메시지 처리기


void CDlgMsg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	CWnd* pWnd = GetDlgItem(IDC_EDIT_MSG);
	if (pWnd != NULL && ::IsWindow(pWnd->GetSafeHwnd())) ArrangeCtrl();
}

void CDlgMsg::ArrangeCtrl()
{
	CRect rc, rcBtn;
	GetClientRect(rc);
	GetDlgItem(IDCANCEL)->GetWindowRect(rcBtn);
	int BH = rcBtn.Height();
	int LH = m_nLogFontHeight;
	rc.DeflateRect(LH, LH, LH, LH);
	rc.bottom = rc.bottom - BH - LH;
	GetDlgItem(IDC_EDIT_MSG)->MoveWindow(rc);
	rc.top = rc.bottom + LH;
	rc.bottom = rc.top + BH;
	GetDlgItem(IDCANCEL)->MoveWindow(rc);
}



BOOL CDlgMsg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(APP()->m_hIcon, TRUE);		// Set big icon
	SetIcon(APP()->m_hIcon, FALSE);		// Set small icon

	LOGFONT lf;
	GetFont()->GetLogFont(&lf);
	m_nLogFontHeight = abs(lf.lfHeight);
	SetWindowText(m_strTitle);
	SetDlgItemText(IDC_EDIT_MSG, m_strMsg);
	ArrangeCtrl();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgMsg::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CDlgMsg::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnOK();
}
