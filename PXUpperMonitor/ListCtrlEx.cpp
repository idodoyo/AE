// ListCtrlEx.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "ListCtrlEx.h"
#include "GraghicWnd.h"


// CListCtrlEx

IMPLEMENT_DYNAMIC(CListCtrlEx, CListCtrl)

CListCtrlEx::CListCtrlEx()
{

}

CListCtrlEx::~CListCtrlEx()
{
}


BEGIN_MESSAGE_MAP(CListCtrlEx, CListCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(NM_CLICK, &CListCtrlEx::OnNMClick)
END_MESSAGE_MAP()



// CListCtrlEx message handlers
void CListCtrlEx::SetParent( CGraghicWnd *pParent )
{
	mParent = pParent;
}

void CListCtrlEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	mParent->OnLButtonDown(nFlags, point);

//	CListCtrl::OnLButtonDown(nFlags, point);
}


void CListCtrlEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	
	mParent->OnLButtonUp(nFlags, point);

	CListCtrl::OnLButtonUp(nFlags, point);
}


void CListCtrlEx::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
/*
	CPoint tPoint;	
	GetCursorPos( &tPoint );    
	this->ScreenToClient( &tPoint ); 

	UINT nFlags = 0;

	mParent->OnLButtonUp(nFlags, tPoint);
	*/
	*pResult = 0;
}
