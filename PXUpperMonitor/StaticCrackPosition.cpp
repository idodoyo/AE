// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "StaticCrackPosition.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"
#include "GraphicManager.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;


// CStaticCrackPosition

CStaticCrackPosition::CStaticCrackPosition()
{			
	mBeginYValue = 1;
	mTotalYValue = 1;	
	mYDirection = -1;
	mMinYValue = 0.01;	

	mMinXValue = 0.01;
	mTotalXValue = 2;

	mType = "[裂缝定位] 尺寸(米)";
	mTitle = "";

	mMaterialLength = 1;
	mMaterialWidth = 1;	
	mHitsPosition = NULL;
	mFocusSensor = -1;
	mSensors = NULL;
	
	mIndexFont.CreatePointFont( 73, "Arial" );	
	mLengthFont.CreatePointFont( 76, "Arial" );	

	Y_SQUARE_LENGTH = 100;
	X_SQUARE_LENGTH = 100;

	mHitRadius = 3;

	mCircleOneRadius = 40;
	mCircleTueRadius = 80;

	mShowOutLine = HSFalse;
	mDrawAxisPos = HSFalse;

	mShowAngleLine = HSFalse;
}

CStaticCrackPosition::~CStaticCrackPosition()
{	
}

// CStaticCrackPosition message handlers
HSVoid CStaticCrackPosition::SetMaterialLength( HSDouble tLength )
{
	mMaterialLength = tLength / 1000.0;	
	ResizeWave();
}

HSVoid CStaticCrackPosition::SetMaterialWidth( HSDouble tWidth )
{
	mMaterialWidth = tWidth / 1000.0;	
	ResizeWave();
}

HSVoid CStaticCrackPosition::SetFocusSensor( HSInt tFocusSensor )
{
	mFocusSensor = tFocusSensor; 
	ResizeWave();
}

HSVoid CStaticCrackPosition::SetSensors( vector< SensorInfo > *pSensors )
{
	mSensors = pSensors;
	ResizeWave();
}

HSVoid CStaticCrackPosition::SetHitsPosition( list< HitPosition > *pHitsPosition )
{ 
	mHitsPosition = pHitsPosition; 
	ResizeWave();
}

HSVoid CStaticCrackPosition::SetOutLinePositions( list< OutLinePos > *pOutLinePositions )
{
	mOutLinePositions = pOutLinePositions;
	ResizeWave();
}

HSInt CStaticCrackPosition::SensorInPosition( CPoint tPos, CRect &tRect )
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

HSVoid CStaticCrackPosition::SetDrawAxisPos( HSBool tDrawAxisPos, OutLinePos *pVerPos, OutLinePos *pHorPos )
{
	mDrawAxisPos = tDrawAxisPos;
	if ( pVerPos != NULL )
	{
		mVerPos[ 0 ] = pVerPos[ 0 ];
		mVerPos[ 1 ] = pVerPos[ 1 ];

		mHorPos[ 0 ] = pHorPos[ 0 ];
		mHorPos[ 1 ] = pHorPos[ 1 ];
	}
}

HSVoid CStaticCrackPosition::SetDrawAngleLine( HSBool tDrawAngleLine, OutLinePos *pPos )
{
	mShowAngleLine = tDrawAngleLine;
	if ( pPos != NULL )
	{
		mAngleEndPos = *pPos;
	}
}

void CStaticCrackPosition::ResizeWave()
{
	WaitForSingleObject( mMutex, INFINITE );

	CStaticGrid::ResizeWave();	

	SetEvent( mMutex );
}

HSVoid CStaticCrackPosition::SetYValue( HSDouble tTotalValue, HSDouble tBeginValue )
{
	CStaticGrid::SetYValue( tTotalValue, tBeginValue );

	char tBuf[ 64 ];
	sprintf_s( tBuf, ( LPCSTR )mYFormat, this->mBeginYValue );	
	int tBeginValueLength = strlen( tBuf );
	sprintf_s( tBuf, ( LPCSTR )mYFormat, mBeginYValue + mTotalYValue * mYDirection );	
	int tEndValueLength = strlen( tBuf );
	mLeftDelta = max( 3 + max( tBeginValueLength, tEndValueLength ) * 9, 30 );
}

void CStaticCrackPosition::Draw( CDC *pDC )
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

bool CStaticCrackPosition::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	WaitForSingleObject( mMutex, INFINITE );

	if ( CStaticGrid::DrawTemplate( pMemDC, tRect ) )		
	{			
		HSLong tLeft = XDigitWithValue( -mMaterialWidth / 2, tRect, HSFalse );
		HSLong tRight = XDigitWithValue( mMaterialWidth / 2, tRect, HSFalse );
		HSLong tHorCenter = YDigitWithValue( 0, tRect, HSFalse );

		HSLong tTop = YDigitWithValue( mMaterialLength / 2, tRect, HSFalse );
		HSLong tBottom = YDigitWithValue( -mMaterialLength / 2, tRect, HSFalse );
		HSLong tVerCenter = XDigitWithValue( 0, tRect, HSFalse );

		CPen tAxisPen;
		tAxisPen.CreatePen( PS_SOLID | PS_COSMETIC, 1, RGB( 105, 105, 105 ) );
		CPen *pOldPen = mTemplateDC->SelectObject( &tAxisPen );
		mTemplateDC->MoveTo( CPoint( tLeft, tHorCenter ) );
		mTemplateDC->LineTo( CPoint( tRight, tHorCenter ) );

		mTemplateDC->MoveTo( CPoint( tVerCenter - 1, tTop ) );
		mTemplateDC->LineTo( CPoint( tVerCenter - 1, tBottom ) );

		mTemplateDC->SelectObject( pOldPen );

		CPoint tNPoint( XDigitWithValue( 0, tRect, HSFalse ), YDigitWithValue( mMaterialLength / 2, tRect, HSFalse ) );
		CPoint tSPoint( XDigitWithValue( 0, tRect, HSFalse ), YDigitWithValue( -mMaterialLength / 2, tRect, HSFalse ) );
		CPoint tWPoint( XDigitWithValue( -mMaterialWidth / 2, tRect, HSFalse ), YDigitWithValue( 0, tRect, HSFalse ) );
		CPoint tEPoint( XDigitWithValue( mMaterialWidth / 2, tRect, HSFalse ), YDigitWithValue( 0, tRect, HSFalse ) );
			
		CRect tNRect( tNPoint.x - 20, tNPoint.y, tNPoint.x + 20, tNPoint.y + 20 );
		mTemplateDC->DrawText( "N(北)", &tNRect, DT_CENTER | DT_VCENTER );

		CRect tSRect( tSPoint.x - 20, tSPoint.y - 18, tSPoint.x + 20, tSPoint.y );
		mTemplateDC->DrawText( "S(南)", &tSRect, DT_CENTER | DT_VCENTER );

		CRect tWRect( tWPoint.x, tWPoint.y - 8, tWPoint.x + 40, tWPoint.y + 10 );
		mTemplateDC->DrawText( "W(西)", &tWRect, DT_CENTER | DT_VCENTER );

		CRect tERect( tEPoint.x - 35, tEPoint.y - 8, tEPoint.x + 4, tEPoint.y + 10 );
		mTemplateDC->DrawText( "E(东)", &tERect, DT_CENTER | DT_VCENTER );


		CPen tPen;
		tPen.CreatePen( PS_SOLID | PS_COSMETIC, 1, RGB( 105, 105, 105 ) );
		pOldPen = mTemplateDC->SelectObject( &tPen );
		if ( mCircleOneRadius > 0 )
		{
			DrawCircle( mTemplateDC, tRect, -mCircleOneRadius, mCircleOneRadius, mCircleOneRadius, -mCircleOneRadius );
		}

		if ( mCircleTueRadius > 0 )
		{
			DrawCircle( mTemplateDC, tRect, -mCircleTueRadius, mCircleTueRadius, mCircleTueRadius, -mCircleTueRadius );
		}

		mTemplateDC->SelectObject( pOldPen );
	
		for ( HSUInt i = 0; i < mSensors->size(); i++ )
		{
			CPoint tPoint( XDigitWithValue( ( *mSensors )[ i ].XPos / 1000.0, tRect, HSFalse ), YDigitWithValue( ( *mSensors )[ i ].YPos / 1000.0, tRect, HSFalse ) );

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

			CFont *pOldFont = mTemplateDC->SelectObject( &mIndexFont );
			mTemplateDC->DrawText( tStrIndex, &tRectangleRect, DT_CENTER | DT_VCENTER );
			mTemplateDC->SelectObject( pOldFont );
		}		
	}

	pMemDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mTemplateDC, 0, 0, SRCCOPY );

	SetEvent( mMutex );

	return true;
}

void CStaticCrackPosition::DrawWave( CDC *pMemDC, CRect &tRect )
{		
	list< HitPosition >::iterator pIterator = mHitsPosition->begin();
	while ( pIterator != mHitsPosition->end() )
	{
		CPoint tPoint( XDigitWithValue( pIterator->XPos, tRect, HSFalse ), YDigitWithValue( pIterator->YPos, tRect, HSFalse ) );
		CPoint tBottomPoint( tPoint.x + mHitRadius, tPoint.y + mHitRadius );
		CPoint tTopPoint( tPoint.x - mHitRadius, tPoint.y - mHitRadius );		

		CRect tRectangleRect( tTopPoint, tBottomPoint );

		CBrush tBrush( pIterator->Color );
		pMemDC->FillRect( &tRectangleRect, &tBrush );

		pIterator++;
	}

	if ( mShowOutLine && mOutLinePositions->size() > 2 )
	{
		CPoint *pPoints = new CPoint[ mOutLinePositions->size() + 1 ];

		list< OutLinePos >::iterator pLineIterator = mOutLinePositions->begin();
		HSInt tIndex = 0;
		while ( pLineIterator != mOutLinePositions->end() )
		{		
			pPoints[ tIndex ].x = XDigitWithValue( pLineIterator->XPos, tRect, HSFalse );
			pPoints[ tIndex ].y = YDigitWithValue( pLineIterator->YPos, tRect, HSFalse );

			tIndex++;
			pLineIterator++;
		}		

		pPoints[ mOutLinePositions->size() ] = pPoints[ 0 ];

		CPen tPen( PS_SOLID, 1, RGB( 0, 0, 150 ) );
		CPen *pOldPen = pMemDC->SelectObject( &tPen );
		pMemDC->Polyline( pPoints, mOutLinePositions->size() + 1 );
		pMemDC->SelectObject( pOldPen );

		delete[] pPoints;

		if ( mDrawAxisPos )
		{
			CPen tPen( PS_DOT, 1, RGB( 0, 0, 120 ) );
			CPen *pOldPen = pMemDC->SelectObject( &tPen );
			HSInt tOldMode = pMemDC->SetBkMode( TRANSPARENT );

			CPoint tPoints[ 4 ];
			tPoints[ 0 ].x = XDigitWithValue( mVerPos[ 0 ].XPos, tRect, HSFalse );
			tPoints[ 0 ].y = YDigitWithValue( mVerPos[ 0 ].YPos, tRect, HSFalse );
			tPoints[ 1 ].x = XDigitWithValue( mVerPos[ 1 ].XPos, tRect, HSFalse );
			tPoints[ 1 ].y = YDigitWithValue( mVerPos[ 1 ].YPos, tRect, HSFalse );
			pMemDC->Polyline( tPoints, 2 );

			tPoints[ 2 ].x = XDigitWithValue( mHorPos[ 0 ].XPos, tRect, HSFalse );
			tPoints[ 2 ].y = YDigitWithValue( mHorPos[ 0 ].YPos, tRect, HSFalse );
			tPoints[ 3 ].x = XDigitWithValue( mHorPos[ 1 ].XPos, tRect, HSFalse );
			tPoints[ 3 ].y = YDigitWithValue( mHorPos[ 1 ].YPos, tRect, HSFalse );
			pMemDC->Polyline( &tPoints[ 2 ], 2 );

			pMemDC->SelectObject( pOldPen );

			HSInt tCircleRadius = 3;
			for ( HSInt i = 0; i < 4; i++ )
			{				
				pMemDC->Ellipse( tPoints[ i ].x - tCircleRadius, tPoints[ i ].y - tCircleRadius, tPoints[ i ].x + tCircleRadius, tPoints[ i ].y + tCircleRadius );
			}

			CFont *pOldFont = mTemplateDC->SelectObject( &mLengthFont );
						
			CString tStrLength;
			tStrLength.Format( "%.02fm", abs( mVerPos[ 0 ].YPos - mVerPos[ 1 ].YPos ) );

			CPoint tCenterPoint;
			tCenterPoint.x = XDigitWithValue( ( mVerPos[ 0 ].XPos + mVerPos[ 1 ].XPos ) / 2.0, tRect, HSFalse );
			tCenterPoint.y = YDigitWithValue( ( mVerPos[ 0 ].YPos + mVerPos[ 1 ].YPos ) / 2.0, tRect, HSFalse );
			CRect tVerRect( tCenterPoint.x - 60, tCenterPoint.y - 10, tCenterPoint.x,  tCenterPoint.y + 10 );
			pMemDC->DrawText( tStrLength, &tVerRect, DT_CENTER | DT_VCENTER );

			tStrLength.Format( "%.02fm", abs( mHorPos[ 0 ].XPos - mHorPos[ 1 ].XPos ) );
			tCenterPoint.x = XDigitWithValue( ( mHorPos[ 0 ].XPos + mHorPos[ 1 ].XPos ) / 2.0, tRect, HSFalse );
			tCenterPoint.y = YDigitWithValue( ( mHorPos[ 0 ].YPos + mHorPos[ 1 ].YPos ) / 2.0, tRect, HSFalse );
			CRect tHorRect( tCenterPoint.x - 30, tCenterPoint.y - 16, tCenterPoint.x + 30,  tCenterPoint.y );
			pMemDC->DrawText( tStrLength, &tHorRect, DT_CENTER | DT_VCENTER );

			mTemplateDC->SelectObject( pOldFont );
			pMemDC->SetBkMode( tOldMode );
		}
	}

	if ( mShowAngleLine )
	{
		CPen tPen( PS_DOT, 1, RGB( 0, 0, 160 ) );
		CPen *pOldPen = pMemDC->SelectObject( &tPen );
		HSInt tOldMode = pMemDC->SetBkMode( TRANSPARENT );

		CPoint tPoints[ 2 ];
		tPoints[ 0 ].x = XDigitWithValue( 0, tRect, HSFalse );
		tPoints[ 0 ].y = YDigitWithValue( 0, tRect, HSFalse );
		tPoints[ 1 ].x = XDigitWithValue( mAngleEndPos.XPos, tRect, HSFalse );
		tPoints[ 1 ].y = YDigitWithValue( mAngleEndPos.YPos, tRect, HSFalse );
		pMemDC->Polyline( tPoints, 2 );

		HSDouble tAngle = DegreeToAngle( acos( mAngleEndPos.YPos / sqrt( mAngleEndPos.XPos * mAngleEndPos.XPos + mAngleEndPos.YPos * mAngleEndPos.YPos ) ) );
		tAngle = ( mAngleEndPos.XPos < 0 ? 360 - tAngle : tAngle );

		CString tStrAngle;
		tStrAngle.Format( "%.01f", tAngle );
		CPoint tCenterPoint;
		tCenterPoint.x = XDigitWithValue( mAngleEndPos.XPos / 2.0, tRect, HSFalse );
		tCenterPoint.y = YDigitWithValue( mAngleEndPos.YPos / 2.0, tRect, HSFalse );
		CRect tAngleRect( tCenterPoint.x - 30, tCenterPoint.y - 16, tCenterPoint.x + 30,  tCenterPoint.y );
		pMemDC->DrawText( tStrAngle, &tAngleRect, DT_CENTER | DT_VCENTER );

		pMemDC->SelectObject( pOldPen );
		pMemDC->SetBkMode( tOldMode );
	}

	DrawCheckCircle( pMemDC, tRect );
	
	DrawFrameRect( pMemDC, tRect );
}