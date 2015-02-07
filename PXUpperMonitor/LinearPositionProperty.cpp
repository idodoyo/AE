// LinearPositionProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "LinearPositionProperty.h"
#include "afxdialogex.h"
#include "LinearPosition.h"
#include "HSLogProtocol.h"
#include "MainFrm.h"


// CLinearPositionProperty dialog

IMPLEMENT_DYNAMIC(CLinearPositionProperty, CDialogEx)

CLinearPositionProperty::CLinearPositionProperty(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLinearPositionProperty::IDD, pParent)
{
	mParent = NULL;
}

CLinearPositionProperty::~CLinearPositionProperty()
{
}

void CLinearPositionProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID_LINEAR_POSITION, mGridPosition);
}


BEGIN_MESSAGE_MAP(CLinearPositionProperty, CDialogEx)
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
	ON_STN_CLICKED(IDC_MFCPROPERTYGRID_LINEAR_POSITION, &CLinearPositionProperty::OnClickedMfcpropertygridLinearPosition)
END_MESSAGE_MAP()


// CLinearPositionProperty message handlers

void CLinearPositionProperty::SetParent( IGraphicProtocol *pGraphic )
{
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CLinearPosition * >( pGraphic );
		mGridPosition.EnableWindow( true );
	}
	else
	{
		mParent = NULL;
		mGridPosition.EnableWindow( false );
	}	
}

void CLinearPositionProperty::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( !mIsInit )
	{
		return;
	}

	CRect tRect;
	this->GetClientRect( &tRect );
	mGridPosition.MoveWindow( tRect );
}


BOOL CLinearPositionProperty::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	mGridPosition.EnableHeaderCtrl(FALSE);
	mGridPosition.EnableDescriptionArea();
	mGridPosition.SetVSDotNetLook();
	mGridPosition.MarkModifiedProperties();	

	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("线性定位属性"));	
	
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("传感器数量"), ( _variant_t )2, _T( "设置传感器数量!" ), PROPERTY_SENSOR_COUNT );
	pProp->EnableSpinControl( TRUE, 2, 100 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("宽度(米)"), ( _variant_t )1.0, _T( "设置材料宽度!" ), PROPERTY_WIDTH );
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("X坐标"), ( _variant_t )2, _T( "设置X坐标!" ), PROPERTY_X_AXIS_LENGTH );
	pProp->EnableSpinControl( TRUE, 1, 1000 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("撞击半径"), ( _variant_t )3, _T( "设置撞击点大小!" ), PROPERTY_HIT_RADUS );
	pProp->EnableSpinControl( TRUE, 1, 3 );	
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

	CMFCPropertyGridProperty *pCheckCircle = new CMFCPropertyGridProperty( "活度计算" );
	pProp = new CMFCPropertyGridProperty(_T("半径(毫米)"), ( _variant_t )20, _T( "活度计算点大小!" ), PROPERTY_CHECK_CIRCLE_RADIUS );
	pProp->EnableSpinControl( TRUE, 5, 500 );	
	pProp->AllowEdit( FALSE );
	pCheckCircle->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("X坐标"), ( _variant_t )0.5, _T( "活度计算点X坐标(米)!" ), PROPERTY_CHECK_CIRCLE_X );
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
	
	for ( HSInt i = 0; i < POSITION_SENSOR_MAX_NUM; i++ )
	{
		CString tStrSensorName;
		tStrSensorName.Format( "传感器%d", i + 1 );
		CMFCPropertyGridProperty *pSensor = new CMFCPropertyGridProperty( tStrSensorName );

		pProp = new CMFCPropertyGridProperty(_T("数据源"), _T( "" ), _T( "选择数据源" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_DATA_SOURCE );	
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("X坐标"), ( _variant_t )0.5, _T( "传感器坐标(米)!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_X );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("禁用"), _T( "否" ), _T( "禁用传感器!" ), 1000 * ( i + 1 ) + PROPERTY_SENSOR_FORBID );
		pProp->AddOption( _T( "是" ) );
		pProp->AddOption( _T( "否" ) );
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pSensor->Show( i < 2 );				
		pGroup->AddSubItem( pSensor );
	}	

	mGridPosition.AddProperty( pGroup );	

	mIsInit = true;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CLinearPositionProperty::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{
	if ( mParent == NULL )
	{
		return 0;
	}

	CMFCPropertyGridProperty * pProperty = ( CMFCPropertyGridProperty * ) lparam;	
	CString tPropValue = pProperty->GetValue();
	
	CMFCPropertyGridProperty *pGroup = mGridPosition.GetProperty( 0 );
	if ( pProperty->GetData() == PROPERTY_SENSOR_COUNT )
	{					
		HSInt tShowCount = atoi( ( LPCSTR )tPropValue );
		//HSInt tTotalCount = pGroup->GetSubItemsCount() - 2;
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
			HSDouble tPos = atof( ( LPCSTR )tValue ) * 1000.0;

			pItemProperty = pSensorProperty->GetSubItem( 2 );
			tValue = pItemProperty->GetValue();
			tIndex = IndexWithString( tValue, pItemProperty );
			HSBool tForbid = ( tIndex == 0 );

			mParent->UpdatePositionSensor( i, tPos, tForbid, i < tShowCount, tChannelIdentifier );
		}
	}
	else if ( pProperty->GetData() == PROPERTY_WIDTH )
	{		
		mParent->SetMaterialLength( atof( ( LPCSTR )tPropValue ) * 1000.0 );	
	}	
	else if ( pProperty->GetData() == PROPERTY_X_AXIS_LENGTH )
	{		
		mParent->SetXAxisLength( atof( ( LPCSTR )tPropValue ) );	
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
		HSDouble tPos = atof( ( LPCSTR )tValue ) * 1000.0;

		pItemProperty = pSensorProperty->GetSubItem( 2 );
		tValue = pItemProperty->GetValue();
		tIndex = IndexWithString( tValue, pItemProperty );
		HSBool tForbid = ( tIndex == 0 );

		mParent->UpdatePositionSensor( tSensorIndex - 1, tPos, tForbid, HSTrue, tChannelIdentifier );
	}

	return 1;
}

void CLinearPositionProperty::RefreshDataSource()
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

	CMFCPropertyGridProperty *pGroup = mGridPosition.GetProperty( 0 );
	HSInt tSensorOnCount = 0;
	for ( HSInt tSensorIndex = 0; tSensorIndex < POSITION_SENSOR_MAX_NUM; tSensorIndex++ )
	{
		CLinearPosition::PositionSensor *pSensorInfo = mParent->GetPositionSensor( tSensorIndex );
		if ( pSensorInfo != NULL && pSensorInfo->IsOn )
		{			
			tSensorOnCount++;
		}

		DWORD tPropDataSourceID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_DATA_SOURCE;
		CMFCPropertyGridProperty* pProp = mGridPosition.FindItemByData( tPropDataSourceID );
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

		DWORD tPropPosID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_X;
		pProp = mGridPosition.FindItemByData( tPropPosID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetValue( ( _variant_t )( pSensorInfo->Pos / 1000.0 ) );
			pProp->Redraw();
		}

		DWORD tPropForbidID = 1000 * ( tSensorIndex + 1 ) + PROPERTY_SENSOR_FORBID;
		pProp = mGridPosition.FindItemByData( tPropForbidID );
		if ( mParent != NULL && pSensorInfo != NULL )
		{			
			pProp->SetValue( pSensorInfo->Forbid ? _T( "是" ) : _T( "否" ) );
			pProp->Redraw();
		}

		CMFCPropertyGridProperty *pProperty = pGroup->GetSubItem( tSensorIndex + 6 );
		pProperty->Show( pSensorInfo != NULL && pSensorInfo->IsOn );
	}
	
	CMFCPropertyGridProperty* pProp = mGridPosition.FindItemByData( PROPERTY_SENSOR_COUNT );	
	if ( mParent != NULL )
	{		
		pProp->SetValue( ( _variant_t )tSensorOnCount );

		COleVariant vtVar( ( long )tSensorOnCount );
		pProp->SetValue( vtVar );		

		pProp->Redraw();
	}	
	
	pProp = mGridPosition.FindItemByData( PROPERTY_WIDTH );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->MaterialLength() / 1000.0 ) );		
		pProp->Redraw();	
	}

	pProp = mGridPosition.FindItemByData( PROPERTY_X_AXIS_LENGTH );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( mParent->XAxisLength() ) ) );	

		COleVariant vtVar( ( long )mParent->XAxisLength() );
		pProp->SetValue( vtVar );	

		pProp->Redraw();	
	}

	pProp = mGridPosition.FindItemByData( PROPERTY_HIT_RADUS );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( mParent->HitRadius() ) ) );

		COleVariant vtVar( ( long )mParent->HitRadius() );
		pProp->SetValue( vtVar );	

		pProp->Redraw();	
	}

	pProp = mGridPosition.FindItemByData( PROPERTY_CHECK_CIRCLE_RADIUS );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( ( HSInt )( mParent->GetCheckRadius() ) ) );	

		COleVariant vtVar( ( long )mParent->GetCheckRadius() );
		pProp->SetValue( vtVar );	

		pProp->Redraw();	
	}

	pProp = mGridPosition.FindItemByData( PROPERTY_CHECK_CIRCLE_X );	
	if ( mParent != NULL )
	{
		pProp->SetValue( ( _variant_t )( mParent->GetCheckPos() / 1000.0 ) );		
		pProp->Redraw();	
	}	
}

void CLinearPositionProperty::OnClickedMfcpropertygridLinearPosition()
{
	// TODO: Add your control notification handler code here

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );		
	
	CMFCPropertyGridProperty* pProp = mGridPosition.HitTest( tPoint, NULL, HSTrue );
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
