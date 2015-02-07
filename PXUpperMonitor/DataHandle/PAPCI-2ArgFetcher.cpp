#include "stdafx.h"
#include "PAPCI-2ArgFetcher.h"
#include "PAPCI-2Handler.h"
#include "LinearTransfer.h"
#include "HSLogProtocol.h"

CPAPCI2ArgFetcher::CPAPCI2ArgFetcher( CArgHandler *pParent ) : mParent( pParent )
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
		
		mParent->AddArg( tChannelIdentifier, &tArgItem, 0.0 );

		mArgIndex++;
	}

	mParent->AddArg( tChannelIdentifier, NULL, 1.0 );
}

HSInt CPAPCI2ArgFetcher::BufferSize()
{
	return 32768;
}




