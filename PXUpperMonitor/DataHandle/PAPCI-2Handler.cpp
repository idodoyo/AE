#include "stdafx.h"
#include "PAPCI-2Handler.h"
#include "HSLogProtocol.h"

HSBool CPAPCI2Handler::mSuccessInit = HSFalse;

CPAPCI2Handler::CPAPCI2Handler(  CIndexManager *pIndexManager, CFileObserver *pFileObserver, HSInt tIndex )
	: mIndexManager( pIndexManager )
{
	mListBuffer = new CListBuffer( 10 * 1024 * 1024, 0, 2 );
	mListBuffer->SetFileObserver( pFileObserver );

	mDataIdentifier.CARD_INDEX = tIndex;
	mDataIdentifier.CARD_IDENTIFIER = CARD_PAPCI2;	

	mWriteDataMutex = CreateEvent( NULL, FALSE, TRUE, NULL );

	mSampleInOneFrame = 5 * 1024;

	mDataStreamThread = NULL;
	mDataMessageThread = NULL;
	mFileThread = NULL;

	mBypassVoltages[ 0 ] = 0.0;
	mBypassVoltages[ 1 ] = 0.0;

	mDefaultSeting.On = HSFalse;
	mDefaultSeting.Buffer = NULL;
	mDefaultSeting.SampleLength = 0;
	mDefaultSeting.SampleRate = 500;
	mDefaultSeting.ThresholdType = 0;
	mDefaultSeting.Threshold = 60;
	mDefaultSeting.ThresholdDeadband = 10;
	mDefaultSeting.Gain = 0;
	mDefaultSeting.HDT = 800;
	mDefaultSeting.PDT = 200;
	mDefaultSeting.HLT = 1000;
	mDefaultSeting.MaxDuration = 10;
	mDefaultSeting.WaveformLength = 1;
	mDefaultSeting.PreTrigger = 100;
	mDefaultSeting.LowFilter = 0;
	mDefaultSeting.HighFilter = 2;
	mDefaultSeting.AmpGain = 40;
	mDefaultSeting.DrivenRate = 1000;
	mDefaultSeting.RMS_ASL_Time = 500;
	mDefaultSeting.CounterThreshold = 3000;
}

CPAPCI2Handler::~CPAPCI2Handler()
{
	delete mListBuffer;

	CloseHandle( mWriteDataMutex );
}

HSString CPAPCI2Handler::Name()
{
	HSChar tBuf[ 32 ];
	sprintf_s( tBuf, "PAPCI2-%02d", mDataIdentifier.CARD_INDEX + 1 );
	return tBuf; 
}

HSVoid CPAPCI2Handler::SetupPCI2Channel( HSInt tChannel )
{   
	HS_INFO( "Setup Channel: %d", tChannel );

	/* Enable AE channel */
	setChannel( tChannel, ENABLE );       
    
	/* Set the sample rate in Ksamples/s, 1000 = 1Msample/s. */
	setSampleRate( tChannel, mChannels[ tChannel ].SampleRate );

	/* Set the threshold type, 0=fixed, 1=floating. */
	setChannelThresholdType( tChannel, mChannels[ tChannel ].ThresholdType );

	/* Deadband used for floating threshold only, specified in dB */
	setChannelFloatingThresholdDeadband( tChannel, mChannels[ tChannel ].ThresholdDeadband );

	/* Set the threshold, in dB for the channel */
	setChannelThreshold( tChannel, mChannels[ tChannel ].Threshold );

	/* 0 Gain, could also be 6 */
	setChannelGain( tChannel, mChannels[ tChannel ].Gain ); 

	/* Set hit definition time in us, 2us steps */
	setChannelHDT( tChannel, mChannels[ tChannel ].HDT ); 

	/* Set the peak defintion time in us, 1us steps */
	setChannelPDT( tChannel, mChannels[ tChannel ].PDT ); 

	/* Set the hit lockout time in us, 2us steps */
	setChannelHLT( tChannel, mChannels[ tChannel ].HLT );   

	/* Limit a hit to no longer than 10ms duration */
	setChannelMaxDuration( tChannel, mChannels[ tChannel ].MaxDuration );

	/* Waveform length (in kSamples, 1kSample=1024 samples) */
	setWaveformLength( tChannel, mChannels[ tChannel ].WaveformLength );

	/* Set the pre-trigger length in samples */
	setWaveformPreTrigger( tChannel, mChannels[ tChannel ].PreTrigger );  

	/* Set lowpass and highpass filter setting */
	setAnalogFilter( tChannel, mChannels[ tChannel ].LowFilter, mChannels[ tChannel ].HighFilter ); 

	/* Set preamplifier gain */
	setPreAmpGain( tChannel, mChannels[ tChannel ].AmpGain ); 

    /* Generate a Time Driven message every 1 second, specified in milliseconds */
	setTimeDrivenRate( mChannels[ tChannel ].DrivenRate );

    /* Set the time constant for RMS and ASL in milliseconds */
	setRMS_ASL_TimeConstant( mChannels[ tChannel ].RMS_ASL_Time );

    /* We want Parametrics 1 & 2 data reported in the TD messages */
    setTimeDrivenParametric( PARAM1, ENABLE );
    setTimeDrivenParametric( PARAM2, ENABLE );
    
    /* If you want the cycle counter reported in the TD message, enable this */
    setTimeDrivenParametric( CYCLES, DISABLE );

    /* no gain on parametric 1 */
    setParametricGain( PARAM1, 0 ); 
    
    /* no filter on either parametric */
    setParametricFilter( PARAM1, 0 );
    setParametricFilter( PARAM2, 0 );

    /* We want RMS and ASL for each enabled AE channel
       reported in the TD messages */
    setTimeDrivenFeature( RMS, ENABLE );
    setTimeDrivenFeature( ASL, ENABLE );

    /* Hit features we want in the hit messages */
    setHitFeature( AMPLITUDE, ENABLE );
    setHitFeature( ENERGY, ENABLE );
    setHitFeature( COUNTS, ENABLE );
    setHitFeature( DURATION, ENABLE ); 
	setHitFeature( RISETIME, ENABLE ); 

    /* Put parametrics 1 and 2 in the hit 
	data set also do the cycle counter */
    setHitParametric( PARAM1, ENABLE );
    setHitParametric( PARAM2, ENABLE );
    setHitParametric( CYCLES, ENABLE );

    /* Use parametric 1 as cycle counter input
	   3V threshold and filter enabled */
    setCycleCounterSource( PARAM1 );
	setCycleCounterThreshold( mChannels[ tChannel ].CounterThreshold );
    setCycleCounterFilter( ENABLE );

    /* Enable waveform collection, start with waveform transfer enabled */
	//setWaveformCollection( ENABLE ); 
    setWaveformTransfer( ENABLE );	

	setWaveformStreamingChannel( tChannel, TRUE );

	/* tChannel, 2 ksample pretrigger */
    setWaveformStreamingLength( tChannel, 2, mSampleInOneFrame / 1024 );
}

HSVoid CPAPCI2Handler::SetupChannelBuffer( HSInt tChannel )
{	
	mChannels[ tChannel ].SampleLength = mSampleInOneFrame;
	HSULong tLength = getRequiredSampleBufferLen( tChannel );
	mChannels[ tChannel ].Buffer = new HSFloat[ tLength ];	

	//HS_INFO( "Requried Buffer Length: %d", tLength );

    /* Give the buffers to the driver */   
	setStreamingBuffer( tChannel, mChannels[ tChannel ].Buffer, tLength, ENABLE );
}

HSVoid CPAPCI2Handler::InitBuffer()
{
	map< HSUInt, CHANNEL_INFO >::iterator pIterator = mChannels.begin();
	while ( pIterator != mChannels.end() )
	{
		if ( pIterator->second.Buffer != NULL )
		{
			delete[] pIterator->second.Buffer;
			pIterator->second.Buffer = NULL;
		}

		pIterator++;
	}
}

HSBool CPAPCI2Handler::Start()
{
	if ( !IsOn() )
	{
		return HSFalse;
	}

	if ( mDataStreamThread != NULL && mDataStreamThread->IsPaused() )
	{
		resumeTest();

		mDataStreamThread->Start();
		mDataMessageThread->Start();
		return HSTrue;
	}

	HSInt tSampleDataChannels = 0;
	map< HSUInt, CHANNEL_INFO >::iterator pIterator = mChannels.begin();
	while ( pIterator != mChannels.end() )
	{
		setChannel( pIterator->first, DISABLE );
		if ( pIterator->second.On )
		{
			SetupPCI2Channel( pIterator->first );
			tSampleDataChannels++;
		}

		pIterator++;
	}

	if ( tSampleDataChannels == 0 )
	{			
		return HSFalse;
	}

	setWaveformStreamingMode( STREAMTRIG_HIT ); 
	//setWaveformStreamingPeriod( 1 );   
    enableWFSOutput( 0 );
	enableWaveformStreaming( TRUE );

	// int tStartSegment[ 4 ] = {  0, 128, 256, 384 };
    // int tEndSegment[ 4 ]   = { 127, 255, 383, 511 };
   //  setHitFftFrequencySpan( 0, 511, 4, tStartSegment, tEndSegment ); 

	pIterator = mChannels.begin();
	while ( pIterator != mChannels.end() )
	{		
		if ( pIterator->second.On )
		{
			SetupChannelBuffer( pIterator->first );
		}

		pIterator++;
	}		

	mStartStreamEvent = CreateEvent( NULL, TRUE, FALSE, "StartStream" );
	if ( mStartStreamEvent == NULL )
    {
        HS_ERROR( "Create Start Event failed (%d)", GetLastError() );       
		InitBuffer();

        return HSFalse;
    }

	mStopStreamEvent = CreateEvent( NULL, TRUE, FALSE, "StopStream" );
	if ( mStopStreamEvent == NULL ) 
	{
		HS_ERROR( "Create Stop Event failed (%d)", GetLastError() );
		CloseHandle( mStartStreamEvent );       
		InitBuffer();

        return HSFalse;
	}

	mExitEvent = CreateEvent( NULL, TRUE, FALSE, "ExitStream" );
	if ( mExitEvent == NULL ) 
	{
		HS_ERROR( "Create Exit Event failed (%d)", GetLastError() );
		CloseHandle( mStartStreamEvent );
		CloseHandle( mStopStreamEvent );       
		InitBuffer();

        return HSFalse;
	}

	setStreamingEvents( mStartStreamEvent, mStopStreamEvent );

	mListBuffer->SetCardIdentifier( this->Identifier() );	
	mListBuffer->Reset( 1 );

	if ( validateSetup() != GOOD || startTest() != GOOD )
	{
		HS_ERROR( "validateSetup Or startTest Error!" );		
		CloseHandle( mStartStreamEvent );
		CloseHandle( mStopStreamEvent );
		CloseHandle( mExitEvent );		
		InitBuffer();
		
        return HSFalse;
	}

	mArgsInfo.clear();
	
	mDataStreamThread = new CThreadControlEx< CPAPCI2Handler >( this, 0, DATA_STREAM_THREAD_ID );	
	mDataMessageThread = new CThreadControlEx< CPAPCI2Handler >( this, 0, DATA_MESSAGE_THREAD_ID );	
	mFileThread = new CThreadControlEx< CPAPCI2Handler >( this, 0, FILE_THREAD_ID );

	mDataStreamThread->Start();
	mDataMessageThread->Start();
	mFileThread->Start();

	return HSTrue;
}

HSVoid CPAPCI2Handler::Pause()
{
	if ( mDataStreamThread != NULL )
	{
		mDataStreamThread->Pause();
		mDataMessageThread->Pause();

		pauseTest();
	}
}

HSVoid CPAPCI2Handler::Stop()
{
	if ( mDataStreamThread == NULL )
	{
		return;
	}	

	mDataMessageThread->Stop();			
	Sleep( 100 );

	mFileThread->Stop();

	if ( stopTest() != GOOD )
	{
		abortTest();
	}		
}

HSUInt CPAPCI2Handler::GetChannelData( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSChar *pSrcBuf, HSChar *pDestBuf, HSInt tOffset, HSInt tSamples )
{	
	HSInt tLength = min( mSampleInOneFrame - tOffset, tSamples );
	memcpy( pDestBuf, &pSrcBuf[ tOffset * 4 ], 4 * tLength );

	return tLength;	
}

HSBool CPAPCI2Handler::OpenChannel( vector< HSInt > &tChannels, vector< HSInt > *pSampleRates, HSString *pErrorMsg )
{
	CloseAllChannel();

	for ( HSUInt i = 0; i < tChannels.size(); i++ )
	{		
		mChannels[ tChannels[ i ] + 1 ].On = HSTrue;
		if ( pSampleRates != NULL )
		{
			mChannels[ tChannels[ i ] + 1 ].SampleRate = ( *pSampleRates )[ i ] / 1000;
		}
	}

	return HSTrue;
}

HSBool CPAPCI2Handler::CloseAllChannel()
{
	map< HSUInt, CHANNEL_INFO >::iterator pIterator = mChannels.begin();
	while ( pIterator != mChannels.end() )
	{
		pIterator->second.On = HSFalse;

		pIterator++;
	}

	return HSTrue;
}

HSBool CPAPCI2Handler::IsOn()
{
	map< HSUInt, CHANNEL_INFO >::iterator pIterator = mChannels.begin();
	while ( pIterator != mChannels.end() )
	{
		if ( pIterator->second.On )
		{
			return HSTrue;
		}

		pIterator++;
	}

	return HSFalse;
}

HSVoid CPAPCI2Handler::HandleMessage()
{
	HSByte *pMsgBuf = mMessage;   
	HSFloat tValue = 0;
	vector< HSInt > tChannels;

	CArgTableManager::ArgItem tArgItem;
	memset( &tArgItem, 0, sizeof tArgItem );

	//HS_INFO( "Handle Message: %d", mMessage[ 2 ] );

	HSChar *pSaveBuf = ( HSChar * )&tArgItem;
	HSInt tSaveBufLength = sizeof( tArgItem );	

    switch ( mMessage[ 2 ] )
	{
		case 10:    /* message id for AST hit */
		case 1:     /* AE hit message */
			if ( getHitDataSetValue( pMsgBuf, CHANNEL, &tValue ) == GOOD && mChannels.find( ( HSUInt )tValue ) != mChannels.end() )
			{
				tChannels.push_back( ( HSUInt )tValue );

				HSFloat tWaveChannel = tValue;

				getHitDataSetValue( pMsgBuf, TIME, &tValue );
				
				HSDouble tHitTime = tValue;
				mWaveStreamTime = ( HSUInt64 )( tHitTime * 1000000000.0 );

				HSDouble tDBValue = tValue;

				tArgItem.NBeginTime = mWaveStreamTime;

				getHitDataSetValue( pMsgBuf, COUNTS, &tValue );		
				tArgItem.RingCount = ( HSUInt )tValue;

				if ( tArgItem.RingCount < 2 )
				{
					return;
				}

				getHitDataSetValue( pMsgBuf, RISETIME, &tValue );
				tDBValue = tValue;
				tArgItem.IncreaseNTime = ( HSUInt64 )( tDBValue * 1000.0 );

				getHitDataSetValue( pMsgBuf, DURATION, &tValue );
				tDBValue = tValue;
				tArgItem.NDuration = ( HSUInt64 )( tDBValue * 1000.0 );

				getHitDataSetValue( pMsgBuf, ENERGY, &tValue );
				tDBValue = tValue;
				tArgItem.Energy = tDBValue;

				getHitDataSetValue( pMsgBuf, AMPLITUDE, &tValue );
				tDBValue = tValue;
				tArgItem.Amplitude = tDBValue;

				getHitDataSetValue( pMsgBuf, ASL, &tValue );
				tArgItem.HASL = tValue;

				getHitDataSetValue( pMsgBuf, RMS, &tValue );
				tArgItem.HRMS = tValue;

				float tValues[ 128 ];
				memset( tValues, 0, sizeof tValues );

				//getHitDataSetValue( pMsgBuf, FREQPP1, tValues );
				//getHitDataSetValue( pMsgBuf, FREQPP2, &tValue );
				//getHitDataSetValue( pMsgBuf, FREQPP3, &tValue );
				//getHitDataSetValue( pMsgBuf, FREQPP4, &tValue );

				//HS_INFO( "FREQPP1: %f, %f, %f, %f, %f, %f, %f, %f", tValues[ 0 ], tValues[ 1 ], tValues[ 2 ], tValues[ 3 ], tValues[ 4 ], tValues[ 5 ], tValues[ 6 ], tValues[ 7 ] );

				//HS_INFO( "Message Type: %d for Channel: %f, Time: %f", tMessageType, tWaveChannel, tValue );
			}
			break;

		case 173:	/* waveform related */
			if ( getWaveformValue( pMsgBuf, CHANNEL, &tValue ) == GOOD && mChannels.find( ( HSUInt )tValue ) != mChannels.end())
			{
				tChannels.push_back( ( HSUInt )tValue );	

				HSFloat tWaveChannel = tValue;

				getWaveformValue( pMsgBuf, TIME, &tValue );

				//HS_INFO( "Wave Message for Channel: %f, Time: %f", tWaveChannel, tValue );
			}
			return;

		case 2:     /* Time driven data message */
			{
				map< HSUInt, CHANNEL_INFO >::iterator pIterator = mChannels.begin();
				while ( pIterator != mChannels.end() )
				{
					if ( pIterator->second.On  )
					{
						tChannels.push_back( pIterator->first );							
					}
					pIterator++;
				}				
			}
			return;

		default:
			return;
	}	
	
	WaitForSingleObject( mWriteDataMutex, INFINITE );

	HSChar *pBuf = NULL;
	if ( tChannels.size() > 0 && mListBuffer->BeginWrite( pBuf, tSaveBufLength ) )
	{					
		memcpy( pBuf, pSaveBuf, tSaveBufLength );
		HSUInt tIndex = 0;
		HSUInt tOffset = 0;						
		mListBuffer->FinishWrite( tSaveBufLength, tIndex, tOffset );

		//HS_INFO( "Frame Index: %d, Frame Offset: %d", tIndex, tOffset );

		for ( HSUInt i = 0; i < tChannels.size(); i++ )
		{
			DEVICE_CH_IDENTIFIER tChannelIdentifier = Identifier();
			tChannelIdentifier.InitChannel( tChannels[ i ] );
			tChannelIdentifier.TYPE = DEVICE_CH_IDENTIFIER::CHANNEL_DATA_TYPE_ARG;			
			mIndexManager->AddIndex( tChannelIdentifier, tIndex, tOffset, tSaveBufLength );
		}
	}

	SetEvent( mWriteDataMutex );
	
}

HSBool CPAPCI2Handler::ThreadRuning( HSInt tThreadID )
{
	if ( tThreadID == DATA_STREAM_THREAD_ID )
	{
		HANDLE tEvents[ 3 ] = { mExitEvent, mStartStreamEvent, mStopStreamEvent };

		DWORD tRes = WaitForMultipleObjects( 3, tEvents, FALSE, INFINITE ) - WAIT_OBJECT_0;
        
        if ( tRes == 0 )
        {
			::ResetEvent( tEvents[ 0 ] );
            return HSFalse;
        }
        else if ( tRes == 1 )
        {
            ::ResetEvent( tEvents[ 1 ] );
        }		
		else if ( tRes == 2 )
        {
			map< HSUInt, CHANNEL_INFO >::iterator pIterator = mChannels.begin();
			while ( pIterator != mChannels.end() )
			{
				if ( pIterator->second.On && EffectiveTest( ( HSChar * )pIterator->second.Buffer ) )
				{
					WaitForSingleObject( mWriteDataMutex, INFINITE );

					HSChar *pBuf = NULL;
					HSInt tLength = pIterator->second.SampleLength * EachSampleSize();
					if ( mListBuffer->BeginWrite( pBuf, tLength ) )	
					{
						memcpy( pBuf, pIterator->second.Buffer, tLength );
						HSUInt tIndex = 0;
						HSUInt tOffset = 0;
						mListBuffer->FinishWrite( tLength, tIndex, tOffset );

						DEVICE_CH_IDENTIFIER tChannelIdentifier = Identifier();
						tChannelIdentifier.InitChannel( pIterator->first );
						mIndexManager->AddIndex( tChannelIdentifier, tIndex, tOffset, tLength );
						
						HSUInt tTimePeriod = 10 * 1024 * 1024;
						tChannelIdentifier.TYPE = DEVICE_CH_IDENTIFIER::CHANNEL_DATA_TYPE_TIME;
						mIndexManager->AddIndex( tChannelIdentifier, ( HSUInt )( mWaveStreamTime / tTimePeriod ), ( HSUInt )( mWaveStreamTime % tTimePeriod ), tLength );
					}

					SetEvent( mWriteDataMutex );
				}

				pIterator++;
			}						

            ::ResetEvent( tEvents[ 2 ] );

            rearmStreaming();		

			//HS_INFO( "Receive Stream Data!" );
        }		
	}
	else if ( tThreadID == DATA_MESSAGE_THREAD_ID )
	{
		poll();
		if ( getMessage( mMessage, MAX_MSG_LEN ) == 0 )
		{
			HandleMessage();
		}			
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

HSVoid CPAPCI2Handler::ThreadWillStop( HSInt tThreadID )
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

		CloseHandle( mStartStreamEvent );
		CloseHandle( mStopStreamEvent );
		CloseHandle( mExitEvent );

		InitBuffer();
	}
	else if ( tThreadID == DATA_MESSAGE_THREAD_ID )
	{				
		delete mDataMessageThread;
		mDataMessageThread = NULL;

		if ( mDataStreamThread != NULL )
		{
			SetEvent( mExitEvent );	
		}
	}	
}

HSBool CPAPCI2Handler::EffectiveTest( HSChar *pBuffer )
{
	return HSTrue;
}

HSVoid CPAPCI2Handler::AddChannel( HSInt tChannel, CHANNEL_INFO tChanneInfo )
{
	mChannels[ tChannel ] = tChanneInfo;
}

HSVoid CPAPCI2Handler::CloseDevice()
{
	if ( mSuccessInit )
	{
		closePCI2();
	}
}

HSInt CPAPCI2Handler::GetDevice( vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver )
{
	CPAPCI2Handler *pPCI2Handler = new CPAPCI2Handler( pIndexManager, pFileObserver, 0 );
	pDevices->push_back( pPCI2Handler );

	/*
	if ( !mSuccessInit )
	{
		ifstream tFile( "C:\\Windows\\System32\\drivers\\PCIAE.sys" );
		if ( !tFile.is_open() )
		{		
			return 0;
		}

		tFile.close();

		if ( openPCI2() != GOOD )
		{					
			return 0;
		}

		mSuccessInit = HSTrue;
	}

	HSInt tChannelNum = 0;
	for ( HSInt i = 1; i <= MAX_AE_CHANNELS; i++ )
    {		
        if ( !checkChannelHardwarePresent( i ) )		
		{			
			tChannelNum = i - 1;
            break;
		}  		
    }

	if ( tChannelNum == 0 )
	{
		return 0;
	}
	
	CPAPCI2Handler *pPCI2Handler = new CPAPCI2Handler( pIndexManager, pFileObserver, 0 );
	pDevices->push_back( pPCI2Handler );
	for ( HSInt i = 1; i <= tChannelNum; i++ )
	{				
		pPCI2Handler->AddChannel( i, pPCI2Handler->mDefaultSeting );
	}	

	HS_INFO( "Find Device: PAPCI2" );

	return tChannelNum;
	*/

	return 1;
}

HSBool CPAPCI2Handler::GetDevice( DEVICE_CH_IDENTIFIER tIdentifier, vector< HSInt > &tChannels, vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver )
{
	HSInt tIndex = tIdentifier.CARD_INDEX;
	HSInt tCardIdentifier = tIdentifier.CARD_IDENTIFIER;
	switch ( tCardIdentifier )
	{
	case CARD_PAPCI2:
		{
			CPAPCI2Handler *pPCI2Handler = new CPAPCI2Handler( pIndexManager, pFileObserver, tIndex );
					
			pPCI2Handler->ListBuffer()->SetCardIdentifier( pPCI2Handler->Identifier() );
			pPCI2Handler->ListBuffer()->InitWithFile();

			for ( HSUInt i = 0; i < tChannels.size(); i++ )
			{				
				pPCI2Handler->AddChannel( tChannels[ i ] + 1, pPCI2Handler->mDefaultSeting );
			}
			
			pDevices->push_back( pPCI2Handler );
			
			HS_INFO( "Open Device: PAPCI2" );
		}
		return HSTrue;

	default:
		return HSFalse;
	}
}
