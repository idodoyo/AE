// CrackPositionProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "CrackPositionProperty.h"
#include "afxdialogex.h"
#include "CrackPosition.h"
#include "HSLogProtocol.h"
#include "MainFrm.h"


// CCrackPositionProperty dialog

CCrackPositionProperty::CCrackPositionProperty(CWnd* pParent /*=NULL*/)
	: CNormalProperty(pParent)
{
	mParent = NULL;
	mForPosition = HSTrue;
}

CCrackPositionProperty::~CCrackPositionProperty()
{
}

void CCrackPositionProperty::SetParent( IGraphicProtocol *pGraphic )
{
	mParent = NULL;		
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CCrackPosition * >( pGraphic );
	}	

	CNormalProperty::SetParent( pGraphic );
}

void CCrackPositionProperty::RefreshDataSource()
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
		CCrackPosition::PositionSensor *pSensorInfo = mParent->GetPositionSensor( tSensorIndex );
		if ( pSensorInfo != NULL && pSensorInfo->IsOn )
		{			
			tSensorOnCount++;
		}

		DWORD tPropDataSourceID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_DATA_SOURCE;
		CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( tPropDataSourceID );
		SetDataSourceForItem( pProp, pSensorInfo );

		DWORD tPropPosXID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_X;
		pProp = mPropertyGrid.FindItemByData( tPropPosXID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetValue( ( _variant_t )pSensorInfo->XPos );
			pProp->Redraw();
		}

		DWORD tPropPosYID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_Y;
		pProp = mPropertyGrid.FindItemByData( tPropPosYID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetValue( ( _variant_t )pSensorInfo->YPos );
			pProp->Redraw();
		}

		DWORD tPropForbidID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_FORBID;
		pProp = mPropertyGrid.FindItemByData( tPropForbidID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetValue( pSensorInfo->Forbid ? _T( "是" ) : _T( "否" ) );
			pProp->Redraw();
		}

		CMFCPropertyGridProperty *pProperty = pGroup->GetSubItem( tSensorIndex + 8 );
		pProperty->Show( pSensorInfo != NULL && pSensorInfo->IsOn );
	}
	
	CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( PROPERTY_SENSOR_COUNT );	
	if ( mParent != NULL )
	{		
		pProp->SetValue( ( _variant_t )tSensorOnCount );		

		COleVariant vtVar( ( long )tSensorOnCount );
		pProp->SetValue( vtVar );

		pProp->Redraw();
	}	
	
	pProp = mPropertyGrid.FindItemByData( PROPERTY_WIDTH );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->MaterialWidth() / 1000.0 ) );
		pProp->Redraw();	
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_HEIGHT );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->MaterialLength() / 1000.0 ) );
		pProp->Redraw();
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_CENTER_X );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->CenterXCoord() ) );		
		pProp->Redraw();	
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_CENTER_Y );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->CenterYCoord() ) );
		pProp->Redraw();	
	}

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
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_HIT_RADUS );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( mParent->HitRadius() ) ) );		

		COleVariant vtVar( ( long )mParent->HitRadius() );
		pProp->SetValue( vtVar );	

		pProp->Redraw();	
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_CHECK_CIRCLE_RADIUS );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( mParent->GetCheckRadius() / 1000.0 ) ) );

		COleVariant vtVar( ( long )( mParent->GetCheckRadius() / 1000.0 ) );
		pProp->SetValue( vtVar );

		pProp->Redraw();	
	}

	HSDouble tCheckPosX = 0;
	HSDouble tCheckPosY = 0;
	pProp = mPropertyGrid.FindItemByData( PROPERTY_CHECK_CIRCLE_X );
	if ( mParent != NULL )
	{
		mParent->GetCheckPos( tCheckPosX, tCheckPosY );
		pProp->SetValue( ( _variant_t )( tCheckPosX / 1000.0 ) );
		pProp->Redraw();
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_CHECK_CIRCLE_Y );
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( tCheckPosY / 1000.0 ) );		
		pProp->Redraw();	
	}

	HSDouble tFirstCircleRadius = 0;
	HSDouble tSecondCircleRadius = 0;
	HSDouble tStartAngle = 0;
	pProp = mPropertyGrid.FindItemByData( PROPERTY_SENSOR_CIRCLE_ONE );	
	if ( mParent != NULL )
	{
		mParent->GetSensorCircleInfo( tFirstCircleRadius, tSecondCircleRadius, tStartAngle );
		pProp->SetValue( ( _variant_t )tFirstCircleRadius );
		pProp->Redraw();
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_SENSOR_CIRCLE_TUE );	
	if ( mParent != NULL )
	{		
		pProp->SetValue( ( _variant_t )tSecondCircleRadius );		
		pProp->Redraw();	
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_SENSOR_ONE_ANGLE );	
	if ( mParent != NULL )
	{		
		pProp->SetValue( ( _variant_t )tStartAngle );		
		pProp->Redraw();	
	}
}


HSBool CCrackPositionProperty::IsDataHandlerMachedWith( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSVoid *pSensorInfo )
{
	CCrackPosition::PositionSensor *pSensor = ( CCrackPosition::PositionSensor * )pSensorInfo;
	return ( pSensorInfo != NULL && pSensor->DataIdentifier == tChannelIdentifier );
}

HSVoid CCrackPositionProperty::InitDialog()
{
	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("裂缝定位属性"));	
	
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("传感器数量"), ( _variant_t )3, _T( "设置传感器数量!" ), PROPERTY_SENSOR_COUNT );
	pProp->EnableSpinControl( TRUE, 3, 8 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

	CMFCPropertyGridProperty *pCoord = new CMFCPropertyGridProperty( "坐标" );

	pProp = new CMFCPropertyGridProperty(_T("X坐标"), ( _variant_t )10, _T( "X坐标(长)!" ), PROPERTY_X_AXIS_LENGTH );	
	pProp->EnableSpinControl( TRUE, 1, 10000 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("Y坐标"), ( _variant_t )10, _T( "Y坐标(宽)!" ), PROPERTY_Y_AXIS_LENGTH );
	pProp->EnableSpinControl( TRUE, 1, 10000 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pCoord );


	CMFCPropertyGridProperty *pMaterialCoord = new CMFCPropertyGridProperty( "材料尺寸" );

	pProp = new CMFCPropertyGridProperty(_T("长"), ( _variant_t )8.0, _T( "设置材料尺寸!" ), PROPERTY_HEIGHT );
	//pProp->EnableSpinControl( TRUE, 1, 1000 );	
	//pProp->AllowEdit( FALSE );
	pMaterialCoord->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("宽"), ( _variant_t )8.0, _T( "设置材料尺寸!" ), PROPERTY_WIDTH );	
	//pProp->EnableSpinControl( TRUE, 1, 1000 );	
	//pProp->AllowEdit( FALSE );
	pMaterialCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pMaterialCoord );

	CMFCPropertyGridProperty *pCenterCoord = new CMFCPropertyGridProperty( "油井坐标" );
	pProp = new CMFCPropertyGridProperty(_T("经度"), ( _variant_t )16.0, _T( "油井所处经度!" ), PROPERTY_CENTER_X );
	pCenterCoord->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("纬度"), ( _variant_t )19.0, _T( "油井所处纬度!" ), PROPERTY_CENTER_Y );
	pCenterCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pCenterCoord );
	

	pProp = new CMFCPropertyGridProperty(_T("撞击半径"), ( _variant_t )3, _T( "设置撞击点大小!" ), PROPERTY_HIT_RADUS );
	pProp->EnableSpinControl( TRUE, 1, 3 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );	

	CMFCPropertyGridProperty *pCheckCircle = new CMFCPropertyGridProperty( "活度计算" );
	pProp = new CMFCPropertyGridProperty(_T("半径(米)"), ( _variant_t )20, _T( "活度计算点大小!" ), PROPERTY_CHECK_CIRCLE_RADIUS );
	pProp->EnableSpinControl( TRUE, 1, 50000 );
	pProp->AllowEdit( FALSE );
	pCheckCircle->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("X坐标"), ( _variant_t )0.5, _T( "活度计算点X坐标(米)!" ), PROPERTY_CHECK_CIRCLE_X );
	pCheckCircle->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("Y坐标"), ( _variant_t )0.5, _T( "活度计算点Y坐标(米)!" ), PROPERTY_CHECK_CIRCLE_Y );
	pCheckCircle->AddSubItem( pProp );

	pGroup->AddSubItem( pCheckCircle );


	CMFCPropertyGridProperty *pHitColor = new CMFCPropertyGridProperty( "撞击等级" );
	vector< CGraphicManager::HIT_LEVEL_INFO > *pHitLevels = CGraphicManager::SharedInstance()->HitLevels();
	for ( HSUInt i = 0; i < pHitLevels->size(); i++ )
	{
		CMFCPropertyGridColorProperty *pColorProperty = new CMFCPropertyGridColorProperty( ( *pHitLevels )[ i ].Desc.c_str(), ( *pHitLevels )[ i ].Color );
		pColorProperty->AllowEdit( FALSE );
		pColorProperty->Enable( FALSE );
		pHitColor->AddSubItem( pColorProperty );
	}

	pGroup->AddSubItem( pHitColor );

	CMFCPropertyGridProperty *pSensorCircle = new CMFCPropertyGridProperty( "传感器布局" );
	pProp = new CMFCPropertyGridProperty(_T("内圈半径(米)"), ( _variant_t )20.0, _T( "第一圈半径!" ), PROPERTY_SENSOR_CIRCLE_ONE );	
	pSensorCircle->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("外圈半径(米)"), ( _variant_t )60.0, _T( "第二圈半径!" ), PROPERTY_SENSOR_CIRCLE_TUE );
	pSensorCircle->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("起始角度"), ( _variant_t )45.0, _T( "第一圈中第一个传感器的角度( 0 - 360 )!" ), PROPERTY_SENSOR_ONE_ANGLE );
	pSensorCircle->AddSubItem( pProp );	

	pGroup->AddSubItem( pSensorCircle );
	
	for ( HSInt i = 0; i < POSITION_SENSOR_MAX_NUM; i++ )
	{
		CString tStrSensorName;
		tStrSensorName.Format( "传感器%d", i + 1 );
		CMFCPropertyGridProperty *pSensor = new CMFCPropertyGridProperty( tStrSensorName );

		pProp = new CMFCPropertyGridProperty(_T("数据源"), _T( "" ), _T( "选择数据源" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_DATA_SOURCE );
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("经度"), ( _variant_t )0.5, _T( "传感器所处经度!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_X );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("纬度"), ( _variant_t )0.5, _T( "传感器所处纬度!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_Y );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("禁用"), _T( "否" ), _T( "禁用传感器!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_FORBID );
		pProp->AddOption( _T( "是" ) );
		pProp->AddOption( _T( "否" ) );
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pSensor->Show( i < 3 );				
		pGroup->AddSubItem( pSensor );
	}

	mPropertyGrid.AddProperty( pGroup );	
}

HSVoid CCrackPositionProperty::PropertyChanged( CMFCPropertyGridProperty * pProperty )
{
	CString tPropValue = pProperty->GetValue();
	
	CMFCPropertyGridProperty *pGroup = mPropertyGrid.GetProperty( 0 );
	if ( pProperty->GetData() == PROPERTY_SENSOR_COUNT )
	{					
		HSInt tShowCount = atoi( ( LPCSTR )tPropValue );		
		for ( HSInt i = 0; i < POSITION_SENSOR_MAX_NUM; i++ )
		{
			CMFCPropertyGridProperty *pSensorProperty = pGroup->GetSubItem( i + 8 );
			pSensorProperty->Show( i < tShowCount );

			CMFCPropertyGridProperty *pItemProperty = pSensorProperty->GetSubItem( 0 );
			CString tValue = pItemProperty->GetValue();
			HSInt tIndex = IndexWithString( tValue, pItemProperty );
			DEVICE_CH_IDENTIFIER tChannelIdentifier = 0;
			if ( tIndex >= 0 )
			{
				DataHandlerWithListIndex( tIndex, tChannelIdentifier, mDeviceChannelInfo );
			}

			pItemProperty = pSensorProperty->GetSubItem( 1 );
			tValue = pItemProperty->GetValue();
			HSDouble tPosX = atof( ( LPCSTR )tValue );

			pItemProperty = pSensorProperty->GetSubItem( 2 );
			tValue = pItemProperty->GetValue();
			HSDouble tPosY = atof( ( LPCSTR )tValue );

			pItemProperty = pSensorProperty->GetSubItem( 3 );
			tValue = pItemProperty->GetValue();
			tIndex = IndexWithString( tValue, pItemProperty );
			HSBool tForbid = ( tIndex == 0 );

			mParent->UpdatePositionSensor( i, tPosX, tPosY, tForbid, i < tShowCount, tChannelIdentifier );
			mParent->RefreshSensors();
		}
	}
	else if ( pProperty->GetData() == PROPERTY_WIDTH )
	{		
		mParent->SetMaterialWidth( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
	}	
	else if ( pProperty->GetData() == PROPERTY_HEIGHT )
	{		
		mParent->SetMaterialLength( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
	}	
	else if ( pProperty->GetData() == PROPERTY_X_AXIS_LENGTH )
	{		
		mParent->SetXAxisLength( atof( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() == PROPERTY_Y_AXIS_LENGTH )
	{		
		mParent->SetYAxisLength( atof( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() == PROPERTY_CENTER_X )
	{		
		mParent->SetCenterXCoord( atof( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() == PROPERTY_CENTER_Y )
	{		
		mParent->SetCenterYCoord( atof( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() == PROPERTY_HIT_RADUS )
	{		
		mParent->SetHitRadius( atoi( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() == PROPERTY_CHECK_CIRCLE_RADIUS )
	{		
		mParent->SetCheckRadius( atof( ( LPCSTR )tPropValue ) * 1000.0 );
	}
	else if ( pProperty->GetData() == PROPERTY_CHECK_CIRCLE_X )
	{		
		mParent->SetCheckPosX( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
	}	
	else if ( pProperty->GetData() == PROPERTY_CHECK_CIRCLE_Y )
	{		
		mParent->SetCheckPosY( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
	}	
	else if ( pProperty->GetData() == PROPERTY_SENSOR_CIRCLE_ONE )
	{		
		mParent->SetSensorCircleInfo( atof( ( LPCSTR )tPropValue ), -1, -1 );	
	}	
	else if ( pProperty->GetData() == PROPERTY_SENSOR_CIRCLE_TUE )
	{		
		mParent->SetSensorCircleInfo( -1, atof( ( LPCSTR )tPropValue ), -1 );	
	}	
	else if ( pProperty->GetData() == PROPERTY_SENSOR_ONE_ANGLE )
	{		
		mParent->SetSensorCircleInfo( -1, -1, atof( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() > 1000 )
	{
		HSInt tSensorIndex = pProperty->GetData() / 1000;
		CMFCPropertyGridProperty *pSensorProperty = pGroup->GetSubItem( tSensorIndex - 1 + 8 );

		CMFCPropertyGridProperty *pItemProperty = pSensorProperty->GetSubItem( 0 );
		CString tValue = pItemProperty->GetValue();
		HSInt tIndex = IndexWithString( tValue, pItemProperty );
		DEVICE_CH_IDENTIFIER tChannelIdentifier = 0;
		if ( tIndex >= 0 )
		{
			DataHandlerWithListIndex( tIndex, tChannelIdentifier, mDeviceChannelInfo );			
		}

		pItemProperty = pSensorProperty->GetSubItem( 1 );
		tValue = pItemProperty->GetValue();
		HSDouble tPosX = atof( ( LPCSTR )tValue );

		pItemProperty = pSensorProperty->GetSubItem( 2 );
		tValue = pItemProperty->GetValue();
		HSDouble tPosY = atof( ( LPCSTR )tValue );

		pItemProperty = pSensorProperty->GetSubItem( 3 );
		tValue = pItemProperty->GetValue();
		tIndex = IndexWithString( tValue, pItemProperty );
		HSBool tForbid = ( tIndex == 0 );

		mParent->UpdatePositionSensor( tSensorIndex - 1, tPosX, tPosY, tForbid, HSTrue, tChannelIdentifier );
		mParent->RefreshSensors();

		//mParent->SetLastGPSModifiedSensor( tSensorIndex - 1 );
	}
}

HSVoid CCrackPositionProperty::SensorClicked( HSInt tIndex )
{
	mParent->SetFocusSensor( tIndex );
}