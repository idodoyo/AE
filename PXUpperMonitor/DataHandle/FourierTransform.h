#pragma once

#include "DataTypes.h"

#include <complex>
#include <math.h>

using std::complex;

class IFourierTransformResHandleProtocol
{
public:
	virtual HSVoid Handle( HSDouble &tValue ) = 0;

	virtual ~IFourierTransformResHandleProtocol(){}
};

class CFourierTransform
{
public:
	CFourierTransform( IFourierTransformResHandleProtocol *pHandler = NULL )
	{		
		mXin = NULL;
		mResData = NULL;
		mResHz = NULL;
		mLength = 0;

		mHandler = pHandler;
	}

	~CFourierTransform()
	{
		if ( mXin != NULL )
		{
			delete[] mXin;
			delete[] mResData;
			delete[] mResHz;
		}
	}

	HSVoid FFT( HSFloat *pData, HSInt tLength, HSDouble tSampleRate, HSDouble * &pResData, HSDouble * &pResHz )
	{  
		if ( mLength != tLength )
		{
			if ( mXin != NULL )
			{
				delete[] mXin;
				delete[] mResData;
				delete[] mResHz;				
			}

			mXin = new std::complex< HSDouble >[ tLength ];
			mResData = new HSDouble[ tLength ];
			mResHz = new HSDouble[ tLength ];
			mLength = tLength;
		}

		tSampleRate /= mLength;
		for ( HSInt i = 0; i < mLength; i++ )
		{
			mResHz[ i ] = i * tSampleRate;
		}
		
		for ( HSInt i = 0; i < tLength; i++ )
		{
			mXin[ i ].real( pData[ i ] );
			mXin[ i ].imag( 0 );
		}
		
		HSInt tNv2 = tLength / 2;
		HSInt tNm1 = tLength - 1;
		HSInt j = 0;
		for ( HSInt i = 0; i < tNm1; i++ )
		{
			if ( i < j )
			{
				std::swap( mXin[ j ], mXin[ i ] );
			}

			HSInt k = tNv2;
			while ( k <= j )
			{
				j = j - k;
				k = k / 2;
			}
			j = j + k;
		}

		HSDouble PI = 3.1415926535897932384626433832795028841971;
		HSInt tTmpLength = tLength;

		HSInt tL = 0;
		for ( tL = 1; ( tTmpLength = tTmpLength / 2 ) != 1; tL++ );

		HSInt tLe = 0; 
		HSInt tLei = 0;   
		HSInt tIP = 0;
		complex< HSDouble > tU;
		complex< HSDouble > tW;
		complex< HSDouble > tT;
		for ( HSInt m = 1; m <= tL; m++ )
		{
			tLe = 2 << ( m - 1 ); 
			tLei = tLe / 2; 

			tU.real( 1.0 );
			tU.imag( 0.0 );
			tW.real( cos( PI / tLei ) );
			tW.imag( -sin( PI / tLei ) );

			for ( HSInt j = 0; j <= tLei - 1; j++ ) 
			{
				for ( HSInt i = j; i <= tLength - 1; i = i + tLe ) 
				{
					tIP = i + tLei;
					tT = mXin[ tIP ] * tU;
					mXin[ tIP ].real( mXin[ i ].real() - tT.real() );
					mXin[ tIP ].imag( mXin[ i ].imag() - tT.imag() );
					mXin[ i ].real( mXin[ i ].real() + tT.real() );
					mXin[ i ].imag( mXin[ i ].imag() + tT.imag() );
				}

				tU = ( tU * tW );
			}
		}			
				
		for ( HSInt i = 0; i < tLength; i++ ) 
		{
			mResData[ i ] = sqrt( mXin[ i ].real() * mXin[ i ].real() + mXin[ i ].imag() * mXin[ i ].imag() );
			if ( mHandler )
			{
				mResData[ i ] /= ( tLength / 2 );
				mHandler->Handle( mResData[ i ] );
			}			
		}

		pResData = mResData;
		pResHz = mResHz;
	}

private:
	complex< HSDouble > *mXin;
	HSDouble *mResData;
	HSDouble *mResHz;
	HSInt mLength;

	IFourierTransformResHandleProtocol *mHandler;
};