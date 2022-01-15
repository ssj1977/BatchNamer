#pragma once

// CDlgPreset 대화 상자

class CDlgPreset : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPreset)

public:
	CDlgPreset(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgPreset();

	CToolBar m_toolPreset;
	int m_nLogFontHeight;

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
	afx_msg void OnBnClickedBtnPresetTaskDelete();
	afx_msg void OnBnClickedBtnPresetTaskEdit();
	afx_msg void OnSelchangeCbPresetSelect();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnPresetUp();
	afx_msg void OnBnClickedBtnPresetDown();

	void ArrangeCtrl();
	void SwapListItem(int n1, int n2);
	void SetListTask(int nItem, PresetTask& task);
	afx_msg void OnDblclkListPreset(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnPresetName();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
//	afx_msg void OnBnClickedRadioApplyMove();
//	afx_msg void OnBnClickedRadioApplyCopy();
};