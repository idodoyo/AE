// CylinderPositionProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "CylinderPositionProperty.h"
#include "afxdialogex.h"
#include "PlanePositionProperty.h"
#include "CylinderPosition.h"
#include "HSLogProtocol.h"
#include "MainFrm.h"


// CCylinderPositionProperty dialog

IMPLEMENT_DYNAMIC(CCylinderPositionProperty, CDialogEx)

CCylinderPositionProperty::CCylinderPositionProperty(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCylinderPositionProperty::IDD, pParent)
{
	mParent = NULL;
	mIsInit = HSFalse;
}

CCylinderPositionProperty::~CCylinderPositionProperty()
{
}

void CCylinderPositionProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID_CYLINDER, mGridCylinder);
}


BEGIN_MESSAGE_MAP(CCylinderPositionProperty, CDialogEx)
	ON_WM_SIZE()	
	ON_STN_CLICKED(IDC_MFCPROPERTYGRID_CYLINDER, &CCylinderPositionProperty::OnStnClickedMfcpropertygridCylinder)
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
END_MESSAGE_MAP()


// CCylinderPositionProperty message handlers


BOOL CCylinderPositionProperty::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	mGridCylinder.EnableHeaderCtrl(FALSE);
	mGridCylinder.EnableDescriptionArea();
	mGridCylinder.SetVSDotNetLook();
	mGridCylinder.MarkModifiedProperties();	

	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("方体定位属性"));	
	
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("传感器数量"), ( _variant_t )6, _T( "设置传感器数量!" ), PROPERTY_SENSOR_COUNT );
	pProp->EnableSpinControl( TRUE, 6, 100 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

	CMFCPropertyGridProperty *pCoord = new CMFCPropertyGridProperty( "坐标" );

	pProp = new CMFCPropertyGridProperty(_T("X坐标"), ( _variant_t )10, _T( "X坐标(长)!" ), PROPERTY_X );	
	pProp->EnableSpinControl( TRUE, 1, 1000 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("Y坐标"), ( _variant_t )10, _T( "Y坐标(高)!" ), PROPERTY_Y );
	pProp->EnableSpinControl( TRUE, 1, 1000 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("Z坐标"), ( _variant_t )10, _T( "Z坐标(宽)!" ), PROPERTY_Z );
	pProp->EnableSpinControl( TRUE, 1, 1000 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pCoord );



	CMFCPropertyGridProperty *pMaterialCoord = new CMFCPropertyGridProperty( "材料尺寸" );

	pProp = new CMFCPropertyGridProperty(_T("长"), ( _variant_t )8.0, _T( "X坐标!" ), PROPERTY_MATERIAL_X );	
	pMaterialCoord->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("高"), ( _variant_t )8.0, _T( "Y坐标!" ), PROPERTY_MATERIAL_Y );	
	pMaterialCoord->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("宽"), ( _variant_t )8.0, _T( "Z坐标!" ), PROPERTY_MATERIAL_Z );	
	pMaterialCoord->AddSubItem( pProp );

	pGroup->AddSubItem( pMaterialCoord );	

	pProp = new CMFCPropertyGridProperty(_T("撞击半径"), ( _variant_t )3, _T( "设置撞击点大小!" ), PROPERTY_HIT_RADUS );
	pProp->EnableSpinControl( TRUE, 1, 3 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

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
	
	for ( HSInt i = 0; i < POSITION_SENSOR_MAX_NUM; i++ )
	{
		CString tStrSensorName;
		tStrSensorName.Format( "传感器%d", i + 1 );
		CMFCPropertyGridProperty *pSensor = new CMFCPropertyGridProperty( tStrSensorName );

		pProp = new CMFCPropertyGridProperty(_T("数据源"), _T( "" ), _T( "选择数据源" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_DATA_SOURCE );		
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("面"), _T( "前" ), _T( "XY为前面" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_FACE );
		pProp->AddOption( _T( "前" ) );
		pProp->AddOption( _T( "后" ) );
		pProp->AddOption( _T( "左" ) );
		pProp->AddOption( _T( "右" ) );
		pProp->AddOption( _T( "上" ) );
		pProp->AddOption( _T( "下" ) );
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("X坐标"), ( _variant_t )0.5, _T( "" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_FIRST_COORD );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("Y坐标"), ( _variant_t )0.5, _T( "" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_SECOND_COORD );
		pSensor->AddSubItem( pProp );		

		pProp = new CMFCPropertyGridProperty(_T("禁用"), _T( "否" ), _T( "禁用传感器!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_FORBID );
		pProp->AddOption( _T( "是" ) );
		pProp->AddOption( _T( "否" ) );
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pSensor->Show( i < 6 );				
		pGroup->AddSubItem( pSensor );
	}	

	mGridCylinder.AddProperty( pGroup );	

	mIsInit = true;	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CCylinderPositionProperty::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( !mIsInit )
	{
		return;
	}

	CRect tRect;
	this->GetClientRect( &tRect );
	mGridCylinder.MoveWindow( tRect );
}


void CCylinderPositionProperty::OnStnClickedMfcpropertygridCylinder()
{
	// TODO: Add your control notification handler code here

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );		
	
	CMFCPropertyGridProperty* pProp = mGridCylinder.HitTest( tPoint, NULL, HSTrue );
	if ( mParent != NULL )
	{
		HSInt tIndex = -1;
		if ( pProp && pProp->GetData() > 1000 )
		{
			tIndex = pProp->GetData() / 1000 - 1;
		}

		mParent->SetFocusSensor( tIndex );		
	}
}

void CCylinderPositionProperty::SetParent( IGraphicProtocol *pGraphic )
{
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CCylinderPosition * >( pGraphic );
		mGridCylinder.EnableWindow( true );
	}
	else
	{
		mParent = NULL;
		mGridCylinder.EnableWindow( false );
	}	
}

LRESULT CCylinderPositionProperty::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{
	if ( mParent == NULL )
	{
		return 0;
	}

	CMFCPropertyGridProperty * pProperty = ( CMFCPropertyGridProperty * ) lparam;	
	CString tPropValue = pProperty->GetValue();
	
	CMFCPropertyGridProperty *pGroup = mGridCylinder.GetProperty( 0 );
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
			HSInt tFace = IndexWithString( tValue, pItemProperty );			

			pItemProperty = pSensorProperty->GetSubItem( 2 );
			tValue = pItemProperty->GetValue();
			HSDouble tFirstCoord = atof( ( LPCSTR )tValue );

			pItemProperty = pSensorProperty->GetSubItem( 3 );
			tValue = pItemProperty->GetValue();
			HSDouble tSecondCoord = atof( ( LPCSTR )tValue );

			pItemProperty = pSensorProperty->GetSubItem( 4 );
			tValue = pItemProperty->GetValue();
			tIndex = IndexWithString( tValue, pItemProperty );
			HSBool tForbid = ( tIndex == 0 );
						
			mParent->UpdatePositionSensor( i, tFace, tFirstCoord * 1000.0, tSecondCoord * 1000.0, tForbid, i < tShowCount, tChannelIdentifier );
		}		

		mParent->RefreshSensors();
	}
	else if ( pProperty->GetData() == PROPERTY_X )
	{		
		mParent->SetXAxisLength( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
	}		
	else if ( pProperty->GetData() == PROPERTY_Y )
	{		
		mParent->SetYAxisLength( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
	}		
	else if ( pProperty->GetData() == PROPERTY_Z )
	{		
		mParent->SetZAxisLength( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
	}		
	else if ( pProperty->GetData() == PROPERTY_MATERIAL_X )
	{		
		mParent->SetMaterialXLength( atof( ( LPCSTR )tPropValue ) * 1000.0 );
	}		
	else if ( pProperty->GetData() == PROPERTY_MATERIAL_Y )
	{		
		mParent->SetMaterialYLength( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
	}		
	else if ( pProperty->GetData() == PROPERTY_MATERIAL_Z )
	{		
		mParent->SetMaterialZLength( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
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
		HSInt tFace = IndexWithString( tValue, pItemProperty );

		HSBool tNeedRefreshAxisName = ( pProperty->GetData() % 1000 == PROPERTY_SENSOR_FACE ? HSTrue : HSFalse );
		HSString tFristAxisName = "X坐标";
		HSString tSecondAxisName = "Y坐标";

		if ( tNeedRefreshAxisName )
		{
			mParent->FaceWithIndex( tFace, tFristAxisName, tSecondAxisName );			
		}

		pItemProperty = pSensorProperty->GetSubItem( 2 );		
		tValue = pItemProperty->GetValue();
		HSDouble tFirstCoord = atof( ( LPCSTR )tValue );

		if ( tNeedRefreshAxisName )
		{
			pItemProperty->SetName( tFristAxisName.c_str() );
			pItemProperty->Redraw();
		}

		pItemProperty = pSensorProperty->GetSubItem( 3 );
		tValue = pItemProperty->GetValue();
		HSDouble tSecondCoord = atof( ( LPCSTR )tValue );

		if ( tNeedRefreshAxisName )
		{
			pItemProperty->SetName( tSecondAxisName.c_str() );
			pItemProperty->Redraw();
		}

		pItemProperty = pSensorProperty->GetSubItem( 4 );
		tValue = pItemProperty->GetValue();
		tIndex = IndexWithString( tValue, pItemProperty );
		HSBool tForbid = ( tIndex == 0 );

		mParent->UpdatePositionSensor( tSensorIndex - 1, tFace, tFirstCoord * 1000.0, tSecondCoord * 1000.0, tForbid, HSTrue, tChannelIdentifier );

		mParent->RefreshSensors();
	}

	return 1;
}

void CCylinderPositionProperty::RefreshDataSource()
{	
	if ( mParent == NULL )
	{
		return;
	}

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	HSUInt tIndex = 0;
	IDataHandler *pDataHandler = NULL;
	while ( ( pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIndex( tIndex++ ) ) )
	{
		DEVICE_CH_IDENTIFIER tType = pDataHandler->Identifier();
		for ( HSInt i = 0; i < pDataHandler->ChannelNum(); i++ )
		{
			mDeviceChannelInfo[ tType ].push_back( i );
		}		
	}	
	
	CMFCPropertyGridProperty *pGroup = mGridCylinder.GetProperty( 0 );
	HSInt tSensorOnCount = 0;
	for ( HSInt tSensorIndex = 0; tSensorIndex < POSITION_SENSOR_MAX_NUM; tSensorIndex++ )
	{
		CCylinderPosition::PositionSensor *pSensorInfo = mParent->GetPositionSensor( tSensorIndex );
		if ( pSensorInfo != NULL && pSensorInfo->IsOn )
		{			
			tSensorOnCount++;
		}

		DWORD tPropDataSourceID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_DATA_SOURCE;
		CMFCPropertyGridProperty* pProp = mGridCylinder.FindItemByData( tPropDataSourceID );
		pProp->RemoveAllOptions();
		
		HSBool tGotDataSource = HSFalse;
		map< DEVICE_CH_IDENTIFIER, vector< HSInt > >::iterator pIterator = mDeviceChannelInfo.begin();
		while ( pIterator != mDeviceChannelInfo.end() )
		{
			IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( pIterator->first );
			for ( HSUInt i = 0; i < pIterator->second.size(); i++ )
			{
				CString tStr;			
				tStr.Format( "%s - 通道%d", pDataHandler->Name().c_str(), pIterator->second[ i ] + 1 );			
				pProp->AddOption( tStr );
				
				DEVICE_CH_IDENTIFIER tChannelIdentifier = pIterator->first;
				tChannelIdentifier.InitChannel( pIterator->second[ i ] );
				if ( mParent != NULL && pSensorInfo != NULL && pSensorInfo->DataIdentifier == tChannelIdentifier && !tGotDataSource )
				{
					pProp->SetValue( tStr );
					tGotDataSource = HSTrue;
				}
			}
			pIterator++;
		}

		if ( !tGotDataSource )
		{
			pProp->SetValue( "" );
		}
		pProp->Redraw();

		HSString tFristAxisName = "X坐标";
		HSString tSecondAxisName = "Y坐标";

		DWORD tPropPosFaceID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_FACE;
		pProp = mGridCylinder.FindItemByData( tPropPosFaceID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetValue( mParent->FaceWithIndex( pSensorInfo->Face, tFristAxisName, tSecondAxisName ).c_str() );
			pProp->Redraw();
		}

		DWORD tPropPosXID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_FIRST_COORD;
		pProp = mGridCylinder.FindItemByData( tPropPosXID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetName( tFristAxisName.c_str() );
			pProp->SetValue( ( _variant_t )( pSensorInfo->PosX / 1000.0 ) );
			pProp->Redraw();
		}

		DWORD tPropPosYID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_SECOND_COORD;
		pProp = mGridCylinder.FindItemByData( tPropPosYID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetName( tSecondAxisName.c_str() );
			pProp->SetValue( ( _variant_t )( pSensorInfo->PosY / 1000.0 ) );
			pProp->Redraw();
		}

		DWORD tPropForbidID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_FORBID;
		pProp = mGridCylinder.FindItemByData( tPropForbidID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{				
			pProp->SetValue( pSensorInfo->Forbid ? _T( "是" ) : _T( "否" ) );
			pProp->Redraw();
		}

		CMFCPropertyGridProperty *pProperty = pGroup->GetSubItem( tSensorIndex + 5 );
		pProperty->Show( pSensorInfo != NULL && pSensorInfo->IsOn );
		
	}		
	
	CMFCPropertyGridProperty* pProp = mGridCylinder.FindItemByData( PROPERTY_SENSOR_COUNT );	
	if ( mParent != NULL )
	{		
		pProp->SetValue( ( _variant_t )tSensorOnCount );		

		COleVariant vtVar( ( long )tSensorOnCount );
		pProp->SetValue( vtVar );	

		pProp->Redraw();
	}	

	HSDouble tXLength = 0;
	HSDouble tYLength = 0;
	HSDouble tZLength = 0;

	if ( mParent != NULL )
	{
		mParent->GetAxisLength( tXLength, tYLength, tZLength );
	}	
	
	pProp = mGridCylinder.FindItemByData( PROPERTY_X );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( tXLength / 1000.0 ) ) );	

		COleVariant vtVar( ( long )( tXLength / 1000.0 ) );
		pProp->SetValue( vtVar );	

		pProp->Redraw();	
	}	

	pProp = mGridCylinder.FindItemByData( PROPERTY_Y );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( tYLength / 1000.0 ) ) );	

		COleVariant vtVar( ( long )( tYLength / 1000.0 ) );
		pProp->SetValue( vtVar );	

		pProp->Redraw();
	}	

	pProp = mGridCylinder.FindItemByData( PROPERTY_Z );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( tZLength / 1000.0 ) ) );		

		COleVariant vtVar( ( long )( tZLength / 1000.0 ) );
		pProp->SetValue( vtVar );	

		pProp->Redraw();
	}	

	pProp = mGridCylinder.FindItemByData( PROPERTY_MATERIAL_X );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->MaterialXLength() / 1000.0 ) );		
		pProp->Redraw();
	}	

	pProp = mGridCylinder.FindItemByData( PROPERTY_MATERIAL_Y );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->MaterialYLength() / 1000.0 ) );		
		pProp->Redraw();
	}	

	pProp = mGridCylinder.FindItemByData( PROPERTY_MATERIAL_Z );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->MaterialZLength() / 1000.0 ) );		
		pProp->Redraw();	
	}	

	pProp = mGridCylinder.FindItemByData( PROPERTY_HIT_RADUS );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( mParent->HitRadius() ) ) );		

		COleVariant vtVar( ( long )( mParent->HitRadius() ) );
		pProp->SetValue( vtVar );	

		pProp->Redraw();	
	}
}
