// ArgListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "ArgListCtrl.h"
#include "ArgTable.h"


// CArgListCtrl

IMPLEMENT_DYNAMIC(CArgListCtrl, CListCtrl)

CArgListCtrl::CArgListCtrl()
{

}

CArgListCtrl::~CArgListCtrl()
{
}


BEGIN_MESSAGE_MAP(CArgListCtrl, CListCtrl)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// CArgListCtrl message handlers




void CArgListCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	mParent->OnLButtonUp( nFlags, point );

	CListCtrl::OnLButtonUp(nFlags, point);
}


void CArgListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	mParent->OnMouseMove( nFlags, point );

	CListCtrl::OnMouseMove(nFlags, point);
}


void CArgListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	mParent->FocusWnd();

	CListCtrl::OnLButtonDown(nFlags, point);
}
