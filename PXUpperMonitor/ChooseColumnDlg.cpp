// ChooseColumnDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "ChooseColumnDlg.h"
#include "afxdialogex.h"


// CChooseColumnDlg dialog

IMPLEMENT_DYNAMIC(CChooseColumnDlg, CDialogEx)

CChooseColumnDlg::CChooseColumnDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CChooseColumnDlg::IDD, pParent)
{

}

CChooseColumnDlg::~CChooseColumnDlg()
{
}

void CChooseColumnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChooseColumnDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CChooseColumnDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CChooseColumnDlg message handlers


void CChooseColumnDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString tStrValue;
	GetDlgItemText( IDC_EDIT_COLUMN_NUM, tStrValue );
	mColumnNum = atoi( tStrValue );
	if ( mColumnNum < 1 )
	{
		MessageBox( "无效的列!", "警告" );
		return;
	}

	CDialogEx::OnOK();
}

HSInt CChooseColumnDlg::ColumnNum()
{
	return mColumnNum;
}

BOOL CChooseColumnDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CSpinButtonCtrl *pSpin = ( CSpinButtonCtrl * )GetDlgItem( IDC_SPIN_COLUMN );
	pSpin->SetRange( 1, 10 );
	pSpin->SetBuddy( GetDlgItem( IDC_EDIT_COLUMN_NUM ) );
	//UDACCEL tAccel;
	//pSpin->SetAccel( 1, &tAccel );

	GetDlgItem( IDC_EDIT_COLUMN_NUM )->SetWindowText( "3" );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
