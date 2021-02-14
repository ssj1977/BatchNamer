#pragma once


// CDlgCFG_Load 대화 상자

class CDlgCFG_Load : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCFG_Load)

public:
	CDlgCFG_Load(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgCFG_Load();
	BOOL m_bShowEverytime;
	BOOL m_nLoadType;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CFG_LOAD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
};
