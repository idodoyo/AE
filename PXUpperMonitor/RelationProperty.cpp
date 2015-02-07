// RelationProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "RelationProperty.h"
#include "afxdialogex.h"
#include "GraphicRelation.h"
#include "MainFrm.h"
#include "ArgDataFectcher.h"

// CRelationProperty dialog

IMPLEMENT_DYNAMIC(CRelationProperty, CDialogEx)

CRelationProperty::CRelationProperty(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRelationProperty::IDD, pParent)
{
	mParent = NULL;
}

CRelationProperty::~CRelationProperty()
{
}

void CRelationProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID_RELATION, mGridRelation);
}


BEGIN_MESSAGE_MAP(CRelationProperty, CDialogEx)
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
END_MESSAGE_MAP()


// CRelationProperty message handlers

void CRelationProperty::SetParent( IGraphicProtocol *pGraphic )
{
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CGraphicRelation * >( pGraphic );
		mGridRelation.EnableWindow( true );
	}
	else
	{
		mParent = NULL;
		mGridRelation.EnableWindow( false );
	}	
}


void CRelationProperty::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if ( !mIsInit )
	{
		return;
	}

	CRect tRect;
	this->GetClientRect( &tRect );
	mGridRelation.MoveWindow( tRect );
}

LRESULT CRelationProperty::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{
	if ( mParent == NULL )
	{
		return 0;
	}

	CMFCPropertyGridProperty * pProperty = ( CMFCPropertyGridProperty * ) lparam;	
	CString tValue = pProperty->GetValue();	

	HSInt tIndex = IndexWithString( tValue, pProperty );	
	if ( tIndex < 0 &&  pProperty->GetData() != PROPERTY_SQUARE_LENGTH )
	{
		return 0;
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
		mParent->SetIsSolidRect( tIndex == 0 );
	}
	else if ( pProperty->GetData() == PROPERTY_SQUARE_LENGTH )
	{		
		mParent->SetUnitDelta( atof( ( LPCSTR )tValue ) );
	}
	else 
	{
		DEVICE_CH_IDENTIFIER tChannelIdentifier = 0;
		DataHandlerWithListIndex( tIndex, tChannelIdentifier, mDeviceChannelInfo );
		mParent->SetDataIdentifier( tChannelIdentifier );		
	}

	return 1;
}

BOOL CRelationProperty::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	mGridRelation.EnableHeaderCtrl(FALSE);
	mGridRelation.EnableDescriptionArea();
	mGridRelation.SetVSDotNetLook();
	mGridRelation.MarkModifiedProperties();	

	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("柱状图属性"));	
	
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
	

	pProp = new CMFCPropertyGridProperty(_T("类型"),  _T( "实心柱状图" ), _T( "设置柱状图类型!" ), PROPERTY_FILL_RECT );
	pProp->AddOption( _T( "实心柱状图" ) );
	pProp->AddOption( _T( "空心柱状图" ) );
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );
	/*
	pProp = new CMFCPropertyGridProperty(_T("跨度"), ( _variant_t )1.0, _T( "单个矩形宽度!"), PROPERTY_SQUARE_LENGTH );
	//pProp->EnableSpinControl( TRUE, 1, 100 );
	pGroup->AddSubItem( pProp );	
	*/
	pProp = new CMFCPropertyGridProperty(_T("数据源"), _T( "" ), _T( "选择数据源" ), PROPERTY_DATA_SOURCE );		
	pProp->AllowEdit( false );	
	
	pGroup->AddSubItem( pProp );

	mGridRelation.AddProperty( pGroup );	

	mIsInit = true;


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRelationProperty::RefreshDataSource()
{
	if ( mParent == NULL )
	{
		return;
	}

	mDeviceChannelInfo.clear();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	HSUInt tIndex = 0;
	IDataHandler *pDataHandler = NULL;
	while ( ( pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIndex( tIndex++ ) ) )
	{
		DEVICE_CH_IDENTIFIER tType = pDataHandler->Identifier();
		mDeviceChannelInfo[ tType ].push_back( 1016 );
		for ( HSInt i = 0; i < pDataHandler->ChannelNum(); i++ )
		{
			mDeviceChannelInfo[ tType ].push_back( i );
		}		
	}

	CMFCPropertyGridProperty* pProp = mGridRelation.FindItemByData( PROPERTY_DATA_SOURCE );
	pProp->RemoveAllOptions();

	bool tGotDataSource = false;
	map< DEVICE_CH_IDENTIFIER, vector< HSInt > >::iterator pIterator = mDeviceChannelInfo.begin();
	while ( pIterator != mDeviceChannelInfo.end() )
	{
		IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( pIterator->first );
		for ( HSUInt i = 0; i < pIterator->second.size(); i++ )
		{
			CString tStr;
			if ( pIterator->second[ i ] == 1016 )
			{
				tStr.Format( "%s - 所有通道", pDataHandler->Name().c_str() );
			}
			else
			{
				tStr.Format( "%s - 通道%d", pDataHandler->Name().c_str(), pIterator->second[ i ] + 1 );
			}
			pProp->AddOption( tStr );
			
			if ( mParent != NULL && pDataHandler == mParent->DataHandler() && !tGotDataSource )
			{
				if ( mParent->DataIdentifier().NumOfChannels() == pDataHandler->ChannelNum() || mParent->DataIdentifier().ChannelIndex() == pIterator->second[ i ] )
				{
					pProp->SetValue( tStr );
					tGotDataSource = true;
				}				
			}			 
		}
		pIterator++;
	}

	if ( !tGotDataSource )
	{
		pProp->SetValue( "" );
	}
	pProp->Redraw();
	
	pProp = mGridRelation.FindItemByData( PROPERTY_X );	
	if ( mParent != NULL )
	{		
		pProp->SetValue( mParent->XArg()->ArgName().c_str() );			
	}
	
	pProp->Redraw();
	
	
	pProp = mGridRelation.FindItemByData( PROPERTY_Y );	
	if ( mParent != NULL )
	{
		pProp->SetValue( mParent->YArg()->ArgName().c_str() );			
	}

	pProp->Redraw();

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

	/*
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