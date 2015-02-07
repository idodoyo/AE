#pragma once

#include "DataTypes.h"
#include <algorithm>
#include "HSLogProtocol.h"

class CVirtualHitPosition
{
public:
	typedef struct HIT_INFO
	{
		HSDouble X;
		HSDouble Y;
		HSDouble Z;
		HSDouble Energy;
		HSDouble Distance;
		HSInt Channels[ 3 ];

		bool operator<( const HIT_INFO &tValue ) const
		{
			return Distance < tValue.Distance;
		}

	} HIT_INFO;

public:
	static CVirtualHitPosition * SharedInstance()
	{
		static CVirtualHitPosition sVirtualHitPosition;
		return &sVirtualHitPosition;
	}

	HSBool HitWithIndex( HSUInt tIndex, HIT_INFO &tHit )
	{
		WaitForSingleObject( mMutex, INFINITE );

		HSBool tRes = HSFalse;
		if ( tIndex < mHits.size() )
		{
			tHit = mHits[ tIndex ];
			tRes = HSTrue;
		}

		SetEvent( mMutex );

		return tRes;
	}

	HSVoid AddHit( HIT_INFO &tHit )
	{
		WaitForSingleObject( mMutex, INFINITE );

		mHits.push_back( tHit );

		SetEvent( mMutex );
	}

	HSVoid Reset( HSVoid *pParent )
	{
		if ( mParent == NULL )
		{
			mParent = pParent;
		}
		else
		{
			return;
		}

		WaitForSingleObject( mMutex, INFINITE );

		mHits.clear();

		SetEvent( mMutex );
	}

	HSVoid Sort()
	{
		WaitForSingleObject( mMutex, INFINITE );

		std::sort( mHits.begin(), mHits.end() );

		SetEvent( mMutex );
	}

	HSBool EnableStart( HSVoid *pParent )
	{
		return pParent == mParent;
	}

	HSVoid Finish()
	{
		mParent = NULL;
	}

private:
	CVirtualHitPosition()
	{
		mMutex = CreateEvent( NULL, FALSE, TRUE, NULL );
		mParent = NULL;
	}

	~CVirtualHitPosition()
	{
		CloseHandle( mMutex );
	}

private:
	vector< HIT_INFO > mHits;
	HANDLE mMutex;

	HSVoid *mParent;
};




