#pragma once

class CArgTable;

// CArgListCtrl

class CArgListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CArgListCtrl)

public:
	CArgListCtrl();
	virtual ~CArgListCtrl();

public:
	void SetParent( CArgTable *pParent ){ mParent = pParent; }

private:
	CArgTable *mParent;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};


