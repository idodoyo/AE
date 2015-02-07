#include "StdAfx.h"
#include "LinearTransfer.h"
#include <fstream>
#include "HSLogProtocol.h"

using std::ofstream;

/*************************************************
  Function:		CLinearTransfer	
  Description:	构造函数
  Input:		pListBuffer -- 缓冲区		
				pFrameChecker -- 帧检查
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-03-01
  Last Time Modify:  None
**************************************************/	
CLinearTransfer::CLinearTransfer( CIndexManager *pIndexManager, IDataHandler *pDataHandler, DEVICE_CH_IDENTIFIER tIdentifier )
	: mIndexManager( pIndexManager ), mDataHandler( pDataHandler ), mIdentifier( tIdentifier )
{
	mListBuffer = pDataHandler->ListBuffer();

	mCurItem = NULL;

	mCHTimeIdentifier = mDataHandler->DataIdentifier( mIdentifier );
	mCHTimeIdentifier.TYPE = DEVICE_CH_IDENTIFIER::CHANNEL_DATA_TYPE_TIME;

	Reset();
}

/*************************************************
  Function:		~CLinearTransfer	
  Description:	析构函数
  Input:		None
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-03-01
  Last Time Modify:  None
**************************************************/	
CLinearTransfer::~CLinearTransfer()
{
}

HSInt CLinearTransfer::Read( HSUInt tSampleReadIndex, HSChar * &pBuf )
{	
	if ( mIndexManager->GetIndex( mDataHandler->DataIdentifier( mIdentifier ), tSampleReadIndex, mFrameIndex, mFrameOffset, mFrameLength ) 
		&& mListBuffer->ReadData( mFrameIndex, mFrameOffset, mCurItem, pBuf ) )
	{			
		return mFrameLength;
	}

	return 0;
}

/*************************************************
  Function:		Read	
  Description:	读取数据
  Input:		pBuf -- 数据存储区				
  Output:		pBuf -- 结果数据
				tLength -- 结果长度
  Return:		1 ok, 0 failed
  Author:		Guo Dengjia
  Date :		2014-03-01
  Last Time Modify:  None
**************************************************/	
HSInt CLinearTransfer::Read( HSChar *pBuf, HSDouble tSecond )
{	
	//HSUInt tFrameIndex = 0;
	//HSUInt tFrameOffset = 0;
	//HSUInt tFrameLength = 0;
	//if ( !mIndexManager->GetIndex( mCHTimeIdentifier, 0, tFrameIndex, tFrameOffset, tFrameLength ) )
	{
		return LinearRead( pBuf, tSecond );
	}
	//else
	//{
	//	return SegmentRead( pBuf, tSecond );
	//}
}

/*************************************************
  Function:		Seek	
  Description:	定位侦
  Input:		tFrameIndex -- 帧	
				tType -- 查找类型
  Output:		None
  Return:		1 ok, 0 failed
  Author:		Guo Dengjia
  Date :		2014-03-01
  Last Time Modify:  None
**************************************************/	
HSBool CLinearTransfer::Seek( HSDouble tSecond, HSDouble tSecondLengthTip, SAMPLE_SEEK_TYPE tType )
{		
	if ( mSampleReadIndex < 0 )
	{ 
		return this->Init();
	}	

	//HSUInt tFrameIndex = 0;
	//HSUInt tFrameOffset = 0;
	//HSUInt tFrameLength = 0;
	//if ( !mIndexManager->GetIndex( mCHTimeIdentifier, 0, tFrameIndex, tFrameOffset, tFrameLength ) )
	{
		return LinearSeek( tSecond, tSecondLengthTip, tType );		
	}
	//else
	//{
	//	return SegmentSeek( tSecond, tSecondLengthTip, tType );
	//}
}

HSBool CLinearTransfer::LinearSeek( HSDouble tSecond, HSDouble tSecondLengthTip, SAMPLE_SEEK_TYPE tType )
{
	HSInt64 tSamples = ( HSInt )( tSecond * mDataHandler->SampleRate( mIdentifier ) );
	HSInt tTipSamplesCount = ( HSInt )( tSecondLengthTip * mDataHandler->SampleRate( mIdentifier ) );
	mNeedRefresh = HSTrue;
	if ( tType == SET_SAMPLE )
	{
		HSInt tSampleReadIndex = ( HSInt )( tSamples / mSamplesInOneFrame );
		HSInt tSampleReadOffset = ( HSInt )( tSamples % mSamplesInOneFrame );
		if ( mIndexManager->GetIndex( mDataHandler->DataIdentifier( mIdentifier ), tSampleReadIndex, mFrameIndex, mFrameOffset, mFrameLength ) 
			&& mListBuffer->SeekToPosition( mCurItem, mFrameIndex ) )
		{			
			mSampleReadIndex = tSampleReadIndex;
			mSampleReadOffset = tSampleReadOffset;
			return HSTrue;
		}

		return HSFalse;
	}
	else
	{
		HSInt64 tPureSampleReadOffset = ( ( HSInt64 )mSamplesInOneFrame ) * mSampleReadIndex + mSampleReadOffset;
		tPureSampleReadOffset += tSamples;
		HSInt tSampleReadIndex = ( HSInt )( tPureSampleReadOffset / mSamplesInOneFrame );
		HSInt tSampleReadOffset = ( HSInt )( tPureSampleReadOffset % mSamplesInOneFrame );

		HSInt64 tTotalSampleSize = 0;
		if ( tPureSampleReadOffset <= 0 )
		{
			tSampleReadIndex = 0;
			tSampleReadOffset = 0;
		}
		else
		{
			/* force the seek is correct */
			HSInt tTotalIndexCount = mIndexManager->IndexCountWithType( mDataHandler->DataIdentifier( mIdentifier ) );			
			HSInt64 tOffsetIndexSize = tTotalIndexCount * mSamplesInOneFrame - tTipSamplesCount;
			tTotalSampleSize = tTotalIndexCount * mSamplesInOneFrame;
			HSInt tFinalSampleReadIndex = 0;
			HSInt tFinalSampleReadOffset = 0;
			if ( tOffsetIndexSize > 0 )
			{			
				tFinalSampleReadIndex = ( HSInt )( tOffsetIndexSize / mSamplesInOneFrame );
				tFinalSampleReadOffset = tOffsetIndexSize % mSamplesInOneFrame;
			}

			if ( tSampleReadIndex > tFinalSampleReadIndex )
			{
				tSampleReadIndex = tFinalSampleReadIndex;
				tSampleReadOffset = tFinalSampleReadOffset;
				mGotToEnd = HSTrue;
			}
			else if ( tSampleReadIndex == tFinalSampleReadIndex && tSampleReadOffset >= tFinalSampleReadOffset )
			{
				tSampleReadOffset = tFinalSampleReadOffset;
				mGotToEnd = HSTrue;
			}
		}
		
		if ( mIndexManager->GetIndex( mDataHandler->DataIdentifier( mIdentifier ), tSampleReadIndex, mFrameIndex, mFrameOffset, mFrameLength ) )
		{
			mListBuffer->SeekToPosition( mCurItem, mFrameIndex );

			if ( mSampleReadIndex == tSampleReadIndex && mSampleReadOffset == tSampleReadOffset && tTipSamplesCount < tTotalSampleSize )
			{
				mNeedRefresh = HSFalse;
			}

			mSampleReadIndex = tSampleReadIndex;
			mSampleReadOffset = tSampleReadOffset;

			return HSTrue;
		}

		return HSFalse;
	}
}

HSInt CLinearTransfer::LinearRead( HSChar *pBuf, HSDouble tSecond )
{
	HSUInt tSamples = ( HSUInt )( tSecond * mDataHandler->SampleRate( mIdentifier ) );
	HSInt tSampleIndex = mSampleReadIndex;
	HSInt tSampleOffset = mSampleReadOffset;

	HSUInt tSamplesRead = 0;
	HSChar *pListItemBuf = NULL;
	while ( tSamplesRead < tSamples )
	{				
		if ( !mListBuffer->ReadData( mFrameIndex, mFrameOffset, mCurItem, pListItemBuf ) )
		{			
			break;
		}

		tSamplesRead += mDataHandler->GetChannelData( mIdentifier, pListItemBuf, &pBuf[ tSamplesRead * mDataHandler->EachSampleSize() ], tSampleOffset, tSamples - tSamplesRead );
		tSampleOffset = 0;

		if ( !mIndexManager->GetIndex( mDataHandler->DataIdentifier( mIdentifier ), ++tSampleIndex, mFrameIndex, mFrameOffset, mFrameLength ) )
		{
			break;
		}		
	}

	return tSamplesRead;
}


HSBool CLinearTransfer::GetIndexItemTime( HSUInt tIndex, HSInt64 &tTime, HSInt64 &tTimeLength )
{
	HSUInt tResIndex = 0;
	HSUInt tResOffset = 0;
	HSUInt tResLength = 0;	
	if ( !mIndexManager->GetIndex( mCHTimeIdentifier, tIndex, tResIndex, tResOffset, tResLength ) )
	{
		return HSFalse;
	}	

	tTime = tResIndex;
	tTime *= ( 10 * 1024 * 1024 );
	tTime += tResOffset;

	tTimeLength = tResLength / mDataHandler->EachSampleSize();
	tTimeLength *= 1000000000;
	tTimeLength /= mDataHandler->SampleRate( mIdentifier );	

	return HSTrue;
}

HSInt64 CLinearTransfer::TotalSampleNSecondLength()
{
	HSInt tTotalIndexCount = mIndexManager->IndexCountWithType( mCHTimeIdentifier );
	
	if ( tTotalIndexCount < 1 )
	{
		return 0;
	}

	HSInt64 tTime = 0;
	HSInt64 tTimeLength = 0;

	GetIndexItemTime( tTotalIndexCount - 1, tTime, tTimeLength );

	return tTime + tTimeLength;	
}

HSBool CLinearTransfer::SampleReadInfoWithTime( HSInt64 tTime, HSInt &tSampleReadIndex, HSInt &tSampleReadOffset )
{	
	HSInt64 tCurNSecond = 0;
	HSInt64 tCurNSecondLength = 0;

	tSampleReadIndex = 0;
	tSampleReadOffset = 0;
	while ( GetIndexItemTime( tSampleReadIndex, tCurNSecond, tCurNSecondLength ) )
	{
		if ( tCurNSecond >= tTime )
		{			
			return HSTrue;
		}
		else if ( tCurNSecond + tCurNSecondLength >= tTime )
		{
			tSampleReadOffset = ( HSInt )( ( tTime - tCurNSecond ) / 1000000000.0 * mDataHandler->SampleRate( mIdentifier ) );			
			return HSTrue;
		}	

		tSampleReadIndex++;
	}

	return HSFalse;
}

HSBool CLinearTransfer::SegmentSeek( HSDouble tSecond, HSDouble tSecondLengthTip, SAMPLE_SEEK_TYPE tType )
{	
	mNeedRefresh = HSTrue;
	HSInt64 tCurReadTime = 0;	
	HSInt64 tFinalReadTime = TotalSampleNSecondLength();
	if ( tType == SET_SAMPLE )
	{						
		tCurReadTime = ( HSInt64 )( tSecond * 1000000000 );		
	}
	else
	{			
		tCurReadTime = min( tFinalReadTime - ( HSInt64 )( tSecondLengthTip * 1000000000 ), mCurReadTime + ( HSInt64 )( tSecond * 1000000000 ) );
		tCurReadTime = max( 0, tCurReadTime );
	}	

	HSInt tSampleReadIndex = 0;
	HSInt tSampleReadOffset = 0;
	if ( !SampleReadInfoWithTime( tCurReadTime, tSampleReadIndex, tSampleReadOffset ) )
	{		
		return HSFalse;
	}

	if ( mIndexManager->GetIndex( mDataHandler->DataIdentifier( mIdentifier ), tSampleReadIndex, mFrameIndex, mFrameOffset, mFrameLength ) 
		&& mListBuffer->SeekToPosition( mCurItem, mFrameIndex ) )
	{			
		if ( mSampleReadIndex == tSampleReadIndex && mSampleReadOffset == tSampleReadOffset && ( HSInt64 )( tSecondLengthTip * 1000000000 ) < tFinalReadTime )
		{
			mNeedRefresh = HSFalse;
		}

		mSampleReadIndex = tSampleReadIndex;
		mSampleReadOffset = tSampleReadOffset;

		if ( mCurReadTime >= tCurReadTime )
		{
			mGotToEnd = HSTrue;
		}

		mCurReadTime = tCurReadTime;	

		return HSTrue;
	}

	return HSFalse;
}

HSInt CLinearTransfer::SegmentRead( HSChar *pBuf, HSDouble tSecond )
{
	HSUInt tSamples = ( HSUInt )( tSecond * mDataHandler->SampleRate( mIdentifier ) );
	HSInt tSampleIndex = mSampleReadIndex;
	HSInt tSampleOffset = mSampleReadOffset;

	HSUInt tSamplesRead = 0;
	HSChar *pListItemBuf = NULL;

	memset( pBuf, 0, tSamples * mDataHandler->EachSampleSize() );

	HSInt64 tCurNSecond = 0;
	HSInt64 tCurNSecondLength = 0;
	HSInt64 tLastItemSecond = 0;

	GetIndexItemTime( tSampleIndex, tCurNSecond, tCurNSecondLength );
	tLastItemSecond = tCurNSecond + tCurNSecondLength;

	if ( tSampleOffset == 0 )
	{			
		tSamplesRead += ( HSInt )( ( tCurNSecond - mCurReadTime ) / 1000000000.0 * mDataHandler->SampleRate( mIdentifier ) );
	}
	
	while ( tSamplesRead < tSamples )
	{	
		if ( !mListBuffer->ReadData( mFrameIndex, mFrameOffset, mCurItem, pListItemBuf ) )
		{			
			break;
		}

		tSamplesRead += mDataHandler->GetChannelData( mIdentifier, pListItemBuf, &pBuf[ tSamplesRead * mDataHandler->EachSampleSize() ], tSampleOffset, tSamples - tSamplesRead );
		tSampleOffset = 0;		

		if ( !mIndexManager->GetIndex( mDataHandler->DataIdentifier( mIdentifier ), ++tSampleIndex, mFrameIndex, mFrameOffset, mFrameLength ) )
		{
			break;
		}

		GetIndexItemTime( tSampleIndex, tCurNSecond, tCurNSecondLength );
		tSamplesRead += ( HSInt )( ( tCurNSecond - tLastItemSecond ) / 1000000000.0 * mDataHandler->SampleRate( mIdentifier ) );
		tLastItemSecond = tCurNSecond + tCurNSecondLength;	
	}

	return min( tSamplesRead, tSamples );
}

/*************************************************
  Function:		Init	
  Description:	初始化参数
  Input:		None			
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-03-06
  Last Time Modify:  None
**************************************************/	
HSBool CLinearTransfer::Init()
{			
	mSamplesInOneFrame = mDataHandler->SamplesInFrame( mIdentifier );
	if ( mSamplesInOneFrame == 0 )
	{
		return HSFalse;
	}	

	if ( !mIndexManager->GetIndex( mDataHandler->DataIdentifier( mIdentifier ), 0, mFrameIndex, mFrameOffset, mFrameLength ) )
	{
		return HSFalse;		
	}

	mCurItem = mListBuffer->Head();	

	mListBuffer->SeekToPosition( mCurItem, mFrameIndex );	
	mSampleReadIndex = 0;
	mSampleReadOffset = 0;
	
	mGotToEnd = HSFalse;

	return HSTrue;
}

/*************************************************
  Function:		Reset	
  Description:	初始化参数
  Input:		None			
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-03-06
  Last Time Modify:  None
**************************************************/	
HSVoid CLinearTransfer::Reset()
{	
	mGotToEnd = HSFalse;
	mNeedRefresh = HSTrue;
	mSampleReadIndex = -1;
	mCurReadTime = 0;
}

/*************************************************
  Function:		GotToEnd	
  Description:	初始化参数
  Input:		None			
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-05-04
  Last Time Modify:  None
**************************************************/	
HSBool CLinearTransfer::GotToEnd()
{	
	return mGotToEnd;
}

/*************************************************
  Function:		GetSampleReadInfo	
  Description:	获取当前帧信息，用于计算时间
  Input:		None			
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-05-04
  Last Time Modify:  None
**************************************************/	
HSInt64 CLinearTransfer::GetSampleReadInfo( HSUInt &tSampleIndex, HSUInt &tSampleOffset )
{	
	tSampleIndex = mSampleReadIndex;
	tSampleOffset = mSampleReadOffset;

	return mCurReadTime;
}