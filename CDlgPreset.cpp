// CDlgPreset.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgPreset.h"
#include "afxdialogex.h"
#include "EtcFunctions.h"
#include "CDlgInput.h"


// CDlgPreset 대화 상자

IMPLEMENT_DYNAMIC(CDlgPreset, CDialogEx)

CDlgPreset::CDlgPreset(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PRESET, pParent)
{

}

CDlgPreset::~CDlgPreset()
{
}

void CDlgPreset::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgPreset, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_PRESET_TASK_DELETE, &CDlgPreset::OnBnClickedBtnPresetTaskDelete)
	ON_BN_CLICKED(IDC_BTN_PRESET_TASK_EDIT, &CDlgPreset::OnBnClickedBtnPresetTaskEdit)
	ON_CBN_SELCHANGE(IDC_CB_PRESET_SELECT, &CDlgPreset::OnSelchangeCbPresetSelect)
	ON_BN_CLICKED(IDC_BTN_PRESET_UP, &CDlgPreset::OnBnClickedBtnPresetUp)
	ON_BN_CLICKED(IDC_BTN_PRESET_DOWN, &CDlgPreset::OnBnClickedBtnPresetDown)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PRESET, &CDlgPreset::OnDblclkListPreset)
END_MESSAGE_MAP()


// CDlgPreset 메시지 처리기

BOOL CDlgPreset::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_toolPreset.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_WRAPABLE, WS_CHILD | WS_VISIBLE );
	m_toolPreset.LoadToolBar(IDR_TOOLBAR_PRESET);
	UINT nStyle;
	int nCount = m_toolPreset.GetCount();
	int nTextIndex = 0;
	int aID[] = {	IDS_TB_01, IDS_TB_02, IDS_TB_03, IDS_TB_04, IDS_TB_05, 
					IDS_TB_06, IDS_TB_07, IDS_TB_08, IDS_TB_09, IDS_TB_16,
					IDS_TB_17, IDS_TB_18, IDS_TB_19 };

	for (int i = 0; i < nCount; i++)
	{
		nStyle = m_toolPreset.GetButtonStyle(i);
		if (!(nStyle & TBBS_SEPARATOR))
		{
			m_toolPreset.SetButtonText(i, IDSTR(aID[nTextIndex]));
			nTextIndex += 1;
		}
	}
	CRect rc, rcButton, rcSplit, rcTool;
	GetDlgItem(IDC_ST_PRESET_ADD)->GetWindowRect(rcTool);
	ScreenToClient(rcTool);
	rcTool.DeflateRect(CRect(5, 25, 5, 5));
	m_toolPreset.MoveWindow(rcTool, TRUE);
	//GetClientRect(rc);
	//m_toolPreset.GetToolBarCtrl().GetItemRect(0, rcButton);
	//m_toolPreset.GetToolBarCtrl().GetItemRect(1, rcSplit);
	//int TOOLHEIGHT = (rcButton.Height() + rcSplit.Width()) * 13;
	//int TOOLWIDTH = (rcButton.Width() + rcSplit.Width());
	//m_toolPreset.MoveWindow(rc.Width()-TOOLWIDTH-10, 0, TOOLWIDTH, TOOLHEIGHT);
	
	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	//Init Preset Mode
	CListCtrl* pListPreset = (CListCtrl*)GetDlgItem(IDC_LIST_PRESET);
	CComboBox* pCBPreset = (CComboBox*)GetDlgItem(IDC_CB_PRESET_SELECT);
	pListPreset->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	pListPreset->InsertColumn(0, L"작업", LVCFMT_LEFT, APP()->m_nFontSize * 6);
	pListPreset->InsertColumn(1, L"기능", LVCFMT_LEFT, APP()->m_nFontSize * 18);
	pListPreset->InsertColumn(2, L"설정값1", LVCFMT_RIGHT, APP()->m_nFontSize * 7);
	pListPreset->InsertColumn(3, L"설정값2", LVCFMT_RIGHT, APP()->m_nFontSize * 7);

	PresetArray& aPreset = APP()->m_aPreset;
	CString strTemp;
	int nIndex = 0;
	for (int i = 0; i < aPreset.GetSize(); i++)
	{
		strTemp.Format(L"프리셋%d", i+1);
		nIndex = pCBPreset->AddString(strTemp);
	}
	pCBPreset->SetCurSel(0);
	OnSelchangeCbPresetSelect();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgPreset::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnOK();
}


void CDlgPreset::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CDlgPreset::ArrangeCtrl()
{
	//GetDlgItem(IDC_CB_PRESET)->MoveWindow(rcList.left, rcList.top, rcList.Width(), BARHEIGHT);
	//GetDlgItem(IDC_LIST_PRESET)->MoveWindow(rcList.left, rcList.top + BARHEIGHT, rcList.Width(), rcList.Height() - BARHEIGHT);
}


void CDlgPreset::OnBnClickedBtnPresetTaskDelete()
{
	CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_PRESET_SELECT);
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_PRESET);
	BatchNamerPreset& preset = APP()->m_aPreset[pCB->GetCurSel()];
	int nItem = pList->GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	pList->DeleteItem(nItem);
	preset.m_aTask.RemoveAt(nItem);

}

void CDlgPreset::OnBnClickedBtnPresetTaskEdit()
{
	CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_PRESET_SELECT);
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_PRESET);
	BatchNamerPreset& preset = APP()->m_aPreset[pCB->GetCurSel()];
	int nItem = pList->GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1) return;
	PresetTask& task = preset.m_aTask[nItem];
	BOOL bUseInputDlg = FALSE;
	switch (task.m_nCommand)
	{
//	case IDS_TB_01:	case IDS_TB_02:	case IDS_TB_03:	case IDS_TB_05:	
//	case IDS_TB_08:	case IDS_TB_09:	case IDS_TB_18:	case IDS_TB_19:
//		bUseInputDlg = TRUE;
//		break;
//	case IDS_TB_04: case IDS_TB_06: case IDS_TB_07: case IDS_TB_17:
//		break;
	case IDS_TB_16:
		if (TRUE)
		{
			CFolderPickerDialog dlg;
			CString strTitle;
			strTitle.LoadString(IDS_SETPARENT);
			dlg.GetOFN().lpstrTitle = strTitle;
			if (dlg.DoModal() == IDCANCEL) return;
			task.m_str1 = dlg.GetPathName();
		}
		break;
	default:
		bUseInputDlg = (task.m_nSubCommand != 0);
	}
	if (bUseInputDlg == TRUE)
	{
		CDlgInput dlg;
		dlg.InitInputByCommand(task.m_nCommand);
		dlg.InitValue(task.m_nSubCommand, task.m_str1, task.m_str2);
		if (dlg.DoModal() == IDCANCEL) return;
		if (dlg.VerifyReturnValue() == FALSE) return;
		task.m_nSubCommand = dlg.GetSubCommand();
		task.m_str1 = dlg.m_strReturn1;
		task.m_str2 = dlg.m_strReturn2;
	}
	SetListTask(nItem, task);
}

void CDlgPreset::OnSelchangeCbPresetSelect()
{
	CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_PRESET_SELECT);
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_PRESET);
	BatchNamerPreset& preset = APP()->m_aPreset[pCB->GetCurSel()];
	pList->DeleteAllItems();
	for (int i = 0; i<preset.m_aTask.GetSize(); i++)
	{
		SetListTask(i, preset.m_aTask[i]);
	}
}

BOOL CDlgPreset::OnCommand(WPARAM wParam, LPARAM lParam)
{
	BOOL bUseInputDlg = FALSE;
	int nCommand = 0, nSubCommand =0;
	CString str1, str2;
	switch (wParam)
	{
	case IDM_NAME_REPLACE:		nCommand = IDS_TB_01;	bUseInputDlg = TRUE;		break;
	case IDM_NAME_ADD_FRONT:	nCommand = IDS_TB_02;	bUseInputDlg = TRUE;		break;
	case IDM_NAME_ADD_REAR:		nCommand = IDS_TB_03;	bUseInputDlg = TRUE;		break;
	case IDM_NAME_EMPTY:		nCommand = IDS_TB_04;	break;
	case IDM_NAME_REMOVESELECTED: nCommand = IDS_TB_05;	bUseInputDlg = TRUE;		break;
	case IDM_NAME_EXTRACTNUMBER: nCommand = IDS_TB_06;	break;
	case IDM_NAME_REMOVENUMBER: nCommand = IDS_TB_07;	break;
	case IDM_NAME_DIGIT:		nCommand = IDS_TB_08;	bUseInputDlg = TRUE;		break;
	case IDM_NAME_ADDNUM:		nCommand = IDS_TB_09;	bUseInputDlg = TRUE;		break;
	case IDM_NAME_SETPARENT: 
		nCommand = IDS_TB_16;
		if (TRUE)
		{
			CFolderPickerDialog dlg;
			CString strTitle;
			strTitle.LoadString(IDS_SETPARENT);
			dlg.GetOFN().lpstrTitle = strTitle;
			if (dlg.DoModal() == IDCANCEL) return TRUE;
			str1 = dlg.GetPathName();
		}
		break;
	case IDM_EXT_DEL:			nCommand = IDS_TB_17;	break;
	case IDM_EXT_ADD:			nCommand = IDS_TB_18;	bUseInputDlg = TRUE;		break;
	case IDM_EXT_REPLACE:		nCommand = IDS_TB_19;	bUseInputDlg = TRUE;		break;
	default:
		return CDialogEx::OnCommand(wParam, lParam);
	}
	if (nCommand != 0)
	{
		if (bUseInputDlg == TRUE)
		{
			CDlgInput dlg;
			dlg.InitInputByCommand(nCommand);
			if (dlg.DoModal() == IDCANCEL) return TRUE;
			if (dlg.VerifyReturnValue() == FALSE) return TRUE;
			nSubCommand = dlg.GetSubCommand();
			str1 = dlg.m_strReturn1;
			str2 = dlg.m_strReturn2;
		}
		CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_PRESET_SELECT);
		BatchNamerPreset& preset = APP()->m_aPreset[pCB->GetCurSel()];
		CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_PRESET);
		int nItem = pList->GetItemCount();
		if (nItem == preset.m_aTask.GetSize())
		{
			PresetTask task;
			task.m_nCommand = nCommand;
			task.m_nSubCommand = nSubCommand;
			task.m_str1 = str1;
			task.m_str2 = str2;
			SetListTask(nItem, task);
			preset.m_aTask.Add(task);
		}
	}
	return TRUE;
}


BOOL CDlgPreset::PreTranslateMessage(MSG* pMsg)
{
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_PRESET);
	int nItem = pList->GetNextItem(-1, LVNI_SELECTED);
	BOOL bSelected = (nItem != -1);
	GetDlgItem(IDC_BTN_PRESET_DOWN)->EnableWindow(bSelected);
	GetDlgItem(IDC_BTN_PRESET_UP)->EnableWindow(bSelected);
	GetDlgItem(IDC_BTN_PRESET_TASK_DELETE)->EnableWindow(bSelected);
	if (bSelected)
	{
		CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_PRESET_SELECT);
		BatchNamerPreset& preset = APP()->m_aPreset[pCB->GetCurSel()];
		if (nItem < preset.m_aTask.GetSize())
		{
			PresetTask& task = preset.m_aTask[nItem];
			if (task.m_nSubCommand == 0 && task.m_str1.IsEmpty()) bSelected = FALSE;
		}
	}
	GetDlgItem(IDC_BTN_PRESET_TASK_EDIT)->EnableWindow(bSelected);
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgPreset::OnBnClickedBtnPresetUp()
{
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_PRESET);
	int nItem = pList->GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1 || (nItem-1) <0 ) return;
	SwapListItem(nItem, nItem - 1);
}


void CDlgPreset::OnBnClickedBtnPresetDown()
{
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_PRESET);
	int nItem = pList->GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1 || (nItem+1) > (pList->GetItemCount()-1) ) return;
	SwapListItem(nItem, nItem + 1);
}

void CDlgPreset::SwapListItem(int n1, int n2)
{
	CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_PRESET_SELECT);
	BatchNamerPreset& preset = APP()->m_aPreset[pCB->GetCurSel()];
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_PRESET);
	PresetTask task1(preset.m_aTask[n1]), task2(preset.m_aTask[n2]);
	preset.m_aTask[n1] = task2;
	preset.m_aTask[n2] = task1;
	SetListTask(n1, task2);
	SetListTask(n2, task1);
	pList->SetItemState(n2, LVIS_SELECTED, LVIS_SELECTED);
}

void CDlgPreset::SetListTask(int nItem, PresetTask& task)
{
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_PRESET);
	CString strCommand, strSubCommand;
	if (nItem >= pList->GetItemCount()) nItem = pList->InsertItem(pList->GetItemCount(), _T(""));
	if (task.m_nCommand > 0) strCommand.LoadString(task.m_nCommand);
	if (task.m_nSubCommand > 0) strSubCommand.LoadString(task.m_nSubCommand);
	pList->SetItemText(nItem, 0, strCommand);
	pList->SetItemText(nItem, 1, strSubCommand);
	pList->SetItemText(nItem, 2, task.m_str1);
	pList->SetItemText(nItem, 3, task.m_str2);
}


void CDlgPreset::OnDblclkListPreset(NMHDR* pNMHDR, LRESULT* pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	OnBnClickedBtnPresetTaskEdit();
	*pResult = 0;
}
