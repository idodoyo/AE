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
	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("��¯��λ����"));
	
	/*
	CMFCPropertyGridProperty *pCoord = new CMFCPropertyGridProperty( "����" );

	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("X����"), ( _variant_t )1, _T( "X����(��)!" ), PROPERTY_X_AXIS_LENGTH );	
	pProp->EnableSpinControl( TRUE, 1, 100 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("Y����"), ( _variant_t )1, _T( "Y����(��)!" ), PROPERTY_Y_AXIS_LENGTH );
	pProp->EnableSpinControl( TRUE, 1, 100 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pCoord );
	*/

	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("����"), "��¯1", _T( "���ù�¯����!" ), PROPERTY_BOILER_NAME );	
	pGroup->AddSubItem( pProp );	

	CMFCPropertyGridProperty *pMaterialCoord = new CMFCPropertyGridProperty( "��¯�ߴ�(��)" );

	pProp = new CMFCPropertyGridProperty(_T("ֱ��"), ( _variant_t )8.0, _T( "��¯ֱ��!" ), PROPERTY_RADIUS );	
	pMaterialCoord->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("���¼�1"), ( _variant_t )4.0, _T( "����ʮ�ֲ��¼ܳ���!" ), PROPERTY_PILLAR1_LENGTH );		
	pMaterialCoord->AddSubItem( pProp );
	pProp = new CMFCPropertyGridProperty(_T("���¼�2"), ( _variant_t )4.0, _T( "����ʮ�ֲ��¼ܳ���!" ), PROPERTY_PILLAR2_LENGTH );		
	pMaterialCoord->AddSubItem( pProp );
	pProp = new CMFCPropertyGridProperty(_T("���¼�3"), ( _variant_t )4.0, _T( "����ʮ�ֲ��¼ܳ���!" ), PROPERTY_PILLAR3_LENGTH );		
	pMaterialCoord->AddSubItem( pProp );
	pProp = new CMFCPropertyGridProperty(_T("���¼�4"), ( _variant_t )4.0, _T( "����ʮ�ֲ��¼ܳ���!" ), PROPERTY_PILLAR4_LENGTH );		
	pMaterialCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pMaterialCoord );	

	CMFCPropertyGridProperty *pBoilerInfo = new CMFCPropertyGridProperty( "������Ϣ" );
	pProp = new CMFCPropertyGridProperty(_T("��ʯ����Ȧ��"), ( _variant_t )10.0, _T( "���ÿ�ʯ����Ȧ��!" ), PROPERTY_IRONSTONE_CIRLE_COUNT );		
	pBoilerInfo->AddSubItem( pProp );
	pProp = new CMFCPropertyGridProperty(_T("��ʯ����ʱ�䣨�룩"), ( _variant_t )7.5, _T( "���ÿ�ʯ����һȦ��ʱ��!" ), PROPERTY_IRONSTONE_TIME );		
	pBoilerInfo->AddSubItem( pProp );
	pProp = new CMFCPropertyGridColorProperty( "��ʯ��ע��ɫ", RGB( 255, 0, 0 ) );
	pProp->AllowEdit( FALSE );
	pProp->Enable( FALSE );
	pBoilerInfo->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("��̿����Ȧ��"), ( _variant_t )11.5, _T( "���ý�̿����Ȧ��!" ), PROPERTY_COKE_CIRLE_COUNT );		
	pBoilerInfo->AddSubItem( pProp );
	pProp = new CMFCPropertyGridProperty(_T("��̿����ʱ�䣨�룩"), ( _variant_t )7.5, _T( "���ý�̿����һȦʱ��!" ), PROPERTY_COKE_TIME );		
	pBoilerInfo->AddSubItem( pProp );
	pProp = new CMFCPropertyGridColorProperty( "��̿��ע��ɫ", RGB( 0, 0, 255 ) );
	pProp->AllowEdit( FALSE );
	pProp->Enable( FALSE );
	pBoilerInfo->AddSubItem( pProp );

	pGroup->AddSubItem( pBoilerInfo );
	
	CMFCPropertyGridProperty *pSensor = new CMFCPropertyGridProperty( "����Դ" );
	for ( HSInt i = 0; i < POSITION_SENSOR_MAX_NUM; i++ )
	{		
		CString tStrSensorName;
		tStrSensorName.Format( "������%d", i + 1 );
		pProp = new CMFCPropertyGridProperty( tStrSensorName, _T( "" ), _T( "ѡ������Դ" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_DATA_SOURCE );
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