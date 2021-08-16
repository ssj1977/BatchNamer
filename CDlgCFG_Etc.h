#pragma once


// CDlgCFG_Etc 대화 상자

class CDlgCFG_Etc : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCFG_Etc)

public:
	CDlgCFG_Etc(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgCFG_Etc();
	BOOL m_bNameAutoFix;
	CMenu* m_pMenu;
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CFG_ETC };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CListCtrl m_listHotKey;
};
