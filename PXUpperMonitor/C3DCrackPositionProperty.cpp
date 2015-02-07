// C3DCrackPositionProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "C3DCrackPositionProperty.h"
#include "afxdialogex.h"
#include "C3DCrackPosition.h"
#include "CrackPosition.h"
#include "HSLogProtocol.h"
#include "MainFrm.h"

// C3DCrackPositionProperty dialog


C3DCrackPositionProperty::C3DCrackPositionProperty(CWnd* pParent /*=NULL*/)
	: CNormalProperty(pParent)
{
	mParent = NULL;
	mForPosition = HSTrue;
}

C3DCrackPositionProperty::~C3DCrackPositionProperty()
{
}

void C3DCrackPositionProperty::SetParent( IGraphicProtocol *pGraphic )
{
	mParent = NULL;		
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< C3DCrackPosition * >( pGraphic );		
	}	

	CNormalProperty::SetParent( pGraphic );
}

void C3DCrackPositionProperty::RefreshDataSource()
{	
	if ( mParent == NULL )
	{
		return;
	}

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	
	CMFCPropertyGridProperty *pGroup = mPropertyGrid.GetProperty( 0 );
	HSInt tSensorOnCount = 0;
	for ( HSInt tSensorIndex = 0; tSensorIndex < POSITION_SENSOR_MAX_NUM; tSensorIndex++ )
	{
		CCrackPosition::PositionSensor *pSensorInfo = NULL;
		if ( mParent->RefCrackPosition() != NULL )
		{
			pSensorInfo = mParent->RefCrackPosition()->GetPositionSensor( tSensorIndex );
		}

		if ( pSensorInfo != NULL && pSensorInfo->IsOn )
		{			
			tSensorOnCount++;
		}

		DWORD tPropDataSourceID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_DATA_SOURCE;
		CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( tPropDataSourceID );

		IDataHandler *pDataHandler = NULL;
		if ( pSensorInfo != NULL && ( pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( pSensorInfo->DataIdentifier ) ) != NULL )
		{		
			CString tStr;			
			tStr.Format( "%s - ͨ��%d", pDataHandler->Name().c_str(), pSensorInfo->DataIdentifier.ChannelIndex() + 1 );			
			pProp->SetValue( tStr );			
		}
		else
		{
			pProp->SetValue( "" );
		}

		pProp->Redraw();			

		HSString tFristAxisName = "X����";
		HSString tSecondAxisName = "Y����";

		DWORD tPropPosXID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_FIRST_COORD;
		pProp = mPropertyGrid.FindItemByData( tPropPosXID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetName( tFristAxisName.c_str() );
			pProp->SetValue( ( _variant_t )( pSensorInfo->XPos ) );
			pProp->Redraw();
		}

		DWORD tPropPosYID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_SECOND_COORD;
		pProp = mPropertyGrid.FindItemByData( tPropPosYID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetName( tSecondAxisName.c_str() );
			pProp->SetValue( ( _variant_t )( pSensorInfo->YPos ) );
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

	HSDouble tXLength = 0;
	HSDouble tYLength = 0;	

	if ( mParent != NULL )
	{
		mParent->GetAxisLength( tXLength, tYLength );
	}	
	
	pProp = mPropertyGrid.FindItemByData( PROPERTY_X );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( tXLength ) ) );	

		COleVariant vtVar( ( long )( tXLength ) );
		pProp->SetValue( vtVar );	

		pProp->Redraw();	
	}	

	pProp = mPropertyGrid.FindItemByData( PROPERTY_Y );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( tYLength ) ) );	

		COleVariant vtVar( ( long )( tYLength ) );
		pProp->SetValue( vtVar );	

		pProp->Redraw();
	}

	CCrackPosition *pCrackPosition = mParent->RefCrackPosition();	

	pProp = mPropertyGrid.FindItemByData( PROPERTY_MATERIAL_X );	
	
	if ( mParent != NULL && pCrackPosition != NULL )
	{
		pProp->SetValue( ( _variant_t )( pCrackPosition->MaterialLength() / 1000.0 ) );		
		pProp->Redraw();
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_MATERIAL_Y );	
	if ( mParent != NULL && pCrackPosition != NULL )
	{
		pProp->SetValue( ( _variant_t )( pCrackPosition->MaterialWidth() / 1000.0 ) );		
		pProp->Redraw();
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_CENTER_X );	
	if ( mParent != NULL && pCrackPosition != NULL )
	{
		pProp->SetValue( ( _variant_t )( pCrackPosition->CenterXCoord() ) );		
		pProp->Redraw();
	}

	pProp = mPropertyGrid.FindItemByData( PROPERTY_CENTER_Y );	
	if ( mParent != NULL && pCrackPosition != NULL )
	{
		pProp->SetValue( ( _variant_t )( pCrackPosition->CenterYCoord() ) );		
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

HSVoid C3DCrackPositionProperty::InitDialog()
{
	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("3D�ѷ�ͼ����"));	
	
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("����������"), ( _variant_t )6, _T( "���ô���������!" ), PROPERTY_SENSOR_COUNT );
	//pProp->EnableSpinControl( TRUE, 6, 100 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

	CMFCPropertyGridProperty *pCoord = new CMFCPropertyGridProperty( "����" );

	pProp = new CMFCPropertyGridProperty(_T("X����"), ( _variant_t )10, _T( "X����(��)!" ), PROPERTY_X );	
	pProp->EnableSpinControl( TRUE, 1, 100000 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("Y����"), ( _variant_t )10, _T( "Y����(��)!" ), PROPERTY_Y );
	pProp->EnableSpinControl( TRUE, 1, 100000 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pCoord );


	CMFCPropertyGridProperty *pMaterialCoord = new CMFCPropertyGridProperty( "���ϳߴ�" );

	pProp = new CMFCPropertyGridProperty(_T("��"), ( _variant_t )8.0, _T( "X����!" ), PROPERTY_MATERIAL_X );	
	pProp->AllowEdit( FALSE );
	pMaterialCoord->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("��"), ( _variant_t )8.0, _T( "Y����!" ), PROPERTY_MATERIAL_Y );	
	pProp->AllowEdit( FALSE );
	pMaterialCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pMaterialCoord );

	CMFCPropertyGridProperty *pCenterCoord = new CMFCPropertyGridProperty( "�;�����" );
	pProp = new CMFCPropertyGridProperty(_T("����"), ( _variant_t )16.0, _T( "�;���������!" ), PROPERTY_CENTER_X );
	pProp->AllowEdit( FALSE );
	pCenterCoord->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("γ��"), ( _variant_t )19.0, _T( "�;�����γ��!" ), PROPERTY_CENTER_Y );
	pProp->AllowEdit( FALSE );
	pCenterCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pCenterCoord );

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

		pProp = new CMFCPropertyGridProperty(_T("����"), ( _variant_t )0.5, _T( "" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_FIRST_COORD );
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("γ��"), ( _variant_t )0.5, _T( "" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_SECOND_COORD );
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );		

		pProp = new CMFCPropertyGridProperty(_T("����"), _T( "��" ), _T( "���ô�����!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_FORBID );		
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pSensor->Show( i < 6 );
		pGroup->AddSubItem( pSensor );
	}	

	mPropertyGrid.AddProperty( pGroup );	
}

HSVoid C3DCrackPositionProperty::PropertyChanged( CMFCPropertyGridProperty * pProperty )
{
	CString tPropValue = pProperty->GetValue();
	
	if ( pProperty->GetData() == PROPERTY_X )
	{		
		mParent->SetXAxisLength( atof( ( LPCSTR )tPropValue ) );	
	}		
	else if ( pProperty->GetData() == PROPERTY_Y )
	{		
		mParent->SetYAxisLength( atof( ( LPCSTR )tPropValue ) );	
	}
	else if ( pProperty->GetData() == PROPERTY_HIT_RADUS )
	{		
		mParent->SetHitRadius( atoi( ( LPCSTR )tPropValue ) );	
	}
}

HSVoid C3DCrackPositionProperty::SensorClicked( HSInt tIndex )
{
	mParent->SetFocusSensor( tIndex );
}