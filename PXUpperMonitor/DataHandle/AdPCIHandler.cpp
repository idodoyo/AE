#include "StdAfx.h"
#include "AdPCIHandler.h"
#include "AdPCI1714Handler.h"
#include "HSLogProtocol.h"


CAdPCIHandler::CAdPCIHandler( CIndexManager *pIndexManager, CFileObserver *pFileObserver ) 
	: mIndexManager( pIndexManager )
{		
	mSampleRate = 1000000;
	mSampleCount = 2 * 1024 * 1024;
	mTrigerVoltage = 0;
	mStartChannel = 0;
	mNumChannel = 0;

	mUserBuffer = NULL;
	mDataThread = NULL;
	mFileThread = NULL;

	mDeviceNum = -1;

	mListBuffer = new CListBuffer( 10 * 1024 * 1024, 0, 2 );
	mListBuffer->SetFileObserver( pFileObserver );			
}


CAdPCIHandler::~CAdPCIHandler()
{
	if ( mUserBuffer != NULL )
	{
		delete[] mUserBuffer;		
		delete mDataThread;	
		delete mFileThread;
	}	

	if ( mListBuffer != NULL )
	{
		delete mListBuffer;
	}
}

HSVoid CAdPCIHandler::SetParam( HSLong tDeviceNum, HSInt tSampleRate, HSFloat tTrigerVoltage )
{
	mSampleRate = ( tSampleRate >= 0 ? tSampleRate : mSampleRate );	
	mTrigerVoltage = ( tTrigerVoltage >= 0 ? tTrigerVoltage : mTrigerVoltage );	
	mDeviceNum = ( tDeviceNum >= 0 ? tDeviceNum : mDeviceNum );	
	mIdentifier.CARD_INDEX = ( HSUChar )mDeviceNum;
}

HSInt CAdPCIHandler::GetDevice( vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver )
{
//	DEVLIST tDevList[ 16 ];
	HSShort tActualDevNum = 0;

	CAdPCI1714Handler *pPCI1714 = new CAdPCI1714Handler( pIndexManager, pFileObserver );		
	pDevices->push_back( pPCI1714 );

	/*
	if ( SUCCESS == DRV_DeviceGetList( tDevList, 16, &tActualDevNum ) )
	{		
		for ( HSInt i = 0; i < tActualDevNum && pDevices != NULL; i++ )
		{			
			string tDeviceName = tDevList[ i ].szDeviceName;
			if ( tDeviceName.find( "1714" ) != string::npos )
			{
				CAdPCI1714Handler *pPCI1714 = new CAdPCI1714Handler( pIndexManager, pFileObserver );
				pPCI1714->SetParam( tDevList[ i ].dwDeviceNum );			

				pDevices->push_back( pPCI1714 );

				HS_INFO( "Find Device: PXDAQ12204" );
			}
		}
	}		
	*/
	return tActualDevNum;
}

HSBool CAdPCIHandler::GetDevice( DEVICE_CH_IDENTIFIER tIdentifier, vector< HSInt > &tChannels, vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver )
{	
	HSInt tIndex = tIdentifier.CARD_INDEX;
	HSInt tCardIdentifier = tIdentifier.CARD_IDENTIFIER;

	switch ( tCardIdentifier )
	{
	case CARD_PCI1714:
		{
			CAdPCI1714Handler *pPCI1714 = new CAdPCI1714Handler( pIndexManager, pFileObserver );
			pPCI1714->SetParam( tIndex );
			
			pPCI1714->ListBuffer()->SetCardIdentifier( pPCI1714->Identifier() );	
			pPCI1714->ListBuffer()->InitWithFile();
			
			pDevices->push_back( pPCI1714 );

			HS_INFO( "Open Device: PXDAQ12204" );
		}
		return HSTrue;

	default:
		return HSFalse;
	}
}

HSBool CAdPCIHandler::Start()
{
	if ( mDataThread != NULL && mDataThread->IsPaused() )
	{
		mDataThread->Start();
		return HSTrue;
	}

	if ( mDeviceNum < 0 || mNumChannel == 0 )
	{
		return HSFalse;
	}	
		
	HSLong tRes = 0;
	if ( ( tRes =  DRV_DeviceOpen( mDeviceNum, ( LONG far * )&mDeviceHandle ) ) != SUCCESS )
	{
		HSUChar tErrorMsg[ 64 ];
		DRV_GetErrorMessage( tRes,( LPSTR )tErrorMsg);
		HS_ERROR( "Device Open Error: %s", ( LPSTR )tErrorMsg );		
		return HSFalse;
	}

	DEVFEATURES tDevFeatures;
	PT_DeviceGetFeatures tDevGetFeatures;
	tDevGetFeatures.buffer = ( LPDEVFEATURES )&tDevFeatures;
	tDevGetFeatures.size = sizeof( DEVFEATURES );

	if ( SUCCESS == DRV_DeviceGetFeatures( mDeviceHandle, &tDevGetFeatures ) )
	{
		 if ( tDevFeatures.usNumGain != 0 )
		 {
			 CString tStrMsg = "";
			 for ( int i = 0; i < tDevFeatures.usNumGain; i++ )
			 {
				 tStrMsg += tDevFeatures.glGainList[ i ].szGainStr;
				 tStrMsg += "\n";
			 }
			 /* AfxMessageBox( tStrMsg ); */
		 }
		 else
		 {
			 /* AfxMessageBox( "No Gain!" ); */
		 }
	}

	DEVCONFIG_AI tDevAIConfig;
	PT_AIGetConfig tAIGetConfig; 
	tAIGetConfig.buffer = ( LPDEVCONFIG_AI )&tDevAIConfig;
	if ( SUCCESS == DRV_AIGetConfig( mDeviceHandle, &tAIGetConfig ) )
	{
		/*
		CString tStrMsg;
		tStrMsg.Format( "BoardID: %d, GainConfigMode: %d", tDevAIConfig.dwBoardID, tDevAIConfig.usGainCtrMode );
		AfxMessageBox( tStrMsg );
		*/
	}

	PT_EnableEvent tEnableEvent;
	tEnableEvent.EventType = ADS_EVT_BUFCHANGE | ADS_EVT_OVERRUN | ADS_EVT_TERMINATED;
	tEnableEvent.Enabled = 1;
    /* tEnableEvent.Count = ( HSUShort )( 1 * 2048 ); */
	if ( SUCCESS != DRV_EnableEvent( mDeviceHandle, &tEnableEvent ) )
	{
		DRV_DeviceClose( &mDeviceHandle );

		HSUChar tErrorMsg[ 128 ];
		DRV_GetErrorMessage( tRes,( LPSTR )tErrorMsg);
		HS_ERROR( "Enable Event Error: %s", ( LPSTR )tErrorMsg );			
		return HSFalse;
	}

	const HSInt MAX_CHANNELS = 64;
	HSUShort tGainCde[ MAX_CHANNELS ];
	for ( HSInt i = 0; i < MAX_CHANNELS; i++ )        
    {         
		int tGainIndex = 0;
		tGainCde[ i ] = tDevFeatures.glGainList[ tGainIndex ].usGainCde;     
    }
	
	HSInt tMaxChanNum = 4;//GetMaxLogChanNum( &tDevAIConfig,&tDevFeatures);
	HSInt tLogChan = 0;//PhyChanToLogChan( &tDevAIConfig, 0 );
	HSInt tChannelNum = min( 4, tMaxChanNum );
    for( HSInt i = 0; i < tChannelNum; i++, tLogChan++ )
    {
		mRecorderGainCode[ i ] = tGainCde[ tLogChan % tMaxChanNum ];
    }	

	if ( mUserBuffer == NULL )
	{	
		mUserBuffer = new HSChar[ mSampleCount * sizeof( HSShort ) ];		
	}

	PT_FAIDmaExStart tFAIDmaExStart;
	tFAIDmaExStart.TrigSrc = 0;
    tFAIDmaExStart.TrigMode = 0;
    tFAIDmaExStart.ClockSrc = 0;
    tFAIDmaExStart.TrigEdge = 0;
    tFAIDmaExStart.SRCType = 0;
    tFAIDmaExStart.CyclicMode = 1;
    tFAIDmaExStart.TrigVol = mTrigerVoltage;
    tFAIDmaExStart.StartChan = mStartChannel;
    tFAIDmaExStart.NumChans = mNumChannel;
    tFAIDmaExStart.ulDelayCnt = 2;
    tFAIDmaExStart.SampleRate = mSampleRate * mNumChannel;
    tFAIDmaExStart.GainList = &mRecorderGainCode[ 0 ];
    tFAIDmaExStart.count = mSampleCount;
    tFAIDmaExStart.buffer0 = ( USHORT * )mUserBuffer;
	
	if ( ( tRes = DRV_FAIDmaExStart( mDeviceHandle, &tFAIDmaExStart ) ) != SUCCESS )
	{
		DRV_DeviceClose( &mDeviceHandle );

		HSUChar tErrorMsg[ 64 ];
		DRV_GetErrorMessage( tRes,( LPSTR )tErrorMsg);		
		HS_ERROR( "Device DRV_FAIDmaExStart Error: %s", ( LPSTR )tErrorMsg );		

		delete[] mUserBuffer;
		mUserBuffer = NULL;

		return HSFalse;
	}		

	mDataThread = new CThreadControlEx< CAdPCIHandler >( this, 0, DATA_THREAD_ID );	
	mFileThread = new CThreadControlEx< CAdPCIHandler >( this, 0, FILE_THREAD_ID );	

	mListBuffer->SetCardIdentifier( this->Identifier() );
	mListBuffer->Reset( 1 );

	mArgsInfo.clear();

	mDataThread->Start();
	mFileThread->Start();

	return HSTrue;
}

HSVoid CAdPCIHandler::Pause()
{
	if ( mDataThread != NULL )
	{
		mDataThread->Pause();	
	}
}

HSVoid CAdPCIHandler::Stop()
{
	if ( mDataThread == NULL )
	{
		return;
	}

	if ( SUCCESS != DRV_FAITerminate( mDeviceHandle ) )
	{
		HS_ERROR( "FAI Terminate Error!\n " );
	}	

	mDataThread->Stop();	
	Sleep( 50 );

	mFileThread->Stop();
}

CListBuffer * CAdPCIHandler::ListBuffer()
{
	return mListBuffer;
}

HSUInt CAdPCIHandler::SamplesInFrame( DEVICE_CH_IDENTIFIER tChannelIdentifier )
{
	HSInt tChannels = mIdentifier.NumOfChannels();	
	if ( tChannels == 0 )
	{
		return 0;
	}	

	tChannelIdentifier.ActionAnd( mIdentifier );

	return mSampleCount / 2 / tChannels * tChannelIdentifier.NumOfChannels();
}

HSUInt CAdPCIHandler::GetChannelData( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSChar *pSrcBuf, HSChar *pDestBuf, HSInt tOffset, HSInt tSamples )
{
	HSInt tStart = 0;
	HSInt tLength = 0;

	HSInt tChannels = mIdentifier.NumOfChannels();
	mIdentifier.ChannelInfoWithIdentifier( tChannelIdentifier, tStart, tLength );
	
	HSInt tIndex = tOffset * tChannels;
	HSInt tSamplesHandled = 0;
	HSInt tHalfSampleCount = mSampleCount / 2;

	while ( tIndex < tHalfSampleCount && tSamplesHandled < tSamples )
	{
		memcpy( &pDestBuf[ tSamplesHandled * 2 ], &pSrcBuf[ ( tIndex + tStart ) * 2 ], 2 * tLength );

		tIndex += tChannels;
		tSamplesHandled += tLength;
	}

	return tSamplesHandled;
}


HSBool CAdPCIHandler::EffectiveTest( HSChar *pBuffer )
{
	return HSTrue;
	/*
	HSInt tEachSampleSize = this->EachSampleSize();
	HSInt tLength = mSampleCount / 2 * tEachSampleSize;	
	HSDouble tValue = 0;
	for ( HSInt i = 0; i < tLength; i += tEachSampleSize )
	{		
		tValue = this->VoltageWithPointer( &pBuffer[ i ] );
		if ( tValue > 0.05 || tValue < -0.05  )
		{
			return HSTrue;
		}
	}

	return HSFalse;*/
}

HSBool CAdPCIHandler::ThreadRuning( HSInt tThreadID )
{
	if ( tThreadID == FILE_THREAD_ID )
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

		return HSTrue;
	}

	HSUShort tEventType = 0;
	PT_CheckEvent tCheckEvent;
	tCheckEvent.EventType = &tEventType;
	tCheckEvent.Milliseconds = INFINITE;

	if ( SUCCESS != DRV_CheckEvent( mDeviceHandle, &tCheckEvent ) )
	{
		HS_ERROR( " Thread Check Event Error!\n" );
		return HSTrue;
	}

	if ( tEventType & ADS_EVT_BUFCHANGE )
	{        
		BufChangeEvent();
	}

	if ( tEventType & ADS_EVT_OVERRUN )
	{
		OverrunEvent();
	}

	if ( tEventType & ADS_EVT_TERMINATED )
	{
		TerminateEvent();
	}

	return HSTrue;
}

HSVoid CAdPCIHandler::ThreadWillStop( HSInt tThreadID )
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

		return;
	}

	DRV_DeviceClose( &mDeviceHandle );		

	if ( mUserBuffer != NULL )
	{	
		delete[] mUserBuffer;
		delete mDataThread;			
	
		mUserBuffer = NULL;	
		mDataThread = NULL;		
	}
}

//HSInt CAdPCIHandler::GetMaxLogChanNum( DEVCONFIG_AI *pDevCfg, DEVFEATURES *pDevFeature )
//{
//	HSInt i;
//	int result = 0;
//
//	if (pDevCfg->ulChanConfig == 1)
//	{
//		return pDevFeature->usMaxAIDiffChl;
//	}
//
//	for(i = 0; i< pDevFeature->usMaxAISiglChl ; i++)
//	{
//		if(i<32)
//			if(pDevCfg->ulChanConfig & (1<<i))
//				i++;
//		else
//			if(pDevCfg->ulChanConfigEx[0] & (1<<(i-32)))
//				i++;
//		result++;
//	}
//
//	return result;
//}
//
//HSInt CAdPCIHandler::PhyChanToLogChan( DEVCONFIG_AI *pDevCfg, HSInt phychan )
//{
//	HSInt i;
//	HSInt result = 0;
//	
//	if (pDevCfg->ulChanConfig == 1)
//	{//For ISA Cards,when differetial, Phychan is LogChan
//		result = phychan;
//		return result;
//	}
//
//	for(i = 0; i<=min(phychan,31);i++)
//	{
//		if(pDevCfg->ulChanConfig & (1<<i))
//			i++;
//		result++;
//	}
//
//	if(phychan >= 32)
//	{
//		for(i = 32; i<=phychan;i++)
//		{
//			if(pDevCfg->ulChanConfigEx[0] & (1<<(i-32)))
//				i++;
//			result++;
//		}
//	}
//	return result - 1;
//}

HSVoid CAdPCIHandler::BufChangeEvent()
{
	HSUShort tHalfReady = 0;
	HSUShort tActiveBuf = 0;
	HSUShort tStopped = 0;
	HSULong tRetrieved = 0;
	HSUShort tOverRun = 0;

	PT_FAICheck tFAICheck;
	tFAICheck.ActiveBuf = &tActiveBuf;
    tFAICheck.stopped   = &tStopped;
    tFAICheck.retrieved = &tRetrieved;
    tFAICheck.overrun   = &tOverRun;
    tFAICheck.HalfReady = &tHalfReady;

    if ( SUCCESS != DRV_FAICheck( mDeviceHandle, &tFAICheck ) )
    {       
		HS_ERROR( "Buf Change FAICheck ERROR!\n" );
        return;
    }

	HSUInt tHalfSampleCount = mSampleCount / 2;

	HSChar *pBuf = NULL;
	if ( !mListBuffer->BeginWrite( pBuf ) )
	{
		HS_ERROR( "Begin Write Get Buffer Error!\n" );
		return;
	}
	
	PT_FAITransfer tFAITransfer;
	tFAITransfer.ActiveBuf = 0;
	tFAITransfer.DataType = 0;	
	tFAITransfer.start = 0;
	tFAITransfer.count = tHalfSampleCount;
	tFAITransfer.overrun = &tOverRun;
	tFAITransfer.DataBuffer = pBuf;	

    if ( 2 == tHalfReady )
    {	
		tFAITransfer.start = tHalfSampleCount;
	}

    if ( SUCCESS != DRV_FAITransfer( mDeviceHandle, &tFAITransfer ) )
    {       
		HS_ERROR( "Buf Change FAITransfer ERROR!\n" );
        return;
    }	

	if ( !this->EffectiveTest( pBuf ) )
	{
		return;
	}
	
	HSUInt tIndex = 0;
	HSUInt tOffset = 0;
	mListBuffer->FinishWrite( tHalfSampleCount * sizeof( SHORT ), tIndex, tOffset );
	mIndexManager->AddIndex( mIdentifier, tIndex, tOffset, tHalfSampleCount * sizeof( SHORT ) );
}
	
HSVoid CAdPCIHandler::OverrunEvent()
{
	if ( SUCCESS != DRV_ClearOverrun( mDeviceHandle ) )
    {        
        HS_ERROR( "Clear Overun Error!\n" );
    }

	HS_INFO( "Clear OverRun!\n" );
}

HSVoid CAdPCIHandler::TerminateEvent()
{
	HSUShort tOverRun = 0;

	HSChar tBuf[ 1024 ];

	PT_FAITransfer tFAITransfer;
	tFAITransfer.ActiveBuf = 0;
    tFAITransfer.DataType = 0;
    tFAITransfer.start = 0;
    tFAITransfer.count = 256;
    tFAITransfer.overrun = &tOverRun;
	tFAITransfer.DataBuffer = tBuf;

    if ( SUCCESS != DRV_FAITransfer( mDeviceHandle, &tFAITransfer ) )
	{
		HS_ERROR( "Terminate Event FAITransfer ERROR!\n" );
	}			
}



