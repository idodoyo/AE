#include "stdafx.h"
#include "USBHandler.h"
#include "HSLogProtocol.h"
#include "USBControlCommand.h"

CUSBHandler::CUSBHandler(  CIndexManager *pIndexManager, CFileObserver *pFileObserver, HSInt tIndex )
	: mIndexManager( pIndexManager ), mDeviceIndex( tIndex )
{
	mListBuffer = new CListBuffer( 10 * 1024 * 1024, 0, 2 );
	mListBuffer->SetFileObserver( pFileObserver );

	mDataIdentifier[ 0 ].CARD_IDENTIFIER = CARD_EASYUSB;
	mDataIdentifier[ 0 ].CARD_INDEX = mDeviceIndex;
	mDataIdentifier[ 0 ].InitChannel( 0 );

	mDataIdentifier[ 1 ].CARD_IDENTIFIER = CARD_EASYUSB;
	mDataIdentifier[ 1 ].CARD_INDEX = mDeviceIndex;
	mDataIdentifier[ 1 ].InitChannel( 1 );

	mSampleRates[ 0 ] = 1 * 1000 * 1000;
	mSampleRates[ 1 ] = mSampleRates[ 0 ];

	mChannelOn[ 0 ] = HSFalse;
	mChannelOn[ 1 ] = HSFalse;

	mDataThread = NULL;
	mFileThread = NULL;

	mUSBDevice = NULL;

	mBypassVoltages[ 0 ] = 0.0501191;
	mBypassVoltages[ 1 ] = 0.0501191;
}

CUSBHandler::~CUSBHandler()
{
	if ( mListBuffer != NULL )
	{
		delete mListBuffer;
	}

	if ( mUSBDevice != NULL )
	{
		delete mUSBDevice;		
	}
}

HSBool CUSBHandler::Start()
{
	if ( !IsOn() )
	{
		return HSFalse;
	}

	if ( mDataThread != NULL && mDataThread->IsPaused() )
	{
		mDataThread->Start();
		return HSTrue;
	}	

	if ( mUSBDevice == NULL )
	{
		mUSBDevice = new CCyUSBDevice();
	}

	mUSBDevice->Open( mDeviceIndex );

	if ( !mUSBDevice->IsOpen() )
	{
		mUSBDevice->Reset();
		mUSBDevice->Open( mDeviceIndex );
	}

	if ( !mUSBDevice->IsOpen() )
	{
		HS_ERROR( "Device %d Open Error!", mDeviceIndex );		
		delete mUSBDevice;
		mUSBDevice = NULL;
		return HSFalse;
	}
	
	if ( mChannelOn[ 0 ] && !CUSBControlCommand::SharedInstance()->OpenChannel( mUSBDevice->ControlEndPt, 0, mSampleRates[ 0 ] ) )
	{
		delete mUSBDevice;
		mUSBDevice = NULL;
		return HSFalse;
	}

	if ( mChannelOn[ 1 ] && !CUSBControlCommand::SharedInstance()->OpenChannel( mUSBDevice->ControlEndPt, 1, mSampleRates[ 1 ] ) )
	{
		if ( mChannelOn[ 0 ] )
		{
			CUSBControlCommand::SharedInstance()->CloseChannel( mUSBDevice->ControlEndPt, 0 );
		}

		delete mUSBDevice;
		mUSBDevice = NULL;
		return HSFalse;
	}	
	/*
	HSDouble tTotalSampleRate = ( mChannelOn[ 0 ] ? mSampleRates[ 0 ] : 0 ) + ( mChannelOn[ 1 ] ? mSampleRates[ 1 ] : 0 );	
	mSampleTimeDelta = 4096 / tTotalSampleRate;
	mCurrentSampleTime = 0;
	*/
	mDataThread = new CThreadControlEx< CUSBHandler >( this, 0, DATA_THREAD_ID );	
	mFileThread = new CThreadControlEx< CUSBHandler >( this, 0, FILE_THREAD_ID );

	mListBuffer->SetCardIdentifier( this->Identifier() );	
	mListBuffer->Reset( 1 );	
	mArgsInfo.clear();

	mDataThread->Start();
	mFileThread->Start();	

	return HSTrue;
}

HSVoid CUSBHandler::Pause()
{
	if ( mDataThread != NULL )
	{
		mDataThread->Pause();	
	}
}

HSVoid CUSBHandler::Stop()
{
	if ( mDataThread == NULL )
	{
		return;
	}		

	mDataThread->Stop();
	Sleep( 50 );

	mFileThread->Stop();

	if ( mChannelOn[ 0 ] )
	{
		CUSBControlCommand::SharedInstance()->CloseChannel( mUSBDevice->ControlEndPt, 0 );
	}

	if ( mChannelOn[ 1 ] )
	{
		CUSBControlCommand::SharedInstance()->CloseChannel( mUSBDevice->ControlEndPt, 1 );
	}

	delete mUSBDevice;
	mUSBDevice = NULL;	
}

HSString CUSBHandler::Name()
{
	HSChar tNameBuf[ 32 ];
	sprintf_s( tNameBuf, "PXUAE1520-%02d", mDeviceIndex + 1 );

	return tNameBuf;
}

HSUInt CUSBHandler::GetChannelData( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSChar *pSrcBuf, HSChar *pDestBuf, HSInt tOffset, HSInt tSamples )
{
	HSInt tIndex = tOffset;
	HSInt tSamplesHandled = min( EASY_USB_EACH_FRAME_SAMPLES - 2 - tIndex, tSamples );
	
	memcpy( pDestBuf, &pSrcBuf[ tIndex * 2 + 4 ], 2 * tSamplesHandled );	

	return tSamplesHandled;	
}

HSBool CUSBHandler::OpenChannel( vector< HSInt > &tChannels, vector< HSInt > *pSampleRates, HSString *pErrorMsg )
{
	CloseAllChannel();

	for ( HSUInt i = 0; i < tChannels.size(); i++ )
	{
		mChannelOn[ tChannels[ i ] ] = HSTrue;
		if ( pSampleRates != NULL )
		{
			mSampleRates[ tChannels[ i ] ] = ( *pSampleRates )[ i ];
		}
	}

	return HSTrue;
}

HSBool CUSBHandler::CloseAllChannel()
{
	mChannelOn[ 0 ] = HSFalse;
	mChannelOn[ 1 ] = HSFalse;
	return HSTrue;
}

HSBool CUSBHandler::ThreadRuning( HSInt tThreadID )
{
	if ( tThreadID == FILE_THREAD_ID )
	{
		HSChar *pBuf = NULL;
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

	HSChar *pDataBuffer = NULL;
	if ( !mListBuffer->BeginWrite( pDataBuffer ) )
	{
		HS_ERROR( "Begin Write Get Buffer Error!\n" );
		return HSTrue;
	}

	CCyBulkEndPoint *pInBulkEndPoint = mUSBDevice->BulkInEndPt;

	HSInt tFrameBufferLength = EASY_USB_EACH_FRAME_SAMPLES * EachSampleSize();
	HSLong tLength = tFrameBufferLength;	
	if ( pInBulkEndPoint == NULL || !pInBulkEndPoint->XferData( ( HSUChar *)pDataBuffer, tLength ) || tLength < tFrameBufferLength )
	{						
		//HS_INFO( "Recv Data Length: %d", tLength );
		return HSTrue;		
	}

	HSUInt tIndex = 0;
	HSUInt tOffset = 0;	
	mListBuffer->FinishWrite( tFrameBufferLength, tIndex, tOffset );
	/*
	HSUInt64 tCurTime = 1000000000;
	tCurTime = ( HSUInt64 )( tCurTime * mCurrentSampleTime );	
	HSUInt tTimePeriod = 10 * 1024 * 1024;
	HSUInt tTimeIndex = ( HSUInt )( tCurTime / tTimePeriod );
	HSUInt tTimeOffset = ( HSUInt )( tCurTime % tTimePeriod );
	*/

	static HSInt sFirstCount = 0;
	static HSInt sSecondCount = 0;

	if ( pDataBuffer[ 2 ] == 0x00 )
	{
		mIndexManager->AddIndex( mDataIdentifier[ 0 ], tIndex, tOffset, tFrameBufferLength );
		//mIndexManager->AddIndex( mDataTimeIdentifier[ 0 ], tTimeIndex, tTimeOffset, 0 );
		sFirstCount++;
	}
	else if ( pDataBuffer[ 2 ] == 0x01 )
	{
		mIndexManager->AddIndex( mDataIdentifier[ 1 ], tIndex, tOffset, tFrameBufferLength );	
		//mIndexManager->AddIndex( mDataTimeIdentifier[ 1 ], tTimeIndex, tTimeOffset, 0 );
		sSecondCount++;
	}
	
	if ( abs( sFirstCount - sSecondCount ) > 2 )
	{
		HS_INFO( "Incorrect: %d", sFirstCount - sSecondCount );
	}
	
	//mCurrentSampleTime += mSampleTimeDelta;

	return HSTrue;
}

HSVoid CUSBHandler::ThreadWillStop( HSInt tThreadID )
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

	if ( mDataThread != NULL )
	{			
		delete mDataThread;		
		mDataThread = NULL;		
	}
}

HSBool CUSBHandler::EffectiveTest( HSChar *pBuffer )
{
	return HSTrue;
}
/*
HSUInt64 CUSBHandler::GetSampleNSecond( HSUInt tSampleReadIndex, HSUInt tSampleReadOffset, DEVICE_CH_IDENTIFIER tChannelIdentifier )
{		
	HSUInt tResIndex = 0;
	HSUInt tResOffset = 0;
	HSUInt tResLength = 0;

	HSInt tChannel = tChannelIdentifier.ChannelIndex();

	mIndexManager->GetIndex( mDataTimeIdentifier[ tChannel ], tSampleReadIndex, tResIndex, tResOffset, tResLength );

	HSUInt tTimePeriod = 10 * 1024 * 1024;
	HSUInt64 tCurTime = tResIndex;
	tCurTime *= tTimePeriod;
	tCurTime += tResOffset;

	tCurTime += ( tSampleReadOffset * 1000000000 / this->SampleRate( tChannelIdentifier ) );

	return tCurTime;
}*/

HSInt CUSBHandler::GetDevice( vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver )
{
	CCyUSBDevice *pUSBDevice = new  CCyUSBDevice( NULL );

	CUSBHandler *pUsbHandler = new CUSBHandler( pIndexManager, pFileObserver, 0 );
	pDevices->push_back( pUsbHandler );

	/*
	HSInt tDevices = pUSBDevice->DeviceCount();
	for ( HSInt i = 0; i < tDevices; i++ )
	{
		CUSBHandler *pUsbHandler = new CUSBHandler( pIndexManager, pFileObserver, i );
		pDevices->push_back( pUsbHandler );
		HS_INFO( "Find Device: PXUAE1005" );
	}
	*/
	delete pUSBDevice;

	/*return tDevices;*/

	return 0;
}

HSBool CUSBHandler::GetDevice( DEVICE_CH_IDENTIFIER tIdentifier, vector< HSInt > &tChannels, vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver )
{
	HSInt tIndex = tIdentifier.CARD_INDEX;
	HSInt tCardIdentifier = tIdentifier.CARD_IDENTIFIER;
	switch ( tCardIdentifier )
	{
	case CARD_EASYUSB:
		{
			CUSBHandler *pUsbHandler = new CUSBHandler( pIndexManager, pFileObserver, tIndex );				

			pUsbHandler->ListBuffer()->SetCardIdentifier( pUsbHandler->Identifier() );
			pUsbHandler->ListBuffer()->InitWithFile();

			pDevices->push_back( pUsbHandler );

			HS_INFO( "Open Device: PXUAE1005" );
		}
		return HSTrue;

	default:
		return HSFalse;
	}	
}