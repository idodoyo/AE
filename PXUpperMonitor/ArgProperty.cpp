// ArgProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "ArgProperty.h"
#include "afxdialogex.h"
#include "ArgTable.h"
#include "MainFrm.h"
#include "HSLogProtocol.h"

// CArgProperty dialog

IMPLEMENT_DYNAMIC(CArgProperty, CDialogEx)

CArgProperty::CArgProperty(CWnd* pParent /*=NULL*/)
	: CDialogEx(CArgProperty::IDD, pParent)
{
	mIsInit = false;
	mParent = NULL;
}

CArgProperty::~CArgProperty()
{
}

void CArgProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID_PROPERTY, mArgProperty);
}


BEGIN_MESSAGE_MAP(CArgProperty, CDialogEx)
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
END_MESSAGE_MAP()


// CArgProperty message handlers
void CArgProperty::SetParent( IGraphicProtocol *pGraphic )
{
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CArgTable * >( pGraphic );	
		mArgProperty.EnableWindow( true );
	}
	else
	{
		mParent = NULL;		
		mArgProperty.EnableWindow( false );
	}		
}

void CArgProperty::RefreshDataSource()
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

	CMFCPropertyGridProperty* pProp = mArgProperty.FindItemByData( PROPERTY_DATA_SOURCE );
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

	pProp = mArgProperty.FindItemByData( PROPERTY_BYPASS_VOLTAGE );	
	if ( mParent != NULL && mParent->DataHandler() != NULL )
	{		
		pProp->SetValue( GetStrVoltage( mParent->DataHandler()->BypassVoltage( mParent->DataIdentifier().ChannelIndex() ) ) );		
	}
	else
	{
		pProp->SetValue( "0 v" );
	}

	pProp->Redraw();
}

LRESULT CArgProperty::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{
	if ( mParent == NULL )
	{
		return 0;
	}

	CMFCPropertyGridProperty * pProperty = ( CMFCPropertyGridProperty * ) lparam;	
	if ( pProperty->GetData() == PROPERTY_BYPASS_VOLTAGE )
	{				
	}	
	else 
	{	
		HSInt tIndex = IndexWithString( pProperty->GetValue(), pProperty );	
		if ( tIndex < 0 )
		{
			return 0;
		}

		DEVICE_CH_IDENTIFIER tChannelIdentifier = 0;
		DataHandlerWithListIndex( tIndex, tChannelIdentifier, mDeviceChannelInfo );
		mParent->SetDataIdentifier( tChannelIdentifier );

		CMFCPropertyGridProperty* pProp = mArgProperty.FindItemByData( PROPERTY_BYPASS_VOLTAGE );		
		if ( mParent != NULL && mParent->DataHandler() != NULL )
		{		
			pProp->SetValue( GetStrVoltage( mParent->DataHandler()->BypassVoltage( mParent->DataIdentifier().ChannelIndex() ) ) );		
		}
		else
		{
			pProp->SetValue( "0 v" );
		}

		pProp->Redraw();
	}

	return 1;
}


BOOL CArgProperty::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	mIsInit = true;

	mArgProperty.EnableHeaderCtrl(FALSE);
	mArgProperty.EnableDescriptionArea();
	mArgProperty.SetVSDotNetLook();
	mArgProperty.MarkModifiedProperties();

	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("参数表属性"));
	
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("门槛电压"), _T( "0 v" ), _T( "设备门槛电压" ), PROPERTY_BYPASS_VOLTAGE );	
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("数据源"), _T( "" ), _T( "选择数据源" ), PROPERTY_DATA_SOURCE );		
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );

	mArgProperty.AddProperty( pGroup );


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CArgProperty::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if ( !mIsInit )
	{
		return;
	}

	CRect tRect;
	this->GetClientRect( &tRect );
	mArgProperty.MoveWindow( tRect );
}
