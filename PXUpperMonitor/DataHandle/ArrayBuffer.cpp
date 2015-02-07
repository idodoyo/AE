#include "StdAfx.h"
#include "ArrayBuffer.h"

/*************************************************
  Function:		CArrayBuffer	
  Description:	���캯��
  Input:		tType -- ����������
				tBufferCount -- ����������
				tBufferSize -- ��������С
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
  Description:	��������
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
  Description:	��ȡ��������Ŀ
  Input:		None
  Output:		None  					
  Return:		���
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
  Description:	д����
  Input:		pBuf -- Ŀ������
				tLength -- Ŀ�����ݳ���
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
  Description:	��ȡ���û�����
  Input:		tLength -- Ŀ�����ݳ���				
  Output:		None  					
  Return:		-1 failed, else ok
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
HSInt CArrayBuffer::GetWriteBuffer( HSInt tLength )
{
	/* ֻдģʽ����������ǰ���� */
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
	else	/* ��дģʽ������֮�����д */
	{
		/* �鿴��һ�������� */
		if ( BUFFER_READY == GetBufferState( mCurrentWriteIndex ) )
		{
			if ( mBuffers[ mCurrentWriteIndex ].Position + tLength <= mBufferSize )
			{
				return mCurrentWriteIndex;
			}
			
			SetBufferState( mCurrentWriteIndex, BUFFER_NEED_READ );		
			mCurrentWriteIndex = ( mCurrentWriteIndex + 1 ) % mBufferCount;

			/* �鿴���ڻ����� */
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
  Description:	��ȡ������״̬
  Input:		tIndex -- ���������				
  Output:		None  					
  Return:		���
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
  Description:	���û�����״̬
  Input:		tIndex -- ���������
				tState -- Ŀ��״̬
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
  Description:	��ʼ������
  Input:		pBuf -- ���ݻ�������ַ				
  Output:		pBuf -- Ŀ������
				tLength -- Ŀ�����ݳ���
  Return:		���������, ���� FinishReadBuffer
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
  Description:	�����ݽ���
  Input:		tBufferIndex -- Ҫ�����Ļ�����				
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
  Description:	���ڲ���ֹͣʱ������Ӧ��С�����ݱ��浽�ļ�
  Input:		tSize -- �ļ���С
  Output:		tBufferItems -- �������б�					
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
	
		/* ������� */
		tIndex = ( tIndex - 1 + mBufferCount ) % mBufferCount;

	} while ( mCurrentReadIndex != tIndex && tSize > 0 );

	return ( tBufferItems.size() > 0 ? HSTrue : HSFalse );
}

/*************************************************
  Function:		WritingBuffer	
  Description:	����ֹͣд����ʱ���ڵ�ǰд�������������
  Input:		pBuf -- ���ݻ�������ַ				
  Output:		pBuf -- Ŀ������
				tLength -- Ŀ�����ݳ���					
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