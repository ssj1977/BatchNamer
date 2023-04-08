#pragma once
#include "afxdialogex.h"


// CDlgCFG_Hotkey 대화 상자

class CDlgCFG_Hotkey : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCFG_Hotkey)

public:
	CDlgCFG_Hotkey(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgCFG_Hotkey();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	CListCtrl m_listHotKey;
	afx_msg void OnDblclkListHotkey(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedBtnEditHotkey();
	afx_msg void OnBnClickedBtnClearHotkey();
	afx_msg void OnBnClickedBtnDefaultHotkey();
	void LoadHotKey();

	CMenu* m_pMenu;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CFG_HOTKEY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

};
