// WaveProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "WaveProperty.h"
#include "afxdialogex.h"
#include "GraphicWave.h"
#include "HSLogProtocol.h"
#include "MainFrm.h"

// CWaveProperty dialog

int CWaveProperty::mVoltages[ 10 ] = { 20 * 1000, 10 * 1000, 5 * 1000, 2 * 1000, 1 * 1000, 500, 250, 100, 50, 10 };
int CWaveProperty::mUSeconds[ 6 ] = { 1 * 1000 * 1000, 100 * 1000, 10 * 1000, 1 * 1000, 100, 10 };

CString CWaveProperty::mStrVoltages[ 10 ] = { "20v", "10v", "5v", "2v", "1v", "500mv", "250mv", "100mv", "50mv", "10mv" };
CString CWaveProperty::mStrTimes[ 6 ] = { "1s", "100ms", "10ms", "1ms", "100us", "10us" };

IMPLEMENT_DYNAMIC(CWaveProperty, CDialogEx)

CWaveProperty::CWaveProperty(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWaveProperty::IDD, pParent)
{
	mIsInit = false;
	mParent = NULL;
}

CWaveProperty::~CWaveProperty()
{
}

void CWaveProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID_PROPERTY, mPropertyGrid);
}


BEGIN_MESSAGE_MAP(CWaveProperty, CDialogEx)
	ON_WM_SIZE()		
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
END_MESSAGE_MAP()


// CWaveProperty message handlers

void CWaveProperty::SetParent( IGraphicProtocol *pGraphic )
{
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CGraphicWave * >( pGraphic );
		mPropertyGrid.EnableWindow( true );
	}
	else
	{
		mParent = NULL;
		mPropertyGrid.EnableWindow( false );
	}	
}


void CWaveProperty::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if ( !mIsInit )
	{
		return;
	}

	CRect tRect;
	this->GetClientRect( &tRect );
	mPropertyGrid.MoveWindow( tRect );
}

LRESULT CWaveProperty::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{
	if ( mParent == NULL )
	{
		return 0;
	}

	CMFCPropertyGridProperty * pProperty = ( CMFCPropertyGridProperty * ) lparam;	
	HSInt tIndex = IndexWithString( pProperty->GetValue(), pProperty );	
	if ( tIndex < 0 )
	{
		return 0;
	}
	
	if ( pProperty->GetData() == PROPERTY_VOLTAGE )
	{		
		mParent->SetVoltage( mVoltages[ tIndex ] / 1000.0 );
	}
	else if ( pProperty->GetData() == PROPERTY_TIME )
	{		
		mParent->SetTime( mUSeconds[ tIndex ] / 1000.0 / 1000.0 );
	}
	else 
	{		
		DEVICE_CH_IDENTIFIER tChannelIdentifier = 0;
		DataHandlerWithListIndex( tIndex, tChannelIdentifier, mDeviceChannelInfo );
		mParent->SetDataIdentifier( tChannelIdentifier );		

		CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( PROPERTY_BYPASS_VOLTAGE );		
		if ( mParent != NULL && mParent->DataHandler() != NULL )
		{		
			pProp->SetValue( GetStrVoltage( mParent->DataHandler()->BypassVoltage( mParent->DataIdentifier().ChannelIndex() ) ) );		
		}
		else
		{
			pProp->SetValue( "0 v" );
		}

		pProp->Redraw();
	}

	return 1;
}

BOOL CWaveProperty::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	mPropertyGrid.EnableHeaderCtrl(FALSE);
	mPropertyGrid.EnableDescriptionArea();
	mPropertyGrid.SetVSDotNetLook();
	mPropertyGrid.MarkModifiedProperties();		

	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("波形图属性"));	
	
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("电压"), _T( "10v" ), _T( "选择总电压" ), PROPERTY_VOLTAGE );
	for ( int i = 0; i < sizeof( mStrVoltages ) / sizeof( CString ); i++ )
	{
		pProp->AddOption( mStrVoltages[ i ] );
	}   
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("门槛电压"), _T( "0 v" ), _T( "设备门槛电压" ), PROPERTY_BYPASS_VOLTAGE );	
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );
	
	pProp = new CMFCPropertyGridProperty(_T("时间"), _T( "1s" ), _T( "选择总时间" ), PROPERTY_TIME );
	for ( int i = 0; i < sizeof( mStrTimes ) / sizeof( CString ); i++ )
	{
		pProp->AddOption( mStrTimes[ i ] );
	}   
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );	
	
	pProp = new CMFCPropertyGridProperty(_T("数据源"), _T( "" ), _T( "选择数据源" ), PROPERTY_DATA_SOURCE );		
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );

	mPropertyGrid.AddProperty( pGroup );	

	mIsInit = true;


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CWaveProperty::RefreshDataSource()
{
	if ( mParent == NULL )
	{
		return;
	}

	mDeviceChannelInfo.clear();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	HSUInt tIndex = 0;
	IDataHandler *pDataHandler = NULL;
	while ( ( pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIndex( tIndex++ ) ) )
	{
		DEVICE_CH_IDENTIFIER tType = pDataHandler->Identifier();
		for ( HSInt i = 0; i < pDataHandler->ChannelNum(); i++ )
		{
			mDeviceChannelInfo[ tType ].push_back( i );
		}		
	}

	CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( PROPERTY_DATA_SOURCE );
	pProp->RemoveAllOptions();

	bool tGotDataSource = false;
	map< DEVICE_CH_IDENTIFIER, vector< HSInt > >::iterator pIterator = mDeviceChannelInfo.begin();
	while ( pIterator != mDeviceChannelInfo.end() )
	{
		IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( pIterator->first );
		for ( HSUInt i = 0; i < pIterator->second.size(); i++ )
		{
			CString tStr;
			tStr.Format( "%s - 通道%d", pDataHandler->Name().c_str(), pIterator->second[ i ] + 1 );
			pProp->AddOption( tStr );
			
			if ( mParent != NULL && pDataHandler == mParent->DataHandler() && mParent->DataIdentifier().ChannelIndex() == i )
			{
				pProp->SetValue( tStr );
				tGotDataSource = true;
			}			 
		}
		pIterator++;
	}		

	if ( !tGotDataSource )
	{
		pProp->SetValue( "" );
	}
	pProp->Redraw();

	pProp = mPropertyGrid.FindItemByData( PROPERTY_VOLTAGE );	
	if ( mParent != NULL )
	{
		int tVoltage = int( mParent->TotalVoltage() * 1000 );
		for ( int i = 0; i < sizeof( mVoltages ) / sizeof( int ); i++ )
		{
			if ( tVoltage == mVoltages[ i ] )
			{
				pProp->SetValue( mStrVoltages[ i ] );
				break;
			}
		}
	}

	pProp->Redraw();

	pProp = mPropertyGrid.FindItemByData( PROPERTY_TIME );	
	if ( mParent != NULL )
	{
		int tUSeconds = int( mParent->RefreshTime() * 1000 * 1000 );
		for ( int i = 0; i < sizeof( mUSeconds ) / sizeof( int ); i++ )
		{
			if ( tUSeconds == mUSeconds[ i ] )
			{
				pProp->SetValue( mStrTimes[ i ] );
				break;
			}
		}
	}

	pProp->Redraw();

	pProp = mPropertyGrid.FindItemByData( PROPERTY_BYPASS_VOLTAGE );	
	if ( mParent != NULL && mParent->DataHandler() != NULL )
	{		
		pProp->SetValue( GetStrVoltage( mParent->DataHandler()->BypassVoltage( mParent->DataIdentifier().ChannelIndex() ) ) );		
	}
	else
	{
		pProp->SetValue( "0 v" );
	}

	pProp->Redraw();
}

