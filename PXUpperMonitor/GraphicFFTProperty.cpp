// GraphicFFTProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GraphicFFTProperty.h"
#include "afxdialogex.h"
#include "GraphicFFT.h"
#include "HSLogProtocol.h"
#include "MainFrm.h"

// CGraphicFFTProperty dialog


CGraphicFFTProperty::CGraphicFFTProperty(CWnd* pParent /*=NULL*/)
	: CNormalProperty(pParent)
{	
	mParent = NULL;
}

CGraphicFFTProperty::~CGraphicFFTProperty()
{
}

void CGraphicFFTProperty::SetParent( IGraphicProtocol *pGraphic )
{
	mParent = NULL;		
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CGraphicFFT * >( pGraphic );		
	}	

	CNormalProperty::SetParent( pGraphic );
}


void CGraphicFFTProperty::RefreshDataSource()
{
	if ( mParent == NULL )
	{
		return;
	}	

	RefreshDeviceChannel();

	CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( PROPERTY_DATA_SOURCE );
	SetDataSourceForItem( pProp );		

	/*
	pProp = mPropertyGrid.FindItemByData( PROPERTY_AMPLITUDE );	
	if ( mParent != NULL )
	{
		int tAmplitude = int( mParent->TotalAmplitude() / 10 );		

		pProp->SetValue( ( _variant_t )tAmplitude );

		COleVariant vtVar( ( long )tAmplitude );
		pProp->SetValue( vtVar );		
	}

	pProp->Redraw();

	pProp = mPropertyGrid.FindItemByData( PROPERTY_BEGIN_AMPLITUDE );	
	if ( mParent != NULL )
	{
		int tAmplitude = int( mParent->BeginAmplitude() / 10 );		
		pProp->SetValue( ( _variant_t )tAmplitude );	

		COleVariant vtVar( ( long )tAmplitude );
		pProp->SetValue( vtVar );
	}

	pProp->Redraw();
	*/
}


HSBool CGraphicFFTProperty::IsDataHandlerMachedWith( IDataHandler *pDataHandler, HSUInt tChannel )
{
	return ( pDataHandler == mParent->DataHandler() && mParent->DataIdentifier().ChannelIndex() == tChannel );
}

HSVoid CGraphicFFTProperty::InitDialog()
{
	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("FFT属性"));	
	/*
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("幅度(10dB)"), ( _variant_t )5, _T( "设置总幅度(垂直坐标)!" ), PROPERTY_AMPLITUDE );
	pProp->EnableSpinControl( TRUE, 1, 100 );	
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("开始幅度(10dB)"), ( _variant_t )( -10 ), _T( "设置开始幅度!" ), PROPERTY_BEGIN_AMPLITUDE );
	pProp->EnableSpinControl( TRUE, -100, 100 );	
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );
	*/
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("数据源"), _T( "" ), _T( "选择数据源" ), PROPERTY_DATA_SOURCE );		
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );
	
	mPropertyGrid.AddProperty( pGroup );	

}

HSVoid CGraphicFFTProperty::PropertyChanged( CMFCPropertyGridProperty * pProperty )
{
	if ( pProperty->GetData() == PROPERTY_AMPLITUDE )
	{		
		CString tValue = pProperty->GetValue();			
		mParent->SetTotalAmplitude( atof( ( LPCSTR )tValue ) * 10 );
	}
	else if ( pProperty->GetData() == PROPERTY_BEGIN_AMPLITUDE )
	{		
		CString tValue = pProperty->GetValue();			
		mParent->SetBeginAmplitude( atof( ( LPCSTR )tValue ) * 10 );
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
	}
}
