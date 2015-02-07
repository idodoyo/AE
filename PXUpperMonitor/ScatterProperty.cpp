// ScatterProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "ScatterProperty.h"
#include "afxdialogex.h"
#include "GraphicScatter.h"
#include "MainFrm.h"

// CScatterProperty dialog

IMPLEMENT_DYNAMIC(CScatterProperty, CDialogEx)

CScatterProperty::CScatterProperty(CWnd* pParent /*=NULL*/)
	: CDialogEx(CScatterProperty::IDD, pParent)
{
	mParent = NULL;
}

CScatterProperty::~CScatterProperty()
{
}

void CScatterProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID_SCATTER, mPropertyScatter);
}


BEGIN_MESSAGE_MAP(CScatterProperty, CDialogEx)
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
END_MESSAGE_MAP()


// CScatterProperty message handlers


void CScatterProperty::SetParent( IGraphicProtocol *pGraphic )
{
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CGraphicScatter * >( pGraphic );
		mPropertyScatter.EnableWindow( true );
	}
	else
	{
		mParent = NULL;
		mPropertyScatter.EnableWindow( false );
	}	
}


void CScatterProperty::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if ( !mIsInit )
	{
		return;
	}

	CRect tRect;
	this->GetClientRect( &tRect );
	mPropertyScatter.MoveWindow( tRect );
}

LRESULT CScatterProperty::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
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

	return 1;
}

BOOL CScatterProperty::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	mPropertyScatter.EnableHeaderCtrl(FALSE);
	mPropertyScatter.EnableDescriptionArea();
	mPropertyScatter.SetVSDotNetLook();
	mPropertyScatter.MarkModifiedProperties();		

	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("ɢ��ͼ����"));	
	
	HSString *pArgs = NULL;
	HSInt tArgNum = CArgDataFectcher::GetArgs( pArgs );
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("Y��"), _T( "ײ��" ), _T( "����Y( ��ֱ���� )��!" ), PROPERTY_Y );	
	for ( int i = 0; i < tArgNum; i++ )
	{
		pProp->AddOption( pArgs[ i ].c_str() );
	}   
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("X��"), _T( "ͨ��" ), _T( "����X( ˮƽ���� )��!" ), PROPERTY_X );	
	for ( int i = 0; i < tArgNum; i++ )
	{
		pProp->AddOption( pArgs[ i ].c_str() );
	}   
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );		

	/*
	pProp = new CMFCPropertyGridProperty(_T("����"),  _T( "ʵ����״ͼ" ), _T( "������״ͼ����!" ), PROPERTY_FILL_RECT );
	pProp->AddOption( _T( "ʵ����״ͼ" ) );
	pProp->AddOption( _T( "������״ͼ" ) );
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );
	*/
	/*
	pProp = new CMFCPropertyGridProperty(_T("���"), ( _variant_t )1.0, _T( "�������ο���!"), PROPERTY_SQUARE_LENGTH );
	//pProp->EnableSpinControl( TRUE, 1, 100 );
	pGroup->AddSubItem( pProp );	
	*/
	pProp = new CMFCPropertyGridProperty(_T("����Դ"), _T( "" ), _T( "ѡ������Դ" ), PROPERTY_DATA_SOURCE );		
	pProp->AllowEdit( false );	
	
	pGroup->AddSubItem( pProp );

	mPropertyScatter.AddProperty( pGroup );	

	mIsInit = true;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CScatterProperty::RefreshDataSource()
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

	CMFCPropertyGridProperty* pProp = mPropertyScatter.FindItemByData( PROPERTY_DATA_SOURCE );
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
				tStr.Format( "%s - ����ͨ��", pDataHandler->Name().c_str() );
			}
			else
			{
				tStr.Format( "%s - ͨ��%d", pDataHandler->Name().c_str(), pIterator->second[ i ] + 1 );
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
	
	pProp = mPropertyScatter.FindItemByData( PROPERTY_X );	
	if ( mParent != NULL )
	{		
		pProp->SetValue( mParent->XArg()->ArgName().c_str() );			
	}
	
	pProp->Redraw();
	
	
	pProp = mPropertyScatter.FindItemByData( PROPERTY_Y );	
	if ( mParent != NULL )
	{
		pProp->SetValue( mParent->YArg()->ArgName().c_str() );			
	}

	pProp->Redraw();

	/*
	pProp = mGridRelation.FindItemByData( PROPERTY_FILL_RECT );	
	if ( mParent == NULL || mParent->IsSolidRect() )
	{				
		pProp->SetValue( "ʵ����״ͼ" );
	}
	else
	{
		pProp->SetValue( "������״ͼ" );
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