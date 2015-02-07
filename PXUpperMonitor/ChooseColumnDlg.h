#pragma once


#include "DataHandle\DataTypes.h"

// CChooseColumnDlg dialog

class CChooseColumnDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChooseColumnDlg)

public:
	CChooseColumnDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChooseColumnDlg();

// Dialog Data
	enum { IDD = IDD_CHOOSECOLUMNDLG };

public:
	HSInt ColumnNum();

private:
	HSInt mColumnNum;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
