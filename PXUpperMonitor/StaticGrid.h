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
	HSVoid SetMinXValue( HSDouble tValue );
	virtual HSVoid SetXValue( HSDouble tTotalValue, HSDouble tBeginValue );		

	HSDouble BeginYValue(){ return mBeginYValue; }
	HSDouble TotalYValue(){ return mTotalYValue; }		
	HSVoid SetMinYValue( HSDouble tValue );
	virtual HSVoid SetYValue( HSDouble tTotalValue, HSDouble tBeginValue );
	
	HSVoid SetCheckRadius( HSDouble tCheckRadius, HSBool tEnable ){ mCheckRadius = tCheckRadius; mCheckRadiusEnabled = tEnable; }
	HSVoid SetCheckPos( HSDouble tX, HSDouble tY ){ mCheckPosX = tX; mCheckPosY = tY; }
	HSDouble XCheckPos(){ return mCheckPosX; }
	HSDouble YCheckPos(){ return mCheckPosY; }

	HSBool PositionWithinCheckCircle( CPoint tPos, CRect &tRect );

	HSBool PosWithPoint( CPoint tPoint, CRect &tRect, HSDouble &tXValue, HSDouble &tYValue );

protected:		
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );	
	HSVoid DrawGrid( CRect &tRect );

	HSLong XDigitWithValue( HSDouble tValue, CRect &tRect, HSBool tCorrectValue = HSTrue );	
	HSLong YDigitWithValue( HSDouble tValue, CRect &tRect, HSBool tCorrectValue = HSTrue );
	HSLong CorrectXDigit( HSLong tDigit, CRect &tRect );
	HSLong CorrectYDigit( HSLong tDigit, CRect &tRect );	

	HSBool IsPointValid( CPoint tPoint, CRect &tRect );

	HSVoid DrawCheckCircle( CDC *pDC, CRect &tRect );
	HSVoid DrawCircle( CDC *pDC, CRect &tRect, HSDouble tLeft, HSDouble tTop, HSDouble tRight, HSDouble tBottom );
	
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


