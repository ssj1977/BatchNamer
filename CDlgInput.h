#pragma once

// CDlgInput 대화 상자

class CDlgInput : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgInput)

public:
	CDlgInput(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgInput();
	void AddOption(InputItem* pItem);
	void InitValue(int nSubCommand, CString str1, CString str2);
	void SetInputItem(InputItem* pItem);
	void InitInputByCommand(int nCommand);
	BOOL VerifyReturnValue();
	int GetSubCommand();
	CString m_strTitle;
	CString m_strReturn2;
	CString m_strReturn1;
	InputItemArray m_aInput;
	int m_nCB;
	int m_nCommand;
	int m_nFontHeight;
	int m_nMinWidth;
	int m_nMinHeight;
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
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void ArrangeCtrl();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnBnClickedBtnFolderSelect();
};
