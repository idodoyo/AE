// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "StaticPlanePosition.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;


// CStaticPlanePosition

CStaticPlanePosition::CStaticPlanePosition()
{			
	mBeginYValue = 1;
	mTotalYValue = 1;	
	mYDirection = -1;
	mMinYValue = 0.01;	

	mMinXValue = 0.01;
	mTotalXValue = 2;

	mType = "[平面定位] 尺寸(米)";
	mTitle = "";

	mMaterialLength = 1;
	mMaterialWidth = 1;	
	mHitsPosition = NULL;
	mFocusSensor = -1;
	mSensors = NULL;

	mAutoDrawGrid = HSFalse;

	mIndexFont.CreatePointFont( 73, "Arial" );	

	Y_SQUARE_LENGTH = 100;
	X_SQUARE_LENGTH = 100;

	mHitRadius = 3;
}

CStaticPlanePosition::~CStaticPlanePosition()
{	
}

// CStaticPlanePosition message handlers
HSVoid CStaticPlanePosition::SetMaterialLength( HSDouble tLength )
{
	mMaterialLength = tLength / 1000.0;	
	ResizeWave();
}

HSVoid CStaticPlanePosition::SetMaterialWidth( HSDouble tWidth )
{
	mMaterialWidth = tWidth / 1000.0;	
	ResizeWave();
}

HSVoid CStaticPlanePosition::SetFocusSensor( HSInt tFocusSensor )
{
	mFocusSensor = tFocusSensor; 
	ResizeWave();
}

HSVoid CStaticPlanePosition::SetSensors( vector< SensorInfo > *pSensors )
{
	mSensors = pSensors;
	ResizeWave();
}

HSVoid CStaticPlanePosition::SetHitsPosition( list< HitPosition > *pHitsPosition )
{ 
	mHitsPosition = pHitsPosition; 
	ResizeWave();
}

HSInt CStaticPlanePosition::SensorInPosition( CPoint tPos, CRect &tRect )
{		
	for ( HSUInt i = 0; i < mSensors->size(); i++ )
	{
		CPoint tPoint( XDigitWithValue( ( *mSensors )[ i ].XPos / 1000.0, tRect, HSFalse ), YDigitWithValue( ( *mSensors )[ i ].YPos / 1000.0, tRect, HSFalse ) );
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

void CStaticPlanePosition::ResizeWave()
{
	WaitForSingleObject( mMutex, INFINITE );

	CStaticGrid::ResizeWave();	

	SetEvent( mMutex );
}

HSVoid CStaticPlanePosition::SetYValue( HSDouble tTotalValue, HSDouble tBeginValue )
{
	CStaticGrid::SetYValue( tTotalValue, tBeginValue );

	char tBuf[ 64 ];
	sprintf_s( tBuf, ( LPCSTR )mYFormat, this->mBeginYValue );	
	int tBeginValueLength = strlen( tBuf );
	sprintf_s( tBuf, ( LPCSTR )mYFormat, mBeginYValue + mTotalYValue * mYDirection );	
	int tEndValueLength = strlen( tBuf );
	mLeftDelta = max( 3 + max( tBeginValueLength, tEndValueLength ) * 9, 30 );
}

void CStaticPlanePosition::Draw( CDC *pDC )
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

bool CStaticPlanePosition::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	WaitForSingleObject( mMutex, INFINITE );

	if ( CStaticGrid::DrawTemplate( pMemDC, tRect ) )		
	{			
		CPoint tMaterialTop;
		CPoint tMaterialBottom;

		HSLong tTopX = XDigitWithValue( 0, tRect, HSFalse );		
		HSLong tTopY = YDigitWithValue( mMaterialLength, tRect, HSFalse );
		HSLong tBottomX  = XDigitWithValue( mMaterialWidth, tRect, HSFalse );
		HSLong tBottomY  = YDigitWithValue( 0, tRect, HSFalse );
		
		tMaterialTop.x = CorrectXDigit( tTopX, tRect );
		tMaterialTop.y = CorrectYDigit( tTopY, tRect );
		tMaterialBottom.x = CorrectXDigit( tBottomX, tRect );
		tMaterialBottom.y = CorrectYDigit( tBottomY, tRect );

		CRect tMaterialRect( tMaterialTop, tMaterialBottom );
		CBrush tBrush( RGB( 200, 200, 200 ) );
		mTemplateDC->FillRect( &tMaterialRect, &tBrush );

		
		HSLong tDelta = 10;		
		tMaterialTop.x = CorrectXDigit( tTopX + tDelta, tRect );
		tMaterialTop.y = CorrectYDigit( tTopY + tDelta, tRect );
		
		tMaterialBottom.x = CorrectXDigit( tBottomX - tDelta, tRect );
		tMaterialBottom.y = CorrectYDigit( tBottomY - tDelta, tRect );

		CRect tInlineMaterialRect( tMaterialTop, tMaterialBottom );
		CBrush tInlineBrush( RGB( 220, 220, 220 ) );
		mTemplateDC->FillRect( &tInlineMaterialRect, &tInlineBrush );
		
		DrawGrid( tRect );

		for ( HSUInt i = 0; i < mSensors->size(); i++ )
		{
			CPoint tPoint( XDigitWithValue( ( *mSensors )[ i ].XPos / 1000.0, tRect, HSFalse ), YDigitWithValue( ( *mSensors )[ i ].YPos / 1000.0, tRect, HSFalse ) );
			if ( IsPointValid( tPoint, tRect ) )
			{						
				HSInt tMargin = 7;
				CPoint tBottomPoint( CorrectXDigit( tPoint.x + tMargin, tRect ), CorrectYDigit( tPoint.y + tMargin, tRect ) );
				CPoint tTopPoint( CorrectXDigit( tPoint.x - tMargin, tRect ), CorrectYDigit( tPoint.y - tMargin, tRect ) );
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
	}

	pMemDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mTemplateDC, 0, 0, SRCCOPY );

	SetEvent( mMutex );

	return true;
}

void CStaticPlanePosition::DrawWave( CDC *pMemDC, CRect &tRect )
{	
	list< HitPosition >::iterator pIterator = mHitsPosition->begin();
	while ( pIterator != mHitsPosition->end() )
	{
		CPoint tPoint( XDigitWithValue( pIterator->XPos / 1000.0, tRect, HSFalse ), YDigitWithValue( pIterator->YPos / 1000.0, tRect, HSFalse ) );
		CPoint tBottomPoint( tPoint.x + mHitRadius, tPoint.y + mHitRadius );
		CPoint tTopPoint( tPoint.x - mHitRadius, tPoint.y - mHitRadius );
		if ( IsPointValid( tPoint, tRect ) )
		{							
			CRect tRectangleRect( tTopPoint, tBottomPoint );

			CBrush tBrush( pIterator->Color );
			pMemDC->FillRect( &tRectangleRect, &tBrush );	
		}

		pIterator++;
	}		

	DrawCheckCircle( pMemDC, tRect );

	DrawFrame( pMemDC, tRect );
}