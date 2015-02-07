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


HSBool CScatterProperty::IsDataHandlerMachedWith( IDataHandler *pDataHandler, HSUInt tChannel )
{
	return ( pDataHandler == mParent->DataHandler() && ( mParent->DataIdentifier().ChannelIndex() == tChannel || mParent->DataIdentifier().NumOfChannels() == pDataHandler->ChannelNum() ) );
}

HSVoid CScatterProperty::InitDialog()
{
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
	pProp = new CMFCPropertyGridProperty(_T("���"), ( _variant_t )1.0, _T( "�������ο��!"), PROPERTY_SQUARE_LENGTH );
	//pProp->EnableSpinControl( TRUE, 1, 100 );
	pGroup->AddSubItem( pProp );	
	*/
	pProp = new CMFCPropertyGridProperty(_T("����Դ"), _T( "" ), _T( "ѡ������Դ" ), PROPERTY_DATA_SOURCE );		
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