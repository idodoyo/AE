#include "stdafx.h"
#include "ARB1410-Handler.h"
#include "HSLogProtocol.h"
#include "WaveGenClient.h"

CARB1410Handler::CARB1410Handler(  CIndexManager *pIndexManager, CFileObserver *pFileObserver, HSInt tIndex )
	: mIndexManager( pIndexManager )
{
	mListBuffer = new CListBuffer( 10 * 1024 * 1024, 0, 2 );
	mListBuffer->SetFileObserver( pFileObserver );

	mDataIdentifier.CARD_INDEX = tIndex;
	mDataIdentifier.CARD_IDENTIFIER = CARD_ARB1410;

	mSampleInOneFrame = 5 * 1024;

	mDataStreamThread = NULL;
	mFileThread = NULL;

	mBypassVoltages[ 0 ] = 0.0;	

	mSampleRate = 1 * 1000 * 1000;

	mIsOn = HSFalse;

	mFrequcencyMod.StartFrequency = 10  *1000;
	mFrequcencyMod.EndFrequency = 100 *1000;
	mFrequcencyMod.Duration = 1000;
	mFrequcencyMod.MinPtsPerCycle = 100;

	mSignalSet.Coupling = 0;
	mSignalSet.Amplitude = 10 *1000;
	mSignalSet.DCoffset = 0;
	mSignalSet.Filter = 2;
	mSignalSet.Delay = 0;

	mTriggerSet.Mode = 1;
	mTriggerSet.RepRate = 100 *1000;
	mTriggerSet.OutPolarity = 0;
	mTriggerSet.OutWidth = 10;
	mTriggerSet.Intype = 0;
	mTriggerSet.InSlope = 0;
}

CARB1410Handler::~CARB1410Handler()
{
	delete mListBuffer;	
}

HSString CARB1410Handler::Name()
{
	HSChar tBuf[ 32 ];
	sprintf_s( tBuf, "ARB1410-%02d", mDataIdentifier.CARD_INDEX + 1 );
	return tBuf; 
}

HSBool CARB1410Handler::Start()
{
	if ( !IsOn() )
	{
		return HSFalse;
	}
	
	if ( mDataStreamThread != NULL && mDataStreamThread->IsPaused() )
	{
		mDataStreamThread->Start();	
		return HSTrue;
	}
	
	if ( CWaveGenClient::SharedInstance()->OpenAWG() != 0 )
	{
		HS_ERROR( "Problem encountered. Exiting...\n" );
		return HSFalse;
	}

	EffectiveSettings();
	
	mArgsInfo.clear();

	mListBuffer->SetCardIdentifier( this->Identifier() );
	mListBuffer->Reset( 1 );
	
	mDataStreamThread = new CThreadControlEx< CARB1410Handler >( this, 0, DATA_STREAM_THREAD_ID );
	mFileThread = new CThreadControlEx< CARB1410Handler >( this, 0, FILE_THREAD_ID );

	mDataStreamThread->Start();
	mFileThread->Start();
	
	return HSTrue;
}

HSVoid CARB1410Handler::Pause()
{
	if ( mDataStreamThread != NULL )
	{
		mDataStreamThread->Pause();
	}
}

HSVoid CARB1410Handler::Stop()
{
	if ( mDataStreamThread == NULL )
	{
		return;
	}

	CWaveGenClient::SharedInstance()->CloseAWG();

	mDataStreamThread->Stop();
	Sleep( 100 );

	mFileThread->Stop();
}

HSUInt CARB1410Handler::GetChannelData( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSChar *pSrcBuf, HSChar *pDestBuf, HSInt tOffset, HSInt tSamples )
{	
	HSInt tLength = min( mSampleInOneFrame - tOffset, tSamples );
	memcpy( pDestBuf, &pSrcBuf[ tOffset * 4 ], 4 * tLength );

	return tLength;	
}

HSBool CARB1410Handler::OpenChannel( vector< HSInt > &tChannels, vector< HSInt > *pSampleRates, HSString *pErrorMsg )
{
	CloseAllChannel();

	mIsOn = HSTrue;
	if ( pSampleRates != NULL )
	{
		mSampleRate = ( *pSampleRates )[ 0 ];
	}

	return HSTrue;
}

HSBool CARB1410Handler::CloseAllChannel()
{
	mIsOn = HSFalse;

	return HSTrue;
}

HSBool CARB1410Handler::IsOn()
{
	return mIsOn;
}

HSBool CARB1410Handler::EffectiveSettings()
{
	CWaveGenClient::SharedInstance()->SetFrequencySweepControls( mFrequcencyMod.StartFrequency, mFrequcencyMod.EndFrequency, mFrequcencyMod.Duration, mFrequcencyMod.MinPtsPerCycle );

	CWaveGenClient::SharedInstance()->SetSignalControls( mSignalSet.Coupling, mSignalSet.Amplitude, mSignalSet.DCoffset, mSignalSet.Filter, mSignalSet.Delay );
	
	CWaveGenClient::SharedInstance()->SetTriggerControls( mTriggerSet.Mode, mTriggerSet.RepRate, mTriggerSet.OutPolarity, mTriggerSet.OutWidth, mTriggerSet.Intype, mTriggerSet.InSlope );

	CWaveGenClient::SharedInstance()->SetWaveformType( 0 );

	CWaveGenClient::SharedInstance()->ActivateControlSettings();

	CWaveGenClient::SharedInstance()->SetOutputEnable( 0 );

	return HSTrue;
}

HSBool CARB1410Handler::ThreadRuning( HSInt tThreadID )
{
	if ( tThreadID == DATA_STREAM_THREAD_ID )
	{
		
	}
	else if ( tThreadID == FILE_THREAD_ID )
	{
		HSChar *pBuf;
		HSInt tLength = 0;
		HSInt tBufferIndex = mListBuffer->BeginReadBuffer( pBuf, tLength );
		if ( tBufferIndex >= 0 ) 
		{
			mListBuffer->FileObserver()->Update( this->Identifier(), pBuf, tLength );
			mListBuffer->FinishReadBuffer( tBufferIndex );
		}
		else
		{
			Sleep( 10 );
		}
	}

	return HSTrue;
}

HSVoid CARB1410Handler::ThreadWillStop( HSInt tThreadID )
{
	if ( tThreadID == FILE_THREAD_ID )
	{
		HSChar *pBuffer = NULL;
		HSInt tLength = 0;
		if ( mListBuffer->WritingBuffer( pBuffer, tLength ) )
		{
			mListBuffer->FileObserver()->Update( this->Identifier(), pBuffer, tLength );
		}

		delete mFileThread;
		mFileThread = NULL;
	}	
	else if ( tThreadID == DATA_STREAM_THREAD_ID )
	{		
		delete mDataStreamThread;
		mDataStreamThread = NULL;
	}
}

HSBool CARB1410Handler::EffectiveTest( HSChar *pBuffer )
{
	return HSTrue;
}

HSInt CARB1410Handler::GetDevice( vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver )
{	
	if ( CWaveGenClient::SharedInstance()->OpenAWG() != 0 )
	{		
		return 0;
	}
	
	CARB1410Handler *pARB1410Handler = new CARB1410Handler( pIndexManager, pFileObserver, 0 );
	pDevices->push_back( pARB1410Handler );

	CWaveGenClient::SharedInstance()->CloseAWG();

	HS_INFO( "Find Device: ARB1410" );

	return 1;
}

HSBool CARB1410Handler::GetDevice( DEVICE_CH_IDENTIFIER tIdentifier, HSUInt tVersion, HSInt64 tDeviceSavePos, vector< HSInt > &tChannels, vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver )
{
	HSInt tIndex = tIdentifier.CARD_INDEX;
	HSInt tCardIdentifier = tIdentifier.CARD_IDENTIFIER;
	switch ( tCardIdentifier )
	{
	case CARD_ARB1410:
		{
			CARB1410Handler *pARB1410Handler = new CARB1410Handler( pIndexManager, pFileObserver, tIndex );

			pARB1410Handler->ListBuffer()->SetCardIdentifier( pARB1410Handler->Identifier() );
			pARB1410Handler->ListBuffer()->InitWithFile();

			pDevices->push_back( pARB1410Handler );

			HS_INFO( "Open Device: ARB1410" );
		}
		return HSTrue;

	default:
		return HSFalse;
	}
}
