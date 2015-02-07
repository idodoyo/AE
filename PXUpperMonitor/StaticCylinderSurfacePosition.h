#pragma once

#include "DataHandler.h"
#include "StaticGrid.h"
#include "GL\glut.h"

// CStaticGraphic

class CStaticCylinderSurfacePosition : public CStaticGrid
{	
public:
	CStaticCylinderSurfacePosition();
	virtual ~CStaticCylinderSurfacePosition();	

public:	
	void ResizeWave();		

private:	
	void Draw( CDC *pDC );
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );	
	
};


