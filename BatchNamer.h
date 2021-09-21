
// BatchNamer.h: PROJECT_NAME 애플리케이션에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.
#include <afxwinappex.h>
#include <map>

//CDlgInput 및 CDlgPreset에 사용되는 입력창 설정용 구조체
struct InputItem
{
	CString m_strItemName;	//콤보박스에 들어갈 이름
	int m_nSubCommand;		//동작하는 기능 종류  
	CString m_strLabel1;	//첫번째 입력창 상단 라벨
	BOOL m_bIsNumber1;		//첫번째 입력창이 숫자만 받는지 여부
	CString m_strLabel2;	//두번째 입력창 상단 라벨
	BOOL m_bIsNumber2;		//두번째 입력창이 숫자만 받는지 여부
	InputItem()
	{
		m_nSubCommand = 0;
		m_bIsNumber1 = FALSE;
		m_bIsNumber2 = FALSE;
	}
	void Clear() 
	{
		m_strItemName.Empty();
		m_strLabel1.Empty();
		m_strLabel2.Empty();
		m_nSubCommand = 0;
		m_bIsNumber1 = FALSE;
		m_bIsNumber2 = FALSE;
	}
};

typedef CArray<InputItem, InputItem&> InputItemArray;

//프리셋의 단위 작업 항목을 나타내는 구조체
struct PresetTask
{
	int m_nCommand;		//기능 종류 (툴바 버튼과 연결)
	int m_nSubCommand;  //기능 종류 (입력창 콤보박스와 연결)
	CString m_str1;		//첫번째 인자
	CString m_str2;		//두번째 인자
	PresetTask() { m_nCommand = 0; m_nSubCommand = 0; }
	PresetTask(PresetTask& task) 
	{
		m_nCommand = task.m_nCommand; 
		m_nSubCommand = task.m_nSubCommand; 
		m_str1 = task.m_str1;
		m_str2 = task.m_str2;
	}
};

typedef CArray<PresetTask, PresetTask&> PresetTaskArray;

//프리셋을 나타내는 구조체
struct BatchNamerPreset
{
	CString m_strName;
	PresetTaskArray m_aTask;
	void Clear()
	{
		m_strName = L"";
		for (int i = 0; i < m_aTask.GetSize(); i++)
		{
			m_aTask.RemoveAll();
		}
	};
};


typedef CArray<BatchNamerPreset, BatchNamerPreset&> PresetArray;

CString ConvertKeyCodeToName(DWORD code);

//단축키를 나타내는 구조체
class HotKey
{
public:
	int nKeyCode;
	BOOL bCtrl;
	BOOL bShift;
	HotKey() { nKeyCode = 0; bCtrl = FALSE; bShift = FALSE;}
	HotKey(int nKeyCode_Val, BOOL bCtrl_Val, BOOL bShift_Val)
	{ 
		nKeyCode = nKeyCode_Val;
		bCtrl = bCtrl_Val;
		bShift = bShift_Val;
	}
	HotKey(HotKey& hk)
	{
		nKeyCode = hk.nKeyCode;
		bCtrl = hk.bCtrl;
		bShift = hk.bShift;
	}
	CString GetKeyString()
	{
		CString strRet;
		if (bCtrl) strRet += "Ctrl＋";
		if (bShift) strRet += "Shift＋";
		if (nKeyCode != 0) strRet += ConvertKeyCodeToName((DWORD)nKeyCode);
		return strRet;
	}
};

typedef std::map<int, HotKey> CHotKeyMap;

// CBatchNamerApp:
// 이 클래스의 구현에 대해서는 BatchNamer.cpp을(를) 참조하세요.
//

class CBatchNamerApp : public CWinAppEx
{
public:
	CBatchNamerApp();
//	ULONG_PTR m_gdiplusToken;
	HICON m_hIcon;
	BOOL m_bEnglishUI;
	CString m_strINIPath;
	BOOL m_bShowEverytime;
	BOOL m_bAutoSort;
	BOOL m_bNameAutoFix;
	int m_nLoadType;
	int m_nShowFlag;
	CRect m_rcMain;
	COLORREF m_clrText;
	COLORREF m_clrBk;
	BOOL m_bUseDefaultColor;
	int m_nFontSize;
	BOOL m_bUseDefaultFont;
	int m_nIconType;
	PresetArray m_aPreset;
	void PresetExport();
	void PresetImport();
	void INISave(CString strFile);
	void INILoad(CString strFile);
	void SetLocale(int nLanguageID);
	void UpdateThreadLocale();
	void ShowMsg(CString strMsg, CString strTitle);
	CHotKeyMap m_mapHotKey;
	void InitHotKey();
// 재정의입니다.
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
// 구현입니다.
	DECLARE_MESSAGE_MAP()
};
inline CBatchNamerApp* APP() { return (CBatchNamerApp*)AfxGetApp(); };

extern CBatchNamerApp theApp;


