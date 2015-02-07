// PlanePositionProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "BoilerPositionProperty.h"
#include "afxdialogex.h"
#include "BoilerPosition.h"
#include "HSLogProtocol.h"
#include "MainFrm.h"


// CBoilerPositionProperty dialog


CBoilerPositionProperty::CBoilerPositionProperty(CWnd* pParent /*=NULL*/)
	: CNormalProperty(pParent)
{
	mParent = NULL;
	mForPosition = HSTrue;
}

CBoilerPositionProperty::~CBoilerPositionProperty()
{
}

void CBoilerPositionProperty::SetParent( IGraphicProtocol *pGraphic )
{
	mParent = NULL;		
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CBoilerPosition * >( pGraphic );		
	}	

	CNormalProperty::SetParent( pGraphic );
}

void CBoilerPositionProperty::RefreshDataSource()
{
	if ( mParent == NULL )
	{
		return;
	}

	RefreshDeviceChannel();

	CMFCPropertyGridProperty *pGroup = mPropertyGrid.GetProperty( 0 );
	HSInt tSensorOnCount = 0;
	for ( HSInt tSensorIndex = 0; tSensorIndex < POSITION_SENSOR_MAX_NUM; tSensorIndex++ )
	{
		CBoilerPosition::PositionSensor *pSensorInfo = mParent->GetPositionSensor( tSensorIndex );
		if ( pSensorInfo != NULL )
		{			
			tSensorOnCount++;
		}

		DWORD tPropDataSourceID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_DATA_SOURCE;
		CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( tPropDataSourceID );
		SetDataSourceForItem( pProp, pSensorInfo );		
	}	
	
	
	CMFCPropertyGridProperty *pProp = mPropertyGrid.FindItemByData( PROPERTY_RADIUS );	
	pProp->SetValue( ( _variant_t )( mParent->MaterailRadius() * 2 ) );		
	pProp->Redraw();	

	pProp = mPropertyGrid.FindItemByData( PROPERTY_PILLAR1_LENGTH );
	pProp->SetValue( ( _variant_t )( mParent->PillarLength( 0 ) ) );		
	pProp->Redraw();	

	pProp = mPropertyGrid.FindItemByData( PROPERTY_PILLAR2_LENGTH );		
	pProp->SetValue( ( _variant_t )( mParent->PillarLength( 1 ) ) );		
	pProp->Redraw();	


	pProp = mPropertyGrid.FindItemByData( PROPERTY_PILLAR3_LENGTH );		
	pProp->SetValue( ( _variant_t )( mParent->PillarLength( 2 ) ) );		
	pProp->Redraw();	


	pProp = mPropertyGrid.FindItemByData( PROPERTY_PILLAR4_LENGTH );
	pProp->SetValue( ( _variant_t )( mParent->PillarLength( 3 ) ) );		
	pProp->Redraw();	
	

	HSDouble tIronCycle = 0;
	HSDouble tIronTime = 0;
	mParent->IronStoneInfo( tIronCycle, tIronTime );
	pProp = mPropertyGrid.FindItemByData( PROPERTY_IRONSTONE_CIRLE_COUNT );	
	pProp->SetValue( ( _variant_t )( tIronCycle ) );
	pProp->Redraw();

	pProp = mPropertyGrid.FindItemByData( PROPERTY_IRONSTONE_TIME );	
	pProp->SetValue( ( _variant_t )( tIronTime ) );
	pProp->Redraw();

	HSDouble tCokeCycle = 0;
	HSDouble tCokeTime = 0;
	mParent->CokeInfo( tCokeCycle, tCokeTime );

	pProp = mPropertyGrid.FindItemByData( PROPERTY_COKE_CIRLE_COUNT );	
	pProp->SetValue( ( _variant_t )( tCokeCycle ) );
	pProp->Redraw();

	pProp = mPropertyGrid.FindItemByData( PROPERTY_COKE_TIME );	
	pProp->SetValue( ( _variant_t )( tCokeTime ) );
	pProp->Redraw();

	pProp = mPropertyGrid.FindItemByData( PROPERTY_BOILER_NAME );	
	pProp->SetValue( mParent->BoilerName().c_str() );
	pProp->Redraw();
	

	/*
	pProp = mPropertyGrid.FindItemByData( PROPERTY_X_AXIS_LENGTH );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( mParent->XAxisLength() ) ) );		

		COleVariant vtVar( ( long )mParent->XAxisLength() );
		pProp->SetValue( vtVar );	

		pProp->Redraw();	
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_Y_AXIS_LENGTH );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( mParent->YAxisLength() ) ) );	

		COleVariant vtVar( ( long )mParent->YAxisLength() );
		pProp->SetValue( vtVar );	

		pProp->Redraw();	
	}	*/
}

HSBool CBoilerPositionProperty::IsDataHandlerMachedWith( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSVoid *pSensorInfo )
{
	CBoilerPosition::PositionSensor *pSensor = ( CBoilerPosition::PositionSensor * )pSensorInfo;
	return ( pSensorInfo != NULL && pSensor->DataIdentifier == tChannelIdentifier );
}

HSVoid CBoilerPositionProperty::InitDialog()
{
	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("锅炉定位属性"));
	
	/*
	CMFCPropertyGridProperty *pCoord = new CMFCPropertyGridProperty( "坐标" );

	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("X坐标"), ( _variant_t )1, _T( "X坐标(长)!" ), PROPERTY_X_AXIS_LENGTH );	
	pProp->EnableSpinControl( TRUE, 1, 100 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("Y坐标"), ( _variant_t )1, _T( "Y坐标(宽)!" ), PROPERTY_Y_AXIS_LENGTH );
	pProp->EnableSpinControl( TRUE, 1, 100 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pCoord );
	*/

	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("名称"), "锅炉1", _T( "设置锅炉名称!" ), PROPERTY_BOILER_NAME );	
	pGroup->AddSubItem( pProp );	

	CMFCPropertyGridProperty *pMaterialCoord = new CMFCPropertyGridProperty( "锅炉尺寸(米)" );

	pProp = new CMFCPropertyGridProperty(_T("直径"), ( _variant_t )8.0, _T( "锅炉直径!" ), PROPERTY_RADIUS );	
	pMaterialCoord->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("测温架1"), ( _variant_t )4.0, _T( "设置十字测温架长度!" ), PROPERTY_PILLAR1_LENGTH );		
	pMaterialCoord->AddSubItem( pProp );
	pProp = new CMFCPropertyGridProperty(_T("测温架2"), ( _variant_t )4.0, _T( "设置十字测温架长度!" ), PROPERTY_PILLAR2_LENGTH );		
	pMaterialCoord->AddSubItem( pProp );
	pProp = new CMFCPropertyGridProperty(_T("测温架3"), ( _variant_t )4.0, _T( "设置十字测温架长度!" ), PROPERTY_PILLAR3_LENGTH );		
	pMaterialCoord->AddSubItem( pProp );
	pProp = new CMFCPropertyGridProperty(_T("测温架4"), ( _variant_t )4.0, _T( "设置十字测温架长度!" ), PROPERTY_PILLAR4_LENGTH );		
	pMaterialCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pMaterialCoord );	

	CMFCPropertyGridProperty *pBoilerInfo = new CMFCPropertyGridProperty( "加料信息" );
	pProp = new CMFCPropertyGridProperty(_T("矿石加料圈数"), ( _variant_t )10.0, _T( "设置矿石加料圈数!" ), PROPERTY_IRONSTONE_CIRLE_COUNT );		
	pBoilerInfo->AddSubItem( pProp );
	pProp = new CMFCPropertyGridProperty(_T("矿石加料时间（秒）"), ( _variant_t )7.5, _T( "设置矿石加料一圈的时间!" ), PROPERTY_IRONSTONE_TIME );		
	pBoilerInfo->AddSubItem( pProp );
	pProp = new CMFCPropertyGridColorProperty( "矿石标注颜色", RGB( 255, 0, 0 ) );
	pProp->AllowEdit( FALSE );
	pProp->Enable( FALSE );
	pBoilerInfo->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("焦炭加料圈数"), ( _variant_t )11.5, _T( "设置焦炭加料圈数!" ), PROPERTY_COKE_CIRLE_COUNT );		
	pBoilerInfo->AddSubItem( pProp );
	pProp = new CMFCPropertyGridProperty(_T("焦炭加料时间（秒）"), ( _variant_t )7.5, _T( "设置焦炭加料一圈时间!" ), PROPERTY_COKE_TIME );		
	pBoilerInfo->AddSubItem( pProp );
	pProp = new CMFCPropertyGridColorProperty( "焦炭标注颜色", RGB( 0, 0, 255 ) );
	pProp->AllowEdit( FALSE );
	pProp->Enable( FALSE );
	pBoilerInfo->AddSubItem( pProp );

	pGroup->AddSubItem( pBoilerInfo );
	
	CMFCPropertyGridProperty *pSensor = new CMFCPropertyGridProperty( "数据源" );
	for ( HSInt i = 0; i < POSITION_SENSOR_MAX_NUM; i++ )
	{		
		CString tStrSensorName;
		tStrSensorName.Format( "传感器%d", i + 1 );
		pProp = new CMFCPropertyGridProperty( tStrSensorName, _T( "" ), _T( "选择数据源" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_DATA_SOURCE );
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );
	}	

	pGroup->AddSubItem( pSensor );

	mPropertyGrid.AddProperty( pGroup );	
}

HSVoid CBoilerPositionProperty::PropertyChanged( CMFCPropertyGridProperty * pProperty )
{
	CString tPropValue = pProperty->GetValue();
	
	CMFCPropertyGridProperty *pGroup = mPropertyGrid.GetProperty( 0 );
	if ( pProperty->GetData() == PROPERTY_RADIUS )
	{		
		mParent->SetMaterailRadius( atof( ( LPCSTR )tPropValue ) / 2 );	
	}	
	else if ( pProperty->GetData() == PROPERTY_PILLAR1_LENGTH )
	{		
		mParent->SetPillarLength( 0, atof( ( LPCSTR )tPropValue ) );	
	}		
	else if ( pProperty->GetData() == PROPERTY_PILLAR2_LENGTH )
	{		
		mParent->SetPillarLength( 1, atof( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() == PROPERTY_PILLAR3_LENGTH )
	{		
		mParent->SetPillarLength( 2, atof( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() == PROPERTY_PILLAR4_LENGTH )
	{		
		mParent->SetPillarLength( 3, atof( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() == PROPERTY_X_AXIS_LENGTH )
	{		
		mParent->SetXAxisLength( atof( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() == PROPERTY_Y_AXIS_LENGTH )
	{		
		mParent->SetYAxisLength( atof( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() == PROPERTY_BOILER_NAME )
	{		
		mParent->SetBoilerName( ( LPCSTR )tPropValue );	
	}	
	else if ( pProperty->GetData() == PROPERTY_IRONSTONE_CIRLE_COUNT )
	{		
		mParent->SetIronStoneInfo( atof( ( LPCSTR )tPropValue ), -1 );
	}	
	else if ( pProperty->GetData() == PROPERTY_IRONSTONE_TIME )
	{		
		mParent->SetIronStoneInfo( -1, atof( ( LPCSTR )tPropValue ) );
	}	
	else if ( pProperty->GetData() == PROPERTY_COKE_CIRLE_COUNT )
	{		
		mParent->SetCokeInfo( atof( ( LPCSTR )tPropValue ), -1 );	
	}	
	else if ( pProperty->GetData() == PROPERTY_COKE_TIME )
	{		
		mParent->SetCokeInfo( -1, atof( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() > 1000 )
	{
		HSInt tSensorIndex = pProperty->GetData() / 1000;				
		
		HSInt tIndex = IndexWithString( tPropValue, pProperty );
		DEVICE_CH_IDENTIFIER tChannelIdentifier = 0;
		if ( tIndex >= 0 )
		{
			DataHandlerWithListIndex( tIndex, tChannelIdentifier, mDeviceChannelInfo );			
		}		

		mParent->UpdatePositionSensor( tSensorIndex - 1, tChannelIdentifier );		
	}
}

HSVoid CBoilerPositionProperty::SensorClicked( HSInt tIndex )
{
	mParent->SetFocusSensor( tIndex );
}