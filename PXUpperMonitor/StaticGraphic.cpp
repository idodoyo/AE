// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "StaticGraphic.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"
#include "DataExportHelper.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;


// CStaticGraphic

CStaticGraphic::CStaticGraphic()
{	
	mTotalVoltage = 10;
	mBeginVoltage = 5;

	mTotalSeconds = 1.0;
	mBeginSeconds = 0;

	mPoints = NULL;
	mPointLength = 0;

	mDataHandler = NULL;
	mData = NULL;
	mLength = 0;

	mSecondUnit = "ms";
	mVoltageUnit = "v";	

	mYFormat = "%.0f";	
	mBeginYValue = 5.0;
	mTotalYValue = 10.0;
	mMinYValue = 0.01;
	
	mBeginXValue = 0;	
	mTotalXValue = 1000;
	mXFormat = "%.0f";
	mMinXValue = 0.00001;

	mYDirection = -1;

	mType = "[²¨ÐÎÍ¼] ";	

	mVoltageOffset = 0;
}

CStaticGraphic::~CStaticGraphic()
{
	if ( mPoints != NULL )
	{
		delete[] mPoints;
	}
}

// CStaticGraphic message handlers

void CStaticGraphic::SetVoltage( double tTotalVoltage, double tBeginVoltage )
{				
	if ( tTotalVoltage < 0.01 )
	{
		tTotalVoltage = 0.01;
	}	

	mTotalVoltage = tTotalVoltage;	
	mBeginVoltage = tBeginVoltage;	

	mBeginYValue = mBeginVoltage;
	mTotalYValue = mTotalVoltage;	
	
	int tVoltageBase = 1;
	if ( mBeginVoltage < 1.0 && mBeginVoltage > -1.0 && mBeginVoltage - mTotalVoltage < 1.0 && mBeginVoltage - mTotalVoltage > -1.0 )
	{
		mVoltageUnit = "mv";
		mBeginYValue *= 1000;
		mTotalYValue *= 1000;				
		mMinYValue = 10;

		tVoltageBase = 1000;
	}	
	else
	{
		mVoltageUnit = "v";	
		mMinYValue = 0.01;
	}
	
	CStaticGrid::SetYValue( mTotalYValue, mBeginYValue );	
	
	char tBuf[ 64 ];
	sprintf_s( tBuf, ( LPCSTR )mYFormat, ( mBeginVoltage - mTotalVoltage ) * tVoltageBase );	
	int tBeginVoltageLength = strlen( tBuf );
	sprintf_s( tBuf, ( LPCSTR )mYFormat, mBeginVoltage * tVoltageBase );	
	int tEndVoltageLength = strlen( tBuf );
	mLeftDelta = max( 3 + max( tBeginVoltageLength, tEndVoltageLength ) * 9, 30 );

	CString tTitle;
	if ( mDataHandler != NULL )
	{				
		tTitle.Format( "%s < %d >  (%s/%s)", mDataHandler->Name().c_str(), mChannelIdentifier.ChannelIndex() + 1, mVoltageUnit, mSecondUnit );			
	}
	else
	{
		tTitle.Format( "DEVICE < CHANNEL >  (%s/%s)", mVoltageUnit, mSecondUnit );	
	}	

	SetTitle( tTitle );
	ResizeWave();
}

double CStaticGraphic::TotalVoltage()
{
	return mTotalVoltage;
}

double CStaticGraphic::BeginVoltage()
{
	return mBeginVoltage;
}

void CStaticGraphic::SetSeconds( double tTotalSeconds, double tBeginSeconds )
{
	mTotalSeconds = max( tTotalSeconds, 0.00001 );	
	mBeginSeconds = tBeginSeconds;	

	mBeginXValue = mBeginSeconds;	
	mTotalXValue = mTotalSeconds;	
	mMinXValue = 0.00001;
	mSecondUnit = "s";	
	if ( mBeginSeconds < 0.001 && mBeginSeconds > -0.001 && mBeginSeconds + mTotalSeconds < 0.001 && mBeginSeconds + mTotalSeconds > -0.001 )
	{
		mSecondUnit = "us";		
		
		mBeginXValue = mBeginSeconds * 1000000;	
		mTotalXValue = mTotalSeconds * 1000000;		

		mMinXValue = 10;
	}
	else if ( mBeginSeconds < 1.0 && mBeginSeconds > -1.0 && mBeginSeconds + mTotalSeconds < 1.0 && mBeginSeconds + mTotalSeconds > -1.0 )
	{
		mSecondUnit = "ms";	
		
		mBeginXValue = mBeginSeconds * 1000;	
		mTotalXValue = mTotalSeconds * 1000;			
	}

	CStaticGrid::SetXValue( mTotalXValue, mBeginXValue );		

	CString tTitle;
	if ( mDataHandler != NULL )
	{				
		tTitle.Format( "%s < %d >  (%s/%s)", mDataHandler->Name().c_str(), mChannelIdentifier.ChannelIndex() + 1, mVoltageUnit, mSecondUnit );			
	}
	else
	{
		tTitle.Format( "DEVICE < CHANNEL >  (%s/%s)", mVoltageUnit, mSecondUnit );	
	}

	SetTitle( tTitle );
	ResizeWave();
}

void CStaticGraphic::ResizeWave()
{	
	WaitForSingleObject( mMutex, INFINITE );

	CStaticGrid::ResizeWave();
	
	CRect tRect;
	this->GetClientRect( &tRect );
	
	int tLength = ( tRect.Width() - mLeftDelta * 2 ) * 4 + 1600;
	if ( tLength != mPointLength )
	{
		 if ( mPoints != NULL )
		 {
			 delete[] mPoints;
		 }

		 mPoints = new CPoint[ tLength ];
		 mPointLength = tLength;
	}

	SetEvent( mMutex );	
}

void CStaticGraphic::SetData( char *pData, int tLength, HSDouble tBeginSecond )
{
	mData = pData;
	mLength = tLength;
	mDataBeginSecond = tBeginSecond;
}

void CStaticGraphic::SetDataHandler( IDataHandler *pDataHandler, DEVICE_CH_IDENTIFIER tChannelIdentifier )
{
	mDataHandler = pDataHandler;
	mChannelIdentifier = tChannelIdentifier;
	mLength = 0;

	if ( pDataHandler != NULL )
	{		
		CString tTitle;
		tTitle.Format( "%s < %d >  (%s/%s)", pDataHandler->Name().c_str(), tChannelIdentifier.ChannelIndex() + 1, mVoltageUnit, mSecondUnit );		

		SetTitle( tTitle );
	}
}

void CStaticGraphic::Draw( CDC *pDC )
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

bool CStaticGraphic::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	WaitForSingleObject( mMutex, INFINITE );

	if ( CStaticGrid::DrawTemplate( pMemDC, tRect ) )		
	{
		if ( mDataHandler && int( mDataHandler->BypassVoltage( mChannelIdentifier.ChannelIndex() ) * 1000000 ) != 0 )
		{			
			int tBypassY = this->DigitWithVoltage( tRect, mDataHandler->BypassVoltage( mChannelIdentifier.ChannelIndex() ) - mVoltageOffset, ( tRect.Height() - ( mTopDelta * 2 ) ) / mTotalVoltage );
			if ( tBypassY > mTopDelta && tBypassY < tRect.Height() - mTopDelta )
			{				
				CPen tBypassPen( PS_DOT, 1, RGB( 0, 0, 250 ) );	
				CPen *pOldPen = mTemplateDC->SelectObject( &tBypassPen );

				mTemplateDC->MoveTo( tRect.left + mLeftDelta, tBypassY );
				mTemplateDC->LineTo( tRect.right - mLeftDelta, tBypassY );

				mTemplateDC->SelectObject( pOldPen );
			}	
		}
	}

	pMemDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mTemplateDC, 0, 0, SRCCOPY );

	SetEvent( mMutex );

	return true;
}

void CStaticGraphic::DrawWave( CDC *pMemDC, CRect &tRect )
{	
	if ( mData == NULL || mLength == 0 || mDataHandler == NULL )
	{
		return;
	}

	HSInt tEachSampleSize = mDataHandler->EachSampleSize();

	double tTotalSamples = mDataHandler->SampleRate( mChannelIdentifier ) * mTotalSeconds;
	double tSamplesPerDigit = tTotalSamples / ( mPointLength / 4 - 400 );
	double tDigitPerVoltage = ( tRect.Height() - ( mTopDelta * 2 ) ) / mTotalVoltage;	
	double tTimeDigitOffset = ( mDataBeginSecond - mBeginSeconds ) * mDataHandler->SampleRate( mChannelIdentifier ) / tSamplesPerDigit;
	
	int tDrawPointLength = 0;
	for ( int i = 0; i < mPointLength; i += 4 )
	{
		double tSmallValue = 0;
		double tBigValue = 0;
		double tFistValue = 0;
		double tLastValue = 0;
		
		if ( !ValueInRange( int( i / 4 * tSamplesPerDigit ), int( ( i / 4 + 1 ) * tSamplesPerDigit ), tSmallValue, tBigValue, tFistValue, tLastValue, tEachSampleSize ) )
		{
			break;
		}

		mPoints[ i + 0 ].y = DigitWithVoltage( tRect, tFistValue, tDigitPerVoltage );
		mPoints[ i + 0 ].x = i / 4 + mLeftDelta + int( tTimeDigitOffset );
		/*if ( mPoints[ i + 0 ].x < mLeftDelta )
		{
			mPoints[ i + 0 ].x = mLeftDelta - 1;
		}
		else if ( mPoints[ i + 0 ].x > tRect.Width() - mLeftDelta )
		{
			mPoints[ i + 0 ].x = tRect.Width() - mLeftDelta + 1;
		}*/

		mPoints[ i + 1 ].y = DigitWithVoltage( tRect, tBigValue, tDigitPerVoltage );
		mPoints[ i + 1 ].x = mPoints[ i + 0 ].x;				

		mPoints[ i + 2 ].y = DigitWithVoltage( tRect, tSmallValue, tDigitPerVoltage );
		mPoints[ i + 2 ].x = mPoints[ i + 0 ].x;	

		mPoints[ i + 3 ].y = DigitWithVoltage( tRect, tLastValue, tDigitPerVoltage );
		mPoints[ i + 3 ].x = mPoints[ i + 0 ].x;

		tDrawPointLength += 4;
	}	
	
	pMemDC->Polyline( mPoints, tDrawPointLength );

	DrawFrameRect( pMemDC, tRect );
}

long CStaticGraphic::DigitWithVoltage( CRect &tRect, double tVoltage, double tDigitPerVoltage )
{	
	tVoltage += mVoltageOffset;
	return ( long )( ( mBeginVoltage - tVoltage ) * tDigitPerVoltage ) + mTopDelta + tRect.top;
}

bool CStaticGraphic::ValueInRange( int tStart, int tEnd, double &tBigValue, double &tSmallValue, double &tFirstValue, double &tLastValue, int tEachSampleSize )
{	
	if ( tStart >= mLength )
	{
		return false;
	}
	
	tFirstValue = mDataHandler->VoltageWithPointer( &mData[ tStart * tEachSampleSize ] );
	tSmallValue = tFirstValue;
	tBigValue = tFirstValue;
	tLastValue = tFirstValue;

	int tActualLength = min( tEnd, mLength );
	for ( int i = tStart + 1; i < tActualLength; i++ )
	{
		double tValue = mDataHandler->VoltageWithPointer( &mData[ i * tEachSampleSize ] );
		if ( tValue < tSmallValue )
		{
			tSmallValue = tValue;
		}
		else if ( tValue > tBigValue )
		{
			tBigValue = tValue;		
		}

		tLastValue = tValue;
	}
	
	return true;
}

bool CStaticGraphic::ExportDataToFile( CDataExportHelper &tDataExport )
{		
	double tEndVoltage = this->mBeginVoltage - this->mTotalVoltage;

	HSInt tEachSampleSize = mDataHandler->EachSampleSize();
	ostringstream tStringStream;
	for ( int i = 0; i < mLength; i++ )
	{
		double tValue = mDataHandler->VoltageWithPointer( &mData[ i * tEachSampleSize ] );
		tStringStream << tValue;
		tStringStream << "\n";		

		if ( i % 1000 == 0 )
		{			
			tDataExport.Write( tStringStream.str().c_str() ); 				
			tStringStream.str( "" );
		}
	}

	tDataExport.Write( tStringStream.str().c_str() );


	return true;
}
