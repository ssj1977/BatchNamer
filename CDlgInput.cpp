// CDlgInput.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgInput.h"
#include "afxdialogex.h"
#include "EtcFunctions.h"

// CDlgInput 대화 상자

IMPLEMENT_DYNAMIC(CDlgInput, CDialogEx)

CDlgInput::CDlgInput(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INPUT, pParent)
{
	m_nCB = 0;
	m_nCommand = 0;
	m_nFontHeight = 12;
}

CDlgInput::~CDlgInput()
{
}

void CDlgInput::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgInput)
	DDX_Control(pDX, IDC_CB_INPUT, m_cb);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgInput, CDialogEx)
	//{{AFX_MSG_MAP(CDlgInput)
	ON_CBN_SELCHANGE(IDC_CB_INPUT, OnSelchangeCbInput)
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_BTN_FOLDER_SELECT, &CDlgInput::OnBnClickedBtnFolderSelect)
END_MESSAGE_MAP()


// CDlgInput 메시지 처리기
BOOL CDlgInput::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CRect rcWin;
	GetWindowRect(rcWin); 
	m_nMinWidth = rcWin.Width();
	m_nMinHeight = rcWin.Height();

	SetWindowText(m_strTitle);
	CString strItem;
	if (m_aInput.GetSize() > 0)
	{
		int nItem;
		for (int i = 0; i < m_aInput.GetSize(); i++)
		{
			InputItem& item = m_aInput[i];
			strItem.Format(L"%d) %s", i + 1, item.m_strItemName);
			nItem = m_cb.AddString(strItem);
			m_cb.SetItemData(nItem, (DWORD_PTR)&item);
		}
		m_cb.SetCurSel(m_nCB);
		OnSelchangeCbInput();
	}
	else return FALSE;

	if (m_strReturn1.IsEmpty() == FALSE) SetDlgItemText(IDC_EDIT_1, m_strReturn1);
	if (m_strReturn2.IsEmpty() == FALSE) SetDlgItemText(IDC_EDIT_2, m_strReturn2);

	LOGFONT lf;
	GetDlgItem(IDC_STATIC_1)->GetFont()->GetLogFont(&lf);
	m_nFontHeight = MulDiv(-1 * lf.lfHeight, 72, GetDeviceCaps(GetDC()->GetSafeHdc(), LOGPIXELSY));
	if (APP()->m_rcInput.IsRectEmpty() == FALSE)
	{
		CRect rcScreen;
		::GetWindowRect(::GetDesktopWindow(), &rcScreen);
		APP()->m_rcInput.NormalizeRect();
		CRect rcVisible;
		rcVisible.IntersectRect(APP()->m_rcInput, rcScreen);
		if (rcVisible.Width() > 200 && rcVisible.Height() > 100)
		{
			MoveWindow(APP()->m_rcInput, TRUE);
		}
	}
	ArrangeCtrl();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgInput::SetInputItem(InputItem* pItem)
{
	BOOL bShow1 = !(pItem->m_strLabel1.IsEmpty());
	BOOL bShow2 = !(pItem->m_strLabel2.IsEmpty());
	SetDlgItemText(IDC_STATIC_1, pItem->m_strLabel1);
	SetDlgItemText(IDC_STATIC_2, pItem->m_strLabel2);
	GetDlgItem(IDC_STATIC_1)->ShowWindow(bShow1 ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_EDIT_1)->ShowWindow(bShow1 ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_STATIC_2)->ShowWindow(bShow2 ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_EDIT_2)->ShowWindow(bShow2 ? SW_SHOW : SW_HIDE);

	HWND h1 = GetDlgItem(IDC_EDIT_1)->GetSafeHwnd();
	HWND h2 = GetDlgItem(IDC_EDIT_2)->GetSafeHwnd();
	SetWindowLong(h1, GWL_STYLE, pItem->m_bIsNumber1 
		? (GetWindowLong(h1, GWL_STYLE) | ES_NUMBER) : (GetWindowLong(h1, GWL_STYLE) & ~ES_NUMBER));
	SetWindowLong(h2, GWL_STYLE, pItem->m_bIsNumber2 
		? (GetWindowLong(h2, GWL_STYLE) | ES_NUMBER) : (GetWindowLong(h2, GWL_STYLE) & ~ES_NUMBER));
	//필요한 경우 기본값을 넣어준다
	if (pItem->m_strLabel1 == IDSTR(IDS_DATETIME_FORMAT))
	{ //시각형일때 기본 날짜 포맷 스트링
		SetDlgItemText(IDC_EDIT_1, L"%Y-%m-%d");
		SetDlgItemText(IDC_EDIT_2, L"");
	}
	else if (pItem->m_nSubCommand == IDS_FOLDER_SPECIFIC)
	{ //폴더 지정일때 마지막으로 열어본 폴더 
		CFileDialog dlg(TRUE, _T("*.*"), NULL,
			OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_HIDEREADONLY,
			_T("All Files(*.*)|*.*||"), NULL, 0, TRUE);
		SetDlgItemText(IDC_EDIT_1, dlg.GetFolderPath());
		SetDlgItemText(IDC_EDIT_2, L"");
	}
	else // 나머지는 모두 비운다
	{
		SetDlgItemText(IDC_EDIT_1, L"");
		SetDlgItemText(IDC_EDIT_2, L"");
	}
	GetDlgItem(IDC_BTN_FOLDER_SELECT)->ShowWindow((pItem->m_nSubCommand == IDS_FOLDER_SPECIFIC) ? SW_SHOW : SW_HIDE);
	ArrangeCtrl();
}

//파일 이름에 맞지 않는 글자(\, /, | ,<. >, :, ", ?, *) 를 미리 체크
BOOL CheckInvalidCharForFile(CString str, BOOL bPassWildCard, BOOL bPathWithFolder);
void RemoveInvalidCharForFile(CString& str, BOOL bPassWildCard, BOOL bPathWithFolder);

CString ExtractWildCard(CString str, BOOL bAddToken)
{
	CString strRet;
	TCHAR c;
	BOOL bToken = FALSE;
	for (int i = 0; i < str.GetLength(); i++)
	{
		c = str.GetAt(i);
		if (c == _T('?') || c == _T('*'))
		{
			strRet += c;
			bToken = FALSE;
		}
		else if (bToken == FALSE && bAddToken == TRUE)
		{
			bToken = TRUE;
			strRet += _T('_');
		}
	}
	return strRet;
}

BOOL ValidateWildCard(CString str)
{
	if (str.IsEmpty() == FALSE)
	{
		// ** , ?*, ?* 는 비정상
		if (str.Find(_T("**")) != -1 ||
			str.Find(_T("*?")) != -1 ||
			str.Find(_T("?*")) != -1) return FALSE;
		// 상수 문자열 토큰(_)이 하나도 없으면 비정상
		if (str.Find(_T("_")) == -1) return FALSE;
	}
	return TRUE;
}


inline void RemoveEnter(CString& str)
{
	str.Remove(_T('\r'));
	str.Remove(_T('\n'));
	str.Remove(_T('\t'));
}

void CDlgInput::OnOK()
{
	m_nCB = m_cb.GetCurSel();
	if (GetDlgItem(IDC_EDIT_1)->IsWindowVisible())	GetDlgItemText(IDC_EDIT_1, m_strReturn1);
	else m_strReturn1.Empty();
	if (GetDlgItem(IDC_EDIT_2)->IsWindowVisible())	GetDlgItemText(IDC_EDIT_2, m_strReturn2);
	else m_strReturn2.Empty();
	BOOL bWildCard = FALSE;
	BOOL bPathWithFolder = (m_nCommand == IDS_TB_16) ? TRUE : FALSE;
	int nSubCommand = GetSubCommand();
	if (nSubCommand == IDS_REPLACESTRING || nSubCommand == IDS_FOLDER_PATTERN) bWildCard = TRUE;
	RemoveEnter(m_strReturn1);
	RemoveEnter(m_strReturn2);
	if (APP()->m_bNameAutoFix)
	{
		RemoveInvalidCharForFile(m_strReturn1, bWildCard, bPathWithFolder);
		RemoveInvalidCharForFile(m_strReturn2, bWildCard, bPathWithFolder);
	}
	else
	{
		if (CheckInvalidCharForFile(m_strReturn1, bWildCard, bPathWithFolder)
			|| CheckInvalidCharForFile(m_strReturn2, bWildCard, bPathWithFolder))
		{
			APP()->ShowMsg(IDSTR(IDS_INVALID_CHAR), IDSTR(IDS_MSG_ERROR));
			return;
		}
	}
	if (bWildCard)
	{
		//str1과 str2의 wildcard 종류, 개수, 순서가 일치하여야 함
		//wild카드 이외의 문자를 지우고 난후 값이 일치하는지 본다
		//VerifyReturnValue가 아닌 여기에서 처리하여 창을 닫히지 않게 한다.
		CString strWild1 = ExtractWildCard(m_strReturn1, TRUE);
		CString strWild2 = ExtractWildCard(m_strReturn2, TRUE);
		if (strWild1 != strWild2)
		{
			if (strWild2 != _T("_") && strWild2.IsEmpty() == FALSE)
			{
				AfxMessageBox(IDSTR(IDS_INVALID_WILDCARD_PAIR));
				return;
			}
		}
		//Wildcard 자체의 문법적 오류가 있는 경우를 판별한다.
		if (ValidateWildCard(strWild1) == FALSE || ValidateWildCard(strWild2) == FALSE)
		{
			AfxMessageBox(IDSTR(IDS_INVALID_WILDCARD));
			return;
		}
	}
	GetWindowRect(APP()->m_rcInput);
	CDialogEx::OnOK();
}

void CDlgInput::OnCancel()
{
	GetWindowRect(APP()->m_rcInput);
	CDialogEx::OnCancel();
}

void CDlgInput::InitValue(int nSubCommand, CString str1, CString str2)
{
	if (nSubCommand != 0)
	{
		for (int i = 0; i < m_aInput.GetSize(); i++)
		{
			if (m_aInput[i].m_nSubCommand == nSubCommand)
			{
				m_nCB = i;
				break;
			}
		}
	}
	m_strReturn1 = str1;
	m_strReturn2 = str2;
}
void CDlgInput::AddOption(InputItem* pItem)
{
	if (pItem == NULL) return;
	m_aInput.Add(*pItem);
}

void CDlgInput::OnSelchangeCbInput()
{
	int nSel = m_cb.GetCurSel();
	if (nSel < 0 || nSel >= m_aInput.GetSize()) return;
	InputItem* pItem = (InputItem*)m_cb.GetItemData(nSel);
	m_nCB = nSel;
	SetInputItem(pItem);
}

int CDlgInput::GetSubCommand()
{
	return m_aInput[m_nCB].m_nSubCommand;
}


void CDlgInput::InitInputByCommand(int nCommand)
{
	m_nCommand = nCommand;
	m_strTitle = IDSTR(nCommand);
	InputItem item;
	switch (nCommand)
	{
	case IDS_TB_01: // 이름 처리
	case IDS_TB_19: // 확장자 처리
		if (nCommand == IDS_TB_19)
		{
			item.m_strItemName = IDSTR(IDS_EXT_REPLACE); //"확장자를 추가합니다."
			item.m_nSubCommand = IDS_EXT_REPLACE;
			item.m_strLabel1 = IDSTR(IDS_EXT_OLD); //_T("원래 확장자")
			item.m_strLabel2 = IDSTR(IDS_EXT_NEW); //_T("바꿀 확장자")
			m_aInput.Add(item);
		}
		item.m_strItemName = IDSTR(IDS_REPLACESTRING);
		item.m_nSubCommand = IDS_REPLACESTRING;
		item.m_strLabel1 = IDSTR(IDS_REPLACEOLD);
		item.m_strLabel2 = IDSTR(IDS_REPLACENEW);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_FLIPSTRING);
		item.m_nSubCommand = IDS_FLIPSTRING;
		item.m_strLabel1 = IDSTR(IDS_FLIPPIVOT);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_LOWERCASE);
		item.m_nSubCommand = IDS_LOWERCASE;
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_UPPERCASE);
		item.m_nSubCommand = IDS_UPPERCASE;
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_UPPERCASE_FIRST);
		item.m_nSubCommand = IDS_UPPERCASE_FIRST;
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_UPPERCASE_WORD);
		item.m_nSubCommand = IDS_UPPERCASE_WORD;
		m_aInput.Add(item);
		break;
	case IDS_TB_02: //Add Front Name
	case IDS_TB_03: //Add End Name
	case IDS_TB_18: //Add End Ext
		item.m_strItemName = IDSTR(IDS_ADDSTRING);
		item.m_nSubCommand = IDS_ADDSTRING;
		item.m_strLabel1 = IDSTR(IDS_STRINGTOADD);
		if (nCommand == IDS_TB_02)	item.m_strLabel2 = IDSTR(IDS_INSERT_BYPOS_FRONT);
		else						item.m_strLabel2 = IDSTR(IDS_INSERT_BYPOS_BACK);
		item.m_bIsNumber2 = TRUE;
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_ADDPARENT);
		item.m_nSubCommand = IDS_ADDPARENT;
		item.m_strLabel1 = IDSTR(IDS_ADDPREFIX);
		item.m_strLabel2 = IDSTR(IDS_ADDSUFFIX);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_ADDDATECREATE);
		item.m_nSubCommand = IDS_ADDDATECREATE;
		item.m_strLabel1 = IDSTR(IDS_ADDPREFIX);
		item.m_strLabel2 = IDSTR(IDS_ADDSUFFIX);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_ADDDATEMODIFY);
		item.m_nSubCommand = IDS_ADDDATEMODIFY;
		item.m_strLabel1 = IDSTR(IDS_ADDPREFIX);
		item.m_strLabel2 = IDSTR(IDS_ADDSUFFIX);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_ADDTIMECREATE);
		item.m_nSubCommand = IDS_ADDTIMECREATE;
		item.m_strLabel1 = IDSTR(IDS_ADDPREFIX);
		item.m_strLabel2 = IDSTR(IDS_ADDSUFFIX);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_ADDTIMEMODIFY);
		item.m_nSubCommand = IDS_ADDTIMEMODIFY;
		item.m_strLabel1 = IDSTR(IDS_ADDPREFIX);
		item.m_strLabel2 = IDSTR(IDS_ADDSUFFIX);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_ADDDATETIMECREATE);
		item.m_nSubCommand = IDS_ADDDATETIMECREATE;
		item.m_strLabel1 = IDSTR(IDS_DATETIME_FORMAT);
		if (nCommand == IDS_TB_02)	item.m_strLabel2 = IDSTR(IDS_INSERT_BYPOS_FRONT);
		else						item.m_strLabel2 = IDSTR(IDS_INSERT_BYPOS_BACK);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_ADDDATETIMEMODIFY);
		item.m_nSubCommand = IDS_ADDDATETIMEMODIFY;
		item.m_strLabel1 = IDSTR(IDS_DATETIME_FORMAT);
		if (nCommand == IDS_TB_02)	item.m_strLabel2 = IDSTR(IDS_INSERT_BYPOS_FRONT);
		else						item.m_strLabel2 = IDSTR(IDS_INSERT_BYPOS_BACK);
		m_aInput.Add(item);
		break;
	case IDS_TB_05: // 삭제하기
		item.m_strItemName = IDSTR(IDS_DELPOS_FRONT);
		item.m_nSubCommand = IDS_DELPOS_FRONT;
		item.m_bIsNumber1 = TRUE;
		item.m_bIsNumber2 = TRUE;
		item.m_strLabel1 = IDSTR(IDS_POS_1);
		item.m_strLabel2 = IDSTR(IDS_POS_2);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_DELPOS_REAR);
		item.m_nSubCommand = IDS_DELPOS_REAR;
		item.m_bIsNumber1 = TRUE;
		item.m_strLabel1 = IDSTR(IDS_POS_1_REVERSE);
		item.m_strLabel2 = IDSTR(IDS_POS_2_REVERSE);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_REMOVEBYBRACKET); //_T("지정된 문자로 묶인 부분을 삭제합니다.")
		item.m_nSubCommand = IDS_REMOVEBYBRACKET;
		item.m_strLabel1 = IDSTR(IDS_BRACKET1); // _T("시작문자")
		item.m_strLabel2 = IDSTR(IDS_BRACKET2); // _T("끝문자")
		m_aInput.Add(item);
		item.m_strItemName = IDSTR(IDS_DELPOS_FRONT_INVERT);
		item.m_nSubCommand = IDS_DELPOS_FRONT_INVERT;
		item.m_bIsNumber1 = TRUE;
		item.m_bIsNumber2 = TRUE;
		item.m_strLabel1 = IDSTR(IDS_POS_1);
		item.m_strLabel2 = IDSTR(IDS_POS_2);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_DELPOS_REAR_INVERT);
		item.m_nSubCommand = IDS_DELPOS_REAR_INVERT;
		item.m_bIsNumber1 = TRUE;
		item.m_strLabel1 = IDSTR(IDS_POS_1_REVERSE);
		item.m_strLabel2 = IDSTR(IDS_POS_2_REVERSE);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_REMOVEBYBRACKET_INVERT); //_T("지정된 문자로 묶인 부분을 삭제합니다.")
		item.m_nSubCommand = IDS_REMOVEBYBRACKET_INVERT;
		item.m_strLabel1 = IDSTR(IDS_BRACKET1); // _T("시작문자")
		item.m_strLabel2 = IDSTR(IDS_BRACKET2); // _T("끝문자")
		m_aInput.Add(item);
		break;
	case IDS_TB_08: // 자릿수 맞추기
		item.m_strItemName = IDSTR(IDS_DIGITBACK); //"맨 뒤쪽 숫자의 앞에 0을 추가해서 자릿수를 맞춥니다."
		item.m_nSubCommand = IDS_DIGITBACK;
		item.m_bIsNumber1 = TRUE;
		item.m_strLabel1 = IDSTR(IDS_DIGITCOUNT); //_T("자릿수")
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_DIGITFRONT); //"맨 앞쪽 숫자의 앞에 0을 추가해서 자릿수를 맞춥니다."
		item.m_nSubCommand = IDS_DIGITFRONT;
		item.m_bIsNumber1 = TRUE;
		item.m_strLabel1 = IDSTR(IDS_DIGITCOUNT); //_T("자릿수")
		m_aInput.Add(item);
		break;
	case IDS_TB_09: //번호 붙이기
		item.m_strItemName = IDSTR(IDS_ADDNUM_ALL_BACK); //"모든 이름 뒤에 목록 순서대로 번호를 붙입니다."
		item.m_nSubCommand = IDS_ADDNUM_ALL_BACK;
		item.m_bIsNumber1 = TRUE;
		item.m_strLabel1 = IDSTR(IDS_DIGITCOUNT); //_T("자릿수")
		item.m_bIsNumber2 = TRUE;
		item.m_strLabel2 = IDSTR(IDS_STARTNUMBER); //_T("시작값")
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_ADDNUM_ALL_FRONT); //"모든 이름 앞에 목록 순서대로 번호를 붙입니다."
		item.m_nSubCommand = IDS_ADDNUM_ALL_FRONT;
		item.m_bIsNumber1 = TRUE;
		item.m_strLabel1 = IDSTR(IDS_DIGITCOUNT); //_T("자릿수")
		item.m_bIsNumber2 = TRUE;
		item.m_strLabel2 = IDSTR(IDS_STARTNUMBER); //_T("시작값")
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_ADDNUM_BYFOLDER_BACK); //"폴더별로 이름 뒤에 목록 순서대로 번호를 붙입니다."
		item.m_nSubCommand = IDS_ADDNUM_BYFOLDER_BACK;
		item.m_bIsNumber1 = TRUE;
		item.m_strLabel1 = IDSTR(IDS_DIGITCOUNT); //_T("자릿수")
		item.m_bIsNumber2 = TRUE;
		item.m_strLabel2 = IDSTR(IDS_STARTNUMBER); //_T("시작값")
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_ADDNUM_BYFOLDER_FRONT); //"폴더별로 이름 앞에 목록 순서대로 번호를 붙입니다."
		item.m_nSubCommand = IDS_ADDNUM_BYFOLDER_FRONT;
		item.m_bIsNumber1 = TRUE;
		item.m_strLabel1 = IDSTR(IDS_DIGITCOUNT); //_T("자릿수")
		item.m_bIsNumber2 = TRUE;
		item.m_strLabel2 = IDSTR(IDS_STARTNUMBER); //_T("시작값")
		m_aInput.Add(item);
		break;
	case IDS_TB_13: // Manual Change
		item.m_strItemName = IDSTR(IDS_MANUAL_CHANGE);
		item.m_nSubCommand = IDS_MANUAL_CHANGE;
		item.m_strLabel1 = IDSTR(IDS_COL_NEWNAME);
		m_aInput.Add(item);
		break;
	case IDS_TB_16: // 폴더 변경
		item.m_strItemName = IDSTR(IDS_FOLDER_SPECIFIC); 		//지정한 특정 폴더로
		item.m_nSubCommand = IDS_FOLDER_SPECIFIC;
		item.m_strLabel1 = IDSTR(IDS_COL_NEWFOLDER);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_FOLDER_PARENT); 		//지정한 단계만큼 상위 폴더로
		item.m_nSubCommand = IDS_FOLDER_PARENT;
		item.m_strLabel1 = IDSTR(IDS_LEVEL);
		item.m_bIsNumber1 = TRUE;
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_FOLDER_PATTERN); 		//지정한 패턴을 추출해서 하위폴더 생성
		item.m_nSubCommand = IDS_FOLDER_PATTERN;
		item.m_strLabel1 = IDSTR(IDS_PATTERN_FIND);
		item.m_strLabel2 = IDSTR(IDS_PATTERN_OUTPUT);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_FOLDER_BRACKET); 		//묶인곳을 추출해서 하위폴더 생성
		item.m_nSubCommand = IDS_FOLDER_BRACKET;
		item.m_strLabel1 = IDSTR(IDS_BRACKET1);
		item.m_strLabel2 = IDSTR(IDS_BRACKET2);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_FOLDER_POS); 		//앞에서부터 지정된 부분을 추출하여 하위폴더 생성
		item.m_nSubCommand = IDS_FOLDER_POS;
		item.m_strLabel1 = IDSTR(IDS_POS_1);
		item.m_bIsNumber1 = TRUE;
		item.m_strLabel2 = IDSTR(IDS_POS_2);
		item.m_bIsNumber2 = TRUE;
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_FOLDER_POS_REVERSE); 		//뒤에서부터 지정된 부분을 추출하여 하위폴더 생성
		item.m_nSubCommand = IDS_FOLDER_POS_REVERSE;
		item.m_strLabel1 = IDSTR(IDS_POS_1_REVERSE);
		item.m_bIsNumber1 = TRUE;
		item.m_strLabel2 = IDSTR(IDS_POS_2_REVERSE);
		item.m_bIsNumber2 = TRUE;
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_FOLDER_DATECREATE); 		//생성된 날짜를 추출하여 하위폴더 생성
		item.m_nSubCommand = IDS_FOLDER_DATECREATE;
		item.m_strLabel1 = IDSTR(IDS_DATETIME_FORMAT); // 시간값에서 추출할 문자열 포맷
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_FOLDER_DATEMODIFY); 		//변경된 날짜를 추출하여 하위폴더 생성
		item.m_nSubCommand = IDS_FOLDER_DATEMODIFY;
		item.m_strLabel1 = IDSTR(IDS_DATETIME_FORMAT); // 시간값에서 추출할 문자열 포맷
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_FOLDER_EXT); 		//확장자를 추출하여 하위폴더 생성
		item.m_nSubCommand = IDS_FOLDER_EXT;
		m_aInput.Add(item);
		break;
	case IDS_PRESET_NAME:
		item.m_strItemName = IDSTR(IDS_PRESET_NAME_DESC);
		item.m_nSubCommand = IDS_PRESET_NAME_DESC;
		item.m_strLabel1 = IDSTR(IDS_PRESET_NAME); //_T("원래 확장자")
		m_aInput.Add(item);
		break;
	}
}

BOOL CDlgInput::VerifyReturnValue()
{
	int nCommand = m_nCommand;
	int nSubCommand = GetSubCommand();
	switch (nCommand)
	{
	case IDS_TB_01: //Replace
	case IDS_TB_19: // Replace Extension
		if (nSubCommand == IDS_REPLACESTRING || nSubCommand == IDS_FLIPSTRING)
		{
			if (m_strReturn1.IsEmpty()) return FALSE;
		}
		if (nSubCommand == IDS_EXT_REPLACE)
		{
			if (m_strReturn2.IsEmpty()) return FALSE;
		}
		break;
		break;
	case IDS_TB_02: //Add Front
	case IDS_TB_03: //Add End
	case IDS_TB_18: // Add Extension
		if (nSubCommand == IDS_ADDSTRING) {	if (m_strReturn1.IsEmpty()) return FALSE;}
		break;
	case IDS_TB_05:
		if (nSubCommand == IDS_DELPOS_FRONT || nSubCommand == IDS_DELPOS_REAR ||
			nSubCommand == IDS_DELPOS_FRONT_INVERT || nSubCommand == IDS_DELPOS_REAR_INVERT)
		{
			int nStart = _ttoi(m_strReturn1);
			int nEnd = _ttoi(m_strReturn2);
			if (nStart == 0 && nEnd == 0) return FALSE;
			if (nEnd > 0 && nStart > nEnd)
			{
				AfxMessageBox(IDSTR(IDS_MSG_INVALIDPOS));
				return FALSE;
			}
		}
		else if (nSubCommand == IDS_REMOVEBYBRACKET)
		{
			if (m_strReturn1.IsEmpty() || m_strReturn2.IsEmpty())
			{
				AfxMessageBox(IDSTR(IDS_MSG_BRACKETINVALID)); //_T("시작/끝 문자가 정확하게 지정되지 않았습니다.")
				return FALSE;
			}
			if (m_strReturn1.GetLength() > 1 || m_strReturn2.GetLength() > 1)
			{
				AfxMessageBox(IDSTR(IDS_MSG_BRACKETLEN)); //(_T("구분자 길이가 한 글자가 아닙니다."));
				return FALSE;
			}
		}
		break;
	case IDS_TB_08:
		if (nSubCommand == IDS_DIGITBACK || nSubCommand == IDS_DIGITFRONT)
		{
			int nDigit = _ttoi(m_strReturn1);
			if (nDigit < 0 || nDigit > 10)
			{
				AfxMessageBox(IDSTR(IDS_MSG_INVALIDDIGIT)); //_T("자릿수 입력이 잘못되었습니다."));
				return FALSE;
			}
		}
		break;
	case IDS_TB_09:
		if (nSubCommand == IDS_ADDNUM_ALL_BACK || nSubCommand == IDS_ADDNUM_ALL_FRONT ||
			nSubCommand == IDS_ADDNUM_BYFOLDER_BACK || nSubCommand == IDS_ADDNUM_BYFOLDER_FRONT	)
		{
			int nDigit = _ttoi(m_strReturn1);
			if (nDigit < 0 || nDigit > 10)
			{
				AfxMessageBox(IDSTR(IDS_MSG_INVALIDDIGIT));
				return FALSE;
			}
		}
		break;
	case IDS_TB_13: // Manual Change
		break;
	case IDS_TB_16: // 폴더 변경
		if (nSubCommand == IDS_FOLDER_POS || nSubCommand == IDS_FOLDER_POS_REVERSE)
		{
			int nStart = _ttoi(m_strReturn1);
			int nEnd = _ttoi(m_strReturn2);
			if (nStart == 0 && nEnd == 0) return FALSE;
			if (nEnd > 0 && nStart > nEnd)
			{
				AfxMessageBox(IDSTR(IDS_MSG_INVALIDPOS));
				return FALSE;
			}
		}
		else if (nSubCommand == IDS_FOLDER_BRACKET)
		{
			if (m_strReturn1.IsEmpty() || m_strReturn2.IsEmpty())
			{
				AfxMessageBox(IDSTR(IDS_MSG_BRACKETINVALID)); //_T("시작/끝 문자가 정확하게 지정되지 않았습니다.")
				return FALSE;
			}
			if (m_strReturn1.GetLength() > 1 || m_strReturn2.GetLength() > 1)
			{
				AfxMessageBox(IDSTR(IDS_MSG_BRACKETLEN)); //(_T("구분자 길이가 한 글자가 아닙니다."));
				return FALSE;
			}
		}
		else if (nSubCommand == IDS_FOLDER_PATTERN)
		{
			if (m_strReturn1.IsEmpty()) return FALSE;
		}
		break;
	case IDS_PRESET_NAME:
		//if (m_strReturn1.IsEmpty()) return FALSE;
		break;
	}
	return TRUE;
}


BOOL CDlgInput::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if ((GetKeyState(VK_CONTROL) & 0xFF00) != 0)
		{
			int nItem = -1;
			if (pMsg->wParam >= 0x31 && pMsg->wParam <= 0x39) //Number Keys
			{
				nItem = int(pMsg->wParam) - 0x31;
			}
			if (pMsg->wParam >= 0x61 && pMsg->wParam <= 0x69) //Numpad Keys
			{
				nItem = int(pMsg->wParam) - 0x61;
			}
			if (nItem != -1 && nItem < m_cb.GetCount())
			{
				m_cb.SetCurSel(nItem);
				OnSelchangeCbInput();
				return TRUE;
			}
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgInput::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (::IsWindow(m_cb.GetSafeHwnd()) != FALSE)
	{
		ArrangeCtrl();
	}
}


void CDlgInput::ArrangeCtrl()
{
	CRect rc;
	GetClientRect(rc);
	rc.DeflateRect(m_nFontHeight, m_nFontHeight);
	int nInputHeight = rc.Height();
	CRect rcStatic; GetDlgItem(IDC_STATIC_1)->GetWindowRect(rcStatic); int nStaticHeight = rcStatic.Height();
	CRect rcCombo;	m_cb.GetWindowRect(rcCombo);	int nComboHeight = rcCombo.Height();
	CRect rcButton; GetDlgItem(IDOK)->GetWindowRect(rcButton); int nButtonHeight = rcButton.Height(); int nButtonWidth = rcButton.Width();
	//
	GetDlgItem(IDC_STATIC_3)->MoveWindow(rc.left,rc.top,rc.Width(),nStaticHeight);
	rc.top += nStaticHeight + 2;
	m_cb.MoveWindow(rc.left, rc.top, rc.Width(), nComboHeight);
	rc.top += nComboHeight + m_nFontHeight;
	//
	nInputHeight -= (nStaticHeight + 2) * 3 + nComboHeight + (m_nFontHeight * 3) + nButtonHeight;
	nInputHeight /= 2;

	CButton* pBtn = (CButton*)(GetDlgItem(IDC_BTN_FOLDER_SELECT));
	CRect rcBtn;
	pBtn->GetWindowRect(rcBtn);
	int nBtnW = rcBtn.Width();
	rcBtn.left = rc.right - nBtnW;
	rcBtn.right = rc.right;
	rcBtn.top = rc.top;
	rcBtn.bottom = rc.top + rcStatic.Height() + 2;
	pBtn->MoveWindow(rcBtn);
	if (pBtn->IsWindowVisible() == FALSE)
	{
		GetDlgItem(IDC_STATIC_1)->MoveWindow(rc.left, rc.top, rc.Width(), rcStatic.Height());
	}
	else
	{
		GetDlgItem(IDC_STATIC_1)->MoveWindow(rc.left, rc.top, rc.Width() - rcBtn.Width() - 5, rcStatic.Height());
	}
	rc.top += nStaticHeight + 2;

	GetDlgItem(IDC_EDIT_1)->MoveWindow(rc.left, rc.top, rc.Width(), nInputHeight);
	rc.top += nInputHeight + m_nFontHeight;
	//
	GetDlgItem(IDC_STATIC_2)->MoveWindow(rc.left, rc.top, rc.Width(), rcStatic.Height());
	rc.top += nStaticHeight + 2;
	GetDlgItem(IDC_EDIT_2)->MoveWindow(rc.left, rc.top, rc.Width(), nInputHeight);
	//
	GetDlgItem(IDOK)->MoveWindow(rc.right - (nButtonWidth) *2 - m_nFontHeight, rc.bottom - nButtonHeight, nButtonWidth, nButtonHeight );
	GetDlgItem(IDCANCEL)->MoveWindow(rc.right - nButtonWidth, rc.bottom - nButtonHeight, nButtonWidth, nButtonHeight);
	Invalidate();
	RedrawWindow();
}


void CDlgInput::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	lpMMI->ptMinTrackSize.x = m_nMinWidth;
	lpMMI->ptMinTrackSize.y = m_nMinHeight;
	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CDlgInput::OnBnClickedBtnFolderSelect()
{
	CFolderPickerDialog dlg;
	CString strTitle;
	strTitle.LoadString(IDS_FOLDER_SPECIFIC);
	dlg.GetOFN().lpstrTitle = strTitle;
	if (dlg.DoModal() == IDCANCEL) return;
	SetDlgItemText(IDC_EDIT_1, dlg.GetPathName());
}
