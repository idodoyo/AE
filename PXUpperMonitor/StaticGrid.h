#pragma once

#include "DataHandler.h"
#include "StaticTitle.h"

// CStaticGrid

class CStaticGrid : public CStaticTitle
{	
public:
	CStaticGrid();
	virtual ~CStaticGrid();

public:	
	HSDouble BeginXValue(){ return mBeginXValue; }
	HSDouble TotalXValue(){ return mTotalXValue; }	
	HSVoid SetXValue( HSDouble tTotalValue, HSDouble tBeginValue );		
	HSVoid SetMinXValue( HSDouble tValue );

	HSDouble BeginYValue(){ return mBeginYValue; }
	HSDouble TotalYValue(){ return mTotalYValue; }	
	HSVoid SetYValue( HSDouble tTotalValue, HSDouble tBeginValue );		
	HSVoid SetMinYValue( HSDouble tValue );
	
	HSVoid SetCheckRadius( HSDouble tCheckRadius, HSBool tEnable ){ mCheckRadius = tCheckRadius; mCheckRadiusEnabled = tEnable; }
	HSVoid SetCheckPos( HSDouble tX, HSDouble tY ){ mCheckPosX = tX; mCheckPosY = tY; }
	HSDouble XCheckPos(){ return mCheckPosX; }
	HSDouble YCheckPos(){ return mCheckPosY; }

	HSBool PositionWithinCheckCircle( CPoint tPos, CRect &tRect );

protected:		
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );	
	HSVoid DrawGrid( CRect &tRect );

	HSLong XDigitWithValue( HSDouble tValue, CRect &tRect, HSBool tCorrectValue = HSTrue );	
	HSLong YDigitWithValue( HSDouble tValue, CRect &tRect, HSBool tCorrectValue = HSTrue );
	HSLong CorrectXDigit( HSLong tDigit, CRect &tRect );
	HSLong CorrectYDigit( HSLong tDigit, CRect &tRect );

	HSBool IsPointValid( CPoint tPoint, CRect &tRect );

	HSVoid DrawCheckCircle( CDC *pDC, CRect &tRect );
	
private:	
	HSDouble GetCorrectValue( HSDouble tValue, CString &tPreciseFormat );

protected:
	HSDouble mXSepValue;
	HSDouble mBeginXValue;	
	HSDouble mTotalXValue;
	HSInt mXDirection;
	CString mXFormat;
	HSDouble mMinXValue;
	HSBool mShowXAxis;

	HSDouble mYSepValue;
	HSDouble mBeginYValue;
	HSDouble mTotalYValue;	
	HSInt mYDirection;
	CString mYFormat;
	HSDouble mMinYValue;
	HSBool mShowYAxis;

	HSBool mAutoDrawGrid;

	HSInt Y_SQUARE_LENGTH;
	HSInt X_SQUARE_LENGTH;

	HSDouble mCheckRadius;
	HSBool mCheckRadiusEnabled;	
	HSDouble mCheckPosX;
	HSDouble mCheckPosY;
};


