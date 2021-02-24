#pragma once


// CDlgSort 대화 상자

class CDlgSort : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSort)

public:
	CDlgSort(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgSort();
	void SetCheckByID(int nID, BOOL bCheck);
	BOOL GetCheckByID(int nID);
	int m_nSortCol;
	BOOL m_bAsc;

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
