#pragma once
#include "afxwin.h"

#ifndef IDM_ARRANGECTRL
#define IDM_ARRANGECTRL 36001
#endif 

class CWndDragBar : public CStatic
{
public:
	CWndDragBar();
	virtual ~CWndDragBar();
	BOOL CreateDragBar(BOOL bVertical, CWnd* pParentWnd, UINT nID);

	BOOL m_bVertical;
	BOOL m_bDragging;
	int* m_pOldSize; //바의 크기 포인터 / 현재는 가로창만
	int m_nParentCommand;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

