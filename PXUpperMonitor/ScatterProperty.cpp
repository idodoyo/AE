// ScatterProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "ScatterProperty.h"
#include "afxdialogex.h"
#include "GraphicScatter.h"
#include "MainFrm.h"

// CScatterProperty dialog

CScatterProperty::CScatterProperty(CWnd* pParent /*=NULL*/)
	: CNormalProperty(pParent)
{
	mParent = NULL;
	mTotalChannelEnabled = HSTrue;
}

CScatterProperty::~CScatterProperty()
{
}

void CScatterProperty::SetParent( IGraphicProtocol *pGraphic )
{
	mParent = NULL;		
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CGraphicScatter * >( pGraphic );		
	}	

	CNormalProperty::SetParent( pGraphic );
}

void CScatterProperty::RefreshDataSource()
{
	if ( mParent == NULL )
	{
		return;
	}

	RefreshDeviceChannel();

	CMFCPropertyGridProperty* pProp = mPropertyGrid.FindItemByData( PROPERTY_DATA_SOURCE );
	SetDataSourceForItem( pProp );			
	
	pProp = mPropertyGrid.FindItemByData( PROPERTY_X );	
	if ( mParent != NULL )
	{		
		pProp->SetValue( mParent->XArg()->ArgName().c_str() );			
	}
	
	pProp->Redraw();
	
	
	pProp = mPropertyGrid.FindItemByData( PROPERTY_Y );	
	if ( mParent != NULL )
	{
		pProp->SetValue( mParent->YArg()->ArgName().c_str() );			
	}

	pProp->Redraw();

	/*
	pProp = mGridRelation.FindItemByData( PROPERTY_FILL_RECT );	
	if ( mParent == NULL || mParent->IsSolidRect() )
	{				
		pProp->SetValue( "实心柱状图" );
	}
	else
	{
		pProp->SetValue( "空心柱状图" );
	}

	pProp->Redraw();

	
	pProp = mGridRelation.FindItemByData( PROPERTY_SQUARE_LENGTH );	
	if ( mParent == NULL )
	{				
		pProp->SetValue( 1.0 );
	}
	else
	{
		pProp->SetValue( mParent->UnitDelta() );
	}

	pProp->Redraw();
	*/
}


HSBool CScatterProperty::IsDataHandlerMachedWith( IDataHandler *pDataHandler, HSUInt tChannel )
{
	return ( pDataHandler == mParent->DataHandler() && ( mParent->DataIdentifier().ChannelIndex() == tChannel || mParent->DataIdentifier().NumOfChannels() == pDataHandler->ChannelNum() ) );
}

HSVoid CScatterProperty::InitDialog()
{
	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("散点图属性"));	
	
	HSString *pArgs = NULL;
	HSInt tArgNum = CArgDataFectcher::GetArgs( pArgs );
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("Y轴"), _T( "撞击" ), _T( "设置Y( 垂直坐标 )轴!" ), PROPERTY_Y );	
	for ( int i = 0; i < tArgNum; i++ )
	{
		pProp->AddOption( pArgs[ i ].c_str() );
	}   
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("X轴"), _T( "通道" ), _T( "设置X( 水平坐标 )轴!" ), PROPERTY_X );	
	for ( int i = 0; i < tArgNum; i++ )
	{
		pProp->AddOption( pArgs[ i ].c_str() );
	}   
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );		

	/*
	pProp = new CMFCPropertyGridProperty(_T("类型"),  _T( "实心柱状图" ), _T( "设置柱状图类型!" ), PROPERTY_FILL_RECT );
	pProp->AddOption( _T( "实心柱状图" ) );
	pProp->AddOption( _T( "空心柱状图" ) );
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );
	*/
	/*
	pProp = new CMFCPropertyGridProperty(_T("跨度"), ( _variant_t )1.0, _T( "单个矩形宽度!"), PROPERTY_SQUARE_LENGTH );
	//pProp->EnableSpinControl( TRUE, 1, 100 );
	pGroup->AddSubItem( pProp );	
	*/
	pProp = new CMFCPropertyGridProperty(_T("数据源"), _T( "" ), _T( "选择数据源" ), PROPERTY_DATA_SOURCE );		
	pProp->AllowEdit( false );	
	
	pGroup->AddSubItem( pProp );

	mPropertyGrid.AddProperty( pGroup );	
}

HSVoid CScatterProperty::PropertyChanged( CMFCPropertyGridProperty * pProperty )
{
	CString tValue = pProperty->GetValue();	

	HSInt tIndex = IndexWithString( tValue, pProperty );	
	if ( tIndex < 0 &&  pProperty->GetData() != PROPERTY_SQUARE_LENGTH )
	{
		return;
	}
	
	if ( pProperty->GetData() == PROPERTY_X )
	{			
		mParent->SetXArg( ( LPCSTR )tValue );		
	}
	else if ( pProperty->GetData() == PROPERTY_Y )
	{		
		mParent->SetYArg( ( LPCSTR )tValue );	
	}
	else if ( pProperty->GetData() == PROPERTY_FILL_RECT )
	{		
		//mParent->SetIsSolidRect( tIndex == 0 );
	}
	else if ( pProperty->GetData() == PROPERTY_SQUARE_LENGTH )
	{		
		//mParent->SetUnitDelta( atof( ( LPCSTR )tValue ) );
	}
	else 
	{
		DEVICE_CH_IDENTIFIER tChannelIdentifier = 0;
		DataHandlerWithListIndex( tIndex, tChannelIdentifier, mDeviceChannelInfo );
		mParent->SetDataIdentifier( tChannelIdentifier );		
	}
}