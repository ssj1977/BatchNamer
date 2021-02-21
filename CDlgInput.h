#pragma once

struct InputItem
{
	CString m_strItemName;	//콤보박스에 들어갈 이름
	int m_nCommand;		//동작하는 기능 종류  
	CString m_strLabel1;	//첫번째 입력창 상단 라벨
	BOOL m_bIsNumber1;		//첫번째 입력창이 숫자만 받는지 여부
	CString m_strLabel2;	//두번째 입력창 상단 라벨
	BOOL m_bIsNumber2;		//두번째 입력창이 숫자만 받는지 여부
	InputItem()
	{
		m_nCommand = 0;
		m_bIsNumber1 = FALSE;
		m_bIsNumber2 = FALSE;
	}
};


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
