#pragma once


// CDlgSort 대화 상자

class CDlgSort : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSort)

public:
	CDlgSort(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgSort();
	CWnd* m_pSortWnd; // 값이 NULL이라면 프리셋용 설정창
	int m_nSortCol;
	BOOL m_bSortAscend;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SORT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
};
