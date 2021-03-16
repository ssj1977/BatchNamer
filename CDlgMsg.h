#pragma once


// CDlgMsg 대화 상자

class CDlgMsg : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMsg)

public:
	CDlgMsg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgMsg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MSG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
};
