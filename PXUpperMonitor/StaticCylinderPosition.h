#pragma once

#include "DataHandler.h"
#include "StaticGrid.h"
#include "GL\glut.h"

// CStaticGraphic

class CStaticCylinderPosition : public CStaticGrid
{	
public:
	CStaticCylinderPosition();
	virtual ~CStaticCylinderPosition();	

public:	
	void ResizeWave();		

private:	
	void Draw( CDC *pDC );
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );	
	
};


