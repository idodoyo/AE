#pragma once

#include "DataHandler.h"
#include "StaticGrid.h"

// CStaticGraphic

class CStaticBoilerPosition : public CStaticGrid
{	
public:
	CStaticBoilerPosition();
	virtual ~CStaticBoilerPosition();

public:
	typedef struct SensorInfo
	{
		HSDouble XPos;
		HSDouble YPos;
		HSInt Index;
	} SensorInfo;

public:	
	void ResizeWave();
	HSVoid SetMaterialRadius( HSDouble tRadius );	
	HSVoid SetFocusSensor( HSInt tFocusSensor );
	HSVoid SetSensors( vector< SensorInfo > *pSensors );
	HSVoid SetPillarLength( vector< HSDouble > tPillarsLength );
	HSVoid SetPillarArea( HSInt tIndex, HSInt tCount, HSInt tRealCount, COLORREF tColor );
	HSVoid SetFeedMaterialTotalInfo( HSInt tIronGood, HSInt tIronBad, HSInt tCokeGood, HSInt CokeBad );
	HSVoid SetFeedTitleInfo( CString *pTitleValues, HSInt tTotalCount, HSInt tCurIndex );

	HSVoid SetYValue( HSDouble tTotalValue, HSDouble tBeginValue );

private:	
	void Draw( CDC *pDC );
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );
	void DrawWave( CDC *pMemDC, CRect &tRect );

private:			
	HSDouble mMaterialRadius;
	HSInt mFocusSensor;

	vector< SensorInfo > *mSensors;

	CFont mIndexFont;
	CFont mGLTitleFont;
	CFont mContentFont;

	vector< HSDouble > mPillarsLength;

	HSDouble mMaxPillar;

	typedef struct PillarAreaInfo
	{
		CPoint StartPoint;
		HSInt DefaultCount;
		HSInt RealCount;
		COLORREF Color;
	} PillarAreaInfo;

	vector< PillarAreaInfo > mPillarAreaInfo;

	CPoint mDrawLabelStartPoint;
	HSInt mDrawLabelDelta;
	
	CPoint mDrawTotalStartPoint;

	HSInt mPointRectRadius;

	CString mTitleValues[ 6 ];
	CString mTotalValues[ 9 ];

	CString mFeedMaterialTitle;
};


