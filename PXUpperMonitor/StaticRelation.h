#pragma once

#include "DataHandler.h"
#include "StaticGrid.h"

// CStaticGraphic

class CStaticRelation : public CStaticGrid
{	
public:
	CStaticRelation();
	virtual ~CStaticRelation();

public:	
	void ResizeWave();
	void SetPeriodValue( map< HSInt, HSDouble > *pPeriodValues, HSDouble tPeriod );
	void SetFillRectangle( HSBool tFillRectangle ){ mFillRectangle = tFillRectangle; }
	void SetBeginPeriod( HSDouble tPeriod ){ mBeginPeriod = tPeriod; }

	HSVoid SetYValue( HSDouble tTotalValue, HSDouble tBeginValue );	

	HSVoid SetIsValueAdded( HSBool tValue ){ mIsValueAdded = tValue; }

private:	
	void Draw( CDC *pDC );
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );
	void DrawWave( CDC *pMemDC, CRect &tRect );

	HSDouble AccumulateValue( HSInt tIndex );

private:
	map< HSInt, HSDouble > *mPeriodValues;
	HSDouble mPeriod;
	HSDouble mBeginPeriod;
	HSBool mFillRectangle;

	HSBool mIsValueAdded;

	CPoint *mPoints;
	HSInt mPointLength;
};


