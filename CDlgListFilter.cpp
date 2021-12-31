// CDlgListFilter.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgListFilter.h"
#include "afxdialogex.h"
#include "EtcFunctions.h"

CString ExtractWildCard(CString str, BOOL bAddToken);
// CDlgListFilter 대화 상자

IMPLEMENT_DYNAMIC(CDlgListFilter, CDialogEx)

CDlgListFilter::CDlgListFilter(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LIST_FILTER, pParent)
{
	m_nClearOption = CLEAR_LIST_ALL;
}

CDlgListFilter::~CDlgListFilter()
{
}

void CDlgListFilter::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgListFilter, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_FILTER, &CDlgListFilter::OnBnClickedBtnFilter)
	ON_BN_CLICKED(IDC_BTN_FILTER_INVERT, &CDlgListFilter::OnBnClickedBtnFilterInvert)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CDlgListFilter::OnBnClickedBtnClear)
END_MESSAGE_MAP()


// CDlgListFilter 메시지 처리기


BOOL CDlgListFilter::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgListFilter::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CDlgListFilter::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnOK();
}


BOOL CDlgListFilter::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			CWnd* pWnd = GetFocus();
			if (pWnd == GetDlgItem(IDC_EDIT_FILTER_NAME_WORD) || pWnd == GetDlgItem(IDC_EDIT_FILTER_EXT_WORD))
			{ //필터 입력창에 포커스가 있는 상태에서 엔터를 눌렀을때
				return TRUE;
			}
			OnBnClickedBtnClear();
			return TRUE;
		}
		else if (pMsg->wParam == VK_ESCAPE)
		{
			OnCancel();
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CDlgListFilter::OnBnClickedBtnClear()
{
	m_nClearOption = CLEAR_LIST_ALL;
	OnOK();
}

void CDlgListFilter::OnBnClickedBtnFilter()
{
	m_nClearOption = CLEAR_LIST_BYFILTER;
	if (UpdateFilters(TRUE) == FALSE) return;
	OnOK();
}


void CDlgListFilter::OnBnClickedBtnFilterInvert()
{
	m_nClearOption = CLEAR_LIST_BYFILTER_INVERT;
	if (UpdateFilters(TRUE) == FALSE) return;
	OnOK();
}


BOOL CDlgListFilter::ValidateFilter(CString strFilter)
{
	if (strFilter.IsEmpty() == FALSE)
	{
		CString str = ExtractWildCard(strFilter, TRUE);
		if (str.IsEmpty()) return FALSE;
		// ** , ?*, ?* 는 비정상
		if (str.Find(_T("**")) != -1 ||
			str.Find(_T("*?")) != -1 ||
			str.Find(_T("?*")) != -1) return FALSE;
		// 상수 문자열 토큰(_)이 하나도 없으면 비정상
		if (str.Find(_T("_")) == -1) return FALSE;
	}
	return TRUE;
}

BOOL CDlgListFilter::UpdateFilters(BOOL bShowMsg)
{
	GetDlgItemText(IDC_EDIT_FILTER_NAME_WORD, m_strFilter_Name);
	GetDlgItemText(IDC_EDIT_FILTER_EXT_WORD, m_strFilter_Ext);
	if (m_strFilter_Name.IsEmpty() && m_strFilter_Ext.IsEmpty())
	{
		if (bShowMsg) AfxMessageBox(IDSTR(IDS_FILTER_EMPTY));
		return FALSE;
	}
	if (ValidateFilter(m_strFilter_Name) == FALSE || ValidateFilter(m_strFilter_Ext) == FALSE)
	{
		if (bShowMsg) AfxMessageBox(IDSTR(IDS_INVALID_WILDCARD));
		return FALSE;
	}
	return TRUE;
}



