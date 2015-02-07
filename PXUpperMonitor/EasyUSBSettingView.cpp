// EasyUSBSettingView.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "EasyUSBSettingView.h"
#include "MainFrm.h"
#include "HSLogProtocol.h"
#include "EasyUSBSettingDoc.h"
#include "USBHandler.h"

const int WM_VALUE_INCORRCET_MESSAGE = WM_USER + 9;

CString CEasyUSBSettingView::mStrRate[ 5 ] = { "100K", "200K", "500K", "1M", "2M" };

IMPLEMENT_DYNCREATE(CEasyUSBSettingView, CFormView)

	CEasyUSBSettingView::CEasyUSBSettingView()
	: CFormView(CEasyUSBSettingView::IDD), mAutoAlignManager( this, CRect( 0, 0, 100, 100 ) )
{
	;
}

CEasyUSBSettingView::~CEasyUSBSettingView()
{
	;
}

void CEasyUSBSettingView::SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier )
{
	CString tStr;
	HSDouble tUnit = 1000000.0;
	HSDouble tTimes = 100;
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	mCardIdentifier = tCardIdentifier; 

	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( tCardIdentifier );	
	CUSBHandler *pUSBHandler = dynamic_cast< CUSBHandler * >( pDataHandler );

	this->mPropertyGrid.RemoveAll();

	this->mPropertyGrid.EnableHeaderCtrl( false );
	this->mPropertyGrid.EnableDescriptionArea();
	this->mPropertyGrid.SetVSDotNetLook();
	this->mPropertyGrid.MarkModifiedProperties();

	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("EZSUB设置"));

	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(_T("通道1采样率"),_T(""), _T("设置通道1的采样率"), PROPERTY_EZUSB_RATEONE );
	for ( int i = 0; i < sizeof( mStrRate ) / sizeof( CString ); i++ )
	{
		pProp->AddOption( mStrRate[ i ] );
	}

	DEVICE_CH_IDENTIFIER tIdentifierCH1 = mCardIdentifier;
	tIdentifierCH1.InitChannel( 0 );
	if ( pUSBHandler->SampleRate( tIdentifierCH1 ) >= 1000000 )
	{
		tStr.Format("%dM", pUSBHandler->SampleRate( tIdentifierCH1 ) / 1000000);
	}
	else
	{
		tStr.Format("%dK", pUSBHandler->SampleRate( tIdentifierCH1 ) / 1000);
	}
	pProp->SetValue( tStr );
	pProp->AllowEdit(false);
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("通道2采样率"), _T(""), _T("设置通道2的采样率"), PROPERTY_EZUSB_RATETWO );
	for ( int i = 0; i < sizeof( mStrRate ) / sizeof( CString ); i++ )
	{
		pProp->AddOption( mStrRate[ i ] );
	}
	DEVICE_CH_IDENTIFIER tIdentifierCH2 = mCardIdentifier;
	tIdentifierCH2.InitChannel( 1 );
	if ( pUSBHandler->SampleRate( tIdentifierCH1 ) >= 1000000 )
	{
		tStr.Format("%dM", pUSBHandler->SampleRate( tIdentifierCH2 )/ 1000000);
	}
	else
	{
		tStr.Format("%dK", pUSBHandler->SampleRate( tIdentifierCH2 )/ 1000);
	}
	pProp->SetValue( tStr );
	pProp->AllowEdit(false);
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("通道1门槛"), _T(""), _T("设置通道1的门槛"), PROPERTY_THRESHOLD_ONE );
	for ( int i = 10; i <= 100; i++ )
	{
		tStr.Format("%ddB", i ); 
		pProp->AddOption( tStr );
	}
	tStr.Format( "%ddB",HSInt( 20 * log10( pUSBHandler->BypassVoltage( 0 ) * tUnit / tTimes ) ) );
	pProp->SetValue( tStr );
	pProp->AllowEdit( false );
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("通道2门槛"), _T(""), _T("设置通道2的门槛"), PROPERTY_THRESHOLD_TWO );
	for ( int i = 10; i <= 100; i++ )
	{
		tStr.Format( "%ddB", i );
		pProp->AddOption( tStr );
	}
	tStr.Format( "%ddB",HSInt( 20 * log10( pUSBHandler->BypassVoltage( 1 ) * tUnit / tTimes ) ) );
	pProp->SetValue( tStr );
	pProp->AllowEdit( false );
	pGroup->AddSubItem( pProp );

	this->mPropertyGrid.AddProperty( pGroup );

	CString tTitle;
	tTitle.Format( "%s 设置", pDataHandler->Name().c_str() );

	CEasyUSBSettingDoc *pDoc = dynamic_cast< CEasyUSBSettingDoc * >( this->GetDocument() );
	pDoc->SetCardIdentifier( mCardIdentifier, tTitle );

}

void CEasyUSBSettingView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID1, mPropertyGrid);
}

BEGIN_MESSAGE_MAP(CEasyUSBSettingView, CFormView)
	ON_MESSAGE( WM_VALUE_INCORRCET_MESSAGE, OnValueIncorrect )
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BTN_OK, &CEasyUSBSettingView::OnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CEasyUSBSettingView::OnClickedBtnCancel)
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
END_MESSAGE_MAP()


// CEasyUSBSettingView diagnostics

#ifdef _DEBUG
void CEasyUSBSettingView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CEasyUSBSettingView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CEasyUSBSettingView message handlers


void CEasyUSBSettingView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class

	mAutoAlignManager.AddControl( IDC_BTN_OK, CRect( -1, -1, 10, 15 ) );
	mAutoAlignManager.AddControl( IDC_BTN_CANCEL, CRect( -1, -1, 100, 15 ) );
	this->mAutoAlignManager.AddControl( IDC_MFCPROPERTYGRID1, CRect( 0, 0, 0, 100 ));
}


void CEasyUSBSettingView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	mAutoAlignManager.Resize();
}


void CEasyUSBSettingView::OnLButtonUp(UINT nFlags, CPoint point)
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


void CEasyUSBSettingView::OnClickedBtnOk()
{
	CString tStr;
	HSInt tVal;
	HSDouble tUnit = 1000000.0;
	HSDouble tTimes = 100;

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());		

	DEVICE_CH_IDENTIFIER tIdentifierCH1 = mCardIdentifier;
	tIdentifierCH1.InitChannel( 0 );

	DEVICE_CH_IDENTIFIER tIdentifierCH2 = mCardIdentifier;
	tIdentifierCH2.InitChannel( 1 );

	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( mCardIdentifier );	
	if ( pDataHandler == NULL )
	{
		MessageBox( "设置失败!", "警告" );
		return;
	}

	CUSBHandler *pUSBHandler = dynamic_cast< CUSBHandler * >( pDataHandler );
	
	if ( pMainFrame->ControlState() != CMainFrame::GRAPHIC_STOP )
	{
		MessageBox( "正在采样或回放中，参数设置失败!", "警告" );
		return;
	}

	CMFCPropertyGridProperty* pProp = this->mPropertyGrid.FindItemByData( PROPERTY_THRESHOLD_ONE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> tVal;
		pUSBHandler->SetBypassVoltage( 0, pow( 10.0, (  tVal ) / 20.0 ) / tUnit * tTimes );
		tStr.Format( "Value: %d\n", tVal );
		OutputDebugStringA( tStr ) ;
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_THRESHOLD_TWO );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> tVal;
		pUSBHandler->SetBypassVoltage( 1, pow( 10.0, (  tVal ) / 20.0 ) / tUnit * tTimes );
	}	

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_EZUSB_RATEONE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();

		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> tVal;
		if ( tStr.Find( 'M') != -1 )
		{
			tVal *= 1000000;
		}
		else
		{
			tVal *= 1000;
		}
		pUSBHandler->SetSampleRate( 0, tVal );
	}
	
	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_EZUSB_RATETWO );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> tVal;
		if ( tStr.Find( 'M') != -1 )
		{
			tVal *= 1000000;
		}
		else
		{
			tVal *= 1000;
		}
		pUSBHandler->SetSampleRate( 1, tVal );
	}	

	MessageBox( "参数已设置!", "警告" );
}


void CEasyUSBSettingView::OnClickedBtnCancel()
{
	// TODO: Add your control notification handler code here
	this->GetDocument()->OnCloseDocument();
}

BOOL CEasyUSBSettingView::DigcheckOnly( CString tStr )
{
	if ( tStr.GetLength() < 0 )
	{
		return 0;
	}

	for ( int i = 0; i < tStr.GetLength(); i++ )
	{
		if ( tStr[ i ] < '0' || tStr[ i ] > '9' )
		{
			return 0;
		}
	}

	return 1;
}

LRESULT CEasyUSBSettingView::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( this->mCardIdentifier );	
	CUSBHandler *pUSBHandler = dynamic_cast< CUSBHandler * >( pDataHandler );

	HSBool tRateSetEnabled = pMainFrame->IsSettingArgEnabled();	

	CString tStr;
	CMFCPropertyGridProperty* pProp = ( CMFCPropertyGridProperty * )lparam;
	HSInt tVal = pProp->GetData();
	tStr = pProp->GetValue();

	HSInt tChannel = -1;

	switch ( tVal )
	{
	case PROPERTY_EZUSB_RATEONE:	
		tChannel = 0;
		break;

	case PROPERTY_EZUSB_RATETWO:
		tChannel = 1;
		break;

	default:
		break;
	}	

	if ( tChannel >= 0 && !tRateSetEnabled )
	{
		mIncorrect = "回放模式，参数设置失败!";

		DEVICE_CH_IDENTIFIER tIdentifierCH = mCardIdentifier;
		tIdentifierCH.InitChannel( tChannel );
		if ( pUSBHandler->SampleRate( tIdentifierCH ) >= 1000000 )
		{
			tStr.Format("%dM", pUSBHandler->SampleRate( tIdentifierCH ) / 1000000);
		}
		else
		{
			tStr.Format("%dK", pUSBHandler->SampleRate( tIdentifierCH ) / 1000);
		}

		pProp->SetValue( tStr );
		pProp->Redraw();

		this->PostMessageA( WM_VALUE_INCORRCET_MESSAGE , 0, 0 );
	}

	return 1;
}

LRESULT CEasyUSBSettingView::OnValueIncorrect( WPARAM wparam, LPARAM lparam )
{
	AfxMessageBox( mIncorrect );

	return 1;
}