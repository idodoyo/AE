#pragma once

#include "DataTypes.h"
#include "ThreadControlEx.h"

template< typename T >
class CCycleBuffer
{
public:
	CCycleBuffer( T *pParent, HSInt tBufferLength, HSInt tCount = 5 );
	virtual ~CCycleBuffer();

public:
	enum BUFFER_STATE{ STATE_OK, STATE_OBSELETE, STATE_BUSY, STATE_NEW };
	HSInt BeginHandleBuffer( HSChar * &pBuffer );
	HSVoid FinishHandleBuffer( HSInt tIndex, BUFFER_STATE tState );	

	HSVoid Write( HSChar *pData );
	HSInt Read( HSChar *pBuf );

	HSVoid Init();
	HSVoid UnInit();

public:
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

private:
	HSInt mBufferLength;
	HSInt mBufferCount;	

	HSInt mWriteIndex;
	HSInt mReadIndex;
	HSInt mWorkingIndex;

	typedef struct BufferItem
	{
		HSChar *Buffer;
		BUFFER_STATE State;
		HANDLE Mutex;
	} BufferItem;	

	BufferItem *mBuffers;

	CThreadControlEx< CCycleBuffer > *mCheckThread;		

	T *mParent;
};

template< typename T >
CCycleBuffer< T >::CCycleBuffer(  T *pParent, HSInt tBufferLength, HSInt tCount )
	: mParent( pParent ), mBufferLength( tBufferLength ), mBufferCount( tCount )
{
	mBuffers = new BufferItem[ tCount ];
	for ( HSInt i = 0; i < tCount; i++ )
	{
		mBuffers[ i ].Buffer = new HSChar[ tBufferLength ];
		mBuffers[ i ].State = STATE_OBSELETE;
		mBuffers[ i ].Mutex = CreateEvent( NULL, HSFalse, HSTrue, NULL );
	}

	mWriteIndex = 0;
	mReadIndex = 0;
	mWorkingIndex = 0;
	mCheckThread = new CThreadControlEx< CCycleBuffer >( this, 0, 0 );
}

template< typename T >
CCycleBuffer< T >::~CCycleBuffer()
{
	for ( HSInt i = 0; i < mBufferCount; i++ )
	{
		delete[] mBuffers[ i ].Buffer;		
		CloseHandle( mBuffers[ i ].Mutex );
	}

	delete[] mBuffers;

	delete mCheckThread;
}

template< typename T >
HSInt CCycleBuffer< T >::BeginHandleBuffer( HSChar * &pBuffer )
{
	HSInt tIndex = -1;
	do
	{
		WaitForSingleObject( mBuffers[ mWorkingIndex ].Mutex, INFINITE );

		if ( mBuffers[ mWorkingIndex ].State == STATE_NEW )
		{
			mBuffers[ mWorkingIndex ].State = STATE_BUSY;
			tIndex = mWorkingIndex;
			pBuffer = mBuffers[ mWorkingIndex ].Buffer;

			SetEvent( mBuffers[ mWorkingIndex ].Mutex );

			break;
		}

		SetEvent( mBuffers[ mWorkingIndex ].Mutex );

		mWorkingIndex = ( mWorkingIndex + 1 ) % mBufferCount;

	} while ( mWriteIndex != mWorkingIndex );

	return tIndex;	
}

template< typename T >
HSVoid CCycleBuffer< T >::FinishHandleBuffer( HSInt tIndex, BUFFER_STATE tState )
{
	WaitForSingleObject( mBuffers[ tIndex ].Mutex, INFINITE );

	if ( mBuffers[ tIndex ].State != STATE_BUSY )
	{
		SetEvent( mBuffers[ tIndex ].Mutex );
		return;
	}

	mBuffers[ tIndex ].State = STATE_OK;
	mWorkingIndex = ( mWorkingIndex + 1 ) % mBufferCount;

	SetEvent( mBuffers[ tIndex ].Mutex );
}

template< typename T >
HSVoid CCycleBuffer< T >::Write( HSChar *pData )
{
	if ( mWriteIndex == mReadIndex )
	{
		//OutputDebugStringA( "OverRun Read Buffer!\n" );
	}

	WaitForSingleObject( mBuffers[ mWriteIndex ].Mutex, INFINITE );

	memcpy( mBuffers[ mWriteIndex ].Buffer, pData, mBufferLength );
	mBuffers[ mWriteIndex ].State = STATE_OK;//STATE_NEW;	

	SetEvent( mBuffers[ mWriteIndex ].Mutex );

	mWriteIndex = ( mWriteIndex + 1 ) % mBufferCount;
}

template< typename T >
HSInt CCycleBuffer< T >::Read( HSChar *pBuf )
{
	HSInt tRes = 0;

	WaitForSingleObject( mBuffers[ mReadIndex ].Mutex, INFINITE );		
	
	if ( mBuffers[ mReadIndex ].State == STATE_OK )
	{
		memcpy( pBuf, mBuffers[ mReadIndex ].Buffer, mBufferLength );
		mBuffers[ mReadIndex ].State = STATE_OBSELETE;	
		tRes = mBufferLength;
	}
	else if ( mBuffers[ mReadIndex ].State == STATE_NEW || mBuffers[ mReadIndex ].State == STATE_BUSY )
	{
		SetEvent( mBuffers[ mReadIndex ].Mutex );
		return 0;
	}

	SetEvent( mBuffers[ mReadIndex ].Mutex );

	mReadIndex = ( mReadIndex + 1 ) % mBufferCount;	

	return tRes;
}

template< typename T >
HSVoid CCycleBuffer< T >::Init()
{
	mCheckThread->Start();
}

template< typename T >
HSVoid CCycleBuffer< T >::UnInit()
{
	mCheckThread->Stop();
}

/*************************************************
  Function:		ThreadRuning	
  Description:	线程正在运行
  Input:		tThreadID -- 线程ID
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
template< typename T >
HSBool CCycleBuffer< T >::ThreadRuning( HSInt tThreadID )
{	
	HSChar *pBuf = NULL;
	HSInt tIndex = BeginHandleBuffer( pBuf );
	if ( tIndex >= 0 )
	{
		FinishHandleBuffer( tIndex, ( mParent->EffectiveTest( pBuf ) ? STATE_OK : STATE_OBSELETE ) );
	}

	return HSTrue;
}

/*************************************************
  Function:		ThreadWillStop	
  Description:	线程停止， 更新Observer
  Input:		tThreadID -- 线程ID
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
template< typename T >
HSVoid CCycleBuffer< T >::ThreadWillStop( HSInt tThreadID )
{
}
