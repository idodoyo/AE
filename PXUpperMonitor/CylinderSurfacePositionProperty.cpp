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

IMPLEMENT_DYNAMIC(CCylinderSurfacePositionProperty, CDialogEx)

CCylinderSurfacePositionProperty::CCylinderSurfacePositionProperty(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCylinderSurfacePositionProperty::IDD, pParent)
{
	mParent = NULL;
	mIsInit = HSFalse;
}

CCylinderSurfacePositionProperty::~CCylinderSurfacePositionProperty()
{
}

void CCylinderSurfacePositionProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);	
	DDX_Control(pDX, IDC_MFCPROPERTYGRID_CYLINDER_SURFACE, mGridCylinderSurface);
}


BEGIN_MESSAGE_MAP(CCylinderSurfacePositionProperty, CDialogEx)
	ON_STN_CLICKED(IDC_MFCPROPERTYGRID_CYLINDER_SURFACE, &CCylinderSurfacePositionProperty::OnStnClickedMfcpropertygridCylinderSurface)
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CCylinderSurfacePositionProperty message handlers


void CCylinderSurfacePositionProperty::OnStnClickedMfcpropertygridCylinderSurface()
{
	// TODO: Add your control notification handler code here

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );		
	
	CMFCPropertyGridProperty* pProp = mGridCylinderSurface.HitTest( tPoint, NULL, HSTrue );
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


BOOL CCylinderSurfacePositionProperty::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	
	mGridCylinderSurface.EnableHeaderCtrl(FALSE);
	mGridCylinderSurface.EnableDescriptionArea();
	mGridCylinderSurface.SetVSDotNetLook();
	mGridCylinderSurface.MarkModifiedProperties();	

	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("柱面定位属性"));
	
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("传感器数量"), ( _variant_t )3, _T( "设置传感器数量!" ), PROPERTY_SENSOR_COUNT );
	pProp->EnableSpinControl( TRUE, 3, 100 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );
	
	CMFCPropertyGridProperty *pCoord = new CMFCPropertyGridProperty( "坐标" );	

	pProp = new CMFCPropertyGridProperty(_T("Y坐标"), ( _variant_t )10, _T( "Y坐标(高)!" ), PROPERTY_Y );
	pProp->EnableSpinControl( TRUE, 1, 1000 );	
	pProp->AllowEdit( FALSE );
	pCoord->AddSubItem( pProp );	

	pGroup->AddSubItem( pCoord );


	CMFCPropertyGridProperty *pMaterialCoord = new CMFCPropertyGridProperty( "材料尺寸" );	

	pProp = new CMFCPropertyGridProperty(_T("高(m)"), ( _variant_t )1.0, _T( "Y坐标!" ), PROPERTY_MATERIAL_Y );	
	pMaterialCoord->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("直径(m)"), ( _variant_t )1.0, _T( "直径!" ), PROPERTY_MATERIAL_RADIUS );	
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

		pProp = new CMFCPropertyGridProperty(_T("角度"), ( _variant_t )16.0, _T( "" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_ANGLE );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("高度"), ( _variant_t )0.5, _T( "" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_HEIGHT );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("禁用"), _T( "否" ), _T( "禁用传感器!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_FORBID );
		pProp->AddOption( _T( "是" ) );
		pProp->AddOption( _T( "否" ) );
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pSensor->Show( i < 3 );				
		pGroup->AddSubItem( pSensor );
	}	
	
	mGridCylinderSurface.AddProperty( pGroup );	
	
	mIsInit = true;	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CCylinderSurfacePositionProperty::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( !mIsInit )
	{
		return;
	}

	CRect tRect;
	this->GetClientRect( &tRect );
	mGridCylinderSurface.MoveWindow( tRect );
}

void CCylinderSurfacePositionProperty::SetParent( IGraphicProtocol *pGraphic )
{
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CCylinderSurfacePosition * >( pGraphic );
		mGridCylinderSurface.EnableWindow( true );
	}
	else
	{
		mParent = NULL;
		mGridCylinderSurface.EnableWindow( false );		
	}	
}

LRESULT CCylinderSurfacePositionProperty::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{
	if ( mParent == NULL )
	{
		return 0;
	}
	
	CMFCPropertyGridProperty * pProperty = ( CMFCPropertyGridProperty * ) lparam;	
	CString tPropValue = pProperty->GetValue();
	
	CMFCPropertyGridProperty *pGroup = mGridCylinderSurface.GetProperty( 0 );
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
	
	return 1;
}

void CCylinderSurfacePositionProperty::RefreshDataSource()
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
	
	CMFCPropertyGridProperty *pGroup = mGridCylinderSurface.GetProperty( 0 );
	HSInt tSensorOnCount = 0;
	for ( HSInt tSensorIndex = 0; tSensorIndex < POSITION_SENSOR_MAX_NUM; tSensorIndex++ )
	{
		CCylinderSurfacePosition::PositionSensor *pSensorInfo = mParent->GetPositionSensor( tSensorIndex );
		if ( pSensorInfo != NULL && pSensorInfo->IsOn )
		{			
			tSensorOnCount++;
		}

		DWORD tPropDataSourceID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_DATA_SOURCE;
		CMFCPropertyGridProperty* pProp = mGridCylinderSurface.FindItemByData( tPropDataSourceID );
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
		

		DWORD tPropPosXID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_ANGLE;
		pProp = mGridCylinderSurface.FindItemByData( tPropPosXID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{						
			pProp->SetValue( ( _variant_t ) pSensorInfo->Angle );
			pProp->Redraw();
		}

		DWORD tPropPosYID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_HEIGHT;
		pProp = mGridCylinderSurface.FindItemByData( tPropPosYID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{						
			pProp->SetValue( ( _variant_t )( pSensorInfo->Height / 1000.0 ) );
			pProp->Redraw();
		}

		DWORD tPropForbidID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_FORBID;
		pProp = mGridCylinderSurface.FindItemByData( tPropForbidID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{				
			pProp->SetValue( pSensorInfo->Forbid ? _T( "是" ) : _T( "否" ) );
			pProp->Redraw();
		}

		CMFCPropertyGridProperty *pProperty = pGroup->GetSubItem( tSensorIndex + 5 );
		pProperty->Show( pSensorInfo != NULL && pSensorInfo->IsOn );
		
	}		
	
	CMFCPropertyGridProperty* pProp = mGridCylinderSurface.FindItemByData( PROPERTY_SENSOR_COUNT );	
	if ( mParent != NULL )
	{		
		pProp->SetValue( ( _variant_t )tSensorOnCount );	

		COleVariant vtVar( ( long )tSensorOnCount );
		pProp->SetValue( vtVar );	

		pProp->Redraw();
	}			
	
	pProp = mGridCylinderSurface.FindItemByData( PROPERTY_Y );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( mParent->GetAxisLength() / 1000.0 ) ) );	

		COleVariant vtVar( ( long )( mParent->GetAxisLength() / 1000.0 ) );
		pProp->SetValue( vtVar );	

		pProp->Redraw();
	}			

	pProp = mGridCylinderSurface.FindItemByData( PROPERTY_MATERIAL_Y );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->MaterialYLength() / 1000.0 ) );		
		pProp->Redraw();
	}	

	pProp = mGridCylinderSurface.FindItemByData( PROPERTY_MATERIAL_RADIUS );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->MaterialRadius() / 1000.0 * 2.0 ) );		
		pProp->Redraw();	
	}	

	pProp = mGridCylinderSurface.FindItemByData( PROPERTY_HIT_RADUS );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( mParent->HitRadius() ) ) );	

		COleVariant vtVar( ( long )( mParent->HitRadius() ) );
		pProp->SetValue( vtVar );

		pProp->Redraw();	
	}			
}


void CCylinderSurfacePositionProperty::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default	

	__super::OnLButtonDown(nFlags, point);
}
