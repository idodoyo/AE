#pragma once

#include "DataTypes.h"

#include <complex>
#include <math.h>

using std::complex;

class CFourierTransform
{
public:
	CFourierTransform()
	{		
		mXin = NULL;
		mResData = NULL;
		mResHz = NULL;
		mLength = 0;	
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
	
	HSVoid Scale( complex< HSDouble > *pData, const HSUInt N )
	{
		const double tFactor = 1.0 / double( N );

		for ( HSUInt tPosition = 0; tPosition < N; tPosition++ )
		{
			pData[ tPosition ] *= tFactor;
		}
	}

	HSVoid Rearrange( complex< HSDouble > *pData, const HSUInt N )
	{		
		HSUInt tTarget = 0;
		
		for ( HSUInt tPosition = 0; tPosition < N; tPosition++ )
		{			
			if ( tTarget > tPosition )
			{			
				const complex< HSDouble > tTemp( pData[ tTarget ] );
				pData[ tTarget ] = pData[ tPosition ];
				pData[ tPosition ] = tTemp;
			}
			
			HSUInt tMask = N;			
			while ( tTarget & ( tMask >>= 1 ) )
			{
				tTarget &= ~tMask;
			}
			
			tTarget |= tMask;
		}
	}

	HSVoid Perform( complex< HSDouble > *pData, const HSUInt N, const HSBool tInverse = HSFalse )
	{
		const HSDouble PI = tInverse ? 3.14159265358979323846 : -3.14159265358979323846;
		for ( HSUInt tStep = 1; tStep < N; tStep <<= 1 )
		{		
			const HSUInt tJump = tStep << 1;
			const HSDouble tDelta = PI / HSDouble( tStep );
			const HSDouble tSine = sin( tDelta * 0.5 );
			const complex< HSDouble > tMultiplier( -2. * tSine * tSine, sin( tDelta ) );

			complex< HSDouble > tFactor( 1.0 );

			for ( HSUInt tGroup = 0; tGroup < tStep; tGroup++ )
			{
				for ( HSUInt tPair = tGroup; tPair < N; tPair += tJump )
				{
					const HSUInt tMatch = tPair + tStep;
					const complex< HSDouble > Product( tFactor * pData[ tMatch ] );
					pData[ tMatch ] = pData[ tPair ] - Product;
					pData[ tPair ] += Product;
				}
				tFactor = tMultiplier * tFactor + tFactor;
			}
		}
	}	

	HSBool Inverse( complex< HSDouble > *pData, const HSUInt N, const HSBool tScale = HSTrue )
	{
		if ( !pData || N < 1 || N & ( N - 1 ) )
			return HSFalse;

		Rearrange( pData, N );

		Perform( pData, N, HSTrue );

		if ( tScale )
		{
			Scale( pData, N );
		}

		return HSTrue;
	}	

	HSBool Forward( complex< HSDouble > *pData, const HSUInt N )
	{	
		if ( !pData || N < 1 || N & ( N - 1 ) )
			return HSFalse;

		Rearrange( pData, N );

		Perform( pData, N );

		return HSTrue;
	}
	
	HSBool FFT( HSFloat *pData, HSInt tLength, HSDouble tSampleRate, HSDouble * &pResData, HSDouble * &pResHz )
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

		if ( !Forward( mXin, tLength ) )
		{
			return HSFalse;
		}

		for ( HSInt i = 0; i < tLength; i++ ) 
		{
			mResData[ i ] = sqrt( mXin[ i ].real() * mXin[ i ].real() + mXin[ i ].imag() * mXin[ i ].imag() );
			mResData[ i ] = 10 * log10( mResData[ i ] );
		}

		pResData = mResData;
		pResHz = mResHz;

		return HSTrue;
	}

private:
	complex< HSDouble > *mXin;
	HSDouble *mResData;
	HSDouble *mResHz;
	HSInt mLength;
};