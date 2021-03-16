#pragma once


// CDlgMsg 대화 상자

class CDlgMsg : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMsg)

public:
	CDlgMsg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgMsg();
	CString m_strTitle;
	CString m_strMsg;
	int m_nLogFontHeight;
	void ArrangeCtrl();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MSG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
};
