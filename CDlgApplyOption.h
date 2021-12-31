#pragma once

// CDlgApplyOption 대화 상자

class CDlgApplyOption : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgApplyOption)

public:
	CDlgApplyOption(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgApplyOption();

	int m_nApplyOption;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_APPLY_OPTION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	virtual void OnCancel();
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnMove();
	afx_msg void OnBnClickedBtnCopy();
};
