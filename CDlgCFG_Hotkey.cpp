// CDlgCFG_Hotkey.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "afxdialogex.h"
#include "CDlgCFG_Hotkey.h"
#include "EtcFunctions.h"
#include "CDlgHotKey.h"


// CDlgCFG_Hotkey 대화 상자

IMPLEMENT_DYNAMIC(CDlgCFG_Hotkey, CDialogEx)

CDlgCFG_Hotkey::CDlgCFG_Hotkey(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CFG_HOTKEY, pParent)
{
	m_pMenu = NULL;
}

CDlgCFG_Hotkey::~CDlgCFG_Hotkey()
{
}

void CDlgCFG_Hotkey::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_HOTKEY, m_listHotKey);
}


BEGIN_MESSAGE_MAP(CDlgCFG_Hotkey, CDialogEx)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_HOTKEY, &CDlgCFG_Hotkey::OnDblclkListHotkey)
	ON_BN_CLICKED(IDC_BTN_EDIT_HOTKEY, &CDlgCFG_Hotkey::OnBnClickedBtnEditHotkey)
	ON_BN_CLICKED(IDC_BTN_CLEAR_HOTKEY, &CDlgCFG_Hotkey::OnBnClickedBtnClearHotkey)
	ON_BN_CLICKED(IDC_BTN_DEFAULT_HOTKEY, &CDlgCFG_Hotkey::OnBnClickedBtnDefaultHotkey)
END_MESSAGE_MAP()


// CDlgCFG_Hotkey 메시지 처리기


BOOL CDlgCFG_Hotkey::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_listHotKey.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_listHotKey.InsertColumn(0, IDSTR(IDS_HOTKEY_COMMAND), LVCFMT_LEFT, 250);
	m_listHotKey.InsertColumn(1, IDSTR(IDS_HOTKEY_KEY), LVCFMT_LEFT, 150);

	LoadHotKey();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgCFG_Hotkey::OnOK()
{
	CDialogEx::OnOK();
}

void CDlgCFG_Hotkey::OnCancel()
{
	CDialogEx::OnCancel();
}

void CDlgCFG_Hotkey::OnDblclkListHotkey(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	OnBnClickedBtnEditHotkey();
	*pResult = 0;
}


void CDlgCFG_Hotkey::OnBnClickedBtnEditHotkey()
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
			m_listHotKey.EnsureVisible(nItem, FALSE);
		}
	}
}


void CDlgCFG_Hotkey::OnBnClickedBtnClearHotkey()
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


void CDlgCFG_Hotkey::OnBnClickedBtnDefaultHotkey()
{
	if (AfxMessageBox(IDSTR(IDS_RESET_HOTKEY), MB_YESNO) == IDNO) return;
	APP()->InitHotKey();
	LoadHotKey();
}


void CDlgCFG_Hotkey::LoadHotKey()
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
