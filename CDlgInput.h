#pragma once

#ifndef INPUT_NONE
#define INPUT_NONE 0
#define INPUT_ONE 1
#define INPUT_TWO 2
#endif
// CDlgInput 대화 상자

class CDlgInput : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgInput)

public:
	CDlgInput(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgInput();
	void AddOption(CString strText, UINT nData);
	void InitValue(CString str1, CString str2);
	void InitInputDlg(CString strTitle, CString strStatic1, CString strStatic2);
	CString m_strTitle;
	void ArrangeCtrl();
	CString m_strStatic2;
	CString m_strStatic1;
	CString m_strReturn2;
	CString m_strReturn1;
	CStringArray m_aOptionText;
	CUIntArray m_aOptionData;
	int m_nCB;
	int m_nDlgType;
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
