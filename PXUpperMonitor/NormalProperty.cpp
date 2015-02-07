// NormalProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "NormalProperty.h"
#include "afxdialogex.h"
#include "HSLogProtocol.h"
#include "MainFrm.h"


// CNormalProperty dialog

IMPLEMENT_DYNAMIC(CNormalProperty, CDialogEx)

CNormalProperty::CNormalProperty(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNormalProperty::IDD, pParent)
{
	mIsInit = false;
	mTotalChannelEnabled = HSFalse;
	mForPosition = HSFalse;
	mMultiDataSourceConditionEnabled = HSFalse;
}

CNormalProperty::~CNormalProperty()
{
}

void CNormalProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID_PROP, mPropertyGrid);
}


BEGIN_MESSAGE_MAP(CNormalProperty, CDialogEx)
	ON_STN_CLICKED(IDC_MFCPROPERTYGRID_PROP, &CNormalProperty::OnStnClickedMfcpropertygridProp)
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
END_MESSAGE_MAP()


// CNormalProperty message handlers


void CNormalProperty::OnStnClickedMfcpropertygridProp()
{
	// TODO: Add your control notification handler code here

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );		
	
	CMFCPropertyGridProperty* pProp = mPropertyGrid.HitTest( tPoint, NULL, HSTrue );
	if ( mGraphic != NULL )
	{
		HSInt tIndex = -1;
		if ( pProp && pProp->GetData() > 1000 )
		{
			tIndex = pProp->GetData() / 1000 - 1;
		}

		SensorClicked( tIndex );
	}	
}


void CNormalProperty::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( !mIsInit )
	{
		return;
	}

	CRect tRect;
	this->GetClientRect( &tRect );
	mPropertyGrid.MoveWindow( tRect );
}


BOOL CNormalProperty::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	mPropertyGrid.EnableHeaderCtrl(FALSE);
	mPropertyGrid.EnableDescriptionArea();
	mPropertyGrid.SetVSDotNetLook();
	mPropertyGrid.MarkModifiedProperties();		

	InitDialog();

	mIsInit = true;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CNormalProperty::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{		
	if ( mGraphic == NULL )
	{
		return 0;
	}

	CMFCPropertyGridProperty * pProperty = ( CMFCPropertyGridProperty * ) lparam;
	PropertyChanged( pProperty );

	return 1;
}

void CNormalProperty::SetParent( IGraphicProtocol *pGraphic )
{	
	mGraphic = pGraphic;
	mPropertyGrid.EnableWindow( pGraphic != NULL );	
}

HSVoid CNormalProperty::RefreshDeviceChannel()
{
	mDeviceChannelInfo.clear();
	
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	HSUInt tIndex = 0;
	IDataHandler *pDataHandler = NULL;	

	while ( ( pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIndex( tIndex++ ) ) )
	{
		DEVICE_CH_IDENTIFIER tType = pDataHandler->Identifier();
		if ( mTotalChannelEnabled )
		{
			mDeviceChannelInfo[ tType ].push_back( 1016 );
		}

		for ( HSInt i = 0; i < pDataHandler->ChannelNum(); i++ )
		{
			mDeviceChannelInfo[ tType ].push_back( i );
		}		
	}
}

HSVoid CNormalProperty::SetDataSourceForItem( CMFCPropertyGridProperty* pProp, HSVoid *pInfo )
{	
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
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
			
			if ( mForPosition )
			{
				DEVICE_CH_IDENTIFIER tChannelIdentifier = pIterator->first;
				tChannelIdentifier.InitChannel( pIterator->second[ i ] );
				if ( IsDataHandlerMachedWith( tChannelIdentifier, pInfo ) && !tGotDataSource )
				{
					pProp->SetValue( tStr );
					tGotDataSource = true;
				}
			}
			else if ( !mMultiDataSourceConditionEnabled )
			{
				if ( IsDataHandlerMachedWith( pDataHandler, pIterator->second[ i ] ) && !tGotDataSource )
				{
					pProp->SetValue( tStr );
					tGotDataSource = true;
				}
			}
			else if ( IsDataHandlerMachedWith( pDataHandler, pIterator->second[ i ], pInfo ) && !tGotDataSource )
			{
				pProp->SetValue( tStr );
				tGotDataSource = true;
			}
		}
		pIterator++;
	}		

	if ( !tGotDataSource )
	{
		pProp->SetValue( "" );
	}

	pProp->Redraw();
}

