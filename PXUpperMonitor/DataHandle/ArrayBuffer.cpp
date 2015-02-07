#include "StdAfx.h"
#include "ArrayBuffer.h"

/*************************************************
  Function:		CArrayBuffer	
  Description:	构造函数
  Input:		tType -- 缓冲区类型
				tBufferCount -- 缓冲区个数
				tBufferSize -- 缓冲区大小
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
CArrayBuffer::CArrayBuffer( BUFFER_TYPE tType, HSInt tBufferCount, HSInt tBufferSize )
	: mBufferType( tType ), mBufferCount( tBufferCount ), mBufferSize( tBufferSize )
{
	for ( HSInt i = 0; i < tBufferCount; i++ )
	{
		BufferInfo tBufferInfo;
		tBufferInfo.Buffer = new HSChar[ tBufferSize ];
		if ( tBufferInfo.Buffer == NULL )
		{			
			continue;
		}

		memset( tBufferInfo.Buffer, 0, tBufferSize );
		tBufferInfo.Mutex = CreateEvent( NULL, false, true, NULL );
		tBufferInfo.Position = 0;
		tBufferInfo.State = BUFFER_READY;

		mBuffers.push_back( tBufferInfo );
	}
	
	mCurrentWriteIndex = 0;
	mCurrentReadIndex = 0;
}

/*************************************************
  Function:		~CArrayBuffer	
  Description:	析构函数
  Input:		None
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
CArrayBuffer::~CArrayBuffer()
{
	for ( HSUInt i = 0; i < mBuffers.size(); i++ )
	{
		delete[] mBuffers[ i ].Buffer;
		CloseHandle( mBuffers[ i ].Mutex );
	}
}

/*************************************************
  Function:		BufferCount	
  Description:	获取缓冲区数目
  Input:		None
  Output:		None  					
  Return:		结果
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
HSBool CArrayBuffer::IsBufferReady()
{	
	return ( mBufferCount == mBuffers.size() ? HSTrue : HSFalse );
}

/*************************************************
  Function:		Write	
  Description:	写数据
  Input:		pBuf -- 目标数据
				tLength -- 目标数据长度
  Output:		None  					
  Return:		0 ok, 0 failed
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
HSInt CArrayBuffer::Write( HSChar *pBuf, HSInt tLength )
{
	HSInt tBufferIndex = GetWriteBuffer( tLength );
	if ( tBufferIndex < 0 )
	{
		return 0;
	}

	HSInt tPosition = mBuffers[ tBufferIndex ].Position;
	memcpy( &( mBuffers[ tBufferIndex ].Buffer[ tPosition ] ), pBuf, tLength );
	mBuffers[ tBufferIndex ].Position += tLength;	
	
	return 1;
}

/*************************************************
  Function:		GetWriteBuffer	
  Description:	获取可用缓冲区
  Input:		tLength -- 目标数据长度				
  Output:		None  					
  Return:		-1 failed, else ok
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
HSInt CArrayBuffer::GetWriteBuffer( HSInt tLength )
{
	/* 只写模式，将覆盖以前数据 */
	if ( mBufferType == BUFFER_WRITE_ONLY )
	{
		if ( mBuffers[ mCurrentWriteIndex ].Position + tLength > mBufferSize )
		{
			mCurrentWriteIndex = ( mCurrentWriteIndex + 1 ) % mBufferCount;
			mBuffers[ mCurrentWriteIndex ].Position = 0;
			memset( mBuffers[ mCurrentWriteIndex ].Buffer, 0, mBufferSize );
		}

		return  mCurrentWriteIndex;
	}
	else	/* 读写模式，读完之后才能写 */
	{
		/* 查看第一个缓冲区 */
		if ( BUFFER_READY == GetBufferState( mCurrentWriteIndex ) )
		{
			if ( mBuffers[ mCurrentWriteIndex ].Position + tLength <= mBufferSize )
			{
				return mCurrentWriteIndex;
			}
			
			SetBufferState( mCurrentWriteIndex, BUFFER_NEED_READ );		
			mCurrentWriteIndex = ( mCurrentWriteIndex + 1 ) % mBufferCount;

			/* 查看相邻缓冲区 */
			if ( BUFFER_READY == GetBufferState( mCurrentWriteIndex ) )
			{
				return mCurrentWriteIndex;
			}
		}		
	}
	
	return -1;
}

/*************************************************
  Function:		GetBufferState	
  Description:	获取缓冲区状态
  Input:		tIndex -- 缓冲区编号				
  Output:		None  					
  Return:		结果
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
CArrayBuffer::BUFFER_STATE CArrayBuffer::GetBufferState( HSInt tIndex )
{
	WaitForSingleObject( mBuffers[ tIndex ].Mutex, INFINITE );

	BUFFER_STATE tState = mBuffers[ tIndex ].State;

	SetEvent( mBuffers[ tIndex ].Mutex );

	return tState;
}

/*************************************************
  Function:		SetBufferState	
  Description:	设置缓冲区状态
  Input:		tIndex -- 缓冲区编号
				tState -- 目标状态
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
HSVoid CArrayBuffer::SetBufferState( HSInt tIndex, BUFFER_STATE tState )
{
	WaitForSingleObject( mBuffers[ tIndex ].Mutex, INFINITE );

	mBuffers[ tIndex ].State = tState;

	SetEvent( mBuffers[ tIndex ].Mutex );
}

/*************************************************
  Function:		BeginReadBuffer	
  Description:	开始读数据
  Input:		pBuf -- 数据缓冲区地址				
  Output:		pBuf -- 目标数据
				tLength -- 目标数据长度
  Return:		缓冲区编号, 用于 FinishReadBuffer
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
HSInt CArrayBuffer::BeginReadBuffer( HSChar * &pBuf, HSInt &tLength )
{
	HSInt tBeginIndex = mCurrentReadIndex;
	do
	{
		if ( BUFFER_NEED_READ == GetBufferState( mCurrentReadIndex ) )
		{
			pBuf = mBuffers[ mCurrentReadIndex ].Buffer;
			tLength = mBuffers[ mCurrentReadIndex ].Position;

			return mCurrentReadIndex;
		}		
	
		mCurrentReadIndex = ( mCurrentReadIndex + 1 ) % mBufferCount;

	} while ( mCurrentReadIndex != tBeginIndex );

	return -1;
}

/*************************************************
  Function:		FinishReadBuffer	
  Description:	读数据结束
  Input:		tBufferIndex -- 要结束的缓冲区				
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
HSVoid CArrayBuffer::FinishReadBuffer( HSInt tBufferIndex )
{	
	mBuffers[ tBufferIndex ].Position = 0;
	memset( mBuffers[ tBufferIndex ].Buffer, 0, mBufferSize );

	SetBufferState( tBufferIndex, BUFFER_READY );
	
	mCurrentReadIndex = ( tBufferIndex + 1 ) % mBufferCount;
}

/*************************************************
  Function:		BuffersWithSize	
  Description:	用于采样停止时，将相应大小的数据保存到文件
  Input:		tSize -- 文件大小
  Output:		tBufferItems -- 缓冲区列表					
  Return:		1 ok, 0 failed
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
HSBool CArrayBuffer::BuffersWithSize( list< BufferItem > &tBufferItems, HSInt tSize )
{
	HSInt tIndex = mCurrentWriteIndex;
	do
	{
		if ( mBuffers[ tIndex ].Position > 0 )
		{			
			HSInt tBeginPos = 0;
			HSInt tBufferFetchSize = mBuffers[ tIndex ].Position;
			if ( mBuffers[ tIndex ].Position > tSize )
			{
				tBeginPos = mBuffers[ tIndex ].Position - tSize;
				tBufferFetchSize = tSize;
			}
						
			tSize -= tBufferFetchSize;

			BufferItem tItem = { &( mBuffers[ tIndex ].Buffer[ tBeginPos ] ), tBufferFetchSize };
			tBufferItems.push_front( tItem );				
		}
	
		/* 向后搜索 */
		tIndex = ( tIndex - 1 + mBufferCount ) % mBufferCount;

	} while ( mCurrentReadIndex != tIndex && tSize > 0 );

	return ( tBufferItems.size() > 0 ? HSTrue : HSFalse );
}

/*************************************************
  Function:		WritingBuffer	
  Description:	返回停止写数据时，在当前写缓冲区里的数据
  Input:		pBuf -- 数据缓冲区地址				
  Output:		pBuf -- 目标数据
				tLength -- 目标数据长度					
  Return:		1 ok, 0 failed
  Author:		Guo Dengjia
  Date :		2014-03-04
  Last Time Modify:  None
**************************************************/	
HSBool CArrayBuffer::WritingBuffer( HSChar * &pBuf, HSInt &tLength )
{
	if ( mBuffers[ mCurrentWriteIndex ].State == BUFFER_NEED_READ )
	{
		return HSFalse;
	}

	pBuf = mBuffers[ mCurrentWriteIndex ].Buffer;
	tLength = mBuffers[ mCurrentWriteIndex ].Position;

	return HSTrue;
}