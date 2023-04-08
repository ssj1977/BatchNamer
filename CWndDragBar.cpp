#include "pch.h"
//#include "BatchNamer.h"
//#include "BatchNamerDlg.h"
#include "CWndDragBar.h"


CWndDragBar::CWndDragBar()
{
	m_bDragging = FALSE;
	m_pOldSize = NULL;
	m_bVertical = TRUE;
	m_nParentCommand = IDM_ARRANGECTRL;
}

CWndDragBar::~CWndDragBar()
{
}

BOOL CWndDragBar::CreateDragBar(BOOL bVertical, CWnd* pParentWnd, UINT nID)
{
	DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_DLGFRAME | SS_NOTIFY | SS_ETCHEDFRAME;
	RECT rect = CRect (0,0,0,0);
	CCreateContext* pContext = NULL;
	m_bVertical = bVertical;
	return CStatic::Create(NULL, dwStyle, rect, pParentWnd, nID);
}


BEGIN_MESSAGE_MAP(CWndDragBar, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


void CWndDragBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	m_bDragging = TRUE;
	CStatic::OnLButtonDown(nFlags, point);
}


void CWndDragBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	m_bDragging = FALSE;
	CStatic::OnLButtonUp(nFlags, point);
}

BOOL CWndDragBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nHitTest == HTCLIENT || nHitTest == HTBORDER)
	{
		if (m_bVertical)	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
		else				::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
		return TRUE;
	}
	return CStatic::OnSetCursor(pWnd, nHitTest, message);
}


void CWndDragBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDragging && m_pOldSize)
	{
		int& nOldSize = (*m_pOldSize);
		int nGap = m_bVertical ? point.y : point.x;
		int nNewSize = nOldSize - nGap;
		if (abs(nGap) > 5)
		{
			nOldSize = nNewSize;
			GetParent()->SendMessage(WM_COMMAND, m_nParentCommand);
		}
	}
	CStatic::OnMouseMove(nFlags, point);
}
