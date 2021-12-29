
// BatchNamerDlg.h: 헤더 파일
//

#pragma once

#include "CNameListCtrl.h"

// CBatchNamerDlg 대화 상자
class CBatchNamerDlg : public CDialogEx
{
// 생성입니다.
public:
	CBatchNamerDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	CToolBar m_tool1;
	CToolBar m_tool2;
	BOOL m_bSelected;
	int m_nDefault_FontSize;
	COLORREF m_clrDefault_Bk;
	COLORREF m_clrDefault_Text;
	CFont m_font;
	int m_lfHeight;
	HIMAGELIST* m_pSysImgList;
	int m_nTempLoadType;
	void UpdateImageList();
	void UpdateFontSize();
	void ConfigLoadType();
	void ConfigViewOption();
	void ConfigEtc();
	void UpdateCount();
	void UpdateColumnSizes();
	void UpdateToolBar();
	void UpdateMenuEnable();
	void UpdateMenuPreset();
	void UpdateMenuHotkey();
	void ArrangeCtrl();
	static UINT ApplyChange_Thread(void* lParam);
// 대화 상자 데이터입니다.
//#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BATCHNAMER };
	CNameListCtrl	 m_list;
//#endif

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// 구현입니다.
protected:
	void AddByFolderPicker();
	void AddByFileDialog();
	void AddPath(CString strFile, BOOL bIsDirectory);
	void AddPathStart(CString strPath);
	void AddListItem(WIN32_FIND_DATA& fd, CString strDir);
	void SortList();
	void ClearList();
	void ToggleListColumn(int nCol);
	void Export(int nMode);
	void ImportNewName(BOOL bFromFile);
	void ImportPath(BOOL bFromFile);
	void ListDown();
	void ListUp();
	void SwapItem(int n1, int n2);
	void ApplyChange(int nApplyOption);
	void ApplyChange_Start();
	void UndoChanges(BOOL bSelectedOnly = FALSE);
	void PresetEdit();
	void PresetApply(BatchNamerPreset& preset);
	CString GetItemFullPath(int nItem, BOOL bOld = TRUE);
	void ManualChange();
	void NameEmpty(BOOL bToggleRedraw = TRUE);
	void NameNumberFilter(BOOL bRemoveNumber); // , BOOL bToggleRedraw = TRUE);
	void NameRemoveSelected();
	void NameRemoveSelected(int nSubCommand, CString str1, CString str2);
	void NameSetFolder();
	void NameSetFolder(int nSubCommand, CString str1, CString str2);
	void NameAddNum();
	void NameAddNum(int nSubCommand, CString str1, CString str2);
	void NameDigit();
	void NameDigit(int nSubCommand, CString str1, CString str2);
	//Add, Replace는 확장자에도 사용할 수 있도록 구현
	void NameAdd(BOOL bFront);
	void ExtAdd();
	void StringAdd(int nSubCommand, CString str1, CString str2, BOOL bFront, BOOL bForExt);
	void NameReplace();
	void ExtReplace();
	void StringReplace(int nSubCommand, CString str1, CString str2, BOOL bForExt);
	//void ExtReplace(int nSubCommand, CString str1, CString str2, BOOL bForExt);
	//void ExtAdd(int nSubCommand, CString str1, CString str2);
	void ExtDel(BOOL bToggleRedraw = TRUE);
	CString ProcessDropFiles(HDROP hDropInfo, int nActionType); // nActionType 0:목록에 추가, 1:경로를 텍스트로 반환, 2:이름을 텍스트로 반환
	void LoadPathArray(CStringArray& aPath);
	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnDblclkListFile(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnStopthread();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnInitMenu(CMenu* pMenu);
};
