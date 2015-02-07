#include "StdAfx.h"
#include "LinearTransfer.h"
#include <fstream>
#include "HSLogProtocol.h"
#include "ListItemManager.h"

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

	Reset();

	CListBuffer::ListItemManager()->AddListBuffer( mListBuffer );
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
	CListBuffer::ListItemManager()->ResetItem( mListBuffer, mCurItem );
}

HSInt CLinearTransfer::Read( HSUInt tSampleReadIndex, HSChar * &pBuf )
{	
	if ( mIndexManager->GetIndex( mDataHandler->DataIdentifier( mIdentifier ), tSampleReadIndex, mFrameIndex, mFrameOffset, mFrameLength ) )
	{					
		if ( mCurItem == NULL )
		{
			mCurItem = mListBuffer->Head();
		}

		if ( mListBuffer->ReadData( mFrameIndex, mFrameOffset, mCurItem, pBuf ) )
		{
			return mFrameLength;
		}
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