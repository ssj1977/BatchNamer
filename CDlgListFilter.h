#pragma once

#ifndef CLEAR_LIST_ALL
#define CLEAR_LIST_ALL 0
#define CLEAR_LIST_BYFILTER 1
#define CLEAR_LIST_BYFILTER_INVERT 2
#endif

// CDlgListFilter 대화 상자

class CDlgListFilter : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgListFilter)

public:
	CDlgListFilter(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgListFilter();

	int m_nClearOption;
	CString m_strFilter_Name;
	CString m_strFilter_Ext;


// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LIST_FILTER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnFilter();
	afx_msg void OnBnClickedBtnFilterInvert();
	afx_msg void OnBnClickedBtnClear();
};
