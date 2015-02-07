#pragma once

class CGraghicWnd;
// CListCtrlEx

class CListCtrlEx : public CListCtrl
{
	DECLARE_DYNAMIC(CListCtrlEx)

public:
	CListCtrlEx();
	virtual ~CListCtrlEx();

	void SetParent( CGraghicWnd *pParent );

private:
	CGraghicWnd *mParent;

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
};


