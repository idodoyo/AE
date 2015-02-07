// UA306SettingView.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "UA306SettingView.h"
#include "MainFrm.h"
#include "UA306SettingDoc.h"
#include "HSLogProtocol.h"

const int WM_VALUE_INCORRCET_MESSAGE = WM_USER + 10;

// CUA306SettingView

IMPLEMENT_DYNCREATE(CUA306SettingView, CFormView)

CUA306SettingView::CUA306SettingView()
	: CFormView(CUA306SettingView::IDD), mAutoAlignManager( this, CRect( 0, 0, 100, 100) )
{

}

CUA306SettingView::~CUA306SettingView()
{
}

void CUA306SettingView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID_PROPERTY, mPropertyGrid);
}

BEGIN_MESSAGE_MAP(CUA306SettingView, CFormView)
	ON_MESSAGE( WM_VALUE_INCORRCET_MESSAGE, OnValueIncorrect )
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
	ON_BN_CLICKED(IDC_BTN_OK, &CUA306SettingView::OnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CUA306SettingView::OnClickedBtnCancel)
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CUA306SettingView diagnostics

#ifdef _DEBUG
void CUA306SettingView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CUA306SettingView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CUA306SettingView message handlers


void CUA306SettingView::OnClickedBtnOk()
{
	// TODO: Add your control notification handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());		
	HSInt tVal;
	HSDouble tUnit = 1000000.0;
	HSDouble tTimes = 100;
	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( mCardIdentifier );	
	if ( pDataHandler == NULL )
	{
		MessageBox( "设置失败!", "警告" );
		return;
	}

	if ( pMainFrame->ControlState() != CMainFrame::GRAPHIC_STOP )
	{
		MessageBox( "正在采样或回放中，参数设置失败!", "警告" );
		return;
	}

	CUA306Handler *pUA306Handler = dynamic_cast< CUA306Handler * >( pDataHandler );

	CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( PROPERTY_RATE );
	if ( pProp != NULL )
	{
		CString tStr = pProp->GetValue();
		std::istringstream  tStream( ( LPCSTR ) tStr );
		tStream >> tVal;

		pUA306Handler->SetParam( tVal * 1000 );
	}

	for ( HSInt i = 1; i <= 8; i++ )
	{
		pProp = mPropertyGrid.FindItemByData( i * 1000 + PROPERTY_THRESHOLD );
		if ( pProp != NULL )
		{
			CString tStr = pProp->GetValue();
			std::istringstream  tStream( ( LPCSTR ) tStr );			
			tStream >> tVal;
			pUA306Handler->SetBypassVoltage( i - 1, pow( 10.0, tVal / 20.0 ) / tUnit * tTimes );
		}
	}	
	
	MessageBox( "参数已设置!", "警告" );
}

void CUA306SettingView::SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier )
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	mCardIdentifier = tCardIdentifier; 

	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( tCardIdentifier );	
	CUA306Handler *pUA306Handler = dynamic_cast< CUA306Handler * >( pDataHandler );

	CString tStr;
	HSDouble tUnit = 1000000.0;
	HSDouble tTimes = 100;

	this->mPropertyGrid.RemoveAll();

	this->mPropertyGrid.EnableHeaderCtrl( FALSE );
	this->mPropertyGrid.EnableDescriptionArea();
	this->mPropertyGrid.SetVSDotNetLook();
	this->mPropertyGrid.MarkModifiedProperties();

	CMFCPropertyGridProperty * pGroup = new CMFCPropertyGridProperty( _T("UA306设置" )) ;

	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty( _T("采样率"), _T(""), _T("设置采样率"), PROPERTY_RATE );
	CString tStrRates[] = { "25K", "50K", "100K", "200K" };
	for ( int i = 0; i < sizeof( tStrRates ) / sizeof( CString ); i++ )
	{
		pProp->AddOption( tStrRates[ i ] );
	}
	
	tStr.Format( "%dK", pUA306Handler->SampleRate( 0 ) / 1000 );	
	pProp->SetValue( tStr );
	pProp->AllowEdit( false );
	pGroup->AddSubItem( pProp );

	for ( HSInt i = 1; i <= 8; i++ )
	{
		CString tPropName;
		tPropName.Format( "通道%d门槛", i );

		CString tPropDesc;
		tPropDesc.Format( "设置通道%d门槛", i );

		CString tValue;
		tValue.Format( "%ddB",HSInt( 20 * log10( pUA306Handler->BypassVoltage( i - 1 ) * tUnit / tTimes ) ) );

		//HS_INFO( "bypass voltate: %lf, db: %lf", pUA306Handler->BypassVoltage( i - 1 ), 20 * log10( pUA306Handler->BypassVoltage( i - 1 ) * tUnit / tTimes ) );

		pProp = new CMFCPropertyGridProperty( tPropName, tValue, tPropDesc, i * 1000 + PROPERTY_THRESHOLD );
		for ( int i = 10; i <= 100; i++ )
		{
			tStr.Format("%ddB", i );
			pProp->AddOption( tStr );
		}
		
		pProp->AllowEdit( false );
		pGroup->AddSubItem( pProp );
	}	

	this->mPropertyGrid.AddProperty( pGroup );

	CString tTitle;
	tTitle.Format( "%s 设置", pDataHandler->Name().c_str() );

	CUA306SettingDoc *pDoc = dynamic_cast< CUA306SettingDoc * >( this->GetDocument() );
	pDoc->SetCardIdentifier( mCardIdentifier, tTitle );	
}


void CUA306SettingView::OnClickedBtnCancel()
{
	// TODO: Add your control notification handler code here

	this->GetDocument()->OnCloseDocument();
}


void CUA306SettingView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class

	this->mAutoAlignManager.AddControl( IDC_BTN_OK, CRect( -1, -1, 100, 15 ));
	this->mAutoAlignManager.AddControl( IDC_BTN_CANCEL, CRect( -1, -1, 10, 15 ));
	this->mAutoAlignManager.AddControl( IDC_MFCPROPERTYGRID_PROPERTY, CRect( 0, 0, 0, 100 ) );
}


void CUA306SettingView::OnLButtonUp(UINT nFlags, CPoint point)
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


void CUA306SettingView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	this->mAutoAlignManager.Resize();
}

LRESULT CUA306SettingView::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{	
	CMFCPropertyGridProperty* pProp = ( CMFCPropertyGridProperty * )lparam;
	HSInt tVal = pProp->GetData();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	

	HSBool tRateSetEnabled = pMainFrame->IsSettingArgEnabled();	

	switch ( tVal )
	{
	case PROPERTY_RATE:
		if ( !tRateSetEnabled )
		{
			mIncorrect = "回放模式，参数设置失败!";			
			
			IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( this->mCardIdentifier );	
			CUA306Handler *pUA306Handler = dynamic_cast< CUA306Handler * >( pDataHandler );

			CString tStr;
			tStr.Format( "%dK", pUA306Handler->SampleRate( 0 ) / 1000 );

			pProp->SetValue( tStr );
			pProp->Redraw();

			this->PostMessageA( WM_VALUE_INCORRCET_MESSAGE, 0, 0 );
		}
		break;

	default:
		break;
	}


	return 1;
}

LRESULT CUA306SettingView::OnValueIncorrect( WPARAM wparam, LPARAM lparam )
{
	AfxMessageBox( mIncorrect );

	return 1;
}