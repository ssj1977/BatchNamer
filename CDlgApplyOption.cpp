// CDlgApplyOption.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgApplyOption.h"
#include "afxdialogex.h"


// CDlgApplyOption 대화 상자

IMPLEMENT_DYNAMIC(CDlgApplyOption, CDialogEx)

CDlgApplyOption::CDlgApplyOption(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_APPLY_OPTION, pParent)
{
	m_nApplyOption = APPLY_MOVE;
}

CDlgApplyOption::~CDlgApplyOption()
{
}

void CDlgApplyOption::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgApplyOption, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_MOVE, &CDlgApplyOption::OnBnClickedBtnMove)
	ON_BN_CLICKED(IDC_BTN_COPY, &CDlgApplyOption::OnBnClickedBtnCopy)
END_MESSAGE_MAP()


// CDlgApplyOption 메시지 처리기


void CDlgApplyOption::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CDlgApplyOption::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnOK();
}


BOOL CDlgApplyOption::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


BOOL CDlgApplyOption::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		BOOL bShift = ((GetKeyState(VK_SHIFT) & 0xFF00) != 0);

		if (pMsg->wParam == VK_RETURN && bShift == FALSE)
		{
			OnBnClickedBtnMove();
			return TRUE;
		}
		else if (pMsg->wParam == VK_RETURN && bShift == TRUE)
		{
			OnBnClickedBtnCopy();
			return TRUE;
		}
		else if (pMsg->wParam == VK_ESCAPE)
		{
			OnCancel();
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgApplyOption::OnBnClickedBtnMove()
{
	m_nApplyOption = APPLY_MOVE;
	OnOK();
}


void CDlgApplyOption::OnBnClickedBtnCopy()
{
	m_nApplyOption = APPLY_COPY;
	OnOK();
}
