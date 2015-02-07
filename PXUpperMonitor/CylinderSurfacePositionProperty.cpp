// CylinderSurfacePositionProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "CylinderSurfacePositionProperty.h"
#include "afxdialogex.h"
#include "CylinderSurfacePosition.h"
#include "HSLogProtocol.h"
#include "MainFrm.h"


// CCylinderSurfacePositionProperty dialog

CCylinderSurfacePositionProperty::CCylinderSurfacePositionProperty(CWnd* pParent /*=NULL*/)
	: CNormalProperty(pParent)
{
	mParent = NULL;
	mForPosition = HSTrue;
}

CCylinderSurfacePositionProperty::~CCylinderSurfacePositionProperty()
{
}


void CCylinderSurfacePositionProperty::SetParent( IGraphicProtocol *pGraphic )
{
	mParent = NULL;		
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CCylinderSurfacePosition * >( pGraphic );		
	}	

	CNormalProperty::SetParent( pGraphic );
}

void CCylinderSurfacePositionProperty::RefreshDataSource()
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
		CCylinderSurfacePosition::PositionSensor *pSensorInfo = mParent->GetPositionSensor( tSensorIndex );
		if ( pSensorInfo != NULL && pSensorInfo->IsOn )
		{			
			tSensorOnCount++;
		}

		DWORD tPropDataSourceID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_DATA_SOURCE;
		CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( tPropDataSourceID );
		SetDataSourceForItem( pProp, pSensorInfo );		

		DWORD tPropPosXID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_ANGLE;
		pProp = mPropertyGrid.FindItemByData( tPropPosXID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{						
			pProp->SetValue( ( _variant_t ) pSensorInfo->Angle );
			pProp->Redraw();
		}

		DWORD tPropPosYID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_HEIGHT;
		pProp = mPropertyGrid.FindItemByData( tPropPosYID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{						
			pProp->SetValue( ( _variant_t )( pSensorInfo->Height / 1000.0 ) );
			pProp->Redraw();
		}

		DWORD tPropForbidID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_FORBID;
		pProp = mPropertyGrid.FindItemByData( tPropForbidID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{				
			pProp->SetValue( pSensorInfo->Forbid ? _T( "��" ) : _T( "��" ) );
			pProp->Redraw();
		}

		CMFCPropertyGridProperty *pProperty = pGroup->GetSubItem( tSensorIndex + 5 );
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
	
	pProp = mPropertyGrid.FindItemByData( PROPERTY_Y );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( mParent->GetAxisLength() / 1000.0 ) ) );	

		COleVariant vtVar( ( long )( mParent->GetAxisLength() / 1000.0 ) );
		pProp->SetValue( vtVar );	

		pProp->Redraw();
	}			

	pProp = mPropertyGrid.FindItemByData( PROPERTY_MATERIAL_Y );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->MaterialYLength() / 1000.0 ) );		
		pProp->Redraw();
	}	

	pProp = mPropertyGrid.FindItemByData( PROPERTY_MATERIAL_RADIUS );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->MaterialRadius() / 1000.0 * 2.0 ) );		
		pProp->Redraw();	
	}	

	pProp = mPropertyGrid.FindItemByData( PROPERTY_HIT_RADUS );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( mParent->HitRadius() ) ) );	

		COleVariant vtVar( ( long )( mParent->HitRadius() ) );
		pProp->SetValue( vtVar );

		pProp->Redraw();	
	}			
}


HSBool CCylinderSurfacePositionProperty::IsDataHandlerMachedWith( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSVoid *pSensorInfo )
{
	CCylinderSurfacePosition::PositionSensor *pSensor = ( CCylinderSurfacePosition::PositionSensor * )pSensorInfo;
	return ( pSensorInfo != NULL && pSensor->DataIdentifier == tChannelIdentifier );
}

HSVoid CCylinderSurfacePositionProperty::InitDialog()
{
	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("���涨λ����"));
	
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("����������"), ( _variant_t )3, _T( "���ô���������!" ), PROPERTY_SENSOR_COUNT );
	pProp->EnableSpinControl( TRUE, 3, 100 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );
	
	CMFCPropertyGridProperty *pCoord = new CMFCPropertyGridProperty( "����" );	

	pProp = new CMFCPropertyGridProperty(_T("Y����"), ( _variant_t )10, _T( "Y����(��)!" ), PROPERTY_Y );
	pProp->EnableSpinControl( TRUE, 1, 1000 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );	

	pGroup->AddSubItem( pCoord );


	CMFCPropertyGridProperty *pMaterialCoord = new CMFCPropertyGridProperty( "���ϳߴ�" );	

	pProp = new CMFCPropertyGridProperty(_T("��(m)"), ( _variant_t )1.0, _T( "Y����!" ), PROPERTY_MATERIAL_Y );	
	pMaterialCoord->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("ֱ��(m)"), ( _variant_t )1.0, _T( "ֱ��!" ), PROPERTY_MATERIAL_RADIUS );	
	pMaterialCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pMaterialCoord );	

	pProp = new CMFCPropertyGridProperty(_T("ײ���뾶"), ( _variant_t )3, _T( "����ײ�����С!" ), PROPERTY_HIT_RADUS );
	pProp->EnableSpinControl( TRUE, 1, 3 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

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

		pProp = new CMFCPropertyGridProperty(_T("�Ƕ�"), ( _variant_t )16.0, _T( "" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_ANGLE );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("�߶�"), ( _variant_t )0.5, _T( "" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_HEIGHT );
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

HSVoid CCylinderSurfacePositionProperty::PropertyChanged( CMFCPropertyGridProperty * pProperty )
{
	CString tPropValue = pProperty->GetValue();
	
	CMFCPropertyGridProperty *pGroup = mPropertyGrid.GetProperty( 0 );
	if ( pProperty->GetData() == PROPERTY_SENSOR_COUNT )
	{					
		HSInt tShowCount = atoi( ( LPCSTR )tPropValue );		
		for ( HSInt i = 0; i < POSITION_SENSOR_MAX_NUM; i++ )
		{
			CMFCPropertyGridProperty *pSensorProperty = pGroup->GetSubItem( i + 5 );
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
			HSInt tAngle = atof( ( LPCSTR )tValue );	

			pItemProperty = pSensorProperty->GetSubItem( 2 );
			tValue = pItemProperty->GetValue();
			HSDouble tHeight = atof( ( LPCSTR )tValue );
		
			pItemProperty = pSensorProperty->GetSubItem( 3 );
			tValue = pItemProperty->GetValue();
			tIndex = IndexWithString( tValue, pItemProperty );
			HSBool tForbid = ( tIndex == 0 );
						
			mParent->UpdatePositionSensor( i, tAngle, tHeight * 1000.0, tForbid, i < tShowCount, tChannelIdentifier );
		}		

		mParent->RefreshSensors();
	}	
	else if ( pProperty->GetData() == PROPERTY_Y )
	{		
		mParent->SetYAxisLength( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
	}	
	else if ( pProperty->GetData() == PROPERTY_MATERIAL_Y )
	{		
		mParent->SetMaterialYLength( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
	}		
	else if ( pProperty->GetData() == PROPERTY_MATERIAL_RADIUS )
	{		
		mParent->SetMaterialRadius( atof( ( LPCSTR )tPropValue ) * 1000.0 / 2.0 );	
	}		
	else if ( pProperty->GetData() == PROPERTY_HIT_RADUS )
	{		
		mParent->SetHitRadius( atoi( ( LPCSTR )tPropValue ) );	
	}	
	else if ( pProperty->GetData() > 1000 )
	{
		HSInt tSensorIndex = pProperty->GetData() / 1000;
		CMFCPropertyGridProperty *pSensorProperty = pGroup->GetSubItem( tSensorIndex - 1 + 5 );

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
		HSDouble tAngle = atof( ( LPCSTR )tValue );
		
		pItemProperty = pSensorProperty->GetSubItem( 2 );
		tValue = pItemProperty->GetValue();
		HSDouble tHeight = atof( ( LPCSTR )tValue );
	
		pItemProperty = pSensorProperty->GetSubItem( 3 );
		tValue = pItemProperty->GetValue();
		tIndex = IndexWithString( tValue, pItemProperty );
		HSBool tForbid = ( tIndex == 0 );

		mParent->UpdatePositionSensor( tSensorIndex - 1, tAngle, tHeight * 1000.0, tForbid, HSTrue, tChannelIdentifier );

		mParent->RefreshSensors();
	}
}

HSVoid CCylinderSurfacePositionProperty::SensorClicked( HSInt tIndex )
{
	mParent->SetFocusSensor( tIndex );
}