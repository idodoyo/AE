// PlanePositionProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "PlanePositionProperty.h"
#include "afxdialogex.h"
#include "PlanePosition.h"
#include "HSLogProtocol.h"
#include "MainFrm.h"


// CPlanePositionProperty dialog


CPlanePositionProperty::CPlanePositionProperty(CWnd* pParent /*=NULL*/)
	: CNormalProperty(pParent)
{
	mParent = NULL;
	mForPosition = HSTrue;
}

CPlanePositionProperty::~CPlanePositionProperty()
{
}


void CPlanePositionProperty::SetParent( IGraphicProtocol *pGraphic )
{
	mParent = NULL;		
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CPlanePosition * >( pGraphic );		
	}	

	CNormalProperty::SetParent( pGraphic );
}

void CPlanePositionProperty::RefreshDataSource()
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
		CPlanePosition::PositionSensor *pSensorInfo = mParent->GetPositionSensor( tSensorIndex );
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
			pProp->SetValue( ( _variant_t )( pSensorInfo->XPos / 1000.0 ) );
			pProp->Redraw();
		}

		DWORD tPropPosYID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_Y;
		pProp = mPropertyGrid.FindItemByData( tPropPosYID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetValue( ( _variant_t )( pSensorInfo->YPos / 1000.0 ) );
			pProp->Redraw();
		}

		DWORD tPropForbidID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_FORBID;
		pProp = mPropertyGrid.FindItemByData( tPropForbidID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetValue( pSensorInfo->Forbid ? _T( "��" ) : _T( "��" ) );
			pProp->Redraw();
		}

		CMFCPropertyGridProperty *pProperty = pGroup->GetSubItem( tSensorIndex + 6 );
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
		pProp->SetValue( ( _variant_t )( ( HSInt )( mParent->GetCheckRadius() ) ) );	

		COleVariant vtVar( ( long )mParent->GetCheckRadius() );
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
}

HSBool CPlanePositionProperty::IsDataHandlerMachedWith( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSVoid *pSensorInfo )
{
	CPlanePosition::PositionSensor *pSensor = ( CPlanePosition::PositionSensor * )pSensorInfo;
	return ( pSensorInfo != NULL && pSensor->DataIdentifier == tChannelIdentifier );
}

HSVoid CPlanePositionProperty::InitDialog()
{
	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("ƽ�涨λ����"));	
	
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("����������"), ( _variant_t )3, _T( "���ô���������!" ), PROPERTY_SENSOR_COUNT );
	pProp->EnableSpinControl( TRUE, 3, 100 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

	CMFCPropertyGridProperty *pCoord = new CMFCPropertyGridProperty( "����" );

	pProp = new CMFCPropertyGridProperty(_T("X����"), ( _variant_t )1, _T( "X����(��)!" ), PROPERTY_X_AXIS_LENGTH );	
	pProp->EnableSpinControl( TRUE, 1, 10000 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("Y����"), ( _variant_t )1, _T( "Y����(��)!" ), PROPERTY_Y_AXIS_LENGTH );
	pProp->EnableSpinControl( TRUE, 1, 10000 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pCoord );


	CMFCPropertyGridProperty *pMaterialCoord = new CMFCPropertyGridProperty( "���ϳߴ�" );

	pProp = new CMFCPropertyGridProperty(_T("��"), ( _variant_t )8.0, _T( "���ò��ϳߴ�!" ), PROPERTY_HEIGHT );	
	pMaterialCoord->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("��"), ( _variant_t )8.0, _T( "���ò��ϳߴ�!" ), PROPERTY_WIDTH );		
	pMaterialCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pMaterialCoord );

	pProp = new CMFCPropertyGridProperty(_T("ײ���뾶"), ( _variant_t )3, _T( "����ײ�����С!" ), PROPERTY_HIT_RADUS );
	pProp->EnableSpinControl( TRUE, 1, 3 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );	

	CMFCPropertyGridProperty *pCheckCircle = new CMFCPropertyGridProperty( "��ȼ���" );
	pProp = new CMFCPropertyGridProperty(_T("�뾶(����)"), ( _variant_t )20, _T( "��ȼ�����С!" ), PROPERTY_CHECK_CIRCLE_RADIUS );
	pProp->EnableSpinControl( TRUE, 5, 500 );	
	pProp->AllowEdit( FALSE );
	pCheckCircle->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("X����"), ( _variant_t )0.5, _T( "��ȼ����X����(��)!" ), PROPERTY_CHECK_CIRCLE_X );
	pCheckCircle->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("Y����"), ( _variant_t )0.5, _T( "��ȼ����Y����(��)!" ), PROPERTY_CHECK_CIRCLE_Y );
	pCheckCircle->AddSubItem( pProp );	

	pGroup->AddSubItem( pCheckCircle );


	CMFCPropertyGridProperty *pHitColor = new CMFCPropertyGridProperty( "ײ���ȼ�" );
	vector< CGraphicManager::HIT_LEVEL_INFO > *pHitLevels = CGraphicManager::SharedInstance()->HitLevels();
	for ( HSUInt i = 0; i < pHitLevels->size(); i++ )
	{
		CMFCPropertyGridColorProperty *pColorProperty = new CMFCPropertyGridColorProperty( ( *pHitLevels )[ i ].Desc.c_str(), ( *pHitLevels )[ i ].Color );
		pColorProperty->AllowEdit( FALSE );
		pColorProperty->Enable( FALSE );
		pHitColor->AddSubItem( pColorProperty );
	}

	pGroup->AddSubItem( pHitColor );
	
	for ( HSInt i = 0; i < POSITION_SENSOR_MAX_NUM; i++ )
	{
		CString tStrSensorName;
		tStrSensorName.Format( "������%d", i + 1 );
		CMFCPropertyGridProperty *pSensor = new CMFCPropertyGridProperty( tStrSensorName );

		pProp = new CMFCPropertyGridProperty(_T("����Դ"), _T( "" ), _T( "ѡ������Դ" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_DATA_SOURCE );		
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("X����"), ( _variant_t )0.5, _T( "������X����(��)!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_X );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("Y����"), ( _variant_t )0.5, _T( "������Y����(��)!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_Y );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("����"), _T( "��" ), _T( "���ô�����!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_FORBID );
		pProp->AddOption( _T( "��" ) );
		pProp->AddOption( _T( "��" ) );
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pSensor->Show( i < 3 );				
		pGroup->AddSubItem( pSensor );
	}	

	mPropertyGrid.AddProperty( pGroup );	
}

HSVoid CPlanePositionProperty::PropertyChanged( CMFCPropertyGridProperty * pProperty )
{
	CString tPropValue = pProperty->GetValue();
	
	CMFCPropertyGridProperty *pGroup = mPropertyGrid.GetProperty( 0 );
	if ( pProperty->GetData() == PROPERTY_SENSOR_COUNT )
	{					
		HSInt tShowCount = atoi( ( LPCSTR )tPropValue );		
		for ( HSInt i = 0; i < POSITION_SENSOR_MAX_NUM; i++ )
		{
			CMFCPropertyGridProperty *pSensorProperty = pGroup->GetSubItem( i + 6 );
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
			HSDouble tPosX = atof( ( LPCSTR )tValue ) * 1000.0;

			pItemProperty = pSensorProperty->GetSubItem( 2 );
			tValue = pItemProperty->GetValue();
			HSDouble tPosY = atof( ( LPCSTR )tValue ) * 1000.0;

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
	else if ( pProperty->GetData() == PROPERTY_HIT_RADUS )
	{		
		mParent->SetHitRadius( atoi( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() == PROPERTY_CHECK_CIRCLE_RADIUS )
	{		
		mParent->SetCheckRadius( atoi( ( LPCSTR )tPropValue ) );
	}	
	else if ( pProperty->GetData() == PROPERTY_CHECK_CIRCLE_X )
	{		
		mParent->SetCheckPosX( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
	}	
	else if ( pProperty->GetData() == PROPERTY_CHECK_CIRCLE_Y )
	{		
		mParent->SetCheckPosY( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
	}	
	else if ( pProperty->GetData() > 1000 )
	{
		HSInt tSensorIndex = pProperty->GetData() / 1000;
		CMFCPropertyGridProperty *pSensorProperty = pGroup->GetSubItem( tSensorIndex - 1 + 6 );

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
		HSDouble tPosX = atof( ( LPCSTR )tValue ) * 1000.0;

		pItemProperty = pSensorProperty->GetSubItem( 2 );
		tValue = pItemProperty->GetValue();
		HSDouble tPosY = atof( ( LPCSTR )tValue ) * 1000.0;

		pItemProperty = pSensorProperty->GetSubItem( 3 );
		tValue = pItemProperty->GetValue();
		tIndex = IndexWithString( tValue, pItemProperty );
		HSBool tForbid = ( tIndex == 0 );

		mParent->UpdatePositionSensor( tSensorIndex - 1, tPosX, tPosY, tForbid, HSTrue, tChannelIdentifier );
		mParent->RefreshSensors();
	}
}

HSVoid CPlanePositionProperty::SensorClicked( HSInt tIndex )
{
	mParent->SetFocusSensor( tIndex );
}