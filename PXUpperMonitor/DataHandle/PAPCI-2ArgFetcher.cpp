#include "stdafx.h"
#include "PAPCI-2ArgFetcher.h"
#include "PAPCI-2Handler.h"
#include "LinearTransfer.h"
#include "HSLogProtocol.h"

CPAPCI2ArgFetcher::CPAPCI2ArgFetcher( CPAPCI2Handler *pParent )	: mParent( pParent )
{
	mArgIndex = -1;	
}

CPAPCI2ArgFetcher::~CPAPCI2ArgFetcher()
{
}

HSVoid CPAPCI2ArgFetcher::FetchArg( DEVICE_CH_IDENTIFIER tChannelIdentifier, CLinearTransfer *pLinearTransfer, HSDouble tTime )
{
	if ( mArgIndex == -1 )
	{
		if ( !pLinearTransfer->Seek( 0.0, 0.0, CLinearTransfer::SET_SAMPLE ) )
		{
			mParent->AddArg( tChannelIdentifier, NULL, 1.0 );
			return;			
		}

		mArgIndex = 0;
	}

	CArgTableManager::ArgItem tArgItem;
	HSChar *pBuf = NULL;
	HSFloat tValue = 0;

	HSUInt64 tEndTime = ( HSUInt64 )( ( tTime + 1.0 ) * 1000000000.0 );

	while ( pLinearTransfer->Read( mArgIndex, pBuf ) )
	{	
		tArgItem = *( CArgTableManager::ArgItem * )pBuf;
		if ( tArgItem.NBeginTime > tEndTime )
		{
			break;
		}

		/*
		HSByte *pMsgBuf = ( HSByte * )pBuf; 
		memset( &tArgItem, 0, sizeof tArgItem );

		getHitDataSetValue( pMsgBuf, TIME, &tValue );
		HSDouble tDBValue = tValue;

		HS_INFO( "Hit Time: %f, Cur Time: %f, Type: %d", tDBValue, tTime + 1.0, pBuf[ 2 ] );

		if ( tDBValue > ( tTime + 1.0 ) )
		{
			break;
		}

		tArgItem.NBeginTime = ( HSUInt64 )( tDBValue * 1000000000.0 );

		getHitDataSetValue( pMsgBuf, RISETIME, &tValue );
		tDBValue = tValue;
		tArgItem.IncreaseNTime = ( HSUInt64 )( tDBValue * 1000.0 );

		getHitDataSetValue( pMsgBuf, DURATION, &tValue );
		tDBValue = tValue;
		tArgItem.NDuration = ( HSUInt64 )( tDBValue * 1000.0 );

		getHitDataSetValue( pMsgBuf, ENERGY, &tValue );
		tDBValue = tValue;
		tArgItem.Energy = tDBValue;

		getHitDataSetValue( pMsgBuf, COUNTS, &tValue );		
		tArgItem.RingCount = ( HSUInt )tValue;

		getHitDataSetValue( pMsgBuf, AMPLITUDE, &tValue );
		tDBValue = tValue;
		tArgItem.Amplitude = tDBValue;

		HS_INFO( "NBegin Time: %lld, Rise Time: %lld, Duation: %lld, Energy: %f, Count: %d, Amplitude: %f\n", tArgItem.NBeginTime, tArgItem.IncreaseNTime, tArgItem.NDuration, tArgItem.Energy, tArgItem.RingCount, tArgItem.Amplitude );
		*/
		mParent->AddArg( tChannelIdentifier, &tArgItem, 0.0 );

		mArgIndex++;
	}

	mParent->AddArg( tChannelIdentifier, NULL, 1.0 );
}

HSInt CPAPCI2ArgFetcher::BufferSize()
{
	return 32768;
}




