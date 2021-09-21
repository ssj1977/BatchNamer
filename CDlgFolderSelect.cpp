// CDlgFolderSelect.cpp: 구현 파일
//

#include "pch.h"
#include "BatchNamer.h"
#include "CDlgFolderSelect.h"
#include "afxdialogex.h"


// CDlgFolderSelect 대화 상자

IMPLEMENT_DYNAMIC(CDlgFolderSelect, CDialogEx)

CDlgFolderSelect::CDlgFolderSelect(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FOLDER_SELECT, pParent)
{
	m_nLevel = 1;
	m_bUseParent = FALSE;
}

CDlgFolderSelect::~CDlgFolderSelect()
{
}

void CDlgFolderSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgFolderSelect, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_FOLDER_SELECT, &CDlgFolderSelect::OnBnClickedBtnFolderSelect)
	ON_BN_CLICKED(IDC_RADIO_PARENT_FOLDER, &CDlgFolderSelect::OnBnClickedRadioParentFolder)
	ON_BN_CLICKED(IDC_RADIO_SPECIFIC_FOLDER, &CDlgFolderSelect::OnBnClickedRadioSpecificFolder)
END_MESSAGE_MAP()


// CDlgFolderSelect 메시지 처리기
BOOL CDlgFolderSelect::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString strLevel; strLevel.Format(L"%d", m_nLevel);
	SetDlgItemText(IDC_EDIT_PARENT_LEVEL, strLevel);
	SetDlgItemText(IDC_EDIT_PARENT_FOLDER, m_strFolder);
	((CButton*)GetDlgItem(IDC_RADIO_PARENT_FOLDER))->SetCheck(m_bUseParent);
	((CButton*)GetDlgItem(IDC_RADIO_SPECIFIC_FOLDER))->SetCheck(!m_bUseParent);
	if (m_bUseParent == TRUE)	OnBnClickedRadioParentFolder();
	else						OnBnClickedRadioSpecificFolder();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgFolderSelect::OnOK()
{
	m_bUseParent = ((CButton*)GetDlgItem(IDC_RADIO_PARENT_FOLDER))->GetCheck();

	CString strTemp;
	if (m_bUseParent == TRUE)
	{
		GetDlgItemText(IDC_EDIT_PARENT_LEVEL, strTemp); m_nLevel = _ttoi(strTemp);
		if (m_nLevel < 1) m_nLevel = 1;
	}
	else
	{
		GetDlgItemText(IDC_EDIT_PARENT_FOLDER, m_strFolder);
		m_strFolder.Trim();
		DWORD dwAttribute = GetFileAttributes(m_strFolder);
		BOOL bIsDirectory = FALSE;
		if (dwAttribute != -1)
		{
			bIsDirectory = (dwAttribute & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
		}
		if (bIsDirectory == FALSE)
		{
			CString strMsg;
			CString strTemp; strTemp.LoadString(IDS_INVALID_FOLDER);
			strMsg.Format(L"%s:\n", strTemp, m_strFolder);
			AfxMessageBox(strMsg, MB_OK);
			return;
		}
	}


	CDialogEx::OnOK();
}


void CDlgFolderSelect::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


void CDlgFolderSelect::OnBnClickedBtnFolderSelect()
{
	CFolderPickerDialog dlg;
	CString strTitle;
	strTitle.LoadString(IDS_FOLDER_SPECIFIC);
	dlg.GetOFN().lpstrTitle = strTitle;
	if (dlg.DoModal() == IDCANCEL) return;
	SetDlgItemText(IDC_EDIT_PARENT_FOLDER, dlg.GetPathName());
}


void CDlgFolderSelect::OnBnClickedRadioParentFolder()
{
	GetDlgItem(IDC_EDIT_PARENT_LEVEL)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_PARENT_FOLDER)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_FOLDER_SELECT)->EnableWindow(FALSE);
}


void CDlgFolderSelect::OnBnClickedRadioSpecificFolder()
{
	GetDlgItem(IDC_EDIT_PARENT_LEVEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_PARENT_FOLDER)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_FOLDER_SELECT)->EnableWindow(TRUE);
}
