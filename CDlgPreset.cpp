// CDlgPreset.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgPreset.h"
#include "afxdialogex.h"


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
	ON_BN_CLICKED(IDC_BTN_PRESET_TASK_ADD, &CDlgPreset::OnBnClickedBtnPresetTaskAdd)
	ON_BN_CLICKED(IDC_BTN_PRESET_TASK_DELETE, &CDlgPreset::OnBnClickedBtnPresetTaskDelete)
	ON_BN_CLICKED(IDC_BTN_PRESET_TASK_EDIT, &CDlgPreset::OnBnClickedBtnPresetTaskEdit)
	ON_CBN_SELCHANGE(IDC_CB_PRESET_COMMAND, &CDlgPreset::OnSelchangeCbPresetCommand)
	ON_CBN_SELCHANGE(IDC_CB_PRESET_SELECT, &CDlgPreset::OnSelchangeCbPresetSelect)
END_MESSAGE_MAP()


// CDlgPreset 메시지 처리기

BOOL CDlgPreset::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	//Init Preset Mode
	CListCtrl* pListPreset = (CListCtrl*)GetDlgItem(IDC_LIST_PRESET);
	CComboBox* pCBPreset = (CComboBox*)GetDlgItem(IDC_CB_PRESET_SELECT);
	pListPreset->SetExtendedStyle(LVS_EX_FULLROWSELECT); 
	pListPreset->InsertColumn(0, L"기능", LVCFMT_LEFT, 150);
	pListPreset->InsertColumn(1, L"설정값1", LVCFMT_LEFT, 100);
	pListPreset->InsertColumn(2, L"설정값2", LVCFMT_LEFT, 100);

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


void CDlgPreset::OnBnClickedBtnPresetTaskAdd()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CDlgPreset::OnBnClickedBtnPresetTaskDelete()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CDlgPreset::OnBnClickedBtnPresetTaskEdit()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CDlgPreset::OnSelchangeCbPresetCommand()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CDlgPreset::OnSelchangeCbPresetSelect()
{
	CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_PRESET_SELECT);
	int nIndex = pCB->GetCurSel();
}
