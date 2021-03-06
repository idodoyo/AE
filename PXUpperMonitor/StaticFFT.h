#pragma once

#include "DataHandler.h"
#include "StaticGrid.h"

// CStaticGraphic

class CStaticFFT : public CStaticGrid
{	
public:
	CStaticFFT();
	virtual ~CStaticFFT();

public:	
	void ResizeWave();	
	
	HSVoid SetYValue( HSDouble tTotalValue, HSDouble tBeginValue );	
	HSVoid SetData( HSDouble *pHz, HSDouble *pDB, HSInt tLength );

private:	
	void Draw( CDC *pDC );
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );
	void DrawWave( CDC *pMemDC, CRect &tRect );	

private:
	HSDouble *mXHz;
	HSDouble *mYdB;
	HSInt mLength;

	CPoint *mPoints;
};


