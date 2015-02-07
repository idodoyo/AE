#pragma once

#include "DataHandler.h"
#include "StaticGrid.h"

// CStaticGraphic

class CStaticPlanePosition : public CStaticGrid
{	
public:
	CStaticPlanePosition();
	virtual ~CStaticPlanePosition();

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
		COLORREF Color;
	} HitPosition;

public:	
	void ResizeWave();
	HSVoid SetMaterialLength( HSDouble tLength );
	HSVoid SetMaterialWidth( HSDouble tWidth );
	HSVoid SetFocusSensor( HSInt tFocusSensor );
	HSVoid SetSensors( vector< SensorInfo > *pSensors );
	HSVoid SetHitsPosition( list< HitPosition > *pHitsPosition );

	HSInt SensorInPosition( CPoint tPos, CRect &tRect );

	HSVoid SetYValue( HSDouble tTotalValue, HSDouble tBeginValue );

	HSVoid SetHitRadius( HSInt tHitRadius ){ mHitRadius = tHitRadius; }

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

	CFont mIndexFont;

	HSInt mHitRadius;	
};


