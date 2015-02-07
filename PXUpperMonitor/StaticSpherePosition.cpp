// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "StaticSpherePosition.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"
#include "GL\glut.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;

// CStaticSpherePosition

CStaticSpherePosition::CStaticSpherePosition()
{			
	mBeginYValue = 1000;
	mTotalYValue = 1000;	
	mYDirection = -1;
	mMinYValue = 10;	

	mMinXValue = 10;
	mTotalXValue = 1000;

	mType = "[球面定位] 坐标(极坐标)";
	mTitle = "";	

	mAutoDrawGrid = HSFalse;
}

CStaticSpherePosition::~CStaticSpherePosition()
{		  
}

// CStaticSpherePosition message handlers

void CStaticSpherePosition::ResizeWave()
{
	WaitForSingleObject( mMutex, INFINITE );	

	CStaticGrid::ResizeWave();

	SetEvent( mMutex );
}

void CStaticSpherePosition::Draw( CDC *pDC )
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

bool CStaticSpherePosition::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	WaitForSingleObject( mMutex, INFINITE );

	if ( CStaticGrid::DrawTemplate( pMemDC, tRect ) )
	{
		
	}

	pMemDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mTemplateDC, 0, 0, SRCCOPY );

	SetEvent( mMutex );

	return true;
}	
