// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "StaticScatter.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;


// CStaticScatter

CStaticScatter::CStaticScatter()
{
	mTotalYValue = 20;
	mYSepValue = 2.0;
	mYDirection = -1;

	mIsValueAdded = HSFalse;

	mArgsValue = NULL;

	mType = "[É¢µãÍ¼] ";
}

CStaticScatter::~CStaticScatter()
{		
}

// CStaticScatter message handlers


void CStaticScatter::ResizeWave()
{
	WaitForSingleObject( mMutex, INFINITE );

	CStaticGrid::ResizeWave();	

	SetEvent( mMutex );
}

HSVoid CStaticScatter::SetYValue( HSDouble tTotalValue, HSDouble tBeginValue )
{
	CStaticGrid::SetYValue( tTotalValue, tBeginValue );

	char tBuf[ 64 ];
	sprintf_s( tBuf, ( LPCSTR )mYFormat, this->mBeginYValue );	
	int tBeginValueLength = strlen( tBuf );
	sprintf_s( tBuf, ( LPCSTR )mYFormat, mBeginYValue + mTotalYValue * mYDirection );	
	int tEndValueLength = strlen( tBuf );
	mLeftDelta = max( 3 + max( tBeginValueLength, tEndValueLength ) * 9, 30 );
}

void CStaticScatter::Draw( CDC *pDC )
{
	CRect tRect;
	this->GetClientRect( &tRect );

	CDC tMemDC;
	tMemDC.CreateCompatibleDC( pDC );
	CBitmap tBitmap;
	tBitmap.CreateCompatibleBitmap( pDC, tRect.Width(), tRect.Height() );
	tMemDC.SelectObject( &tBitmap );	
	
	DrawTemplate( &tMemDC, tRect );
	DrawWave( &tMemDC, tRect );

	pDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), &tMemDC, 0, 0, SRCCOPY );
}

bool CStaticScatter::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	WaitForSingleObject( mMutex, INFINITE );

	if ( CStaticGrid::DrawTemplate( pMemDC, tRect ) )		
	{			
	}

	pMemDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mTemplateDC, 0, 0, SRCCOPY );

	SetEvent( mMutex );

	return true;
}

void CStaticScatter::DrawWave( CDC *pMemDC, CRect &tRect )
{			
	list< ArgValueInfo >::iterator pItemIterator = mArgsValue->begin();
	while ( pItemIterator != mArgsValue->end() )	
	{		
		HSDouble tValue = pItemIterator->YValue;
		if ( mIsValueAdded )
		{
			tValue += AccumulateValue( pItemIterator );			
		}

		CPoint tPoint( XDigitWithValue( pItemIterator->XValue, tRect, HSFalse ), YDigitWithValue( tValue, tRect, HSFalse ) );
		CPoint tBottomPoint( tPoint.x + 3, tPoint.y + 3 );
		CPoint tTopPoint( tPoint.x - 3, tPoint.y - 3 );
							
		CRect tRectangleRect( tTopPoint, tBottomPoint );
		CBrush tBrush( RGB( 240, 0, 0 ) );
		pMemDC->FillRect( &tRectangleRect, &tBrush );
				
		pItemIterator++;
	}		
	
	DrawFrameRect( pMemDC, tRect );
}

HSDouble CStaticScatter::AccumulateValue( list< ArgValueInfo >::iterator pDestIterator )
{	
	HSDouble tValue = 0;
	list< ArgValueInfo >::iterator pItemIterator = mArgsValue->begin();
	while ( pItemIterator != mArgsValue->end() && &( *pItemIterator ) != &( *pDestIterator ) )	
	{	
		tValue += pItemIterator->YValue;

		pItemIterator++;
	}
	
	return tValue;
}