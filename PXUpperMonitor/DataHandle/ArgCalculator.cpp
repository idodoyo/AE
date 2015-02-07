#include "StdAfx.h"
#include "ArgCalculator.h"
#include "HSLogProtocol.h"

/*************************************************
  Function:		SharedInstance	
  Description:	singlaton for the calss
  Input:		None;
  Output:		None
  Return:		the only object of the class
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
CArgCalculator * CArgCalculator::SharedInstance()
{
	static CArgCalculator sArgCalculator;
	return &sArgCalculator;
}

/*************************************************
  Function:		CArgCalculator	
  Description:	Constructor
  Input:		None;
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
CArgCalculator::CArgCalculator()
{
	mMutex = CreateEvent( NULL, FALSE, TRUE, NULL );

	COMMON_MATERIAL_DEF tValues[] = {   { 35 * 1000, 150 * 1000, 300 * 1000, 1000.0 * 1000.0 }, 
										{ 300 * 1000, 600 * 1000, 1000 * 1000, 1000.0 * 1000.0 }, 
										{ 300 * 1000, 600 * 1000, 1000 * 1000, 1000.0 * 1000.0 }, 
										{ 1000 * 1000, 2000 * 1000, 20000 * 1000, 5000.0 * 1000.0 },
										{ 1000 * 1000, 2000 * 1000, 20000 * 1000, 340.0 * 1000.0 } };	
	
	mCommonMaterialWaveArgs[ MATERIAL_KINE_ONE ] = tValues[ 0 ];
	mCommonMaterialWaveArgs[ MATERIAL_KINE_TWO ] = tValues[ 1 ];
	mCommonMaterialWaveArgs[ MATERIAL_KINE_THREE ] = tValues[ 2 ];
	mCommonMaterialWaveArgs[ MATERIAL_KINE_FOUR ] = tValues[ 3 ];
	mCommonMaterialWaveArgs[ MATERIAL_KINE_FIVE ] = tValues[ 4 ];
}

/*************************************************
  Function:		~CArgCalculator	
  Description:	Destructor
  Input:		None;
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
CArgCalculator::~CArgCalculator()
{
	Reset();

	CloseHandle( mMutex );
}

/*************************************************
  Function:		ThreadRuning	
  Description:	Thread function.
  Input:		tThreadID -- identifer of the thread call this function.				
  Output:		None
  Return:		HSTrue to Continue, HSFalse to Stop the thread.
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
HSBool CArgCalculator::ThreadRuning( HSInt tThreadID )
{
	vector< CArgTableManager::ArgItem > tArgList;

	HSDouble tLastSecond = 0;
	HSDouble tLastArgSecond = 0;

	DEVICE_CH_IDENTIFIER tIdentifier = mArgThreadsID[ tThreadID ];

	mArgHandles[ tIdentifier ].DataHandler->GetArgList( tIdentifier, 0, 0, tArgList, tLastSecond, tLastArgSecond );
	if ( mArgHandles[ tIdentifier ].ArgFetcher != NULL )
	{
		mArgHandles[ tIdentifier ].ArgFetcher->FetchArg( tIdentifier, mArgHandles[ tIdentifier ].DataTransfer, tLastSecond );
	}
	else
	{	
		if ( mArgHandles[ tIdentifier ].DataTransfer->Seek( tLastSecond, 1.0, CLinearTransfer::SET_SAMPLE ) )
		{
			mArgHandles[ tIdentifier ].SampleNSecond = mArgHandles[ tIdentifier ].DataTransfer->GetSampleReadInfo( mArgHandles[ tIdentifier ].SampleIndex, mArgHandles[ tIdentifier ].SampleOffset );

			HSInt tLength = mArgHandles[ tIdentifier ].DataTransfer->Read( mArgHandles[ tIdentifier ].Buffer, 1.0 );
			if ( tLength > 0 )
			{
				CalculateArgs( tLength, tLastSecond, tIdentifier );
			}		
		}
	}

	return HSTrue;
}

/*************************************************
  Function:		ThreadWillStop	
  Description:	Thread function used when thread going to stop.
  Input:		tThreadID -- identifer of the thread call this function.				
  Output:		None
  Return:		None.
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
HSVoid CArgCalculator::ThreadWillStop( HSInt tThreadID )
{
}

/*************************************************
  Function:		SetIndexManager	
  Description:	used to calculate args.
  Input:		pIndexManager -- Data index.		
				pDeviceManager -- used to get device info.
  Output:		None
  Return:		None.
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
HSVoid CArgCalculator::SetIndexManager( CIndexManager *pIndexManager, CDeviceManager *pDeviceManager )
{
	mIndexManager = pIndexManager;
	mDeviceManager = pDeviceManager;
}

/*************************************************
  Function:		Start	
  Description:	start handle data.
  Input:		None
  Output:		None
  Return:		None.
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
HSVoid CArgCalculator::Start()
{
	WaitForSingleObject( mMutex, INFINITE );

	map< DEVICE_CH_IDENTIFIER, ArgHandleInfo >::iterator pIterator = mArgHandles.begin();
	while ( pIterator != mArgHandles.end() )
	{
		pIterator->second.ArgThread->Start();

		pIterator++;
	}	

	SetEvent( mMutex );
}

/*************************************************
  Function:		Reset	
  Description:	clear all saved info.
  Input:		None
  Output:		None
  Return:		None.
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
HSVoid CArgCalculator::Reset()
{
	WaitForSingleObject( mMutex, INFINITE );

	map< DEVICE_CH_IDENTIFIER, ArgHandleInfo >::iterator pIterator = mArgHandles.begin();
	while ( pIterator != mArgHandles.end() )
	{
		pIterator->second.ArgThread->Stop();
		delete pIterator->second.ArgThread;
		delete pIterator->second.DataTransfer;

		if ( pIterator->second.ArgFetcher != NULL )
		{
			delete pIterator->second.ArgFetcher;
		}

		if ( pIterator->second.Buffer != NULL )
		{
			delete[] pIterator->second.Buffer;
		}

		pIterator++;
	}

	mArgHandles.clear();
	mArgThreadsID.clear();

	SetEvent( mMutex );
}

/*************************************************
  Function:		AddChannel	
  Description:	Add channels which need to cal arg.
  Input:		tIdentifier -- channel 
  Output:		None
  Return:		None.
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
HSVoid CArgCalculator::AddChannel( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mMutex, INFINITE );

	map< DEVICE_CH_IDENTIFIER, ArgHandleInfo >::iterator pIterator = mArgHandles.find( tIdentifier );
	DEVICE_CH_IDENTIFIER tCardIdentifier( tIdentifier.CARD_IDENTIFIER, tIdentifier.CARD_INDEX );
	ArgHandleInfo tArgHandle;	
	tArgHandle.DataHandler = mDeviceManager->DataHandlerWithIdentifier( tCardIdentifier );
	if ( pIterator == mArgHandles.end() && tArgHandle.DataHandler != NULL && tArgHandle.DataHandler->SampleRate( tIdentifier ) > 0 )
	{						
		tArgHandle.SampleOffset = 0;
		tArgHandle.SampleIndex = 0;
		tArgHandle.HLTEndNTime = 0;
		tArgHandle.ArgFetcher = tArgHandle.DataHandler->ArgFetcher();
		if ( tArgHandle.ArgFetcher != NULL )
		{
			DEVICE_CH_IDENTIFIER tArgIdentifier = tIdentifier;
			tArgIdentifier.TYPE = DEVICE_CH_IDENTIFIER::CHANNEL_DATA_TYPE_ARG;
			tArgHandle.DataTransfer = new CLinearTransfer( mIndexManager, tArgHandle.DataHandler, tArgIdentifier );
			tArgHandle.Buffer = NULL;
		}
		else
		{
			tArgHandle.DataTransfer = new CLinearTransfer( mIndexManager, tArgHandle.DataHandler, tIdentifier );
			tArgHandle.Buffer = new HSChar[ tArgHandle.DataHandler->SampleRate( tIdentifier ) * tArgHandle.DataHandler->EachSampleSize() ];
		}

		HSInt tID = mArgThreadsID.size();
		mArgThreadsID[ tID ] = tIdentifier;
		tArgHandle.ArgThread = new CThreadControlEx< CArgCalculator >( this, 700, tID );

		mArgHandles[ tIdentifier ] = tArgHandle;
	}	

	SetEvent( mMutex );
}

/*************************************************
  Function:		CalculateArgs	
  Description:	cal args for each channel.
  Input:		tLength -- data readed. 
				tLastSecond -- last second of reading data.
				tIdentifier -- channel 
  Output:		None
  Return:		result.
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/
HSBool CArgCalculator::CalculateArgs( HSInt tLength, HSDouble tLastSecond, DEVICE_CH_IDENTIFIER tIdentifier )
{	
	CArgTableManager::ArgItem tArgItem;
	memset( &tArgItem, 0, sizeof tArgItem );

	vector< WAVE_ITEM > tWaves;
	GetWaveItem( tLength, tWaves, tIdentifier );			

	HSUInt tHandledLength = 0;
	HSBool tGotNewArgs = HSFalse;

	HSDouble tTimes = 100;

	//HS_INFO( "WaveItem Size: %d", tWaves.size() );
	
	for ( HSUInt i = 0; i < tWaves.size(); i++ )
	{		
		if ( tWaves[ i ].NBeginTime < mArgHandles[ tIdentifier ].HLTEndNTime )
		{
			tHandledLength = max( tWaves[ i ].EndIndex + 1, tHandledLength );
			continue;
		}

		if ( tArgItem.NBeginTime == 0 )
		{
			tArgItem.NBeginTime = tWaves[ i ].NBeginTime;
			tArgItem.IncreaseNTime = tWaves[ i ].NTime - tArgItem.NBeginTime;	
			tArgItem.Amplitude = tWaves[ i ].Amplitude;
		}

		if ( tWaves[ i ].NTime - tArgItem.NBeginTime < mCommonMaterialWaveArgs[ MATERIAL_KINE_FOUR ].PDT )
		{
			if ( tWaves[ i ].Amplitude > tArgItem.Amplitude )
			{
				tArgItem.Amplitude = tWaves[ i ].Amplitude;
				tArgItem.IncreaseNTime = tWaves[ i ].NTime - tArgItem.NBeginTime;
			}			
		}
		else if ( tWaves[ i ].NEndTime - tArgItem.NBeginTime/* - mCommonMaterialWaveArgs[ MATERIAL_KINE_FOUR ].PDT*/ > mCommonMaterialWaveArgs[ MATERIAL_KINE_FOUR ].HDT )
		{					
			if ( tArgItem.NBeginTime != 0 && tArgItem.RingCount > 1 )
			{
				tArgItem.Amplitude = 20 * log10( tArgItem.Amplitude * 1000000.0 / tTimes );
				tArgItem.NDuration = tWaves[ i - 1 ].NEndTime - tArgItem.NBeginTime;
				tArgItem.HRMS = sqrt( tArgItem.HRMS );
				tArgItem.HASL = 20 * log10( tArgItem.HRMS * 1000000.0 / tTimes );
				mArgHandles[ tIdentifier ].DataHandler->AddArg( tIdentifier, &tArgItem, 0 );
				mArgHandles[ tIdentifier ].HLTEndNTime = tArgItem.NBeginTime + /*mCommonMaterialWaveArgs[ MATERIAL_KINE_FOUR ].PDT +*/ mCommonMaterialWaveArgs[ MATERIAL_KINE_FOUR ].HDT + mCommonMaterialWaveArgs[ MATERIAL_KINE_FOUR ].HLT;
				tHandledLength = tWaves[ i - 1 ].EndIndex + HSUInt( mCommonMaterialWaveArgs[ MATERIAL_KINE_FOUR ].HLT / 1000000000.0 * mArgHandles[ tIdentifier ].DataHandler->SampleRate( tIdentifier ) ) + 1;

				tGotNewArgs = HSTrue;
			}

			memset( &tArgItem, 0, sizeof tArgItem );

			continue;
		}

		tArgItem.RingCount++;	
		tArgItem.Energy += tWaves[ i ].Energy;
		tArgItem.HRMS += tWaves[ i ].RMS;
	}
	
	HSDouble tFullWaveLength = ( /*mCommonMaterialWaveArgs[ MATERIAL_KINE_FOUR ].PDT + */mCommonMaterialWaveArgs[ MATERIAL_KINE_FOUR ].HDT ) / 1000000000.0;
	if ( tWaves.size() < 1 || ( tWaves.size() > 0 && ( ( HSDouble )( tLength - tWaves[ tWaves.size() - 1 ].EndIndex ) ) / mArgHandles[ tIdentifier ].DataHandler->SampleRate( tIdentifier ) > tFullWaveLength ) )
	{		
		tHandledLength = tLength;
	}
	
	mArgHandles[ tIdentifier ].DataHandler->AddArg( tIdentifier, NULL, ( HSDouble )tHandledLength / mArgHandles[ tIdentifier ].DataHandler->SampleRate( tIdentifier ) );

	return tGotNewArgs;
}

/*************************************************
  Function:		GetWaveItem	
  Description:	get all waveitem( up then dowm ).
  Input:		tLength -- data readed. 				
				tIdentifier -- channel 
  Output:		tWaves -- keep all waveitems
  Return:		result.
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/
HSInt CArgCalculator::GetWaveItem( HSInt tLength, vector< WAVE_ITEM > &tWaves, DEVICE_CH_IDENTIFIER tIdentifier )
{
	WAVE_ITEM tWaveItem;
	memset( &tWaveItem, 0, sizeof tWaveItem );	

	HSDouble tBypassVoltage = mArgHandles[ tIdentifier ].DataHandler->BypassVoltage( tIdentifier.ChannelIndex() );
	HSInt tEachSampleSize = mArgHandles[ tIdentifier ].DataHandler->EachSampleSize();

	HSDouble tPrevValue = -10169;	
	HSInt tTopPosition = 0;	
	HSInt tEndPosition = 0;
	tLength *= tEachSampleSize;	
	for ( HSInt i = 0; i < tLength; i += tEachSampleSize )
	{
		HSDouble tValue = mArgHandles[ tIdentifier ].DataHandler->VoltageWithPointer( &( mArgHandles[ tIdentifier ].Buffer[ i ] ) );

		if ( tValue > tPrevValue )
		{
			if ( tEndPosition != 0 )
			{
				tWaveItem.NEndTime = mArgHandles[ tIdentifier ].DataHandler->GetSampleNSecond( mArgHandles[ tIdentifier ].SampleIndex, tEndPosition + mArgHandles[ tIdentifier ].SampleOffset, tIdentifier );	
				tWaveItem.NTime = mArgHandles[ tIdentifier ].DataHandler->GetSampleNSecond( mArgHandles[ tIdentifier ].SampleIndex, tTopPosition + mArgHandles[ tIdentifier ].SampleOffset, tIdentifier );
				tWaveItem.EndIndex = tEndPosition;
				HSUInt tRegardDuration = 100;//10000;
				if ( tWaves.size() > 0 && ( tWaveItem.NBeginTime - tWaves[ tWaves.size() - 1 ].NTime < tRegardDuration || tWaveItem.NTime - tWaves[ tWaves.size() - 1 ].NEndTime < tRegardDuration ) )
				{
					tWaves[ tWaves.size() - 1 ].Energy += tWaveItem.Energy;
					tWaves[ tWaves.size() - 1 ].NEndTime = tWaveItem.NEndTime;
					tWaves[ tWaves.size() - 1 ].EndIndex = tWaveItem.EndIndex;
					if ( tWaveItem.Amplitude > tWaves[ tWaves.size() - 1 ].Amplitude )
					{
						tWaves[ tWaves.size() - 1 ].Amplitude = tWaveItem.Amplitude;
						tWaves[ tWaves.size() - 1 ].NTime = tWaveItem.NTime;
					}				
				}
				else
				{					

					tWaves.push_back( tWaveItem );
				}

				memset( &tWaveItem, 0, sizeof tWaveItem );		
				tEndPosition = 0;
			}
			
			if ( tValue > tBypassVoltage && tWaveItem.NBeginTime == 0 )
			{
				tWaveItem.NBeginTime = mArgHandles[ tIdentifier ].DataHandler->GetSampleNSecond( mArgHandles[ tIdentifier ].SampleIndex, i / tEachSampleSize + mArgHandles[ tIdentifier ].SampleOffset, tIdentifier );
			}
			
			tWaveItem.Amplitude = tValue;
			tTopPosition = i / tEachSampleSize;
		}
		else if ( tWaveItem.NBeginTime != 0 )
		{
			if ( tValue > tBypassVoltage )
			{
				tEndPosition = i / tEachSampleSize;
			}
			else if ( tEndPosition == 0 )
			{
				tEndPosition = tTopPosition;
			}
		}

		if ( tValue > tBypassVoltage && tWaveItem.NBeginTime != 0 )
		{
			tWaveItem.Energy += ( tValue - tBypassVoltage );
			tWaveItem.RMS += ( tValue - tBypassVoltage ) * ( tValue - tBypassVoltage );
		}

		tPrevValue = tValue;
	}

	return tWaves.size();
}

/*
HSInt CArgCalculator::GetWaveItem( HSInt tLength, vector< WAVE_ITEM > &tWaves )
{
	WAVE_ITEM tWaveItem;
	memset( &tWaveItem, 0, sizeof tWaveItem );	

	HSDouble tBypassVoltage = mDataHandler->BypassVoltage( mIdentifier.ChannelIndex() );
	HSInt tEachSampleSize = mDataHandler->EachSampleSize();	

	HSInt tTopPosition = 0;
	tLength *= tEachSampleSize;	
	for ( HSInt i = 0; i < tLength; i += tEachSampleSize )
	{
		HSDouble tValue = mDataHandler->VoltageWithPointer( &mBuffer[ i ] );

		if ( tValue > tBypassVoltage )
		{
			if ( tValue > tWaveItem.Amplitude )
			{
				tWaveItem.Amplitude = tValue;
				tTopPosition = max( i / tEachSampleSize, 0 );
			}

			if ( tWaveItem.NBeginTime == 0 )
			{
				tWaveItem.NBeginTime = mDataHandler->GetSampleNSecond( mFrameIndex, mFrameOffset, i / tEachSampleSize + mSampleOffset, mIdentifier );
			}
					
			tWaveItem.Energy += ( tValue - tBypassVoltage );	
			tWaveItem.RMS += ( tValue - tBypassVoltage ) * ( tValue - tBypassVoltage );
		}
		else if ( tWaveItem.Energy > 0 )
		{			
			tWaveItem.EndIndex = i / tEachSampleSize - 1;
			tWaveItem.NTime = mDataHandler->GetSampleNSecond( mFrameIndex, mFrameOffset, tTopPosition + mSampleOffset, mIdentifier );
			if ( tWaves.size() > 0 && tWaveItem.NTime - tWaves[ tWaves.size() - 1 ].NTime < 16000 )
			{
				tWaves[ tWaves.size() - 1 ].Energy += tWaveItem.Energy;
				tWaves[ tWaves.size() - 1 ].NEndTime = mDataHandler->GetSampleNSecond( mFrameIndex, mFrameOffset, i / tEachSampleSize - 1 + mSampleOffset, mIdentifier );
				tWaves[ tWaves.size() - 1 ].EndIndex = tWaveItem.EndIndex;
				if ( tWaveItem.Amplitude > tWaves[ tWaves.size() - 1 ].Amplitude )
				{
					tWaves[ tWaves.size() - 1 ].Amplitude = tWaveItem.Amplitude;
					tWaves[ tWaves.size() - 1 ].NTime = tWaveItem.NTime;
				}				
			}
			else
			{				
				tWaveItem.NEndTime = mDataHandler->GetSampleNSecond( mFrameIndex, mFrameOffset, i / tEachSampleSize - 1 + mSampleOffset, mIdentifier );				
				tWaves.push_back( tWaveItem );
			}

			memset( &tWaveItem, 0, sizeof tWaveItem );			
		}
	}

	return tWaves.size();
}
*/