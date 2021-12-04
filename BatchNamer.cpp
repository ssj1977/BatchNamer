
// BatchNamer.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "BatchNamer.h"
#include "BatchNamerDlg.h"
#include "EtcFunctions.h"
#include "CDlgMsg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBatchNamerApp

BEGIN_MESSAGE_MAP(CBatchNamerApp, CWinAppEx)
	//ON_COMMAND(ID_HELP, &CWinAppEx::OnHelp)
END_MESSAGE_MAP()


// CBatchNamerApp 생성

CBatchNamerApp::CBatchNamerApp()
{
	m_bEnglishUI = FALSE;
	m_bShowEverytime = FALSE; // 목록 읽기 방법 설정창을 매번 표시할지 여부
	m_bAutoSort = TRUE; //항목 추가시 자동 정렬 여부
	m_nLoadType = 0; //목록 읽기 방법 : 0 = 폴더를 그대로 추가 / 1 = 폴더 안의 파일을 추가
	m_nShowFlag = 0; //칼럼 표시 여부
	m_rcMain = CRect(0, 0, 0, 0);
	m_rcInput = CRect(0, 0, 0, 0);
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
	m_nIconType = SHIL_EXTRALARGE;
	m_aPreset.SetSize(5);
	m_bNameAutoFix = FALSE;
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

//	Gdiplus::GdiplusShutdown(m_gdiplusToken);

	return CWinAppEx::ExitInstance();
}

void CBatchNamerApp::UpdateThreadLocale()
{
	if (m_bEnglishUI == TRUE) SetLocale(LANG_ENGLISH);
}

BOOL CBatchNamerApp::InitInstance()
{
	TCHAR szBuff[MAX_PATH];
	GetModuleFileName(m_hInstance, szBuff, MAX_PATH);
	CString strExePath = szBuff;
	m_strINIPath = Get_Folder(strExePath) + L"\\" + Get_Name(strExePath, FALSE) + L".ini";
	InitHotKey();
	INILoad(m_strINIPath);
	m_hIcon = LoadIcon(IDR_MAINFRAME);
	if (m_bEnglishUI == TRUE) SetLocale(LANG_ENGLISH);

/*	if (!AfxOleInit())
	{
		AfxMessageBox(_T("Ole Initialization Failure"));
		return FALSE;
	}
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);*/

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
	strLine.Format(_T("EnglishUI=%d\r\n"), m_bEnglishUI); strData += strLine;
	strLine.Format(_T("ShowCFGLoad=%d\r\n"), m_bShowEverytime); strData += strLine;
	strLine.Format(_T("AutoSort=%d\r\n"), m_bAutoSort); strData += strLine;
	strLine.Format(_T("LoadType=%d\r\n"), m_nLoadType);	strData += strLine;
	strLine.Format(_T("ShowColumnFlag=%d\r\n"), m_nShowFlag);	strData += strLine;
	//strLine.Format(_T("MainX1=%d\r\n"), m_rcMain.left);		strData += strLine;
	//strLine.Format(_T("MainY1=%d\r\n"), m_rcMain.top);		strData += strLine;
	//strLine.Format(_T("MainX2=%d\r\n"), m_rcMain.right);	strData += strLine;
	//strLine.Format(_T("MainY2=%d\r\n"), m_rcMain.bottom);	strData += strLine;
	if (m_rcMain.IsRectEmpty() == FALSE)
	{
		strLine.Format(_T("RectMain=%d,%d,%d,%d\r\n"), m_rcMain.left, m_rcMain.top, m_rcMain.right, m_rcMain.bottom); 
		strData += strLine;;
	}
	if (m_rcInput.IsRectEmpty() == FALSE)
	{
		strLine.Format(_T("RectInput=%d,%d,%d,%d\r\n"), m_rcInput.left, m_rcInput.top, m_rcInput.right, m_rcInput.bottom);
		strData += strLine;
	}

	strLine.Format(_T("UseDefaultColor=%d\r\n"), m_bUseDefaultColor);	strData += strLine;
	strLine.Format(_T("ColorBk=%d\r\n"), m_clrBk);	strData += strLine;
	strLine.Format(_T("ColorText=%d\r\n"), m_clrText);	strData += strLine;
	strLine.Format(_T("UseDefaultFont=%d\r\n"), m_bUseDefaultFont);	strData += strLine;
	strLine.Format(_T("FontSize=%d\r\n"), m_nFontSize);	strData += strLine;
	strLine.Format(_T("IconType=%d\r\n"), m_nIconType);	strData += strLine;
	strLine.Format(_T("NameAutoFix=%d\r\n"), m_bNameAutoFix);	strData += strLine;
	for (int i = 0; i < m_aPreset.GetSize(); i++)
	{
		BatchNamerPreset& ps = m_aPreset[i];
		strLine.Format(_T("Preset_Name=%s\r\n"), ps.m_strName);	strData += strLine;
		for (int j = 0; j < ps.m_aTask.GetSize(); j++)
		{
			PresetTask& pt = ps.m_aTask[j];
			strLine.Format(_T("PresetTask_Command=%d\r\n"), pt.m_nCommand);	strData += strLine;
			strLine.Format(_T("PresetTask_SubCommand=%d\r\n"), pt.m_nSubCommand);	strData += strLine;
			strLine.Format(_T("PresetTask_Arg1=<%s>\r\n"), pt.m_str1);	strData += strLine;
			strLine.Format(_T("PresetTask_Arg2=<%s>\r\n"), pt.m_str2);	strData += strLine;
		}
	}
	//컬럼폭 저장
	strData += _T("ColWidths=");
	strLine.Empty();
	for (int i = 0; i < m_aColWidth.GetSize(); i++)
	{
		if (strLine.IsEmpty() == FALSE) strLine += L',';
		strLine += INTtoSTR(m_aColWidth.GetAt(i));
	}
	strData += strLine + _T("\r\n");
	// 단축키 저장
	CHotKeyMap& hkm = APP()->m_mapHotKey;
	CHotKeyMap::iterator i;
	int nPos = -1;
	for (i = hkm.begin(); i != hkm.end(); i++)
	{
		strLine.Format(_T("Hotkey=%d,%d,%d,%d\r\n"), i->first, i->second.nKeyCode, i->second.bCtrl, i->second.bShift); strData += strLine;
	}
	WriteCStringToFile(strFile, strData);
}

static CRect ConvertString2Rect(CString& str)
{
	CRect rc;
	CString strValue;
	int i = 0, nVal = 0;
	while (AfxExtractSubString(strValue, str, i, L','))
	{
		nVal = _ttoi(strValue);
		if (i == 0) rc.left = nVal;
		else if (i == 1) rc.top = nVal;
		else if (i == 2) rc.right = nVal;
		else if (i == 3) rc.bottom = nVal;
		i++;
	}
	return rc;
}

void CBatchNamerApp::INILoad(CString strFile)
{
	CString strData, strLine, str1, str2, strTemp;
	ReadFileToCString(strFile, strData);
	int nPos = 0;
	int nPreset = -1 ;
	int nTask = -1;
	while (nPos != -1)
	{
		nPos = GetLine(strData, nPos, strLine, _T("\r\n"));
		GetToken(strLine, str1, str2, _T('='), FALSE);
		if (str2.IsEmpty() == FALSE)
		{
			if (str1.CompareNoCase(_T("ShowCFGLoad")) == 0) m_bShowEverytime = CString2BOOL(str2);
			else if (str1.CompareNoCase(_T("AutoSort")) == 0) m_bAutoSort = CString2BOOL(str2);
			else if (str1.CompareNoCase(_T("LoadType")) == 0) m_nLoadType = _ttoi(str2);
			else if (str1.CompareNoCase(_T("ShowColumnFlag")) == 0) m_nShowFlag = _ttoi(str2);
			//For Legacy INI
			else if (str1.CompareNoCase(_T("MainX1")) == 0) m_rcMain.left = _ttoi(str2);
			else if (str1.CompareNoCase(_T("MainY1")) == 0) m_rcMain.top = _ttoi(str2);
			else if (str1.CompareNoCase(_T("MainX2")) == 0) m_rcMain.right = _ttoi(str2);
			else if (str1.CompareNoCase(_T("MainY2")) == 0) m_rcMain.bottom = _ttoi(str2);
			//New INI, all 4 values are in a single line
			else if (str1.CompareNoCase(_T("RectMain")) == 0) m_rcMain = ConvertString2Rect(str2);
			else if (str1.CompareNoCase(_T("RectInput")) == 0) m_rcInput = ConvertString2Rect(str2);
			else if (str1.CompareNoCase(_T("UseDefaultColor")) == 0) m_bUseDefaultColor = CString2BOOL(str2);
			else if (str1.CompareNoCase(_T("ColorBk")) == 0) m_clrBk = _ttoi(str2);
			else if (str1.CompareNoCase(_T("ColorText")) == 0) m_clrText = _ttoi(str2);
			else if (str1.CompareNoCase(_T("UseDefaultFont")) == 0) m_bUseDefaultFont = CString2BOOL(str2);
			else if (str1.CompareNoCase(_T("FontSize")) == 0) m_nFontSize = _ttoi(str2);
			else if (str1.CompareNoCase(_T("IconType")) == 0) m_nIconType = _ttoi(str2);
			else if (str1.CompareNoCase(_T("NameAutoFix")) == 0) m_bNameAutoFix = CString2BOOL(str2);
			else if (str1.CompareNoCase(_T("EnglishUI")) == 0) m_bEnglishUI = CString2BOOL(str2);
			else if (str1.CompareNoCase(_T("ColWidths")) == 0)
			{
				CString strWidth;
				m_aColWidth.RemoveAll();
				int i = 0;
				while (AfxExtractSubString(strWidth, str2, i, L','))
				{
					i++;
					m_aColWidth.Add(_ttoi(strWidth));
				}
			}
			else if (str1.CompareNoCase(_T("Hotkey")) == 0)
			{
				CString strCmd, strKey, strCtrl, strShift;
				AfxExtractSubString(strCmd, str2, 0, L',');
				AfxExtractSubString(strKey, str2, 1, L',');
				AfxExtractSubString(strCtrl, str2, 2, L',');
				AfxExtractSubString(strShift, str2, 3, L',');
				int nCommand = _ttoi(str2);
				CHotKeyMap& hkm = APP()->m_mapHotKey;
				CHotKeyMap::iterator i;
				i = hkm.find(nCommand);
				if (i != hkm.end())
				{
					i->second.nKeyCode = _ttoi(strKey);
					i->second.bCtrl = CString2BOOL(strCtrl);
					i->second.bShift = CString2BOOL(strShift);
				}
			}
		}
		//이 부분은 str2가 비어 있더라도 받는다
		if (str1.CompareNoCase(_T("Preset_Name")) == 0)
		{
			nTask = -1;
			nPreset++;
			if (nPreset >= 0 && nPreset < m_aPreset.GetSize()) m_aPreset[nPreset].m_strName = str2;
		}
		else if (str1.CompareNoCase(_T("PresetTask_Command")) == 0)
		{
			PresetTask pt;
			nTask++;
			m_aPreset[nPreset].m_aTask.Add(pt);
			if (nPreset >= 0 && nPreset < m_aPreset.GetSize())
				if (nTask >= 0 && nTask < m_aPreset[nPreset].m_aTask.GetSize())
					m_aPreset[nPreset].m_aTask[nTask].m_nCommand = _ttoi(str2);
		}
		else if (str1.CompareNoCase(_T("PresetTask_SubCommand")) == 0)
		{
			if (nPreset >= 0 && nPreset < m_aPreset.GetSize())
				if (nTask >= 0 && nTask < m_aPreset[nPreset].m_aTask.GetSize())
					m_aPreset[nPreset].m_aTask[nTask].m_nSubCommand = _ttoi(str2);
		}
		else if (str1.CompareNoCase(_T("PresetTask_Arg1")) == 0)
		{
			if (nPreset >= 0 && nPreset < m_aPreset.GetSize())
				if (nTask >= 0 && nTask < m_aPreset[nPreset].m_aTask.GetSize())
					if (str2.GetLength() >= 2 && str2.GetAt(0) == _T('<'))
					{
						str2.Delete(str2.GetLength() - 1, 1);
						str2.Delete(0, 1);
					}
					m_aPreset[nPreset].m_aTask[nTask].m_str1 = str2;
		}
		else if (str1.CompareNoCase(_T("PresetTask_Arg2")) == 0)
		{
			if (nPreset >= 0 && nPreset < m_aPreset.GetSize())
				if (nTask >= 0 && nTask < m_aPreset[nPreset].m_aTask.GetSize())
					if (str2.GetLength() >= 2 && str2.GetAt(0) == _T('<'))
					{
						str2.Delete(str2.GetLength() - 1, 1);
						str2.Delete(0, 1);
					}
					m_aPreset[nPreset].m_aTask[nTask].m_str2 = str2;
		}
	}
}


void CBatchNamerApp::SetLocale(int nLanguageID)
{
	//LANG_KOREAN / LANG_ENGLISH
	SetThreadLocaleEx(MAKELCID(MAKELANGID(nLanguageID, SUBLANG_DEFAULT), SORT_DEFAULT));
}

void CBatchNamerApp::ShowMsg(CString strMsg, CString strTitle)
{
	CDlgMsg dlg;
	dlg.m_strTitle = strTitle;
	dlg.m_strMsg = strMsg;
	dlg.DoModal();
}

void CBatchNamerApp::InitHotKey()
{
	int aCommand[] = {	IDM_PRESET_APPLY1, IDM_PRESET_APPLY2, IDM_PRESET_APPLY3, IDM_PRESET_APPLY4, IDM_PRESET_APPLY5, 
						IDM_PRESET_EDIT, IDM_PRESET_EXPORT, IDM_PRESET_IMPORT,
						IDM_EDIT_UP, IDM_EDIT_DOWN, IDM_APPLY_CHANGE, IDM_UNDO_CHANGE, IDM_UNDO_SELECTED,
						IDM_LIST_ADD, IDM_CLEAR_LIST, IDM_SORT_LIST, IDM_REMOVE_ITEM,
						IDM_EXPORT_CLIP_NEWNAME, IDM_IMPORT_CLIP_NEWNAME, IDM_EXPORT_FILE_NEWNAME, IDM_IMPORT_FILE_NEWNAME, 
						IDM_EXPORT_CLIP_PATH, IDM_IMPORT_CLIP_PATH,IDM_EXPORT_FILE_PATH, IDM_IMPORT_FILE_PATH,
						IDM_CFG_LOAD, IDM_CFG_VIEW, IDM_CFG_ETC, IDM_VERSION,
						IDM_NAME_REPLACE, IDM_NAME_ADD_FRONT, IDM_NAME_ADD_REAR, IDM_NAME_EMPTY, IDM_NAME_REMOVESELECTED, 
						IDM_NAME_EXTRACTNUMBER, IDM_NAME_REMOVENUMBER, IDM_NAME_DIGIT, IDM_NAME_ADDNUM, IDM_NAME_SETFOLDER,
						IDM_EXT_DEL, IDM_EXT_ADD, IDM_EXT_REPLACE
		};
	int aKeyCode[] = {	VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, 
						VK_F11, VK_F7, VK_F8,
						188, 190, 0x53, 0x5A, 0x5A,
						0x4F, 0x4C, 0x52, VK_DELETE,
						0x43, 0x56, 0x44, 0x46,
						0x43, 0x56, 0x44, 0x46,
						0, 0, 0, 0,
						0x31, 0x32, 0x33, 0x34, 0x35,
						0x36, 0x37, 0x38, 0x39, 0x30,
						0x31, 0x32, 0x33
	};
	BOOL aCtrl[] = {	FALSE, FALSE, FALSE, FALSE, FALSE, 
						FALSE, FALSE, FALSE,
						FALSE, FALSE, TRUE, TRUE, TRUE,
						TRUE, TRUE, TRUE, FALSE,
						TRUE, TRUE, TRUE, TRUE,	
						TRUE, TRUE, TRUE, TRUE,
						FALSE, FALSE, FALSE, FALSE,
						TRUE, TRUE, TRUE, TRUE, TRUE,
						TRUE, TRUE, TRUE, TRUE, TRUE,
						FALSE, FALSE, FALSE
	};
	BOOL aShift[] = {	FALSE, FALSE, FALSE, FALSE, FALSE, 
						FALSE, FALSE, FALSE,
						FALSE, FALSE, FALSE, FALSE, TRUE,
						FALSE, FALSE, FALSE, FALSE,
						FALSE, FALSE, FALSE, FALSE,
						TRUE, TRUE, TRUE, TRUE,
						FALSE, FALSE, FALSE, FALSE,
						FALSE, FALSE, FALSE, FALSE, FALSE, 
						FALSE, FALSE, FALSE, FALSE, FALSE,
						TRUE, TRUE, TRUE
	};
	int nCount = sizeof(aShift) / sizeof(BOOL);
	HotKey hk;
	m_mapHotKey.clear();
	for(int i = 0; i < nCount; i++)
	{
		hk.nKeyCode = aKeyCode[i];
		hk.bCtrl = aCtrl[i];
		hk.bShift = aShift[i];
		m_mapHotKey.insert(CHotKeyMap::value_type(aCommand[i], hk));
	}
}


void CBatchNamerApp::PresetExport()
{
	CFileDialog dlg(FALSE, _T("bnp"), NULL, OFN_ENABLESIZING | OFN_LONGNAMES | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, _T("BatchNamer Preset(*.bnp)|*.bnp|All Files(*.*)|*.*||"), NULL);
	CString strTitle;
	strTitle.LoadString(IDS_PRESET_EXPORT);
	dlg.GetOFN().lpstrTitle = strTitle;
	if (dlg.DoModal() == IDCANCEL) return;
	CString strFile = dlg.GetPathName();

	CString strData, strLine, str1, str2;
	for (int i = 0; i < m_aPreset.GetSize(); i++)
	{
		BatchNamerPreset& ps = m_aPreset[i];
		strLine.Format(_T("Preset_Name=%s\r\n"), ps.m_strName);	strData += strLine;
		for (int j = 0; j < ps.m_aTask.GetSize(); j++)
		{
			PresetTask& pt = ps.m_aTask[j];
			strLine.Format(_T("PresetTask_Command=%d\r\n"), pt.m_nCommand);	strData += strLine;
			strLine.Format(_T("PresetTask_SubCommand=%d\r\n"), pt.m_nSubCommand);	strData += strLine;
			strLine.Format(_T("PresetTask_Arg1=<%s>\r\n"), pt.m_str1);	strData += strLine;
			strLine.Format(_T("PresetTask_Arg2=<%s>\r\n"), pt.m_str2);	strData += strLine;
		}
	}
	WriteCStringToFile(strFile, strData);
}

void CBatchNamerApp::PresetImport()
{
	CFileDialog dlg(TRUE, _T("bnp"), NULL, OFN_ENABLESIZING | OFN_LONGNAMES | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, _T("BatchNamer Preset(*.bnp)|*.bnp|All Files(*.*)|*.*||"), NULL);
	CString strTitle;
	strTitle.LoadString(IDS_PRESET_IMPORT);
	dlg.GetOFN().lpstrTitle = strTitle;
	if (dlg.DoModal() == IDCANCEL) return;
	CString strFile = dlg.GetPathName();
	m_aPreset.RemoveAll();
	m_aPreset.SetSize(5);
	INILoad(strFile);
}