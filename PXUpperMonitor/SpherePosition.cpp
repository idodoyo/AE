// SpherePosition.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "SpherePosition.h"
#include "afxdialogex.h"
#include "SphereProperty.h"
#include "MainFrm.h"
#include <algorithm>
#include "ArgCalculator.h"
#include "HSLogProtocol.h"
#include "DataExportHelper.h"


// CSpherePosition dialog

CSpherePosition::CSpherePosition(CWnd* pParent /*=NULL*/)
	: CNormalGraphic( pParent )
{
	mMaterialRadius = 1000;	

	mHitRadius = 3;
}

CSpherePosition::~CSpherePosition()
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

//	delete mSphere;
}

HSBool CSpherePosition::Start()
{
	if ( mPositionSensors.size() > 0 )
	{		
		mRefreshThread->Start();
		return HSTrue;
	}

	return HSFalse;
}

HSBool CSpherePosition::Save( CINIConfig *pIniConfig, string tGroup )
{		
	pIniConfig->SetValue( "MaterialRadius", this->mMaterialRadius, tGroup );	
	pIniConfig->SetValue( "HitRadius", this->mHitRadius, tGroup );	
		
	HSChar tBuf[ 1024 ];
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn )
		{
			sprintf_s( tBuf, "Sensor%d_DataIdentifier", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.DataIdentifier.ToString(), tGroup );

			sprintf_s( tBuf, "Sensor%d_AngleD", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.AngleDirction, tGroup );

			sprintf_s( tBuf, "Sensor%d_AngleU", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.AngleUp, tGroup );

			sprintf_s( tBuf, "Sensor%d_Forbid", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Forbid, tGroup );
		}
		pIterator++;
	}	
	
	return HSTrue;
}

HSBool CSpherePosition::Load( CINIConfig *pIniConfig, string tGroup )
{		
	pIniConfig->ValueWithKey( "MaterialRadius", this->mMaterialRadius, tGroup );	
	pIniConfig->ValueWithKey( "HitRadius", this->mHitRadius, tGroup );	
	
	HSString tDataIdentifier = "";
	HSDouble tAngleD = 0;
	HSDouble tAngleU = 0;
	HSBool tForbid = HSFalse;

	HSChar tBufId[ 1024 ];
	HSChar tBufAngleD[ 1024 ];
	HSChar tBufAngleU[ 1024 ];
	HSChar tBufForbid[ 1024 ];

	for ( HSInt i = 0; i < CSphereProperty::POSITION_SENSOR_MAX_NUM; i++ )
	{
		sprintf_s( tBufId, "Sensor%d_DataIdentifier", i );
		sprintf_s( tBufAngleD, "Sensor%d_AngleD", i );
		sprintf_s( tBufAngleU, "Sensor%d_AngleU", i );
		sprintf_s( tBufForbid, "Sensor%d_Forbid", i );
		if ( pIniConfig->ValueWithKey( tBufId, tDataIdentifier, tGroup )
			&& pIniConfig->ValueWithKey( tBufAngleD, tAngleD, tGroup )
			&& pIniConfig->ValueWithKey( tBufAngleU, tAngleU, tGroup )
			&& pIniConfig->ValueWithKey( tBufForbid, tForbid, tGroup ) )
		{			
			mPositionSensors[ i ].AngleDirction = tAngleD;
			mPositionSensors[ i ].AngleUp = tAngleU;
			mPositionSensors[ i ].Forbid = tForbid;
			mPositionSensors[ i ].IsOn = HSTrue;
			mPositionSensors[ i ].DataIdentifier.InitWithString( tDataIdentifier );
			mPositionSensors[ i ].ArgTransfer = NULL;
		}
	}	

	return HSTrue;
}
	
IGraphicPropertyProtocol * CSpherePosition::CreatePropertyDlg( CWnd *pParent )
{
	CSphereProperty *pPositionProperty = new CSphereProperty;
	pPositionProperty->Create( IDD_NORMALPROPERTY, pParent );
	return pPositionProperty;
}

HSVoid CSpherePosition::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		this->UpdatePositionSensor( pIterator->first, pIterator->second.AngleDirction, pIterator->second.AngleUp, pIterator->second.Forbid, pIterator->second.IsOn, pIterator->second.DataIdentifier );
		pIterator++;
	}

	this->mHitsPosition.clear();

	this->ResetData( NULL );

	SetEvent( mDataMutex );	
}

HSVoid CSpherePosition::SetHitRadius( HSInt tHitRadius )
{
	mHitRadius = tHitRadius;
	mSphere->SetHitRadius( mHitRadius );
}

HSDouble CSpherePosition::DistanceWithSensorIndex( HSDouble tAngleD, HSDouble tAngleU, HSInt tSensor )
{		
	HSDouble tTmpValue = sin( AtoR( tAngleU ) ) * sin( AtoR( mPositionSensors[ tSensor ].AngleUp ) ) * cos( AtoR( tAngleD - mPositionSensors[ tSensor ].AngleDirction ) ) + cos ( AtoR( tAngleU ) ) * cos ( AtoR( mPositionSensors[ tSensor ].AngleUp ) );
	
	return mMaterialRadius * acos( tTmpValue );
}

HSBool CSpherePosition::IsHitQualified( HSInt64 tDuration, HSDouble tVelocity, HSInt tSensorOne, HSInt tSensorTwo )
{
	HSDouble tDistanceVoice = abs( tDuration / 1000000000.0 * tVelocity );	

	HSDouble tPhyDistance = DistanceWithSensorIndex( mPositionSensors[ tSensorOne ].AngleDirction, mPositionSensors[ tSensorOne ].AngleUp, tSensorTwo );

	return tDistanceVoice < tPhyDistance;
}

bool CSpherePosition::HitSensorComapre( HitCalcInfo &t1, HitCalcInfo &t2 )
{
	return ( *t1.PositionSensors )[ t1.SensorIndex ].ArgList[ t1.ArgIndex ].NBeginTime < ( *t2.PositionSensors )[ t2.SensorIndex ].ArgList[ t2.ArgIndex ].NBeginTime;	
}

bool CSpherePosition::HitSensorDistanceComapre( SensorDistanceInfo &t1, SensorDistanceInfo &t2 )
{
	return t1.Distance < t2.Distance;
}

HSBool CSpherePosition::IsResQualified( HSDouble tAngleD, HSDouble tAngleU, vector< CSpherePosition::HitCalcInfo > &tHitSensors, HSDouble &tEnergy )
{	
	std::sort( tHitSensors.begin(), tHitSensors.end(), HitSensorComapre );	

	vector< SensorDistanceInfo > tPosDistance;
	for ( HSUInt i = 0; i < tHitSensors.size(); i++ )
	{
		SensorDistanceInfo tDistanceInfo = { tHitSensors[ i ].SensorIndex, DistanceWithSensorIndex( tAngleD, tAngleU, tHitSensors[ i ].SensorIndex ) };
		tPosDistance.push_back( tDistanceInfo );

		tEnergy = max( tEnergy, mPositionSensors[ tHitSensors[ i ].SensorIndex ].ArgList[ tHitSensors[ i ].ArgIndex ].Energy );
	}

	std::sort( tPosDistance.begin(), tPosDistance.end(), HitSensorDistanceComapre );

	for ( HSUInt i = 0; i < tHitSensors.size(); i++ )
	{
		if ( tHitSensors[ i ].SensorIndex != tPosDistance[ i ].SensorIndex )
		{
			return HSFalse;
		}
	}
	
	return HSTrue;
}

HSVoid CSpherePosition::GetFunctionValue( HSDouble tX0[ 3 ], HSDouble *tPoints[ 3 ], HSDouble tRes[ 3 ] )
{
	HSDouble *tPoint1 = tPoints[ 0 ];
	HSDouble *tPoint2 = tPoints[ 1 ];
	HSDouble *tPoint3 = tPoints[ 2 ];

	tRes[ 0 ] = sin( tX0[ 1 ] ) * sin( tPoint1[ 1 ] ) * cos( tX0[ 0 ] - tPoint1[ 0 ] ) + cos( tX0[ 1 ] ) * cos( tPoint1[ 1 ] ) - cos( tX0[ 2 ] / mMaterialRadius );
	tRes[ 1 ] = sin( tX0[ 1 ] ) * sin( tPoint2[ 1 ] ) * cos( tX0[ 0 ] - tPoint2[ 0 ] ) + cos( tX0[ 1 ] ) * cos( tPoint2[ 1 ] ) - cos( ( tX0[ 2 ] + tPoint2[ 2 ] ) / mMaterialRadius );
	tRes[ 2 ] = sin( tX0[ 1 ] ) * sin( tPoint3[ 1 ] ) * cos( tX0[ 0 ] - tPoint3[ 0 ] ) + cos( tX0[ 1 ] ) * cos( tPoint3[ 1 ] ) - cos( ( tX0[ 2 ] + tPoint3[ 2 ] ) / mMaterialRadius );
}

HSVoid CSpherePosition::GetFunctionMValue( HSDouble tX0[ 3 ], HSDouble *tPoints[ 3 ], HSDouble tRes[ 3 ][ 3 ] )
{
	HSDouble *tPoint1 = tPoints[ 0 ];
	HSDouble *tPoint2 = tPoints[ 1 ];
	HSDouble *tPoint3 = tPoints[ 2 ];

	tRes[ 0 ][ 0 ] = -sin( tX0[ 1 ] ) * sin( tPoint1[ 1 ] ) * sin( tX0[ 0 ] - tPoint1[ 0 ] );
	tRes[ 1 ][ 0 ] = -sin( tX0[ 1 ] ) * sin( tPoint2[ 1 ] ) * sin( tX0[ 0 ] - tPoint2[ 0 ] );
	tRes[ 2 ][ 0 ] = -sin( tX0[ 1 ] ) * sin( tPoint3[ 1 ] ) * sin( tX0[ 0 ] - tPoint3[ 0 ] );

	tRes[ 0 ][ 1 ] = cos( tX0[ 1 ] ) * sin( tPoint1[ 1 ] ) * cos( tX0[ 0 ] - tPoint1[ 0 ] ) - sin( tX0[ 1 ] ) * cos( tPoint1[ 1 ] );
	tRes[ 1 ][ 1 ] = cos( tX0[ 1 ] ) * sin( tPoint2[ 1 ] ) * cos( tX0[ 0 ] - tPoint2[ 0 ] ) - sin( tX0[ 1 ] ) * cos( tPoint2[ 1 ] );
	tRes[ 2 ][ 1 ] = cos( tX0[ 1 ] ) * sin( tPoint3[ 1 ] ) * cos( tX0[ 0 ] - tPoint3[ 0 ] ) - sin( tX0[ 1 ] ) * cos( tPoint3[ 1 ] );

	tRes[ 0 ][ 2 ] = 1 / mMaterialRadius * sin( tX0[ 2 ] / mMaterialRadius );
	tRes[ 1 ][ 2 ] = 1 / mMaterialRadius * sin( ( tX0[ 2 ] + tPoint2[ 2 ] ) / mMaterialRadius );
	tRes[ 2 ][ 2 ] = 1 / mMaterialRadius * sin( ( tX0[ 2 ] + tPoint2[ 3 ] ) / mMaterialRadius );
}

HSBool CSpherePosition::PositionWithHitSensor( HSDouble tInitX0[ 3 ], vector< CSpherePosition::HitCalcInfo > &tHitSensors, vector< HSInt > &tHitSensorsIndex, vector< CGraphicSphere::HitPosition > &tHitPos )
{
	HSInt tSensor1Index = tHitSensors[ tHitSensorsIndex[ 0 ] ].SensorIndex;
	HSInt tSensor2Index = tHitSensors[ tHitSensorsIndex[ 1 ] ].SensorIndex;
	HSInt tSensor3Index = tHitSensors[ tHitSensorsIndex[ 2 ] ].SensorIndex;

	HSInt tArg1Index = tHitSensors[ tHitSensorsIndex[ 0 ] ].ArgIndex;
	HSInt tArg2Index = tHitSensors[ tHitSensorsIndex[ 1 ] ].ArgIndex;
	HSInt tArg3Index = tHitSensors[ tHitSensorsIndex[ 2 ] ].ArgIndex;

	HSDouble tVelocity = CArgCalculator::SharedInstance()->GetMaterialVelocity();
	HSDouble tDeltaS1 = ( ( HSInt64 )( mPositionSensors[ tSensor2Index ].ArgList[ tArg2Index ].NBeginTime - mPositionSensors[ tSensor1Index ].ArgList[ tArg1Index ].NBeginTime ) ) / 1000000000.0 * tVelocity;
	HSDouble tDeltaS2 = ( ( HSInt64 )( mPositionSensors[ tSensor3Index ].ArgList[ tArg3Index ].NBeginTime - mPositionSensors[ tSensor1Index ].ArgList[ tArg1Index ].NBeginTime ) ) / 1000000000.0 * tVelocity;
	
	HSDouble tPoint1[ 2 ] = { AtoR( mPositionSensors[ tSensor1Index ].AngleDirction ), AtoR( mPositionSensors[ tSensor1Index ].AngleUp ) };
	HSDouble tPoint2[ 3 ] = { AtoR( mPositionSensors[ tSensor2Index ].AngleDirction ), AtoR( mPositionSensors[ tSensor2Index ].AngleUp ), tDeltaS1 };
	HSDouble tPoint3[ 3 ] = { AtoR( mPositionSensors[ tSensor3Index ].AngleDirction ), AtoR( mPositionSensors[ tSensor3Index ].AngleUp ), tDeltaS2 };	

	HSDouble tX0[ 3 ] = { AtoR( tInitX0[ 0 ] ), AtoR( tInitX0[ 1 ] ), AtoR( tInitX0[ 2 ] ) };
	HSDouble tX1[ 3 ] = { 0 };	

	HSDouble *pPoints[ 3 ] = { tPoint1, tPoint2, tPoint3 };
	
	if ( !mNewtonIterator.StartIterator( tX0, pPoints, tX1, this ) )
	{
		return HSFalse;
	}

	CGraphicSphere::HitPosition tPos = { CorrectAngle( RtoA( tX1[ 0 ] ), 360 ), abs( RtoA( tX1[ 1 ] ) ), RGB( 255, 0, 0 ) };
	tHitPos.push_back( tPos );
	
	return HSTrue;
}

HSDouble CSpherePosition::CorrectAngle( HSDouble tAngle, HSDouble tTop )
{
	if ( tAngle < 0 )
	{
		return tAngle - ( ( HSInt )( ( tAngle - tTop ) / tTop ) ) * tTop;
	}
	else if ( tAngle > tTop )
	{
		return tAngle - ( ( HSInt )( tAngle / tTop ) ) * tTop;
	}

	return tAngle;
}

HSBool CSpherePosition::CalcHitPosition( HSInt tSensor, HSUInt tArgIndex )
{	
	vector< HitCalcInfo > tHitSensors;
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();		
	HitCalcInfo tHit = { tSensor, tArgIndex, &mPositionSensors };
	tHitSensors.push_back( tHit );	

	
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.ArgTransfer != NULL && pIterator->second.IsOn && !pIterator->second.Forbid )
		{			
			for ( HSUInt i = 0; i < pIterator->second.ArgList.size(); i++ )
			{
				if ( pIterator->first != tSensor 
					&& IsHitQualified( mPositionSensors[ tSensor ].ArgList[ tArgIndex ].NBeginTime - pIterator->second.ArgList[ i ].NBeginTime, CArgCalculator::SharedInstance()->GetMaterialVelocity(), tSensor, pIterator->first ) )
				{				
					HitCalcInfo tHit = { pIterator->first, i, &mPositionSensors };
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
	
	HSInt tLowIndex = 0;
	HSInt tBigIndex = 2;
	HSInt tThirdSensor = 1;

	vector< HSInt > tHitSensorsIndex;
	tHitSensorsIndex.push_back( tLowIndex );
	tHitSensorsIndex.push_back( tBigIndex );
	tHitSensorsIndex.push_back( tThirdSensor );

	HSDouble tX0[ 3 ] = { 0 };	

	tX0[ 0 ] = ( mPositionSensors[ tHitSensors[ tLowIndex ].SensorIndex ].AngleDirction + mPositionSensors[ tHitSensors[ tBigIndex ].SensorIndex ].AngleDirction + 360.0 );
	tX0[ 0 ] = ( tX0[ 0 ] - ( ( HSInt )( tX0[ 0 ] / 360 ) ) * 360.0 ) / 2.0;
	tX0[ 1 ] = ( mPositionSensors[ tHitSensors[ tLowIndex ].SensorIndex ].AngleUp + mPositionSensors[ tHitSensors[ tBigIndex ].SensorIndex ].AngleUp ) / 2.0;
	tX0[ 2 ] = DistanceWithSensorIndex( tX0[ 0 ], tX0[ 1 ], tHitSensors[ tLowIndex ].SensorIndex );
	/*
	HS_INFO( "Plane Position, LowIndex: %d ( %f, %f ), BigIndex: %d ( %f, %f ), ThirdSensor: %d ( %f, %f )\n InitX0: ( %f, %f, %f )", tLowIndex, mPositionSensors[ tHitSensors[ tLowIndex ].SensorIndex ].AngleDirction, mPositionSensors[ tHitSensors[ tLowIndex ].SensorIndex ].AngleUp, 
																											tBigIndex, mPositionSensors[ tHitSensors[ tBigIndex ].SensorIndex ].AngleDirction, mPositionSensors[ tHitSensors[ tBigIndex ].SensorIndex ].AngleUp, 
																											tThirdSensor, mPositionSensors[ tHitSensors[ tThirdSensor ].SensorIndex ].AngleDirction, mPositionSensors[ tHitSensors[ tThirdSensor ].SensorIndex ].AngleUp,
																											tX0[ 0 ], tX0[ 1 ], tX0[ 2 ] );
																												
	HS_INFO( "Arg Index( %d, %d, %d )", tHitSensors[ tLowIndex ].ArgIndex, tHitSensors[ tBigIndex ].ArgIndex, tHitSensors[ tThirdSensor ].ArgIndex );
		*/															
	vector< CGraphicSphere::HitPosition > tResHitPos;
	if ( !PositionWithHitSensor( tX0, tHitSensors, tHitSensorsIndex, tResHitPos ) )
	{
		return HSFalse;
	}

	HSBool tRes = HSFalse;
	for ( HSUInt i = 0; i < tResHitPos.size(); i++ )
	{		
		HSDouble tEnergy = 0;
		if ( IsResQualified( tResHitPos[ i ].AngleDirection, tResHitPos[ i ].AngleUp, tHitSensors, tEnergy ) )
		{
			tResHitPos[ i ].Color = CGraphicManager::SharedInstance()->ColorWithEnergy( tEnergy );
			mHitsPosition.push_back( tResHitPos[ i ] );
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

HSVoid CSpherePosition::InitPositionSensors()
{		
	HSDouble tAngleDDelta = 360 / 6;
	HSDouble tAngleUpDelta = 180 / 6;
	HSDouble tAngleD = 0;
	HSDouble tAngleU = 0;
	HSInt tIndex = 0;	
	while ( mPositionSensors.size() < DEFALUT_SENSOR_NUM )
	{
		if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
		{
			this->UpdatePositionSensor( tIndex, tAngleD, tAngleU, HSFalse, HSTrue, 0 );		
			tAngleD += tAngleDDelta;
			tAngleU += tAngleUpDelta;
		}
		tIndex++;
	}
	
	RefreshSensors();
	mSphere->Refresh();	
}

HSVoid CSpherePosition::SetFocusSensor( HSInt tIndex )
{
	mSphere->SetFocusdSensor( tIndex );
	mSphere->Refresh();	
}

HSVoid CSpherePosition::UpdatePositionSensor( HSInt tIndex, HSDouble tAngleD, HSDouble tAngleU, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier )
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

	mPositionSensors[ tIndex ].AngleDirction = tAngleD;
	mPositionSensors[ tIndex ].AngleUp = tAngleU;
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

HSVoid CSpherePosition::RefreshSensors()
{
	mSensors.clear();
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			CGraphicSphere::SensorInfo tSensor;
			tSensor.Index = pIterator->first;
			tSensor.AngleDirection = pIterator->second.AngleDirction;
			tSensor.AngleUp = pIterator->second.AngleUp;
			mSensors[ pIterator->first ] = tSensor;
		}
		pIterator++;
	}
}

CSpherePosition::PositionSensor * CSpherePosition::GetPositionSensor( HSInt tIndex )
{
	if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
	{
		return NULL;
	}

	return &( mPositionSensors[ tIndex ] );
}

HSVoid CSpherePosition::FocusWnd()
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	pMainFrame->SetFocusedGraphic( this );
	if ( !mStaticSphere.Focused() )
	{			
		Sleep( 10 );
		mStaticSphere.SetFocused( true );
		mStaticSphere.ResizeWave();
		mStaticSphere.Invalidate();
		mSphere->Invalidate();
	}		
}

HSVoid CSpherePosition::InitDialog()
{
	mPopupMenu.LoadMenu( IDR_POPUP_SPHERE_POSITION_C );

	CRect tRect;
	this->GetClientRect( &tRect );
	mStaticSphere.Create( "", WS_CHILD | WS_VISIBLE, tRect, this );

	mStaticGraphic = &mStaticSphere;

	mSphere = new CGraphicSphere( this );
	mSphere->SetHits( &mHitsPosition );
	mSphere->SetSensor( &mSensors );	

	mSphere->Create( IDD_GRAPHICNORMAL3D, this );
	mSphere->ShowWindow( SW_SHOW );

	mSphere->SetHitRadius( mHitRadius );

	InitPositionSensors();
}

HSVoid CSpherePosition::ViewResized( CRect &tRect )
{
	mStaticSphere.MoveWindow( tRect, true );
	mStaticSphere.SetXValue( mStaticSphere.TotalXValue(), mStaticSphere.BeginXValue() );
	mStaticSphere.SetYValue( mStaticSphere.TotalYValue(), mStaticSphere.BeginYValue() );
	mStaticSphere.ResizeWave();
	mStaticSphere.Invalidate();

	CRect tSphereRect( tRect.left + mStaticSphere.LeftDelta(), tRect.top + mStaticSphere.TopDelta(), tRect.right - mStaticSphere.LeftDelta(), tRect.bottom - mStaticSphere.TopDelta() );
	mSphere->MoveWindow( tSphereRect );
}


HSBool CSpherePosition::GraphicNeedRefresh()
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
	
	mSphere->InDirectRefresh();

	return HSTrue;
}


HSBool CSpherePosition::IsDataExportNeeded()
{
	return HSTrue;
}

HSVoid CSpherePosition::ExportData( CDataExportHelper &tDataExport )
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	HSChar tBuf[ 1024 ];
	CString tStrChannel;

	sprintf_s( tBuf, "直径：%.2f (m)\n", mMaterialRadius / 1000.0 * 2 );
	tDataExport.Write( tBuf ); 

	sprintf_s( tBuf, "\n% 10s\t% 18s\t% 18s\t% 16s\t% 16s\n", "传感器编号", "方位角( 0 - 360 )", "顶点角( 0 - 180 )", "采集卡", "通道" );
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
				pSensorIterator->second.AngleDirction, 
				pSensorIterator->second.AngleUp, 
				( pDataHandler == NULL ? "" : pDataHandler->Name().c_str() ),
				( pDataHandler == NULL ? "" : ( LPCSTR )tStrChannel ) );

			tDataExport.Write( tBuf ); 
		}

		pSensorIterator++;
	}

	sprintf_s( tBuf, "\n% 10s\t% 18s\t% 18s\n", "撞击编号", "方位角( 0 - 360 )", "顶点角( 0 - 180 )" );
	tDataExport.Write( tBuf ); 

	HSInt tIndex = 1;
	list< CGraphicSphere::HitPosition >::iterator pHitIterator = mHitsPosition.begin();
	while ( pHitIterator != mHitsPosition.end() )
	{
		sprintf_s( tBuf, "% 10d\t% 18f\t% 18f\n", tIndex++, pHitIterator->AngleDirection, pHitIterator->AngleUp );
		tDataExport.Write( tBuf ); 

		pHitIterator++;
	}
}

HSVoid CSpherePosition::ResetData( CMainFrame* pMainFrame )
{
	mSphere->Reset();
	mSphere->Refresh();
}

HSVoid CSpherePosition::AutoSetSensors()
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

	if ( tSensorCount < 6 )
	{
		return;
	}

	HSInt tCenterSensorNum = 1000;
	HSInt tIndex = 1;
	while ( tCenterSensorNum > MAX_SENSOR_NUM_IN_CENTER_LINE )
	{
		tIndex++;

		HSInt tTmpNum = tSensorCount - 2 + ( tIndex - 1 ) * tIndex;
		//HSInt tTmpNum = tSensorCount - 2 + ( tIndex - 1 ) * tIndex * 2;
		tCenterSensorNum = tTmpNum / ( 2 * tIndex - 1 );
		tCenterSensorNum += ( tTmpNum % ( 2 * tIndex - 1 ) != 0 ? 1 : 0 );
	}		

	tCenterSensorNum = max( 4, tCenterSensorNum );
	
	HSDouble tUpAngleDelta = 180.0 / ( 2 * tIndex );
	HSDouble tDirectionAngleDelta = 360 / tCenterSensorNum;

	HSDouble tUpAngle = 90;
	HSDouble tDirectionAngle = 0;

	HSInt tTmpCenterSensorNum = tCenterSensorNum;
	HSInt tSensorIndex = 0;

	PositionSensor *pLastOne = NULL;
	PositionSensor *pLastTue = NULL;
	
	HSBool tSwitchedUpAngle = HSFalse;
	
	pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			pIterator->second.AngleDirction = tDirectionAngle;
			pIterator->second.AngleUp = tUpAngle;

			tSensorIndex++;
			tDirectionAngle += tDirectionAngleDelta;
			
			if ( tSensorIndex >= tTmpCenterSensorNum )
			{
				if ( tTmpCenterSensorNum != tCenterSensorNum && !tSwitchedUpAngle )
				{
					tUpAngle = 180.0 - tUpAngle;
					tSwitchedUpAngle = HSTrue;
				}
				else
				{
					tTmpCenterSensorNum--;
					//tTmpCenterSensorNum -= 2;
					tDirectionAngleDelta = 360 / tTmpCenterSensorNum;					
					tUpAngle = ( 180.0 - tUpAngle ) - tUpAngleDelta;
					tSwitchedUpAngle = HSFalse;
				}

				tSensorIndex = 0;				
				tDirectionAngle = tDirectionAngleDelta;
			}

			pLastTue = pLastOne;
			pLastOne = &( pIterator->second );
		}

		pIterator++;
	}

	if ( pLastOne != NULL )
	{
		pLastOne->AngleDirction = 0;
		pLastOne->AngleUp = 0;
	}

	if ( pLastTue != NULL )
	{
		pLastTue->AngleDirction = 0;
		pLastTue->AngleUp = 180.0;
	}

	RefreshSensors();
	mSphere->Refresh();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
	pMainFrame->SetFocusedGraphic( this, HSTrue );	
}

HSVoid CSpherePosition::Graphic3DRefresh( HSBool tNeedInvalidate )
{
	if ( tNeedInvalidate )
	{
		mSphere->Invalidate();
	}
	else
	{
		mSphere->Refresh();
	}
}

HSVoid CSpherePosition::AutoRatate()
{
	mSphere->AutoRatate();
}