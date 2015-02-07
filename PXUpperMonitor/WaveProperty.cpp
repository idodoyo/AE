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

int CWaveProperty::mVoltages[ 11 ] = { 20 * 1000, 10 * 1000, 5 * 1000, 2 * 1000, 1 * 1000, 500, 200, 100, 50, 20, 10 };
int CWaveProperty::mUSeconds[ 12 ] = { 1 * 1000 * 1000, 500 * 1000, 200 * 1000, 100 * 1000, 50 * 1000, 20 * 1000, 10 * 1000, 1 * 1000, 500, 200, 100, 10 };

CString CWaveProperty::mStrVoltages[ 11 ] = { "20v", "10v", "5v", "2v", "1v", "500mv", "200mv", "100mv", "50mv", "20mv", "10mv" };
CString CWaveProperty::mStrTimes[ 12 ] = { "1s", "500ms",  "200ms",  "100ms",  "50ms",  "20ms", "10ms", "1ms", "500us", "200us", "100us", "10us" };


CWaveProperty::CWaveProperty(CWnd* pParent /*=NULL*/)
	: CNormalProperty(pParent)
{
	mParent = NULL;
}

CWaveProperty::~CWaveProperty()
{
}

void CWaveProperty::SetParent( IGraphicProtocol *pGraphic )
{
	mParent = NULL;		
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CGraphicWave * >( pGraphic );		
	}	

	CNormalProperty::SetParent( pGraphic );
}

void CWaveProperty::RefreshDataSource()
{
	if ( mParent == NULL )
	{
		return;
	}

	RefreshDeviceChannel();

	CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( PROPERTY_DATA_SOURCE );
	SetDataSourceForItem( pProp );	

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

	pProp = mPropertyGrid.FindItemByData( PROPERTY_DISCARD_DATA );	
	if ( mParent != NULL )
	{		
		pProp->SetValue( mParent->OnlyHitWaves() ? _T( "是" ) : _T( "否" ) );
	}

	pProp->Redraw();

	pProp = mPropertyGrid.FindItemByData( PROPERTY_OFFSET_VOLTAGE );	
	if ( mParent != NULL )
	{		
		pProp->SetValue( _variant_t( mParent->VoltageOffset() ) );
	}

	pProp->Redraw();
}

HSBool CWaveProperty::IsDataHandlerMachedWith( IDataHandler *pDataHandler, HSUInt tChannel )
{
	return ( pDataHandler == mParent->DataHandler() && mParent->DataIdentifier().ChannelIndex() == tChannel );
}

HSVoid CWaveProperty::InitDialog()
{
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

	pProp = new CMFCPropertyGridProperty(_T("仅撞击点波形"), _T( "否" ), _T( "只显示撞击时刻的波形!" ), PROPERTY_DISCARD_DATA );		
	pProp->AddOption( _T( "是" ) );
	pProp->AddOption( _T( "否" ) );
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("电压偏移"), _variant_t( 0.0 ), _T( "设备电压偏移值" ), PROPERTY_OFFSET_VOLTAGE );	
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );
	
	pProp = new CMFCPropertyGridProperty(_T("数据源"), _T( "" ), _T( "选择数据源" ), PROPERTY_DATA_SOURCE );		
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );

	mPropertyGrid.AddProperty( pGroup );
}

HSVoid CWaveProperty::PropertyChanged( CMFCPropertyGridProperty * pProperty )
{
	if ( pProperty->GetData() == PROPERTY_OFFSET_VOLTAGE )
	{		
		CString tValue = pProperty->GetValue();
		mParent->SetVoltageOffset( atof( tValue ) );
		return;
	}

	HSInt tIndex = IndexWithString( pProperty->GetValue(), pProperty );	
	if ( tIndex < 0 )
	{
		return;
	}
	
	if ( pProperty->GetData() == PROPERTY_VOLTAGE )
	{		
		mParent->SetVoltage( mVoltages[ tIndex ] / 1000.0 );
	}
	else if ( pProperty->GetData() == PROPERTY_TIME )
	{		
		mParent->SetTime( mUSeconds[ tIndex ] / 1000.0 / 1000.0 );
	}
	else if ( pProperty->GetData() == PROPERTY_DISCARD_DATA )
	{		
		mParent->SetOnlyHitWaves( tIndex == 0 ? HSTrue : HSFalse );
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
}