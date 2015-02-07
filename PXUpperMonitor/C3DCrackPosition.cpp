// C3DCrackPosition.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "C3DCrackPosition.h"
#include "afxdialogex.h"

#include "C3DCrackPositionProperty.h"
#include "MainFrm.h"
#include "DataExportHelper.h"
#include "HSLogProtocol.h"


// C3DCrackPosition dialog

C3DCrackPosition::C3DCrackPosition(CWnd* pParent /*=NULL*/)
	: CNormalGraphic( pParent )
{
	mAxisXLength = 300;
	mAxisYLength = 100;
	mAxisZLength = 300;

	mHitRadius = 3;
}

C3DCrackPosition::~C3DCrackPosition()
{	
}

HSBool C3DCrackPosition::Start()
{	
	mRefreshThread->Start();	

	return HSTrue;
}

HSBool C3DCrackPosition::Save( CINIConfig *pIniConfig, string tGroup )
{				
	pIniConfig->SetValue( "AxisXLength", this->mAxisXLength, tGroup );
	pIniConfig->SetValue( "AxisYLength", this->mAxisYLength, tGroup );
	pIniConfig->SetValue( "HitRadius", this->mHitRadius, tGroup );		
	
	return HSTrue;
}

HSBool C3DCrackPosition::Load( CINIConfig *pIniConfig, string tGroup )
{			
	pIniConfig->ValueWithKey( "AxisXLength", this->mAxisXLength, tGroup );	
	pIniConfig->ValueWithKey( "AxisYLength", this->mAxisYLength, tGroup );		
	pIniConfig->ValueWithKey( "HitRadius", this->mHitRadius, tGroup );	
	
	return HSTrue;
}
	
IGraphicPropertyProtocol * C3DCrackPosition::CreatePropertyDlg( CWnd *pParent )
{
	C3DCrackPositionProperty *pPositionProperty = new C3DCrackPositionProperty;
	pPositionProperty->Create( IDD_NORMALPROPERTY, pParent );
	return pPositionProperty;
}

HSVoid C3DCrackPosition::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mDataMutex, INFINITE );	

	mHitsPosition.clear();

	this->ResetData( NULL );

	SetEvent( mDataMutex );	
}

HSVoid C3DCrackPosition::SetHitRadius( HSInt tHitRadius )
{
	mHitRadius = tHitRadius;
	mCylinder->SetHitRadius( mHitRadius );
}

HSVoid C3DCrackPosition::SetFocusSensor( HSInt tIndex )
{
	mCylinder->SetFocusdSensor( tIndex );
	mCylinder->Refresh();
}

HSVoid C3DCrackPosition::SetXAxisLength( HSDouble tX )
{
	mAxisXLength = tX;
	mCylinder->SetXYZ( mAxisXLength, mAxisYLength, mAxisZLength );
	mCylinder->Refresh();
}

HSVoid C3DCrackPosition::SetYAxisLength( HSDouble tY )
{
	mAxisZLength = tY;
	mCylinder->SetXYZ( mAxisXLength, mAxisYLength, mAxisZLength );
	mCylinder->Refresh();
}

HSVoid C3DCrackPosition::RefreshSensors()
{
	CCrackPosition *pCrackPosition = RefCrackPosition();	
	if ( pCrackPosition != NULL )
	{
		mAxisXLength = pCrackPosition->XAxisLength();
		mAxisZLength = pCrackPosition->YAxisLength();
		mCylinder->SetMaterialXYZ( pCrackPosition->MaterialWidth() / 1000.0, 25, pCrackPosition->MaterialLength() / 1000.0 );
		mCylinder->SetXYZ( mAxisXLength, mAxisYLength, mAxisZLength );

		mSensors.clear();
		for ( HSInt tSensorIndex = 0; tSensorIndex < 16; tSensorIndex++ )
		{
			CCrackPosition::PositionSensor *pSensorInfo = pCrackPosition->GetPositionSensor( tSensorIndex );
			if ( pSensorInfo != NULL && pSensorInfo->IsOn && !pSensorInfo->Forbid )
			{			
				CGraphicCylinder::SensorInfo tSensor = { pSensorInfo->AxisPosX, 13, pSensorInfo->AxisPosY, tSensorIndex };
				mSensors[ tSensorIndex ] = tSensor;
			}
		}		
	}
}

CCrackPosition * C3DCrackPosition::RefCrackPosition()
{
	IGraphicProtocol *pGraphic = mParent->GraphicWithIdentifier( "GRAPHIC_CRACK_POSITION" );
	if ( pGraphic == NULL )
	{
		return NULL;
	}

	return dynamic_cast< CCrackPosition * >( pGraphic );
}

HSVoid C3DCrackPosition::FocusWnd()
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	pMainFrame->SetFocusedGraphic( this );	

	if ( !mStaticRelation.Focused() )
	{			
		RefreshSensors();

		Sleep( 10 );

		mStaticRelation.SetFocused( true );
		mStaticRelation.ResizeWave();
		mStaticRelation.Invalidate();
		mCylinder->Invalidate();
	}		
}

HSVoid C3DCrackPosition::InitDialog()
{
	mPopupMenu.LoadMenu( IDR_POPUP_3DCRACK_POSITION_C );

	CRect tRect;
	this->GetClientRect( &tRect );
	mStaticRelation.Create( "", WS_CHILD | WS_VISIBLE, tRect, this );

	mStaticGraphic = &mStaticRelation;
	
	mCylinder = new CGraphicCylinder( this, HSTrue );
	mCylinder->SetHits( &mHitsPosition );
	mCylinder->SetSensor( &mSensors );	

	mCylinder->SetXYZ( mAxisXLength, mAxisYLength, mAxisZLength );
	mCylinder->SetMaterialXYZ( 200, 25, 200 );
	
	mCylinder->Create( IDD_GRAPHICNORMAL3D, this );
	mCylinder->ShowWindow( SW_SHOW );

	RefreshSensors();

	mCylinder->SetHitRadius( mHitRadius );	
}

HSVoid C3DCrackPosition::ViewResized( CRect &tRect )
{
	mStaticRelation.MoveWindow( tRect, true );
	mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() );
	mStaticRelation.SetYValue( mStaticRelation.TotalYValue(), mStaticRelation.BeginYValue() );
	mStaticRelation.ResizeWave();
	mStaticRelation.Invalidate();

	CRect tCylinderRect( tRect.left + mStaticRelation.LeftDelta(), tRect.top + mStaticRelation.TopDelta(), tRect.right - mStaticRelation.LeftDelta(), tRect.bottom - mStaticRelation.TopDelta() );
	mCylinder->MoveWindow( tCylinderRect );
}


HSBool C3DCrackPosition::GraphicNeedRefresh()
{
	CCrackPosition *pCrackPosition = RefCrackPosition();	
	if ( pCrackPosition != NULL )
	{
		mHitsPosition.clear();

		list< CStaticCrackPosition::HitPosition > *pHitsPostion = pCrackPosition->HitsPosition();	
		list< CStaticCrackPosition::HitPosition >::iterator pIterator = pHitsPostion->begin();
		while ( pIterator != pHitsPostion->end() )
		{
			CGraphicCylinder::HitPosition tHitPos = { pIterator->XPos, pIterator->ZPos, pIterator->YPos, pIterator->Color };
			mHitsPosition.push_back( tHitPos );
			pIterator++;
		}

		mCylinder->InDirectRefresh();
	}

	return HSTrue;
}


HSVoid C3DCrackPosition::ResetData( CMainFrame* pMainFrame )
{
	mCylinder->Reset();
	mCylinder->Refresh();
}

HSVoid C3DCrackPosition::Graphic3DRefresh( HSBool tNeedInvalidate )
{
	if ( tNeedInvalidate )
	{
		mCylinder->Invalidate();
	}
	else
	{
		mCylinder->Refresh();
	}
}