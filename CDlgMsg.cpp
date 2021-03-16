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
END_MESSAGE_MAP()


// CDlgMsg 메시지 처리기
