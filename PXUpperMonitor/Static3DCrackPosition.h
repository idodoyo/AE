#pragma once

#include "DataHandler.h"
#include "StaticGrid.h"
#include "GL\glut.h"

// CStaticGraphic

class CStatic3DCrackPosition : public CStaticGrid
{	
public:
	CStatic3DCrackPosition();
	virtual ~CStatic3DCrackPosition();	

public:	
	void ResizeWave();		

private:	
	void Draw( CDC *pDC );
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );	
	
};


