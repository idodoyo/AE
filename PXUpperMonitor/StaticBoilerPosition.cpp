// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "StaticBoilerPosition.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;


// CStaticBoilerPosition

CStaticBoilerPosition::CStaticBoilerPosition()
{			
	mBeginYValue = 1;
	mTotalYValue = 1;	
	mYDirection = -1;
	mMinYValue = 0.01;	

	mMinXValue = 0.01;
	mTotalXValue = 2;

	mType = "[锅炉定位]";
	mTitle = "";

	mMaterialRadius = 1;	
	
	mFocusSensor = -1;
	mSensors = NULL;

	mIndexFont.CreatePointFont( 73, "Arial" );	
	mGLTitleFont.CreatePointFont( 110, "宋体" );		
	mContentFont.CreateFont( 16, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T( "宋体" ) ); 

	Y_SQUARE_LENGTH = 100;
	X_SQUARE_LENGTH = 100;	

	mShowXAxis = HSFalse;
	mShowYAxis = HSFalse;

	mMaxPillar = 4;

	mPointRectRadius = 10;

	for ( HSInt i = 0; i < sizeof( mTitleValues ) / sizeof( CString ); i++ )
	{
		mTitleValues[ i ] = "";
	}

	for ( HSInt i = 0; i < sizeof( mTotalValues ) / sizeof( CString ); i++ )
	{
		mTotalValues[ i ] = "";
	}
	
}

CStaticBoilerPosition::~CStaticBoilerPosition()
{	
}

HSVoid CStaticBoilerPosition::SetMaterialRadius( HSDouble tRadius )
{
	mMaterialRadius = tRadius;
	ResizeWave();
}

HSVoid CStaticBoilerPosition::SetFocusSensor( HSInt tFocusSensor )
{
	mFocusSensor = tFocusSensor; 
	ResizeWave();
}

HSVoid CStaticBoilerPosition::SetSensors( vector< SensorInfo > *pSensors )
{
	mSensors = pSensors;
	ResizeWave();
}

HSVoid CStaticBoilerPosition::SetPillarLength( vector< HSDouble > tPillarsLength )
{
	mPillarsLength = tPillarsLength;

	mMaxPillar = tPillarsLength[ 0 ];
	for ( HSUInt i = 1; i < tPillarsLength.size(); i++ )
	{
		if ( tPillarsLength[ i ] > mMaxPillar )
		{
			mMaxPillar = tPillarsLength[ i ];
		}
	}

	if ( mPillarAreaInfo.size() == 0 )
	{
		PillarAreaInfo tPillarAreaInfo = { CPoint( 0, 0 ), 0, 0, RGB( 255, 0, 0 ) };		
		for ( HSUInt i = 0; i < tPillarsLength.size(); i++ )
		{			
			mPillarAreaInfo.push_back( tPillarAreaInfo );			
		}
	}

	ResizeWave();
}

HSVoid CStaticBoilerPosition::SetPillarArea( HSInt tIndex, HSInt tCount, HSInt tRealCount, COLORREF tColor )
{
	mPillarAreaInfo[ tIndex ].Color = tColor;
	mPillarAreaInfo[ tIndex ].DefaultCount = tCount;
	mPillarAreaInfo[ tIndex ].RealCount = tRealCount;
}

HSVoid CStaticBoilerPosition::SetFeedMaterialTotalInfo( HSInt tIronGood, HSInt tIronBad, HSInt tCokeGood, HSInt CokeBad )
{
	mTotalValues[ 0 ].Format( "%d", tIronGood );
	mTotalValues[ 1 ].Format( "%d", tIronBad );
	mTotalValues[ 2 ].Format( "%d", tIronGood + tIronBad );
	mTotalValues[ 3 ].Format( "%d", tCokeGood );
	mTotalValues[ 4 ].Format( "%d", CokeBad );
	mTotalValues[ 5 ].Format( "%d", tCokeGood + CokeBad );
	mTotalValues[ 6 ].Format( "%d", tIronGood + tCokeGood );
	mTotalValues[ 7 ].Format( "%d", tIronBad + CokeBad );
	mTotalValues[ 8 ].Format( "%d", tIronGood + tCokeGood + tIronBad + CokeBad );
}

HSVoid CStaticBoilerPosition::SetFeedTitleInfo( CString *pTitleValues, HSInt tTotalCount, HSInt tCurIndex )
{
	mFeedMaterialTitle.Format( "加料信息( %d / %d )", tCurIndex, tTotalCount );

	for ( HSInt i = 0; i < sizeof( mTitleValues ) / sizeof( CString ); i++ )
	{
		mTitleValues[ i ] = pTitleValues[ i ];
	}
}

void CStaticBoilerPosition::ResizeWave()
{
	WaitForSingleObject( mMutex, INFINITE );

	CStaticGrid::ResizeWave();	

	SetEvent( mMutex );
}

HSVoid CStaticBoilerPosition::SetYValue( HSDouble tTotalValue, HSDouble tBeginValue )
{
	CStaticGrid::SetYValue( tTotalValue, tBeginValue );

	char tBuf[ 64 ];
	sprintf_s( tBuf, ( LPCSTR )mYFormat, this->mBeginYValue );	
	int tBeginValueLength = strlen( tBuf );
	sprintf_s( tBuf, ( LPCSTR )mYFormat, mBeginYValue + mTotalYValue * mYDirection );	
	int tEndValueLength = strlen( tBuf );
	mLeftDelta = max( 3 + max( tBeginValueLength, tEndValueLength ) * 9, 30 );
}

void CStaticBoilerPosition::Draw( CDC *pDC )
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

bool CStaticBoilerPosition::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	WaitForSingleObject( mMutex, INFINITE );

	if ( CStaticGrid::DrawTemplate( pMemDC, tRect ) )
	{					
		HSLong tCenterX = XDigitWithValue( 0, tRect, HSFalse );
		HSLong tCenterY = YDigitWithValue( 0, tRect, HSFalse );
		HSLong tRadius = min( XDigitWithValue( mMaterialRadius, tRect, HSFalse ) - tCenterX - 20, YDigitWithValue( -mMaterialRadius, tRect, HSFalse ) - tCenterY - 20 );
		tCenterX = max( mLeftDelta + tRadius + 50, tRect.Width() / 2 - tRadius - 100 );

		mPointRectRadius = max( min( tRadius / 20, 16 ), 3 );

		HSLong tTopX = tCenterX - tRadius;
		HSLong tTopY = tCenterY - tRadius;
		HSLong tBottomX  = tCenterX + tRadius;
		HSLong tBottomY  = tCenterY + tRadius;
		
		CRect tMaterialRect( tTopX, tTopY, tBottomX, tBottomY );
		CBrush tBrush( RGB( 200, 200, 200 ) );
		
		CBrush *pOldBrush = mTemplateDC->SelectObject( &tBrush );
		mTemplateDC->Ellipse( tMaterialRect );
		mTemplateDC->SelectObject( pOldBrush );
		
		HSLong tDelta = 10;
		CRect tInlineMaterialRect( tTopX + tDelta, tTopY + tDelta, tBottomX - tDelta, tBottomY - tDelta );
		CBrush tInlineBrush( RGB( 220, 220, 220 ) );

		pOldBrush = mTemplateDC->SelectObject( &tInlineBrush );
		mTemplateDC->Ellipse( tInlineMaterialRect );
		mTemplateDC->SelectObject( pOldBrush );	

		HSLong tDefaultPillerLength = tBottomY - YDigitWithValue( 0, tRect, HSFalse ) + 30;
		HSLong tPillarWidth = 10;
		CRect tPillar1Rect( tCenterX - tPillarWidth, tTopY - 30, tCenterX + tPillarWidth, tTopY - 30 + HSLong( tDefaultPillerLength * mPillarsLength[ 0 ] / mMaxPillar ) );
		mTemplateDC->FillSolidRect( &tPillar1Rect, RGB( 150, 150, 150 ) );
		CRect tPillar2Rect( tBottomX + 30, tCenterY - tPillarWidth, tBottomX + 30 - HSLong( tDefaultPillerLength * mPillarsLength[ 1 ] / mMaxPillar ), tCenterY + tPillarWidth );
		mTemplateDC->FillSolidRect( &tPillar2Rect, RGB( 150, 150, 150 ) );
		CRect tPillar3Rect( tCenterX - tPillarWidth, tBottomY + 30, tCenterX + tPillarWidth, tBottomY + 30 - HSLong( tDefaultPillerLength * mPillarsLength[ 2 ] / mMaxPillar ) );
		mTemplateDC->FillSolidRect( &tPillar3Rect, RGB( 150, 150, 150 ) );
		CRect tPillar4Rect( tTopX - 30, tCenterY - tPillarWidth, tTopX - 30 + HSLong( tDefaultPillerLength * mPillarsLength[ 3 ] / mMaxPillar ), tCenterY + tPillarWidth );
		mTemplateDC->FillSolidRect( &tPillar4Rect, RGB( 150, 150, 150 ) );

		CPen tPen( PS_DOT, 1, RGB( 0, 0, 160 ) );
		CPen *pOldPen = mTemplateDC->SelectObject( &tPen );
		HSInt tOldMode = mTemplateDC->SetBkMode( TRANSPARENT );

		HSLong tCircleDelta = HSLong( ( sqrt( HSDouble( ( tTopX - tCenterX ) * ( tTopX - tCenterX ) + ( tTopY - tCenterY ) * ( tTopY - tCenterY ) ) ) - tRadius ) * sin( 45.0 * 3.14159265 / 180.0 ) ) + tDelta;		

		CPoint tPoints[ 4 ] = { CPoint( tTopX + tCircleDelta, tTopY + tCircleDelta ), CPoint( tBottomX - tCircleDelta, tBottomY - tCircleDelta ), CPoint( tMaterialRect.right - tCircleDelta, tMaterialRect.top + tCircleDelta ), CPoint( tMaterialRect.left + tCircleDelta, tMaterialRect.bottom - tCircleDelta ) };		
		mTemplateDC->Polyline( tPoints, 2 );
		mTemplateDC->Polyline( &tPoints[ 2 ], 2 );

		mTemplateDC->SelectObject( pOldPen );
		mTemplateDC->SetBkMode( tOldMode );


		HBRUSH tNullBrush = ( HBRUSH )GetStockObject( NULL_BRUSH );
		pOldBrush = mTemplateDC->SelectObject( CBrush::FromHandle( tNullBrush ) );
		mTemplateDC->Ellipse( tMaterialRect );
		mTemplateDC->Ellipse( tInlineMaterialRect );
		mTemplateDC->SelectObject( pOldBrush );
		

		if ( mPillarAreaInfo.size() > 0 )
		{
			HSInt tPillarDelta = mPointRectRadius * 2 + mPointRectRadius;
			mPillarAreaInfo[ 0 ].StartPoint = CPoint( tCenterX - tPillarDelta * 2, ( tCenterY + tCenterY - tRadius ) / 2 );
			mPillarAreaInfo[ 1 ].StartPoint = CPoint( ( tCenterX + tCenterX + tRadius ) / 2, tCenterY - tPillarDelta * 2 );
			mPillarAreaInfo[ 2 ].StartPoint = CPoint( tCenterX + tPillarDelta * 2, ( tCenterY + tCenterY + tRadius ) / 2 );
			mPillarAreaInfo[ 3 ].StartPoint = CPoint( ( tCenterX + tCenterX - tRadius ) / 2, tCenterY + tPillarDelta * 2 );			
		}
		
		CFont *tOldFont = mTemplateDC->SelectObject( &mGLTitleFont );
		CString tTitles[] = { "加料时间:", "结束时间:", "加料种类:", "预期圈数:", "实际圈数:", "加料结果:" };
		HSInt tTitleBeginPos = tCenterY - tRadius - 20;
		HSInt tTilePosDelta = max( tRadius * 2 * 3 / 5 / ( sizeof( tTitles ) / sizeof( CString ) ), 20 );
		mDrawLabelStartPoint = CPoint( tCenterX + tRadius * 3 / 2 + 110, tTitleBeginPos );
		mDrawLabelDelta = tTilePosDelta;
		for ( HSInt i = 0; i < sizeof( tTitles ) / sizeof( CString ); i++ )
		{
			CRect tTitleRect( tCenterX + tRadius * 3 / 2, tTitleBeginPos, tCenterX + tRadius * 3 / 2 + 100, tTitleBeginPos + 50 );
			mTemplateDC->DrawText( tTitles[ i ], &tTitleRect, DT_CENTER | DT_VCENTER );

			tTitleBeginPos += tTilePosDelta;
		}

		tTitleBeginPos += tTilePosDelta / 2;

		HSInt tLeftBegin = tCenterX + tRadius * 3 / 2 + 100;
		mDrawTotalStartPoint = CPoint( tLeftBegin, tTitleBeginPos );
		CString tTotalTitles[] = { "均匀次数", "不均匀次数", "总次数", "铁矿石", "焦  炭", "汇  总" };
		for ( HSInt i = 0; i < sizeof( tTotalTitles ) / sizeof( CString ) - 3; i++ )
		{			
			CRect tTotalRect( tLeftBegin, tTitleBeginPos, tLeftBegin + 100, tTitleBeginPos + 50 );
			mTemplateDC->DrawText( tTotalTitles[ i ], &tTotalRect, DT_CENTER | DT_VCENTER );
			tLeftBegin += 100;
		}

		tTitleBeginPos += 25;
		mDrawTotalStartPoint.y = tTitleBeginPos;
		for ( HSInt i = 3; i < sizeof( tTotalTitles ) / sizeof( CString ); i++ )
		{			
			CRect tTotalRect( tCenterX + tRadius * 3 / 2, tTitleBeginPos, tCenterX + tRadius * 3 / 2 + 100, tTitleBeginPos + 50 );
			mTemplateDC->DrawText( tTotalTitles[ i ], &tTotalRect, DT_CENTER | DT_VCENTER );			
			tTitleBeginPos += tTilePosDelta;
		}

		mTemplateDC->SelectObject( tOldFont );
		

		HSLong tSensorDelta = 15;
		CPoint tSensorPoints[] = { CPoint( tCenterX, tTopY - tSensorDelta ), CPoint( tBottomX + tSensorDelta, tCenterY ), CPoint( tCenterX, tBottomY + tSensorDelta ), CPoint( tTopX - tSensorDelta, tCenterY ) };

		for ( HSUInt i = 0; i < mSensors->size(); i++ )
		{			
			CPoint tPoint = tSensorPoints[ i ];

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

void CStaticBoilerPosition::DrawWave( CDC *pMemDC, CRect &tRect )
{	
	HSInt tDelta = mPointRectRadius;
	HSInt tPillarDelta = mPointRectRadius * 2 + mPointRectRadius;

	for ( HSUInt i = 0; i < mPillarAreaInfo.size(); i++ )
	{	
		HSInt tMaxCount = max( mPillarAreaInfo[ i ].DefaultCount, mPillarAreaInfo[ i ].RealCount );
		for ( HSInt j = 0; j < tMaxCount; j++ )
		{			
			CPoint tPoint( mPillarAreaInfo[ i ].StartPoint.x + ( j % 5 ) * tPillarDelta, mPillarAreaInfo[ i ].StartPoint.y + j / 5 * -tPillarDelta );
			if ( i == 1 )
			{
				tPoint = CPoint( mPillarAreaInfo[ i ].StartPoint.x + ( j / 5 ) * tPillarDelta, mPillarAreaInfo[ i ].StartPoint.y + ( j % 5 ) * tPillarDelta );
			}
			else if ( i == 2 )
			{
				tPoint = CPoint( mPillarAreaInfo[ i ].StartPoint.x + ( j % 5 ) * -tPillarDelta, mPillarAreaInfo[ i ].StartPoint.y + j / 5 * tPillarDelta );
			}
			else if ( i == 3 )
			{
				tPoint = CPoint( mPillarAreaInfo[ i ].StartPoint.x + ( j / 5 ) * -tPillarDelta, mPillarAreaInfo[ i ].StartPoint.y + ( j % 5 ) * -tPillarDelta );
			}

			CRect tPointRect( tPoint.x - tDelta, tPoint.y - tDelta, tPoint.x + tDelta, tPoint.y + tDelta );
			if ( j < mPillarAreaInfo[ i ].DefaultCount )
			{
				pMemDC->FillSolidRect( &tPointRect, RGB( 180, 180, 180 ) );
			}

			if ( j < mPillarAreaInfo[ i ].RealCount )
			{
				pMemDC->FillSolidRect( &tPointRect, mPillarAreaInfo[ i ].Color );
			}
		}	
	}

	HSInt tOldMode = pMemDC->SetBkMode( TRANSPARENT );
	CFont *tOldFont = pMemDC->SelectObject( &mContentFont );

	CString tTitle = "加料信息( 16 / 100 )";
	CRect tTitleRect( mDrawLabelStartPoint.x - 100, mTopDelta, mDrawLabelStartPoint.x + 240, mDrawLabelStartPoint.y + mDrawLabelDelta / 4 );
	pMemDC->DrawText( mFeedMaterialTitle, &tTitleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
	
	CString tTitleValues[] = { "12:11:16", "12:12:19", "铁矿石", "10", "6", "不均匀" };
	for ( HSInt i = 0; i < 6; i++ )
	{
		CRect tTitleRect( mDrawLabelStartPoint.x, mDrawLabelStartPoint.y + i * mDrawLabelDelta, mDrawLabelStartPoint.x + 100, mDrawLabelStartPoint.y + ( i + 1 ) * mDrawLabelDelta );
		pMemDC->DrawText( mTitleValues[ i ], &tTitleRect, DT_LEFT | DT_VCENTER );
	}

	CString tTotalValues[] = { "48", "2", "50", "40", "10", "50", "88", "12", "100" };
	for ( HSInt i = 0; i < 9; i++ )
	{
		CRect tTitleRect( mDrawTotalStartPoint.x + ( i % 3 ) * 100, mDrawTotalStartPoint.y + i / 3 * mDrawLabelDelta, mDrawTotalStartPoint.x + ( i % 3 + 1 ) * 100, mDrawTotalStartPoint.y + ( i / 3 + 1 ) * mDrawLabelDelta );
		pMemDC->DrawText( mTotalValues[ i ], &tTitleRect, DT_CENTER | DT_VCENTER );
	}

	pMemDC->SetBkMode( tOldMode );
	pMemDC->SelectObject( tOldFont );

	DrawFrameRect( pMemDC, tRect );
}