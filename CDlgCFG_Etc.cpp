// CDlgCFG_Etc.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgCFG_Etc.h"
#include "afxdialogex.h"
#include "EtcFunctions.h"


// CDlgCFG_Etc 대화 상자

IMPLEMENT_DYNAMIC(CDlgCFG_Etc, CDialogEx)

CDlgCFG_Etc::CDlgCFG_Etc(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CFG_ETC, pParent)
{
	m_pMenu = NULL;
}

CDlgCFG_Etc::~CDlgCFG_Etc()
{
}

void CDlgCFG_Etc::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_HOTKEY, m_listHotKey);
}


BEGIN_MESSAGE_MAP(CDlgCFG_Etc, CDialogEx)
END_MESSAGE_MAP()


// CDlgCFG_Etc 메시지 처리기


BOOL CDlgCFG_Etc::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_listHotKey.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	((CButton*)GetDlgItem(IDC_CHK_NAMEAUTOFIX))->SetCheck(m_bNameAutoFix);
	m_listHotKey.InsertColumn(0, L"Command", LVCFMT_LEFT, 200);
	m_listHotKey.InsertColumn(1, L"Key", LVCFMT_LEFT, 100);
	m_listHotKey.InsertColumn(2, L"Shift", LVCFMT_CENTER, 50);
	m_listHotKey.InsertColumn(3, L"Ctrl", LVCFMT_CENTER, 50);

	CHotKeyMap& hkm = APP()->m_mapHotKey;
	CHotKeyMap::iterator i;
	CString strCmd;
	int nPos = -1;
	for(i = hkm.begin(); i != hkm.end(); i++)
	{
		int nCommand = i->first;
		HotKey hk = i->second;
		if (m_pMenu == NULL) strCmd.Empty();
		else
		{
			m_pMenu->GetMenuString(nCommand, strCmd, MF_BYCOMMAND);
			nPos = strCmd.Find(L'\t');
			if (nPos != -1) strCmd = strCmd.Left(nPos);
		}
		int nItem = m_listHotKey.InsertItem(m_listHotKey.GetItemCount(), strCmd);
		m_listHotKey.SetItemText(nItem, 1, ConvertKeyCodeToName(hk.nKeyCode));
		m_listHotKey.SetItemText(nItem, 2, (hk.bShift ? L"O" : L""));
		m_listHotKey.SetItemText(nItem, 3, (hk.bCtrl ? L"O" : L""));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgCFG_Etc::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_bNameAutoFix = ((CButton*)GetDlgItem(IDC_CHK_NAMEAUTOFIX))->GetCheck();
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
