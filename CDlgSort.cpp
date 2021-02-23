// CDlgSort.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgSort.h"
#include "afxdialogex.h"


// CDlgSort 대화 상자

IMPLEMENT_DYNAMIC(CDlgSort, CDialogEx)

CDlgSort::CDlgSort(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SORT, pParent)
{

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

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgSort::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnOK();
}


void CDlgSort::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}
