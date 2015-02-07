// PCI1714SettingView.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "PCI1714SettingView.h"
#include "MainFrm.h"
#include "PCI1714SettingDoc.h"
#include "HSLogProtocol.h"


// CPCI1714SettingView

IMPLEMENT_DYNCREATE(CPCI1714SettingView, CFormView)

	const int WM_VALUE_INCORRCET_MESSAGE = WM_USER + 8;

CString CPCI1714SettingView::mstrRate[ 20 ] = { "1M", "2M", "3M", "4M", "5M", "6M", "7M", "8M", "9M", "10M", "11M", "12M", "13M",
	"14M", "15M", "16M", "17M", "18M", "19M", "20M" };

CPCI1714SettingView::CPCI1714SettingView()
	: CFormView(CPCI1714SettingView::IDD), mAutoAlignManager( this, CRect( 0, 0, 100, 100) )
{
	mIsInit = HSFalse;
}

CPCI1714SettingView::~CPCI1714SettingView()
{
}

void CPCI1714SettingView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID1, mPropertyGrid);
}

BEGIN_MESSAGE_MAP(CPCI1714SettingView, CFormView)
	ON_MESSAGE( WM_VALUE_INCORRCET_MESSAGE, OnValueIncorrect )
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_OK, &CPCI1714SettingView::OnBnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CPCI1714SettingView::OnBnClickedBtnCancel)
	ON_WM_LBUTTONUP()
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
END_MESSAGE_MAP()


// CPCI1714SettingView diagnostics

#ifdef _DEBUG
void CPCI1714SettingView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPCI1714SettingView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPCI1714SettingView message handlers


void CPCI1714SettingView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);
	this->mAutoAlignManager.Resize();

}


void CPCI1714SettingView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	this->mAutoAlignManager.AddControl( IDC_BTN_OK, CRect( -1, -1, 100, 15 ));
	this->mAutoAlignManager.AddControl( IDC_BTN_CANCEL, CRect( -1, -1, 10, 15 ));
	this->mAutoAlignManager.AddControl( IDC_MFCPROPERTYGRID1, CRect( 0, 0, 0, 100 ));

}


void CPCI1714SettingView::OnBnClickedBtnOk()
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
	CString tStr;
	HSInt tVal;
	HSDouble tUnit = 1000000.0;
	HSDouble tTimes = 100;
	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( mCardIdentifier );	
	if ( pDataHandler == NULL )
	{
		MessageBox( "设置失败!", "警告" );
		return;
	}

	CAdPCIHandler *pAdPCIHandler = dynamic_cast< CAdPCIHandler * >( pDataHandler );

	CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( PROPERTY_PCI1714_RATE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream  Istring( ( LPCSTR ) tStr );
		Istring >> tVal;
		if ( tStr.Find( 'M') != -1 )
		{
			tVal *= 1000000;
		}

		pAdPCIHandler->SetParam(-1, tVal, -1 );
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_THRESHOLD_ONE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream  Istring( ( LPCSTR ) tStr );
		Istring >> tVal;
		pAdPCIHandler->SetBypassVoltage( 0, pow( 10.0, (  tVal ) / 20.0 ) / tUnit * tTimes );
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_THRESHOLD_TWO );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream  Istring( ( LPCSTR ) tStr );
		Istring >> tVal;
		pAdPCIHandler->SetBypassVoltage( 1, pow( 10.0, (  tVal ) / 20.0 ) / tUnit * tTimes );
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_THRESHOLD_THREE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream  Istring( ( LPCSTR ) tStr );
		Istring >> tVal;
		pAdPCIHandler->SetBypassVoltage( 2, pow( 10.0, (  tVal ) / 20.0 ) / tUnit * tTimes );
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_THRESHOLD_FOUR );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream  Istring( ( LPCSTR ) tStr );
		Istring >> tVal;
		pAdPCIHandler->SetBypassVoltage( 3, pow( 10.0, (  tVal ) / 20.0 ) / tUnit * tTimes );
	}

	if ( pMainFrame->ControlState() != CMainFrame::GRAPHIC_STOP )
	{
		MessageBox( "正在采样或回放中，参数设置失败!", "警告" );
		return;
	}

	HS_INFO("%d, %lf, %lf, %lf, %lf\n", pAdPCIHandler->SampleRate( this->mCardIdentifier),pAdPCIHandler->BypassVoltage( 0 ),  pAdPCIHandler->BypassVoltage( 1 ), pAdPCIHandler->BypassVoltage( 2 ), pAdPCIHandler->BypassVoltage( 3 ));

	MessageBox( "参数已设置!", "警告" );
}


void CPCI1714SettingView::OnBnClickedBtnCancel()
{
	// TODO: Add your control notification handler code here
	this->GetDocument()->OnCloseDocument();
}

void CPCI1714SettingView::SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier )
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	mCardIdentifier = tCardIdentifier; 

	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( tCardIdentifier );	
	CAdPCIHandler *pAdPCIHandler = dynamic_cast< CAdPCIHandler * >( pDataHandler );

	CString tStr;
	HSDouble tUnit = 1000000.0;
	HSDouble tTimes = 100;

	this->mPropertyGrid.RemoveAll();

	this->mPropertyGrid.EnableHeaderCtrl( FALSE );
	this->mPropertyGrid.EnableDescriptionArea();
	this->mPropertyGrid.SetVSDotNetLook();
	this->mPropertyGrid.MarkModifiedProperties();

	CMFCPropertyGridProperty * pGroup = new CMFCPropertyGridProperty( _T("PCI1714设置" )) ;

	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty( _T("采样率"), _T(""), _T("设置采样率"), PROPERTY_PCI1714_RATE );
	for ( int i = 0; i < sizeof( mstrRate ) / sizeof( CString ); i++ )
	{
		pProp->AddOption( mstrRate[ i ] );
	}
	if ( pAdPCIHandler->SampleRate( 0 ) >= 1000000 )
	{
		tStr.Format( "%dM", pAdPCIHandler->SampleRate( 0 ) / 1000000 );
	}
	else
	{
		tStr.Format( "%dK", pAdPCIHandler->SampleRate( 0 ) / 1000 );
	}
	pProp->SetValue( tStr );
	pProp->AllowEdit(false);
	pGroup->AddSubItem( pProp);

	pProp = new CMFCPropertyGridProperty(_T("通道1门槛"), _T(""),_T("设置通道1门槛"), PROPERTY_THRESHOLD_ONE );
	for ( int i = 10; i <= 100; i++ )
	{
		tStr.Format("%ddB", i );
		pProp->AddOption( tStr );
	}
	tStr.Format( "%ddB",HSInt( 20 * log10( pAdPCIHandler->BypassVoltage( 0 ) * tUnit / tTimes ) ) );
	pProp->SetValue( tStr );
	pProp->AllowEdit(false);
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("通道2门槛"), _T(""), _T("设置通道2门槛"), PROPERTY_THRESHOLD_TWO );
	for ( int i = 10; i <= 100; i++ )
	{
		tStr.Format("%ddB", i );
		pProp->AddOption( tStr );
	}
	tStr.Format( "%ddB", HSInt( 20 * log10( pAdPCIHandler->BypassVoltage( 1 ) * tUnit / tTimes ) ) );
	pProp->SetValue( tStr );
	pProp->AllowEdit(false);
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("通道3门槛"), _T(""), _T("设置通道3门槛"), PROPERTY_THRESHOLD_THREE );
	for ( int i = 10; i <= 100; i++ )
	{
		tStr.Format("%ddB", i );
		pProp->AddOption( tStr );
	}
	tStr.Format( "%ddB", HSInt( 20 * log10( pAdPCIHandler->BypassVoltage( 2 ) * tUnit / tTimes ) ) );
	pProp->SetValue( tStr );
	pProp->AllowEdit(false);
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("通道4门槛"), _T(""), _T("设置通道4门槛"), PROPERTY_THRESHOLD_FOUR );
	for ( int i = 10; i <= 100; i++ )
	{
		tStr.Format( "%ddB", i );
		pProp->AddOption( tStr );
	}
	tStr.Format( "%ddB", HSInt( 20 * log10( pAdPCIHandler->BypassVoltage( 3 ) * tUnit / tTimes ) ) );
	pProp->SetValue( tStr );
	pProp->AllowEdit(false);
	pGroup->AddSubItem( pProp );

	this->mPropertyGrid.AddProperty( pGroup );

	CString tTitle;
	tTitle.Format( "%s 设置", pDataHandler->Name().c_str() );

	CPCI1714SettingDoc *pDoc = dynamic_cast< CPCI1714SettingDoc * >( this->GetDocument() );
	pDoc->SetCardIdentifier( mCardIdentifier, tTitle );



}

void CPCI1714SettingView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( pMainFrame->DragingGraphic() != "" )
	{		
		pMainFrame->SetSelectedGraphic( "" );
		pMainFrame->SetDragingGraphic( "" );
	}

	__super::OnLButtonUp(nFlags, point);
}

BOOL CPCI1714SettingView::DigcheckOnly( CString tStr )
{
	if ( tStr.GetLength() < 0 )
	{
		return 0;
	}

	for ( int i = 0; i < tStr.GetLength() ; i++ )
	{
		if ( tStr[ i ] < '0' || tStr[ i ] > '9')
		{
			return 0;
		}
	}

	return 1;
}

LRESULT CPCI1714SettingView::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{
	CString tStr;
	CMFCPropertyGridProperty* pProp = ( CMFCPropertyGridProperty * )lparam;
	HSInt tVal = pProp->GetData( );

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( this->mCardIdentifier );	
	CAdPCIHandler *pAdPCIHandler = dynamic_cast< CAdPCIHandler * >( pDataHandler );

	HSBool tRateSetEnabled = pMainFrame->IsSettingArgEnabled();	

	switch ( tVal )
	{
	case PROPERTY_PCI1714_RATE:
		if ( !tRateSetEnabled )
		{
			mIncorrect = "回放模式，参数设置失败!";

			if ( pAdPCIHandler->SampleRate( 0 ) >= 1000000 )
			{
				tStr.Format("%dM", pAdPCIHandler->SampleRate( 0 ) / 1000000);
			}
			else
			{
				tStr.Format("%dK", pAdPCIHandler->SampleRate( 0 ) / 1000);
			}

			pProp->SetValue( tStr );
			pProp->Redraw();

			this->PostMessageA(WM_VALUE_INCORRCET_MESSAGE, 0,  0);
		}
		break;

	default:
		break;
	}


	return 1;
}

LRESULT CPCI1714SettingView::OnValueIncorrect( WPARAM wparam, LPARAM lparam )
{
	AfxMessageBox( mIncorrect );

	return 1;
}