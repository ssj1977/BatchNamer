
// BatchNamer.h: PROJECT_NAME 애플리케이션에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.
#include <afxwinappex.h>

// CBatchNamerApp:
// 이 클래스의 구현에 대해서는 BatchNamer.cpp을(를) 참조하세요.
//

class CBatchNamerApp : public CWinAppEx
{
public:
	CBatchNamerApp();
	CString m_strINIPath;
	BOOL m_bShowEverytime;
	int m_nLoadType;
	int m_nShowFlag;
	CRect m_rcMain;
	COLORREF m_clrText;
	COLORREF m_clrBk;
	BOOL m_bUseDefaultColor;
	int m_nFontSize;
	BOOL m_bUseDefaultFont;
	int m_nIconType;
	void INISave(CString strFile);
	void INILoad(CString strFile);

// 재정의입니다.
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
// 구현입니다.

	DECLARE_MESSAGE_MAP()
};
inline CBatchNamerApp* APP() { return (CBatchNamerApp*)AfxGetApp(); };


extern CBatchNamerApp theApp;
