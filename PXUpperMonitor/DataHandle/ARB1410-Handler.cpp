#include "stdafx.h"
#include "ARB1410-Handler.h"
#include "HSLogProtocol.h"

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
	mARB1410_SETINFO.tAMwaveform.frequency = 100 * 1000;
	mARB1410_SETINFO.tAMwaveform.carrierType = 0;
	mARB1410_SETINFO.tAMwaveform.envelope = 0;
	mARB1410_SETINFO.tAMwaveform.durationUnits = 0;
	mARB1410_SETINFO.tAMwaveform.duration = 10;

	mARB1410_SETINFO.tFrequencySweep.startFrequency = 10 * 1000;
	mARB1410_SETINFO.tFrequencySweep.endFrequency = 100 * 1000;
	mARB1410_SETINFO.tFrequencySweep.duration = 1000;
	mARB1410_SETINFO.tFrequencySweep.minPtsPerCycle = 100;

	mARB1410_SETINFO.tAEwaveform.frequency = 100 * 1000;
	mARB1410_SETINFO.tAEwaveform.riseTime = 50;
	mARB1410_SETINFO.tAEwaveform.fallTime = 500;

	mARB1410_SETINFO.tSignal.coupling = 0;
	mARB1410_SETINFO.tSignal.amplitude = 10 * 1000;
	mARB1410_SETINFO.tSignal.DCoffset = 0;
	mARB1410_SETINFO.tSignal.filter = 2;
	mARB1410_SETINFO.tSignal.delay = 0;

	mARB1410_SETINFO.tTrigger.mode = 1;
	mARB1410_SETINFO.tTrigger.repRate = 100 * 1000;
	mARB1410_SETINFO.tTrigger.outPolarity = 0;
	mARB1410_SETINFO.tTrigger.outWidth = 10;
	mARB1410_SETINFO.tTrigger.inType = 0;
	mARB1410_SETINFO.tTrigger.inSlope = 0;

	mARB1410_SETINFO.WaveformType = 0;
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
	/*
	if ( mDataStreamThread != NULL && mDataStreamThread->IsPaused() )
	{
		mDataStreamThread->Start();	
		return HSTrue;
	}
	
	if ( AWG_open() != 0 )
	{
		HS_ERROR( "Problem encountered. Exiting...\n" );
		return 0;
	}

	AWG_setAMwaveformControls(
		100 *1000,	//int frequency;
		0,			//int carrierType;
		0,			//int envelope;
		0,			//int durationUnits;
		10 			//int duration;
		);

	AWG_setFrequencySweepControls(
		10  *1000,	//int startFrequency;
		100 *1000,	//int endFrequency;
		1000,		//int duration;
		100			//int minPtsPerCycle;
		);

	AWG_setAEwaveformControls(
		100 *1000,	//int frequency;
		50,			//int riseTime;
		500			//int fallTime;
		);


	AWG_setSignalControls(
		0,			//int coupling;
		10 *1000,	//int amplitude;
		0,			//int DCoffset;
		2,			//int filter;
		0 			//int delay;
		);

	AWG_setTriggerControls(
		1,			//int mode;
		100 *1000,	//int repRate;
		0,			//int outPolarity;
		10,			//int outWidth;
		0,			//int inType;
		0 			//int inSlope;
		);

	AWG_setWaveformType( 0 );
	AWG_activateControlSettings();

	AWG_setOutputEnable( 1 );
	
	mArgsInfo.clear();

	mListBuffer->SetCardIdentifier( this->Identifier() );
	mListBuffer->Reset( 1 );
	
	mDataStreamThread = new CThreadControlEx< CARB1410Handler >( this, 0, DATA_STREAM_THREAD_ID );
	mFileThread = new CThreadControlEx< CARB1410Handler >( this, 0, FILE_THREAD_ID );

	mDataStreamThread->Start();
	mFileThread->Start();
	*/
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

	//AWG_close();

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
	CARB1410Handler *pARB1410Handler = new CARB1410Handler( pIndexManager, pFileObserver, 0 );
	pDevices->push_back( pARB1410Handler );

	return 1;

	/*	
	if ( AWG_open() != 0 )
	{		
		return 0;
	}
	
	CARB1410Handler *pARB1410Handler = new CARB1410Handler( pIndexManager, pFileObserver, 0 );
	pDevices->push_back( pARB1410Handler );

	AWG_close();

	HS_INFO( "Find Device: ARB1410" );

	return 1;
	*/
}

HSBool CARB1410Handler::GetDevice( DEVICE_CH_IDENTIFIER tIdentifier, vector< HSInt > &tChannels, vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver )
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
