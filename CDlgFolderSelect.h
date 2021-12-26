#pragma once


// CDlgFolderSelect 대화 상자

class CDlgFolderSelect : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgFolderSelect)

public:
	CDlgFolderSelect(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgFolderSelect();

	BOOL m_bUseParent;
	CString m_strFolder;
	int m_nLevel;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FOLDER_SELECT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnBnClickedBtnFolderSelect();
	afx_msg void OnBnClickedRadioParentFolder();
	afx_msg void OnBnClickedRadioSpecificFolder();
	afx_msg void OnBnClickedCancel();
};
