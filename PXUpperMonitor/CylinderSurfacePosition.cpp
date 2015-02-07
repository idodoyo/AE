// CylinderSurfacePosition.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "CylinderSurfacePosition.h"
#include "afxdialogex.h"
#include "CylinderSurfacePositionProperty.h"
#include <algorithm>
#include "MainFrm.h"
#include "DataExportHelper.h"
#include "HSLogProtocol.h"
#include "ArgCalculator.h"


// CCylinderSurfacePosition dialog

CCylinderSurfacePosition::CCylinderSurfacePosition(CWnd* pParent /*=NULL*/)
	: CNormalGraphic( pParent )
{		
	mMaterialYLength = 1500;
	mMaterialRadius = 500;

	mAxisYLength = 2000;	

	mHitRadius = 3;
}

CCylinderSurfacePosition::~CCylinderSurfacePosition()
{	
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.ArgTransfer != NULL )
		{
			delete pIterator->second.ArgTransfer;					
		}
		pIterator++;
	}
}

HSBool CCylinderSurfacePosition::Start()
{
	if ( mPositionSensors.size() > 0 )
	{		
		mRefreshThread->Start();
		return HSTrue;
	}

	return HSFalse;
}

HSBool CCylinderSurfacePosition::Save( CINIConfig *pIniConfig, string tGroup )
{				
	pIniConfig->SetValue( "MaterialYLength", this->mMaterialYLength, tGroup );
	pIniConfig->SetValue( "MaterialRadius", this->mMaterialRadius, tGroup );	
	
	pIniConfig->SetValue( "AxisYLength", this->mAxisYLength, tGroup );

	pIniConfig->SetValue( "HitRadius", this->mHitRadius, tGroup );	
		
	HSChar tBuf[ 1024 ];
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn )
		{
			sprintf_s( tBuf, "Sensor%d_DataIdentifier", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.DataIdentifier.ToString(), tGroup );			

			sprintf_s( tBuf, "Sensor%d_Angle", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Angle, tGroup );

			sprintf_s( tBuf, "Sensor%d_Height", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Height, tGroup );

			sprintf_s( tBuf, "Sensor%d_Forbid", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Forbid, tGroup );
		}
		pIterator++;
	}	
	
	return HSTrue;
}

HSBool CCylinderSurfacePosition::Load( CINIConfig *pIniConfig, string tGroup )
{		
	pIniConfig->ValueWithKey( "MaterialYLength", this->mMaterialYLength, tGroup );	
	pIniConfig->ValueWithKey( "MaterialRadius", this->mMaterialRadius, tGroup );
	
	pIniConfig->ValueWithKey( "AxisYLength", this->mAxisYLength, tGroup );	

	pIniConfig->ValueWithKey( "HitRadius", this->mHitRadius, tGroup );
	
	HSString tDataIdentifier = "";
	HSDouble tAngle = 0;
	HSDouble tHeight = 0;	
	HSBool tForbid = HSFalse;

	HSChar tBufId[ 1024 ];
	HSChar tBufAngle[ 1024 ];
	HSChar tBufHeight[ 1024 ];	
	HSChar tBufForbid[ 1024 ];

	for ( HSInt i = 0; i < CCylinderSurfacePositionProperty::POSITION_SENSOR_MAX_NUM; i++ )
	{
		sprintf_s( tBufId, "Sensor%d_DataIdentifier", i );
		sprintf_s( tBufAngle, "Sensor%d_Angle", i );
		sprintf_s( tBufHeight, "Sensor%d_Height", i );		
		sprintf_s( tBufForbid, "Sensor%d_Forbid", i );
		if ( pIniConfig->ValueWithKey( tBufId, tDataIdentifier, tGroup )
			&& pIniConfig->ValueWithKey( tBufAngle, tAngle, tGroup )
			&& pIniConfig->ValueWithKey( tBufHeight, tHeight, tGroup )	
			&& pIniConfig->ValueWithKey( tBufForbid, tForbid, tGroup ) )
		{						
			mPositionSensors[ i ].Angle = tAngle;
			mPositionSensors[ i ].Height = tHeight;
			mPositionSensors[ i ].Forbid = tForbid;
			mPositionSensors[ i ].IsOn = HSTrue;
			mPositionSensors[ i ].DataIdentifier.InitWithString( tDataIdentifier );
			mPositionSensors[ i ].ArgTransfer = NULL;
		}
	}	
	
	return HSTrue;
}
	
IGraphicPropertyProtocol * CCylinderSurfacePosition::CreatePropertyDlg( CWnd *pParent )
{
	CCylinderSurfacePositionProperty *pPositionProperty = new CCylinderSurfacePositionProperty;
	pPositionProperty->Create( IDD_NORMALPROPERTY, pParent );
	return pPositionProperty;	
}

HSVoid CCylinderSurfacePosition::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mDataMutex, INFINITE );
	
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		this->UpdatePositionSensor( pIterator->first, pIterator->second.Angle, pIterator->second.Height, pIterator->second.Forbid, pIterator->second.IsOn, pIterator->second.DataIdentifier );
		pIterator++;
	}
	
	this->mHitsPosition.clear();

	this->ResetData( NULL );

	SetEvent( mDataMutex );	
}

HSDouble CCylinderSurfacePosition::DistanceBetweenSensors( HSInt tSensorOne, HSInt tSensorTwo )
{
	HSDouble tX1Angle = mPositionSensors[ tSensorOne ].Angle;
	HSDouble tX2Angle = mPositionSensors[ tSensorTwo ].Angle;
	
	tX2Angle = tX1Angle + min( 360 - abs( tX2Angle - tX1Angle ), abs( tX2Angle - tX1Angle ) );

	HSDouble tX = XPosWithAngle( tX1Angle ) - XPosWithAngle( tX2Angle );
	HSDouble tY = mPositionSensors[ tSensorOne ].Height - mPositionSensors[ tSensorTwo ].Height;
	
	return sqrt( tX * tX + tY * tY );
}

HSDouble CCylinderSurfacePosition::DistanceFromSensor( HSDouble tPosX, HSDouble tPosY, HSInt tSensor )
{
	HSDouble tX1Angle = AngleWithXPos( tPosX );
	HSDouble tX2Angle = mPositionSensors[ tSensor ].Angle;
	
	tX2Angle = tX1Angle + min( 360 - abs( tX2Angle - tX1Angle ), abs( tX2Angle - tX1Angle ) );

	HSDouble tX = tPosX - XPosWithAngle( tX2Angle );
	HSDouble tY = tPosY - mPositionSensors[ tSensor ].Height;
	
	return sqrt( tX * tX + tY * tY );	
}

HSVoid CCylinderSurfacePosition::SetHitRadius( HSInt tHitRadius )
{
	mHitRadius = tHitRadius;
	mCylinder->SetHitRadius( mHitRadius );
}

HSBool CCylinderSurfacePosition::IsHitQualified( HSInt64 tDuration, HSDouble tVelocity, HSInt tSensorOne, HSInt tSensorTwo )
{	
	return abs( tDuration / 1000000000.0 * tVelocity ) < DistanceBetweenSensors( tSensorOne, tSensorTwo );
}

HSBool CCylinderSurfacePosition::IsResQualified( HSDouble tPosX, HSDouble tPosY, vector< CPlanePositionCalulator::HIT_CALC_INFO > &tHitSensors, HSDouble &tEnergy )
{	
	if ( tPosX < 0 || tPosX > XPosWithAngle( 360 ) || tPosY < 0 || tPosY > this->mMaterialYLength )
	{
		return HSFalse;
	}	

	std::sort( tHitSensors.begin(), tHitSensors.end(), CPlanePositionCalulator::HitSensorComapre );	

	vector< CPlanePositionCalulator::SENSOR_DIST_INFO > tPosDistance;
	for ( HSUInt i = 0; i < tHitSensors.size(); i++ )
	{
		CPlanePositionCalulator::SENSOR_DIST_INFO tDistanceInfo = { tHitSensors[ i ].SensorIndex, DistanceFromSensor( tPosX, tPosY, tHitSensors[ i ].SensorIndex ) };
		tPosDistance.push_back( tDistanceInfo );

		tEnergy = max( tEnergy, mPositionSensors[ tHitSensors[ i ].SensorIndex ].ArgList[ tHitSensors[ i ].ArgIndex ].Energy );
	}

	std::sort( tPosDistance.begin(), tPosDistance.end(), CPlanePositionCalulator::HitSensorDistanceComapre );

	for ( HSUInt i = 0; i < tHitSensors.size(); i++ )
	{
		if ( tHitSensors[ i ].SensorIndex != tPosDistance[ i ].SensorIndex )
		{
			return HSFalse;
		}
	}
	
	return HSTrue;
}

HSBool CCylinderSurfacePosition::CalcHitPosition( HSInt tSensor, HSUInt tArgIndex )
{	
	vector< CPlanePositionCalulator::HIT_CALC_INFO > tHitSensors;	
	
	CPlanePositionCalulator::HIT_CALC_INFO tHit = { tSensor, tArgIndex, this };
	tHitSensors.push_back( tHit );

	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();		
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.ArgTransfer != NULL && pIterator->second.IsOn && !pIterator->second.Forbid )
		{			
			for ( HSUInt i = 0; i < pIterator->second.ArgList.size(); i++ )
			{
				if ( pIterator->first != tSensor 
					&& IsHitQualified( mPositionSensors[ tSensor ].ArgList[ tArgIndex ].NBeginTime - pIterator->second.ArgList[ i ].NBeginTime, CArgCalculator::SharedInstance()->GetMaterialVelocity(), tSensor, pIterator->first ) )
				{				
					CPlanePositionCalulator::HIT_CALC_INFO tHit = { pIterator->first, i, this };
					tHitSensors.push_back( tHit );			

					break;
				}
			}
		}
		pIterator++;
	}

	if ( tHitSensors.size() < 3 )
	{
		//HS_INFO( "Not Enough Sensor: %d", tHitSensors.size() );
		mPositionSensors[ tSensor ].ArgHandled[ tArgIndex ] = HSTrue;
		return HSFalse;
	}	

	
	vector< CPlanePositionCalulator::HIT_POS > tResHitPos;

	if ( !mPositionCalulator.CalcHit( tHitSensors, tResHitPos, this ) )
	{
		return HSFalse;
	}

	HSBool tRes = HSFalse;
	for ( HSUInt i = 0; i < tResHitPos.size(); i++ )
	{		
		HSDouble tEnergy = 0;
		if ( IsResQualified( tResHitPos[ i ].PosX, tResHitPos[ i ].PosY, tHitSensors, tEnergy ) )
		{
			CGraphicCylinderSurface::HitPosition tHitPos = { AngleWithXPos( tResHitPos[ i ].PosX ), tResHitPos[ i ].PosY, CGraphicManager::SharedInstance()->ColorWithEnergy( tEnergy ) };
			
			mHitsPosition.push_back( tHitPos );
			tRes = HSTrue;
		}
	}

	if ( !tRes )
	{
		return HSFalse;
	}

	for ( HSUInt i = 0; i < tHitSensors.size(); i++ )
	{
		mPositionSensors[ tHitSensors[ i ].SensorIndex ].ArgHandled[ tHitSensors[ i ].ArgIndex ] = HSTrue;		
	}	
	
	return HSTrue;
}

HSVoid CCylinderSurfacePosition::InitPositionSensors()
{		
	HSInt tIndex = 0;		
	HSInt tAngle = 0;
	
	while ( mPositionSensors.size() < DEFALUT_SENSOR_NUM )
	{
		if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
		{
			this->UpdatePositionSensor( tIndex, tAngle, 500, HSFalse, HSTrue, 0 );		
			tAngle = ( tAngle + 10 ) % 360;
		}

		tIndex++;
	}
	
	RefreshSensors();
	mCylinder->Refresh();	
}

HSVoid CCylinderSurfacePosition::SetFocusSensor( HSInt tIndex )
{
	mCylinder->SetFocusdSensor( tIndex );
	mCylinder->Refresh();	
}

HSVoid CCylinderSurfacePosition::SetMaterialYLength( HSDouble tLength )
{
	mMaterialYLength = tLength;
	mCylinder->SetMaterialHeight( mMaterialYLength / 1000.0 );
	this->RefreshSensors();	
}

HSVoid CCylinderSurfacePosition::SetMaterialRadius( HSDouble tRadius )
{
	mMaterialRadius = tRadius;		
}

HSVoid CCylinderSurfacePosition::SetYAxisLength( HSDouble tY )
{
	mAxisYLength = tY;
	mCylinder->SetHeight(  mAxisYLength / 1000.0 );
	mCylinder->Refresh();	
}

HSVoid CCylinderSurfacePosition::UpdatePositionSensor( HSInt tIndex, HSDouble tAngle, HSDouble tHeight, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier )
{	
	if ( mPositionSensors.find( tIndex ) != mPositionSensors.end() )
	{
		if ( mPositionSensors[ tIndex ].ArgTransfer != NULL )
		{
			delete mPositionSensors[ tIndex ].ArgTransfer;			
			mPositionSensors[ tIndex ].ArgList.clear();		
			mPositionSensors[ tIndex ].ArgHandled.clear();			
		}
	}
	else if ( !tIsOn )
	{
		return;
	}

	mPositionSensors[ tIndex ].Angle = tAngle;
	mPositionSensors[ tIndex ].Height = tHeight;
	mPositionSensors[ tIndex ].Forbid = tForbid;
	mPositionSensors[ tIndex ].IsOn = tIsOn;	
	mPositionSensors[ tIndex ].DataIdentifier = tDataIdentifier;
	mPositionSensors[ tIndex ].ArgTransfer = NULL;	
	mPositionSensors[ tIndex ].Index = 0;

	if ( tIsOn )
	{
		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

		DEVICE_CH_IDENTIFIER tCardIdentifer( tDataIdentifier.CARD_IDENTIFIER, tDataIdentifier.CARD_INDEX );
		IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( tCardIdentifer );
		if ( pDataHandler != NULL )
		{			
			mPositionSensors[ tIndex ].ArgTransfer = new CArgListTransfer( pDataHandler, tDataIdentifier );
		}
	}

	RefreshSensors();
}

HSVoid CCylinderSurfacePosition::RefreshSensors()
{
	mSensors.clear();
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			CGraphicCylinderSurface::SensorInfo tSensor;
			tSensor.Index = pIterator->first;
			tSensor.Angle = pIterator->second.Angle;
			tSensor.Height = pIterator->second.Height;

			mSensors[ pIterator->first ] = tSensor;
		}
		pIterator++;
	}

	mCylinder->Refresh();
}

CCylinderSurfacePosition::PositionSensor * CCylinderSurfacePosition::GetPositionSensor( HSInt tIndex )
{
	if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
	{
		return NULL;
	}

	return &( mPositionSensors[ tIndex ] );
}

HSVoid CCylinderSurfacePosition::FocusWnd()
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	pMainFrame->SetFocusedGraphic( this );
	if ( !mStaticCylinderSurface.Focused() )
	{			
		Sleep( 10 );

		mStaticCylinderSurface.SetFocused( true );
		mStaticCylinderSurface.ResizeWave();
		mStaticCylinderSurface.Invalidate();
		mCylinder->Invalidate();
	}			
}

HSVoid CCylinderSurfacePosition::InitDialog()
{
	mPopupMenu.LoadMenu( IDR_POPUP_CYLINDER_SURFACE_POSITION_C );

	CRect tRect;
	this->GetClientRect( &tRect );
	mStaticCylinderSurface.Create( "", WS_CHILD | WS_VISIBLE, tRect, this );

	mStaticGraphic = &mStaticCylinderSurface;	

	mCylinder = new CGraphicCylinderSurface( this );
	mCylinder->SetHits( &mHitsPosition );
	mCylinder->SetSensor( &mSensors );	

	mCylinder->SetHeight( mAxisYLength / 1000.0 );
	mCylinder->SetMaterialHeight( mMaterialYLength / 1000.0 );

	mCylinder->Create( IDD_GRAPHICNORMAL3D, this );
	mCylinder->ShowWindow( SW_SHOW );

	mCylinder->SetHitRadius( mHitRadius );

	InitPositionSensors();
}

HSVoid CCylinderSurfacePosition::ViewResized( CRect &tRect )
{
	mStaticCylinderSurface.MoveWindow( tRect, true );
	mStaticCylinderSurface.SetXValue( mStaticCylinderSurface.TotalXValue(), mStaticCylinderSurface.BeginXValue() );
	mStaticCylinderSurface.SetYValue( mStaticCylinderSurface.TotalYValue(), mStaticCylinderSurface.BeginYValue() );
	mStaticCylinderSurface.ResizeWave();
	mStaticCylinderSurface.Invalidate();

	CRect tCylinderRect( tRect.left + mStaticCylinderSurface.LeftDelta(), tRect.top + mStaticCylinderSurface.TopDelta(), tRect.right - mStaticCylinderSurface.LeftDelta(), tRect.bottom - mStaticCylinderSurface.TopDelta() );
	mCylinder->MoveWindow( tCylinderRect );					
}


HSBool CCylinderSurfacePosition::GraphicNeedRefresh()
{
	HSDouble tHitDuration = CArgCalculator::SharedInstance()->GetHitFullTime() / 1000000000.0 + 0.0005;

	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.ArgTransfer != NULL && pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			pIterator->second.ArgTransfer->GetArg( pIterator->second.ArgList );	
			for ( HSUInt i = pIterator->second.Index; i < pIterator->second.ArgList.size(); i++ )
			{
				pIterator->second.ArgHandled.push_back( HSFalse );
			}
			pIterator->second.Index = pIterator->second.ArgList.size();
		}
		pIterator++;
	}

	pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.ArgTransfer != NULL && pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			for ( HSUInt i = 0; i < pIterator->second.ArgList.size(); i++ )
			{
				if ( pIterator->second.ArgHandled[ i ] )
				{								
					continue;
				}
				else 
				{
					HSDouble tDuration = ( i > 0 ? ( pIterator->second.ArgList[ i ].NBeginTime - pIterator->second.ArgList[ i - 1 ].NBeginTime ) / 1000000000.0 : 1.0 );
					if ( tDuration < tHitDuration )
					{
						pIterator->second.ArgHandled[ i ] = HSTrue;
						continue;
					}
				}
				
				CalcHitPosition( pIterator->first, i );
			}			
		}
		pIterator++;	
	}	
	
	mCylinder->InDirectRefresh();
	
	return HSTrue;
}


HSBool CCylinderSurfacePosition::IsDataExportNeeded()
{
	return HSTrue;
}

HSVoid CCylinderSurfacePosition::ExportData( CDataExportHelper &tDataExport )
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	HSChar tBuf[ 1024 ];
	CString tStrChannel;

	tDataExport.Write( "柱面定位\n" );
	sprintf_s( tBuf, "日期: %s %s\n", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
	tDataExport.Write( tBuf ); 

	sprintf_s( tBuf, "材料尺寸(半径，高度)：%.2f (m), %.2f (m)\n", this->mMaterialRadius / 1000.0, this->mMaterialYLength / 1000.0 );
	tDataExport.Write( tBuf ); 

	sprintf_s( tBuf, "\n% 10s\t% 18s\t% 18s\t% 16s\t% 16s\n", "传感器编号", "角度", "高度(m)", "采集卡", "通道" );
	tDataExport.Write( tBuf );

	map< HSInt, PositionSensor >::iterator pSensorIterator = mPositionSensors.begin();
	while ( pSensorIterator != mPositionSensors.end() )
	{
		if ( pSensorIterator->second.IsOn && !pSensorIterator->second.Forbid )
		{
			tStrChannel.Format( "%d", pSensorIterator->second.DataIdentifier.ChannelIndex() + 1 );
			IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( pSensorIterator->second.DataIdentifier );
			sprintf_s( tBuf, "% 10d\t% 18f\t% 18f\t% 16s\t% 16s\n", 
				pSensorIterator->first + 1, 
				pSensorIterator->second.Angle, 
				pSensorIterator->second.Height / 1000.0, 					
				( pDataHandler == NULL ? "" : pDataHandler->Name().c_str() ),
				( pDataHandler == NULL ? "" : ( LPCSTR )tStrChannel ) );

			tDataExport.Write( tBuf ); 
		}

		pSensorIterator++;
	}

	sprintf_s( tBuf, "\n% 10s\t% 18s\t% 18s\n", "撞击编号", "角度", "高度(m)" );
	tDataExport.Write( tBuf ); 

	HSInt tIndex = 1;
	list< CGraphicCylinderSurface::HitPosition >::iterator pHitIterator = mHitsPosition.begin();
	while ( pHitIterator != mHitsPosition.end() )
	{
		sprintf_s( tBuf, "% 10d\t% 18f\t% 18f\n", tIndex++, pHitIterator->Angle, pHitIterator->Height / 1000.0 );
		tDataExport.Write( tBuf ); 

		pHitIterator++;
	}
}

HSVoid CCylinderSurfacePosition::ResetData( CMainFrame* pMainFrame )
{
	mCylinder->Reset();
	mCylinder->Refresh();
}

HSVoid CCylinderSurfacePosition::AutoSetSensors()
{
	HSInt tSensorCount = 0;
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			tSensorCount++;
		}
		pIterator++;
	}

	if ( tSensorCount < 3 )
	{
		return;
	}	

	HSInt tRows = 2;	

	HSDouble tAngleDelta = 360.0 / tSensorCount;
	HSDouble tHeightDelta = this->mMaterialYLength / ( tRows - 1 );

	HSInt tIndex = 0;
	HSInt tRowIndex = 0;
	pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			pIterator->second.Angle = tAngleDelta * tIndex;
			pIterator->second.Height = tRowIndex * tHeightDelta;					

			tIndex++;
			tRowIndex++;

			if ( tRowIndex == tRows )
			{
				tRowIndex = 0;
			}
		}

		pIterator++;
	}

	RefreshSensors();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
	pMainFrame->SetFocusedGraphic( this, HSTrue );
}

HSVoid CCylinderSurfacePosition::Graphic3DRefresh( HSBool tNeedInvalidate )
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