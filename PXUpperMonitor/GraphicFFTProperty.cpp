// GraphicFFTProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GraphicFFTProperty.h"
#include "afxdialogex.h"
#include "GraphicFFT.h"
#include "HSLogProtocol.h"
#include "MainFrm.h"


// CGraphicFFTProperty dialog

IMPLEMENT_DYNAMIC(CGraphicFFTProperty, CDialogEx)

CGraphicFFTProperty::CGraphicFFTProperty(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGraphicFFTProperty::IDD, pParent)
{
	mIsInit = false;
	mParent = NULL;
}

CGraphicFFTProperty::~CGraphicFFTProperty()
{
}

void CGraphicFFTProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID_FFT, mGridFFT);
}


BEGIN_MESSAGE_MAP(CGraphicFFTProperty, CDialogEx)
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
END_MESSAGE_MAP()


// CGraphicFFTProperty message handlers

void CGraphicFFTProperty::SetParent( IGraphicProtocol *pGraphic )
{
	if ( pGraphic != NULL )
	{
		mParent = dynamic_cast< CGraphicFFT * >( pGraphic );
		mGridFFT.EnableWindow( true );
	}
	else
	{
		mParent = NULL;
		mGridFFT.EnableWindow( false );
	}	
}


BOOL CGraphicFFTProperty::OnInitDialog()
{
	__super::OnInitDialog();

	// TODO:  Add extra initialization here

	mGridFFT.EnableHeaderCtrl(FALSE);
	mGridFFT.EnableDescriptionArea();
	mGridFFT.SetVSDotNetLook();
	mGridFFT.MarkModifiedProperties();		

	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("FFT属性"));	
	/*
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("幅度(10dB)"), ( _variant_t )5, _T( "设置总幅度(垂直坐标)!" ), PROPERTY_AMPLITUDE );
	pProp->EnableSpinControl( TRUE, 1, 100 );	
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("开始幅度(10dB)"), ( _variant_t )( -10 ), _T( "设置开始幅度!" ), PROPERTY_BEGIN_AMPLITUDE );
	pProp->EnableSpinControl( TRUE, -100, 100 );	
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );
	*/
	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("数据源"), _T( "" ), _T( "选择数据源" ), PROPERTY_DATA_SOURCE );		
	pProp->AllowEdit( false );	
	pGroup->AddSubItem( pProp );
	
	mGridFFT.AddProperty( pGroup );	

	mIsInit = true;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CGraphicFFTProperty::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( !mIsInit )
	{
		return;
	}

	CRect tRect;
	this->GetClientRect( &tRect );
	mGridFFT.MoveWindow( tRect );
}

LRESULT CGraphicFFTProperty::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{
	if ( mParent == NULL )
	{
		return 0;
	}
	
	CMFCPropertyGridProperty * pProperty = ( CMFCPropertyGridProperty * ) lparam;		
	
	if ( pProperty->GetData() == PROPERTY_AMPLITUDE )
	{		
		CString tValue = pProperty->GetValue();			
		mParent->SetTotalAmplitude( atof( ( LPCSTR )tValue ) * 10 );
	}
	else if ( pProperty->GetData() == PROPERTY_BEGIN_AMPLITUDE )
	{		
		CString tValue = pProperty->GetValue();			
		mParent->SetBeginAmplitude( atof( ( LPCSTR )tValue ) * 10 );
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
	}
	
	return 1;
}

void CGraphicFFTProperty::RefreshDataSource()
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
		for ( HSInt i = 0; i < pDataHandler->ChannelNum(); i++ )
		{
			mDeviceChannelInfo[ tType ].push_back( i );
		}		
	}

	CMFCPropertyGridProperty* pProp = mGridFFT.FindItemByData( PROPERTY_DATA_SOURCE );
	pProp->RemoveAllOptions();

	bool tGotDataSource = false;
	map< DEVICE_CH_IDENTIFIER, vector< HSInt > >::iterator pIterator = mDeviceChannelInfo.begin();
	while ( pIterator != mDeviceChannelInfo.end() )
	{
		IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( pIterator->first );
		for ( HSUInt i = 0; i < pIterator->second.size(); i++ )
		{
			CString tStr;
			tStr.Format( "%s - 通道%d", pDataHandler->Name().c_str(), pIterator->second[ i ] + 1 );
			pProp->AddOption( tStr );
			
			if ( mParent != NULL && pDataHandler == mParent->DataHandler() && mParent->DataIdentifier().ChannelIndex() == i )
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
	/*
	pProp = mGridFFT.FindItemByData( PROPERTY_AMPLITUDE );	
	if ( mParent != NULL )
	{
		int tAmplitude = int( mParent->TotalAmplitude() / 10 );		

		pProp->SetValue( ( _variant_t )tAmplitude );

		COleVariant vtVar( ( long )tAmplitude );
		pProp->SetValue( vtVar );		
	}

	pProp->Redraw();

	pProp = mGridFFT.FindItemByData( PROPERTY_BEGIN_AMPLITUDE );	
	if ( mParent != NULL )
	{
		int tAmplitude = int( mParent->BeginAmplitude() / 10 );		
		pProp->SetValue( ( _variant_t )tAmplitude );	

		COleVariant vtVar( ( long )tAmplitude );
		pProp->SetValue( vtVar );
	}

	pProp->Redraw();
	*/
}

