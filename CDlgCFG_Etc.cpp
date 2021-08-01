// CDlgCFG_Etc.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgCFG_Etc.h"
#include "afxdialogex.h"


// CDlgCFG_Etc 대화 상자

IMPLEMENT_DYNAMIC(CDlgCFG_Etc, CDialogEx)

CDlgCFG_Etc::CDlgCFG_Etc(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CFG_ETC, pParent)
{

}

CDlgCFG_Etc::~CDlgCFG_Etc()
{
}

void CDlgCFG_Etc::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgCFG_Etc, CDialogEx)
END_MESSAGE_MAP()


// CDlgCFG_Etc 메시지 처리기


BOOL CDlgCFG_Etc::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgCFG_Etc::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnOK();
}


void CDlgCFG_Etc::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


BOOL CDlgCFG_Etc::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CDialogEx::PreTranslateMessage(pMsg);
}
