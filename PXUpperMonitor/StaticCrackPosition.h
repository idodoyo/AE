#pragma once

#include "DataHandler.h"
#include "StaticGrid.h"

// CStaticGraphic

class CStaticCrackPosition : public CStaticGrid
{	
public:
	CStaticCrackPosition();
	virtual ~CStaticCrackPosition();

public:
	typedef struct SensorInfo
	{
		HSDouble XPos;
		HSDouble YPos;
		HSInt Index;
	} SensorInfo;	

	typedef struct HitPosition
	{
		HSDouble XPos;
		HSDouble YPos;
		HSDouble ZPos;
		COLORREF Color;
	} HitPosition;

	typedef struct OutLinePos
	{
		HSDouble XPos;
		HSDouble YPos;		
	} OutLinePos;

public:	
	void ResizeWave();
	HSVoid SetMaterialLength( HSDouble tLength );
	HSVoid SetMaterialWidth( HSDouble tWidth );
	HSVoid SetFocusSensor( HSInt tFocusSensor );
	HSVoid SetSensors( vector< SensorInfo > *pSensors );
	HSVoid SetHitsPosition( list< HitPosition > *pHitsPosition );
	HSVoid SetOutLinePositions( list< OutLinePos > *pOutLinePositions );

	HSInt SensorInPosition( CPoint tPos, CRect &tRect );
	HSVoid SetYValue( HSDouble tTotalValue, HSDouble tBeginValue );
	HSVoid SetHitRadius( HSInt tHitRadius ){ mHitRadius = tHitRadius; }	
	HSVoid SetSensorCircle( HSDouble tFirstCircle, HSDouble tSecondCircle ){ mCircleOneRadius = tFirstCircle; mCircleTueRadius = tSecondCircle; }
	HSVoid SetShowOutLine( HSBool tShowOutLine ){ mShowOutLine = tShowOutLine; }
	HSVoid SetDrawAxisPos( HSBool tDrawAxisPos, OutLinePos *pVerPos = NULL, OutLinePos *pHorPos = NULL );
	HSVoid SetDrawAngleLine( HSBool tDrawAngleLine, OutLinePos *pPos = NULL );

private:	
	void Draw( CDC *pDC );
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );
	void DrawWave( CDC *pMemDC, CRect &tRect );

private:		
	HSDouble mMaterialLength;
	HSDouble mMaterialWidth;
	HSInt mFocusSensor;

	list< HitPosition > *mHitsPosition;
	vector< SensorInfo > *mSensors;
	list< OutLinePos > *mOutLinePositions;

	CFont mIndexFont;
	CFont mLengthFont;

	HSInt mHitRadius;	

	HSDouble mCircleOneRadius;
	HSDouble mCircleTueRadius;

	HSBool mShowOutLine;

	OutLinePos mVerPos[ 2 ];
	OutLinePos mHorPos[ 2 ];
	HSBool mDrawAxisPos;

	HSBool mShowAngleLine;
	OutLinePos mAngleEndPos;
};


