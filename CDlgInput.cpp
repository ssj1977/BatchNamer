// CDlgInput.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgInput.h"
#include "afxdialogex.h"


// CDlgInput 대화 상자

IMPLEMENT_DYNAMIC(CDlgInput, CDialogEx)

CDlgInput::CDlgInput(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INPUT, pParent)
{
	m_nCB = 0;
}

CDlgInput::~CDlgInput()
{
}

void CDlgInput::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgInput)
	DDX_Control(pDX, IDC_CB_INPUT, m_cb);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgInput, CDialogEx)
	//{{AFX_MSG_MAP(CDlgInput)
	ON_CBN_SELCHANGE(IDC_CB_INPUT, OnSelchangeCbInput)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CDlgInput 메시지 처리기
BOOL CDlgInput::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(m_strTitle);
	if (m_strReturn1.IsEmpty() == FALSE) SetDlgItemText(IDC_EDIT_1, m_strReturn1);
	if (m_strReturn2.IsEmpty() == FALSE) SetDlgItemText(IDC_EDIT_2, m_strReturn2);

	if (m_aInputItemPtr.GetSize() > 0)
	{
		int nItem;
		for (int i = 0; i < m_aInputItemPtr.GetSize(); i++)
		{
			InputItem* pItem = (InputItem*)m_aInputItemPtr[i];
			nItem = m_cb.AddString(pItem->m_strItemName);
			m_cb.SetItemData(nItem, (DWORD_PTR)pItem);
		}
		m_cb.SetCurSel(m_nCB);
		OnSelchangeCbInput();
	}
	else
	{
		return FALSE;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgInput::InitInputDlg(InputItem* pItem)
{
	BOOL bShow1 = !(pItem->m_strLabel1.IsEmpty());
	BOOL bShow2 = !(pItem->m_strLabel2.IsEmpty());
	SetDlgItemText(IDC_STATIC_1, pItem->m_strLabel1);
	SetDlgItemText(IDC_STATIC_2, pItem->m_strLabel2);
	GetDlgItem(IDC_STATIC_1)->ShowWindow(bShow1 ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_EDIT_1)->ShowWindow(bShow1 ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_STATIC_2)->ShowWindow(bShow2 ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_EDIT_2)->ShowWindow(bShow2 ? SW_SHOW : SW_HIDE);
	HWND h1 = GetDlgItem(IDC_EDIT_1)->GetSafeHwnd();
	HWND h2 = GetDlgItem(IDC_EDIT_2)->GetSafeHwnd();
	SetWindowLong(h1, GWL_STYLE, pItem->m_bIsNumber1 
		? (GetWindowLong(h1, GWL_STYLE) | ES_NUMBER) : (GetWindowLong(h1, GWL_STYLE) & ~ES_NUMBER));
	SetWindowLong(h2, GWL_STYLE, pItem->m_bIsNumber2 
		? (GetWindowLong(h2, GWL_STYLE) | ES_NUMBER) : (GetWindowLong(h2, GWL_STYLE) & ~ES_NUMBER));
}

void CDlgInput::OnOK()
{
	GetDlgItemText(IDC_EDIT_1, m_strReturn1);
	GetDlgItemText(IDC_EDIT_2, m_strReturn2);
	m_nCB = m_cb.GetCurSel();
	CDialogEx::OnOK();
}

void CDlgInput::OnCancel()
{
	// TODO: Add extra cleanup here

	CDialogEx::OnCancel();
}

void CDlgInput::InitValue(CString str1, CString str2)
{
	m_strReturn1 = str1;
	m_strReturn2 = str2;
}
void CDlgInput::AddOption(InputItem* pItem)
{
	if (pItem == NULL) return;
	m_aInputItemPtr.Add(pItem);
}

void CDlgInput::OnSelchangeCbInput()
{
	int nSel = m_cb.GetCurSel();
	if (nSel < 0 || nSel >= m_aInputItemPtr.GetSize()) return;
	InputItem* pItem = (InputItem*)m_cb.GetItemData(nSel);
	InitInputDlg(pItem);
}

InputItem* CDlgInput::GetCurrentItem()
{
	return (InputItem*)m_aInputItemPtr[m_nCB];
}