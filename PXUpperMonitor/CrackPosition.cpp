// CrackPosition.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "CrackPosition.h"
#include "afxdialogex.h"
#include "CrackPositionProperty.h"
#include "MainFrm.h"
#include <algorithm>
#include "ArgCalculator.h"
#include "HSLogProtocol.h"
#include "DataExportHelper.h"
#include "VirtualHitPosition.h"

// CCrackPosition dialog

CCrackPosition::CCrackPosition(CWnd* pParent /*=NULL*/)
	: CNormalGraphic( pParent )
{
	mDragingSensorIndex = -1;
	mMaterialLength = 300 * 1000.0;
	mMaterialWidth = 300 * 1000.0;

	mXAxisLength = 300;
	mYAxisLength = 300;
	mHitRadius = 3;

	mCheckRadius = 10 * 1000;
	mDragCheckCircle = HSFalse;
	mCheckCircleEnable = HSFalse;

	mSensorFirstCircle = 40;
	mSensorSecondCircle = 80;
	mSensorStartAngle = 45;

	mLastSensorSetGPS = 0;

	mIsShowOutLine = HSFalse;
	mIsShowAngleLine = HSFalse;

	mCenterXCoord = 16.0;
	mCenterYCoord = 19.0;
}

CCrackPosition::~CCrackPosition()
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

HSBool CCrackPosition::Start()
{
	if ( mPositionSensors.size() > 0 && CVirtualHitPosition::SharedInstance()->EnableStart( this ) )
	{		
		mRefreshThread->Start();
		return HSTrue;
	}

	return HSFalse;
}

HSBool CCrackPosition::Save( CINIConfig *pIniConfig, string tGroup )
{
	pIniConfig->SetValue( "MaterialLength", this->mMaterialLength, tGroup );
	pIniConfig->SetValue( "MaterialWidth", this->mMaterialWidth, tGroup );

	pIniConfig->SetValue( "XAxisLength", this->mXAxisLength, tGroup );
	pIniConfig->SetValue( "YAxisLength", this->mYAxisLength, tGroup );
	pIniConfig->SetValue( "HitRadius", this->mHitRadius, tGroup );

	pIniConfig->SetValue( "CenterXCoord", this->mCenterXCoord, tGroup );
	pIniConfig->SetValue( "CenterYCoord", this->mCenterYCoord, tGroup );

	pIniConfig->SetValue( "SensorFirstCircle", this->mSensorFirstCircle, tGroup );
	pIniConfig->SetValue( "SensorSecondCircle", this->mSensorSecondCircle, tGroup );
	pIniConfig->SetValue( "SensorStartAngle", this->mSensorStartAngle, tGroup );
		
	HSChar tBuf[ 1024 ];
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn )
		{
			sprintf_s( tBuf, "Sensor%d_DataIdentifier", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.DataIdentifier.ToString(), tGroup );

			sprintf_s( tBuf, "Sensor%d_XPosition", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.XPos, tGroup );

			sprintf_s( tBuf, "Sensor%d_YPosition", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.YPos, tGroup );

			sprintf_s( tBuf, "Sensor%d_Forbid", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Forbid, tGroup );
		}
		pIterator++;
	}	
	
	return HSTrue;
}

HSBool CCrackPosition::Load( CINIConfig *pIniConfig, string tGroup )
{		
	pIniConfig->ValueWithKey( "MaterialLength", this->mMaterialLength, tGroup );
	pIniConfig->ValueWithKey( "MaterialWidth", this->mMaterialWidth, tGroup );

	pIniConfig->ValueWithKey( "XAxisLength", this->mXAxisLength, tGroup );
	pIniConfig->ValueWithKey( "YAxisLength", this->mYAxisLength, tGroup );
	pIniConfig->ValueWithKey( "HitRadius", this->mHitRadius, tGroup );	

	pIniConfig->ValueWithKey( "CenterXCoord", this->mCenterXCoord, tGroup );
	pIniConfig->ValueWithKey( "CenterYCoord", this->mCenterYCoord, tGroup );	

	pIniConfig->ValueWithKey( "SensorFirstCircle", this->mSensorFirstCircle, tGroup );
	pIniConfig->ValueWithKey( "SensorSecondCircle", this->mSensorSecondCircle, tGroup );
	pIniConfig->ValueWithKey( "SensorStartAngle", this->mSensorStartAngle, tGroup );	
	
	HSString tDataIdentifier = "";
	HSDouble tXPosition = 0;
	HSDouble tYPosition = 0;
	HSBool tForbid = HSFalse;

	HSChar tBufId[ 1024 ];
	HSChar tBufXPos[ 1024 ];
	HSChar tBufYPos[ 1024 ];
	HSChar tBufForbid[ 1024 ];

	for ( HSInt i = 0; i < CCrackPositionProperty::POSITION_SENSOR_MAX_NUM; i++ )
	{
		sprintf_s( tBufId, "Sensor%d_DataIdentifier", i );
		sprintf_s( tBufXPos, "Sensor%d_XPosition", i );
		sprintf_s( tBufYPos, "Sensor%d_YPosition", i );
		sprintf_s( tBufForbid, "Sensor%d_Forbid", i );
		if ( pIniConfig->ValueWithKey( tBufId, tDataIdentifier, tGroup )
			&& pIniConfig->ValueWithKey( tBufXPos, tXPosition, tGroup )
			&& pIniConfig->ValueWithKey( tBufYPos, tYPosition, tGroup )
			&& pIniConfig->ValueWithKey( tBufForbid, tForbid, tGroup ) )
		{			
			mPositionSensors[ i ].XPos = tXPosition;
			mPositionSensors[ i ].YPos = tYPosition;
			mPositionSensors[ i ].Forbid = tForbid;
			mPositionSensors[ i ].IsOn = HSTrue;	
			mPositionSensors[ i ].DataIdentifier.InitWithString( tDataIdentifier );
			mPositionSensors[ i ].ArgTransfer = NULL;			
		}
	}		
	
	return HSTrue;
}	

HSBool CCrackPosition::CrossPosition( HSDouble tPosX, HSDouble tPosY, CStaticCrackPosition::OutLinePos tResVerPos[ 2 ], CStaticCrackPosition::OutLinePos tResHorPos[ 2 ] )
{
	vector< CStaticCrackPosition::OutLinePos > tTmpOutLinePoints;
	list< CStaticCrackPosition::OutLinePos >::iterator pIterator = mOutLinePoints.begin();
	while ( pIterator != mOutLinePoints.end() )
	{
		tTmpOutLinePoints.push_back( *pIterator );
		pIterator++;
	}

	if ( tTmpOutLinePoints.size() < 3 )
	{
		return HSFalse;
	}

	tTmpOutLinePoints.push_back( *mOutLinePoints.begin() );

	vector< CStaticCrackPosition::OutLinePos > tVerLine;
	vector< CStaticCrackPosition::OutLinePos > tHorLine;

	for ( HSUInt i = 1; i < tTmpOutLinePoints.size(); i++ )
	{
		if ( ( tTmpOutLinePoints[ i ].YPos > tPosY && tTmpOutLinePoints[ i - 1 ].YPos < tPosY ) || ( tTmpOutLinePoints[ i ].YPos < tPosY && tTmpOutLinePoints[ i - 1 ].YPos > tPosY ) )
		{
			tHorLine.push_back( tTmpOutLinePoints[ i ] );
			tHorLine.push_back( tTmpOutLinePoints[ i - 1 ] );
		}

		if ( ( tTmpOutLinePoints[ i ].XPos > tPosX && tTmpOutLinePoints[ i - 1 ].XPos < tPosX ) || ( tTmpOutLinePoints[ i ].XPos < tPosX && tTmpOutLinePoints[ i - 1 ].XPos > tPosX ) )
		{
			tVerLine.push_back( tTmpOutLinePoints[ i ] );
			tVerLine.push_back( tTmpOutLinePoints[ i - 1 ] );
		}
	}

	if ( tHorLine.size() < 4 || tVerLine.size() < 4 )
	{
		return HSFalse;
	}

	tResVerPos[ 0 ].XPos = tPosX;
	tResVerPos[ 1 ].XPos = tPosX;
	tResVerPos[ 0 ].YPos = ( tPosX - tVerLine[ 0 ].XPos ) / ( tVerLine[ 1 ].XPos - tVerLine[ 0 ].XPos ) * ( tVerLine[ 1 ].YPos - tVerLine[ 0 ].YPos ) + tVerLine[ 0 ].YPos;
	tResVerPos[ 1 ].YPos = ( tPosX - tVerLine[ 2 ].XPos ) / ( tVerLine[ 3 ].XPos - tVerLine[ 2 ].XPos ) * ( tVerLine[ 3 ].YPos - tVerLine[ 2 ].YPos ) + tVerLine[ 2 ].YPos;

	tResHorPos[ 0 ].YPos = tPosY;
	tResHorPos[ 1 ].YPos = tPosY;
	tResHorPos[ 0 ].XPos = ( tPosY - tHorLine[ 0 ].YPos ) / ( tHorLine[ 1 ].YPos - tHorLine[ 0 ].YPos ) * ( tHorLine[ 1 ].XPos - tHorLine[ 0 ].XPos ) + tHorLine[ 0 ].XPos;
	tResHorPos[ 1 ].XPos = ( tPosY - tHorLine[ 2 ].YPos ) / ( tHorLine[ 3 ].YPos - tHorLine[ 2 ].YPos ) * ( tHorLine[ 3 ].XPos - tHorLine[ 2 ].XPos ) + tHorLine[ 2 ].XPos;

	return HSTrue;
}
	
IGraphicPropertyProtocol * CCrackPosition::CreatePropertyDlg( CWnd *pParent )
{
	CCrackPositionProperty *pPositionProperty = new CCrackPositionProperty;
	pPositionProperty->Create( IDD_NORMALPROPERTY, pParent );
	return pPositionProperty;
}

HSVoid CCrackPosition::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		this->UpdatePositionSensor( pIterator->first, pIterator->second.XPos, pIterator->second.YPos, pIterator->second.Forbid, pIterator->second.IsOn, pIterator->second.DataIdentifier );
		pIterator++;
	}

	this->mHitsPosition.clear();

	RefreshSensors();

	CreateVirtualHits();

	SetEvent( mDataMutex );	
}

HSVoid CCrackPosition::SetCheckRadius( HSDouble tValue )
{
	mCheckRadius = tValue;
	mStaticRelation.SetCheckRadius( tValue, this->mCheckCircleEnable );
	mStaticRelation.Invalidate();
}

HSVoid CCrackPosition::SetCheckPosX( HSDouble tValue )
{
	mStaticRelation.SetCheckPos( tValue, mStaticRelation.YCheckPos() );
	mStaticRelation.Invalidate();
}

HSVoid CCrackPosition::SetCheckPosY( HSDouble tValue )
{
	mStaticRelation.SetCheckPos( mStaticRelation.XCheckPos(), tValue );
	mStaticRelation.Invalidate();
}

HSDouble CCrackPosition::DistanceFromSensor( HSDouble tPosX, HSDouble tPosY, HSInt tSensor )
{	
	HSDouble tX = tPosX - mPositionSensors[ tSensor ].AxisPosX;
	HSDouble tY = tPosY - mPositionSensors[ tSensor ].AxisPosY;

	return sqrt( tX * tX + tY * tY );
}

HSDouble CCrackPosition::DistanceBetweenSensors( HSInt tSensorOne, HSInt tSensorTwo )
{		
	return DistanceFromSensor( mPositionSensors[ tSensorOne ].AxisPosX, mPositionSensors[ tSensorOne ].AxisPosY, tSensorTwo );
}

HSBool CCrackPosition::IsHitQualified( HSInt64 tDuration, HSDouble tVelocity, HSInt tSensorOne, HSInt tSensorTwo )
{
	HSDouble tDistanceVoice = abs( tDuration / 1000000000.0 * tVelocity );	

	HSDouble tPhyDistance = DistanceBetweenSensors( tSensorOne, tSensorTwo );

	return tDistanceVoice < tPhyDistance;
}

HSBool CCrackPosition::GetHitPosition( vector< HIT_CALC_INFO > &tHitSensors, CStaticCrackPosition::HitPosition &tResHitPos )
{
	HSInt tIndex = 0;
	for ( HSUInt i = 1; i < tHitSensors.size(); i++ )
	{
		if ( mPositionSensors[ tHitSensors[ i ].Sensor ].ArgList[ tHitSensors[ i ].ArgIndex ].NBeginTime < mPositionSensors[ tHitSensors[ tIndex ].Sensor ].ArgList[ tHitSensors[ tIndex ].ArgIndex ].NBeginTime )
		{
			tIndex = i;
		}
	}

	tResHitPos.XPos = mPositionSensors[ tHitSensors[ tIndex ].Sensor ].ArgList[ tHitSensors[ tIndex ].ArgIndex ].HRMS;
	tResHitPos.YPos = mPositionSensors[ tHitSensors[ tIndex ].Sensor ].ArgList[ tHitSensors[ tIndex ].ArgIndex ].HASL;
	tResHitPos.ZPos = mPositionSensors[ tHitSensors[ tIndex ].Sensor ].ArgList[ tHitSensors[ tIndex ].ArgIndex ].Amplitude;
	tResHitPos.Color = CGraphicManager::SharedInstance()->ColorWithEnergy( mPositionSensors[ tHitSensors[ tIndex ].Sensor ].ArgList[ tHitSensors[ tIndex ].ArgIndex ].Energy );

	return HSTrue;
}

HSBool CCrackPosition::CalcHitPosition( HSInt tSensor, HSUInt tArgIndex )
{
	vector< HIT_CALC_INFO > tHitSensors;
	
	HIT_CALC_INFO tHit = { tSensor, tArgIndex };
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
					HIT_CALC_INFO tHit = { pIterator->first, i };
					tHitSensors.push_back( tHit );

					break;
				}
			}
		}
		pIterator++;
	}

	if ( tHitSensors.size() < 3 )
	{		
		//mPositionSensors[ tSensor ].ArgHandled[ tArgIndex ] = HSTrue;
		return HSFalse;
	}	

	CStaticCrackPosition::HitPosition tResHitPos;
	if ( GetHitPosition( tHitSensors, tResHitPos ) )
	{	
		mHitsPosition.push_back( tResHitPos );	
	}

	for ( HSUInt i = 0; i < tHitSensors.size(); i++ )
	{
		mPositionSensors[ tHitSensors[ i ].Sensor ].ArgHandled[ tHitSensors[ i ].ArgIndex ] = HSTrue;
	}	
	
	return HSTrue;
}


HSVoid CCrackPosition::SetMaterialLength( HSDouble tLength )
{
	mMaterialLength = max( tLength, 10 );
	mStaticRelation.SetMaterialLength( mMaterialLength );
	mStaticRelation.Invalidate();
}

HSVoid CCrackPosition::SetMaterialWidth( HSDouble tWidth )
{
	mMaterialWidth = max( tWidth, 10 );
	mStaticRelation.SetMaterialWidth( mMaterialWidth );
	mStaticRelation.Invalidate();
}

HSVoid CCrackPosition::InitPositionSensors()
{	
	HSInt tIndex = 0;
	while ( mPositionSensors.size() < 3 )
	{
		if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
		{
			this->UpdatePositionSensor( tIndex, 0.0, 0.0, HSFalse, HSTrue, 0 );
		}
		tIndex++;
	}

	this->RefreshSensors();
}

HSVoid CCrackPosition::SetFocusSensor( HSInt tIndex )
{
	mStaticRelation.SetFocusSensor( tIndex );
	mStaticRelation.Invalidate();
}

HSVoid CCrackPosition::GetSensorCircleInfo( HSDouble &tFirstCircle, HSDouble &tSecondCircle, HSDouble &tStartAngle )
{
	tFirstCircle = mSensorFirstCircle;
	tSecondCircle = mSensorSecondCircle;
	tStartAngle = mSensorStartAngle;
}

HSVoid CCrackPosition::SetSensorCircleInfo( HSDouble tFirstCircle, HSDouble tSecondCircle, HSDouble tStartAngle )
{
	mSensorFirstCircle = ( tFirstCircle < 0 ? mSensorFirstCircle : tFirstCircle );
	mSensorSecondCircle = ( tSecondCircle < 0 ? mSensorSecondCircle : tSecondCircle );
	mSensorStartAngle = ( tStartAngle < 0 ? mSensorStartAngle : tStartAngle );

	mStaticRelation.SetSensorCircle( mSensorFirstCircle, mSensorSecondCircle );

	RefreshSensors();
}

HSVoid CCrackPosition::SetXAxisLength( HSDouble tLength )
{
	mXAxisLength = tLength;
	mStaticRelation.SetXValue( mXAxisLength, -mXAxisLength / 2  );
	mStaticRelation.ResizeWave();
	mStaticRelation.Invalidate();
}

HSVoid CCrackPosition::SetYAxisLength( HSDouble tLength )
{
	mYAxisLength = tLength;	
	mStaticRelation.SetYValue( mYAxisLength, mYAxisLength / 2 );
	mStaticRelation.ResizeWave();
	mStaticRelation.Invalidate();
}

HSVoid CCrackPosition::SetHitRadius( HSInt tHitRadius )
{
	mHitRadius = tHitRadius;
	mStaticRelation.SetHitRadius( mHitRadius );
	mStaticRelation.Invalidate();
}

HSVoid CCrackPosition::UpdatePositionSensor( HSInt tIndex, HSDouble tPositionX, HSDouble tPositionY, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier )
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

	mPositionSensors[ tIndex ].XPos = tPositionX;
	mPositionSensors[ tIndex ].YPos = tPositionY;
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
}

HSVoid CCrackPosition::RefreshSensors()
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

	if ( tSensorCount == 0 )
	{
		mSensors.clear();
		mStaticRelation.SetSensors( &mSensors );
		mStaticRelation.Invalidate();

		return;
	}

	HSDouble tAngleDelta = 360.0 / tSensorCount;
	tAngleDelta = max( tAngleDelta, 90.0 );

	HSDouble tSecondCircleStart = mSensorStartAngle + tAngleDelta / 2;
	tSecondCircleStart -= 360 * HSInt( tSecondCircleStart / 360 );
	
	HSInt tIndex = 0;
	HSDouble tAngle = mSensorStartAngle;
	HSDouble tRadius = mSensorFirstCircle;

	mSensors.clear();
	pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			CStaticCrackPosition::SensorInfo tSensor;
			tSensor.Index = pIterator->first;
			tSensor.YPos = cos( tAngle * 3.14159265 / 180.0 ) * tRadius * 1000.0;
			tSensor.XPos = sin( tAngle * 3.14159265 / 180.0 ) * tRadius * 1000.0;
			mSensors.push_back( tSensor );

			pIterator->second.AxisPosX = tSensor.XPos / 1000.0;
			pIterator->second.AxisPosY = tSensor.YPos / 1000.0;

			pIterator->second.Angle = tAngle;
			pIterator->second.Radius = tRadius;

			tAngle += tAngleDelta;
			tAngle -= 360 * HSInt( tAngle / 360 );
			tIndex++;
			if ( tIndex == 4 )
			{
				tAngle = tSecondCircleStart;
				tRadius = mSensorSecondCircle;
			}
		}
		pIterator++;
	}
	
	mStaticRelation.SetSensorCircle( mSensorFirstCircle, ( tIndex > 4 ? mSensorSecondCircle : -1 ) );
	mStaticRelation.SetSensors( &mSensors );
	mStaticRelation.Invalidate();
}

CCrackPosition::PositionSensor * CCrackPosition::GetPositionSensor( HSInt tIndex )
{
	if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
	{
		return NULL;
	}

	return &( mPositionSensors[ tIndex ] );
}

HSVoid CCrackPosition::PosWithGPSInfo( HSDouble tLongitude, HSDouble tLatitude, HSDouble tAngle, HSDouble tDistance, HSDouble &tResLongitude, HSDouble &tResLatitude )
{
	HSDouble tEarthR = 6378.1;
	HSDouble tBearing = AngleToDegree( tAngle );
	tDistance /= 1000.0;	

	tLongitude = AngleToDegree( tLongitude );
	tLatitude = AngleToDegree( tLatitude );

	tResLatitude = asin( sin( tLatitude ) * cos( tDistance / tEarthR ) + cos( tLatitude ) * sin( tDistance / tEarthR ) * cos( tBearing ) );	
	tResLongitude = tLongitude + atan2( sin( tBearing ) * sin( tDistance / tEarthR ) * cos( tLatitude ), cos( tDistance / tEarthR ) - sin( tLatitude ) * sin( tResLatitude ) );

	tResLatitude = DegreeToAngle( tResLatitude );
	tResLongitude = DegreeToAngle( tResLongitude );
}

HSVoid CCrackPosition::GetTheNearest3Sensor( HSDouble tX, HSDouble tY, HSInt &tChannel1, HSInt &tChannel2, HSInt &tChannel3 )
{
	typedef struct SensorHitDistInfo
	{
		HSInt Sensor;
		HSDouble Distance;

		bool operator<( const SensorHitDistInfo &tValue ) const
		{
			return Distance < tValue.Distance;
		}

	} SensorHitDistInfo;

	vector< SensorHitDistInfo > tSensorsHitDistInfo;

	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			SensorHitDistInfo tSensorHitDistInfo = { pIterator->first, DistanceFromSensor( tX, tY, pIterator->first ) };
			tSensorsHitDistInfo.push_back( tSensorHitDistInfo );
		}
		pIterator++;
	}	

	std::sort( tSensorsHitDistInfo.begin(), tSensorsHitDistInfo.end() );

	if ( tSensorsHitDistInfo.size() < 3 )
	{
		return;
	}

	//HS_INFO( "Nearest Sensors: %d, %d, %d", tSensorsHitDistInfo[ 0 ].Sensor, tSensorsHitDistInfo[ 1 ].Sensor, tSensorsHitDistInfo[ 2 ].Sensor );
	HSDouble tSensorEffectiveMaxDistance = min( 150.0, max( 100.0, this->DistanceBetweenSensors( 0, 2 ) ) );

	if ( mPositionSensors[ tSensorsHitDistInfo[ 0 ].Sensor ].ArgTransfer != NULL && tSensorsHitDistInfo[ 0 ].Distance < tSensorEffectiveMaxDistance )
	{
		tChannel1 = mPositionSensors[ tSensorsHitDistInfo[ 0 ].Sensor ].DataIdentifier.ChannelIndex();
	}

	if ( mPositionSensors[ tSensorsHitDistInfo[ 1 ].Sensor ].ArgTransfer != NULL && tSensorsHitDistInfo[ 1 ].Distance < tSensorEffectiveMaxDistance )
	{
		tChannel2 = mPositionSensors[ tSensorsHitDistInfo[ 1 ].Sensor ].DataIdentifier.ChannelIndex();
	}

	if ( mPositionSensors[ tSensorsHitDistInfo[ 2 ].Sensor ].ArgTransfer != NULL && tSensorsHitDistInfo[ 2 ].Distance < tSensorEffectiveMaxDistance )
	{
		tChannel3 = mPositionSensors[ tSensorsHitDistInfo[ 2 ].Sensor ].DataIdentifier.ChannelIndex();
	}
}

HSVoid CCrackPosition::CreateVirtualHits()
{
	CVirtualHitPosition *pVirtualHitPosition = CVirtualHitPosition::SharedInstance();
	pVirtualHitPosition->Reset( this );

	if ( !pVirtualHitPosition->EnableStart( this ) )
	{
		return;
	}

	srand( ( HSInt ) time( 0 ) );
	HSInt tAngleD = ( HSInt( 58 * mCenterXCoord ) + HSInt( 59 * mCenterYCoord ) ) % 360;
	HSDouble tAngle = tAngleD * 3.14159265 / 180;
	HSDouble tVerAngle = ( ( tAngleD - 90 + 360 ) % 360 ) * 3.14159265 / 180;
	for ( HSInt i = 0; i < 200; i++ )
	{	
		HSDouble tDistance = ( i + 1 ) * 2.0;
		if ( i > 120 && i % 2 == 0 )
		{
			//tDistance /= 8.0;
			tDistance /= ( rand() % 8 + 1 );
		}		
		
		CVirtualHitPosition::HIT_INFO tHit;	
		
		HSInt tDirection = ( rand() % 100 > 50 || tDistance > 120 * 1.5 ? 1 : -1 );
		HSInt tOffsetDirection = ( rand() % 40 > 20 ? 1 : -1 );
		tHit.X = tOffsetDirection * ( rand() % 120 ) * sin( tVerAngle ) + tDistance * sin( tAngle ) * tDirection;
		tHit.Y = tOffsetDirection * ( rand() % 120 ) * cos( tVerAngle ) + tDistance * cos( tAngle ) * tDirection;
		tHit.Z = -( rand() % 12 );
		tHit.Distance = sqrt( tHit.X * tHit.X + tHit.Y * tHit.Y );
		tHit.Energy = rand() % 6000;
		tHit.Channels[ 0 ] = -1;
		tHit.Channels[ 1 ] = -1;
		tHit.Channels[ 2 ] = -1;

		GetTheNearest3Sensor( tHit.X, tHit.Y, tHit.Channels[ 0 ], tHit.Channels[ 1 ], tHit.Channels[ 2 ] );

		if ( tHit.Channels[ 0 ] >= 0 && tHit.Channels[ 1 ] >= 0 && tHit.Channels[ 2 ] >= 0 )
		{
			pVirtualHitPosition->AddHit( tHit );
		}

		for ( HSInt j = 0; j < 2; j++ )
		{
			tHit.X = tOffsetDirection * ( rand() % 40 ) * sin( tVerAngle ) + tDistance * sin( tAngle ) * tDirection;
			tHit.Y = tOffsetDirection * ( rand() % 40 ) * cos( tVerAngle ) + tDistance * cos( tAngle ) * tDirection;
			tHit.Z = -( rand() % 12 );
			tHit.Distance = sqrt( tHit.X * tHit.X + tHit.Y * tHit.Y );// * ( rand() % 1000 );
			tHit.Energy = rand() % 6000;
			tHit.Channels[ 0 ] = -1;
			tHit.Channels[ 1 ] = -1;
			tHit.Channels[ 2 ] = -1;

			GetTheNearest3Sensor( tHit.X, tHit.Y, tHit.Channels[ 0 ], tHit.Channels[ 1 ], tHit.Channels[ 2 ] );

			if ( tHit.Channels[ 0 ] >= 0 && tHit.Channels[ 1 ] >= 0 && tHit.Channels[ 2 ] >= 0 )
			{
				pVirtualHitPosition->AddHit( tHit );
			}
		}

		for ( HSInt j = 0; j < 2; j++ )
		{
			tHit.X = tOffsetDirection * ( rand() % 10 ) * sin( tVerAngle ) + tDistance * sin( tAngle ) * tDirection;
			tHit.Y = tOffsetDirection * ( rand() % 10 ) * cos( tVerAngle ) + tDistance * cos( tAngle ) * tDirection;
			tHit.Z = -( rand() % 12 );
			tHit.Distance = sqrt( tHit.X * tHit.X + tHit.Y * tHit.Y );// * ( rand() % 1000 );
			tHit.Energy = rand() % 6000;
			tHit.Channels[ 0 ] = -1;
			tHit.Channels[ 1 ] = -1;
			tHit.Channels[ 2 ] = -1;

			GetTheNearest3Sensor( tHit.X, tHit.Y, tHit.Channels[ 0 ], tHit.Channels[ 1 ], tHit.Channels[ 2 ] );

			if ( tHit.Channels[ 0 ] >= 0 && tHit.Channels[ 1 ] >= 0 && tHit.Channels[ 2 ] >= 0 )
			{
				pVirtualHitPosition->AddHit( tHit );
			}
		}
	}

	pVirtualHitPosition->Sort();
}

HSVoid CCrackPosition::InitDialog()
{
	mPopupMenu.LoadMenu( IDR_POPUP_CRACK_POSITION_C );

	CRect tRect;
	this->GetClientRect( &tRect );
	mStaticRelation.Create( "", WS_CHILD | WS_VISIBLE, tRect, this );

	mStaticGraphic = &mStaticRelation;	
	
	mStaticRelation.SetOutLinePositions( &mOutLinePoints );
	mStaticRelation.SetHitsPosition( &mHitsPosition );
	mStaticRelation.SetSensors( &mSensors );
	mStaticRelation.SetMaterialLength( mMaterialLength );
	mStaticRelation.SetMaterialWidth( mMaterialWidth );

	mStaticRelation.SetXValue( mXAxisLength, -mXAxisLength / 2  );	
	mStaticRelation.SetYValue( mYAxisLength, mYAxisLength / 2 );
	mStaticRelation.SetHitRadius( mHitRadius );

	mStaticRelation.SetCheckRadius( mCheckRadius, HSFalse );
	mStaticRelation.SetCheckPos( mMaterialWidth / 2, mMaterialLength / 2 );
	mStaticRelation.SetSensorCircle( mSensorFirstCircle, mSensorSecondCircle );

	InitPositionSensors();
}

HSBool CCrackPosition::GraphicNeedRefresh()
{
	//HSDouble tHitDuration = CArgCalculator::SharedInstance()->GetHitFullTime() / 1000000000.0 + 0.0005;
	HSDouble tHitDuration = 0.0;

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

	mStaticRelation.Invalidate();
	
	return HSTrue;
}

HSVoid CCrackPosition::StepGraphic( HSInt tDelta )
{
	WaitForSingleObject( mDataMutex, INFINITE );		

	mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() + tDelta * mStaticRelation.TotalXValue() );
	mStaticRelation.Invalidate();

	SetEvent( mDataMutex );				
}

HSVoid CCrackPosition::ZoomInGraphic( CRect &tViewRect )
{		
	WaitForSingleObject( mDataMutex, INFINITE );			

	HSDouble tXValueOffset = max( 0.0, mZoomInRect.left - mStaticRelation.LeftDelta() ) / tViewRect.Width() * mStaticRelation.TotalXValue();
	HSDouble tNewXValue = ( HSDouble )( mZoomInRect.Width() ) / tViewRect.Width() * mStaticRelation.TotalXValue();			

	HSDouble tBeginYValue = mStaticRelation.BeginYValue() - max( 0.0, mZoomInRect.top - mStaticRelation.TopDelta() ) / tViewRect.Height() * mStaticRelation.TotalYValue();
	HSDouble tTotalYValue = ( HSDouble )mZoomInRect.Height() / tViewRect.Height() * mStaticRelation.TotalYValue();

	mStaticRelation.SetXValue( tNewXValue, mStaticRelation.BeginXValue() + tXValueOffset );
	mStaticRelation.SetYValue( tTotalYValue, tBeginYValue );		
	mStaticRelation.Invalidate();

	SetEvent( mDataMutex );
}

HSVoid CCrackPosition::DragGraphic( CRect &tViewRect, CPoint tPoint )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	HSInt tYOffset = mPrevPoint.y - tPoint.y;			
	HSDouble tYValuePerDigit = mStaticRelation.TotalYValue() / tViewRect.Height();
	HSDouble tBeginYValue = mStaticRelation.BeginYValue() - tYOffset * tYValuePerDigit;			
	mStaticRelation.SetYValue( mStaticRelation.TotalYValue(), tBeginYValue );		

	HSInt tXOffset = mPrevPoint.x - tPoint.x;
	HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticRelation.TotalXValue();		
	mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() + tXOffsetValue );
	mStaticRelation.Invalidate();

	SetEvent( mDataMutex );			
}


HSBool CCrackPosition::IsDataExportNeeded()
{
	return HSTrue;
}

HSVoid CCrackPosition::ExportData( CDataExportHelper &tDataExport )
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	HSChar tBuf[ 1024 ];

	tDataExport.Write( "裂缝定位\n" );
	sprintf_s( tBuf, "日期: %s %s\n", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
	tDataExport.Write( tBuf ); 

	sprintf_s( tBuf, "宽度：%.2f (m)\n", mMaterialWidth / 1000.0 );
	tDataExport.Write( tBuf ); 

	sprintf_s( tBuf, "高度：%.2f (m)\n", mMaterialLength / 1000.0 );
	tDataExport.Write( tBuf ); 


	sprintf_s( tBuf, "\n% 10s\t% 18s\t% 18s\t% 16s\t% 16s\n", "传感器编号", "X(m)", "Y(m)", "采集卡", "通道" );
	tDataExport.Write( tBuf ); 

	map< HSInt, PositionSensor >::iterator pSensorIterator = mPositionSensors.begin();
	while ( pSensorIterator != mPositionSensors.end() )
	{
		if ( pSensorIterator->second.IsOn && !pSensorIterator->second.Forbid )
		{
			IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( pSensorIterator->second.DataIdentifier );
			CString tStrChannel;
			tStrChannel.Format( "%d", pSensorIterator->second.DataIdentifier.ChannelIndex() + 1 );
			sprintf_s( tBuf, "% 10d\t% 18f\t% 18f\t% 16s\t% 16s\n", 
				pSensorIterator->first + 1, 
				pSensorIterator->second.XPos, 
				pSensorIterator->second.YPos, 
				( pDataHandler == NULL ? "" : pDataHandler->Name().c_str() ),
				( pDataHandler == NULL ? "" : tStrChannel ) );

			tDataExport.Write( tBuf ); 
		}

		pSensorIterator++;
	}		

	sprintf_s( tBuf, "\n% 10s\t% 18s\t% 18s\n", "撞击编号", "X(m)", "Y(m)" );
	tDataExport.Write( tBuf ); 

	HSInt tIndex = 1;
	list< CStaticCrackPosition::HitPosition >::iterator pHitIterator = mHitsPosition.begin();
	while ( pHitIterator != mHitsPosition.end() )
	{
		sprintf_s( tBuf, "% 10d\t% 18f\t% 18f\n", tIndex++, pHitIterator->XPos, pHitIterator->YPos );
		tDataExport.Write( tBuf ); 

		pHitIterator++;
	}
}

HSVoid CCrackPosition::ResetData( CMainFrame* pMainFrame )
{	
	WaitForSingleObject( mDataMutex, INFINITE );	
	
	mStaticRelation.SetXValue( mXAxisLength, -mXAxisLength / 2  );	
	mStaticRelation.SetYValue( mYAxisLength, mYAxisLength / 2 );

	mStaticRelation.Invalidate();

	SetEvent( mDataMutex );
}

HSVoid CCrackPosition::MouseDownWhenPointer(  CRect &tRect, CPoint tPoint )
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	if ( !mDragCheckCircle )
	{
		mDragCheckCircle = mStaticRelation.PositionWithinCheckCircle( tPoint, tRect );
	}

	if ( !pMainFrame->IsStart() && !mDragCheckCircle && mDragingSensorIndex < 0 )
	{
		mDragingSensorIndex = -1;//mStaticRelation.SensorInPosition( tPoint, tRect );
	}

	mPrevPoint = tPoint;
}

HSBool CCrackPosition::CustomMouseUpCheck()
{
	if ( mDragCheckCircle )
	{
		mDragCheckCircle = HSFalse;
		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
		pMainFrame->SetFocusedGraphic( this, HSTrue );

		return HSTrue;
	}
	else if ( mDragingSensorIndex >= 0 )
	{
		PositionSensor *pPositionSensor = GetPositionSensor( mDragingSensorIndex );
		if ( pPositionSensor != NULL )
		{
			if ( pPositionSensor->XPos < 0 || pPositionSensor->XPos > this->mMaterialWidth || pPositionSensor->YPos < 0 || pPositionSensor->YPos > this->mMaterialLength )
			{
				pPositionSensor->Forbid = HSTrue;
			}
			else
			{
				pPositionSensor->Forbid = HSFalse;
			}

			this->UpdatePositionSensor( mDragingSensorIndex, pPositionSensor->XPos, pPositionSensor->YPos, pPositionSensor->Forbid, pPositionSensor->IsOn, pPositionSensor->DataIdentifier );
			RefreshSensors();
		}

		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
		pMainFrame->SetFocusedGraphic( this, HSTrue );

		return HSTrue;
	}

	return HSFalse;
}

HSVoid CCrackPosition::MouseUpFinish()
{
	mDragingSensorIndex = -1;
}	

HSBool CCrackPosition::CustomMouseMoveCheck( CPoint tPoint )
{
	if ( mDragCheckCircle )
	{
		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		tViewRect = CRect( tViewRect.left + mStaticRelation.LeftDelta(), tViewRect.top + mStaticRelation.TopDelta(), tViewRect.right - mStaticRelation.LeftDelta(), tViewRect.bottom - mStaticRelation.TopDelta() );

		HSInt tXOffset = mPrevPoint.x - tPoint.x;
		HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticRelation.TotalXValue();

		HSInt tYOffset = mPrevPoint.y - tPoint.y;
		HSDouble tYOffsetValue = ( HSDouble )tYOffset / tViewRect.Height() * mStaticRelation.TotalYValue();
		mStaticRelation.SetCheckPos( mStaticRelation.XCheckPos() - tXOffsetValue * 1000.0, mStaticRelation.YCheckPos() + tYOffsetValue * 1000.0 );
		mStaticRelation.Invalidate();

		mPrevPoint = tPoint;

		return HSTrue;
	}
	else if ( mDragingSensorIndex >= 0 )
	{
		PositionSensor *pPositionSensor = GetPositionSensor( mDragingSensorIndex );
		if ( pPositionSensor != NULL )
		{
			CRect tViewRect;
			this->GetClientRect( &tViewRect );
			tViewRect = CRect( tViewRect.left + mStaticRelation.LeftDelta(), tViewRect.top + mStaticRelation.TopDelta(), tViewRect.right - mStaticRelation.LeftDelta(), tViewRect.bottom - mStaticRelation.TopDelta() );

			HSInt tXOffset = mPrevPoint.x - tPoint.x;
			HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticRelation.TotalXValue();

			HSInt tYOffset = mPrevPoint.y - tPoint.y;
			HSDouble tYOffsetValue = ( HSDouble )tYOffset / tViewRect.Height() * mStaticRelation.TotalYValue();

			this->UpdatePositionSensor( mDragingSensorIndex, pPositionSensor->XPos - tXOffsetValue * 1000.0, pPositionSensor->YPos + tYOffsetValue * 1000.0, pPositionSensor->Forbid, pPositionSensor->IsOn, pPositionSensor->DataIdentifier );			
			RefreshSensors();
		}

		mPrevPoint = tPoint;

		return HSTrue;
	}

	return HSFalse;
}

HSVoid CCrackPosition::MouseMoveOtherWork( CPoint tPoint )
{
	if ( this->mIsShowOutLine || mIsShowAngleLine )
	{
		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

		CRect tRect;
		this->GetClientRect( &tRect );
		HSDouble tXValue = 0;
		HSDouble tYValue = 0;
		if ( mIsShowOutLine )
		{
			CStaticCrackPosition::OutLinePos tVerPos[ 2 ];
			CStaticCrackPosition::OutLinePos tHorPos[ 2 ];
			if( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_POINTER && mStaticRelation.PosWithPoint( tPoint, tRect, tXValue, tYValue ) && CrossPosition( tXValue, tYValue, tVerPos, tHorPos ) )
			{
				mStaticRelation.SetDrawAxisPos( HSTrue, tVerPos, tHorPos );
			}
			else
			{
				mStaticRelation.SetDrawAxisPos( HSFalse );
			}
		}

		if ( mIsShowAngleLine )
		{
			CStaticCrackPosition::OutLinePos tAngleLinePos;
			if( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_POINTER && mStaticRelation.PosWithPoint( tPoint, tRect, tAngleLinePos.XPos, tAngleLinePos.YPos ) )
			{
				mStaticRelation.SetDrawAngleLine( HSTrue, &tAngleLinePos );
			}
			else
			{
				mStaticRelation.SetDrawAngleLine( HSFalse );
			}
		}

		mStaticRelation.Invalidate();
	}		
}

HSVoid CCrackPosition::AutoSetSensors()
{
	HSDouble tCurLongitude = mCenterXCoord;
	HSDouble tCurLatitude = mCenterYCoord;	
	
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid/* && pIterator->first != mLastSensorSetGPS*/ )
		{
			PosWithGPSInfo( tCurLongitude, tCurLatitude, pIterator->second.Angle, pIterator->second.Radius, pIterator->second.XPos, pIterator->second.YPos );
		}
		pIterator++;
	}	

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
	pMainFrame->SetFocusedGraphic( this, HSTrue );
}

HSVoid CCrackPosition::SwitchCheckCircle()
{
	mCheckCircleEnable = !mCheckCircleEnable;
	mStaticRelation.SetCheckRadius( this->mCheckRadius, mCheckCircleEnable );
	mStaticRelation.SetCheckPos( 0, 0 );
	mStaticRelation.Invalidate();
}

HSVoid CCrackPosition::SwitchOutLine()
{
	mIsShowOutLine = !mIsShowOutLine;
	if ( mIsShowOutLine )
	{
		list< CGraham::GPoint > tGPoints;
		list< CStaticCrackPosition::HitPosition >::iterator pHitIterator = mHitsPosition.begin();
		while ( pHitIterator != mHitsPosition.end() )
		{
			CGraham::GPoint tPoint = { pHitIterator->XPos, pHitIterator->YPos, 0 };
			tGPoints.push_back( tPoint );

			pHitIterator++;
		}

		CGraham tGraham;
		tGraham.Cal( &tGPoints );

		mOutLinePoints.clear();
		list< CGraham::GPoint >::iterator pIterator = tGPoints.begin();
		while ( pIterator != tGPoints.end() )
		{
			CStaticCrackPosition::OutLinePos tPos = { pIterator->X, pIterator->Y };
			mOutLinePoints.push_back( tPos );

			pIterator++;
		}
	}

	mStaticRelation.SetShowOutLine( mIsShowOutLine );
	mStaticRelation.Invalidate();
}

HSVoid CCrackPosition::SwitchCheckAngle()
{
	mIsShowAngleLine = !mIsShowAngleLine;
	if ( !mIsShowAngleLine )
	{
		mStaticRelation.SetDrawAngleLine( mIsShowAngleLine );
		mStaticRelation.Invalidate();
	}
}