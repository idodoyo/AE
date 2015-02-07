#pragma once

#include "DataHandler.h"
#include "StaticGrid.h"

// CStaticGraphic

class CStaticScatter : public CStaticGrid
{	
public:
	CStaticScatter();
	virtual ~CStaticScatter();

public:
	typedef struct ArgValueInfo
	{
		HSDouble XValue;
		HSDouble YValue;
	} ArgValueInfo;

public:	
	void ResizeWave();	
	
	HSVoid SetYValue( HSDouble tTotalValue, HSDouble tBeginValue );	

	HSVoid SetIsValueAdded( HSBool tValue ){ mIsValueAdded = tValue; }

	HSVoid SetArgsValue( list< CStaticScatter::ArgValueInfo > *pArgsValue ){ mArgsValue = pArgsValue; }

private:	
	void Draw( CDC *pDC );
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );
	void DrawWave( CDC *pMemDC, CRect &tRect );

	HSDouble AccumulateValue( list< ArgValueInfo >::iterator pDestIterator );

private:
	HSBool mIsValueAdded;	

	list< ArgValueInfo > *mArgsValue;
};


