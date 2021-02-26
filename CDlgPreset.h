#pragma once

// CDlgPreset 대화 상자

class CDlgPreset : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPreset)

public:
	CDlgPreset(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgPreset();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PRESET };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	virtual void OnCancel();
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	void ArrangeCtrl();
	afx_msg void OnBnClickedBtnPresetTaskAdd();
	afx_msg void OnBnClickedBtnPresetTaskDelete();
	afx_msg void OnBnClickedBtnPresetTaskEdit();
	afx_msg void OnSelchangeCbPresetCommand();
	afx_msg void OnSelchangeCbPresetSelect();
};