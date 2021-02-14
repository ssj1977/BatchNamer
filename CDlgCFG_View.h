#pragma once


// CDlgCFG_View 대화 상자

class CDlgCFG_View : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCFG_View)

public:
	CDlgCFG_View(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgCFG_View();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CFG_VIEW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	COLORREF m_clrText;
	COLORREF m_clrBk;
	BOOL m_bUseDefaultColor;
	int m_nFontSize;
	BOOL m_bUseDefaultFont;
	int m_nIconType;
	void UpdateControl();

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBnClickedCheckDefaultColor();
	afx_msg void OnBnClickedCheckDefaultFont();
};
