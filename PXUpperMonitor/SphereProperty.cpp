// SphereProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "SphereProperty.h"
#include "afxdialogex.h"
#include "PlanePositionProperty.h"
#include "SpherePosition.h"
#include "HSLogProtocol.h"
#include "MainFrm.h"



// CSphereProperty dialog


CSphereProperty::CSphereProperty(CWnd* pParent /*=NULL*/)
	: CNormalProperty(pParent)
{
	mParent = NULL;
	mForPosition = HSTrue;
}

CSphereProperty::~CSphereProperty()
{
}

void CSphereProperty::SetParent( IGraphicProtocol *pGraphic )
{	
	mParent = NULL;		
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CSpherePosition * >( pGraphic );
	}

	CNormalProperty::SetParent( pGraphic );
}

void CSphereProperty::RefreshDataSource()
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
		CSpherePosition::PositionSensor *pSensorInfo = mParent->GetPositionSensor( tSensorIndex );
		if ( pSensorInfo != NULL && pSensorInfo->IsOn )
		{			
			tSensorOnCount++;
		}
				
		DWORD tPropDataSourceID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_DATA_SOURCE;
		CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( tPropDataSourceID );
		SetDataSourceForItem( pProp, pSensorInfo );
		
		DWORD tPropPosXID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_ANGLE_D;
		pProp = mPropertyGrid.FindItemByData( tPropPosXID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetValue( ( _variant_t )pSensorInfo->AngleDirction );
			pProp->Redraw();
		}
		
		DWORD tPropPosYID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_ANGLE_U;
		pProp = mPropertyGrid.FindItemByData( tPropPosYID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetValue( ( _variant_t )pSensorInfo->AngleUp );
			pProp->Redraw();
		}
				
		DWORD tPropForbidID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_FORBID;
		pProp = mPropertyGrid.FindItemByData( tPropForbidID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetValue( pSensorInfo->Forbid ? _T( "��" ) : _T( "��" ) );
			pProp->Redraw();
		}
			
		CMFCPropertyGridProperty *pProperty = pGroup->GetSubItem( tSensorIndex + 4 );
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
	
	pProp = mPropertyGrid.FindItemByData( PROPERTY_RADIUS );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->MaterialRadius() / 1000.0 * 2.0 ) );		
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
}


HSBool CSphereProperty::IsDataHandlerMachedWith( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSVoid *pSensorInfo )
{
	CSpherePosition::PositionSensor *pSensor = ( CSpherePosition::PositionSensor * )pSensorInfo;
	return ( pSensorInfo != NULL && pSensor->DataIdentifier == tChannelIdentifier );
}

HSVoid CSphereProperty::InitDialog()
{
	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("���涨λ����"));	
	
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("����������"), ( _variant_t )6, _T( "���ô���������!" ), PROPERTY_SENSOR_COUNT );
	pProp->EnableSpinControl( TRUE, 6, 100 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("ֱ��(��)"), ( _variant_t )10.0, _T( "���ò��ϳߴ�!" ), PROPERTY_RADIUS );
	pGroup->AddSubItem( pProp );

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

		pProp = new CMFCPropertyGridProperty(_T("��λ��"), ( _variant_t )0.0, _T( "��λ��( 0 - 360 )��!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_ANGLE_D );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("�����"), ( _variant_t )0.0, _T( "�����( 0 - 180 )��!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_ANGLE_U );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("����"), _T( "��" ), _T( "���ô�����!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_FORBID );
		pProp->AddOption( _T( "��" ) );
		pProp->AddOption( _T( "��" ) );
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pSensor->Show( i < 6 );				
		pGroup->AddSubItem( pSensor );
	}

	mPropertyGrid.AddProperty( pGroup );	
}

HSVoid CSphereProperty::PropertyChanged( CMFCPropertyGridProperty * pProperty )
{
	CString tPropValue = pProperty->GetValue();
	
	CMFCPropertyGridProperty *pGroup = mPropertyGrid.GetProperty( 0 );
	if ( pProperty->GetData() == PROPERTY_SENSOR_COUNT )
	{					
		HSInt tShowCount = atoi( ( LPCSTR )tPropValue );		
		for ( HSInt i = 0; i < POSITION_SENSOR_MAX_NUM; i++ )
		{
			CMFCPropertyGridProperty *pSensorProperty = pGroup->GetSubItem( i + 4 );
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
			HSDouble tAngleD = atof( ( LPCSTR )tValue );

			pItemProperty = pSensorProperty->GetSubItem( 2 );
			tValue = pItemProperty->GetValue();
			HSDouble tAngleU = atof( ( LPCSTR )tValue );

			pItemProperty = pSensorProperty->GetSubItem( 3 );
			tValue = pItemProperty->GetValue();
			tIndex = IndexWithString( tValue, pItemProperty );
			HSBool tForbid = ( tIndex == 0 );

			mParent->UpdatePositionSensor( i, tAngleD, tAngleU, tForbid, i < tShowCount, tChannelIdentifier );
		}		

		mParent->RefreshSensors();
	}
	else if ( pProperty->GetData() == PROPERTY_RADIUS )
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
		CMFCPropertyGridProperty *pSensorProperty = pGroup->GetSubItem( tSensorIndex - 1 + 4 );

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
		HSDouble tAngleU = atof( ( LPCSTR )tValue );

		pItemProperty = pSensorProperty->GetSubItem( 2 );
		tValue = pItemProperty->GetValue();
		HSDouble tAngleD = atof( ( LPCSTR )tValue );

		pItemProperty = pSensorProperty->GetSubItem( 3 );
		tValue = pItemProperty->GetValue();
		tIndex = IndexWithString( tValue, pItemProperty );
		HSBool tForbid = ( tIndex == 0 );

		mParent->UpdatePositionSensor( tSensorIndex - 1, tAngleU, tAngleD, tForbid, HSTrue, tChannelIdentifier );
		mParent->RefreshSensors();
	}
}

HSVoid CSphereProperty::SensorClicked( HSInt tIndex )
{
	mParent->SetFocusSensor( tIndex );
}