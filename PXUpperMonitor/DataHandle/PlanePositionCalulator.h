#pragma once

#include "DataTypes.h"
#include "NewtonIterator.h"
#include "ArgCalculator.h"
#include "stdafx.h"

class IPlanePositionCalulatorProtocol
{
public:
	virtual HSVoid GetSensorPosition( HSInt tSensor, HSDouble &tPosX, HSDouble &tPosY ) = 0;
	virtual HSUInt64 GetSensorHitTime( HSInt tSensor, HSInt tArgIndex ) = 0;
	virtual HSDouble DistanceBetweenSensors( HSInt tSensorOne, HSInt tSensorTwo ) = 0;
	virtual HSDouble DistanceFromSensor( HSDouble tPosX, HSDouble tPosY, HSInt tSensor ) = 0;

	virtual ~IPlanePositionCalulatorProtocol(){}
};

class CPlanePositionCalulator : public INewtonIteratorProtocol< 3 >
{
public:
	typedef struct HIT_POS
	{
		HSDouble PosX;
		HSDouble PosY;

		HIT_POS( HSDouble tPosX = 0, HSDouble tPosY = 0 ){ PosX = tPosX; PosY = tPosY; }
	} HIT_POS;

	typedef struct HIT_CALC_INFO
	{
		HSInt SensorIndex;
		HSUInt ArgIndex;
		IPlanePositionCalulatorProtocol *Calulator;
	} HIT_CALC_INFO;

	typedef struct SENSOR_DIST_INFO
	{
		HSInt SensorIndex;
		HSDouble Distance;		
	} SENSOR_DIST_INFO;

	static bool HitSensorComapre( HIT_CALC_INFO &t1, HIT_CALC_INFO &t2 )
	{
		return t1.Calulator->GetSensorHitTime( t1.SensorIndex, t1.ArgIndex ) < t2.Calulator->GetSensorHitTime( t2.SensorIndex, t2.ArgIndex );
	}

	static bool HitSensorDistanceComapre( SENSOR_DIST_INFO &t1, SENSOR_DIST_INFO &t2 )
	{
		return t1.Distance < t2.Distance;
	}

	HSVoid GetFunctionValue( HSDouble tX0[ 3 ], HSDouble *tPoints[ 3 ], HSDouble tRes[ 3 ] )
	{
		HSDouble *tPoint1 = tPoints[ 0 ];
		HSDouble *tPoint2 = tPoints[ 1 ];
		HSDouble *tPoint3 = tPoints[ 2 ];

		tRes[ 0 ] = pow( tX0[ 0 ] - tPoint1[ 0 ], 2 ) + pow( tX0[ 1 ] - tPoint1[ 1 ], 2 ) - pow( tX0[ 2 ], 2 );
		tRes[ 1 ] = pow( tX0[ 0 ] - tPoint2[ 0 ], 2 ) + pow( tX0[ 1 ] - tPoint2[ 1 ], 2 ) - pow( tX0[ 2 ] + tPoint2[ 2 ], 2 );
		tRes[ 2 ] = pow( tX0[ 0 ] - tPoint3[ 0 ], 2 ) + pow( tX0[ 1 ] - tPoint3[ 1 ], 2 ) - pow( tX0[ 2 ] + tPoint3[ 2 ], 2 );
	}

	HSVoid GetFunctionMValue( HSDouble tX0[ 3 ], HSDouble *tPoints[ 3 ], HSDouble tRes[ 3 ][ 3 ] )
	{
		HSDouble *tPoint1 = tPoints[ 0 ];
		HSDouble *tPoint2 = tPoints[ 1 ];
		HSDouble *tPoint3 = tPoints[ 2 ];

		tRes[ 0 ][ 0 ] = ( tX0[ 0 ] - tPoint1[ 0 ] ) * 2;
		tRes[ 1 ][ 0 ] = ( tX0[ 0 ] - tPoint2[ 0 ] ) * 2;
		tRes[ 2 ][ 0 ] = ( tX0[ 0 ] - tPoint3[ 0 ] ) * 2;

		tRes[ 0 ][ 1 ] = ( tX0[ 1 ] - tPoint1[ 1 ] ) * 2;
		tRes[ 1 ][ 1 ] = ( tX0[ 1 ] - tPoint2[ 1 ] ) * 2;
		tRes[ 2 ][ 1 ] = ( tX0[ 1 ] - tPoint3[ 1 ] ) * 2;

		tRes[ 0 ][ 2 ] = -tX0[ 2 ] * 2;
		tRes[ 1 ][ 2 ] = -( tX0[ 2 ] + tPoint2[ 2 ] ) * 2;
		tRes[ 2 ][ 2 ] = -( tX0[ 2 ] + tPoint3[ 2 ] ) * 2;
	}		

	HSBool PositionWithHitSensor( HSDouble tInitX0[ 3 ], vector< HIT_CALC_INFO > &tHitSensors, vector< HSInt > &tHitSensorsIndex, vector< HIT_POS > &tHitPos, IPlanePositionCalulatorProtocol *pParent )
	{
		HSInt tSensor1Index = tHitSensors[ tHitSensorsIndex[ 0 ] ].SensorIndex;
		HSInt tSensor2Index = tHitSensors[ tHitSensorsIndex[ 1 ] ].SensorIndex;
		HSInt tSensor3Index = tHitSensors[ tHitSensorsIndex[ 2 ] ].SensorIndex;

		HSInt tArg1Index = tHitSensors[ tHitSensorsIndex[ 0 ] ].ArgIndex;
		HSInt tArg2Index = tHitSensors[ tHitSensorsIndex[ 1 ] ].ArgIndex;
		HSInt tArg3Index = tHitSensors[ tHitSensorsIndex[ 2 ] ].ArgIndex;

		HSDouble tVelocity = CArgCalculator::SharedInstance()->GetMaterialVelocity();
		HSDouble tDeltaS1 = ( ( HSInt64 )( pParent->GetSensorHitTime( tSensor2Index, tArg2Index ) - pParent->GetSensorHitTime( tSensor1Index, tArg1Index ) ) ) / 1000000000.0 * tVelocity;
		HSDouble tDeltaS2 = ( ( HSInt64 )( pParent->GetSensorHitTime( tSensor3Index, tArg3Index ) - pParent->GetSensorHitTime( tSensor1Index, tArg1Index ) ) ) / 1000000000.0 * tVelocity;

		HSDouble tPoint1[ 2 ] = { 0 };
		HSDouble tPoint2[ 3 ] = { 0, 0, tDeltaS1 };
		HSDouble tPoint3[ 3 ] = { 0, 0, tDeltaS2 };

		pParent->GetSensorPosition( tSensor1Index, tPoint1[ 0 ], tPoint1[ 1 ] );
		pParent->GetSensorPosition( tSensor2Index, tPoint2[ 0 ], tPoint2[ 1 ] );
		pParent->GetSensorPosition( tSensor3Index, tPoint3[ 0 ], tPoint3[ 1 ] );

		HSDouble tX0[ 3 ] = { tInitX0[ 0 ], tInitX0[ 1 ], tInitX0[ 2 ] };
		HSDouble tX1[ 3 ] = { 0 };

		HSDouble *pPoints[ 3 ] = { tPoint1, tPoint2, tPoint3 };
		if ( !mNewtonIterator.StartIterator( tX0, pPoints, tX1, this ) )
		{
			return HSFalse;
		}

		HIT_POS tPos( tX1[ 0 ], tX1[ 1 ] );
		tHitPos.push_back( tPos );

		return HSTrue;
	}

	HSBool CalcHit( vector< HIT_CALC_INFO > &tHitSensors, vector< HIT_POS > &tHitPos, IPlanePositionCalulatorProtocol *pParent )
	{
		HSInt tXLowIndex = 0;	
		HSInt tXBigIndex = 0;
		HSInt tThirdIndex = 0;

		HIT_POS tSensor;
		pParent->GetSensorPosition( tHitSensors[ tXLowIndex ].SensorIndex, tSensor.PosX, tSensor.PosY );
		HSDouble tLowPos = tSensor.PosX;
		HSDouble tBigPos = tLowPos;

		for ( HSUInt i = 1; i < tHitSensors.size(); i++ )
		{			
			pParent->GetSensorPosition( tHitSensors[ i ].SensorIndex, tSensor.PosX, tSensor.PosY );
			if ( tSensor.PosX > tBigPos )
			{
				tBigPos = tSensor.PosX;
				tXBigIndex = i;
			}

			if  ( tSensor.PosX < tLowPos )
			{
				tLowPos = tSensor.PosX;
				tXLowIndex = i;
			}
		}

		HIT_POS tSensorLow;
		HIT_POS tSensorBig;
		pParent->GetSensorPosition( tHitSensors[ tXLowIndex ].SensorIndex, tSensorLow.PosX, tSensorLow.PosY );
		pParent->GetSensorPosition( tHitSensors[ tXBigIndex ].SensorIndex, tSensorBig.PosX, tSensorBig.PosY );

		HSDouble tS = 0;
		for ( HSUInt i = 0; i < tHitSensors.size(); i++ )
		{
			if ( i != tXLowIndex && i != tXBigIndex )
			{
				pParent->GetSensorPosition( tHitSensors[ i ].SensorIndex, tSensor.PosX, tSensor.PosY );
				HSDouble tYPos = tSensor.PosY;
				HSDouble tHeight = max( abs( tYPos - tSensorLow.PosY ), abs( tYPos - tSensorBig.PosY ) );
				HSDouble tWidth = abs( tSensorBig.PosX - tSensorLow.PosX );

				if ( tHeight * tWidth > tS )
				{
					tS = tHeight * tWidth;
					tThirdIndex = i;
				}
			}
		}		

		HSDouble tX0[ 3 ] = { ( tSensorLow.PosX + tSensorBig.PosX ) / 2.0, ( tSensorLow.PosY + tSensorBig.PosY ) / 2.0, 0 };
		tX0[ 2 ] = pParent->DistanceFromSensor( tX0[ 0 ], tX0[ 1 ], tHitSensors[ tXLowIndex ].SensorIndex );

		vector< HSInt > tHitSensorsIndex;
		tHitSensorsIndex.push_back( tXLowIndex );
		tHitSensorsIndex.push_back( tXBigIndex );
		tHitSensorsIndex.push_back( tThirdIndex );

		if ( PositionWithHitSensor( tX0, tHitSensors, tHitSensorsIndex, tHitPos, pParent ) )
		{
			return HSTrue;
		}

		return HSFalse;
	}

private:
	CNewtonIterator< 3 > mNewtonIterator;
};