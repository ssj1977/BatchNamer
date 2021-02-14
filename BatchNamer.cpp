
// BatchNamer.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "BatchNamer.h"
#include "BatchNamerDlg.h"
#include "EtcFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBatchNamerApp

BEGIN_MESSAGE_MAP(CBatchNamerApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinAppEx::OnHelp)
END_MESSAGE_MAP()


// CBatchNamerApp 생성

CBatchNamerApp::CBatchNamerApp()
{
	m_bShowEverytime = TRUE; // 목록 읽기 방법 설정창을 매번 표시할지 여부
	m_nLoadType = 0; //목록 읽기 방법 : 0 = 폴더를 그대로 추가 / 1 = 폴더 안의 파일을 추가
	m_nShowFlag = 0; //칼럼 표시 여부
	m_rcMain = CRect(0, 0, 0, 0);
	FlagSET(m_nShowFlag, COL_OLDNAME, TRUE);
	FlagSET(m_nShowFlag, COL_NEWNAME, TRUE);
	FlagSET(m_nShowFlag, COL_OLDFOLDER, TRUE);
	FlagSET(m_nShowFlag, COL_NEWFOLDER, TRUE);
	FlagSET(m_nShowFlag, COL_TIMECREATE, TRUE);
	FlagSET(m_nShowFlag, COL_TIMEMODIFY, TRUE);
	FlagSET(m_nShowFlag, COL_FILESIZE, TRUE);
	FlagSET(m_nShowFlag, COL_FULLPATH, TRUE);
	m_clrText = RGB(0, 0, 0);
	m_clrBk = RGB(255, 255, 255);
	m_bUseDefaultColor = TRUE;
	m_nFontSize = 12;
	m_bUseDefaultFont = TRUE;
	m_nIconType = SHIL_SMALL;
}


// 유일한 CBatchNamerApp 개체입니다.
CBatchNamerApp theApp;

//로케일 설정용 함수
bool SetThreadLocaleEx(LCID lcLocale)
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 6;
	osvi.dwMinorVersion = 2;
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
	if (VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask))
	{
		// >= Windows Vista
		HINSTANCE hKernelDll = ::LoadLibrary(_T("kernel32.dll"));
		if (hKernelDll == NULL) return false;
		unsigned (WINAPI * SetThreadUILanguage)(LANGID) = (unsigned (WINAPI*)(LANGID))::GetProcAddress(hKernelDll, "SetThreadUILanguage");
		if (SetThreadUILanguage == NULL) return false;
		LANGID resLangID = SetThreadUILanguage(static_cast<LANGID>(lcLocale));
		::FreeLibrary(hKernelDll);
		return (resLangID == LOWORD(lcLocale));
	}
	else // <= Windows XP
	{
		return ::SetThreadLocale(lcLocale) ? true : false;
	}
}


// CBatchNamerApp 초기화
#include <afxvisualmanager.h>
int CBatchNamerApp::ExitInstance()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	INISave(m_strINIPath);
	if (CMFCVisualManager::GetInstance() != NULL)
		delete CMFCVisualManager::GetInstance();
	return CWinAppEx::ExitInstance();
}

BOOL CBatchNamerApp::InitInstance()
{
	// 애플리케이션 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다.
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// 대화 상자에 셸 트리 뷰 또는
	// 셸 목록 뷰 컨트롤이 포함되어 있는 경우 셸 관리자를 만듭니다.
	//CShellManager *pShellManager = new CShellManager;
	// MFC 컨트롤의 테마를 사용하기 위해 "Windows 원형" 비주얼 관리자 활성화
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	TCHAR szBuff[MAX_PATH];
	GetModuleFileName(m_hInstance, szBuff, MAX_PATH);
	CString strExePath = szBuff;
	m_strINIPath = Get_Folder(strExePath) + L"\\" + Get_Name(strExePath, FALSE) + L".ini";
	INILoad(m_strINIPath);

	CBatchNamerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{

	}
	else if (nResponse == IDCANCEL)
	{
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "경고: 대화 상자를 만들지 못했으므로 애플리케이션이 예기치 않게 종료됩니다.\n");
		TRACE(traceAppMsg, 0, "경고: 대화 상자에서 MFC 컨트롤을 사용하는 경우 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS를 수행할 수 없습니다.\n");
	}
	/*// 위에서 만든 셸 관리자를 삭제합니다.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}*/

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void CBatchNamerApp::INISave(CString strFile)
{
	CString strData, strLine, str1, str2;
	strLine.Format(_T("ShowCFGLoad=%d\r\n"), m_bShowEverytime); strData += strLine;
	strLine.Format(_T("LoadType=%d\r\n"), m_nLoadType);	strData += strLine;
	strLine.Format(_T("ShowColumnFlag=%d\r\n"), m_nShowFlag);	strData += strLine;
	strLine.Format(_T("MainX1=%d\r\n"), m_rcMain.left);		strData += strLine;
	strLine.Format(_T("MainY1=%d\r\n"), m_rcMain.top);		strData += strLine;
	strLine.Format(_T("MainX2=%d\r\n"), m_rcMain.right);	strData += strLine;
	strLine.Format(_T("MainY2=%d\r\n"), m_rcMain.bottom);	strData += strLine;
	strLine.Format(_T("UseDefaultColor=%d\r\n"), m_bUseDefaultColor);	strData += strLine;
	strLine.Format(_T("ColorBk=%d\r\n"), m_clrBk);	strData += strLine;
	strLine.Format(_T("ColorText=%d\r\n"), m_clrText);	strData += strLine;
	strLine.Format(_T("UseDefaultFont=%d\r\n"), m_bUseDefaultFont);	strData += strLine;
	strLine.Format(_T("FontSize=%d\r\n"), m_nFontSize);	strData += strLine;
	strLine.Format(_T("IconType=%d\r\n"), m_nIconType);	strData += strLine;
	WriteCStringToFile(strFile, strData);
}


void CBatchNamerApp::INILoad(CString strFile)
{
	CString strData, strLine, str1, str2, strTemp;
	ReadFileToCString(strFile, strData);
	int nPos = 0;
	while (nPos != -1)
	{
		nPos = GetLine(strData, nPos, strLine, _T("\r\n"));
		GetToken(strLine, str1, str2, _T('='), FALSE);
		if (str2.IsEmpty() == FALSE)
		{
			if (str1.CompareNoCase(_T("ShowCFGLoad")) == 0) m_bShowEverytime = _ttoi(str2);
			else if (str1.CompareNoCase(_T("LoadType")) == 0) m_nLoadType = _ttoi(str2);
			else if (str1.CompareNoCase(_T("ShowColumnFlag")) == 0) m_nShowFlag = _ttoi(str2);
			else if (str1.CompareNoCase(_T("MainX1")) == 0) m_rcMain.left = _ttoi(str2);
			else if (str1.CompareNoCase(_T("MainY1")) == 0) m_rcMain.top = _ttoi(str2);
			else if (str1.CompareNoCase(_T("MainX2")) == 0) m_rcMain.right = _ttoi(str2);
			else if (str1.CompareNoCase(_T("MainY2")) == 0) m_rcMain.bottom = _ttoi(str2);
			else if (str1.CompareNoCase(_T("UseDefaultColor")) == 0) m_bUseDefaultColor = _ttoi(str2);
			else if (str1.CompareNoCase(_T("ColorBk")) == 0) m_clrBk = _ttoi(str2);
			else if (str1.CompareNoCase(_T("ColorText")) == 0) m_clrText = _ttoi(str2);
			else if (str1.CompareNoCase(_T("UseDefaultFont")) == 0) m_bUseDefaultFont = _ttoi(str2);
			else if (str1.CompareNoCase(_T("FontSize")) == 0) m_nFontSize = _ttoi(str2);
			else if (str1.CompareNoCase(_T("IconType")) == 0) m_nIconType = _ttoi(str2);
		}
	}
}
