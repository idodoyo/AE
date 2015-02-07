// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "StaticCylinderPosition.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"
#include "GL\glut.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;

// CStaticCylinderPosition

CStaticCylinderPosition::CStaticCylinderPosition()
{			
	mBeginYValue = 1000;
	mTotalYValue = 1000;	
	mYDirection = -1;
	mMinYValue = 10;	

	mMinXValue = 10;
	mTotalXValue = 1000;

	mType = "[方体定位] 尺寸(米)";
	mTitle = "";	

	mAutoDrawGrid = HSFalse;
}

CStaticCylinderPosition::~CStaticCylinderPosition()
{		  
}

// CStaticCylinderPosition message handlers

void CStaticCylinderPosition::ResizeWave()
{
	WaitForSingleObject( mMutex, INFINITE );	

	CStaticGrid::ResizeWave();

	SetEvent( mMutex );
}

void CStaticCylinderPosition::Draw( CDC *pDC )
{
	CRect tRect;
	this->GetClientRect( &tRect );

	CDC tMemDC;
	tMemDC.CreateCompatibleDC( pDC );
	CBitmap tBitmap;
	tBitmap.CreateCompatibleBitmap( pDC, tRect.Width(), tRect.Height() );
	tMemDC.SelectObject( &tBitmap );	
	
	DrawTemplate( &tMemDC, tRect );	

	pDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), &tMemDC, 0, 0, SRCCOPY );
}

bool CStaticCylinderPosition::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	WaitForSingleObject( mMutex, INFINITE );

	if ( CStaticGrid::DrawTemplate( pMemDC, tRect ) )
	{
		
	}

	pMemDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mTemplateDC, 0, 0, SRCCOPY );

	SetEvent( mMutex );

	return true;
}	
