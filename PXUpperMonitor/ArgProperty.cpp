// ArgProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "ArgProperty.h"
#include "afxdialogex.h"
#include "ArgTable.h"
#include "MainFrm.h"
#include "HSLogProtocol.h"

// CArgProperty dialog

CArgProperty::CArgProperty(CWnd* pParent /*=NULL*/)
	: CNormalProperty(pParent)
{
	mParent = NULL;
	mTotalChannelEnabled = HSTrue;
}

CArgProperty::~CArgProperty()
{
}


void CArgProperty::SetParent( IGraphicProtocol *pGraphic )
{
	mParent = NULL;		
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CArgTable * >( pGraphic );		
	}	

	CNormalProperty::SetParent( pGraphic );
}

void CArgProperty::RefreshDataSource()
{
	if ( mParent == NULL )
	{
		return;
	}

	RefreshDeviceChannel();

	CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( PROPERTY_DATA_SOURCE );
	SetDataSourceForItem( pProp );		

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

HSBool CArgProperty::IsDataHandlerMachedWith( IDataHandler *pDataHandler, HSUInt tChannel )
{
	return ( pDataHandler == mParent->DataHandler() && ( mParent->DataIdentifier().ChannelIndex() == tChannel || mParent->DataIdentifier().NumOfChannels() == pDataHandler->ChannelNum() ) );
}

HSVoid CArgProperty::InitDialog()
{
	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("参数表属性"));
	
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("门槛电压"), _T( "0 v" ), _T( "设备门槛电压" ), PROPERTY_BYPASS_VOLTAGE );	
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("数据源"), _T( "" ), _T( "选择数据源" ), PROPERTY_DATA_SOURCE );		
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );

	mPropertyGrid.AddProperty( pGroup );
}

HSVoid CArgProperty::PropertyChanged( CMFCPropertyGridProperty * pProperty )
{
	if ( pProperty->GetData() == PROPERTY_BYPASS_VOLTAGE )
	{				
	}	
	else 
	{	
		HSInt tIndex = IndexWithString( pProperty->GetValue(), pProperty );	
		if ( tIndex < 0 )
		{
			return;
		}

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