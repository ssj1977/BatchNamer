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
END_MESSAGE_MAP()


// CDlgInput 메시지 처리기
BOOL CDlgInput::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(m_strTitle);
	if (m_aInput.GetSize() > 0)
	{
		int nItem;
		for (int i = 0; i < m_aInput.GetSize(); i++)
		{
			InputItem& item = m_aInput[i];
			nItem = m_cb.AddString(item.m_strItemName);
			m_cb.SetItemData(nItem, (DWORD_PTR)&item);
		}
		m_cb.SetCurSel(m_nCB);
		OnSelchangeCbInput();
	}
	else return FALSE;

	if (m_strReturn1.IsEmpty() == FALSE) SetDlgItemText(IDC_EDIT_1, m_strReturn1);
	if (m_strReturn2.IsEmpty() == FALSE) SetDlgItemText(IDC_EDIT_2, m_strReturn2);

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
	SetDlgItemText(IDC_EDIT_1, L"");
	SetDlgItemText(IDC_EDIT_2, L"");
}

//파일 이름에 맞지 않는 글자(\, /, | ,<. >, :, ", ?, *) 를 미리 체크
BOOL CheckInvalidCharForFile(CString str, BOOL bPassWildCard)
{
	if (str.Find(_T('\\')) != -1) return TRUE;
	if (str.Find(_T('\"')) != -1) return TRUE;
	if (str.Find(_T('/')) != -1) return TRUE;
	if (str.Find(_T('|')) != -1) return TRUE;
	if (str.Find(_T('<')) != -1) return TRUE;
	if (str.Find(_T('>')) != -1) return TRUE;
	if (str.Find(_T(':')) != -1) return TRUE;
	if (bPassWildCard == FALSE && str.Find(_T('?')) != -1) return TRUE;
	if (bPassWildCard == FALSE && str.Find(_T('*')) != -1) return TRUE;
	return FALSE;
}


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

void CDlgInput::OnOK()
{
	if (GetDlgItem(IDC_EDIT_1)->IsWindowVisible())	GetDlgItemText(IDC_EDIT_1, m_strReturn1);
	else m_strReturn1.Empty();
	if (GetDlgItem(IDC_EDIT_2)->IsWindowVisible())	GetDlgItemText(IDC_EDIT_2, m_strReturn2);
	else m_strReturn2.Empty();
	BOOL bWildCard = (GetSubCommand() == IDS_REPLACESTRING) ? TRUE : FALSE;
	if (CheckInvalidCharForFile(m_strReturn1, bWildCard) 
		|| CheckInvalidCharForFile(m_strReturn2, bWildCard))
	{
		AfxMessageBox(IDSTR(IDS_INVALID_CHAR));
		return;
	}
	if (bWildCard)
	{
		//str1과 str2의 wildcard 종류, 개수, 순서가 일치하여야 함
		//wild카드 이외의 문자를 지우고 난후 값이 일치하는지 본다
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
	m_nCB = m_cb.GetCurSel();
	CDialogEx::OnOK();
}

void CDlgInput::OnCancel()
{
	// TODO: Add extra cleanup here

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
	case IDS_TB_01: //Replace
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
	case IDS_TB_02: //Add Front
	case IDS_TB_03: //Add End
		item.m_strItemName = IDSTR(IDS_ADDSTRING);
		item.m_nSubCommand = IDS_ADDSTRING;
		item.m_strLabel1 = IDSTR(IDS_STRINGTOADD);
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
		break;
	case IDS_TB_05:
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
		item.m_strLabel1 = IDSTR(IDS_POS_1_REAR);
		m_aInput.Add(item);
		item.Clear();
		item.m_strItemName = IDSTR(IDS_REMOVEBYBRACKET); //_T("지정된 문자로 묶인 부분을 삭제합니다.")
		item.m_nSubCommand = IDS_REMOVEBYBRACKET;
		item.m_strLabel1 = IDSTR(IDS_BRACKET1); // _T("시작문자")
		item.m_strLabel2 = IDSTR(IDS_BRACKET2); // _T("끝문자")
		m_aInput.Add(item);
		break;
	case IDS_TB_08:
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
	case IDS_TB_09:
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
	case IDS_TB_18: // Add Extension
		item.m_strItemName = IDSTR(IDS_EXT_APPEND); //"확장자를 추가합니다."
		item.m_nSubCommand = IDS_EXT_APPEND;
		item.m_strLabel1 = IDSTR(IDS_EXT_TOADD); //_T("추가할 확장자")
		m_aInput.Add(item);
		break;
	case IDS_TB_19: // Replac Extension
		item.m_strItemName = IDSTR(IDS_EXT_REPLACE); //"확장자를 변경합니다."
		item.m_nSubCommand = IDS_EXT_REPLACE;
		item.m_strLabel1 = IDSTR(IDS_EXT_OLD); //_T("원래 확장자")
		item.m_strLabel2 = IDSTR(IDS_EXT_NEW); //_T("바뀔 확장자")
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
		if (nSubCommand == IDS_REPLACESTRING || nSubCommand == IDS_FLIPSTRING)
		{
			if (m_strReturn1.IsEmpty()) return FALSE;
		}
		break;
	case IDS_TB_02: //Add Front
	case IDS_TB_03: //Add End
		if (nSubCommand == IDS_ADDSTRING) {	if (m_strReturn1.IsEmpty()) return FALSE;}
		break;
	case IDS_TB_05:
		if (nSubCommand == IDS_DELPOS_FRONT || nSubCommand == IDS_DELPOS_REAR)
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
	case IDS_TB_18: // Add Extension
		if (m_strReturn1.IsEmpty()) return FALSE;
		break;
	case IDS_TB_19: // Replace Extension
		if (m_strReturn2.IsEmpty()) return FALSE;
		break;
	case IDS_PRESET_NAME: // Add Extension
		if (m_strReturn1.IsEmpty()) return FALSE;
		break;
	}
	return TRUE;
}
