// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "StaticFFT.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;


// CStaticFFT

CStaticFFT::CStaticFFT()
{
	mTotalYValue = 20;
	mYSepValue = 2.0;
	mYDirection = -1;

	mMinXValue = 0.001;
	mMinYValue = 10;

	mType = "[¹¦ÂÊÆ×] ";

	mTitle = "DEVICE < CHANNEL > (dB/MHz)";	

	mXHz = NULL;
	mYdB = NULL;
	mLength = 0;

	mPoints = NULL;
}

CStaticFFT::~CStaticFFT()
{		
	if ( mPoints != NULL )
	{
		delete[] mPoints;
	}

	if ( mYdB != NULL )
	{
		delete[] mYdB;
	}
}

void CStaticFFT::ResizeWave()
{
	WaitForSingleObject( mMutex, INFINITE );

	CStaticGrid::ResizeWave();	

	SetEvent( mMutex );
}

HSVoid CStaticFFT::SetYValue( HSDouble tTotalValue, HSDouble tBeginValue )
{
	CStaticGrid::SetYValue( tTotalValue, tBeginValue );

	char tBuf[ 64 ];
	sprintf_s( tBuf, ( LPCSTR )mYFormat, this->mBeginYValue );
	int tBeginValueLength = strlen( tBuf );
	sprintf_s( tBuf, ( LPCSTR )mYFormat, mBeginYValue + mTotalYValue * mYDirection );
	int tEndValueLength = strlen( tBuf );
	mLeftDelta = max( 3 + max( tBeginValueLength, tEndValueLength ) * 9, 30 );
}

HSVoid CStaticFFT::SetData( HSDouble *pHz, HSDouble *pDB, HSInt tLength )
{
	if ( mLength != tLength && tLength != 0 )
	{
		if ( mPoints != NULL )
		{
			delete[] mPoints;
		}

		mPoints = new CPoint[ tLength ];

		if ( mYdB != NULL )
		{
			delete[] mYdB;
		}

		mYdB = new HSDouble[ tLength ];
	}

	if ( pDB != NULL )
	{
		memcpy( mYdB, pDB, tLength * sizeof( HSDouble ) );
	}

	mXHz = pHz;
	//mYdB = pDB;
	mLength = tLength;
}

void CStaticFFT::Draw( CDC *pDC )
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

bool CStaticFFT::DrawTemplate( CDC *pMemDC, CRect &tRect )
{
	WaitForSingleObject( mMutex, INFINITE );

	if ( CStaticGrid::DrawTemplate( pMemDC, tRect ) )
	{
	}

	pMemDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mTemplateDC, 0, 0, SRCCOPY );

	SetEvent( mMutex );

	return true;
}

void CStaticFFT::DrawWave( CDC *pMemDC, CRect &tRect )
{
	if ( mXHz == NULL || mLength == 0 )
	{
		return;
	}

	for ( HSInt i = 0; i < mLength; i++ )
	{
		mPoints[ i ].x = XDigitWithValue( mXHz[ i ], tRect, HSFalse );
		mPoints[ i ].y = YDigitWithValue( mYdB[ i ], tRect, HSFalse );
	}

	pMemDC->Polyline( mPoints, mLength );

	DrawFrameRect( pMemDC, tRect );
}