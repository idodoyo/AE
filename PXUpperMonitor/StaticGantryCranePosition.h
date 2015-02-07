#pragma once

#include "DataHandler.h"
#include "StaticGrid.h"
#include "GL\glut.h"

class CStaticGantryCranePosition : public CStaticGrid
{	
public:
	CStaticGantryCranePosition();
	virtual ~CStaticGantryCranePosition();	

public:	
	void ResizeWave();		

private:	
	void Draw( CDC *pDC );
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );	
	
};


