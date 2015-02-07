#pragma once

#include "DataHandler.h"
#include "StaticGrid.h"

// CStaticGraphic

class CStaticLinearPosition : public CStaticGrid
{	
public:
	CStaticLinearPosition();
	virtual ~CStaticLinearPosition();

public:
	typedef struct SensorInfo
	{
		HSDouble Pos;
		HSInt Index;
	} SensorInfo;

	typedef struct HitPosition
	{
		HSDouble XPos;
		COLORREF Color;
	} HitPosition;

public:	
	void ResizeWave();
	HSVoid SetMaterialLength( HSDouble tLength );
	HSVoid SetFocusSensor( HSInt tFocusSensor );
	HSVoid SetSensors( vector< SensorInfo > *pSensors );
	HSVoid SetHitsPosition( list< HitPosition > *pHitsPosition );

	HSInt SensorInPosition( CPoint tPos, CRect &tRect );

	HSVoid SetHitRadius( HSInt tHitRadius ){ mHitRadius = tHitRadius; }

private:	
	void Draw( CDC *pDC );
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );
	void DrawWave( CDC *pMemDC, CRect &tRect );	

private:		
	HSDouble mMaterialLength;
	HSInt mFocusSensor;

	list< HitPosition > *mHitsPosition;
	vector< SensorInfo > *mSensors;

	CFont mIndexFont;

	HSInt mHitRadius;
};


