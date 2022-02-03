
// BatchNamerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "BatchNamer.h"
#include "BatchNamerDlg.h"
#include <afxdialogex.h>
#include <CommonControls.h>
#include <shlwapi.h>
#include <atlpath.h>
#include <time.h>
#include <shellapi.h>
#include "EtcFunctions.h"
#include "CDlgCFG_Load.h"
#include "CDlgCFG_View.h"
#include "CDlgCFG_Etc.h"
#include "CDlgInput.h"
#include "CDlgSort.h"
#include "CDlgPreset.h"
#include "CDlgApplyOption.h"
#include "CDlgListFilter.h"
#pragma warning(disable:4786)
//#include <map>
#include <vector>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//Actions of ProcessDropFiles()
#define DF_ADDLIST 0
#define DF_READNEWNAME 1
#define DF_READFULLPATH 2

typedef vector<CString> CStrArray;
typedef map<CString, int> CExtMap; //확장자에 해당하는 이미지맵의 번호를 기억
typedef map<CString, int> CFolderMap; //폴더별 카운트용
static CExtMap mapExt;
static BOOL st_bIsIdle;
CString ReplaceWithWildCards(CString strSrc, CString str1, CString str2, BOOL bReturnBlockOnly);

inline CString GetFolderName(CString strPath)
{
	return strPath.Right(strPath.GetLength() - strPath.ReverseFind(_T('\\')) - 1);
}

inline CString GetTimeStringToShow(FILETIME& dt)
{
	CString strRet;
	SYSTEMTIME systime;
	FileTimeToSystemTime(&dt, &systime);
	strRet.Format(_T("%d-%02d-%02d %02d:%02d:%02d"), systime.wYear,
		systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
	return strRet;
}

inline CString GetTimeStringToAdd(CString strDateTime, BOOL bAddDate, BOOL bAddTime)
{
	if (strDateTime.IsEmpty()) return strDateTime;
	strDateTime.Replace(_T(' '), _T('_'));
	strDateTime.Remove(_T(':'));
	if (bAddDate == TRUE && bAddTime == TRUE) return strDateTime;
	if (bAddDate == TRUE && bAddTime == FALSE)
	{
		return strDateTime.Left(strDateTime.ReverseFind(_T('_')));
	}
	else if (bAddDate == FALSE && bAddTime == TRUE)
	{
		return strDateTime.Mid(strDateTime.ReverseFind(_T('_')) + 1);
	}
	return _T("");
}

inline CString FormatTimeString(CString strDateTime, CString strFormat)
{
	COleDateTime dt;
	dt.ParseDateTime(strDateTime);
	CString strTemp;
	TCHAR c;
	//COleDateTime::Format 은 잘못된 인자가 들어가는 경우 런타임 에러 발생
	//사용자 입력값에서 에러가 발생하지 않도록 포맷 문자열 사전 점검 및 교정
	int nLen = strFormat.GetLength();
	for (int i=0; i<nLen; i++)
	{
		c = strFormat.GetAt(i);
		if (c == L'%' && i < nLen - 1)
		{ //%가 있는 경우 바로 뒷 글자를 확인
			i++;
			c = strFormat.GetAt(i);
			switch (c)
			{ //정상적인 포맷은 유지한다
			case L'a': case L'A': case L'b': case L'B':
			case L'c': case L'C': case L'd': case L'D':
			case L'e': case L'F': case L'g': case L'G':
			case L'h': case L'H': case L'I': case L'j':
			case L'm': case L'M': case L'n': case L'p':
			case L'r': case L'R': case L'S': case L'T':
			case L'u': case L'U': case L'V': case L'w':
			case L'W': case L'x': case L'X': case L'y':
			case L'Y': case L'z': case L'Z': case L'%':
				strTemp = strTemp + L'%' + c;
				break;
			default: //잘못된 포맷은 뺀다
				break;
			}
		}
		else strTemp += c; //보통 문자는 유지
	}
	return dt.Format(strTemp);
}


int GetFileImageIndex(CString strPath)
{
	SHFILEINFO sfi;
	memset(&sfi, 0x00, sizeof(sfi));
	SHGetFileInfo((LPCTSTR)strPath, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
	return sfi.iIcon;
}
int GetFileImageIndexFromMap(CString strPath, BOOL bIsDirectory)
{
	if (bIsDirectory)
	{
		//return GetFileImageIndex(_T(""));
		return 3;		// SI_FOLDER_CLOSE
	}
	CPath path = CPath(strPath);
	CString strExt = path.GetExtension();
	if (strExt.CompareNoCase(_T(".exe")) == 0
		|| strExt.CompareNoCase(_T(".ico")) == 0
		|| strExt.CompareNoCase(_T(".lnk")) == 0
		) return GetFileImageIndex(strPath);
	CExtMap::iterator it = mapExt.find(strExt);
	if (it == mapExt.end())
	{
		int nImage = GetFileImageIndex(strPath);
		mapExt.insert(CExtMap::value_type(strExt, nImage));
		return nImage;
	}
	return (*it).second;
}

BOOL FindBracketPart(CString& strSrc, TCHAR c1, TCHAR c2, int& nStart, int& nEnd);


// CBatchNamerDlg 대화 상자



CBatchNamerDlg::CBatchNamerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BATCHNAMER, pParent)
{
	m_bSelected = FALSE; //목록 중 항목 선택 여부 감지
	m_nDefault_FontSize = 12;
	m_clrDefault_Bk = RGB(255, 255, 255);
	m_clrDefault_Text = RGB(0, 0, 0);
	m_lfHeight = 0;
	m_pSysImgList = NULL;
	st_bIsIdle = TRUE;
	m_nTempLoadType = -1;
}

void CBatchNamerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBatchNamerDlg)
	DDX_Control(pDX, IDC_LIST_FILE, m_list);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBatchNamerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_DROPFILES()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILE, OnDblclkListFile)
	ON_BN_CLICKED(IDC_BTN_STOPTHREAD, &CBatchNamerDlg::OnBnClickedBtnStopthread)
	ON_WM_GETMINMAXINFO()
	ON_WM_INITMENU()
END_MESSAGE_MAP()


// CBatchNamerDlg 메시지 처리기

BOOL CBatchNamerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(APP()->m_hIcon, TRUE);		// Set big icon
	SetIcon(APP()->m_hIcon, FALSE);		// Set small icon

	UpdateImageList();
	DragAcceptFiles(TRUE);
	// Windows 기본 UI 설정값을 저장한다.
	LOGFONT lf;
	m_list.GetFont()->GetLogFont(&lf);
	m_nDefault_FontSize = MulDiv(-1 * lf.lfHeight, 72, GetDeviceCaps(GetDC()->GetSafeHdc(), LOGPIXELSY));
	m_clrDefault_Bk = m_list.GetBkColor();
	m_clrDefault_Text = m_list.GetTextColor();
	m_lfHeight = abs(lf.lfHeight);

	m_tool1.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_LIST, WS_CHILD | WS_VISIBLE);
	m_tool1.LoadToolBar(IDR_TOOLBAR1);
	m_tool2.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_LIST, WS_CHILD | WS_VISIBLE);
	m_tool2.LoadToolBar(IDR_TOOLBAR2);

	UINT nStyle;
	int nCount = m_tool1.GetCount();
	int nTextIndex = 0;
	for (int i = 0; i < nCount; i++)
	{
		nStyle = m_tool1.GetButtonStyle(i);
		if (!(nStyle & TBBS_SEPARATOR))
		{
			m_tool1.SetButtonText(i, IDSTR(IDS_TB_00 + nTextIndex));
			nTextIndex += 1;
		}
	}
	nCount = m_tool2.GetCount();
	nTextIndex = 0;
	for (int i = 0; i < nCount; i++)
	{
		nStyle = m_tool2.GetButtonStyle(i);
		if (!(nStyle & TBBS_SEPARATOR))
		{
			m_tool2.SetButtonText(i, IDSTR(IDS_TB_10 + nTextIndex));
			nTextIndex += 1;
		}
	}
	if (APP()->m_bUseDefaultColor == FALSE)
	{
		m_list.SetBkColor(APP()->m_clrBk);
		m_list.SetTextColor(APP()->m_clrText);
	}
	if (APP()->m_bUseDefaultFont == FALSE) UpdateFontSize();
	int nIconWidth = 0;
	switch (APP()->m_nIconType)
	{
	case SHIL_SMALL: nIconWidth = 16; break;
	case SHIL_LARGE: nIconWidth = 32; break;
	case SHIL_EXTRALARGE: nIconWidth = 48; break;
	case SHIL_JUMBO: nIconWidth = 256; break;
	}
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(COL_OLDNAME, IDSTR(IDS_COL_OLDNAME)); //_T("현재이름")
	m_list.InsertColumn(COL_NEWNAME, IDSTR(IDS_COL_NEWNAME)); //_T("바뀔이름"));
	m_list.InsertColumn(COL_OLDFOLDER, IDSTR(IDS_COL_OLDFOLDER)); //_T("현재폴더"));
	m_list.InsertColumn(COL_NEWFOLDER, IDSTR(IDS_COL_NEWFOLDER)); //_T("바뀔폴더")); 
	m_list.InsertColumn(COL_FILESIZE, IDSTR(IDS_COL_FILESIZE), LVCFMT_RIGHT); //_T("파일크기"), LVCFMT_RIGHT); 
	m_list.InsertColumn(COL_TIMEMODIFY, IDSTR(IDS_COL_TIMEMODIFY)); //_T("변경시각")); 
	m_list.InsertColumn(COL_TIMECREATE, IDSTR(IDS_COL_TIMECREATE)); //_T("생성시각")); 
	m_list.InsertColumn(COL_FULLPATH, IDSTR(IDS_COL_FULLPATH)); //_T("전체경로")); 
	int HU = m_lfHeight;
	if (APP()->m_aColWidth.GetSize() != COL_TOTAL)
	{
		m_list.SetColumnWidth(COL_OLDNAME, nIconWidth + HU * 16 * FlagGET(APP()->m_nShowFlag, COL_OLDNAME));
		m_list.SetColumnWidth(COL_NEWNAME, HU * 16 * FlagGET(APP()->m_nShowFlag, COL_NEWNAME));
		m_list.SetColumnWidth(COL_OLDFOLDER, HU * 11 * FlagGET(APP()->m_nShowFlag, COL_OLDFOLDER));
		m_list.SetColumnWidth(COL_NEWFOLDER, HU * 11 * FlagGET(APP()->m_nShowFlag, COL_NEWFOLDER));
		m_list.SetColumnWidth(COL_FILESIZE, HU * 11 * FlagGET(APP()->m_nShowFlag, COL_FILESIZE));
		m_list.SetColumnWidth(COL_TIMEMODIFY, HU * 11 * FlagGET(APP()->m_nShowFlag, COL_TIMEMODIFY));
		m_list.SetColumnWidth(COL_TIMECREATE, HU * 11 * FlagGET(APP()->m_nShowFlag, COL_TIMECREATE));
		m_list.SetColumnWidth(COL_FULLPATH, HU * 11 * FlagGET(APP()->m_nShowFlag, COL_FULLPATH));
	}
	else
	{
		for (int i = 0; i < COL_TOTAL; i++)
		{
			m_list.SetColumnWidth(i, APP()->m_aColWidth.GetAt(i) * FlagGET(APP()->m_nShowFlag, i));
		}
	}
	m_list.GetHeaderCtrl().SetSortColumn(APP()->m_nSortCol, APP()->m_bSortAscend);
	UpdateColumnSizes();
	UpdateMenuPreset();
	UpdateMenuHotkey();
	UpdateCount();

	if (APP()->m_rcMain.IsRectEmpty() == FALSE)
	{
		CRect rcScreen;
		::GetWindowRect(::GetDesktopWindow(), &rcScreen);
		APP()->m_rcMain.NormalizeRect();
		CRect rcVisible;
		rcVisible.IntersectRect(APP()->m_rcMain, rcScreen);
		if (rcVisible.Width() > 200 && rcVisible.Height() > 100)
		{
			MoveWindow(APP()->m_rcMain, TRUE);
		}
	}
	ArrangeCtrl();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBatchNamerDlg::UpdateImageList()
{
	HRESULT hr = SHGetImageList(APP()->m_nIconType, IID_IImageList, (void**)&m_pSysImgList);
	if (m_pSysImgList && SUCCEEDED(hr))
	{
		ListView_SetImageList(m_list.GetSafeHwnd(), m_pSysImgList, LVSIL_SMALL);
		//Header_SetImageList(m_list.GetHeaderCtrl().GetSafeHwnd(), m_pSysImgList);
	}
}

void CBatchNamerDlg::OnOK()
{
	//CDialogEx::OnOK();
}

void CBatchNamerDlg::OnCancel()
{
	if (st_bIsIdle == FALSE)
	{
		if (AfxMessageBox(IDSTR(IDS_MSG_STOPTHREAD), MB_YESNO) == IDNO) return;
		st_bIsIdle = TRUE;
	}
	if (m_list.GetItemCount() > 0)
	{
		if (AfxMessageBox(IDSTR(IDS_CONFIRM_EXIT), MB_YESNO) == IDNO) return;
	}

	ShowWindow(SW_SHOWNORMAL);
	GetWindowRect(APP()->m_rcMain);
	UpdateColumnSizes();
	ClearList(TRUE);
	mapExt.clear();
	CDialogEx::OnCancel();
}

void CBatchNamerDlg::ArrangeCtrl()
{
	CRect rcButton, rcSplit;
	m_tool1.GetToolBarCtrl().GetItemRect(0, rcButton);
	m_tool1.GetToolBarCtrl().GetItemRect(1, rcSplit);
	m_tool2.GetToolBarCtrl().SetButtonWidth(rcButton.Width(), rcButton.Width() * 2);
	int TOOLHEIGHT = (rcButton.Height() + rcSplit.Width()) * 10;
	int TOOLWIDTH = rcButton.Width() + rcSplit.Width();
	int BARHEIGHT = m_lfHeight * 2;
	CRect rc;
	GetClientRect(rc);

	GetDlgItem(IDC_TOOLBORDER_1)->MoveWindow(0, 0, TOOLWIDTH, 4);
	m_tool1.MoveWindow(0, 4, TOOLWIDTH, TOOLHEIGHT);
	if (rc.Height() > (TOOLHEIGHT + 4) * 2 + BARHEIGHT)
	{
		GetDlgItem(IDC_TOOLBORDER_2)->MoveWindow(0, TOOLHEIGHT, TOOLWIDTH, 4);
		m_tool2.MoveWindow(0, TOOLHEIGHT + 4, TOOLWIDTH, TOOLHEIGHT);
		m_list.MoveWindow(TOOLWIDTH, 0, rc.Width() - TOOLWIDTH, rc.Height() - BARHEIGHT);
	}
	else
	{
		GetDlgItem(IDC_TOOLBORDER_2)->MoveWindow(rc.right - TOOLWIDTH, 0, TOOLWIDTH, 4);
		m_tool2.MoveWindow(rc.right - TOOLWIDTH, 4, TOOLWIDTH, TOOLHEIGHT);
		m_list.MoveWindow(TOOLWIDTH, 0, rc.Width() - TOOLWIDTH * 2, rc.Height() - BARHEIGHT);
	}
	GetDlgItem(IDC_BTN_STOPTHREAD)->ShowWindow(st_bIsIdle ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_BTN_STOPTHREAD)->EnableWindow(!st_bIsIdle);
	GetDlgItem(IDC_BTN_STOPTHREAD)->MoveWindow(rc.right - TOOLWIDTH, rc.bottom - BARHEIGHT + 1, TOOLWIDTH, BARHEIGHT - 2);
	int BARWIDTH = rc.Width() - (st_bIsIdle ? 0 : TOOLWIDTH);
	GetDlgItem(IDC_ST_BAR)->MoveWindow(0, rc.bottom - BARHEIGHT + 1, BARWIDTH, BARHEIGHT - 2);
	//m_tool1.Invalidate();
	//m_tool2.Invalidate();
	//m_list.Invalidate();
	Invalidate();
	RedrawWindow();
}

void CBatchNamerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (::IsWindow(m_list.GetSafeHwnd()) != FALSE) ArrangeCtrl();
}

BOOL CBatchNamerDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (st_bIsIdle == FALSE)
	{
		return CDialogEx::OnCommand(wParam, lParam);
	}
	CMenu* pMenu = GetMenu();
	UINT state = pMenu->GetMenuState((UINT)wParam, MF_BYCOMMAND);
	if ((state & MF_GRAYED) != 0) 
		return TRUE;
	switch (wParam)
	{
	case IDM_CLEAR_LIST:		ClearList(((GetKeyState(VK_SHIFT) & 0xFF00) != 0) ? TRUE : FALSE);		break;
	case IDM_CLEAR_LIST_ALL:	ClearList(TRUE);		break;
	case IDM_UNDO_SELECTED:		UndoChanges(TRUE);	break;
	case IDM_UNDO_CHANGE:		UndoChanges(((GetKeyState(VK_SHIFT) & 0xFF00) != 0) ? TRUE : FALSE);;	break;
	case IDM_SORT_LIST:			SortList();		break;
	case IDM_NAME_REPLACE:		NameReplace();	break;
	case IDM_NAME_ADD_FRONT:	NameAdd(TRUE);	break;
	case IDM_NAME_ADD_REAR:		NameAdd(FALSE);	break;
	case IDM_NAME_REMOVESELECTED:	NameRemoveSelected();		break;
	case IDM_NAME_EXTRACTNUMBER:	NameNumberFilter(FALSE); break;
	case IDM_NAME_REMOVENUMBER:		NameNumberFilter(TRUE);	break;
	case IDM_NAME_DIGIT:		NameDigit();		break;
	case IDM_EXT_ADD:			ExtAdd();			break;
	case IDM_EXT_DEL:			ExtDel();			break;
	case IDM_EXT_REPLACE:		ExtReplace();		break;
	case IDM_MANUAL_CHANGE:		ManualChange();		break;
	case IDM_APPLY_CHANGE:		ApplyChange_Start();	break;
	case IDM_NAME_ADDNUM:		NameAddNum();		break;
	case IDM_NAME_EMPTY:		NameEmpty();		break;
	case IDM_EDIT_UP:			ListUp();			break;
	case IDM_EDIT_DOWN:			ListDown();		break;
	case IDM_NAME_SETFOLDER:	NameSetFolder();	break;
	case IDM_EXPORT_CLIP_NEWNAME:		Export(0);		break;
	case IDM_IMPORT_CLIP_NEWNAME:		ImportNewName(FALSE);		break;
	case IDM_EXPORT_FILE_NEWNAME:		Export(1);		break;
	case IDM_IMPORT_FILE_NEWNAME:		ImportNewName(TRUE);		break;
	case IDM_EXPORT_CLIP_PATH:		Export(2);		break;
	case IDM_IMPORT_CLIP_PATH:		ImportPath(FALSE);		break;
	case IDM_EXPORT_FILE_PATH:		Export(3);		break;
	case IDM_IMPORT_FILE_PATH:		ImportPath(TRUE);		break;

	case IDM_LIST_ADD:			AddByFileDialog(); break;
	case IDM_LIST_ADD_FOLDER:	AddByFolderPicker(); break;
	case IDM_SHOW_SIZE:			ToggleListColumn(COL_FILESIZE); break;
	case IDM_SHOW_MODIFYTIME:	ToggleListColumn(COL_TIMEMODIFY); break;
	case IDM_SHOW_CREATETIME:	ToggleListColumn(COL_TIMECREATE); break;
	case IDM_SHOW_OLDFOLDER:	ToggleListColumn(COL_OLDFOLDER); break;
	case IDM_SHOW_NEWFOLDER:	ToggleListColumn(COL_NEWFOLDER); break;
	case IDM_SHOW_FULLPATH:		ToggleListColumn(COL_FULLPATH); break;

	case IDM_VERSION: APP()->ShowMsg(_T("BatchNamer v2.10 (2022-02-04 Release)\r\n\r\nhttps://blog.naver.com/darkwalk77"), IDSTR(IDS_MSG_VERSION)); 	break;
	case IDM_CFG_LOAD: ConfigLoadType(); break;
	case IDM_CFG_VIEW: ConfigViewOption(); break;
	case IDM_CFG_ETC: ConfigEtc(); break;
	case IDM_PRESET_EDIT: PresetEdit(); break;
	case IDM_PRESET_APPLY1: PresetApply(APP()->m_aPreset[0]); break;
	case IDM_PRESET_APPLY2: PresetApply(APP()->m_aPreset[1]); break;
	case IDM_PRESET_APPLY3: PresetApply(APP()->m_aPreset[2]); break;
	case IDM_PRESET_APPLY4: PresetApply(APP()->m_aPreset[3]); break;
	case IDM_PRESET_APPLY5: PresetApply(APP()->m_aPreset[4]); break;
	case IDM_PRESET_EXPORT: APP()->PresetExport(); break;
	case IDM_PRESET_IMPORT: APP()->PresetImport(); UpdateMenuPreset();  break;
	case IDM_REMOVE_ITEM:
	{
		int nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
		m_list.SetRedraw(FALSE);
		while (nItem != -1)
		{
			m_list.DeleteListItem(nItem);
			nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
		}
		m_list.SetRedraw(TRUE);
		UpdateCount();
	}
	break;
	default:
		return CDialogEx::OnCommand(wParam, lParam);
	}
	return TRUE;
}

void CBatchNamerDlg::PresetEdit()
{
	CDlgPreset dlg;
	dlg.DoModal();
	UpdateMenuPreset();
}

void CBatchNamerDlg::PresetApply(BatchNamerPreset& preset)
{
	INT_PTR nSize = preset.m_aTask.GetSize();
	for (INT_PTR i = 0; i < nSize; i++)
	{
		PresetTask& task = preset.m_aTask[i];
		switch (task.m_nCommand)
		{
		case IDS_TB_01:	StringReplace(task.m_nSubCommand, task.m_str1, task.m_str2, FALSE);	break; //Replace
		case IDS_TB_02: StringAdd(task.m_nSubCommand, task.m_str1, task.m_str2, TRUE, FALSE); break; // Add Front
		case IDS_TB_03: StringAdd(task.m_nSubCommand, task.m_str1, task.m_str2, FALSE, FALSE); break; // Add End
		case IDS_TB_04: NameEmpty(FALSE); break; // Empty Name
		case IDS_TB_05: NameRemoveSelected(task.m_nSubCommand, task.m_str1, task.m_str2); break; // Remove Selected
		case IDS_TB_06: NameNumberFilter(FALSE); break;
		case IDS_TB_07: NameNumberFilter(TRUE); break;
		case IDS_TB_08: NameDigit(task.m_nSubCommand, task.m_str1, task.m_str2); break; // Set Digits
		case IDS_TB_09: NameAddNum(task.m_nSubCommand, task.m_str1, task.m_str2); break; // Add Number
		case IDS_TB_16: NameSetFolder(task.m_nSubCommand, task.m_str1, task.m_str2); break; // Set Parent
		case IDS_TB_17: ExtDel(FALSE); break; // Delete Extension
		case IDS_TB_18: StringAdd(task.m_nSubCommand, task.m_str1, task.m_str2, FALSE, TRUE); break;// Add Extension
		case IDS_TB_19: StringReplace(task.m_nSubCommand, task.m_str1, task.m_str2, TRUE); break;// Replace Extension
		}
	}
}


void CBatchNamerDlg::ToggleListColumn(int nCol)
{
	int nOldWidth = m_list.GetColumnWidth(nCol);
	int nNewWidth = 0;
	if (nOldWidth == 0)
	{
		//FlagSET(APP()->m_nShowFlag, nCol, TRUE);
		if (APP()->m_aColWidth.GetSize() > nCol) nNewWidth = APP()->m_aColWidth.GetAt(nCol);
		if (nNewWidth == 0) nNewWidth = m_lfHeight * 11;
	}
	else
	{
		if (APP()->m_aColWidth.GetSize() > nCol)
		{
			APP()->m_aColWidth.SetAt(nCol, nOldWidth);
			wprintf(L"%d, %d\r\n", APP()->m_aColWidth.GetAt(nCol), nOldWidth);
		}
	}
	m_list.SetColumnWidth(nCol, nNewWidth);
}

BOOL CBatchNamerDlg::PreTranslateMessage(MSG* pMsg)
{
	//주로 단축키의 처리
	if (pMsg->message == WM_KEYDOWN)
	{
		if (st_bIsIdle == TRUE)
		{
			if (pMsg->wParam != VK_CONTROL &&
				pMsg->wParam != VK_SHIFT &&
				pMsg->wParam != VK_ESCAPE &&
				pMsg->wParam != VK_RETURN)
			{
				CHotKeyMap& hkm = APP()->m_mapHotKey;
				CHotKeyMap::iterator i;
				int nPos = -1;
				BOOL bOK = FALSE;
				for (i = hkm.begin(); i != hkm.end(); i++)
				{
					if ((i->second.nKeyCode == (int)pMsg->wParam)
						&& (i->second.bCtrl == (((GetKeyState(VK_CONTROL) & 0xFF00) != 0) ? TRUE : FALSE))
						&& (i->second.bShift == (((GetKeyState(VK_SHIFT) & 0xFF00) != 0) ? TRUE : FALSE)) )
					{
						OnCommand(i->first, 0);
						return TRUE;
					}
				}
			}
			//종료시 확인처리
			if (pMsg->wParam == VK_ESCAPE)
			{
				OnCancel();
				return TRUE;
			}

		}
		else //st_bIsIdle == FALSE
		{
			if (pMsg->wParam == VK_ESCAPE) OnBnClickedBtnStopthread();
			return TRUE;
		}
	}

	BOOL b = (m_list.GetNextItem(-1, LVNI_SELECTED) != -1);
	if (b != m_bSelected)
	{
		GetMenu()->EnableMenuItem(IDM_UNDO_SELECTED, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
		GetMenu()->EnableMenuItem(IDM_REMOVE_ITEM, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
		GetMenu()->EnableMenuItem(IDM_MANUAL_CHANGE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
		GetMenu()->EnableMenuItem(IDM_EDIT_UP, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
		GetMenu()->EnableMenuItem(IDM_EDIT_DOWN, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
		m_tool2.GetToolBarCtrl().EnableButton(IDM_MANUAL_CHANGE, b);
		m_tool2.GetToolBarCtrl().EnableButton(IDM_EDIT_UP, b);
		m_tool2.GetToolBarCtrl().EnableButton(IDM_EDIT_DOWN, b);
		m_bSelected = b;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

CString GetFileSizeString(ULONGLONG nSize);

void CBatchNamerDlg::AddListItem(WIN32_FIND_DATA& fd, CString strDir)
{
	TCHAR fullpath[MY_MAX_PATH];
	ULARGE_INTEGER filesize;
	filesize.HighPart = fd.nFileSizeHigh;
	filesize.LowPart = fd.nFileSizeLow;
	PathCombineW(fullpath, strDir, fd.cFileName);
	CString strSize, strTimeCreate, strTimeModify;
	//중복체크
	CPathSet::iterator it = m_list.m_setPath.find(fullpath);
	if (it == m_list.m_setPath.end()) m_list.m_setPath.insert(fullpath);
	else 
		return; // 이미 존재하는 이름
	BOOL bIsDir = (BOOL)((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
	int nImage = GetFileImageIndexFromMap(fullpath, bIsDir);
	COleDateTime tTemp;
	tTemp = COleDateTime(fd.ftCreationTime);
	strTimeCreate = tTemp.Format(_T("%Y-%m-%d %H:%M:%S"));
	tTemp = COleDateTime(fd.ftLastWriteTime);
	strTimeModify = tTemp.Format(_T("%Y-%m-%d %H:%M:%S"));
	strSize = GetFileSizeString(filesize.QuadPart);
	int nItem = m_list.InsertItem(m_list.GetItemCount(), fd.cFileName, nImage);
	m_list.SetItemText(nItem, COL_NEWNAME, fd.cFileName);
	m_list.SetItemText(nItem, COL_OLDFOLDER, strDir);
	m_list.SetItemText(nItem, COL_NEWFOLDER, strDir);
	m_list.SetItemText(nItem, COL_FILESIZE, strSize);
	m_list.SetItemText(nItem, COL_TIMEMODIFY, strTimeModify);
	m_list.SetItemText(nItem, COL_TIMECREATE, strTimeCreate);
	m_list.SetItemText(nItem, COL_FULLPATH, fullpath);
	m_list.SetItemData(nItem, bIsDir);
}

void CBatchNamerDlg::AddPath(CString strPath, BOOL bIsDirectory)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	int nLoadType = APP()->m_nLoadType;
	if (bIsDirectory) //폴더인 경우의 처리 
	{
		if (APP()->m_nLoadType == 2)
		{
			if (m_nTempLoadType == -1) // 처음 한번만 물어보기
			{
				//폴더를 직접 추가하려면 YES / 폴더 안의 파일을 추가하려면 NO를 선택하세요.
				if (AfxMessageBox(IDSTR(IDS_ASK_FOLDERLOADING), MB_YESNO) == IDNO)
					m_nTempLoadType = 1;
				else 
					m_nTempLoadType = 0;
				nLoadType = m_nTempLoadType;
			}
			else
			{
				nLoadType = m_nTempLoadType;
			}
		}
		if (nLoadType == 1) //폴더 내의 파일들을 추가하기로 선택한 경우 
		{
			CString strName, strFolder, strSize, strTimeCreate, strTimeModify, strFind;
			size_t nLen = 0;
			strFind = strPath + _T("\\*");
			TCHAR fullpath[MY_MAX_PATH];
			hFind = FindFirstFileExW(strFind, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
			if (hFind == INVALID_HANDLE_VALUE) return;
			BOOL b = TRUE , bIsDot = FALSE, bIsDir = FALSE;
			while (b)
			{
				bIsDot = FALSE;
				nLen = _tcsclen(fd.cFileName);
				if (nLen == 1 && fd.cFileName[0] == _T('.')) bIsDot = TRUE; //Dots
				else if (nLen == 2 && fd.cFileName[0] == _T('.') && fd.cFileName[1] == _T('.')) bIsDot = TRUE; //Dots
				if (bIsDot == FALSE)
				{
					bIsDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
					if (bIsDir == TRUE) //폴더인 경우 재귀호출
					{
						PathCombineW(fullpath, strPath, fd.cFileName);
						AddPath(fullpath, bIsDir);
					}
					else //폴더 내 파일
					{
						AddListItem(fd, strPath);
					}
				}
				b = FindNextFileW(hFind, &fd);
			}
			FindClose(hFind);
			return; //복귀
		}
		//else if (nLoadType==0) {} //폴더 이름을 직접 추가하기로 선택한 경우는 단순 파일과 동일하게 처리		
	}
	hFind = FindFirstFileExW(strPath, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		AddListItem(fd, Get_Folder(strPath));
		FindClose(hFind);
	}
}

CString CBatchNamerDlg::GetItemFullPath(int nItem, BOOL bOld)
{
	int nColFolder = bOld ? COL_OLDFOLDER : COL_NEWFOLDER;
	int nColName = bOld ? COL_OLDNAME : COL_NEWNAME;
	CPath path = CPath(m_list.GetItemText(nItem, nColFolder));
	path.AddBackslash();
	path.Append(m_list.GetItemText(nItem, nColName));
	return path.m_strPath;
}

void CBatchNamerDlg::ConfigLoadType()
{
	CDlgCFG_Load dlg;
	dlg.m_bShowEverytime = APP()->m_bShowEverytime;
	dlg.m_nLoadType = APP()->m_nLoadType;
	dlg.m_bAutoSort = APP()->m_bAutoSort;
	if (dlg.DoModal() == IDOK)
	{
		APP()->m_bShowEverytime = dlg.m_bShowEverytime;
		APP()->m_nLoadType = dlg.m_nLoadType;
		APP()->m_bAutoSort = dlg.m_bAutoSort;
	}
}

void CBatchNamerDlg::ConfigViewOption()
{
	CDlgCFG_View dlg;
	dlg.m_clrText = APP()->m_clrText;
	dlg.m_clrBk = APP()->m_clrBk;
	dlg.m_bUseDefaultColor = APP()->m_bUseDefaultColor;
	dlg.m_nFontSize = APP()->m_nFontSize;
	dlg.m_bUseDefaultFont = APP()->m_bUseDefaultFont;
	dlg.m_nIconType = APP()->m_nIconType;
	if (dlg.DoModal() == IDOK)
	{
		if (APP()->m_bUseDefaultColor != dlg.m_bUseDefaultColor)
		{
			APP()->m_bUseDefaultColor = dlg.m_bUseDefaultColor;
			if (APP()->m_bUseDefaultColor == TRUE)
			{
				m_list.SetBkColor(m_clrDefault_Bk);
				m_list.SetTextColor(m_clrDefault_Text);
			}
			else
			{
				APP()->m_clrText = dlg.m_clrText;
				APP()->m_clrBk = dlg.m_clrBk;
				m_list.SetBkColor(APP()->m_clrBk);
				m_list.SetTextColor(APP()->m_clrText);
			}
		}
		if (APP()->m_clrText != dlg.m_clrText)
		{
			APP()->m_clrText = dlg.m_clrText;
			if (APP()->m_bUseDefaultColor == FALSE) m_list.SetTextColor(APP()->m_clrText);
		}
		if (APP()->m_clrBk != dlg.m_clrBk)
		{
			APP()->m_clrBk = dlg.m_clrBk;
			if (APP()->m_bUseDefaultColor == FALSE) m_list.SetBkColor(APP()->m_clrBk);
		}
		if (APP()->m_nFontSize != dlg.m_nFontSize)
		{
			APP()->m_nFontSize = dlg.m_nFontSize;
			if (APP()->m_bUseDefaultFont == FALSE)	UpdateFontSize();
		}
		if (APP()->m_bUseDefaultFont != dlg.m_bUseDefaultFont)
		{
			APP()->m_bUseDefaultFont = dlg.m_bUseDefaultFont;
			UpdateFontSize();
		}
		if (APP()->m_nIconType != dlg.m_nIconType)
		{
			APP()->m_nIconType = dlg.m_nIconType;
			UpdateImageList();
		}
		RedrawWindow();
	}
}

void CBatchNamerDlg::ConfigEtc()
{
	CDlgCFG_Etc dlg;
	dlg.m_bNameAutoFix = APP()->m_bNameAutoFix;
	dlg.m_bUseThread = APP()->m_bUseThread;
	dlg.m_pMenu = GetMenu();
	if (dlg.DoModal() == IDOK)
	{
		APP()->m_bNameAutoFix = dlg.m_bNameAutoFix;
		APP()->m_bUseThread = dlg.m_bUseThread;
		UpdateMenuHotkey();
	}
}

void CBatchNamerDlg::AddPathStart(CString strPath)
{
	//디렉토리인지 여부를 판정하여 인자로 넘김
	DWORD dwAttribute = GetFileAttributes(strPath);
	if (dwAttribute != -1)
	{
		BOOL bIsDirectory = (dwAttribute & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
		AddPath(strPath, bIsDirectory);
	}
}

// Drag & Drop 을 사용해서 파일을 목록에 추가한다 
void CBatchNamerDlg::OnDropFiles(HDROP hDropInfo)
{
	ProcessDropFiles(hDropInfo, DF_ADDLIST);
	DragFinish(hDropInfo); // 실제 드래깅해서 가져오는 경우만 이 방식으로 메모리 해제
}

//클립보드에서 HDROP을 가져오는 경우 DragFinish()하면 Heap 오류가 발생하는 문제를 방지하기 위해 별도로 분리
CString CBatchNamerDlg::ProcessDropFiles(HDROP hDropInfo, int nActionType)
{
	CString strRet;
	WORD cFiles;
	TCHAR szFilePath[MY_MAX_PATH];
	memset(szFilePath, 0, sizeof(TCHAR) * MY_MAX_PATH);
	CString strPath;
	cFiles = DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	CStringArray aPath;
	for (int i = 0; i < cFiles; i++)
	{
		DragQueryFile(hDropInfo, i, szFilePath, MY_MAX_PATH);
		strPath = (LPCTSTR)szFilePath;
		aPath.Add(strPath);
	}
	//1)드래그 앤 드롭, 2)클립보드 복사 둘다 정렬이 제대로 안되므로 이름으로 먼저 정렬한다
	//void* pArrayStart = (void*)&aPath[0];
	//qsort(pArrayStart, aPath.GetSize(), sizeof(CString*), CompareFileName);
	qsort(aPath.GetData(), aPath.GetSize(), sizeof(CString*), CompareFileName);

	//정렬된 파일 목록을 처리한다.
	if (nActionType == DF_ADDLIST) 		//순서대로 목록에 추가하기
	{
		LoadPathArray(aPath);
	}
	else //텍스트로 만들어서 반환하기
	{
		for (int i = 0; i < cFiles; i++)
		{
			if (strRet.IsEmpty() == FALSE) strRet += L'\n';
			if (nActionType == DF_READNEWNAME)	strRet += Get_Name(aPath[i], TRUE);
			else if (nActionType == DF_READFULLPATH) strRet += aPath[i];
		}
	}
	return strRet;
}
void CBatchNamerDlg::LoadPathArray(CStringArray& aPath)
{
	SetDlgItemText(IDC_ST_BAR, IDSTR(IDS_WORKING));
	if (APP()->m_bShowEverytime) ConfigLoadType();
	m_list.SetRedraw(FALSE);
	m_nTempLoadType = -1;
	INT_PTR nSize = aPath.GetSize();
	for (int i = 0; i < nSize; i++) AddPathStart(aPath[i]);
	if (APP()->m_bAutoSort)	m_list.Sort(m_list.GetHeaderCtrl().GetSortColumn(), m_list.GetHeaderCtrl().IsAscending());
	m_list.SetRedraw(TRUE);
	UpdateCount();
}

//폴더 열기 시스템 다이얼로그를 사용해서 폴더를 목록에 추가한다
void CBatchNamerDlg::AddByFolderPicker()
{
	CFolderPickerDialog dlg(NULL, OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_ENABLESIZING, this, sizeof(OPENFILENAME));
	CString strTitle;
	strTitle.LoadString(IDS_LOAD_FOLDERPICKER);
	OPENFILENAME& ofn = dlg.GetOFN();
	ofn.lpstrTitle = strTitle;
	ofn.hwndOwner = GetSafeHwnd();
	int nCount = 0;
	if (dlg.DoModal() == IDOK)
	{
		if (APP()->m_bShowEverytime) ConfigLoadType();
		SetDlgItemText(IDC_ST_BAR, IDSTR(IDS_WORKING));
		CString strPath;
		POSITION pos = dlg.GetStartPosition();
		m_list.SetRedraw(FALSE);
		m_nTempLoadType = -1;
		while (pos)
		{
			strPath = dlg.GetNextPathName(pos);
			AddPathStart(strPath);
			nCount++;
		}
		if (APP()->m_bAutoSort)	m_list.Sort(m_list.GetHeaderCtrl().GetSortColumn(), m_list.GetHeaderCtrl().IsAscending());
		m_list.SetRedraw(TRUE);
		UpdateCount();
	}
	if (nCount > 10000) APP()->ShowMsg(IDSTR(IDS_ERR_TOOMANYITEMS), IDSTR(IDS_MSG_ERROR));
	//CFileDialog의 버그로 인해 Modal 창을 닫고 원래 창으로 복귀한 후 넌클라이언트 영역이 다시 그려지지 않음
	//일단 메뉴만이라도 복구, 타이틀 창은 아직 해결책 못찾음
	SetWindowPos(0, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}


//파일 열기 시스템 다이얼로그를 사용해서 파일을 목록에 추가한다
void CBatchNamerDlg::AddByFileDialog()
{
	OPENFILENAME ofn = { 0 };
	CString strTitle;
	if (strTitle.LoadString(IDS_LOAD_FILEDIALOG) == FALSE) strTitle.Empty();
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->GetSafeHwnd();
	ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ENABLESIZING;
	ofn.lpstrTitle = strTitle;
	//ofn.lpstrFilter = _T("All Files(*.*)\0*.*\0\0"); //모든 파일이 대상인 경우는 필터 불필요
	ofn.nMaxFile = MAX_PATH * 10000; //여러개 선택시 파일이름만 포함되므로 MAX_PATH를 기본값으로 해도 될 것으로 생각
	TCHAR* pBuf = new TCHAR[ofn.nMaxFile];
	memset(pBuf, 0, sizeof(TCHAR) * ofn.nMaxFile);
	ofn.lpstrFile = pBuf;
	if (GetOpenFileName(&ofn) == FALSE)
	{
		if (CommDlgExtendedError() == 0x3003) //FNERR_BUFFERTOOSMALL
		{
			APP()->ShowMsg(IDSTR(IDS_ERR_TOOMANYITEMS), IDSTR(IDS_MSG_ERROR));
		}
	}
	else
	{
		//ofn.lpstrFile에 파일이 하나밖에 없는 경우에는 전체 경로가 하나만 들어가고
		//파일이 여러개인 경우는 폴더명이 첫번째로 들어간 후 두번째부터 폴더명 없는 파일명만 저장되는 방식
		if (APP()->m_bShowEverytime) ConfigLoadType();
		SetDlgItemText(IDC_ST_BAR, IDSTR(IDS_WORKING));
		m_list.SetRedraw(FALSE);
		m_nTempLoadType = -1;
		TCHAR* pCurrentPos = pBuf;
		int nLen = (int)_tcslen(pCurrentPos);
		if (*(pCurrentPos + nLen + 1) == _T('\0')) //아이템이 한개뿐인 경우
		{
			AddPathStart(pCurrentPos);
		}
		else //아이템이 여러개인 경우 
		{
			CString strFolder = pCurrentPos;
			pCurrentPos += (nLen + 1);
			nLen = (int)_tcslen(pCurrentPos);
			while (nLen != 0)
			{
				AddPathStart(strFolder + _T('\\') + pCurrentPos);
				pCurrentPos += (nLen + 1);
				nLen = (int)_tcslen(pCurrentPos);
			}
		}
		if (APP()->m_bAutoSort)	m_list.Sort(m_list.GetHeaderCtrl().GetSortColumn(), m_list.GetHeaderCtrl().IsAscending());
		m_list.SetRedraw(TRUE);
		UpdateCount();
	}
	delete[] pBuf;
}


//리스트 모두 삭제
void CBatchNamerDlg::ClearList(BOOL bClearAll)
{
	if (bClearAll == FALSE)
	{
		CDlgListFilter dlg;
		if (dlg.DoModal() == IDCANCEL) return;
		if (dlg.m_nClearOption == CLEAR_LIST_BYFILTER ||
			dlg.m_nClearOption == CLEAR_LIST_BYFILTER_INVERT)
		{
			BOOL bInvert = (dlg.m_nClearOption == CLEAR_LIST_BYFILTER_INVERT) ? TRUE : FALSE;
			int nCount = m_list.GetItemCount() - 1;
			CString& name_f = dlg.m_strFilter_Name;
			CString& ext_f = dlg.m_strFilter_Ext;
			BOOL bUseName = name_f.IsEmpty() ? FALSE : TRUE;
			BOOL bUseExt = ext_f.IsEmpty() ? FALSE : TRUE;
			if (bUseName == FALSE && bUseExt == FALSE) return; //조건이 모두 비어있느 경우
			BOOL bName = !bUseName, bExt = !bUseExt; // 해당 조건이 필요없는 경우에 무시하기 위한 초기값
			BOOL bIsDir = FALSE;
			CString strFullPath, strName, strExt;
			for (int i = nCount; i >= 0 ; i--) //삭제는 끝에서부터
			{
				bIsDir = (BOOL)m_list.GetItemData(i);
				if (bUseName)
				{
					strName = Get_Name(m_list.GetItemText(i, COL_OLDNAME), bIsDir);
					bName = (ReplaceWithWildCards(strName, name_f, name_f, TRUE).IsEmpty()) ? FALSE : TRUE;
				}
				if (bUseExt)
				{
					strExt = Get_Ext(m_list.GetItemText(i, COL_OLDNAME), bIsDir, FALSE);
					bExt = (ReplaceWithWildCards(strExt, ext_f, ext_f, TRUE).IsEmpty()) ? FALSE : TRUE;
				}
				if ( (!bInvert) == (bName && bExt) )
				{
					strFullPath = m_list.GetOldPath(i);
					m_list.m_setPath.erase(strFullPath);
					m_list.DeleteItem(i);
				}
			}
			m_bSelected = (m_list.GetNextItem(-1, LVNI_SELECTED) != -1);
			UpdateCount();
			return;
		}
		else if (dlg.m_nClearOption == CLEAR_LIST_ALL)
		{
			bClearAll = TRUE;
		}
	}
	if (bClearAll == TRUE)
	{
		m_list.DeleteAllItems();
		m_list.m_setPath.clear();
		m_bSelected = FALSE;
		UpdateCount();
		return;
	}
}

//바뀔 이름을 원래 이름으로 다시 복구
void CBatchNamerDlg::UndoChanges(BOOL bSelectedOnly)
{
	BOOL bUndo = TRUE;
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		bUndo = !bSelectedOnly;
		if (bSelectedOnly == TRUE)
		{
			if (m_list.GetItemState(i, LVNI_SELECTED) == LVNI_SELECTED) bUndo = TRUE;
		}
		if (bUndo)
		{
			m_list.SetItemText(i, COL_NEWNAME, m_list.GetItemText(i, COL_OLDNAME));
			m_list.SetItemText(i, COL_NEWFOLDER, m_list.GetItemText(i, COL_OLDFOLDER));
		}
	}
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::ManualChange()
{
	int n = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (n == -1) return;

	CDlgInput dlg;
	dlg.InitInputByCommand(IDS_TB_13);
	dlg.InitValue(0, m_list.GetItemText(n, COL_NEWNAME), _T(""));
	if (dlg.DoModal() == IDOK)
	{
		m_list.SetItemText(n, COL_NEWNAME, dlg.m_strReturn1);
	}
}

int GetStringTokens(CStringArray& aRet, CString str)
{
	TCHAR c;
	BOOL bToken = FALSE;
	CString strToken, strWild; 
	for (int i = 0; i < str.GetLength(); i++)
	{
		c = str.GetAt(i);
		if (c == _T('?') || c == _T('*'))
		{
			if (strToken.IsEmpty() == FALSE)
			{
				aRet.Add(strToken);
				strToken.Empty();
			}
			strWild = c;
			aRet.Add(strWild);
		}
		else
		{
			strToken += c;
		}
	}
	if (strToken.IsEmpty() == FALSE) aRet.Add(strToken);
	return (int)aRet.GetSize();
}

CString ReplaceWithWildCards(CString strSrc, CString str1, CString str2, BOOL bReturnBlockOnly)
{
	//*와 ?를 이용해서 패턴을 교체하기
	//str1은 찾을 패턴, str2는 교체할 패턴
	//str2는 비어있는 경우 삭제처럼 동작, 교체인 경우는 str1과 str2의 구조가 1:1 매칭이거나 str2가 상수여야 함
	CStringArray aStr1, aStr2, aRet; //토큰을 저장하기 위한 배열 
	int nLen1 = GetStringTokens(aStr1, str1); //str1을 토큰으로 쪼개서 aStr1에 넣는다, 토큰은 *, ? , 일반 문자열 3종류
	int nLen2 = GetStringTokens(aStr2, str2); //str2를 쪼갠다
	int nPos = 0; //strSrc에서 현재 탐색중인 위치를 나타낸다
	int nPrevTokenPos = -1;	//바로 앞에 찾았던 토큰의 위치, ?나 * 다음에 특정 문자열이 오는 경우에 필요
	int nLenSrc = strSrc.GetLength(); //strSrc 문자열 길이
	int nMaxPos = nLenSrc - 1; //현재 토큰의 일치 여부를 판정할때 검색하는 끝지점, ?로 한글자만 검색할때를 식별한다.
	int nBlockBeginPos = -1; //검색된 블록의 시작점, 블록은 전체 패턴에 매칭되는 단위로 하나의 문자열 안에 여러개의 블록 존재 가능
	int nBlockEndPos = -1; //검색된 블록의 끝점
	int nBlockPrevEndPos = -1; //직전 발견한 블록의 끝점
	int i =0 ;
	aRet.SetSize(nLen1);
	CString strRet;
	TCHAR cToken = _T('_');
	while (nPos < nLenSrc) //문자열 끝에 도달할 때까지 블록을 여러번 반복해서 찾기
	{
		nBlockBeginPos = -1; //초기화
		nBlockEndPos = -1; //초기화
		nMaxPos = nLenSrc - 1; //초기화
		for (i = 0; i < nLen1; i++) //단위 블록 검색, 토큰 갯수만큼 돈다
		{
			if (aStr1[i] == _T("?")) //탐색할 토큰이 아무거나 한글자인 경우 
			{
				if (i == 0) nBlockBeginPos = nPos; //처음 시작할 때 만났다면 블록 시작점을 현재 위치로 초기화
				//if (nPos >= nLenSrc) break;//현재 탐색중인 위치가 원본 문자열의 끝을 넘어간 경우 중단
				aRet[i] = strSrc.GetAt(nPos); //블록을 구성하는 실제 글자 토큰을 aRet에 쌓는다.
				nPrevTokenPos = nPos; //토큰의 문자열 내 위치 기억
				nBlockEndPos = nPos; //블록이 끝나는 위치를 현재 탐색 위치로 설정
				nPos += 1; //다음 문자로 이동
				nMaxPos = nPos; //nMaxPos == nPos면 이번 토큰의 처리는 끝났다는 뜻
				cToken = _T('?'); 
			}
			else if (aStr1[i] == _T("*")) //탐색할 토큰이 임의의 문자열인 경우
			{
				if (i == 0) nBlockBeginPos = nPos; //처음 시작할 때 만났다면 블록 시작점을 현재 위치로 초기화
				nPrevTokenPos = nPos; //토큰의 문자열 내 위치 기억
				cToken = _T('*');
				nMaxPos = nLenSrc - 1; //다음 토큰을 찾을 때 문자열 끝까지 찾는다
				if (i == nLen1 - 1) // '*' 이 검색패턴의 마지막인 경우는 남은 문자열을 모두 추가하고 끝냄
				{
					aRet[i] = strSrc.Mid(nPos); //현재위치부터 끝까지를 잘라내서 aRet에 토큰으로 쌓는다
					nBlockEndPos = nLenSrc - 1; //블록 끝점 = 문자열 끝점
					nPos = nLenSrc; //nPos를 끝으로 이동시켜서 더이상 처리할 것이 없음을 표시
					nMaxPos = nPos;
					break; //검색을 끝낸다
				}
			}
			else //일반 문자열 토큰인 경우
			{
				nPos = strSrc.Find(aStr1[i], nPos); //현재 위치부터 해당 토큰을 검색, 발견되는 위치로 현재 위치를 옮긴다
				if (nPos == -1)	// 상수 토큰이 없다면 매칭되는 내용이 없으므로 aRet에 저장된 토큰을 모두 지우고 무조건 중단
				{
					aRet.RemoveAll();
					aRet.SetSize(nLen1);
					break;
				}
				else if (nPos > nMaxPos) //매칭되는 상수 토큰이 범위 이후인 경우 ('?' 사용시) 중단
				{
					nPos = nBlockBeginPos + 1; //다음번 검색 시작점을 한칸 뒤로 이동해서 다시 시작
					aRet.RemoveAll();
					aRet.SetSize(nLen1);
					nBlockEndPos = -1;
					break;
				}
				else //매칭되는 상수토큰을 찾은 경우
				{
					if (i == 0) nBlockBeginPos = nPos; //처음 시작할 때 만났다면 블록 시작점을 현재 위치로 초기화
					if (nLen1 == nLen2) //1:1매칭형인 경우 토큰 쌓기, 이게 아닌 경우에는 단순 삭제임
					{
						if (i > 0 && cToken == _T('*')) //앞의 토큰이 * 였던 경우 추출하여 추가
						{
							aRet[i - 1] = strSrc.Mid(nPrevTokenPos, nPos - nPrevTokenPos);
						}
						aRet[i] = aStr2[i];	//해당 상수토큰 추가
					} 
					//교체할 스트링블록의 끝지점 이동
					nPos += aStr1[i].GetLength();
					nBlockEndPos = nPos - 1;
					nMaxPos = nPos;
					cToken = L'_';
				}
			}
		}
		//블록 검색 종료하였으나 하나도 찾지 못한 경우는 중단
		if (nPos == -1)
		{
			nBlockEndPos = -1;
			break;
		}
		//블록을 찾은 경우
		if (nPos <= nMaxPos && nBlockEndPos >= 0)
		{
			//bReturnBlockOnly로 발견된 패턴 매칭 부분만 처리한다면 문자열 추가 불필요
			//아니면 찾아낸 블록과 이전 블록 간 사이의 문자는 그냥 추가
			if (bReturnBlockOnly == FALSE) strRet += strSrc.Mid(nBlockPrevEndPos + 1, nBlockBeginPos - nBlockPrevEndPos - 1);
			if (nLen1 == nLen2) //1:1 매칭인 경우
			{
				for (int j = 0; j < aRet.GetSize(); j++) strRet += aRet[j];
				aRet.RemoveAll();
				aRet.SetSize(nLen1);
			}
			else  //단일 상수 교체인 경우
			{
				if (nLen2 == 1) strRet += aStr2[0];
			}
		}
		if (nPos >= nLenSrc) break;
		if (nBlockEndPos != -1) nBlockPrevEndPos = nBlockEndPos;
	}
	//뒤에 남은 문자열 추가
	//if (nPos == -1) nPos = 0;
	if (nBlockEndPos == -1) nBlockEndPos = nBlockPrevEndPos; //마지막으로 끝난 블록의 끝부분
	if (bReturnBlockOnly == FALSE) // || cToken == L'*')
	{
		strRet += strSrc.Mid(nBlockEndPos + 1);
	}

	return strRet;
}

void CBatchNamerDlg::StringReplace(int nSubCommand, CString str1, CString str2, BOOL bForExt)
{
	CString strOutput, strName, strExt, strOld, strNew;
	int nPos = -1;
	BOOL bIsDir = FALSE;
	BOOL bUseWildCard = FALSE;
	if (nSubCommand == IDS_REPLACESTRING &&
		(str1.Find(_T('?')) != -1 || str1.Find(_T('*')) != -1) ) bUseWildCard = TRUE;

	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		bIsDir = (BOOL)m_list.GetItemData(i);
		strOutput = m_list.GetItemText(i, COL_NEWNAME);
		strName = Get_Name(strOutput, bIsDir);
		strExt = Get_Ext(strOutput, bIsDir, FALSE); // 확장자 처리를 위해 '.' 을 뺀다
		if (bForExt == FALSE) strOld = strName;
		else strOld = strExt;
		if (nSubCommand == IDS_REPLACESTRING)
		{
			if (bUseWildCard == TRUE)
			{
				strNew = ReplaceWithWildCards(strOld, str1, str2, FALSE);
			}
			else
			{
				strNew = strOld;
				strNew.Replace(str1, str2);
			}
		}
		else if (nSubCommand == IDS_FLIPSTRING)
		{
			nPos = strOld.Find(str1);
			if (nPos != -1)
			{
				//strLeft = strOld.Left(nPos);
				//strRight = strOld.Mid(nPos + str1.GetLength());
				strNew = strOld.Mid(nPos + str1.GetLength()) + str1 + strOld.Left(nPos);
			}
			else strNew = strOld;
		}
		else if (nSubCommand == IDS_LOWERCASE)
		{
			strNew = strOld.MakeLower();
		}
		else if (nSubCommand == IDS_UPPERCASE)
		{
			strNew = strOld.MakeUpper();
		}
		else if (nSubCommand == IDS_UPPERCASE_FIRST)
		{
			TCHAR c;
			strNew = strOld.MakeLower();
			for (int i = 0; i < strNew.GetLength(); i++)
			{
				c = strNew.GetAt(i);
				if ((c >= _T('a') && c <= _T('z')) || (c >= _T('A') && c <= _T('Z')))
				{
					strNew.SetAt(i, towupper(strNew.GetAt(i)));
					break;
				}
			}
		}
		else if (nSubCommand == IDS_UPPERCASE_WORD)
		{
			TCHAR c;
			strNew = strOld;
			BOOL bIsAlphabet = FALSE, bIsAlphabetPrev = FALSE;
			for (int i = 0; i < strNew.GetLength(); i++)
			{
				c = strNew.GetAt(i);
				if ((c >= _T('a') && c <= _T('z')) || (c >= _T('A') && c <= _T('Z')))
				{
					if (bIsAlphabet == FALSE)	strNew.SetAt(i, towupper(strNew.GetAt(i)));
					else						strNew.SetAt(i, towlower(strNew.GetAt(i)));
					bIsAlphabet = TRUE;
				}
				else bIsAlphabet = FALSE;
			}
		}
		else if (nSubCommand == IDS_EXT_REPLACE && bIsDir == FALSE)
		{
			//if (str2.IsEmpty()) return; // 바꿀 확장자
			//입력값에 '.' 가 있는경우 제거해 준다
			if (str1.IsEmpty() == FALSE)
			{
				if (str1.GetAt(0) == _T('.')) str1 = str1.Right(str1.GetLength() - 1);
			}
			if (str2.IsEmpty() == FALSE)
			{
				if (str2.GetAt(0) == _T('.')) str2 = str2.Right(str2.GetLength() - 1);
			}
			BOOL bReplace = str1.IsEmpty();
			if (bReplace == FALSE)
			{
				if (strOld.CompareNoCase(str1) == 0) bReplace = TRUE;
			}
			if (bReplace == TRUE) strNew = str2;
			else strNew = strOld;
		}
		else strNew = strOld;
		if (bForExt == FALSE)	strName = strNew;
		else					strExt = strNew;
		if (strExt.IsEmpty() == FALSE)	strOutput = strName + L'.' + strExt;
		else							strOutput = strName;
		m_list.SetItemText(i, COL_NEWNAME, strOutput);
	}
}

void CBatchNamerDlg::NameReplace()
{
	CDlgInput dlg;
	dlg.InitInputByCommand(IDS_TB_01);
	if (dlg.DoModal() == IDCANCEL) return;
	if (dlg.VerifyReturnValue() == FALSE) return;
	CString strTemp;
	m_list.SetRedraw(FALSE);
	StringReplace(dlg.GetSubCommand(), dlg.m_strReturn1, dlg.m_strReturn2, FALSE);
	m_list.SetRedraw(TRUE);
}


void CBatchNamerDlg::StringAdd(int nSubCommand, CString str1, CString str2, BOOL bFront, BOOL bForExt)
{
	CString strAdd, strName, strExt, strHead, strTail, strOld, strNew, strOutput;
	BOOL bIsDir = FALSE;
	int nPos = 0; //문자열을 추가할 위치, 기능에 따라 앞에서부터인지 뒤에서부터인지 바뀐다
	TCHAR c1 = _T('\0');
	TCHAR c2 = _T('\0');
	int nStart = 0;
	int nEnd = 0;
	//초기값 준비
	switch (nSubCommand)
	{
	case IDS_ADDSTRING:
		if (str1.IsEmpty()) return;
		strAdd = str1;
		nPos = _ttoi(str2);
		break;
	case IDS_ADDPARENT: break;
	case IDS_ADDDATETIMEMODIFY:
	case IDS_ADDDATETIMECREATE:
		if (str1.IsEmpty()) return;
		nPos = _ttoi(str2);
		break;
	case IDS_ADDDATEMODIFY: break;
	case IDS_ADDDATECREATE: break;
	case IDS_ADDTIMEMODIFY: break;
	case IDS_ADDTIMECREATE: break;
	case IDS_MOVE_PATTERN: 
		if (str1.IsEmpty()) return;
		nPos = _ttoi(str2);
		break;
	case IDS_MOVE_BRACKET: 
		if (str1.IsEmpty() || str2.IsEmpty()) return;
		if (str1.GetLength() > 1 || str2.GetLength() > 1) return;
		c1 = str1.GetAt(0);
		c2 = str2.GetAt(0);
		break;
	case IDS_MOVE_POS:
	case IDS_MOVE_POS_REVERSE: 
		nStart = _ttoi(str1);
		nEnd = _ttoi(str2);
		if (nStart == 0 && nEnd == 0) return;
		if (nStart > 0) nStart--; //첫번째 문자의 인덱스는 0
		if (nEnd > 0 && nStart > nEnd) return; //nEnd가 입력된 상태에서 시작이 끝보다 뒤일때
		if (nEnd > 0)	nEnd--; //첫번째 문자의 인덱스는 0
		else			nEnd = -1; //항상 끝까지 
		break;
	}
	//항목별로 돌면서 처리
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		bIsDir = (BOOL)m_list.GetItemData(i);
		strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
		strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir, FALSE);

		if (nSubCommand == IDS_ADDSTRING)
		{
			//strAdd = str1; //여기서 strAdd는 고정값
			//nPos = _ttoi(str2); 
		}
		else if (nSubCommand == IDS_ADDPARENT)
		{
			strAdd = GetFolderName(m_list.GetItemText(i, COL_OLDFOLDER));
			//c:, d: 등 드라이브 루트 경로인 경우 추가히지 않음
			if (strAdd.CompareNoCase(m_list.GetItemText(i, COL_OLDFOLDER)) == 0) strAdd.Empty();
			else strAdd = str1 + strAdd + str2;
		}
		else if (nSubCommand == IDS_ADDDATETIMEMODIFY)
		{
			strAdd = FormatTimeString(m_list.GetItemText(i, COL_TIMEMODIFY), str1);
		}
		else if (nSubCommand == IDS_ADDDATETIMECREATE)
		{
			strAdd = FormatTimeString(m_list.GetItemText(i, COL_TIMECREATE), str1);
		}
		//이전 버전 호환성을 위한 코드 시작
		else if (nSubCommand == IDS_ADDDATEMODIFY) strAdd = str1 + GetTimeStringToAdd(m_list.GetItemText(i, COL_TIMEMODIFY), TRUE, FALSE) + str2;
		else if (nSubCommand == IDS_ADDDATECREATE) strAdd = str1 + GetTimeStringToAdd(m_list.GetItemText(i, COL_TIMECREATE), TRUE, FALSE) + str2;
		else if (nSubCommand == IDS_ADDTIMEMODIFY) strAdd = str1 + GetTimeStringToAdd(m_list.GetItemText(i, COL_TIMEMODIFY), FALSE, TRUE) + str2;
		else if (nSubCommand == IDS_ADDTIMECREATE) strAdd = str1 + GetTimeStringToAdd(m_list.GetItemText(i, COL_TIMECREATE), FALSE, TRUE) + str2;
		// 끝
		// 잘라내서 붙이기 기능 (2.1 버전부터 추가)
		else if (nSubCommand == IDS_MOVE_PATTERN)
		{
			strAdd = ReplaceWithWildCards(strName, str1, str1, TRUE); //찾아낸 블록 반환
			if (strAdd.IsEmpty() == FALSE)
			{
				strName = ReplaceWithWildCards(strName, str1, _T(""), FALSE); //찾아낸 부분을 뺀 이름 반환
			}
		}
		else if (nSubCommand == IDS_MOVE_BRACKET)
		{
			if (FindBracketPart(strName, c1, c2, nStart, nEnd) == TRUE)
			{
				strAdd = strName.Mid(nStart, nEnd - nStart + 1);
				strName.Delete(nStart, nEnd - nStart + 1);
			}
		}
		else if (nSubCommand == IDS_MOVE_POS || nSubCommand == IDS_MOVE_POS_REVERSE)
		{
			if (nSubCommand == IDS_MOVE_POS_REVERSE) strName = strName.MakeReverse();
			if (nEnd == -1)
			{
				strAdd = strName.Mid(nStart);
				strName.Delete(nStart, strName.GetLength() - nStart);
			}
			else
			{
				strAdd = strName.Mid(nStart, nEnd - nStart + 1);
				strName.Delete(nStart, nEnd - nStart + 1);
			}
			if (strAdd.IsEmpty() == FALSE)
			{
				if (nSubCommand == IDS_MOVE_POS_REVERSE) strAdd = strAdd.MakeReverse();
			}
		}

		if (bForExt == FALSE)	strOld = strName;
		else					strOld = strExt;

		if (bIsDir != FALSE && bForExt != FALSE)
		{
			strNew = strOld;//디렉토리일때 확장자를 추가하려는 시도 차단
		}
		else if (bFront)
		{
			if (nPos != 0)
			{
				strHead = strOld.Left(nPos);
				strTail = strOld.Mid(nPos);
				strNew = strHead + strAdd + strTail;
			}
			else strNew = strAdd + strOld;
		}
		else
		{
			if (nPos != 0)
			{
				strHead = strOld.Left(strOld.GetLength() - nPos);
				strTail = strOld.Right(nPos);
				strNew = strHead + strAdd + strTail;
			}
			else strNew = strOld + strAdd;
		}
		if (bForExt == FALSE)	strName = strNew;
		else					strExt = strNew;
		if (strExt.IsEmpty() == FALSE)	strOutput = strName + L'.' + strExt;
		else							strOutput = strName;
		m_list.SetItemText(i, COL_NEWNAME, strOutput);
	}
}

void CBatchNamerDlg::NameAdd(BOOL bFront = TRUE)
{
	CDlgInput dlg;
	dlg.InitInputByCommand(bFront ? IDS_TB_02 : IDS_TB_03);
	if (dlg.DoModal() == IDCANCEL) return;
	m_list.SetRedraw(FALSE);
	StringAdd(dlg.GetSubCommand(), dlg.m_strReturn1, dlg.m_strReturn2, bFront, FALSE);
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::NameNumberFilter(BOOL bRemoveNumber)
{
	CString strName, strExt;
	//if (bToggleRedraw == TRUE) 
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		BOOL bIsDir = (BOOL)m_list.GetItemData(i);
		strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
		strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir);

		for (int j = strName.GetLength() - 1; j >= 0; j--)
		{
			if (bRemoveNumber == FALSE)  // 숫자만 남기기
			{
				if (_T('0') > strName.GetAt(j) || strName.GetAt(j) > _T('9'))  strName.Delete(j);
			}
			else // 숫자를 지우기
			{
				if (_T('0') <= strName.GetAt(j) && strName.GetAt(j) <= _T('9'))  strName.Delete(j);
			}
		}
		if (strExt.IsEmpty() == FALSE) strName += strExt;
		m_list.SetItemText(i, COL_NEWNAME, strName);
	}
	//if (bToggleRedraw == TRUE) 
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::NameDigit(int nSubCommand, CString str1, CString str2)
{
	CString strName, strExt;
	int nStatus;
	int nStart;
	int nEnd;
	int nDigit = _ttoi(str1);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		BOOL bIsDir = (BOOL)m_list.GetItemData(i);
		strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
		strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
		nStatus = 0;
		nStart = -1;
		nEnd = -1;
		if (nSubCommand == IDS_DIGITBACK) //뒷번호
		{
			for (int j = strName.GetLength() - 1; j >= 0; j--)
			{
				if ((_T('0') <= strName.GetAt(j) && strName.GetAt(j) <= _T('9')) && nStatus == 0)
				{
					nEnd = j;
					nStatus = 1;
				}
				if ((_T('0') > strName.GetAt(j) || strName.GetAt(j) > _T('9')) && nStatus == 1)
				{
					nStatus = 2;
					nStart = j + 1;
					break;
				}
			}
		}
		else //if (nSubCommand == IDS_DIGITFRONT) //앞번호
		{
			for (int j = 0; j <= strName.GetLength() - 1; j++)
			{
				if ((_T('0') <= strName.GetAt(j) && strName.GetAt(j) <= _T('9')) && nStatus == 0)
				{
					nStart = j;
					nStatus = 1;
				}
				if ((_T('0') > strName.GetAt(j) || strName.GetAt(j) > _T('9')) && nStatus == 1)
				{
					nStatus = 2;
					nEnd = j - 1;
					break;
				}
			}
		}
		if (nStatus == 1) nStart = 0;
		int nNumLength = nEnd - nStart + 1;
		if (nStart != -1 && nEnd != -1 && nNumLength > 0)
		{
			for (int i = 0; i < nDigit - nNumLength; i++) strName.Insert(nStart, _T("0"));
		}
		if (strExt.IsEmpty() == FALSE) strName += strExt;
		m_list.SetItemText(i, COL_NEWNAME, strName);
	}
}

void CBatchNamerDlg::NameDigit()
{
	CDlgInput dlg;
	dlg.InitInputByCommand(IDS_TB_08);
	if (dlg.DoModal() == IDCANCEL) return;
	if (dlg.VerifyReturnValue() == FALSE) return;
	m_list.SetRedraw(FALSE);
	NameDigit(dlg.GetSubCommand(), dlg.m_strReturn1, dlg.m_strReturn2);
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::ExtDel(BOOL bToggleRedraw) //확장자 삭제
{
	CString strTemp;
	if (bToggleRedraw == TRUE) m_list.SetRedraw(FALSE);
	BOOL bIsDir = FALSE;
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		bIsDir = (BOOL)m_list.GetItemData(i);
		if (bIsDir == FALSE)
		{
			strTemp = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
			m_list.SetItemText(i, COL_NEWNAME, strTemp);
		}
	}
	if (bToggleRedraw == TRUE) m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::ExtAdd()
{
	CDlgInput dlg;
	dlg.InitInputByCommand(IDS_TB_18);
	if (dlg.DoModal() == IDCANCEL) return;
	if (dlg.VerifyReturnValue() == FALSE) return;
	m_list.SetRedraw(FALSE);
	StringAdd(dlg.GetSubCommand(), dlg.m_strReturn1, dlg.m_strReturn2, FALSE, TRUE);
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::ExtReplace()
{
	CDlgInput dlg;
	dlg.InitInputByCommand(IDS_TB_19);
	if (dlg.DoModal() == IDCANCEL) return;
	if (dlg.VerifyReturnValue() == FALSE) return;
	m_list.SetRedraw(FALSE);
	StringReplace(dlg.GetSubCommand(), dlg.m_strReturn1, dlg.m_strReturn2, TRUE);
	m_list.SetRedraw(TRUE);
}

/*void CBatchNamerDlg::ExtAdd(int nSubCommand, CString str1, CString str2)
{
	CString strTemp;
	CString strExt = str1;
	if (strExt.IsEmpty()) return;
	if (strExt.GetAt(0) != _T('.')) strExt = _T(".") + strExt;
	BOOL bIsDir = FALSE;
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		bIsDir = (BOOL)m_list.GetItemData(i);
		if (bIsDir == FALSE)
		{
			strTemp = m_list.GetItemText(i, COL_NEWNAME);
			strTemp += strExt;
			m_list.SetItemText(i, COL_NEWNAME, strTemp);
		}
	}
}

void CBatchNamerDlg::ExtReplace(int nSubCommand, CString str1, CString str2)
{
	CString strTemp;
	CString strCurrentExt, strOldExt, strNewExt;
	strOldExt = str1;
	strNewExt = str2;
	if (strNewExt.IsEmpty()) return;
	if (strNewExt.GetAt(0) != _T('.')) strNewExt = _T(".") + strNewExt;
	if (strOldExt.IsEmpty() == FALSE)
	{
		if (strOldExt.GetAt(0) != _T('.')) strOldExt = _T(".") + strOldExt;
	}
	BOOL bIsDir = FALSE;
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		bIsDir = (BOOL)m_list.GetItemData(i);
		if (bIsDir == FALSE)
		{
			if (strOldExt.IsEmpty() == FALSE)
			{
				strCurrentExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
				if (strOldExt.CompareNoCase(strCurrentExt) == 0)
				{
					strTemp = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
					strTemp += strNewExt;
					m_list.SetItemText(i, COL_NEWNAME, strTemp);
				}
			}
			else
			{
				strTemp = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
				strTemp += strNewExt;
				m_list.SetItemText(i, COL_NEWNAME, strTemp);
			}
		}
	}
}*/

static int st_nApplyOption = APPLY_MOVE;

void CBatchNamerDlg::ApplyChange_Start()
{
	CDlgApplyOption dlg;
	if (dlg.DoModal() == IDCANCEL) return;
	st_nApplyOption = dlg.m_nApplyOption;
	if (APP()->m_bUseThread == FALSE)
	{
		ApplyChange(st_nApplyOption);
	}
	else
	{
		AfxBeginThread(ApplyChange_Thread, this);
	}
}

UINT CBatchNamerDlg::ApplyChange_Thread(void* lParam)
{
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
	CBatchNamerDlg* dlg = (CBatchNamerDlg*)lParam;
	APP()->UpdateThreadLocale();
	dlg->ApplyChange(st_nApplyOption);
	return 0;
}

void RemoveInvalidCharForFile(CString& str, BOOL bPassWildCard, BOOL bPathWithFolder)
{
	if (bPathWithFolder == FALSE)
	{
		str.Remove(_T('\\'));
		str.Remove(_T(':'));
	}
	str.Remove(_T('\"'));
	str.Remove(_T('/'));
	str.Remove(_T('|'));
	str.Remove(_T('<'));
	str.Remove(_T('>'));
	str.Remove(_T('\r'));
	str.Remove(_T('\n'));
	str.Remove(_T('\t'));
	if (bPassWildCard == FALSE)
	{
		str.Remove(_T('?'));
		str.Remove(_T('*'));
	}
}

//파일 이름에 맞지 않는 글자(\, /, | ,<. >, :, ", ?, *) 를 미리 체크
BOOL CheckInvalidCharForFile(CString str, BOOL bPassWildCard, BOOL bPathWithFolder)
{
	if (bPathWithFolder == FALSE && str.Find(_T('\\')) != -1) return TRUE;
	if (bPathWithFolder == FALSE && str.Find(_T(':')) != -1) return TRUE;
	if (str.Find(_T('\"')) != -1) return TRUE;
	if (str.Find(_T('/')) != -1) return TRUE;
	if (str.Find(_T('|')) != -1) return TRUE;
	if (str.Find(_T('<')) != -1) return TRUE;
	if (str.Find(_T('>')) != -1) return TRUE;
	if (bPassWildCard == FALSE && str.Find(_T('?')) != -1) return TRUE;
	if (bPassWildCard == FALSE && str.Find(_T('*')) != -1) return TRUE;
	return FALSE;
}

//상위 폴더가 없는 경우 해당 폴더를 포함하여 폴더를 생성한다
BOOL CreateFolder_Ensure(CString strFolder)
{
	CString strParent = Get_Folder(strFolder);
	if (GetFileAttributes(strParent) == INVALID_FILE_ATTRIBUTES)
	{
		//상위 폴더가 없는 경우 재귀호출로 생성 시도
		if (CreateFolder_Ensure(strParent) == FALSE) return FALSE;
	}
	try
	{
		//해당 폴더 생성
		if (CreateDirectory(strFolder, NULL) == 0) // 0 이면 실패
		{
			//생성에 실패하더라도 이미 존재하는 경우는 TRUE 리턴
			if (GetLastError() != ERROR_ALREADY_EXISTS) return FALSE;
		}
	}
	catch (CFileException* e)
	{
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

void StringArray2szzBuffer(CStringArray& aPath, TCHAR*& pszzBuf)
{
	if (aPath.GetSize() == 0)
	{
		pszzBuf = NULL;
		return;
	}
	//Caculate Buffer Size
	size_t uBufSize = 0;
	for (int i = 0; i < aPath.GetSize(); i++)
	{
		uBufSize += aPath[i].GetLength() + 1; //String + '\0'
	}
	uBufSize += 1; //For the last '\0'
	//Copy into buffer
	pszzBuf = new TCHAR[uBufSize];
	memset(pszzBuf, 0, uBufSize * sizeof(TCHAR));
	TCHAR* pBufPos = pszzBuf;
	for (int i = 0; i < aPath.GetSize(); i++)
	{
		lstrcpy(pBufPos, (LPCTSTR)aPath[i]);
		pBufPos = 1 + _tcschr(pBufPos, _T('\0'));
	}
}

//BOOL CopyPath(CStringArray& aOldPath, CStringArray& aNewPath, BOOL bMove)
BOOL CopyPath(CString& strOldPath, CString& strNewPath, BOOL bMove)
{
	//	if (aOldPath.GetSize() == 0 || aOldPath.GetSize() != aNewPath.GetSize()) return FALSE;
	TCHAR pszzBuf_OldPath[MY_MAX_PATH];
	TCHAR pszzBuf_NewPath[MY_MAX_PATH];
	memset(pszzBuf_OldPath, 0, MY_MAX_PATH * sizeof(TCHAR));
	memset(pszzBuf_NewPath, 0, MY_MAX_PATH * sizeof(TCHAR));
	lstrcpy(pszzBuf_OldPath, (LPCTSTR)strOldPath);
	lstrcpy(pszzBuf_NewPath, (LPCTSTR)strNewPath);
	SHFILEOPSTRUCT FileOp = { 0 };
	FileOp.hwnd = NULL;
	FileOp.wFunc = bMove ? FO_MOVE : FO_COPY;
	FileOp.pFrom = pszzBuf_OldPath;
	FileOp.pTo = pszzBuf_NewPath;
	FileOp.fFlags = FOF_MULTIDESTFILES | FOF_ALLOWUNDO; //| FOF_WANTMAPPINGHANDLE 
	//FileOp.fFlags = FileOp.fFlags | FOF_RENAMEONCOLLISION; //중복되는 이름이 있는 경우 자동으로 리네임하기
	FileOp.fAnyOperationsAborted = FALSE;
	FileOp.hNameMappings = NULL;
	FileOp.lpszProgressTitle = NULL;
	int nRet = SHFileOperation(&FileOp);
	if (FileOp.fAnyOperationsAborted != FALSE)
	{
		return FALSE;
	}
	return TRUE;
}

//실제 파일 시스템상의 정보를 바꿔 파일 이름 변경하기
void CBatchNamerDlg::ApplyChange(int nApplyOption)
{
	BOOL bError = FALSE;
	st_bIsIdle = FALSE;
	SetDlgItemText(IDC_ST_BAR, IDSTR(IDS_WORKING));
	m_list.EnableWindow(FALSE);
	m_tool1.EnableWindow(FALSE);
	m_tool2.EnableWindow(FALSE);
	if (APP()->m_bUseThread == FALSE)
	{
		GetDlgItem(IDC_ST_BAR)->Invalidate();
		GetDlgItem(IDC_ST_BAR)->RedrawWindow();
	}
	else
	{
		ArrangeCtrl();
	}
	clock_t st, et;
	st = clock();
	CString strNewPath, strTemp, strLog, strErr;
	//선택된 부분 초기화
	int nItemSel = m_list.GetNextItem(-1, LVNI_SELECTED);
	while (nItemSel != -1)
	{
		m_list.SetItemState(nItemSel, 0, LVIS_SELECTED | LVIS_FOCUSED);
		nItemSel = m_list.GetNextItem(-1, LVNI_SELECTED);
	}
	
	CPathSet setNewFolder; // 폴더 존재여부 확인용
	//중복여부 체크 및 새로운 파일명 만들기
	CPathSet setNewPath;  //파일 이름 중복 확인용
	CPathSet::iterator it;
	CStrArray aNewPath;
	int nCount = m_list.GetItemCount();
	int nMsg = 0;
	for (int i = 0; i < nCount; i++)
	{
		strTemp = m_list.GetItemText(i, COL_NEWNAME);
		nMsg = 0;
		//변경할 이름이 비어있어나 잘못된 문자가 포함된 경우를 검사한다.
		if (strTemp.IsEmpty() == TRUE) nMsg = IDS_MSG_NONAME;
		else if (CheckInvalidCharForFile(strTemp, FALSE, FALSE)) nMsg = IDS_INVALID_CHAR;
		if (nMsg != 0)
		{
			APP()->ShowMsg(IDSTR(nMsg), IDSTR(IDS_MSG_ERROR));
			m_list.SetFocus();
			m_list.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			m_list.EnsureVisible(i, FALSE);
			bError = TRUE; 
			break;
		}
		//이름을 만든다
		strNewPath = m_list.GetItemText(i, COL_NEWFOLDER) + _T("\\") + strTemp;
		//중복되는 이름이 있는지 검사한다
		it = setNewPath.find(strNewPath);
		if (it == setNewPath.end())
		{
			setNewPath.insert(strNewPath); //없으므로 Set에 등록
		}
		else
		{
			strErr.Format(_T("%s\r\n\r\n%s\r\n(%s)"), IDSTR(IDS_MSG_DUPNAME), strTemp, strNewPath);
			APP()->ShowMsg(strErr, IDSTR(IDS_MSG_ERROR));
			m_list.SetFocus();
			m_list.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			m_list.EnsureVisible(i, FALSE);
			bError = TRUE; 
			break;
		}
		aNewPath.push_back(strNewPath); // Array에 추가
	}
	if (bError == FALSE && aNewPath.size() != nCount)
	{ 
		APP()->ShowMsg(IDSTR(IDS_MSG_MISMATCH), IDSTR(IDS_MSG_ERROR)); 
		bError = TRUE;
	}
	if (bError == TRUE)
	{
		st_bIsIdle = TRUE;
		m_list.EnableWindow(TRUE);
		m_tool1.EnableWindow(TRUE);
		m_tool2.EnableWindow(TRUE);
		UpdateCount();
		return;
	}

	//실제 파일이름을 바꾸는 곳
	CString strOldPath, strOldExt, strNewExt, strBar, strNewFolder;
	int i=0, nImage = 0;
	int nChanged = 0, nPercent = 0, nPreviousPercent = 0;
	BOOL bIsDir = FALSE;
	BOOL bSuccess = FALSE;
	for (i = 0; i < nCount; i++)
	{
		if (st_bIsIdle == TRUE) break;
		strOldPath = m_list.GetOldPath(i);
		bIsDir = (BOOL)m_list.GetItemData(i);
		try
		{
			//경로가 존재하는지 확인하고 없다면 만든다
			strNewFolder = m_list.GetItemText(i, COL_NEWFOLDER);
			if (strNewFolder.CompareNoCase(m_list.GetItemText(i, COL_OLDFOLDER)) != 0)
			{ 
				//경로가 바뀌어야 하는 경우 파일시스템을 보기 전에 setNewFolder 를 검색한다.
				it = setNewFolder.find(strNewFolder);
				if (it == setNewFolder.end())
				{  //새로운 폴더인 경우 GetFileAttribute 로 존재여부를 확인한다.
					DWORD dw = GetFileAttributes(strNewFolder);
					if (dw == INVALID_FILE_ATTRIBUTES)
					{
						CreateFolder_Ensure(strNewFolder);//존재하지 않으면 만든다
						setNewFolder.insert(strNewFolder); //폴더 생성이 성공하든 실패하든 반복하지 않도록 set에 넣어준다
					}
					//else if ((dw & FILE_ATTRIBUTE_DIRECTORY) == 0) // 존재는 하지만 디렉토리가 아닌 경우 => Exception 으로 처리
				}
			}
			//실제 파일 처리
			if (aNewPath.at(i).Compare(strOldPath) != 0)
			{
				if (nApplyOption == APPLY_MOVE)
				{
					//MoveFileEx는 폴더/파일 모두 통한다 (CopyFileEx는 파일만 가능)
					bSuccess = MoveFileExW(strOldPath, aNewPath[i], MOVEFILE_COPY_ALLOWED);
				}
				else if (nApplyOption == APPLY_COPY)
				{
					if (bIsDir == FALSE)
					{
						//일반 파일 카피는 중간 캔슬이 가능한 CopyFileEx 사용
						bSuccess = CopyFileExW(strOldPath, aNewPath[i], NULL, NULL, &st_bIsIdle, COPY_FILE_FAIL_IF_EXISTS | COPY_FILE_ALLOW_DECRYPTED_DESTINATION);
					}
					else
					{
						//폴더를 복사하는 경우 간편한 재귀 복사를 위해 SHFileOperation을 사용한다.
						bSuccess = CopyPath(strOldPath, aNewPath[i], FALSE);
					}
				}
				else
				{
					break;
				}
				if (bSuccess == FALSE)
				{
					if (bIsDir == TRUE && nApplyOption == APPLY_COPY)
					{  //SHFileOperation을 캔슬한 경우 GetLastError()를 사용할 수 없으므로 별도로 처리한다.
						strTemp.Format(_T("(%s) %s → %s\r\n - %s"), IDSTR(IDS_MSG_CHANGEFAIL), m_list.GetOldPath(i), aNewPath[i], IDSTR(IDS_FILEOPCANCELED) );
					}
					else
					{
						LPVOID lpMsgBuf;
						DWORD err = GetLastError();
						FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
							FORMAT_MESSAGE_FROM_SYSTEM |
							FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPTSTR)&lpMsgBuf, 0, NULL);
						strErr = (LPCTSTR)lpMsgBuf;
						LocalFree(lpMsgBuf);
						if (strErr.IsEmpty() == FALSE && strErr.GetAt(strErr.GetLength() - 1) != _T('\n')) strErr += _T("\r\n");
						strTemp.Format(_T("(%s) %s → %s\r\n - %s"), IDSTR(IDS_MSG_CHANGEFAIL), Get_Name(strOldPath), Get_Name(aNewPath[i]), strErr);
					}
					strLog += strTemp;
				}
				else
				{
					//변경이 성공한 경우 리스트에 표시하기 
					m_list.SetItemText(i, COL_OLDNAME, m_list.GetItemText(i, COL_NEWNAME));
					m_list.SetItemText(i, COL_OLDFOLDER, m_list.GetItemText(i, COL_NEWFOLDER));
					m_list.SetItemText(i, COL_FULLPATH, aNewPath.at(i));
					//확장자가 바뀐 경우 아이콘 변경
					if (bIsDir == FALSE)
					{
						CString strOldExt = Get_Ext(strOldPath, bIsDir);
						CString strNewExt = Get_Ext(aNewPath[i], bIsDir);
						if (strOldExt.CompareNoCase(strNewExt) != 0)
						{
							nImage = GetFileImageIndexFromMap(aNewPath[i], bIsDir);
							m_list.SetItem(i, 0, LVIF_IMAGE, NULL, nImage, 0, 0, 0);
						}
					}
					nChanged++;
				}
			}
			else
			{
				strTemp.Format(_T("(%s) %s\r\n - %s\r\n"), IDSTR(IDS_MSG_CHANGEFAIL), Get_Name(strOldPath), IDSTR(IDS_MSG_SAMENAME));
				strLog += strTemp;
			}
		}
		catch (CFileException* e)
		{
			TCHAR pBufMsg[1000];
			e->GetErrorMessage(pBufMsg, 1000);
			e->Delete();
			strErr = pBufMsg;
			if (strErr.IsEmpty() == FALSE && strErr.GetAt(strErr.GetLength() - 1) != _T('\n')) strErr += _T("\r\n");
			strTemp.Format(_T("(%s) %s → %s\r\n - %s\r\n"),IDSTR(IDS_MSG_CHANGEFAIL), Get_Name(strOldPath), Get_Name(aNewPath.at(i)), strErr);
			strLog += strTemp;
		}
		if (APP()->m_bUseThread != FALSE)
		{
			nPercent = nChanged * 100 / nCount;
			if (nPercent != nPreviousPercent && nPercent % 2 == 0)
			{
				nPreviousPercent = nPercent;
				strBar.Format(IDSTR(IDS_PROGRESS_CHANGE), nPercent);
				SetDlgItemText(IDC_ST_BAR, strBar);
			}
		}
	}
	strTemp.Format(IDSTR(IDS_MSG_CHANGEDONE), nCount, i, nChanged);
	if (strLog.IsEmpty() == FALSE) strTemp += _T("\r\n\r\n");
	strLog = strTemp + strLog;
	et = clock();
	strTemp.Format(IDSTR(IDS_ELAPSED_TIME), et - st);
	strLog += L"\r\n" + strTemp;
	APP()->ShowMsg(strLog, IDSTR(IDS_RESULT_REPORT));
	m_list.EnableWindow(TRUE);
	m_tool1.EnableWindow(TRUE);
	m_tool2.EnableWindow(TRUE);
	st_bIsIdle = TRUE;
	UpdateCount();
	if (APP()->m_bUseThread != FALSE) ArrangeCtrl();
}

//두개의 아이템 교환
void CBatchNamerDlg::SwapItem(int n1, int n2)
{
	CString str1, str2;
	int nCol = COL_TOTAL, i = 0;

	//아이템 데이타 교환
	DWORD_PTR dw1 = (BOOL)m_list.GetItemData(n1);
	DWORD_PTR dw2 = (BOOL)m_list.GetItemData(n2);
	m_list.SetItemData(n1, dw2);
	m_list.SetItemData(n2, dw1);

	//아이콘 이미지 교환
	LVITEM li;
	memset(&li, 0, sizeof(LVITEM));
	li.mask = LVIF_IMAGE;
	li.iItem = n1;
	m_list.GetItem(&li);
	int img1 = li.iImage;
	li.iItem = n2;
	m_list.GetItem(&li);
	int img2 = li.iImage;

	m_list.SetItem(n1, 0, LVIF_IMAGE, NULL, img2, 0, 0, 0);
	m_list.SetItem(n2, 0, LVIF_IMAGE, NULL, img1, 0, 0, 0);

	//칼럼 정보 교환 
	for (i = 0; i < nCol; i++)
	{
		str1 = m_list.GetItemText(n1, i);
		str2 = m_list.GetItemText(n2, i);
		m_list.SetItemText(n1, i, str2);
		m_list.SetItemText(n2, i, str1);
	}

	//아이템 상태 교환
	DWORD d1 = m_list.GetItemState(n1, LVIS_SELECTED | LVIS_FOCUSED);
	DWORD d2 = m_list.GetItemState(n2, LVIS_SELECTED | LVIS_FOCUSED);
	m_list.SetItemState(n1, d2, LVIS_SELECTED | LVIS_FOCUSED);
	m_list.SetItemState(n2, d1, LVIS_SELECTED | LVIS_FOCUSED);

	m_list.EnsureVisible(n2, FALSE);
}

//선택된 리스트 아이템을 한칸 위로 올린다 
void CBatchNamerDlg::ListUp()
{
	m_list.SetRedraw(FALSE);
	int nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
	while (nItem != -1)
	{
		if (nItem > 0)
		{
			SwapItem(nItem, nItem - 1);
			nItem = m_list.GetNextItem(nItem, LVNI_SELECTED);
		}
		else
		{
			nItem = -1;
		}
	}
	m_list.SetRedraw(TRUE);

}

//선택된 리스트 아이템을 한칸 아래로 내린다 
void CBatchNamerDlg::ListDown()
{
	int nItem = m_list.GetNextItem(m_list.GetItemCount() - 2, LVNI_SELECTED);
	if (nItem != -1) return; //Bottom Item Selected
	nItem = m_list.GetNextItem(m_list.GetItemCount() - 1, LVNI_SELECTED | LVNI_ABOVE);
	while (nItem != -1)
	{
		if (nItem >= 0 && nItem < m_list.GetItemCount() - 1)
		{
			SwapItem(nItem, nItem + 1);
			nItem = m_list.GetNextItem(nItem, LVNI_SELECTED | LVNI_ABOVE);
		}
		else
		{
			nItem = -1;
		}
	}
}

void CBatchNamerDlg::NameAddNum(int nSubCommand, CString str1, CString str2)
{
	int nDigit = _ttoi(str1);
	int nStart = _ttoi(str2);
	if (nStart <= 0) nStart = 0;
	CString strName, strExt, strTemp, strFolder;
	int nCurrent = nStart;
	int nCount = m_list.GetItemCount();
	//폴더별 번호 붙이기를 위해서는 폴더별로 카운트를 따로 해야 함
	//폴더에 대한 map(폴더명, 현재카운트)을 만들어서 처리
	CFolderMap mapFolder;
	for (int i = 0; i < nCount; i++)
	{
		if (nSubCommand == IDS_ADDNUM_BYFOLDER_BACK || nSubCommand == IDS_ADDNUM_BYFOLDER_FRONT)
		{
			strFolder = m_list.GetItemText(i, COL_NEWFOLDER);
			CFolderMap::iterator it = mapFolder.find(strFolder);
			if (it == mapFolder.end())
			{
				mapFolder.insert(CFolderMap::value_type(strFolder, nStart));
				nCurrent = nStart;
			}
			else
			{
				nCurrent = it->second + 1;
				it->second = nCurrent;
			}
		}
		BOOL bIsDir = (BOOL)m_list.GetItemData(i);
		strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
		strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
		strTemp.Format(_T("%d"), nCurrent);
		while (nDigit > strTemp.GetLength()) strTemp = _T('0') + strTemp;
		if (nSubCommand == IDS_ADDNUM_ALL_BACK || nSubCommand == IDS_ADDNUM_BYFOLDER_BACK)
			strName += strTemp; //뒤에 붙이기
		else //if (nSubCommand == IDS_ADDNUM_ALL_FRONT || nSubCommand == IDS_ADDNUM_BYFOLDER_FRONT) 
			strName = strTemp + strName; //앞에 붙이기
		if (strExt.IsEmpty() == FALSE) strName += strExt;
		m_list.SetItemText(i, COL_NEWNAME, strName);
		if (nSubCommand == IDS_ADDNUM_ALL_BACK || nSubCommand == IDS_ADDNUM_ALL_FRONT) nCurrent++;
	}
	mapFolder.clear();
}

void CBatchNamerDlg::NameAddNum()
{
	CDlgInput dlg;
	dlg.InitInputByCommand(IDS_TB_09);
	if (dlg.DoModal() == IDCANCEL) return;
	if (dlg.VerifyReturnValue() == FALSE) return;
	m_list.SetRedraw(FALSE);
	NameAddNum(dlg.GetSubCommand(), dlg.m_strReturn1, dlg.m_strReturn2);
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::NameEmpty(BOOL bToggleRedraw) //바꿀 이름 부분을 확장자만 제외하고 모두 지운다
{
	CString strExt;
	if (bToggleRedraw == TRUE) m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), (BOOL)m_list.GetItemData(i));
		m_list.SetItemText(i, COL_NEWNAME, strExt);
	}
	if (bToggleRedraw == TRUE) m_list.SetRedraw(TRUE);
}

//nMode=0 : 클립보드로 파일명 저장, nMode=1 : 파일로 파일명 저장
//nMode=2 : 클립보드로 경로명 저장, nMode=3 : 파일로 경로명 저장
void CBatchNamerDlg::Export(int nMode)
{
	CString strData;
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		if (strData.IsEmpty() == FALSE) strData += _T("\r\n");
		if (nMode == 0 || nMode == 1)
		{
			strData += m_list.GetItemText(i, COL_NEWNAME);
		}
		else if (nMode == 2 || nMode == 3)
		{
			strData += m_list.GetNewPath(i);
		}
	}

	if (nMode == 0 || nMode == 2)
	{
		CEdit edit;
		edit.Create(ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, CRect(0, 0, 0, 0), this, 38111);
		edit.SetWindowText(strData);
		edit.SetSel(0, -1);
		edit.Copy();
		edit.SetWindowText(_T(""));
	}
	else if (nMode == 1 || nMode == 3)
	{
		OPENFILENAME ofn = { 0 };
		CString strTitle;
		if (nMode == 1)	strTitle.LoadString(IDS_EXPORTNAME); //_T("이름 목록 저장");
		else			strTitle.LoadString(IDS_EXPORTFULLPATH); //_T("전체경로 목록 저장");
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = this->GetSafeHwnd();
		ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_ENABLESIZING;
		ofn.lpstrTitle = strTitle;
		ofn.lpstrFilter = _T("Text Files(*.txt)\0*.txt\0All Files(*.*)\0*.*\0\0");
		ofn.nMaxFile = MY_MAX_PATH;
		ofn.lpstrDefExt = _T("txt");
		TCHAR pBuf[MY_MAX_PATH] = { 0 };
		ofn.lpstrFile = pBuf;
		if (GetSaveFileName(&ofn) != FALSE)
		{
			WriteCStringToFile(ofn.lpstrFile, strData);
		}
	}
}

//파일명을 텍스트 파일 또는 클립보드에서 읽어와서 차례대로 새로 바뀔 이름인 COL_NEWNAME 을 수정한다
//텍스트 파일은 이름 하나당 엔터(\n)로 구분해서 저장한 타입
//현재 리스트에 있는 개수만큼 읽어온다
void CBatchNamerDlg::ImportNewName(BOOL bFromFile)
{
	CString strImportData, strName;
	if (bFromFile == TRUE) // 파일에서 읽기
	{
		OPENFILENAME ofn = { 0 };
		CString strTitle;
		if (strTitle.LoadString(IDS_IMPORTNAME) == FALSE) strTitle.Empty();
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = this->GetSafeHwnd();
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ENABLESIZING;
		ofn.lpstrTitle = strTitle;
		ofn.lpstrFilter = _T("Text Files(*.txt)\0*.txt\0All Files(*.*)\0*.*\0\0");
		ofn.nMaxFile = MY_MAX_PATH;
		ofn.lpstrDefExt = _T("txt");
		TCHAR pBuf[MY_MAX_PATH] = { 0 };
		ofn.lpstrFile = pBuf;
		if (GetOpenFileName(&ofn) != FALSE)
		{
			ReadFileToCString(ofn.lpstrFile, strImportData);
		}
	}
	else // 클립보드에서 읽기
	{
		UINT priority_list[] = { CF_UNICODETEXT , CF_HDROP };
		UINT current_cf = GetPriorityClipboardFormat(priority_list, 2);
		if (current_cf == CF_HDROP) // 파일을 복사한 경우 => 이름을 추출해서 붙여넣는다
		{
			if (OpenClipboard())
			{
				HGLOBAL hGlobal = (HGLOBAL)GetClipboardData(CF_HDROP);
				if (hGlobal)
				{
					HDROP hDropInfo = (HDROP)GlobalLock(hGlobal);
					strImportData = ProcessDropFiles(hDropInfo, DF_READNEWNAME);
					GlobalUnlock(hGlobal);
				}
				CloseClipboard();
			}
		}
		else if (current_cf == CF_UNICODETEXT) // 파일명 텍스트를 복사한 경우
		{
			if (OpenClipboard())
			{
				HANDLE hText = GetClipboardData(CF_UNICODETEXT);
				if (hText)
				{
					strImportData = (TCHAR*)GlobalLock(hText);
					GlobalUnlock(hText);
				}
				CloseClipboard();
			}
		}
		else return;
	}

	int nPos = 0;
	int i = 0;
	m_list.SetRedraw(FALSE);
	while (nPos != -1)
	{
		nPos = GetLine(strImportData, nPos, strName, _T("\n"));
		strName.Trim();
		if (strName.Find(L'\\')) strName = Get_Name(strName, TRUE);
		if (i >= m_list.GetItemCount()) break; //리스트 넘어가면 끝
		m_list.SetItemText(i, COL_NEWNAME, strName); 
		i++;
	}
	m_list.SetRedraw(TRUE);
}

//경로명을 클립보드 또는 텍스트 파일에서 읽어와서 현재 목록 뒤에 추가한다
//텍스트 파일은 이름 하나당 엔터(\n)로 구분해서 저장한 타입
void CBatchNamerDlg::ImportPath(BOOL bFromFile)
{
	CString strImportData;
	if (bFromFile == TRUE) // 파일에서 읽기
	{
		OPENFILENAME ofn = { 0 };
		CString strTitle;
		if (strTitle.LoadString(IDS_IMPORT_PATH) == FALSE) strTitle.Empty();
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = this->GetSafeHwnd();
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ENABLESIZING;
		ofn.lpstrTitle = strTitle;
		ofn.lpstrFilter = _T("Text Files(*.txt)\0*.txt\0All Files(*.*)\0*.*\0\0");
		ofn.nMaxFile = MY_MAX_PATH;
		ofn.lpstrDefExt = _T("txt");
		TCHAR pBuf[MY_MAX_PATH] = { 0 };
		ofn.lpstrFile = pBuf;
		if (GetOpenFileName(&ofn) != FALSE)
		{
			ReadFileToCString(ofn.lpstrFile, strImportData);
		}
	}
	else // 클립보드에서 읽기
	{
		UINT priority_list[] = { CF_UNICODETEXT , CF_HDROP };
		UINT current_cf = GetPriorityClipboardFormat(priority_list, 2);
		if (current_cf == CF_HDROP) // 파일을 복사한 경우
		{
			if (OpenClipboard()) 
			{
				HGLOBAL hGlobal = (HGLOBAL)GetClipboardData(CF_HDROP);
				if (hGlobal)
				{
					HDROP hDropInfo = (HDROP)GlobalLock(hGlobal);
					ProcessDropFiles(hDropInfo, DF_ADDLIST);
					GlobalUnlock(hGlobal);
				}
				CloseClipboard();
			}
			return; // 이경우에는 ProcessDropFiles 안에서 리스트에 추가해 주기 때문에 바로 리턴하면 됨
		}
		else if (current_cf == CF_UNICODETEXT) // 파일명 텍스트를 복사한 경우
		{
			if (OpenClipboard())
			{
				HANDLE hText = GetClipboardData(CF_UNICODETEXT);
				if (hText)
				{
					strImportData = (TCHAR*)GlobalLock(hText);
					GlobalUnlock(hText);
				}
				CloseClipboard();
			}
		}
		else return;
	}
	//한줄씩 잘라서 CStringArray로 만든다
	CString strPath;
	int nPos = 0;
	CStringArray aPath;
	while (nPos != -1)
	{
		nPos = GetLine(strImportData, nPos, strPath, _T("\n"));
		strPath.Trim();
		aPath.Add(strPath);
	}
	//목록에 추가한다
	LoadPathArray(aPath);
}

//파일이 저장된 폴더 변경. 일종의 MoveFile이 됨. 중복 체크 필요
void CBatchNamerDlg::NameSetFolder(int nSubCommand, CString str1, CString str2)
{
	//str1에 값이 들어간 경우에는 지정된 폴더로 이동
	if (nSubCommand == IDS_FOLDER_SPECIFIC)
	{
		CString strPath = str1;
		//e:\ 같은 경우를 대비하여 끝에 오는 \를 삭제
		if (strPath.GetAt(strPath.GetLength() - 1) == _T('\\')) strPath.Delete(strPath.GetLength() - 1);
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			//COL_FOLDER 변경해 주면 나중에 실제 변경때 이값이 COL_NEWNAME과 결합되면서 새 경로가 된다
			m_list.SetItemText(i, COL_NEWFOLDER, strPath);
		}
	}
	else if (nSubCommand == IDS_FOLDER_PARENT)
	{
		if (str1.IsEmpty() == TRUE && str2.IsEmpty() == FALSE) str1 = str2;
		int nLevel = _ttoi(str1);
		CString strNewFolder, strTemp;
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			strNewFolder = m_list.GetItemText(i, COL_OLDFOLDER);
			for (int j = 0; j < nLevel; j++)
			{
				strTemp = Get_Folder(strNewFolder);
				if (strTemp.IsEmpty()) break; //더이상 상위 폴더가 없으면 중단
				strNewFolder = strTemp;
			}
			m_list.SetItemText(i, COL_NEWFOLDER, strNewFolder);
		}
	}
	else if (nSubCommand == IDS_FOLDER_PATTERN)
	{
		if (str1.IsEmpty()) return;
		if (str2.IsEmpty()) str2 = str1;
		CString strName, strFolder, strTemp;
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			BOOL bIsDir = (BOOL)m_list.GetItemData(i);
			strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
			strTemp = ReplaceWithWildCards(strName, str1, str2, TRUE);
			if (strTemp.IsEmpty() == FALSE)
			{
				strFolder = m_list.GetItemText(i, COL_NEWFOLDER) + L'\\' + strTemp;
				m_list.SetItemText(i, COL_NEWFOLDER, strFolder);
			}
		}
	}
	else if (nSubCommand == IDS_FOLDER_BRACKET)
	{
		if (str1.IsEmpty() || str2.IsEmpty()) return;
		if (str1.GetLength() > 1 || str2.GetLength() > 1) return;
		TCHAR c1 = str1.GetAt(0);
		TCHAR c2 = str2.GetAt(0);
		CString strName, strFolder; //strTemp
		int nStart, nEnd;
		//int n1, n2;
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			BOOL bIsDir = (BOOL)m_list.GetItemData(i);
			strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
			if (FindBracketPart(strName, c1, c2, nStart, nEnd) == TRUE)
			{
				strFolder = m_list.GetItemText(i, COL_NEWFOLDER) + L'\\' + strName.Mid(nStart, nEnd - nStart + 1);
				m_list.SetItemText(i, COL_NEWFOLDER, strFolder);
			}
		}
	}
	else if (nSubCommand == IDS_FOLDER_POS || nSubCommand == IDS_FOLDER_POS_REVERSE)
	{
		int nStart = _ttoi(str1);
		int nEnd = _ttoi(str2);
		if (nStart == 0 && nEnd == 0) return;
		if (nStart > 0) nStart--; //첫번째 문자의 인덱스는 0
		if (nEnd > 0 && nStart > nEnd) return; //nEnd가 입력된 상태에서 시작이 끝보다 뒤일때
		if (nEnd > 0)	nEnd--; //첫번째 문자의 인덱스는 0
		else			nEnd = -1; //항상 끝까지 
		CString strName, strFolder, strTemp;
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			BOOL bIsDir = (BOOL)m_list.GetItemData(i);
			strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
			if (nSubCommand == IDS_FOLDER_POS_REVERSE) strName = strName.MakeReverse();
			if (nEnd == -1) strTemp = strName.Mid(nStart);
			else strTemp = strName.Mid(nStart, nEnd - nStart + 1);
			if (strTemp.IsEmpty() == FALSE)
			{
				if (nSubCommand == IDS_FOLDER_POS_REVERSE) strTemp = strTemp.MakeReverse();
				strFolder = m_list.GetItemText(i, COL_NEWFOLDER) + L'\\' + strTemp;
				m_list.SetItemText(i, COL_NEWFOLDER, strFolder);
			}
		}
	}
	else if (nSubCommand == IDS_FOLDER_DATECREATE)
	{
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			CString strFolder = m_list.GetItemText(i, COL_NEWFOLDER) + L'\\' + FormatTimeString(m_list.GetItemText(i, COL_TIMECREATE), str1);
			m_list.SetItemText(i, COL_NEWFOLDER, strFolder);
		}
	}
	else if (nSubCommand == IDS_FOLDER_DATEMODIFY)
	{
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			CString strFolder = m_list.GetItemText(i, COL_NEWFOLDER) + L'\\' + FormatTimeString(m_list.GetItemText(i, COL_TIMEMODIFY), str1);
			m_list.SetItemText(i, COL_NEWFOLDER, strFolder);
		}
	}
	else if (nSubCommand == IDS_FOLDER_EXT)
	{
		CString strFolder, strTemp;
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			BOOL bIsDir = (BOOL)m_list.GetItemData(i);
			if (bIsDir == FALSE)
			{
				strTemp = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), FALSE, FALSE);
				if (strTemp.IsEmpty() == FALSE)
				{
					strFolder = m_list.GetItemText(i, COL_NEWFOLDER) + L'\\' + strTemp;
					m_list.SetItemText(i, COL_NEWFOLDER, strFolder);
				}
			}
		}
	}

}


void CBatchNamerDlg::NameSetFolder()
{
	CDlgInput dlg;
	dlg.InitInputByCommand(IDS_TB_16);
	if (dlg.DoModal() == IDCANCEL) return;
	if (dlg.VerifyReturnValue() == FALSE) return;
	m_list.SetRedraw(FALSE);
	NameSetFolder(dlg.GetSubCommand(), dlg.m_strReturn1, dlg.m_strReturn2);
	m_list.SetRedraw(TRUE);
}

BOOL FindBracketPart(CString& strSrc, TCHAR c1, TCHAR c2, int& nStart, int& nEnd)
{
	int n1, n2;
	nStart = -1; nEnd = strSrc.GetLength() - 1;
	for (n1 = 0; n1 <= nEnd; n1++)
	{
		if (strSrc.GetAt(n1) == c1) { nStart = n1; n1++; break; }
	}
	for (n2 = n1; n2 <= nEnd; n2++)
	{
		if (strSrc.GetAt(n2) == c2) { nEnd = n2; break; }
	}
	if (nStart != -1 && nStart < nEnd && nEnd == n2) return TRUE;
	return FALSE;
}

void CBatchNamerDlg::NameRemoveSelected(int nSubCommand, CString str1, CString str2)
{
	CString strName, strExt, strNameReverse;
	if (nSubCommand == IDS_DELPOS_FRONT || nSubCommand == IDS_DELPOS_REAR
		|| nSubCommand == IDS_DELPOS_FRONT_INVERT || nSubCommand == IDS_DELPOS_REAR_INVERT )
	{
		// 입력값의 종류
		// 1) 모두 비어 있거나 값이 0일때 : 작동하지 않음
		// 2) 시작값이 비어 있을때 또는 0이 입력되어 있을때 : 첫 글자부터
		// 3) 끝값이 비어 있을때 또는 0이 입력되어 있을때: 마지막 글자까지
		// 4) 값이 두개 다 있을 때 : 1부터 시작하는 위치이므로 1씩 빼서 계산
		int nStart = _ttoi(str1);
		int nEnd = _ttoi(str2);
		int nEnd_Current = 0;
		if (nStart == 0 && nEnd == 0) return;
		if (nStart > 0) nStart--;
		if (nEnd > 0 && nStart > nEnd) return; //nEnd가 입력된 상태에서 시작이 끝보다 뒤일때
		if (nEnd >= 0) nEnd--;
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			BOOL bIsDir = (BOOL)m_list.GetItemData(i);
			strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
			strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
			if (nEnd < 0) nEnd_Current = strName.GetLength(); //nEnd 는 0일 경우 길이에 따라 변한다.
			else nEnd_Current = nEnd;
			if (nSubCommand == IDS_DELPOS_FRONT)	//앞의 n부터 m까지
			{
				strName.Delete(nStart, nEnd_Current - nStart + 1);
			}
			else if (nSubCommand == IDS_DELPOS_REAR) //뒤의 n개
			{
				strNameReverse = strName.MakeReverse();
				strNameReverse.Delete(nStart, nEnd_Current - nStart + 1);
				strName = strNameReverse.MakeReverse();
			}
			else if (nSubCommand == IDS_DELPOS_FRONT_INVERT)
			{
				strName = strName.Mid(nStart, nEnd_Current - nStart + 1);
			}
			else if (nSubCommand == IDS_DELPOS_REAR_INVERT)
			{
				strNameReverse = strName.MakeReverse();
				strNameReverse = strNameReverse.Mid(nStart, nEnd_Current - nStart + 1);
				strName = strNameReverse.MakeReverse();
			}
			else break;
			if (strExt.IsEmpty() == FALSE) strName += strExt;
			m_list.SetItemText(i, COL_NEWNAME, strName);
		}
	}
	else if (nSubCommand == IDS_REMOVEBYBRACKET || nSubCommand == IDS_REMOVEBYBRACKET_INVERT)
	{
		if (str1.IsEmpty() || str2.IsEmpty()) return;
		if (str1.GetLength() > 1 || str2.GetLength() > 1) return;
		TCHAR c1 = str1.GetAt(0);
		TCHAR c2 = str2.GetAt(0);
		CString strName, strExt;
		int nStart, nEnd;
		//int n1, n2;
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			BOOL bIsDir = (BOOL)m_list.GetItemData(i);
			strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
			strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
			if (FindBracketPart(strName, c1, c2, nStart, nEnd) == TRUE)
			{
				if (nSubCommand == IDS_REMOVEBYBRACKET)	strName.Delete(nStart, nEnd - nStart + 1);
				else strName = strName.Mid(nStart, nEnd - nStart + 1);//IDS_REMOVEBYBRACKET_INVERT
				if (strExt.IsEmpty() == FALSE) strName += strExt;
				m_list.SetItemText(i, COL_NEWNAME, strName);
			}
		}
	}
	else if (nSubCommand == IDS_TRIM_BOTH)
	{
		CString strName, strExt, strTemp;
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			BOOL bIsDir = (BOOL)m_list.GetItemData(i);
			strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
			strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir, TRUE);
			strTemp = strName;
			strTemp.Trim();
			if (strTemp.GetLength() != strName.GetLength())
			{
				if (strExt.IsEmpty() == FALSE) strTemp += strExt;
				m_list.SetItemText(i, COL_NEWNAME, strTemp);
			}
		}
	}
}

void CBatchNamerDlg::NameRemoveSelected()
{
	CDlgInput dlg;
	dlg.InitInputByCommand(IDS_TB_05);
	if (dlg.DoModal() == IDCANCEL) return;
	if (dlg.VerifyReturnValue() == FALSE) return;
	m_list.SetRedraw(FALSE);
	NameRemoveSelected(dlg.GetSubCommand(), dlg.m_strReturn1, dlg.m_strReturn2);
	m_list.SetRedraw(TRUE);
}

//리스트를 정해진 기준에 따라 정렬한다
void CBatchNamerDlg::SortList()
{
	CDlgSort dlg;
	dlg.m_pSortWnd = &m_list; 
	if (dlg.DoModal() == IDCANCEL) return;
	APP()->m_nSortCol = dlg.m_nSortCol;
	APP()->m_bSortAscend = dlg.m_bSortAscend;
	m_list.SetRedraw(FALSE);
	m_list.Sort(APP()->m_nSortCol, APP()->m_bSortAscend);
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::OnDblclkListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	ManualChange();
	*pResult = 0;
}

void CBatchNamerDlg::UpdateToolBar()
{
	BOOL b = (m_list.GetItemCount() > 0);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_APPLY_CHANGE, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_REPLACE, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_ADD_FRONT, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_ADD_REAR, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_DIGIT, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_ADDNUM, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_EMPTY, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_REMOVESELECTED, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_EXTRACTNUMBER, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_REMOVENUMBER, b);

	m_tool2.GetToolBarCtrl().EnableButton(IDM_CLEAR_LIST, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_SORT_LIST, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_UNDO_CHANGE, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_NAME_SETFOLDER, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EXT_ADD, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EXT_DEL, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EXT_REPLACE, b);

	b = (m_list.GetNextItem(-1, LVNI_SELECTED) != -1);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_MANUAL_CHANGE, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EDIT_UP, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EDIT_DOWN, b);
}

void CBatchNamerDlg::UpdateMenuEnable()
{
	BOOL b = (m_list.GetItemCount() > 0);
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem(IDM_APPLY_CHANGE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_CLEAR_LIST, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_CLEAR_LIST_ALL, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_SORT_LIST, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_REMOVE_ITEM, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_UNDO_CHANGE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_NAME_REPLACE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_NAME_ADD_FRONT, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_NAME_ADD_REAR, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_NAME_REMOVESELECTED, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_NAME_EXTRACTNUMBER, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_NAME_REMOVENUMBER, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_NAME_DIGIT, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_NAME_ADDNUM, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_NAME_EMPTY, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_NAME_SETFOLDER, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_EXT_ADD, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_EXT_DEL, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_EXT_REPLACE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_EXPORT_CLIP_NEWNAME, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_IMPORT_CLIP_NEWNAME, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_EXPORT_FILE_NEWNAME, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_IMPORT_FILE_NEWNAME, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_EXPORT_CLIP_PATH, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_EXPORT_FILE_PATH, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	b = (m_list.GetNextItem(-1, LVNI_SELECTED) != -1);
	pMenu->EnableMenuItem(IDM_MANUAL_CHANGE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_EDIT_UP, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_EDIT_DOWN, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_REMOVE_ITEM, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	pMenu->EnableMenuItem(IDM_UNDO_SELECTED, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);

	int aShowMenuID[] = { IDM_SHOW_OLDFOLDER, IDM_SHOW_NEWFOLDER, IDM_SHOW_SIZE, IDM_SHOW_MODIFYTIME, IDM_SHOW_CREATETIME, IDM_SHOW_FULLPATH };
	int nWidth = 0;
	//UpdateColumnSizes();
	for (int i = 0; i < 6; i++)
	{
		nWidth = m_list.GetColumnWidth(COL_OLDFOLDER + i);
		pMenu->CheckMenuItem(aShowMenuID[i], (nWidth != 0) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
	}
}

void CBatchNamerDlg::UpdateMenuPreset()
{
	CMenu* pMenu = GetMenu();
	PresetArray& aPS = APP()->m_aPreset;
	CString strTemp;
	int n = 0;
	for (int i = 0; i < aPS.GetSize(); i++) //프리셋은 5개로 고정, 대신 프리셋 5개 묶음을 파일로 저장하고 불러올 수 있도록 함
	{
		BatchNamerPreset& ps = aPS[i];
		if (ps.m_strName.IsEmpty()) strTemp.Format(IDSTR(IDS_PRESET_MENU_FORMAT), i + 1, IDSTR(IDS_PRESET_NONAME), i + 1);
		else strTemp.Format(IDSTR(IDS_PRESET_MENU_FORMAT), i + 1, ps.m_strName, i + 1);;
		pMenu->ModifyMenu(IDM_PRESET_APPLY1 + i, MF_BYCOMMAND | MF_STRING, IDM_PRESET_APPLY1 + i, strTemp);
		pMenu->EnableMenuItem(IDM_PRESET_APPLY1 + i, (ps.m_aTask.GetSize() > 0) ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	}
}

void CBatchNamerDlg::UpdateMenuHotkey()
{
	CMenu* pMenu = GetMenu();
	// 단축키를 메뉴에 표시하기
	CHotKeyMap& hkm = APP()->m_mapHotKey;
	CHotKeyMap::iterator i;
	CString strCmd;
	int nPos = -1;
	for (i = hkm.begin(); i != hkm.end(); i++)
	{
		int nCommand = i->first;
		HotKey hk = i->second;
		strCmd.Empty();
		pMenu->GetMenuString(nCommand, strCmd, MF_BYCOMMAND);
		nPos = strCmd.Find(L'\t');
		if (nPos != -1) strCmd = strCmd.Left(nPos);
		strCmd += (L'\t');
		strCmd += hk.GetKeyString();
		pMenu->ModifyMenu(nCommand, MF_BYCOMMAND | MF_STRING, nCommand, strCmd);
	}
}

void CBatchNamerDlg::UpdateCount()
{
	int nCount = m_list.GetItemCount();
	CString strTemp;
	if (nCount == 0)
	{
		strTemp = IDSTR(IDS_PLEASE_ADD);
	}
	else strTemp.Format(IDSTR(IDS_COUNT_FORMAT), nCount);
	UpdateToolBar();
	UpdateMenuEnable();
	SetDlgItemText(IDC_ST_BAR, strTemp);
}

void CBatchNamerDlg::UpdateColumnSizes()
{
	if (APP()->m_aColWidth.GetSize() != m_list.GetHeaderCtrl().GetItemCount())
	{
		APP()->m_aColWidth.RemoveAll();
		APP()->m_aColWidth.SetSize(m_list.GetHeaderCtrl().GetItemCount());
		for (int i = 0; i < APP()->m_aColWidth.GetSize(); i++)
		{
			APP()->m_aColWidth.SetAt(i, m_lfHeight * 11);
		}
	}
	int nColWidth = 0;
	for (int i = 0; i < APP()->m_aColWidth.GetSize(); i++)
	{
		nColWidth = m_list.GetColumnWidth(i);
		if (nColWidth > 0)
		{
			FlagSET(APP()->m_nShowFlag, i, TRUE);
			APP()->m_aColWidth.SetAt(i, nColWidth);
		}
		else
		{
			FlagSET(APP()->m_nShowFlag, i, FALSE);
			if (APP()->m_aColWidth.GetAt(i) == 0) APP()->m_aColWidth.SetAt(i, m_lfHeight * 11);
		}
	}

	CMFCHeaderCtrl& header = m_list.GetHeaderCtrl();
	int nCount = header.GetItemCount();
	APP()->m_nSortCol = header.GetSortColumn();
	APP()->m_bSortAscend = header.IsAscending();
}

void CBatchNamerDlg::UpdateFontSize()
{
	int nFontSize = APP()->m_nFontSize;
	if (APP()->m_bUseDefaultFont == TRUE) nFontSize = m_nDefault_FontSize;
	CFont* pFont = m_list.GetFont();
	LOGFONT lf;
	pFont->GetLogFont(&lf);
	lf.lfHeight = -1 * MulDiv(nFontSize, GetDeviceCaps(GetDC()->GetSafeHdc(), LOGPIXELSY), 72);
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&lf); //자동 소멸되지 않도록 멤버 변수 사용
	m_list.SetFont(&m_font);
}


void CBatchNamerDlg::OnBnClickedBtnStopthread()
{
	if (AfxMessageBox(IDSTR(IDS_MSG_STOPTHREAD), MB_YESNO) == IDNO) return;
	st_bIsIdle = TRUE;
}


void CBatchNamerDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 400;
	lpMMI->ptMinTrackSize.y = 300;
	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CBatchNamerDlg::OnInitMenu(CMenu* pMenu)
{
	CDialogEx::OnInitMenu(pMenu);
	UpdateMenuEnable();
}
