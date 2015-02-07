#include "stdafx.h"
#include "ArgHandler.h"


CArgHandler::CArgHandler()
{
	mArgMutex = CreateEvent( NULL, FALSE, TRUE, NULL );
}

CArgHandler::~CArgHandler()
{
	CloseHandle( mArgMutex );
}

HSBool CArgHandler::GetArgList( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSUInt64 tNBeginTime, HSUInt64 tNDuration, vector< CArgTableManager::ArgItem > &tArgList, HSDouble &tLastSecond, HSDouble &tLastArgSecond )
{
	if ( mBypassVoltages.find( tChannelIdentifier.ChannelIndex() ) == mBypassVoltages.end() 
		|| int( mBypassVoltages[ tChannelIdentifier.ChannelIndex() ] * 1000000 ) == 0 )
	{
		return HSTrue;
	}

	WaitForSingleObject( mArgMutex, INFINITE );

	map< DEVICE_CH_IDENTIFIER, ArgInfo >::iterator pIterator = mArgsInfo.find( tChannelIdentifier );
	if ( pIterator == mArgsInfo.end() )
	{
		SetEvent( mArgMutex );
		return HSFalse;
	}

	tLastArgSecond = pIterator->second.Arg.GetArg( tNBeginTime, tNDuration, tArgList );	
	tLastSecond = pIterator->second.LastSecond;

	SetEvent( mArgMutex );

	return tArgList.size() > 0;	
}

HSVoid CArgHandler::AddArg( DEVICE_CH_IDENTIFIER tChannelIdentifier, CArgTableManager::ArgItem *pItem, HSDouble tLastSecond )
{
	WaitForSingleObject( mArgMutex, INFINITE );	

	map< DEVICE_CH_IDENTIFIER, ArgInfo >::iterator pIterator = mArgsInfo.find( tChannelIdentifier );
	if ( pIterator == mArgsInfo.end() )
	{
		mArgsInfo[ tChannelIdentifier ].LastSecond = 0;
	}

	mArgsInfo[ tChannelIdentifier ].LastSecond += tLastSecond;

	if ( pItem != NULL )
	{
		mArgsInfo[ tChannelIdentifier ].Arg.AddArg( *pItem );			
	}		

	SetEvent( mArgMutex );
}

HSVoid CArgHandler::SetBypassVoltage( HSUInt tChannel, HSDouble tVoltage )
{
	if ( int( tVoltage * 1000000 ) != int( mBypassVoltages[ tChannel ] * 1000000 ) )
	{
		mBypassVoltages[ tChannel ] = tVoltage;

		DEVICE_CH_IDENTIFIER tIdentifier = this->Identifier();
		tIdentifier.InitChannel( tChannel );

		WaitForSingleObject( mArgMutex, INFINITE );	

		if ( mArgsInfo.find( tIdentifier ) != mArgsInfo.end() )		
		{
			mArgsInfo[ tIdentifier ].Arg.Reset();
		}

		SetEvent( mArgMutex );
	}
}

HSUInt64 CArgHandler::GetSampleNSecond( HSUInt tSampleReadIndex, HSUInt tSampleReadOffset, DEVICE_CH_IDENTIFIER tChannelIdentifier )
{		
	HSUInt tSamplesInFrame = SamplesInFrame( tChannelIdentifier );	
	if ( tSamplesInFrame == 0 )
	{
		return 0;
	}

	HSUInt64 tBufferSize = tSampleReadIndex * tSamplesInFrame;
	tBufferSize += tSampleReadOffset;	

	return tBufferSize * 1000000000 / this->SampleRate( tChannelIdentifier );
	
}