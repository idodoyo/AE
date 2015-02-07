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

IMPLEMENT_DYNAMIC(CSpherePosition, CDialogEx)

CSpherePosition::CSpherePosition(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSpherePosition::IDD, pParent)
{
	mInit = HSFalse;
	mRelationMutex = CreateEvent( NULL, FALSE, TRUE, NULL );	

	mUpdateRelationThread = new CThreadControlEx< CSpherePosition >( this, 800, 0 );

	mIsFullScreen = HSFalse;
	
	mIsDraging = HSFalse;
	mIsZoomIn = HSFalse;

	mDragingSensorIndex = -1;	

	mMaterialRadius = 1000;	

	mHitRadius = 3;
}

CSpherePosition::~CSpherePosition()
{
	CloseHandle( mRelationMutex );

	delete mUpdateRelationThread;

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
		mUpdateRelationThread->Start();
		return HSTrue;
	}

	return HSFalse;
}

HSVoid CSpherePosition::Pause()
{
	mUpdateRelationThread->Pause();
}

HSVoid CSpherePosition::Stop()
{
	mUpdateRelationThread->Stop();
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

HSVoid CSpherePosition::FocusGraphic( HSBool tIsFocused )
{
	if ( tIsFocused && !mStaticSphere.Focused() )
	{
		mStaticSphere.SetFocused( true );
		mStaticSphere.ResizeWave();
		mStaticSphere.Invalidate();		
		mSphere->Invalidate();
	}
	else if ( !tIsFocused && mStaticSphere.Focused() )
	{
		mStaticSphere.SetFocused( false );
		mStaticSphere.ResizeWave();
		mStaticSphere.Invalidate();		
		mSphere->Invalidate();
	}	
}


	
IGraphicPropertyProtocol * CSpherePosition::CreatePropertyDlg( CWnd *pParent )
{
	CSphereProperty *pPositionProperty = new CSphereProperty;
	pPositionProperty->Create( IDD_SPHEREPROPERTY, pParent );
	return pPositionProperty;
}

HSVoid CSpherePosition::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mRelationMutex, INFINITE );

	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		this->UpdatePositionSensor( pIterator->first, pIterator->second.AngleDirction, pIterator->second.AngleUp, pIterator->second.Forbid, pIterator->second.IsOn, pIterator->second.DataIdentifier );
		pIterator++;
	}

	this->mHitsPosition.clear();

	this->OnSpherePositionPopupReset();

	SetEvent( mRelationMutex );	
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

HSBool CSpherePosition::ThreadRuning( HSInt tThreadID )
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

HSVoid CSpherePosition::ThreadWillStop( HSInt tThreadID )
{
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



void CSpherePosition::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_SPHERE, mStaticSphere);
}


BEGIN_MESSAGE_MAP(CSpherePosition, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_INITMENUPOPUP()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_SPHERE_POSITION_POPUP_AUTO, &CSpherePosition::OnSpherePositionPopupAuto)
	ON_COMMAND(ID_SPHERE_POSITION_POPUP_CLOSE, &CSpherePosition::OnSpherePositionPopupClose)
	ON_COMMAND(ID_SPHERE_POSITION_POPUP_FULL_SCREEN, &CSpherePosition::OnSpherePositionPopupFullScreen)
	ON_UPDATE_COMMAND_UI(ID_SPHERE_POSITION_POPUP_FULL_SCREEN, &CSpherePosition::OnUpdateSpherePositionPopupFullScreen)
	ON_COMMAND(ID_SPHERE_POSITION_POPUP_RESET, &CSpherePosition::OnSpherePositionPopupReset)
	ON_COMMAND(ID_SPHEREE_POSITION_POPUP_SETDATASOURCE, &CSpherePosition::OnSphereePositionPopupSetdatasource)
	ON_COMMAND(ID_SPHEREE_POSITION_POPUP_ROTATE, &CSpherePosition::OnSphereePositionPopupRotate)
	ON_COMMAND(ID_SPHEREE_POSITION_POPUP_EXPORT_DATA, &CSpherePosition::OnSphereePositionPopupExportData)
END_MESSAGE_MAP()


// CSpherePosition message handlers


void CSpherePosition::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	mUpdateRelationThread->Stop();
}


void CSpherePosition::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialogEx::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// TODO: Add your message handler code here

	ASSERT(pPopupMenu != NULL);     
	// Check the enabled state of various menu items.     

	CCmdUI state;     
	state.m_pMenu = pPopupMenu;     
	ASSERT(state.m_pOther == NULL);     
	ASSERT(state.m_pParentMenu == NULL);     

	// Determine if menu is popup in top-level menu and set m_pOther to     
	// it if so (m_pParentMenu == NULL indicates that it is secondary popup).     
	HMENU hParentMenu;     
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)     
		state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.     
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)     
	{     
		CWnd* pParent = this;     
		// Child windows don't have menus--need to go to the top!     
		if (pParent != NULL &&     
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)     
		{     
			int nIndexMax = ::GetMenuItemCount(hParentMenu);     
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)     
			{     
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)     
				{     
					// When popup is found, m_pParentMenu is containing menu.     
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);     
					break;     
				}     
			}     
		}     
	}     

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();     
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;     
		state.m_nIndex++)     
	{     
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);     
		if (state.m_nID == 0)     
			continue; // Menu separator or invalid cmd - ignore it.     

		ASSERT(state.m_pOther == NULL);     
		ASSERT(state.m_pMenu != NULL);     
		if (state.m_nID == (UINT)-1)     
		{     
			// Possibly a popup menu, route to first item of that popup.     
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);     
			if (state.m_pSubMenu == NULL ||     
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||     
				state.m_nID == (UINT)-1)     
			{     
				continue;       // First item of popup can't be routed to.     
			}     
			state.DoUpdate(this, TRUE);   // Popups are never auto disabled.     
		}     
		else    
		{     
			// Normal menu item.     
			// Auto enable/disable if frame window has m_bAutoMenuEnable     
			// set and command is _not_ a system command.     
			state.m_pSubMenu = NULL;     
			state.DoUpdate(this, FALSE);     
		}     

		// Adjust for menu deletions and additions.     
		UINT nCount = pPopupMenu->GetMenuItemCount();     
		if (nCount < state.m_nIndexMax)     
		{     
			state.m_nIndex -= (state.m_nIndexMax - nCount);     
			while (state.m_nIndex < nCount &&     
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)     
			{     
				state.m_nIndex++;     
			}     
		}     
		state.m_nIndexMax = nCount;     
	} 
}


void CSpherePosition::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	mParent->EndMoveSubView( nFlags, point);

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CSpherePosition::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	CPoint tPoint = point;
	
	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticSphere.LeftDelta() && tPoint.x < tRect.right - mStaticSphere.LeftDelta() && tPoint.y > tRect.top + mStaticSphere.TopDelta() && tPoint.y < tRect.bottom - mStaticSphere.TopDelta() )
	{		
	}	
	else
	{	
		mParent->BeginMoveSubView( this, nFlags, point );
	}

	CDialogEx::OnLButtonDown(nFlags, point);
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

void CSpherePosition::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	
	mParent->MoveingSubView( nFlags, point);

	CDialogEx::OnMouseMove(nFlags, point);
}


void CSpherePosition::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( mInit )
	{
		CRect tRect;
		this->GetClientRect( &tRect );
		if ( tRect.Width() > mStaticSphere.LeftDelta() * 3 && tRect.Height() > mStaticSphere.TopDelta() * 3 )
		{
			mStaticSphere.MoveWindow( tRect, true );
			mStaticSphere.SetXValue( mStaticSphere.TotalXValue(), mStaticSphere.BeginXValue() );
			mStaticSphere.SetYValue( mStaticSphere.TotalYValue(), mStaticSphere.BeginYValue() );
			mStaticSphere.ResizeWave();
			mStaticSphere.Invalidate();
						
			CRect tSphereRect( tRect.left + mStaticSphere.LeftDelta(), tRect.top + mStaticSphere.TopDelta(), tRect.right - mStaticSphere.LeftDelta(), tRect.bottom - mStaticSphere.TopDelta() );
			mSphere->MoveWindow( tSphereRect );
		}
	}
}


void CSpherePosition::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: Add your message handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->SetGrahpicCheckType( CMainFrame::GRAPHIC_CHECK_POINTER );
	
	mPopupMenu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this );
}


BOOL CSpherePosition::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	mInit = HSTrue;

	mPopupMenu.LoadMenu( IDR_POPUP_SPHERE_POSITION_C );		

	mSphere = new CGraphicSphere( this );
	mSphere->SetHits( &mHitsPosition );
	mSphere->SetSensor( &mSensors );	

	mSphere->Create( IDD_GRAPHICSPHERE, this );
	mSphere->ShowWindow( SW_SHOW );

	mSphere->SetHitRadius( mHitRadius );

	InitPositionSensors();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CSpherePosition::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );		

	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticSphere.LeftDelta() && tPoint.x < tRect.right - mStaticSphere.LeftDelta() && tPoint.y > tRect.top + mStaticSphere.TopDelta() && tPoint.y < tRect.bottom - mStaticSphere.TopDelta() )
	{		
		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
		if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_STEP_FRAME )
		{
			if ( tPoint.x < tRect.right / 2 )
			{
				SetCursor( LoadCursor( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDC_CURSOR_PREV_FRAME ) ) );
			}
			else
			{
				SetCursor( LoadCursor( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDC_CURSOR_NEXT_FRAME ) ) );
			}

			return true;
		}
		else if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_RECTANGE )
		{
			SetCursor( AfxGetApp()->LoadStandardCursor( IDC_CROSS ) );
			return true;
		}
		else if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_DRAG )
		{
			SetCursor( AfxGetApp()->LoadStandardCursor( IDC_HAND ) );
			return true;
		}
	}	

	return __super::OnSetCursor(pWnd, nHitTest, message);
}


void CSpherePosition::OnSpherePositionPopupAuto()
{
	// TODO: Add your command handler code here

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


void CSpherePosition::OnSpherePositionPopupClose()
{
	// TODO: Add your command handler code here
	CDialogEx::OnOK();

	mParent->SubViewClosed( this );
}


void CSpherePosition::OnSpherePositionPopupFullScreen()
{
	// TODO: Add your command handler code here
	mIsFullScreen = !mIsFullScreen;
	mParent->SubViewFullScreen( this, mIsFullScreen );

	mSphere->Refresh();
}


void CSpherePosition::OnUpdateSpherePositionPopupFullScreen(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( mIsFullScreen );

	mSphere->Refresh();
}


void CSpherePosition::OnSpherePositionPopupReset()
{
	// TODO: Add your command handler code here
	mSphere->Reset();
	mSphere->Refresh();
}


void CSpherePosition::OnSphereePositionPopupSetdatasource()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->OnViewGraphic();	
	pMainFrame->SetFocusedGraphic( this );

	if ( !mStaticSphere.Focused() || pMainFrame->FocusedGraphic() != this )
	{		
		mStaticSphere.SetFocused( true );
		mStaticSphere.ResizeWave();
		mStaticSphere.Invalidate();
		mSphere->Invalidate();
	}
}


void CSpherePosition::OnSphereePositionPopupRotate()
{
	// TODO: Add your command handler code here
	mSphere->AutoRatate();
}


void CSpherePosition::OnSphereePositionPopupExportData()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	HSChar tBuf[ 1024 ];
	CString tStrChannel;

	CDataExportHelper tDataExport( this );
	if ( tDataExport.GetFilePath() )
	{
		tDataExport.Write( "球面定位\n" );
		sprintf_s( tBuf, "日期: %s %s\n", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
		tDataExport.Write( tBuf ); 

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

		tDataExport.Finish();
	}	
}
