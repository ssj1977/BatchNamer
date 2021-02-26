#pragma once

// CDlgInput 대화 상자

class CDlgInput : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgInput)

public:
	CDlgInput(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgInput();
	void AddOption(InputItem* pItem);
	void InitValue(CString str1, CString str2);
	void InitInputDlg(InputItem* pItem);
	InputItem* GetCurrentItem();
	CString m_strTitle;
	CString m_strReturn2;
	CString m_strReturn1;
	CPtrArray m_aInputItemPtr;
	int m_nCB;
// 대화 상자 데이터입니다.
//#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INPUT };
	CComboBox	m_cb;
//#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelchangeCbInput();
	DECLARE_MESSAGE_MAP()
};
