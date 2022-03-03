// CDlgCFG_View.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "BatchNamer.h"
#include "CDlgCFG_View.h"
#include "EtcFunctions.h"
#include <afxdialogex.h>
#include <afxcolorbutton.h>


static CString GetLogFontInfoString(LOGFONT& ft)
{
	CString str = ft.lfFaceName;
	if (ft.lfWeight > FW_NORMAL) str += _T("/ 굵게");
	else if (ft.lfWeight < FW_NORMAL) str += _T("/ 얇게");
	if (ft.lfItalic != FALSE) str += _T("/ 이탤릭체");
	return str;
}
// CDlgCFG_View 대화 상자

IMPLEMENT_DYNAMIC(CDlgCFG_View, CDialogEx)

CDlgCFG_View::CDlgCFG_View(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CFG_VIEW, pParent)
{
	m_clrText = RGB(0, 0, 0);
	m_clrBk = RGB(255, 255, 255);
	m_bUseDefaultColor = TRUE;
	m_nFontSize = 12;
	m_bUseDefaultFont = TRUE;
	m_nIconType = SHIL_SMALL;
	m_bUpdateFont = FALSE;
}

CDlgCFG_View::~CDlgCFG_View()
{
}

void CDlgCFG_View::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgCFG_View, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT_COLOR, &CDlgCFG_View::OnBnClickedCheckDefaultColor)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT_FONT, &CDlgCFG_View::OnBnClickedCheckDefaultFont)
	ON_BN_CLICKED(IDC_BTN_FONT, &CDlgCFG_View::OnBnClickedBtnFont)
END_MESSAGE_MAP()


// CDlgCFG_View 메시지 처리기


BOOL CDlgCFG_View::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	SetCheckByID(this, IDC_CHECK_DEFAULT_COLOR, m_bUseDefaultColor);
	CMFCColorButton* pColorBk = (CMFCColorButton*)GetDlgItem(IDC_COLOR_BK);
	pColorBk->SetColor(m_clrBk);
	CMFCColorButton* pColorText = (CMFCColorButton*)GetDlgItem(IDC_COLOR_TEXT);
	pColorText->SetColor(m_clrText);

	SetCheckByID(this, IDC_CHECK_DEFAULT_FONT, m_bUseDefaultFont);
	CString strTemp;
	strTemp.Format(_T("%d"), m_nFontSize);
	GetDlgItem(IDC_EDIT_FONTSIZE)->SetWindowText(strTemp);
	GetDlgItem(IDC_EDIT_FONTNAME)->SetWindowText(GetLogFontInfoString(m_lf));

	CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_ICONSIZE);
	int nIndex = 0;
	nIndex = pCB->AddString(IDSTR(IDS_ICON_SMALL));
	pCB->SetItemData(nIndex, SHIL_SMALL);
	nIndex = pCB->AddString(IDSTR(IDS_ICON_LARGE));
	pCB->SetItemData(nIndex, SHIL_LARGE);
	nIndex = pCB->AddString(IDSTR(IDS_ICON_EXLARGE));
	pCB->SetItemData(nIndex, SHIL_EXTRALARGE);
	nIndex = pCB->AddString(IDSTR(IDS_ICON_JUMBO));
	pCB->SetItemData(nIndex, SHIL_JUMBO);
 
	for (int i = 0; pCB->GetCount(); i++)
	{
		if (pCB->GetItemData(i) == m_nIconType)
		{
			pCB->SetCurSel(i);
			break;
		}
	}
	
	UpdateControl();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgCFG_View::UpdateControl()
{
	GetDlgItem(IDC_COLOR_BK)->EnableWindow(!m_bUseDefaultColor);
	GetDlgItem(IDC_ST_COLOR_BK)->EnableWindow(!m_bUseDefaultColor);
	GetDlgItem(IDC_COLOR_TEXT)->EnableWindow(!m_bUseDefaultColor);
	GetDlgItem(IDC_ST_COLOR_TEXT)->EnableWindow(!m_bUseDefaultColor);
	GetDlgItem(IDC_EDIT_FONTSIZE)->EnableWindow(!m_bUseDefaultFont);
	GetDlgItem(IDC_EDIT_FONTNAME)->EnableWindow(!m_bUseDefaultFont);
	GetDlgItem(IDC_BTN_FONT)->EnableWindow(!m_bUseDefaultFont);
	GetDlgItem(IDC_ST_FONTSIZE)->EnableWindow(!m_bUseDefaultFont);
	GetDlgItem(IDC_ST_FONTNAME)->EnableWindow(!m_bUseDefaultFont);
}


void CDlgCFG_View::OnOK()
{
	CMFCColorButton* pColorBk = (CMFCColorButton*)GetDlgItem(IDC_COLOR_BK);
	m_clrBk = pColorBk->GetColor();
	CMFCColorButton* pColorText = (CMFCColorButton*)GetDlgItem(IDC_COLOR_TEXT);
	m_clrText = pColorText->GetColor();
	m_bUseDefaultColor = IsChecked(this, IDC_CHECK_DEFAULT_COLOR);
	m_bUseDefaultFont = IsChecked(this, IDC_CHECK_DEFAULT_FONT);
	CString strTemp;
	GetDlgItem(IDC_EDIT_FONTSIZE)->GetWindowText(strTemp);
	m_nFontSize = _ttoi(strTemp);
	if (m_nFontSize < 1 || m_nFontSize > 100)
	{
		AfxMessageBox(IDSTR(IDS_MSG_INVALIDFONTSIZE));
		return;
	}
	CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_ICONSIZE);
	m_nIconType = (int)pCB->GetItemData(pCB->GetCurSel());
	CDialogEx::OnOK();
}


void CDlgCFG_View::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}



void CDlgCFG_View::OnBnClickedCheckDefaultColor()
{
	m_bUseDefaultColor = !m_bUseDefaultColor;
	UpdateControl();
}


void CDlgCFG_View::OnBnClickedCheckDefaultFont()
{
	m_bUseDefaultFont = !m_bUseDefaultFont;
	UpdateControl();
}


void CDlgCFG_View::OnBnClickedBtnFont()
{
	CString strFontNameOld = m_lf.lfFaceName;
	CFontDialog dlg(&m_lf, CF_SCREENFONTS);
	if (dlg.DoModal() == IDOK)
	{
		dlg.GetCurrentFont(&m_lf);
		CString strFontNameNew = m_lf.lfFaceName;
		if (strFontNameNew.IsEmpty())
		{
			_tcsncpy_s(m_lf.lfFaceName, LF_FACESIZE, strFontNameOld, _TRUNCATE);
		}
		GetDlgItem(IDC_EDIT_FONTNAME)->SetWindowText(GetLogFontInfoString(m_lf));
		int nFontSize = MulDiv(-1 * m_lf.lfHeight, 72, GetDeviceCaps(GetDC()->GetSafeHdc(), LOGPIXELSY));
		GetDlgItem(IDC_EDIT_FONTSIZE)->SetWindowText(INTtoSTR(nFontSize));
		m_bUpdateFont = TRUE;
	}
}
