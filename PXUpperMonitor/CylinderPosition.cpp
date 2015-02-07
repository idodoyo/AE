// CylinderPosition.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "CylinderPosition.h"
#include "afxdialogex.h"
#include "CylinderPositionProperty.h"
#include <algorithm>
#include "MainFrm.h"
#include "DataExportHelper.h"
#include "HSLogProtocol.h"
#include "ArgCalculator.h"


// CCylinderPosition dialog

IMPLEMENT_DYNAMIC(CCylinderPosition, CDialogEx)

CCylinderPosition::CCylinderPosition(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCylinderPosition::IDD, pParent)
{
	mInit = HSFalse;
	mRelationMutex = CreateEvent( NULL, FALSE, TRUE, NULL );	

	mUpdateRelationThread = new CThreadControlEx< CCylinderPosition >( this, 800, 0 );

	mIsFullScreen = HSFalse;
	
	mIsDraging = HSFalse;
	mIsZoomIn = HSFalse;

	mDragingSensorIndex = -1;	

	mMaterialXLength = 1000;
	mMaterialYLength = 1000;
	mMaterialZLength = 1000;

	mAxisXLength = 2000;
	mAxisYLength = 2000;
	mAxisZLength = 2000;

	mHitRadius = 3;
}

CCylinderPosition::~CCylinderPosition()
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
}

HSBool CCylinderPosition::Start()
{
	if ( mPositionSensors.size() > 0 )
	{		
		mUpdateRelationThread->Start();
		return HSTrue;
	}

	return HSFalse;
}

HSVoid CCylinderPosition::Pause()
{
	mUpdateRelationThread->Pause();
}

HSVoid CCylinderPosition::Stop()
{
	mUpdateRelationThread->Stop();
}

HSBool CCylinderPosition::Save( CINIConfig *pIniConfig, string tGroup )
{			
	pIniConfig->SetValue( "MaterialXLength", this->mMaterialXLength, tGroup );
	pIniConfig->SetValue( "MaterialYLength", this->mMaterialYLength, tGroup );
	pIniConfig->SetValue( "MaterialZLength", this->mMaterialZLength, tGroup );
	
	pIniConfig->SetValue( "AxisXLength", this->mAxisXLength, tGroup );
	pIniConfig->SetValue( "AxisYLength", this->mAxisYLength, tGroup );
	pIniConfig->SetValue( "AxisZLength", this->mAxisZLength, tGroup );

	pIniConfig->SetValue( "HitRadius", this->mHitRadius, tGroup );	
		
	HSChar tBuf[ 1024 ];
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn )
		{
			sprintf_s( tBuf, "Sensor%d_DataIdentifier", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.DataIdentifier.ToString(), tGroup );

			sprintf_s( tBuf, "Sensor%d_Face", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Face, tGroup );

			sprintf_s( tBuf, "Sensor%d_PosX", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.PosX, tGroup );

			sprintf_s( tBuf, "Sensor%d_PosY", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.PosY, tGroup );

			sprintf_s( tBuf, "Sensor%d_Forbid", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Forbid, tGroup );
		}
		pIterator++;
	}	
	
	return HSTrue;
}

HSBool CCylinderPosition::Load( CINIConfig *pIniConfig, string tGroup )
{			
	pIniConfig->ValueWithKey( "MaterialXLength", this->mMaterialXLength, tGroup );	
	pIniConfig->ValueWithKey( "MaterialYLength", this->mMaterialYLength, tGroup );	
	pIniConfig->ValueWithKey( "MaterialZLength", this->mMaterialZLength, tGroup );	

	pIniConfig->ValueWithKey( "AxisXLength", this->mAxisXLength, tGroup );	
	pIniConfig->ValueWithKey( "AxisYLength", this->mAxisYLength, tGroup );	
	pIniConfig->ValueWithKey( "AxisZLength", this->mAxisZLength, tGroup );	

	pIniConfig->ValueWithKey( "HitRadius", this->mHitRadius, tGroup );
	
	HSString tDataIdentifier = "";
	HSDouble tPosX = 0;
	HSDouble tPosY = 0;
	HSInt tFace = 0;	
	HSBool tForbid = HSFalse;

	HSChar tBufId[ 1024 ];
	HSChar tBufFace[ 1024 ];
	HSChar tBufPosX[ 1024 ];
	HSChar tBufPosY[ 1024 ];
	HSChar tBufForbid[ 1024 ];

	for ( HSInt i = 0; i < CCylinderPositionProperty::POSITION_SENSOR_MAX_NUM; i++ )
	{
		sprintf_s( tBufId, "Sensor%d_DataIdentifier", i );
		sprintf_s( tBufFace, "Sensor%d_Face", i );
		sprintf_s( tBufPosX, "Sensor%d_PosX", i );
		sprintf_s( tBufPosY, "Sensor%d_PosY", i );
		sprintf_s( tBufForbid, "Sensor%d_Forbid", i );
		if ( pIniConfig->ValueWithKey( tBufId, tDataIdentifier, tGroup )
			&& pIniConfig->ValueWithKey( tBufFace, tFace, tGroup )
			&& pIniConfig->ValueWithKey( tBufPosX, tPosX, tGroup )
			&& pIniConfig->ValueWithKey( tBufPosY, tPosY, tGroup )
			&& pIniConfig->ValueWithKey( tBufForbid, tForbid, tGroup ) )
		{			
			mPositionSensors[ i ].Face = tFace;
			mPositionSensors[ i ].PosX = tPosX;
			mPositionSensors[ i ].PosY = tPosY;
			mPositionSensors[ i ].Forbid = tForbid;
			mPositionSensors[ i ].IsOn = HSTrue;
			mPositionSensors[ i ].DataIdentifier.InitWithString( tDataIdentifier );
			mPositionSensors[ i ].ArgTransfer = NULL;
		}
	}	
	
	return HSTrue;
}
	
HSVoid CCylinderPosition::FocusGraphic( HSBool tIsFocused )
{
	if ( tIsFocused && !mStaticCylinder.Focused() )
	{
		mStaticCylinder.SetFocused( true );
		mStaticCylinder.ResizeWave();
		mStaticCylinder.Invalidate();
		mCylinder->Invalidate();
	}
	else if ( !tIsFocused && mStaticCylinder.Focused() )
	{
		mStaticCylinder.SetFocused( false );
		mStaticCylinder.ResizeWave();
		mStaticCylinder.Invalidate();
		mCylinder->Invalidate();
	}	
}

	
IGraphicPropertyProtocol * CCylinderPosition::CreatePropertyDlg( CWnd *pParent )
{
	CCylinderPositionProperty *pPositionProperty = new CCylinderPositionProperty;
	pPositionProperty->Create( IDD_CYLINDERPOSITIONPROPERTY, pParent );
	return pPositionProperty;
}

HSVoid CCylinderPosition::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mRelationMutex, INFINITE );

	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		this->UpdatePositionSensor( pIterator->first, pIterator->second.Face, pIterator->second.PosX, pIterator->second.PosY, pIterator->second.Forbid, pIterator->second.IsOn, pIterator->second.DataIdentifier );
		pIterator++;
	}

	this->mHitsPosition.clear();

	this->OnCylinderPositionPopupReset();

	SetEvent( mRelationMutex );	
}

HSVoid CCylinderPosition::SetHitRadius( HSInt tHitRadius )
{
	mHitRadius = tHitRadius;
	mCylinder->SetHitRadius( mHitRadius );
}

HSDouble CCylinderPosition::DistanceWithSensorIndex( HSDouble tPosX, HSDouble tPosY, HSDouble tPosZ, HSInt tSensor )
{		
	CGraphicCylinder::SensorInfo tSensorInfo;
	CoordWithFaceInfo( &( mPositionSensors[ tSensor ] ), tSensorInfo );	

	return sqrt( tPosX * tSensorInfo.PosX + tPosY * tSensorInfo.PosY + tPosZ * tSensorInfo.PosZ );
}

HSBool CCylinderPosition::IsHitQualified( HSInt64 tDuration, HSDouble tVelocity, HSInt tSensorOne, HSInt tSensorTwo )
{
	HSDouble tDistanceVoice = abs( tDuration / 1000000000.0 * tVelocity );	

	CGraphicCylinder::SensorInfo tSensorInfo;
	CoordWithFaceInfo( &( mPositionSensors[ tSensorOne ] ), tSensorInfo );	
	HSDouble tPhyDistance = DistanceWithSensorIndex( tSensorInfo.PosX, tSensorInfo.PosY, tSensorInfo.PosZ, tSensorTwo );

	return tDistanceVoice < tPhyDistance;	
}

bool CCylinderPosition::HitSensorComapre( HitCalcInfo &t1, HitCalcInfo &t2 )
{
	return ( *t1.PositionSensors )[ t1.SensorIndex ].ArgList[ t1.ArgIndex ].NBeginTime < ( *t2.PositionSensors )[ t2.SensorIndex ].ArgList[ t2.ArgIndex ].NBeginTime;	
}

bool CCylinderPosition::HitSensorDistanceComapre( SensorDistanceInfo &t1, SensorDistanceInfo &t2 )
{
	return t1.Distance < t2.Distance;
}

HSBool CCylinderPosition::IsResQualified( HSDouble tPosX, HSDouble tPosY, HSDouble tPosZ, vector< CCylinderPosition::HitCalcInfo > &tHitSensors, HSDouble &tEnergy )
{	
	if ( tPosX < 0 || tPosX > this->mMaterialXLength || tPosY < 0 || tPosY > this->mMaterialYLength || tPosZ < 0 || tPosZ > this->mMaterialZLength )
	{
		return HSFalse;
	}	

	std::sort( tHitSensors.begin(), tHitSensors.end(), HitSensorComapre );	

	vector< SensorDistanceInfo > tPosDistance;
	for ( HSUInt i = 0; i < tHitSensors.size(); i++ )
	{
		SensorDistanceInfo tDistanceInfo = { tHitSensors[ i ].SensorIndex, DistanceWithSensorIndex( tPosX, tPosY, tPosZ, tHitSensors[ i ].SensorIndex ) };
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

HSVoid CCylinderPosition::GetFunctionValue( HSDouble tX0[ 4 ], HSDouble *tPoints[ 4 ], HSDouble tRes[ 4 ] )
{
	HSDouble *tPoint1 = tPoints[ 0 ];
	HSDouble *tPoint2 = tPoints[ 1 ];
	HSDouble *tPoint3 = tPoints[ 2 ];
	HSDouble *tPoint4 = tPoints[ 3 ];
	
	tRes[ 0 ] = pow( tX0[ 0 ] - tPoint1[ 0 ], 2 ) + pow( tX0[ 1 ] - tPoint1[ 1 ], 2 ) + pow( tX0[ 2 ] - tPoint1[ 2 ], 2 ) - pow( tX0[ 3 ], 2 );
	tRes[ 1 ] = pow( tX0[ 0 ] - tPoint2[ 0 ], 2 ) + pow( tX0[ 1 ] - tPoint2[ 1 ], 2 ) + pow( tX0[ 2 ] - tPoint2[ 2 ], 2 ) - pow( tX0[ 3 ] + tPoint2[ 3 ], 2 );
	tRes[ 2 ] = pow( tX0[ 0 ] - tPoint3[ 0 ], 2 ) + pow( tX0[ 1 ] - tPoint3[ 1 ], 2 ) + pow( tX0[ 2 ] - tPoint3[ 2 ], 2 ) - pow( tX0[ 3 ] + tPoint3[ 3 ], 2 );
	tRes[ 3 ] = pow( tX0[ 0 ] - tPoint4[ 0 ], 2 ) + pow( tX0[ 1 ] - tPoint4[ 1 ], 2 ) + pow( tX0[ 2 ] - tPoint4[ 2 ], 2 ) - pow( tX0[ 3 ] + tPoint4[ 3 ], 2 );
}

HSVoid CCylinderPosition::GetFunctionMValue( HSDouble tX0[ 4 ], HSDouble *tPoints[ 4 ], HSDouble tRes[ 4 ][ 4 ] )
{
	HSDouble *tPoint1 = tPoints[ 0 ];
	HSDouble *tPoint2 = tPoints[ 1 ];
	HSDouble *tPoint3 = tPoints[ 2 ];
	HSDouble *tPoint4 = tPoints[ 3 ];

	tRes[ 0 ][ 0 ] = ( tX0[ 0 ] - tPoint1[ 0 ] ) * 2;
	tRes[ 1 ][ 0 ] = ( tX0[ 0 ] - tPoint2[ 0 ] ) * 2;
	tRes[ 2 ][ 0 ] = ( tX0[ 0 ] - tPoint3[ 0 ] ) * 2;
	tRes[ 3 ][ 0 ] = ( tX0[ 0 ] - tPoint4[ 0 ] ) * 2;

	tRes[ 0 ][ 1 ] = ( tX0[ 1 ] - tPoint1[ 1 ] ) * 2;
	tRes[ 1 ][ 1 ] = ( tX0[ 1 ] - tPoint2[ 1 ] ) * 2;
	tRes[ 2 ][ 1 ] = ( tX0[ 1 ] - tPoint3[ 1 ] ) * 2;
	tRes[ 3 ][ 1 ] = ( tX0[ 1 ] - tPoint4[ 1 ] ) * 2;

	tRes[ 0 ][ 2 ] = ( tX0[ 2 ] - tPoint1[ 2 ] ) * 2;
	tRes[ 1 ][ 2 ] = ( tX0[ 2 ] - tPoint2[ 2 ] ) * 2;
	tRes[ 2 ][ 2 ] = ( tX0[ 2 ] - tPoint3[ 2 ] ) * 2;
	tRes[ 3 ][ 2 ] = ( tX0[ 2 ] - tPoint4[ 2 ] ) * 2;

	tRes[ 0 ][ 3 ] = -tX0[ 3 ] * 2;
	tRes[ 1 ][ 3 ] = -( tX0[ 3 ] + tPoint2[ 3 ] ) * 2;
	tRes[ 2 ][ 3 ] = -( tX0[ 3 ] + tPoint3[ 3 ] ) * 2;
	tRes[ 3 ][ 3 ] = -( tX0[ 3 ] + tPoint4[ 3 ] ) * 2;
}

HSBool CCylinderPosition::PositionWithHitSensor( HSDouble tInitX0[ 4 ], vector< CCylinderPosition::HitCalcInfo > &tHitSensors, vector< HSInt > &tHitSensorsIndex, vector< CGraphicCylinder::HitPosition > &tHitPos )
{
	HSInt tSensor1Index = tHitSensors[ tHitSensorsIndex[ 0 ] ].SensorIndex;
	HSInt tSensor2Index = tHitSensors[ tHitSensorsIndex[ 1 ] ].SensorIndex;
	HSInt tSensor3Index = tHitSensors[ tHitSensorsIndex[ 2 ] ].SensorIndex;
	HSInt tSensor4Index = tHitSensors[ tHitSensorsIndex[ 3 ] ].SensorIndex;

	HSInt tArg1Index = tHitSensors[ tHitSensorsIndex[ 0 ] ].ArgIndex;
	HSInt tArg2Index = tHitSensors[ tHitSensorsIndex[ 1 ] ].ArgIndex;
	HSInt tArg3Index = tHitSensors[ tHitSensorsIndex[ 2 ] ].ArgIndex;
	HSInt tArg4Index = tHitSensors[ tHitSensorsIndex[ 3 ] ].ArgIndex;

	HSDouble tVelocity = CArgCalculator::SharedInstance()->GetMaterialVelocity();
	HSDouble tDeltaS1 = ( ( HSInt64 )( mPositionSensors[ tSensor2Index ].ArgList[ tArg2Index ].NBeginTime - mPositionSensors[ tSensor1Index ].ArgList[ tArg1Index ].NBeginTime ) ) / 1000000000.0 * tVelocity;
	HSDouble tDeltaS2 = ( ( HSInt64 )( mPositionSensors[ tSensor3Index ].ArgList[ tArg3Index ].NBeginTime - mPositionSensors[ tSensor1Index ].ArgList[ tArg1Index ].NBeginTime ) ) / 1000000000.0 * tVelocity;
	HSDouble tDeltaS3 = ( ( HSInt64 )( mPositionSensors[ tSensor4Index ].ArgList[ tArg4Index ].NBeginTime - mPositionSensors[ tSensor1Index ].ArgList[ tArg1Index ].NBeginTime ) ) / 1000000000.0 * tVelocity;
	
	CGraphicCylinder::SensorInfo tSensorInfo;
	CoordWithFaceInfo( &( mPositionSensors[ tSensor1Index ] ), tSensorInfo );	
	HSDouble tPoint1[ 3 ] = { tSensorInfo.PosX, tSensorInfo.PosY, tSensorInfo.PosZ };

	CoordWithFaceInfo( &( mPositionSensors[ tSensor2Index ] ), tSensorInfo );	
	HSDouble tPoint2[ 4 ] = { tSensorInfo.PosX, tSensorInfo.PosY, tSensorInfo.PosZ, tDeltaS1 };

	CoordWithFaceInfo( &( mPositionSensors[ tSensor3Index ] ), tSensorInfo );	
	HSDouble tPoint3[ 4 ] = { tSensorInfo.PosX, tSensorInfo.PosY, tSensorInfo.PosZ, tDeltaS2 };	

	CoordWithFaceInfo( &( mPositionSensors[ tSensor4Index ] ), tSensorInfo );	
	HSDouble tPoint4[ 4 ] = { tSensorInfo.PosX, tSensorInfo.PosY, tSensorInfo.PosZ, tDeltaS3 };	

	HSDouble tX0[ 4 ] = { tInitX0[ 0 ], tInitX0[ 1 ], tInitX0[ 2 ], tInitX0[ 3 ] };
	HSDouble tX1[ 4 ] = { 0 };
	
	HSDouble *pPoints[ 4 ] = { tPoint1, tPoint2, tPoint3, tPoint4 };

	if ( !mNewtonIterator.StartIterator( tX0, pPoints, tX1, this ) )
	{
		return HSFalse;
	}

	CGraphicCylinder::HitPosition tPos = { tX1[ 0 ], tX1[ 1 ], tX1[ 2 ], RGB( 255, 0, 0 ) };
	tHitPos.push_back( tPos );
	
	return HSTrue;
}

HSBool CCylinderPosition::CalcHitPosition( HSInt tSensor, HSUInt tArgIndex )
{	
	vector< HitCalcInfo > tHitSensors;
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();		
	HitCalcInfo tHit = { tSensor, tArgIndex, &mPositionSensors };
	tHitSensors.push_back( tHit );

	map< HSInt, HSInt > tSensorFaces;
	
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
					tSensorFaces[ pIterator->second.Face ] = 0;

					break;
				}
			}
		}
		pIterator++;
	}

	if ( tHitSensors.size() < 4 )
	{
		//HS_INFO( "Not Enough Sensor: %d", tHitSensors.size() );
		mPositionSensors[ tSensor ].ArgHandled[ tArgIndex ] = HSTrue;
		return HSFalse;
	}	
	
	HSInt tLowIndex = 0;
	HSInt tBigIndex = 3;
	HSInt tThirdSensor = 1;
	HSInt tFourthSensor = 2;

	vector< HSInt > tHitSensorsIndex;
	tHitSensorsIndex.push_back( tLowIndex );
	tHitSensorsIndex.push_back( tBigIndex );
	tHitSensorsIndex.push_back( tThirdSensor );
	tHitSensorsIndex.push_back( tFourthSensor );

	HSDouble tX0[ 4 ] = { 0 };	

	CGraphicCylinder::SensorInfo tSensorLowInfo;
	CoordWithFaceInfo( &( mPositionSensors[ tHitSensors[ tLowIndex ].SensorIndex ] ), tSensorLowInfo );	

	CGraphicCylinder::SensorInfo tSensorBigInfo;
	CoordWithFaceInfo( &( mPositionSensors[ tHitSensors[ tBigIndex ].SensorIndex ] ), tSensorBigInfo );	
	
	tX0[ 0 ] = ( tSensorLowInfo.PosX + tSensorBigInfo.PosX ) / 2.0;
	tX0[ 1 ] = ( tSensorLowInfo.PosY + tSensorBigInfo.PosY ) / 2.0;
	tX0[ 2 ] = ( tSensorLowInfo.PosZ + tSensorBigInfo.PosZ ) / 2.0;
	tX0[ 3 ] = DistanceWithSensorIndex( tX0[ 0 ], tX0[ 1 ], tX0[ 2 ], tHitSensors[ tLowIndex ].SensorIndex );
	/*
	HS_INFO( "Plane Position, LowIndex: %d ( %f, %f ), BigIndex: %d ( %f, %f ), ThirdSensor: %d ( %f, %f )\n InitX0: ( %f, %f, %f )", tLowIndex, mPositionSensors[ tHitSensors[ tLowIndex ].SensorIndex ].AngleDirction, mPositionSensors[ tHitSensors[ tLowIndex ].SensorIndex ].AngleUp, 
																											tBigIndex, mPositionSensors[ tHitSensors[ tBigIndex ].SensorIndex ].AngleDirction, mPositionSensors[ tHitSensors[ tBigIndex ].SensorIndex ].AngleUp, 
																											tThirdSensor, mPositionSensors[ tHitSensors[ tThirdSensor ].SensorIndex ].AngleDirction, mPositionSensors[ tHitSensors[ tThirdSensor ].SensorIndex ].AngleUp,
																											tX0[ 0 ], tX0[ 1 ], tX0[ 2 ] );
																												
	HS_INFO( "Arg Index( %d, %d, %d )", tHitSensors[ tLowIndex ].ArgIndex, tHitSensors[ tBigIndex ].ArgIndex, tHitSensors[ tThirdSensor ].ArgIndex );
		*/															
	vector< CGraphicCylinder::HitPosition > tResHitPos;
	if ( !PositionWithHitSensor( tX0, tHitSensors, tHitSensorsIndex, tResHitPos ) )
	{
		return HSFalse;
	}

	HSBool tRes = HSFalse;
	for ( HSUInt i = 0; i < tResHitPos.size(); i++ )
	{		
		HSDouble tEnergy = 0;
		if ( IsResQualified( tResHitPos[ i ].PosX, tResHitPos[ i ].PosY, tResHitPos[ i ].PosZ, tHitSensors, tEnergy ) )
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

HSBool CCylinderPosition::ThreadRuning( HSInt tThreadID )
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

HSVoid CCylinderPosition::ThreadWillStop( HSInt tThreadID )
{
}

HSVoid CCylinderPosition::InitPositionSensors()
{		
	HSInt tIndex = 0;	
	HSInt tFace = 0;
	
	while ( mPositionSensors.size() < DEFALUT_SENSOR_NUM )
	{
		if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
		{
			this->UpdatePositionSensor( tIndex, tFace, 500, 500, HSFalse, HSTrue, 0 );		
			tFace = ( tFace + 1 ) % 6;
		}
		tIndex++;
	}
	
	RefreshSensors();
	mCylinder->Refresh();	
}

HSVoid CCylinderPosition::SetFocusSensor( HSInt tIndex )
{
	mCylinder->SetFocusdSensor( tIndex );
	mCylinder->Refresh();	
}

HSVoid CCylinderPosition::SetMaterialXLength( HSDouble tLength )
{
	mMaterialXLength = tLength;
	mCylinder->SetMaterialXYZ( mMaterialXLength / 1000.0, mMaterialYLength / 1000.0, mMaterialZLength / 1000.0 );
	this->RefreshSensors();
}

HSVoid CCylinderPosition::SetMaterialYLength( HSDouble tLength )
{
	mMaterialYLength = tLength;
	mCylinder->SetMaterialXYZ( mMaterialXLength / 1000.0, mMaterialYLength / 1000.0, mMaterialZLength / 1000.0 );
	this->RefreshSensors();	
}

HSVoid CCylinderPosition::SetMaterialZLength( HSDouble tLength )
{
	mMaterialZLength = tLength;
	mCylinder->SetMaterialXYZ( mMaterialXLength / 1000.0, mMaterialYLength / 1000.0, mMaterialZLength / 1000.0 );
	this->RefreshSensors();
}

HSVoid CCylinderPosition::SetXAxisLength( HSDouble tX )
{
	mAxisXLength = tX;
	mCylinder->SetXYZ( mAxisXLength / 1000.0, mAxisYLength / 1000.0, mAxisZLength / 1000.0 );
	mCylinder->Refresh();	
}

HSVoid CCylinderPosition::SetYAxisLength( HSDouble tY )
{
	mAxisYLength = tY;
	mCylinder->SetXYZ( mAxisXLength / 1000.0, mAxisYLength / 1000.0, mAxisZLength / 1000.0 );
	mCylinder->Refresh();	
}

HSVoid CCylinderPosition::SetZAxisLength( HSDouble tZ )
{
	mAxisZLength = tZ;
	mCylinder->SetXYZ( mAxisXLength / 1000.0, mAxisYLength / 1000.0, mAxisZLength / 1000.0 );
	mCylinder->Refresh();	
}


HSString CCylinderPosition::FaceWithIndex( HSInt tFace, HSString &tFirstAxisName, HSString &tSecondAxisName )
{
	static HSString sCylinderFaces[ 6 ] = { "前", "后", "左", "右", "上", "下" };

	if ( tFace < 2 )
	{
		tFirstAxisName = "X坐标";
		tSecondAxisName = "Y坐标";
	}
	else if ( tFace < 4 )
	{
		tFirstAxisName = "Z坐标";
		tSecondAxisName = "Y坐标";
	}
	else
	{
		tFirstAxisName = "X坐标";
		tSecondAxisName = "Z坐标";
	}

	return sCylinderFaces[ tFace ];
}

HSVoid CCylinderPosition::UpdatePositionSensor( HSInt tIndex, HSInt tFace, HSDouble tFirstPos, HSDouble tSecondPos, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier )
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

	mPositionSensors[ tIndex ].PosX = tFirstPos;
	mPositionSensors[ tIndex ].PosY = tSecondPos;
	mPositionSensors[ tIndex ].Face = tFace;
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

HSVoid CCylinderPosition::CoordWithFaceInfo( PositionSensor *pSensor, CGraphicCylinder::SensorInfo &tGraphicSensor )
{
	HSInt tFace = pSensor->Face;
	if ( tFace < 2 )
	{
		tGraphicSensor.PosX = pSensor->PosX;
		tGraphicSensor.PosY = pSensor->PosY;
		tGraphicSensor.PosZ = ( tFace == 0 ? 0 : this->mMaterialZLength );		
	}
	else if ( tFace < 4 )
	{
		tGraphicSensor.PosZ = pSensor->PosX;
		tGraphicSensor.PosY = pSensor->PosY;
		tGraphicSensor.PosX = ( tFace == 2 ? 0 : this->mMaterialXLength );
	}
	else
	{
		tGraphicSensor.PosX = pSensor->PosX;
		tGraphicSensor.PosZ = pSensor->PosY;
		tGraphicSensor.PosY = ( tFace == 5 ? 0 : this->mMaterialYLength );			
	}
}

HSVoid CCylinderPosition::RefreshSensors()
{
	mSensors.clear();
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			CGraphicCylinder::SensorInfo tSensor;
			tSensor.Index = pIterator->first;
			CoordWithFaceInfo( &( pIterator->second ), tSensor );
			tSensor.PosX /= 1000.0;
			tSensor.PosY /= 1000.0;
			tSensor.PosZ /= 1000.0;

			mSensors[ pIterator->first ] = tSensor;
		}
		pIterator++;
	}

	mCylinder->Refresh();
}

CCylinderPosition::PositionSensor * CCylinderPosition::GetPositionSensor( HSInt tIndex )
{
	if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
	{
		return NULL;
	}

	return &( mPositionSensors[ tIndex ] );
}

void CCylinderPosition::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CYLINDER, mStaticCylinder);
}


BEGIN_MESSAGE_MAP(CCylinderPosition, CDialogEx)
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	ON_WM_INITMENUPOPUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_CYLINDER_POSITION_POPUP_AUTO, &CCylinderPosition::OnCylinderPositionPopupAuto)
	ON_COMMAND(ID_CYLINDER_POSITION_POPUP_CLOSE, &CCylinderPosition::OnCylinderPositionPopupClose)
	ON_COMMAND(ID_CYLINDER_POSITION_POPUP_EXPORT_DATA, &CCylinderPosition::OnCylinderPositionPopupExportData)
	ON_COMMAND(ID_CYLINDER_POSITION_POPUP_FULL_SCREEN, &CCylinderPosition::OnCylinderPositionPopupFullScreen)
	ON_UPDATE_COMMAND_UI(ID_CYLINDER_POSITION_POPUP_FULL_SCREEN, &CCylinderPosition::OnUpdateCylinderPositionPopupFullScreen)
	ON_COMMAND(ID_CYLINDER_POSITION_POPUP_RESET, &CCylinderPosition::OnCylinderPositionPopupReset)
	ON_COMMAND(ID_CYLINDER_POSITION_POPUP_SETDATASOURCE, &CCylinderPosition::OnCylinderPositionPopupSetdatasource)
END_MESSAGE_MAP()


// CCylinderPosition message handlers


void CCylinderPosition::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: Add your message handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->SetGrahpicCheckType( CMainFrame::GRAPHIC_CHECK_POINTER );
	
	mPopupMenu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this );
}


void CCylinderPosition::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	mUpdateRelationThread->Stop();
}


void CCylinderPosition::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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


void CCylinderPosition::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	CPoint tPoint = point;
	
	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticCylinder.LeftDelta() && tPoint.x < tRect.right - mStaticCylinder.LeftDelta() && tPoint.y > tRect.top + mStaticCylinder.TopDelta() && tPoint.y < tRect.bottom - mStaticCylinder.TopDelta() )
	{		
	}	
	else
	{	
		mParent->BeginMoveSubView( this, nFlags, point );
	}


	CDialogEx::OnLButtonDown(nFlags, point);
}


void CCylinderPosition::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	mParent->EndMoveSubView( nFlags, point);

	CDialogEx::OnLButtonUp(nFlags, point);
}

HSVoid CCylinderPosition::FocusWnd()
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	pMainFrame->SetFocusedGraphic( this );
	if ( !mStaticCylinder.Focused() )
	{			
		Sleep( 10 );

		mStaticCylinder.SetFocused( true );
		mStaticCylinder.ResizeWave();
		mStaticCylinder.Invalidate();
		mCylinder->Invalidate();
	}		
}

void CCylinderPosition::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	mParent->MoveingSubView( nFlags, point);

	CDialogEx::OnMouseMove(nFlags, point);
}


void CCylinderPosition::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( mInit )
	{
		CRect tRect;
		this->GetClientRect( &tRect );
		if ( tRect.Width() > mStaticCylinder.LeftDelta() * 3 && tRect.Height() > mStaticCylinder.TopDelta() * 3 )
		{
			mStaticCylinder.MoveWindow( tRect, true );
			mStaticCylinder.SetXValue( mStaticCylinder.TotalXValue(), mStaticCylinder.BeginXValue() );
			mStaticCylinder.SetYValue( mStaticCylinder.TotalYValue(), mStaticCylinder.BeginYValue() );
			mStaticCylinder.ResizeWave();
			mStaticCylinder.Invalidate();
						
			CRect tCylinderRect( tRect.left + mStaticCylinder.LeftDelta(), tRect.top + mStaticCylinder.TopDelta(), tRect.right - mStaticCylinder.LeftDelta(), tRect.bottom - mStaticCylinder.TopDelta() );
			mCylinder->MoveWindow( tCylinderRect );					
		}
	}
}


BOOL CCylinderPosition::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );		

	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticCylinder.LeftDelta() && tPoint.x < tRect.right - mStaticCylinder.LeftDelta() && tPoint.y > tRect.top + mStaticCylinder.TopDelta() && tPoint.y < tRect.bottom - mStaticCylinder.TopDelta() )
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

	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}


BOOL CCylinderPosition::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here	

	mInit = HSTrue;

	mPopupMenu.LoadMenu( IDR_POPUP_CYLINDER_POSITION_C );		

	mCylinder = new CGraphicCylinder( this );
	mCylinder->SetHits( &mHitsPosition );
	mCylinder->SetSensor( &mSensors );	

	mCylinder->SetXYZ( mAxisXLength / 1000.0, mAxisYLength / 1000.0, mAxisZLength / 1000.0 );
	mCylinder->SetMaterialXYZ( mMaterialXLength / 1000.0, mMaterialYLength / 1000.0, mMaterialZLength / 1000.0 );

	mCylinder->Create( IDD_GRAPHICCYLINDER, this );
	mCylinder->ShowWindow( SW_SHOW );

	mCylinder->SetHitRadius( mHitRadius );

	InitPositionSensors();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CCylinderPosition::OnCylinderPositionPopupAuto()
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

	if ( tSensorCount < 4 )
	{
		return;
	}
	
	HSInt tEachFaceSensorCount = ( tSensorCount + 5 ) / 6;	

	HSInt tRows = max( ( HSInt )sqrt( ( HSDouble )tEachFaceSensorCount ), 1 );
	HSInt tColumns = ( tEachFaceSensorCount + tRows - 1 ) / tRows;

	HSDouble tXDelta = mMaterialXLength / ( tRows + 1 );
	HSDouble tYDelta = mMaterialYLength / ( tColumns + 1 );
	HSDouble tZDelta = mMaterialZLength / ( tRows + 1 );
	
	HSDouble tFirstDelta = tXDelta;
	HSDouble tSecondDelta = tYDelta;
	HSInt tTmpEachFaceSensorCount = tEachFaceSensorCount;
	HSInt tFace = 0;
	HSInt tRow = 0;
	HSInt tColumn = 0;
	pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() && tSensorCount > 0 )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			pIterator->second.Face = tFace;
			pIterator->second.PosX = ( tRow + 1 ) * tFirstDelta;
			pIterator->second.PosY = ( tColumn + 1 ) * tSecondDelta;

			tRow++;
			tTmpEachFaceSensorCount--;
			tSensorCount--;

			if ( tRow == tRows )
			{
				tRow = 0;
				tColumn++;
			}

			if ( tTmpEachFaceSensorCount == 0 )
			{
				tTmpEachFaceSensorCount = tEachFaceSensorCount;
				tFace = ( tFace + 1 ) % 6;	
				tRow = 0;
				tColumn = 0;

				if ( tFace < 2 )
				{
					tFirstDelta = tXDelta;
					tSecondDelta = tYDelta;
				}
				else if ( tFace < 4 )
				{
					tFirstDelta = tZDelta;
					tSecondDelta = tYDelta;
				}
				else
				{		
					 tZDelta = mMaterialZLength / ( tColumns + 1 );
					
					tFirstDelta = tXDelta;
					tSecondDelta = tZDelta;
				}
			}
		}

		pIterator++;
	}	

	RefreshSensors();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
	pMainFrame->SetFocusedGraphic( this, HSTrue );	
}


void CCylinderPosition::OnCylinderPositionPopupClose()
{
	// TODO: Add your command handler code here

	CDialogEx::OnOK();

	mParent->SubViewClosed( this );
}


void CCylinderPosition::OnCylinderPositionPopupExportData()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	HSChar tBuf[ 1024 ];
	CString tStrChannel;

	CDataExportHelper tDataExport( this );
	if ( tDataExport.GetFilePath() )
	{
		tDataExport.Write( "方体定位\n" );
		sprintf_s( tBuf, "日期: %s %s\n", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
		tDataExport.Write( tBuf ); 

		sprintf_s( tBuf, "材料尺寸(长宽高)：%.2f (m), %.2f (m), %.2f (m)\n", this->mMaterialXLength / 1000.0, this->mMaterialZLength / 1000.0, this->mMaterialYLength / 1000.0 );
		tDataExport.Write( tBuf ); 

		sprintf_s( tBuf, "\n% 10s\t% 18s\t% 18s\t% 18s\t% 16s\t% 16s\n", "传感器编号", "X(m)", "Y(m)", "Z(m)", "采集卡", "通道" );
		tDataExport.Write( tBuf );

		map< HSInt, PositionSensor >::iterator pSensorIterator = mPositionSensors.begin();
		while ( pSensorIterator != mPositionSensors.end() )
		{
			if ( pSensorIterator->second.IsOn && !pSensorIterator->second.Forbid )
			{				
				CGraphicCylinder::SensorInfo tSensor;
				CoordWithFaceInfo( &( pSensorIterator->second ), tSensor );

				tStrChannel.Format( "%d", pSensorIterator->second.DataIdentifier.ChannelIndex() + 1 );
				IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( pSensorIterator->second.DataIdentifier );
				sprintf_s( tBuf, "% 10d\t% 18f\t% 18f\t% 18f\t% 16s\t% 16s\n", 
					pSensorIterator->first + 1, 
					tSensor.PosX / 1000.0, 
					tSensor.PosY / 1000.0, 
					tSensor.PosZ / 1000.0, 
					( pDataHandler == NULL ? "" : pDataHandler->Name().c_str() ),
					( pDataHandler == NULL ? "" : ( LPCSTR )tStrChannel ) );

				tDataExport.Write( tBuf ); 
			}

			pSensorIterator++;
		}

		sprintf_s( tBuf, "\n% 10s\t% 18s\t% 18s\t% 18s\n", "撞击编号",  "X(m)", "Y(m)", "Z(m)" );
		tDataExport.Write( tBuf ); 

		HSInt tIndex = 1;
		list< CGraphicCylinder::HitPosition >::iterator pHitIterator = mHitsPosition.begin();
		while ( pHitIterator != mHitsPosition.end() )
		{
			sprintf_s( tBuf, "% 10d\t% 18f\t% 18f\t% 18f\n", tIndex++, pHitIterator->PosX / 1000.0, pHitIterator->PosY / 1000.0, pHitIterator->PosZ / 1000.0 );
			tDataExport.Write( tBuf ); 

			pHitIterator++;
		}

		tDataExport.Finish();
	}	
}


void CCylinderPosition::OnCylinderPositionPopupFullScreen()
{
	// TODO: Add your command handler code here

	mIsFullScreen = !mIsFullScreen;
	mParent->SubViewFullScreen( this, mIsFullScreen );

	mCylinder->Refresh();
}


void CCylinderPosition::OnUpdateCylinderPositionPopupFullScreen(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( mIsFullScreen );

	mCylinder->Refresh();
}


void CCylinderPosition::OnCylinderPositionPopupReset()
{
	// TODO: Add your command handler code here
	mCylinder->Reset();
	mCylinder->Refresh();
}


void CCylinderPosition::OnCylinderPositionPopupSetdatasource()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->OnViewGraphic();	
	pMainFrame->SetFocusedGraphic( this );

	if ( !mStaticCylinder.Focused() || pMainFrame->FocusedGraphic() != this )
	{		
		mStaticCylinder.SetFocused( true );
		mStaticCylinder.ResizeWave();
		mStaticCylinder.Invalidate();
		mCylinder->Invalidate();
	}
}
