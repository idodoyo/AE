// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "StaticGrid.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;


// CStaticGrid

CStaticGrid::CStaticGrid()
{	
	mBeginXValue = 0;
	mTotalXValue = 10;

	mBeginYValue = 0;
	mTotalYValue = 10;

	mXSepValue = 1.0;
	mYSepValue = 1.0;

	mXFormat = "%.0f";
	mYFormat = "%.0f";

	mXDirection = 1;
	mYDirection = 1;

	mMinXValue = mTotalXValue;
	mMinYValue = mTotalYValue;

	mShowXAxis = HSTrue;
	mShowYAxis = HSTrue;

	mAutoDrawGrid = HSTrue;

	Y_SQUARE_LENGTH = 100;
	X_SQUARE_LENGTH = 150;
}

CStaticGrid::~CStaticGrid()
{	
}

HSDouble CStaticGrid::GetCorrectValue( HSDouble tValue, CString &tPreciseFormat )
{		
	HSDouble tTenValue = 1000000;
	HSDouble tFiveValue = 5000000;	
	do
	{
		tTenValue /= 10;
		tFiveValue /= 10;
	} while( min( abs( tTenValue - tValue ), abs( tFiveValue - tValue ) ) > tValue );
	
	HSDouble tResValue = ( abs( tTenValue - tValue ) < abs( tFiveValue - tValue ) ? tTenValue : tFiveValue );	

	HSDouble tTmpValue = tResValue;
	HSInt tPreciseNum = 0;
	while ( tTmpValue < 1 )
	{
		tTmpValue *= 10;
		tPreciseNum++;
	}	

	CString tFormats[] = { "%.0f", "%.1f", "%.2f", "%.3f", "%.4f", "%.5f" };
	tPreciseFormat = tFormats[ min( 5, tPreciseNum ) ];		

	return tResValue;
}

HSVoid CStaticGrid::SetXValue( HSDouble tTotalValue, HSDouble tBeginValue )
{
	mBeginXValue = tBeginValue;
	mTotalXValue = max( tTotalValue, this->mMinXValue );

	CRect tRect;
	this->GetClientRect( &tRect );	
	
	HSInt tPreciseNum = 0;
	mXSepValue = GetCorrectValue( mTotalXValue / ( tRect.Width() - mLeftDelta * 2 ) * X_SQUARE_LENGTH, this->mXFormat );

	ResizeWave();
}

HSVoid CStaticGrid::SetYValue( HSDouble tTotalValue, HSDouble tBeginValue )
{
	mBeginYValue = tBeginValue;
	mTotalYValue = max( tTotalValue, this->mMinYValue );

	CRect tRect;
	this->GetClientRect( &tRect );		
	mYSepValue = GetCorrectValue( mTotalYValue / ( tRect.Height() - mTopDelta * 2 ) * Y_SQUARE_LENGTH, this->mYFormat );

	ResizeWave();
}

HSVoid CStaticGrid::SetMinXValue( HSDouble tValue )
{
	this->mMinXValue = tValue;
}

HSVoid CStaticGrid::SetMinYValue( HSDouble tValue )
{
	this->mMinYValue = tValue;
}

// CStaticGrid message handlers
bool CStaticGrid::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	if ( CStaticTitle::DrawTemplate( pMemDC, tRect ) )		
	{					
		if ( mAutoDrawGrid )
		{
			DrawGrid( tRect );
		}		

		return true;
	}

	return false;	
}

HSVoid CStaticGrid::DrawGrid( CRect &tRect )
{	
	CPen tDigitPen( PS_SOLID, 1, RGB( 180, 180, 180 ) );
	CPen *pOldPen = mTemplateDC->SelectObject( &tDigitPen );

	char tStrBuf[ 64 ];

	if ( mShowXAxis )
	{		
		double tBeginXValue = ( mBeginXValue > 0 ? ( mBeginXValue + mXSepValue - 0.00000001 ) : mBeginXValue );
		tBeginXValue = ( ( int )( tBeginXValue / mXSepValue ) ) * mXSepValue;		
		double tHorDelta = ( tRect.Width() - ( mLeftDelta * 2 ) ) / mTotalXValue * mXSepValue;
		double tHorX = abs( tBeginXValue - mBeginXValue ) * ( ( tRect.Width() - ( mLeftDelta * 2 ) ) / mTotalXValue ) + mLeftDelta;
		while ( tHorX < tRect.right )
		{	
			if ( tHorX + 0.00001 > tRect.left + mLeftDelta && tHorX - 0.00001 < tRect.right - mLeftDelta )
			{
				sprintf_s( tStrBuf, ( LPCSTR )mXFormat, tBeginXValue );				
				CRect tXRect( ( int )( tHorX - 30 ), tRect.bottom - mTopDelta + 4, ( int )( tHorX + 30 ), tRect.bottom );
				mTemplateDC->DrawText( CString( tStrBuf ), &tXRect, DT_CENTER | DT_VCENTER );		
				
				if ( ( int )tHorX != tRect.left + mLeftDelta && ( int )tHorX != tRect.right - mLeftDelta )
				{
					mTemplateDC->MoveTo( ( int )tHorX, tRect.top + mTopDelta );
					mTemplateDC->LineTo( ( int )tHorX, tRect.bottom - mTopDelta );
				}
			}

			tBeginXValue += ( mXSepValue * mXDirection );
			tHorX += tHorDelta;	
		}		
	}

	if ( mShowYAxis )
	{
		double tBeginYValue = ( mBeginYValue > 0 ? mBeginYValue : ( mBeginYValue - mYSepValue + 0.00000001 ) );
		tBeginYValue = ( ( int )( tBeginYValue / mYSepValue ) ) * mYSepValue;		
		double tVerDelta = ( tRect.Height() - ( mTopDelta * 2 ) ) / mTotalYValue * mYSepValue;		
		double tVerY = abs( tBeginYValue - mBeginYValue ) * ( ( tRect.Height() - ( mTopDelta * 2 ) ) / mTotalYValue ) + mTopDelta;
		while ( tVerY < tRect.bottom )
		{	
			if ( tVerY + 0.00001 > tRect.top + mTopDelta && tVerY - 0.00001 < tRect.bottom - mTopDelta )
			{
				sprintf_s( tStrBuf, ( LPCSTR )mYFormat, tBeginYValue );
				CRect tYValueRect( 0, ( int )( tVerY - 10 ), mLeftDelta - 6, ( int )( tVerY + 10 ) );				
				mTemplateDC->DrawText( CString( tStrBuf ), &tYValueRect, DT_RIGHT | DT_VCENTER );				
				if ( ( int )tVerY != tRect.top + mTopDelta && ( int )tVerY != tRect.bottom - mTopDelta )
				{
					mTemplateDC->MoveTo( tRect.left + mLeftDelta, ( int )tVerY );
					mTemplateDC->LineTo( tRect.right - mLeftDelta, ( int )tVerY );
				}
			}

			tBeginYValue += ( mYSepValue * mYDirection );
			tVerY += tVerDelta;	
		}
	}

	mTemplateDC->SelectObject( pOldPen );
	
	mFrameTemplateDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mTemplateDC, 0, 0, SRCCOPY );

	int tTopDelta =  mTopDelta - 3;	
	int tLeftDelta = mLeftDelta - 3;
	CRect tInlineRect( tRect.left + tLeftDelta, tRect.top + tTopDelta, tRect.right - tLeftDelta, tRect.bottom - tTopDelta );
	mFrameTemplateDC->FillSolidRect( &tInlineRect, RGB( 10, 10, 10 ) );	

	CRect tCenterRect( tRect.left + mLeftDelta, tRect.top + mTopDelta, tRect.right - mLeftDelta, tRect.bottom - mTopDelta );
	mFrameTemplateDC->FillSolidRect( &tCenterRect, RGB( 0, 0, 0 ) );
}

HSLong CStaticGrid::CorrectXDigit( HSLong tDigit, CRect &tRect )
{
	if ( tDigit < mLeftDelta )
	{
		tDigit = mLeftDelta - 1;
	}
	else if ( tDigit > tRect.Width() - mLeftDelta )
	{
		tDigit = tRect.Width() - mLeftDelta + 1;
	}

	return tDigit;
}

HSLong CStaticGrid::CorrectYDigit( HSLong tDigit, CRect &tRect )
{
	if ( tDigit < mTopDelta )
	{
		tDigit = mTopDelta - 1;
	}
	else if ( tDigit > tRect.Height() - mTopDelta )
	{
		tDigit = tRect.Height() - mTopDelta + 1;
	}

	return tDigit;
}

HSLong CStaticGrid::XDigitWithValue( HSDouble tValue, CRect &tRect, HSBool tCorrectValue )
{
	HSDouble tDigitPerXValue = ( tRect.Width() - ( mLeftDelta * 2 ) ) / mTotalXValue;

	HSLong tDigit = HSLong( ( tValue - mBeginXValue ) * tDigitPerXValue ) + mLeftDelta;
	if ( !tCorrectValue )
	{
		return tDigit;
	}
	
	return CorrectXDigit( tDigit, tRect );
}

HSLong CStaticGrid::YDigitWithValue( HSDouble tValue, CRect &tRect, HSBool tCorrectValue )
{
	HSDouble tDigitPerYValue = ( tRect.Height() - ( mTopDelta * 2 ) ) / mTotalYValue;	

	HSLong tDigit = HSLong( ( mBeginYValue - tValue ) * tDigitPerYValue ) + mTopDelta + tRect.top;
	if ( !tCorrectValue )
	{
		return tDigit;
	}
	
	return CorrectYDigit( tDigit, tRect );
}

HSBool CStaticGrid::PosWithPoint( CPoint tPoint, CRect &tRect, HSDouble &tXValue, HSDouble &tYValue )
{
	if ( tPoint.x < mLeftDelta || tPoint.x > ( tRect.Width() - mLeftDelta ) || tPoint.y < mTopDelta || tPoint.y > ( tRect.Height() - mTopDelta ) )
	{
		return HSFalse;
	}

	tXValue = tPoint.x - mLeftDelta;
	tXValue = tXValue * mTotalXValue / ( tRect.Width() - mLeftDelta * 2 ) + mBeginXValue;

	tYValue = tPoint.y - mTopDelta;
	tYValue = mBeginYValue - tYValue * mTotalYValue / ( tRect.Height() - mTopDelta * 2 );

	return HSTrue;
}

HSBool CStaticGrid::IsPointValid( CPoint tPoint, CRect &tRect )
{
	if ( tPoint.x < tRect.Width() - mLeftDelta && tPoint.x > mLeftDelta && tPoint.y > mTopDelta && tPoint.y < tRect.Height() - mTopDelta )
	{
		return HSTrue;
	}

	return HSFalse;
}

HSVoid CStaticGrid::DrawCheckCircle( CDC *pDC, CRect &tRect )
{
	if ( this->mCheckRadiusEnabled )
	{
	/*	HSInt tRadius = XDigitWithValue( mCheckRadius / 1000.0, tRect, HSFalse ) - XDigitWithValue( 0, tRect, HSFalse );
		HSLong tPosX = XDigitWithValue( mCheckPosX / 1000.0, tRect, HSFalse );
		HSLong tPosY = YDigitWithValue( mCheckPosY / 1000.0, tRect, HSFalse );

		HBRUSH tBrush = ( HBRUSH )GetStockObject( NULL_BRUSH );
		CBrush *pOldBrush = pDC->SelectObject( CBrush::FromHandle( tBrush ) );
		pDC->Ellipse( tPosX - tRadius, tPosY - tRadius, tPosX + tRadius, tPosY + tRadius );
		pDC->SelectObject( pOldBrush );		*/

		HSLong tLeftPos = XDigitWithValue( mCheckPosX / 1000.0 - mCheckRadius / 1000.0, tRect, HSFalse );
		HSLong tTopPos = YDigitWithValue( mCheckPosY / 1000.0 - mCheckRadius / 1000.0, tRect, HSFalse );
		HSLong tRightPos = XDigitWithValue( mCheckPosX / 1000.0 + mCheckRadius / 1000.0, tRect, HSFalse );
		HSLong tBottomPos = YDigitWithValue( mCheckPosY / 1000.0 + mCheckRadius / 1000.0, tRect, HSFalse );

		HBRUSH tBrush = ( HBRUSH )GetStockObject( NULL_BRUSH );
		CBrush *pOldBrush = pDC->SelectObject( CBrush::FromHandle( tBrush ) );
		pDC->Ellipse( tLeftPos, tTopPos, tRightPos, tBottomPos );
		pDC->SelectObject( pOldBrush );
	}
}

HSVoid CStaticGrid::DrawCircle( CDC *pDC, CRect &tRect, HSDouble tLeft, HSDouble tTop, HSDouble tRight, HSDouble tBottom )
{	
	HSLong tLeftPos = XDigitWithValue( tLeft, tRect, HSFalse );
	HSLong tTopPos = YDigitWithValue( tTop, tRect, HSFalse );
	HSLong tRightPos = XDigitWithValue( tRight, tRect, HSFalse );
	HSLong tBottomPos = YDigitWithValue( tBottom, tRect, HSFalse );

	HBRUSH tBrush = ( HBRUSH )GetStockObject( NULL_BRUSH );
	CBrush *pOldBrush = pDC->SelectObject( CBrush::FromHandle( tBrush ) );
	pDC->Ellipse( tLeftPos, tTopPos, tRightPos, tBottomPos );
	pDC->SelectObject( pOldBrush );
}


HSBool CStaticGrid::PositionWithinCheckCircle( CPoint tPos, CRect &tRect )
{
	if ( !mCheckRadiusEnabled )
	{
		return HSFalse;
	}

	HSDouble tX = 0;
	HSDouble tY = 0;
	if ( PosWithPoint( tPos, tRect, tX, tY ) )
	{
		tX -= mCheckPosX / 1000.0;
		tY -= mCheckPosY / 1000.0;
		if ( sqrt( tX * tX + tY * tY ) < mCheckRadius / 1000.0 )
		{
			return HSTrue;
		}
	}

	/*
	CPoint tPoint( XDigitWithValue( mCheckPosX / 1000.0, tRect, HSFalse ), YDigitWithValue( mCheckPosY / 1000.0, tRect, HSFalse ) );
	if ( IsPointValid( tPoint, tRect ) )
	{						
		HSInt tRadius = XDigitWithValue( mCheckRadius / 1000.0, tRect, HSFalse ) - XDigitWithValue( 0, tRect, HSFalse );

		HSDouble tX = tPos.x - tPoint.x;
		HSDouble tY = tPos.y - tPoint.y;

		if ( tRadius > ( HSInt )sqrt( tX * tX + tY * tY ) )
		{
			return HSTrue;
		}
	}
	*/
	return HSFalse;
}