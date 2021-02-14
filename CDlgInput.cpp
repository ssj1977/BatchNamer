﻿// CDlgInput.cpp: 구현 파일
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
	m_nDlgType = INPUT_TWO;
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

	if (m_aOptionText.GetSize() > 0 && m_aOptionText.GetSize() == m_aOptionData.GetSize())
	{
		int nItem;
		for (int i = 0; i < m_aOptionText.GetSize(); i++)
		{
			nItem = m_cb.AddString(m_aOptionText[i]);
			m_cb.SetItemData(nItem, m_aOptionData[i]);
		}
		m_cb.SetCurSel(m_nCB);
	}
	else
	{
		m_cb.ShowWindow(SW_HIDE);
	}
	ArrangeCtrl();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgInput::ArrangeCtrl()
{
	SetDlgItemText(IDC_STATIC_TITLE, m_strTitle);
	SetDlgItemText(IDC_STATIC_1, m_strStatic1);
	SetDlgItemText(IDC_STATIC_2, m_strStatic2);
	SetDlgItemText(IDC_EDIT_1, m_strReturn1);
	SetDlgItemText(IDC_EDIT_2, m_strReturn2);

	if (m_strStatic1.IsEmpty() == FALSE && m_nDlgType != INPUT_NONE)
	{
		GetDlgItem(IDC_STATIC_1)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_1)->ShowWindow(SW_SHOW);
		//GetDlgItem(IDC_STATIC_1)->EnableWindow(TRUE);
		//GetDlgItem(IDC_EDIT_1)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_STATIC_1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_1)->ShowWindow(SW_HIDE);
		//GetDlgItem(IDC_STATIC_1)->EnableWindow(FALSE);
		//GetDlgItem(IDC_EDIT_1)->EnableWindow(FALSE);
	}

	if (m_strStatic2.IsEmpty() == FALSE && m_nDlgType == INPUT_TWO)
	{
		GetDlgItem(IDC_STATIC_2)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_2)->ShowWindow(SW_SHOW);
		//GetDlgItem(IDC_STATIC_2)->EnableWindow(TRUE);
		//GetDlgItem(IDC_EDIT_2)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_STATIC_2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_2)->ShowWindow(SW_HIDE);
		//GetDlgItem(IDC_STATIC_2)->EnableWindow(FALSE);
		//GetDlgItem(IDC_EDIT_2)->EnableWindow(FALSE);
	}
	//	if (m_strStatic1.IsEmpty() && m_strStatic2.IsEmpty()) m_cb.MoveWindow(20,40,300,500);
}

void CDlgInput::InitInputDlg(CString strTitle, CString strStatic1, CString strStatic2)
{
	m_strTitle = strTitle;
	m_strStatic1 = strStatic1;
	m_strStatic2 = strStatic2;
}

void CDlgInput::OnOK()
{
	GetDlgItemText(IDC_EDIT_1, m_strReturn1);
	GetDlgItemText(IDC_EDIT_2, m_strReturn2);
	m_nCB = m_cb.GetCurSel();
	CDialogEx::OnOK();
}

void CDlgInput::InitValue(CString str1, CString str2)
{
	m_strReturn1 = str1;
	m_strReturn2 = str2;
}


/////////////////////////////////////////////////////////////////////////////
// CDlgInput message handlers

void CDlgInput::OnCancel()
{
	// TODO: Add extra cleanup here

	CDialogEx::OnCancel();
}

void CDlgInput::AddOption(CString strText, UINT nData)
{
	m_aOptionText.Add(strText);
	m_aOptionData.Add(nData);
}

void CDlgInput::OnSelchangeCbInput()
{
	if (m_aOptionData.GetSize() > 0) m_nDlgType = m_cb.GetItemData(m_cb.GetCurSel());
	ArrangeCtrl();
}
