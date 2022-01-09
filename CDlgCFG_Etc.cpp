// CDlgCFG_Etc.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgCFG_Etc.h"
#include "afxdialogex.h"
#include "EtcFunctions.h"
#include "CDlgHotKey.h"

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
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_HOTKEY, &CDlgCFG_Etc::OnDblclkListHotkey)
	ON_BN_CLICKED(IDC_BTN_EDIT_HOTKEY, &CDlgCFG_Etc::OnBnClickedBtnEditHotkey)
	ON_BN_CLICKED(IDC_BTN_CLEAR_HOTKEY, &CDlgCFG_Etc::OnBnClickedBtnClearHotkey)
	ON_BN_CLICKED(IDC_BTN_DEFAULT_HOTKEY, &CDlgCFG_Etc::OnBnClickedBtnDefaultHotkey)
END_MESSAGE_MAP()


// CDlgCFG_Etc 메시지 처리기


BOOL CDlgCFG_Etc::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_listHotKey.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	SetCheckByID(this, IDC_CHK_NAMEAUTOFIX, m_bNameAutoFix);
	SetCheckByID(this, IDC_CHK_USETHREAD, m_bUseThread);
	m_listHotKey.InsertColumn(0, IDSTR(IDS_HOTKEY_COMMAND), LVCFMT_LEFT, 250);
	m_listHotKey.InsertColumn(1, IDSTR(IDS_HOTKEY_KEY), LVCFMT_LEFT, 150);

	LoadHotKey();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgCFG_Etc::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_bNameAutoFix = IsChecked(this, IDC_CHK_NAMEAUTOFIX);
	m_bUseThread = IsChecked(this, IDC_CHK_USETHREAD);
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


void CDlgCFG_Etc::OnDblclkListHotkey(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	OnBnClickedBtnEditHotkey();
	*pResult = 0;
}


void CDlgCFG_Etc::OnBnClickedBtnEditHotkey()
{
	int nItem = m_listHotKey.GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	CHotKeyMap& hkm = APP()->m_mapHotKey;
	int nCommand = (int)m_listHotKey.GetItemData(nItem);
	CHotKeyMap::iterator i;
	i = hkm.find(nCommand);
	if (i != hkm.end())
	{
		CDlgHotKey dlg;
		dlg.m_hk = i->second;
		if (dlg.DoModal() == IDOK)
		{
			i->second = dlg.m_hk;
			m_listHotKey.SetItemText(nItem, 1, i->second.GetKeyString());
		}
	}
}


void CDlgCFG_Etc::OnBnClickedBtnClearHotkey()
{
	int nItem = m_listHotKey.GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	CHotKeyMap& hkm = APP()->m_mapHotKey;
	int nCommand = (int)m_listHotKey.GetItemData(nItem);
	CHotKeyMap::iterator i;
	i = hkm.find(nCommand);
	i->second.nKeyCode = 0;
	i->second.bCtrl = FALSE;
	i->second.bShift = FALSE;
	CString strKey = i->second.GetKeyString();
	if (strKey.IsEmpty()) strKey.LoadStringW(IDS_NOHOTKEY);
	m_listHotKey.SetItemText(nItem, 1, strKey);
}


void CDlgCFG_Etc::OnBnClickedBtnDefaultHotkey()
{
	if (AfxMessageBox(IDSTR(IDS_RESET_HOTKEY), MB_YESNO) == IDNO) return;
	APP()->InitHotKey();
	LoadHotKey();
}


void CDlgCFG_Etc::LoadHotKey()
{
	m_listHotKey.DeleteAllItems();
	CHotKeyMap& hkm = APP()->m_mapHotKey;
	CHotKeyMap::iterator i;
	CString strCmd, strKey;
	int nPos = -1;
	for (i = hkm.begin(); i != hkm.end(); i++)
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
		strKey = hk.GetKeyString();
		if (strKey.IsEmpty()) strKey.LoadStringW(IDS_NOHOTKEY);
		m_listHotKey.SetItemText(nItem, 1, strKey);
		m_listHotKey.SetItemData(nItem, nCommand);
	}
}
