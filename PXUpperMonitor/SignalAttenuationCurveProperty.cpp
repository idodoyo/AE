// SignalAttenuationCurveProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "SignalAttenuationCurveProperty.h"
#include "afxdialogex.h"
#include "SignalAttenuationCurve.h"
#include "HSLogProtocol.h"
#include "MainFrm.h"


// CSignalAttenuationCurveProperty dialog

CSignalAttenuationCurveProperty::CSignalAttenuationCurveProperty(CWnd* pParent /*=NULL*/)
	: CNormalProperty(pParent)
{	
	mParent = NULL;
	mForPosition = HSTrue;
}

CSignalAttenuationCurveProperty::~CSignalAttenuationCurveProperty()
{
}


void CSignalAttenuationCurveProperty::SetParent( IGraphicProtocol *pGraphic )
{
	mParent = NULL;		
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CSignalAttenuationCurve * >( pGraphic );		
	}	

	CNormalProperty::SetParent( pGraphic );
}

void CSignalAttenuationCurveProperty::RefreshDataSource()
{
	if ( mParent == NULL )
	{
		return;
	}	

	RefreshDeviceChannel();

	HSInt tValue = 0;
	CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( PROPERTY_DATA_SOURCE );
	SetDataSourceForItem( pProp, &tValue );

	tValue = 1;
	CMFCPropertyGridProperty* pPropOrig = mPropertyGrid.FindItemByData( PROPERTY_DATA_SOURCE_ORIGIN );
	SetDataSourceForItem( pPropOrig, &tValue );
}

HSBool CSignalAttenuationCurveProperty::IsDataHandlerMachedWith( IDataHandler *pDataHandler, HSUInt tChannel, HSVoid *pInfo )
{
	HSInt tValue = *( HSInt * )pInfo;
	if ( tValue == 0 )
	{
		return ( pDataHandler == mParent->DataHandler() && mParent->DataIdentifier().ChannelIndex() == tChannel );
	}
	else
	{
		return ( pDataHandler == mParent->OrigDataHandler() && mParent->OrigDataIdentifier().ChannelIndex() == tChannel );
	}	
}

HSVoid CSignalAttenuationCurveProperty::InitDialog()
{
	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("压电补偿曲线属性"));
	
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("原始信号数据源"), _T( "" ), _T( "选择数据源" ), PROPERTY_DATA_SOURCE_ORIGIN );		
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("传感器信号数据源"), _T( "" ), _T( "选择数据源" ), PROPERTY_DATA_SOURCE );		
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );
	
	mPropertyGrid.AddProperty( pGroup );	
}

HSVoid CSignalAttenuationCurveProperty::PropertyChanged( CMFCPropertyGridProperty * pProperty )
{
	if ( pProperty->GetData() == PROPERTY_DATA_SOURCE_ORIGIN )
	{		
		HSInt tIndex = IndexWithString( pProperty->GetValue(), pProperty );	
		if ( tIndex < 0 )
		{
			return;
		}

		DEVICE_CH_IDENTIFIER tChannelIdentifier = 0;
		DataHandlerWithListIndex( tIndex, tChannelIdentifier, mDeviceChannelInfo );
		mParent->SetOrigDataIdentifier( tChannelIdentifier );
	}
	else if ( pProperty->GetData() == PROPERTY_DATA_SOURCE )
	{			
		HSInt tIndex = IndexWithString( pProperty->GetValue(), pProperty );	
		if ( tIndex < 0 )
		{
			return;
		}

		DEVICE_CH_IDENTIFIER tChannelIdentifier = 0;
		DataHandlerWithListIndex( tIndex, tChannelIdentifier, mDeviceChannelInfo );
		mParent->SetDataIdentifier( tChannelIdentifier );
	}
	
}
