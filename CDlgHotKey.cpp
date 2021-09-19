// CDlgHotKey.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgHotKey.h"
#include "afxdialogex.h"


// CDlgHotKey 대화 상자

IMPLEMENT_DYNAMIC(CDlgHotKey, CDialogEx)

CDlgHotKey::CDlgHotKey(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HOTKEY, pParent)
{

}

CDlgHotKey::~CDlgHotKey()
{
}

void CDlgHotKey::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgHotKey, CDialogEx)
END_MESSAGE_MAP()


// CDlgHotKey 메시지 처리기

CString ConvertKeyCodeToName(DWORD code);

BOOL CDlgHotKey::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN && 
		pMsg->wParam != VK_CONTROL &&
		pMsg->wParam != VK_SHIFT &&
		pMsg->wParam != VK_ESCAPE &&
		pMsg->wParam != VK_RETURN )
	{
		BOOL bCtrl = (GetKeyState(VK_CONTROL) & 0xFF00);
		BOOL bShift = (GetKeyState(VK_SHIFT) & 0xFF00);
		((CButton*)GetDlgItem(IDC_CHK_CTRL))->SetCheck(bCtrl);
		((CButton*)GetDlgItem(IDC_CHK_SHIFT))->SetCheck(bShift);
		CString strKey = ConvertKeyCodeToName((DWORD)pMsg->wParam);
		SetDlgItemText(IDC_EDIT_HOTKEY, strKey);
		m_nKeyCode = (int)pMsg->wParam;
		return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgHotKey::OnOK()
{
	m_hk.bCtrl = ((CButton*)GetDlgItem(IDC_CHK_CTRL))->GetCheck();
	m_hk.bShift = ((CButton*)GetDlgItem(IDC_CHK_SHIFT))->GetCheck();
	m_hk.nKeyCode = m_nKeyCode;
	CDialogEx::OnOK();
}


void CDlgHotKey::OnCancel()
{
	CDialogEx::OnCancel();
}


BOOL CDlgHotKey::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	((CButton*)GetDlgItem(IDC_CHK_CTRL))->SetCheck(m_hk.bCtrl);
	((CButton*)GetDlgItem(IDC_CHK_SHIFT))->SetCheck(m_hk.bShift);
	CString strKey = ConvertKeyCodeToName((DWORD)m_hk.nKeyCode);
	SetDlgItemText(IDC_EDIT_HOTKEY, strKey);
	m_nKeyCode = m_hk.nKeyCode;
	return TRUE;  
}
