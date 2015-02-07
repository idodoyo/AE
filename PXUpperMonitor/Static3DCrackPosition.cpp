// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "Static3DCrackPosition.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"
#include "GL\glut.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;

// CStatic3DCrackPosition

CStatic3DCrackPosition::CStatic3DCrackPosition()
{			
	mBeginYValue = 1000;
	mTotalYValue = 1000;	
	mYDirection = -1;
	mMinYValue = 10;	

	mMinXValue = 10;
	mTotalXValue = 1000;

	mType = "[3DÁÑ·ìÍ¼] ³ß´ç(Ã×)";
	mTitle = "";

	mAutoDrawGrid = HSFalse;
}

CStatic3DCrackPosition::~CStatic3DCrackPosition()
{		  
}

// CStatic3DCrackPosition message handlers

void CStatic3DCrackPosition::ResizeWave()
{
	WaitForSingleObject( mMutex, INFINITE );	

	CStaticGrid::ResizeWave();

	SetEvent( mMutex );
}

void CStatic3DCrackPosition::Draw( CDC *pDC )
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

bool CStatic3DCrackPosition::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	WaitForSingleObject( mMutex, INFINITE );

	if ( CStaticGrid::DrawTemplate( pMemDC, tRect ) )
	{
		
	}

	pMemDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mTemplateDC, 0, 0, SRCCOPY );

	SetEvent( mMutex );

	return true;
}	
