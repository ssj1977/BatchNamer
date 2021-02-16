
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
#include "EtcFunctions.h"
#include "CDlgCFG_Load.h"
#include "CDlgCFG_View.h"
#include "CDlgInput.h"
#pragma warning(disable:4786)
#include <map>
#include <vector>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef vector<CString> CStrArray;
typedef map<CString, int> CExtMap; //확장자에 해당하는 이미지맵의 번호를 기억
static CExtMap mapExt;

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

inline CString GetTimeStringToAdd(FILETIME& dt)
{
	CString strRet;
	SYSTEMTIME systime;
	FileTimeToSystemTime(&dt, &systime);
	strRet.Format(_T("%d%02d%02d_%02d%02d%02d"), systime.wYear,
		systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
	return strRet;
}

inline CString GetTimeStringToAdd(CString strTime)
{
	strTime.Remove(_T(':'));
	strTime.Remove(_T('-'));
	strTime.Replace(_T(' '), _T('_'));
	return strTime;
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
		return GetFileImageIndex(_T(""));
		//return SI_FOLDER_OPEN;
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



// CBatchNamerDlg 대화 상자



CBatchNamerDlg::CBatchNamerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BATCHNAMER, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bSelected = FALSE; //목록 중 항목 선택 여부 감지
	m_nDefault_FontSize = 12;
	m_clrDefault_Bk = RGB(255, 255, 255);
	m_clrDefault_Text = RGB(0, 0, 0);
	m_lfHeight = 0;
	m_pSysImgList = NULL;
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
END_MESSAGE_MAP()


// CBatchNamerDlg 메시지 처리기

BOOL CBatchNamerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

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
	m_list.SetColumnWidth(COL_OLDNAME, HU * 16 * FlagGET(APP()->m_nShowFlag, COL_OLDNAME));
	m_list.SetColumnWidth(COL_NEWNAME, HU * 16 * FlagGET(APP()->m_nShowFlag, COL_NEWNAME));
	m_list.SetColumnWidth(COL_OLDFOLDER, HU * 11 * FlagGET(APP()->m_nShowFlag, COL_OLDFOLDER));
	m_list.SetColumnWidth(COL_NEWFOLDER, HU * 11 * FlagGET(APP()->m_nShowFlag, COL_NEWFOLDER));
	m_list.SetColumnWidth(COL_FILESIZE, HU * 11 * FlagGET(APP()->m_nShowFlag, COL_FILESIZE));
	m_list.SetColumnWidth(COL_TIMEMODIFY, HU * 11 * FlagGET(APP()->m_nShowFlag, COL_TIMEMODIFY));
	m_list.SetColumnWidth(COL_TIMECREATE, HU * 11 * FlagGET(APP()->m_nShowFlag, COL_TIMECREATE));
	m_list.SetColumnWidth(COL_FULLPATH, HU * 11 * FlagGET(APP()->m_nShowFlag, COL_FULLPATH));
	m_list.GetHeaderCtrl().SetSortColumn(COL_OLDNAME, TRUE);
	UpdateMenu();
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
	CDialogEx::OnOK();
}

void CBatchNamerDlg::OnCancel()
{
	ShowWindow(SW_SHOWNORMAL);
	GetWindowRect(APP()->m_rcMain);
	ClearList();
	mapExt.clear();
	CDialogEx::OnCancel();
}

void CBatchNamerDlg::ArrangeCtrl()
{
	CRect rcButton, rcSplit;
	m_tool1.GetToolBarCtrl().GetItemRect(0, rcButton);
	m_tool1.GetToolBarCtrl().GetItemRect(1, rcSplit);
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
	GetDlgItem(IDC_ST_BAR)->MoveWindow(0, rc.bottom - BARHEIGHT + 1, rc.Width(), BARHEIGHT - 2);
	RedrawWindow();
}

void CBatchNamerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (::IsWindow(m_list.GetSafeHwnd()) != FALSE) ArrangeCtrl();
}

BOOL CBatchNamerDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDM_CLEAR_LIST:		ClearList();		break;
	case IDM_UNDO_CHANGE:		UndoChanges();		break;
	case IDM_SORT_LIST:			SortList();		break;
	case IDM_NAME_REPLACE:		NameReplace();	break;
	case IDM_NAME_ADD_FRONT:	NameAdd(TRUE);	break;
	case IDM_NAME_ADD_REAR:		NameAdd(FALSE);	break;
	case IDM_NAME_DEL_TYPE:		NameDelType();	break;
	case IDM_NAME_DIGIT:		NameDigit();		break;
	case IDM_EXT_ADD:			ExtAdd();			break;
	case IDM_EXT_DEL:			ExtDel();			break;
	case IDM_EXT_REPLACE:		ExtReplace();		break;
	case IDM_MANUAL_CHANGE:		ManualChange();	break;
	case IDM_APPLY_CHANGE:		ApplyChange();	break;
	case IDM_NAME_ADDNUM:		NameAddNum();		break;
	case IDM_NAME_EMPTY:		NameEmpty();		break;
	case IDM_EDIT_UP:			ListUp();			break;
	case IDM_EDIT_DOWN:			ListDown();		break;
	case IDM_NAME_SAMEPATH:		NameSamePath();	break;

	case IDM_NAME_DELPOS:		NameDelPos();		break;
	case IDM_NAME_DELTOKEN:		NameDelToken();	break;

	case IDM_EXPORT_CLIP:		Export(0);		break;
	case IDM_EXPORT_FILE:		Export(1);		break;
	case IDM_EXPORT_CLIP2:		Export(2);		break;
	case IDM_EXPORT_FILE2:		Export(3);		break;
	case IDM_IMPORT_FILE:		ImportName();		break;
	case IDM_IMPORT_FILE2:		ImportPath();		break;

	case IDM_LIST_ADD:			AddByFileDialog(); break;
	case IDM_SHOW_SIZE:			ToggleListColumn(COL_FILESIZE); break;
	case IDM_SHOW_MODIFYTIME:	ToggleListColumn(COL_TIMEMODIFY); break;
	case IDM_SHOW_CREATETIME:	ToggleListColumn(COL_TIMECREATE); break;
	case IDM_SHOW_OLDFOLDER:	ToggleListColumn(COL_OLDFOLDER); break;
	case IDM_SHOW_NEWFOLDER:	ToggleListColumn(COL_NEWFOLDER); break;
	case IDM_SHOW_FULLPATH:		ToggleListColumn(COL_FULLPATH); break;

	case IDM_VERSION: AfxMessageBox(_T("BatchNamer v1.0 (2021-02-21 Release)")); 	break;
	case IDM_CFG_LOAD: ConfigLoadType(); break;
	case IDM_CFG_VIEW: ConfigViewOption(); break;
	default:
		return CDialogEx::OnCommand(wParam, lParam);
	}
	UpdateMenu();
	return TRUE;
}

void CBatchNamerDlg::ToggleListColumn(int nCol)
{
	FlagSET(APP()->m_nShowFlag, nCol, !FlagGET(APP()->m_nShowFlag, nCol));
	m_list.SetColumnWidth(nCol, m_lfHeight * 11 * FlagGET(APP()->m_nShowFlag, nCol));
}

BOOL CBatchNamerDlg::PreTranslateMessage(MSG* pMsg)
{
	//주로 단축키의 처리
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_DELETE)
		{
			int nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
			while (nItem != -1)
			{
				m_list.DeleteListItem(nItem);
				nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
			}
			UpdateCount();
			return TRUE;
		}
		// <>를 이용해 리스트상에서 이동 가능
		if (pMsg->wParam == 188) { ListUp(); return TRUE; }
		if (pMsg->wParam == 190) { ListDown(); return TRUE; }
	}

	if (pMsg->message == WM_KEYUP && (GetKeyState(VK_CONTROL) & 0xFF00) != 0)
	{
		if (pMsg->wParam == _T('O')) { AddByFileDialog(); return TRUE; }
		if (m_list.GetItemCount() > 0)
		{
			if (pMsg->wParam == _T('S')) { ApplyChange(); return TRUE; }
			else if (pMsg->wParam == _T('Z')) { UndoChanges(); return TRUE; }
			else if (pMsg->wParam == _T('L')) { ClearList(); return TRUE; }
			else if (pMsg->wParam == _T('A')) { SortList(); return TRUE; }
			else if (pMsg->wParam == _T('C'))
			{
				if ((GetKeyState(VK_SHIFT) & 0xFF00) != 0)	Export(2);
				else									Export(0);
				return TRUE;
			}
			else if (pMsg->wParam == _T('X'))
			{
				if ((GetKeyState(VK_SHIFT) & 0xFF00) != 0)	Export(3);
				else									Export(1);
				return TRUE;
			}
		}
		if (pMsg->wParam == _T('V'))
		{
			if ((GetKeyState(VK_SHIFT) & 0xFF00) != 0)	ImportPath();
			else if (m_list.GetItemCount() > 0)		ImportName();
			return TRUE;
		}
	}

	BOOL b = (m_list.GetNextItem(-1, LVNI_SELECTED) != -1);
	if (b != m_bSelected)
	{
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

void CBatchNamerDlg::AddListItem(CFileFind& find)
{
	CString strName, strFolder, strSize, strTimeCreate, strTimeModify;
	CString strPath = find.GetFilePath();
	//중복체크
	CPathSet::iterator it = m_list.m_setPath.find(strPath);
	if (it == m_list.m_setPath.end()) m_list.m_setPath.insert(strPath);
	else return; // 이미 존재하는 이름
	BOOL bIsDir = find.IsDirectory();
	int nImage = GetFileImageIndexFromMap(strPath, bIsDir);
	strName = find.GetFileName();
	strFolder = Get_Folder(strPath);
	CTime tTemp;
	find.GetCreationTime(tTemp);
	strTimeCreate = tTemp.Format(_T("%Y-%m-%d %H:%M:%S"));
	find.GetLastWriteTime(tTemp);
	strTimeModify = tTemp.Format(_T("%Y-%m-%d %H:%M:%S"));
	strSize = GetFileSizeString(find.GetLength());
	int nItem = m_list.InsertItem(m_list.GetItemCount(), find.GetFileName(), nImage);
	m_list.SetItemText(nItem, COL_NEWNAME, strName);
	m_list.SetItemText(nItem, COL_OLDFOLDER, strFolder);
	m_list.SetItemText(nItem, COL_NEWFOLDER, strFolder);
	m_list.SetItemText(nItem, COL_FILESIZE, strSize);
	m_list.SetItemText(nItem, COL_TIMEMODIFY, strTimeModify);
	m_list.SetItemText(nItem, COL_TIMECREATE, strTimeCreate);
	m_list.SetItemText(nItem, COL_FULLPATH, strPath);
	m_list.SetItemData(nItem, bIsDir);
}

void CBatchNamerDlg::AddPath(CString strPath, BOOL bIsDirectory)
{
	CFileFind find;
	int nLoadType = APP()->m_nLoadType;
	if (bIsDirectory) //폴더인 경우의 처리 
	{
		if (nLoadType == 1) //폴더 내의 파일들을 추가하기로 선택한 경우 
		{
			CString strName, strFolder, strSize, strTimeCreate, strTimeModify;
			BOOL b = find.FindFile(strPath + _T("\\*.*"));
			while (b)
			{
				b = find.FindNextFile();
				if (find.IsDots() == FALSE)
				{
					if (find.IsDirectory() == TRUE) //폴더인 경우 재귀호출
					{
						AddPath(find.GetFilePath(), find.IsDirectory());
					}
					else //폴더 내 파일
					{
						AddListItem(find);
					}
				}
			}
			find.Close();
			return; //복귀
		}
		//else if (nLoadType==0) {} //폴더 이름을 직접 추가하기로 선택한 경우는 단순 파일과 동일하게 처리		
	}
	BOOL b = find.FindFile(strPath);
	if (b)
	{
		find.FindNextFile();
		AddListItem(find);
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
	if (dlg.DoModal() == IDOK)
	{
		APP()->m_bShowEverytime = dlg.m_bShowEverytime;
		APP()->m_nLoadType = dlg.m_nLoadType;
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
		m_list.RedrawWindow();
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
	WORD cFiles;
	TCHAR szFilePath[_MAX_PATH];
	memset(szFilePath, 0, sizeof(szFilePath));
	CString strPath;
	cFiles = DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	if (APP()->m_bShowEverytime) ConfigLoadType();
	SetDlgItemText(IDC_ST_BAR, IDSTR(IDS_WORKING));
	clock_t timestart = clock();
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < cFiles; i++)
	{
		DragQueryFile(hDropInfo, i, szFilePath, sizeof(szFilePath));
		strPath = (LPCTSTR)szFilePath;
		AddPathStart(strPath);
		UpdateCount();
	}
	DragFinish(hDropInfo);
	//SetDlgItemText(IDC_ST_BAR, _T(""));
	m_list.Sort(m_list.GetHeaderCtrl().GetSortColumn(), m_list.GetHeaderCtrl().IsAscending());
	m_list.SetRedraw(TRUE);
	clock_t timeend = clock();
	UpdateMenu();
	double result = (double)(timeend - timestart);
	CString strTemp;
	strTemp.Format(_T("%f MSecs"), result);
	SetDlgItemText(IDC_ST_BAR, strTemp);
}

//파일 열기 시스템 다이얼로그를 사용해서 파일을 목록에 추가한다
void CBatchNamerDlg::AddByFileDialog()
{
	CFileDialog dlg(TRUE, _T("*.*"), NULL,
		OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_HIDEREADONLY, _T("All Files(*.*)|*.*||"), NULL);

	TCHAR* buf = new TCHAR[65536];
	memset(buf, 0, sizeof(buf));
	dlg.m_ofn.lpstrTitle = _T("이름 붙일 파일 불러오기");
	dlg.m_ofn.lpstrFile = buf;
	dlg.m_ofn.nMaxFile = 65536;

	if (dlg.DoModal() == IDOK)
	{
		if (APP()->m_bShowEverytime) ConfigLoadType();
		SetDlgItemText(IDC_ST_BAR, IDSTR(IDS_WORKING));
		CString strPath;
		POSITION pos = dlg.GetStartPosition();
		m_list.SetRedraw(FALSE);
		while (pos)
		{
			strPath = dlg.GetNextPathName(pos);
			AddPathStart(strPath);
		}
		SetDlgItemText(IDC_ST_BAR, _T(""));
		m_list.SetRedraw(TRUE);
	}
	delete[] buf;
}


//리스트 모두 삭제
void CBatchNamerDlg::ClearList()
{
	m_list.DeleteAllItems();
	m_bSelected = FALSE;
	m_list.m_setPath.clear();
	UpdateCount();
}

//바뀔 이름을 원래 이름으로 다시 복구
void CBatchNamerDlg::UndoChanges()
{
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		m_list.SetItemText(i, COL_NEWNAME, m_list.GetItemText(i, COL_OLDNAME));
	}
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::ManualChange()
{
	int n = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (n == -1) return;

	CDlgInput dlg;
	dlg.InitValue(m_list.GetItemText(n, COL_NEWNAME), _T(""));
	dlg.InitInputDlg(m_list.GetItemText(n, COL_NEWNAME) + _T(" 를"), _T("으로"), _T(""));
	if (dlg.DoModal() == IDOK)
	{
		m_list.SetItemText(n, COL_NEWNAME, dlg.m_strReturn1);
	}
}

void CBatchNamerDlg::NameReplace()
{
	CDlgInput dlg;
	dlg.InitInputDlg(IDSTR(IDS_TB_01),IDSTR(IDS_REPLACEOLD), IDSTR(IDS_REPLACENEW));
	dlg.AddOption(IDSTR(IDS_REPLACESTRING), INPUT_ONE);

	if (dlg.DoModal() == IDCANCEL) return;
	CString strTemp;
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		strTemp = m_list.GetItemText(i, COL_NEWNAME);
		strTemp.Replace(dlg.m_strReturn1, dlg.m_strReturn2);
		m_list.SetItemText(i, COL_NEWNAME, strTemp);
	}
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::NameAdd(BOOL bFront = TRUE)
{
	CDlgInput dlg;
	dlg.InitInputDlg(IDSTR(bFront ? IDS_TB_02 : IDS_TB_03), IDSTR(IDS_STRINGTOADD), _T(""));
	dlg.AddOption(IDSTR(IDS_ADDSTRING), INPUT_ONE);
	dlg.AddOption(IDSTR(IDS_ADDPARENT), INPUT_NONE);
	dlg.AddOption(IDSTR(IDS_ADDTIMECREATE), INPUT_NONE);
	dlg.AddOption(IDSTR(IDS_ADDTIMEMODIFY), INPUT_NONE);

	if (dlg.DoModal() == IDCANCEL) return;
	CString strTemp;
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		switch (dlg.m_nCB)
		{
		case 0: //직접 입력
			strTemp = dlg.m_strReturn1;
			break;
		case 1: //폴더명
			strTemp = GetFolderName(m_list.GetItemText(i, COL_OLDFOLDER));
			//c:, d: 등 드라이브 루트 경로인 경우 추가히지 않음
			if (strTemp.CompareNoCase(m_list.GetItemText(i, COL_OLDFOLDER)) == 0) strTemp.Empty();
			break;
		case 2: //변경일시
			strTemp = GetTimeStringToAdd(m_list.GetItemText(i, COL_TIMEMODIFY));
			break;
		case 3: //생성일시
			strTemp = GetTimeStringToAdd(m_list.GetItemText(i, COL_TIMECREATE));
			break;
		}
		if (bFront)
		{
			strTemp += m_list.GetItemText(i, COL_NEWNAME);
			m_list.SetItemText(i, COL_NEWNAME, strTemp);
		}
		else
		{
			BOOL bIsDir = m_list.GetItemData(i);
			CString strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir) + strTemp;
			CString	strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
			if (strExt.IsEmpty() == FALSE) strName += strExt;
			m_list.SetItemText(i, COL_NEWNAME, strName);
		}
	}
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::NameDelType()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("선택에 따라 숫자를 남기거나 삭제합니다."), _T(""), _T(""));
	dlg.AddOption(_T("숫자만 남기기"), INPUT_NONE);
	dlg.AddOption(_T("숫자를 모두 삭제하기"), INPUT_NONE);

	if (dlg.DoModal() == IDCANCEL) return;
	//이름에서 숫자만 남기기
	CString strName, strExt;
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		BOOL bIsDir = m_list.GetItemData(i);
		strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
		strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir);

		for (int j = strName.GetLength() - 1; j >= 0; j--)
		{
			if (dlg.m_nCB == 0)  // 숫자만 남기기
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
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::NameDigit()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("숫자부분의 자리수를 맞춰 0을 붙입니다."), _T("자리수"), _T(""));
	dlg.AddOption(_T("제일 뒷번호 맞춤"), INPUT_ONE);
	dlg.AddOption(_T("제일 앞번호 맞춤"), INPUT_ONE);

	if (dlg.DoModal() == IDCANCEL) return;
	int nDigit = _ttoi(dlg.m_strReturn1);
	if (nDigit <= 0)
	{
		AfxMessageBox(_T("자리수 입력이 잘못되었습니다."));
		return;
	}
	CString strName, strExt;
	int nStatus;
	int nStart;
	int nEnd;
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		BOOL bIsDir = m_list.GetItemData(i);
		strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
		strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
		nStatus = 0;
		nStart = -1;
		nEnd = -1;
		if (dlg.m_nCB == 0) //뒷번호
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
		else //if (dlg.m_nCB==1) //앞번호
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
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::ExtDel() //확장자 삭제
{
	CString strTemp;
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		strTemp = Get_Name(m_list.GetItemText(i, COL_NEWNAME), ((BOOL)m_list.GetItemData(i)));
		m_list.SetItemText(i, COL_NEWNAME, strTemp);
	}
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::ExtAdd()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("확장자를 뒤에 붙입니다."), _T("붙일 확장자"), _T(""));
	if (dlg.DoModal() == IDCANCEL) return;
	CString strTemp;
	CString strExt = dlg.m_strReturn1;
	if (strExt.IsEmpty()) return;
	if (strExt.GetAt(0) != _T('.')) strExt = _T(".") + strExt;
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		strTemp = m_list.GetItemText(i, COL_NEWNAME);
		strTemp += strExt;
		m_list.SetItemText(i, COL_NEWNAME, strTemp);
	}
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::ExtReplace()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("확장자를 바꿔 줍니다."), _T("바꿀 확장자"), _T(""));

	if (dlg.DoModal() == IDCANCEL) return;
	CString strTemp;
	CString strExt = dlg.m_strReturn1;
	if (strExt.IsEmpty()) return;
	if (strExt.GetAt(0) != _T('.')) strExt = _T(".") + strExt;
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		strTemp = Get_Name(m_list.GetItemText(i, COL_NEWNAME), (BOOL)m_list.GetItemData(i));
		strTemp += strExt;
		m_list.SetItemText(i, COL_NEWNAME, strTemp);
	}
	m_list.SetRedraw(TRUE);
}

//실제 파일 시스템상의 정보를 바꿔 파일 이름 변경하기
void CBatchNamerDlg::ApplyChange()
{
	if (AfxMessageBox(IDSTR(IDS_MSG_APPLYASK), MB_OKCANCEL) == IDCANCEL) return;
	CString strNewPath, strTemp, strLog;
	//선택된 부분 초기화
	int nItemSel = m_list.GetNextItem(-1, LVNI_SELECTED);
	while (nItemSel != -1)
	{
		m_list.SetItemState(nItemSel, 0, LVIS_SELECTED | LVIS_FOCUSED);
		nItemSel = m_list.GetNextItem(-1, LVNI_SELECTED);
	}
	//중복여부 체크 및 새로운 파일명 만들기
	CPathSet setNewPath;
	CPathSet::iterator it;
	CStrArray aNewPath;
	int nCount = m_list.GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		strTemp = m_list.GetItemText(i, COL_NEWNAME);
		if (strTemp.IsEmpty() == TRUE)
		{
			AfxMessageBox(IDSTR(IDS_MSG_NONAME));
			m_list.SetFocus();
			m_list.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			m_list.EnsureVisible(i, FALSE);
			return;
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
			strTemp.Format(_T("%s\n%s"), IDSTR(IDS_MSG_DUPNAME), strNewPath);
			AfxMessageBox(strTemp);
			m_list.SetFocus();
			m_list.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			m_list.EnsureVisible(i, FALSE);
			return;
		}
		aNewPath.push_back(strNewPath); // Array에 추가
	}
	if (aNewPath.size() != nCount) { AfxMessageBox(IDSTR(IDS_MSG_MISMATCH)); return; }

	//실제 파일이름을 바꾸는 곳
	m_list.SetRedraw(FALSE);
	CString strOldPath, strOldExt, strNewExt;
	int nImage = 0;
	BOOL bIsDir = FALSE;
	for (int i = 0; i < nCount; i++)
	{
		strOldPath = m_list.GetOldPath(i);
		bIsDir = (BOOL)m_list.GetItemData(i);
		try
		{
			if (aNewPath.at(i).CompareNoCase(strOldPath) != 0)
			{
				if (MoveFileExW(strOldPath, aNewPath[i], MOVEFILE_COPY_ALLOWED) == FALSE)
				{
					strTemp.Format(_T("%s -> %s %s\n"), strOldPath, aNewPath.at(i), IDSTR(IDS_MSG_CHANGEFAIL));
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
						CString strNewExt = Get_Ext(aNewPath.at(i), bIsDir);
						if (strOldExt.CompareNoCase(strNewExt) != 0)
						{
							nImage = GetFileImageIndexFromMap(aNewPath.at(i), bIsDir);
							m_list.SetItem(i, 0, LVIF_IMAGE, NULL, nImage, 0, 0, 0);
						}
					}
				}
			}
		}
		catch (CFileException* e)
		{
			e->ReportError();
			e->Delete();
			strTemp.Format(_T("%s -> %s %s\n"), strOldPath, aNewPath.at(i), IDSTR(IDS_MSG_CHANGEFAIL));
			strLog += strTemp;
		}
	}
	m_list.SetRedraw(TRUE);
	if (strLog.IsEmpty() == FALSE) AfxMessageBox(strLog);
	else
	{
		strTemp.Format(_T("%d %s\n"), nCount, IDSTR(IDS_MSG_CHANGEDONE));
		AfxMessageBox(strTemp);
	}
}

//두개의 아이템 교환
void CBatchNamerDlg::SwapItem(int n1, int n2)
{
	CString str1, str2;
	int nCol = COL_TOTAL, i = 0;

	//아이템 데이타 교환
	DWORD dw1 = m_list.GetItemData(n1);
	DWORD dw2 = m_list.GetItemData(n2);
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

void CBatchNamerDlg::NameAddNum()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("붙일 숫자의 자리수와 시작값을 지정합니다."), _T("자리수"), _T("시작값"));
	dlg.AddOption(_T("이름뒤에 번호붙임"), INPUT_TWO);
	dlg.AddOption(_T("이름앞에 번호붙임"), INPUT_TWO);
	dlg.AddOption(_T("폴더별로 뒤 번호붙임"), INPUT_TWO);
	dlg.AddOption(_T("폴더별로 앞 번호붙임"), INPUT_TWO);

	if (dlg.DoModal() == IDCANCEL) return;
	int nDigit = _ttoi(dlg.m_strReturn1);
	int nStart = _ttoi(dlg.m_strReturn2);
	if (nDigit <= 0)
	{
		AfxMessageBox(_T("자리수 입력이 잘못되었습니다."));
		return;
	}
	if (nStart <= 0) nStart = 0;
	CString strName, strExt, strTemp;
	int nCurrent = nStart;
	int nCount = m_list.GetItemCount();
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < nCount; i++)
	{
		if (dlg.m_nCB > 1 && i > 0) //dlg.m_nCB==2 || dlg.m_nCB==3 
		{
			//폴더별 번호 붙이기 기능
			if (m_list.GetItemText(i - 1, COL_OLDFOLDER).CompareNoCase(m_list.GetItemText(i, COL_OLDFOLDER)) != 0) nCurrent = nStart;
		}
		BOOL bIsDir = (BOOL)m_list.GetItemData(i);
		strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
		strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
		strTemp.Format(_T("%d"), nCurrent);
		while (nDigit > strTemp.GetLength()) strTemp = _T('0') + strTemp;
		if (dlg.m_nCB == 0 || dlg.m_nCB == 2)	strName += strTemp; //뒤에 붙이기
		else								strName = strTemp + strName; //앞에 붙이기
		if (strExt.IsEmpty() == FALSE) strName += strExt;
		m_list.SetItemText(i, COL_NEWNAME, strName);

		nCurrent++;
	}
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::NameEmpty() //바꿀 이름 부분을 확장자만 제외하고 모두 지운다
{
	CString strExt;
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), (BOOL)m_list.GetItemData(i));
		m_list.SetItemText(i, COL_NEWNAME, strExt);
	}
	m_list.SetRedraw(TRUE);
}

//nMode=0 : 클립보드로 파일명 저장, nMode=1 : 파일로 파일명 저장
//nMode=2 : 클립보드로 경로명 저장, nMode=3 : 파일로 경로명 저장
void CBatchNamerDlg::Export(int nMode)
{
	CString strData;
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		if (nMode == 0 || nMode == 1)
		{
			strData += m_list.GetItemText(i, COL_NEWNAME) + _T("\r\n");
		}
		else if (nMode == 2 || nMode == 3)
		{
			strData += m_list.GetNewPath(i);
			strData += _T("\r\n");
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
		CFileDialog dlg(FALSE, _T("txt"), NULL, OFN_ENABLESIZING | OFN_LONGNAMES | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, _T("Text Files(*.txt)|*.txt|All Files(*.*)|*.*||"), NULL);

		if (nMode == 1)	dlg.m_ofn.lpstrTitle = _T("파일명 저장");
		else			dlg.m_ofn.lpstrTitle = _T("경로명 저장"); //nMode==3
		if (dlg.DoModal() == IDCANCEL) return;
		WriteCStringToFile(dlg.GetPathName(), strData);
	}
}

//파일명을 텍스트 파일에서 읽어와서 차례대로 새로 바뀔 이름인 COL_NEWNAME 을 수정한다
//텍스트 파일은 이름 하나당 엔터(\n)로 구분해서 저장한 타입
//현재 리스트에 있는 개수만큼 읽어온다
void CBatchNamerDlg::ImportName()
{
	CFileDialog dlg(TRUE, _T("*.txt"), NULL, OFN_ENABLESIZING | OFN_LONGNAMES | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, _T("Text Files(*.txt)|*.txt|All Files(*.*)|*.*||"), NULL);
	dlg.m_ofn.lpstrTitle = _T("바꿀 파일 이름 불러오기");
	if (dlg.DoModal() == IDCANCEL) return;
	CString strData, strName;
	ReadFileToCString(dlg.GetPathName(), strData);
	int nPos = 0;
	int i = 0;
	m_list.SetRedraw(FALSE);
	while (nPos != -1)
	{
		nPos = GetLine(strData, nPos, strName, _T("\n"));
		strName.TrimLeft(); strName.TrimRight();
		if (strName.IsEmpty() == FALSE)
		{
			//원래 이름이 존재해야만 하므로 맨 위부터 교체하는 형식
			if (i >= m_list.GetItemCount()) break; //리스트 넘어가면 끝
			m_list.SetItemText(i, COL_NEWNAME, strName);
			i++;
		}
	}
	m_list.SetRedraw(TRUE);
}

//경로명을 텍스트 파일에서 읽어와서 현재 목록 뒤에 추가한다
//텍스트 파일은 이름 하나당 엔터(\n)로 구분해서 저장한 타입
void CBatchNamerDlg::ImportPath()
{
	CFileDialog dlg(TRUE, _T("*.txt"), NULL, OFN_ENABLESIZING | OFN_LONGNAMES | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, _T("Text Files(*.txt)|*.txt|All Files(*.*)|*.*||"), NULL);
	dlg.m_ofn.lpstrTitle = _T("파일에서 경로목록 읽어 추가하기");
	if (dlg.DoModal() == IDCANCEL) return;
	CString strImportData;
	ReadFileToCString(dlg.GetPathName(), strImportData);

	CString strPath;
	CString& strData = strImportData;
	int nPos = 0;
	SetDlgItemText(IDC_ST_BAR, IDSTR(IDS_WORKING));
	m_list.SetRedraw(FALSE);
	while (nPos != -1)
	{
		nPos = GetLine(strData, nPos, strPath, _T("\n"));
		strPath.TrimLeft(); strPath.TrimRight();
		AddPathStart(strPath);
	}
	SetDlgItemText(IDC_ST_BAR, _T(""));
	m_list.SetRedraw(TRUE);
}

//#include <afxdlgs.h>
//파일의 경로를 하나로 통일. 일종의 MoveFile이 됨. 중복 체크 필요
void CBatchNamerDlg::NameSamePath()
{
	CFolderPickerDialog dlg;
///	CFolderDialog dlg(_T("경로 선택"));
	if (dlg.DoModal() == IDCANCEL) return;
	CString strPath = dlg.GetPathName();
	//e:\ 같은 경우를 대비하여 끝에 오는 \를 삭제
	if (strPath.GetAt(strPath.GetLength() - 1) == _T('\\')) strPath.Delete(strPath.GetLength() - 1);
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		//COL_FOLDER 변경해 주면 나중에 실제 변경때 이값이 COL_NEWNAME과 결합되면서 새 경로가 된다
		m_list.SetItemText(i, COL_NEWFOLDER, strPath);
	}
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::NameDelPos()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("지정위치를 삭제합니다.(첫글자는 1번째)"), _T("번째부터"), _T("번째까지"));
	dlg.AddOption(_T("앞에서부터 삭제"), INPUT_TWO);
	dlg.AddOption(_T("제일 뒤부터 삭제"), INPUT_TWO);
	if (dlg.DoModal() == IDCANCEL) return;

	int nStart = _ttoi(dlg.m_strReturn1);
	int nEnd = _ttoi(dlg.m_strReturn2);
	if (nStart == 0 && nEnd == 0) return;

	if (nStart < 0 || nEnd < 0)
	{
		AfxMessageBox(_T("음수값이나 잘못된 값이 입력되었습니다.")); return;
	}
	if (dlg.m_nCB == 0 && nEnd > 0 && nStart > nEnd)
	{
		AfxMessageBox(_T("시작점이 끝점보다 뒤에 있습니다.")); return;
	}
	if (dlg.m_nCB == 1 && nStart != 0)
	{
		AfxMessageBox(_T("맨 뒤에서부터 삭제할때는 '~까지'만 필요합니다.")); return;
	}

	CString strName, strExt;

	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		BOOL bIsDir = (BOOL)m_list.GetItemData(i);
		strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
		strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir);

		if (dlg.m_nCB == 0)	//앞의 n부터 m까지
		{
			if (nStart == 0) nStart = 1;
			int nLen = strName.GetLength();
			if (nStart <= nLen)
			{
				if (nEnd > 0 && nEnd < nLen) nLen = nEnd;
				strName.Delete(nStart - 1, nLen - nStart + 1);
			}
		}
		else if (dlg.m_nCB == 1) //뒤의 n개
		{
			int nLen = strName.GetLength();
			if (nEnd < nLen) nLen = nEnd;
			strName.Delete(strName.GetLength() - nLen, nLen);
		}

		if (strExt.IsEmpty() == FALSE) strName += strExt;
		m_list.SetItemText(i, COL_NEWNAME, strName);
	}
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::NameDelToken()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("지정된 문자로 묶인 부분을 삭제합니다."), _T("시작문자"), _T("끝문자"));

	if (dlg.DoModal() == IDCANCEL) return;
	if (dlg.m_strReturn1.IsEmpty() || dlg.m_strReturn2.IsEmpty())
	{
		AfxMessageBox(_T("시작/끝 문자가 정확하게 지정되지 않았습니다."));
		return;
	}

	TCHAR c1 = dlg.m_strReturn1.GetAt(0);
	TCHAR c2 = dlg.m_strReturn2.GetAt(0);

	CString strName, strExt;
	int n1, n2, nStart, nEnd;
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		BOOL bIsDir = (BOOL)m_list.GetItemData(i);
		strName = Get_Name(m_list.GetItemText(i, COL_NEWNAME), bIsDir);
		strExt = Get_Ext(m_list.GetItemText(i, COL_NEWNAME), bIsDir);

		nStart = -1; nEnd = strName.GetLength() - 1;
		n1 = 0; n2 = 0;

		for (n1 = 0; n1 <= nEnd; n1++)
		{
			if (strName.GetAt(n1) == c1) { nStart = n1; n1++; break; }
		}
		for (n2 = n1; n2 <= nEnd; n2++)
		{
			if (strName.GetAt(n2) == c2) { nEnd = n2; break; }
		}

		if (nStart != -1 && nStart < nEnd && nEnd == n2)
		{
			strName.Delete(nStart, nEnd - nStart + 1);
		}
		if (strExt.IsEmpty() == FALSE) strName += strExt;
		m_list.SetItemText(i, COL_NEWNAME, strName);
	}
	m_list.SetRedraw(TRUE);
}

//리스트를 정해진 기준에 따라 정렬한다
void CBatchNamerDlg::SortList()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("정렬 기준 설정"), _T(""), _T(""));
	dlg.AddOption(_T("파일 이름에 따라 오름차순"), INPUT_NONE);
	dlg.AddOption(_T("파일 이름에 따라 내림차순"), INPUT_NONE);
	dlg.AddOption(_T("전체경로에 따라 오름차순"), INPUT_NONE);
	dlg.AddOption(_T("전체경로에 따라 내림차순"), INPUT_NONE);
	dlg.AddOption(_T("파일 크기에 따라 오름차순"), INPUT_NONE);
	dlg.AddOption(_T("파일 크기에 따라 내림차순"), INPUT_NONE);
	dlg.AddOption(_T("수정한 시각에 따라 오름차순"), INPUT_NONE);
	dlg.AddOption(_T("수정한 시각에 따라 내림차순"), INPUT_NONE);
	dlg.AddOption(_T("만든 시각에 따라 오름차순"), INPUT_NONE);
	dlg.AddOption(_T("만든 시각에 따라 내림차순"), INPUT_NONE);

	if (dlg.DoModal() == IDCANCEL) return;

	int nCol = 0;
	BOOL bAsc = TRUE;
	switch (dlg.m_nCB)
	{
	case 0: nCol = COL_OLDNAME; bAsc = TRUE; break;
	case 1: nCol = COL_OLDNAME; bAsc = FALSE; break;
	case 2: nCol = COL_FULLPATH; bAsc = TRUE; break;
	case 3: nCol = COL_FULLPATH; bAsc = FALSE; break;
	case 4: nCol = COL_FILESIZE; bAsc = TRUE; break;
	case 5: nCol = COL_FILESIZE; bAsc = FALSE; break;
	case 6: nCol = COL_TIMEMODIFY; bAsc = TRUE; break;
	case 7: nCol = COL_TIMEMODIFY; bAsc = FALSE; break;
	case 8: nCol = COL_TIMECREATE; bAsc = TRUE; break;
	case 9: nCol = COL_TIMECREATE; bAsc = FALSE; break;
	}
	m_list.SetRedraw(FALSE);
	m_list.Sort(nCol, bAsc);
	m_list.SetRedraw(TRUE);
}

void CBatchNamerDlg::OnDblclkListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	ManualChange();
	*pResult = 0;
}

void CBatchNamerDlg::UpdateMenu()
{
	BOOL b = (m_list.GetItemCount() > 0);
	GetMenu()->EnableMenuItem(IDM_APPLY_CHANGE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_CLEAR_LIST, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_SORT_LIST, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_UNDO_CHANGE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_REPLACE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_ADD_FRONT, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_ADD_REAR, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_DEL_TYPE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_DIGIT, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_ADDNUM, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_EMPTY, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_SAMEPATH, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_DELPOS, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_DELTOKEN, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXT_ADD, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXT_DEL, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXT_REPLACE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXPORT_CLIP, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXPORT_FILE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXPORT_CLIP2, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXPORT_FILE2, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_IMPORT_FILE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);

	m_tool1.GetToolBarCtrl().EnableButton(IDM_APPLY_CHANGE, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_REPLACE, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_ADD_FRONT, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_ADD_REAR, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_DEL_TYPE, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_DIGIT, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_ADDNUM, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_EMPTY, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_DELPOS, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_DELTOKEN, b);

	m_tool2.GetToolBarCtrl().EnableButton(IDM_CLEAR_LIST, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_SORT_LIST, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_UNDO_CHANGE, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_NAME_SAMEPATH, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EXT_ADD, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EXT_DEL, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EXT_REPLACE, b);

	b = (m_list.GetNextItem(-1, LVNI_SELECTED) != -1);
	GetMenu()->EnableMenuItem(IDM_MANUAL_CHANGE, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EDIT_UP, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EDIT_DOWN, b ? MF_ENABLED | MF_BYCOMMAND : MF_GRAYED | MF_BYCOMMAND);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_MANUAL_CHANGE, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EDIT_UP, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EDIT_DOWN, b);

	GetMenu()->CheckMenuItem(IDM_SHOW_OLDFOLDER,
		FlagGET(APP()->m_nShowFlag, COL_OLDFOLDER) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
	GetMenu()->CheckMenuItem(IDM_SHOW_NEWFOLDER,
		FlagGET(APP()->m_nShowFlag, COL_NEWFOLDER) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
	GetMenu()->CheckMenuItem(IDM_SHOW_FULLPATH,
		FlagGET(APP()->m_nShowFlag, COL_FULLPATH) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
	GetMenu()->CheckMenuItem(IDM_SHOW_SIZE,
		FlagGET(APP()->m_nShowFlag, COL_FILESIZE) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
	GetMenu()->CheckMenuItem(IDM_SHOW_MODIFYTIME,
		FlagGET(APP()->m_nShowFlag, COL_TIMEMODIFY) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
	GetMenu()->CheckMenuItem(IDM_SHOW_CREATETIME,
		FlagGET(APP()->m_nShowFlag, COL_TIMECREATE) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

}

void CBatchNamerDlg::UpdateCount()
{
	int nCount = m_list.GetItemCount();
	CString strTemp;
	if (nCount == 0)
	{
		strTemp = _T("드래그 앤 드롭 또는 메뉴를 이용하여 파일을 추가해 주세요");
		UpdateMenu();
	}
	else strTemp.Format(_T("%d 개"), nCount);
	SetDlgItemText(IDC_ST_BAR, strTemp);
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
	m_font.CreateFontIndirect(&lf);
	m_list.SetFont(&m_font);
	//pFont->DeleteObject();
	//pFont->CreateFontIndirect(&lf);
	//m_list.SetFont(pFont);
}
