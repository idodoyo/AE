#pragma once

#include "DataHandler.h"
#include "StaticGrid.h"
#include "GL\glut.h"

// CStaticGraphic

class CStaticSpherePosition : public CStaticGrid
{	
public:
	CStaticSpherePosition();
	virtual ~CStaticSpherePosition();	

public:	
	void ResizeWave();		

private:	
	void Draw( CDC *pDC );
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );	
	
};


