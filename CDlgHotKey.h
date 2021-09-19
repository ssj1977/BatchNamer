#pragma once


// CDlgHotKey 대화 상자

class CDlgHotKey : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgHotKey)

public:
	CDlgHotKey(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgHotKey();
	HotKey m_hk;
	HotKey m_hkTemp;
	int m_nKeyCode;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HOTKEY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
};
