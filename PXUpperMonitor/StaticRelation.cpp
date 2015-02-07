// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "StaticRelation.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;


// CStaticRelation

CStaticRelation::CStaticRelation()
{			
	mFillRectangle = HSTrue;
	mPeriodValues = NULL;
	mPointLength = 0;
	mPoints = NULL;
	mBeginPeriod = 0;

	mBeginYValue = 20;
	mTotalYValue = 20;
	mYSepValue = 2.0;
	mYDirection = -1;

	mIsValueAdded = HSFalse;

	mType = "[Öù×´Í¼] ";
}

CStaticRelation::~CStaticRelation()
{	
	if ( mPoints != NULL )
	{
		delete[] mPoints;
	}
}

// CStaticRelation message handlers

void CStaticRelation::SetPeriodValue( map< HSInt, HSDouble > *pPeriodValues, HSDouble tPeriod )
{
	mPeriodValues = pPeriodValues;
	mPeriod = tPeriod; 	

	ResizeWave();
}	

void CStaticRelation::ResizeWave()
{
	WaitForSingleObject( mMutex, INFINITE );

	CStaticGrid::ResizeWave();	

	SetEvent( mMutex );
}

HSVoid CStaticRelation::SetYValue( HSDouble tTotalValue, HSDouble tBeginValue )
{
	CStaticGrid::SetYValue( tTotalValue, tBeginValue );

	char tBuf[ 64 ];
	sprintf_s( tBuf, ( LPCSTR )mYFormat, this->mBeginYValue );	
	int tBeginValueLength = strlen( tBuf );
	sprintf_s( tBuf, ( LPCSTR )mYFormat, mBeginYValue + mTotalYValue * mYDirection );	
	int tEndValueLength = strlen( tBuf );
	mLeftDelta = max( 3 + max( tBeginValueLength, tEndValueLength ) * 9, 30 );
}

void CStaticRelation::Draw( CDC *pDC )
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

bool CStaticRelation::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	WaitForSingleObject( mMutex, INFINITE );

	if ( CStaticGrid::DrawTemplate( pMemDC, tRect ) )		
	{			
	}

	pMemDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mTemplateDC, 0, 0, SRCCOPY );

	SetEvent( mMutex );

	return true;
}

void CStaticRelation::DrawWave( CDC *pMemDC, CRect &tRect )
{	
	HSInt tLength = ( int( mTotalXValue / mPeriod ) + 2 ) * 2;
	if ( mPoints == NULL || tLength != mPointLength )
	{
		if ( mPoints != NULL )
		{
			delete[] mPoints;
		}

		mPoints = new CPoint[ tLength ];
		mPointLength = tLength;
	}

	HSDouble tEndPeriod = 0;
	HSDouble tBeginPeriod = 0;
	map< HSInt, HSDouble >::iterator pIterator = mPeriodValues->end();
	if ( pIterator != mPeriodValues->begin() )
	{
		pIterator--;
		tEndPeriod = ( pIterator->first + 1 ) * mPeriod;
		tBeginPeriod = mPeriodValues->begin()->first * mPeriod;
	}

	HSDouble tPeriod = int( mBeginXValue / mPeriod ) * mPeriod;
	if ( mBeginXValue < 0 )
	{
		tPeriod = int( ( mBeginXValue - mPeriod + 0.0000001 ) / mPeriod ) * mPeriod;
	}	
	
	HSInt tIndex = 0;	
	while ( tPeriod < mBeginXValue + mTotalXValue && tPeriod < tEndPeriod )
	{		
		if ( tPeriod < tBeginPeriod && !mIsValueAdded || tPeriod < 0 )
		{
			tPeriod += mPeriod;
			continue;
		}						

		HSDouble tValue = 0;
		map< HSInt, HSDouble >::iterator pItemIterator = mPeriodValues->find( HSInt( tPeriod / mPeriod ) );
		if ( pItemIterator != mPeriodValues->end() )
		{
			tValue = pItemIterator->second;			
		}

		if ( mIsValueAdded )
		{
			tValue += AccumulateValue( HSInt( tPeriod / mPeriod ) );			
		}	

		mPoints[ tIndex ].x = XDigitWithValue( tPeriod, tRect );
		mPoints[ tIndex + 1 ].x = XDigitWithValue( tPeriod + mPeriod, tRect );
		mPoints[ tIndex ].y = YDigitWithValue( tValue, tRect );
		mPoints[ tIndex + 1 ].y = mPoints[ tIndex ].y;
		
		if ( mPoints[ tIndex ].x < tRect.Width() - mLeftDelta && mPoints[ tIndex + 1 ].x > mLeftDelta )
		{			
			if ( mFillRectangle )
			{
				CPoint tBottomPoint = mPoints[ tIndex + 1 ];
				tBottomPoint.y = YDigitWithValue( 0, tRect ) + 1;
				CRect tRectangleRect( mPoints[ tIndex ], tBottomPoint );

				CBrush tBrush( RGB( 240, 0, 0 ) );
				pMemDC->FillRect( &tRectangleRect, &tBrush );
			}

			tIndex += 2;
		}

		tPeriod += mPeriod;
	}

	CPen tDigitPen( PS_SOLID, 1, RGB( 240, 0, 0 ) );	
	pMemDC->SelectObject( &tDigitPen );
	pMemDC->Polyline( mPoints, tIndex );	

	DrawFrame( pMemDC, tRect );
}

HSDouble CStaticRelation::AccumulateValue( HSInt tIndex )
{
	HSDouble tValue = 0;
	map< HSInt, HSDouble >::iterator pItemIterator = mPeriodValues->begin();
	while ( pItemIterator != mPeriodValues->end() )	
	{
		if ( pItemIterator->first < tIndex )
		{
			tValue += pItemIterator->second;
		}
		else
		{
			break;
		}

		pItemIterator++;
	}

	return tValue;
}