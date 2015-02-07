// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "StaticLinearPosition.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;


// CStaticLinearPosition

CStaticLinearPosition::CStaticLinearPosition()
{			
	mBeginYValue = 5;
	mTotalYValue = 10;	
	mYDirection = -1;
	mMinYValue = 10;

	mShowYAxis = HSFalse;

	mMinXValue = 0.01;
	mTotalXValue = 2;

	mType = "[线性定位] 宽度(米)";
	mTitle = "";

	mMaterialLength = 1;
	mHitsPosition = NULL;
	mFocusSensor = -1;
	mSensors = NULL;

	mAutoDrawGrid = HSFalse;

	mIndexFont.CreatePointFont( 73, "Arial" );

	X_SQUARE_LENGTH = 100;

	mHitRadius = 3;
}

CStaticLinearPosition::~CStaticLinearPosition()
{	
}

// CStaticLinearPosition message handlers
HSVoid CStaticLinearPosition::SetMaterialLength( HSDouble tLength )
{
	mMaterialLength = tLength / 1000.0;	
	ResizeWave();
}

HSVoid CStaticLinearPosition::SetFocusSensor( HSInt tFocusSensor )
{
	mFocusSensor = tFocusSensor; 
	ResizeWave();
}

HSVoid CStaticLinearPosition::SetSensors( vector< SensorInfo > *pSensors )
{
	mSensors = pSensors;
	ResizeWave();
}

HSVoid CStaticLinearPosition::SetHitsPosition( list< HitPosition > *pHitsPosition )
{ 
	mHitsPosition = pHitsPosition; 
	ResizeWave();
}

HSInt CStaticLinearPosition::SensorInPosition( CPoint tPos, CRect &tRect )
{	
	for ( HSUInt i = 0; i < mSensors->size(); i++ )
	{
		CPoint tPoint( XDigitWithValue( ( *mSensors )[ i ].Pos / 1000.0, tRect, HSFalse ), YDigitWithValue( 0, tRect, HSFalse ) );
		if ( IsPointValid( tPoint, tRect ) )
		{						
			HSInt tMargin = 7;
			CPoint tBottomPoint( CorrectXDigit( tPoint.x + tMargin, tRect ), CorrectYDigit( tPoint.y + tMargin, tRect ) );
			CPoint tTopPoint( CorrectXDigit( tPoint.x - tMargin, tRect ), CorrectYDigit( tPoint.y - tMargin, tRect ) );
			CRect tRectangleRect( tTopPoint, tBottomPoint );

			if ( tPos.x > tRectangleRect.left && tPos.x < tRectangleRect.right && tPos.y > tRectangleRect.top && tPos.y < tRectangleRect.bottom )
			{
				return ( *mSensors )[ i ].Index;
			}
		}
	}

	return -1;
}

void CStaticLinearPosition::ResizeWave()
{
	WaitForSingleObject( mMutex, INFINITE );

	CStaticGrid::ResizeWave();	

	SetEvent( mMutex );
}

void CStaticLinearPosition::Draw( CDC *pDC )
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

bool CStaticLinearPosition::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	WaitForSingleObject( mMutex, INFINITE );

	if ( CStaticGrid::DrawTemplate( pMemDC, tRect ) )		
	{			
		CPoint tMaterialTop;
		CPoint tMaterialBottom;	

		HSDouble tMaterialHeight = 1.0;
		tMaterialTop.x = XDigitWithValue( 0, tRect );
		tMaterialTop.y = YDigitWithValue( -tMaterialHeight, tRect );

		tMaterialBottom.x = XDigitWithValue( mMaterialLength, tRect );
		tMaterialBottom.y = YDigitWithValue( tMaterialHeight, tRect );

		CRect tMaterialRect( tMaterialTop, tMaterialBottom );
		CBrush tBrush( RGB( 200, 200, 200 ) );
		mTemplateDC->FillRect( &tMaterialRect, &tBrush );


		HSDouble tDelta = 0.1;
		tMaterialTop.x++;
		tMaterialTop.y = YDigitWithValue( -tMaterialHeight + tDelta, tRect );

		tMaterialBottom.x--;
		tMaterialBottom.y = YDigitWithValue( tMaterialHeight - tDelta, tRect );

		CRect tInlineMaterialRect( tMaterialTop, tMaterialBottom );
		CBrush tInlineBrush( RGB( 220, 220, 220 ) );
		mTemplateDC->FillRect( &tInlineMaterialRect, &tInlineBrush );		
		
		DrawGrid( tRect );

		for ( HSUInt i = 0; i < mSensors->size(); i++ )
		{
			CPoint tPoint( XDigitWithValue( ( *mSensors )[ i ].Pos / 1000.0, tRect, HSFalse ), YDigitWithValue( 0, tRect, HSFalse ) );

			HSInt tMargin = 7;

			CPoint tBottomPoint( tPoint.x + tMargin, tPoint.y + tMargin );
			CPoint tTopPoint( tPoint.x - tMargin, tPoint.y - tMargin );
			CRect tRectangleRect( tTopPoint, tBottomPoint );

			CBrush tBrush( ( *mSensors )[ i ].Index == mFocusSensor ?  RGB( 0, 0, 240 ) : RGB( 0, 0, 0 ) );				
			mTemplateDC->FillRect( &tRectangleRect, &tBrush );	

			HSInt tDelta = 1;
			tBottomPoint.x -= tDelta;
			tBottomPoint.y -= tDelta;
			tTopPoint.x += tDelta;
			tTopPoint.y += tDelta;
			tRectangleRect = CRect( tTopPoint, tBottomPoint );

			CBrush tInlineBrush( RGB( 180, 180, 180 ) );
			mTemplateDC->FillRect( &tRectangleRect, &tInlineBrush );

			CString tStrIndex;
			tStrIndex.Format( "%d", ( *mSensors )[ i ].Index + 1 );

			CFont *tOldFont = mTemplateDC->SelectObject( &mIndexFont );
			mTemplateDC->DrawText( tStrIndex, &tRectangleRect, DT_CENTER | DT_VCENTER );
			mTemplateDC->SelectObject( tOldFont );			
		}		
	}

	pMemDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mTemplateDC, 0, 0, SRCCOPY );

	SetEvent( mMutex );

	return true;
}

void CStaticLinearPosition::DrawWave( CDC *pMemDC, CRect &tRect )
{	
	list< HitPosition >::iterator pIterator = mHitsPosition->begin();
	while ( pIterator != mHitsPosition->end() )
	{
		CPoint tPoint( XDigitWithValue( pIterator->XPos / 1000.0, tRect, HSFalse ), YDigitWithValue( 0, tRect, HSFalse ) );
		CPoint tBottomPoint( tPoint.x + mHitRadius, tPoint.y + mHitRadius );
		CPoint tTopPoint( tPoint.x - mHitRadius, tPoint.y - mHitRadius );
							
		CRect tRectangleRect( tTopPoint, tBottomPoint );

		CBrush tBrush( pIterator->Color );
		pMemDC->FillRect( &tRectangleRect, &tBrush );		

		pIterator++;
	}	

	DrawCheckCircle( pMemDC, tRect );
	
	DrawFrameRect( pMemDC, tRect );
}