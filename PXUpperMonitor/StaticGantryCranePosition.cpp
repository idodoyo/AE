// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "StaticGantryCranePosition.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"
#include "GL\glut.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;

// CStaticGantryCranePosition

CStaticGantryCranePosition::CStaticGantryCranePosition()
{			
	mBeginYValue = 1000;
	mTotalYValue = 1000;	
	mYDirection = -1;
	mMinYValue = 10;	

	mMinXValue = 10;
	mTotalXValue = 1000;

	mType = "[ÁúÃÅµõ¼à²â] ";
	mTitle = "";	

	mAutoDrawGrid = HSFalse;
}

CStaticGantryCranePosition::~CStaticGantryCranePosition()
{		  
}

// CStaticGantryCranePosition message handlers

void CStaticGantryCranePosition::ResizeWave()
{
	WaitForSingleObject( mMutex, INFINITE );	

	CStaticGrid::ResizeWave();

	SetEvent( mMutex );
}

void CStaticGantryCranePosition::Draw( CDC *pDC )
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

bool CStaticGantryCranePosition::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	WaitForSingleObject( mMutex, INFINITE );

	if ( CStaticGrid::DrawTemplate( pMemDC, tRect ) )
	{
		
	}

	pMemDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mTemplateDC, 0, 0, SRCCOPY );

	SetEvent( mMutex );

	return true;
}	
