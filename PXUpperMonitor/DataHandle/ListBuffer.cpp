#include "StdAfx.h"
#include "ListBuffer.h"
#include "HSLogProtocol.h"
#include "ListItemManager.h"

/*  Class CListItem Function   */

/*************************************************
  Function:		CListItem	
  Description:	���캯��
  Input:		tBufferLength -- ����������
				tRefCount -- ���������ü���
				pPrev -- ǰһ���ڵ�
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-19	
  Last Time Modify:  None
**************************************************/
CListBuffer::CListItem::CListItem( HSInt tRefCount, HSChar *pBuf, BufferState tState, CListItem *pPrev )	
{
	this->mBuffer = pBuf;
	if ( this->mBuffer != NULL || tState == BUFFER_RELEASED )
	{
		this->mPrev = pPrev;
		this->mNext = NULL;
		this->mMutex = CreateEvent( NULL, false, true, NULL );
		this->mBufferPos = 0;
		this->mRefCount = tRefCount;
		this->mState = tState;	
		this->mIndex = 0;

		if ( pPrev != NULL )
		{
			pPrev->mNext = this;
			this->mIndex = pPrev->mIndex + 1;
		}
	}	
}

/*************************************************
  Function:		~CListItem	
  Description:	��������
  Input:		None
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-19	
  Last Time Modify:  None
**************************************************/
CListBuffer::CListItem::~CListItem()
{
	if ( mBuffer != NULL )
	{
		delete[] mBuffer;			
	}

	CloseHandle( mMutex );
}

/*************************************************
  Function:		State	
  Description:	���ؽڵ�״̬
  Input:		None
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-28	
  Last Time Modify:  None
**************************************************/
CListBuffer::BufferState CListBuffer::CListItem::State()
{
	WaitForSingleObject( this->mMutex, INFINITE );
	BufferState tState = this->mState;
	SetEvent( this->mMutex );

	return tState;
}

/*************************************************
  Function:		SetState	
  Description:	���ýڵ�״̬
  Input:		tState -- ״̬
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-28	
  Last Time Modify:  None
**************************************************/
HSVoid CListBuffer::CListItem::SetState( CListBuffer::BufferState tState )
{
	WaitForSingleObject( this->mMutex, INFINITE );
	this->mState = tState;
	SetEvent( this->mMutex );
}

/*************************************************
  Function:		AddReference	
  Description:	���ӽڵ�����
  Input:		None
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-03-06
  Last Time Modify:  None
**************************************************/
HSVoid CListBuffer::CListItem::AddReference()
{
	WaitForSingleObject( this->mMutex, INFINITE );
	this->mRefCount++;	
	SetEvent( this->mMutex );
}

/*************************************************
  Function:		ReleaseReference	
  Description:	�ͷŽڵ�����
  Input:		tState -- ״̬
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-03-06
  Last Time Modify:  None
**************************************************/
HSVoid CListBuffer::CListItem::ReleaseReference()
{
	WaitForSingleObject( this->mMutex, INFINITE );
	this->mRefCount--;
	SetEvent( this->mMutex );
}

/*************************************************
  Function:		EnoughSpaceForLength	
  Description:	�鿴�Ƿ����㹻�ռ�
  Input:		tLength -- ����
  Output:		None  					
  Return:		���
  Author:		Guo Dengjia
  Date :		2014-06-04
  Last Time Modify:  None
**************************************************/
HSBool CListBuffer::CListItem::EnoughSpaceForLength( HSUInt tLength, HSUInt tBufferLength )
{
	WaitForSingleObject( this->mMutex, INFINITE );	
	HSBool tValue = ( mBufferPos + tLength <= tBufferLength );
	SetEvent( this->mMutex );

	return tValue;
}


/*   Class CListBuffer Function   */

/*************************************************
  Function:		ListItemManager	
  Description:	����ListItem
  Input:		None
  Output:		None  					
  Return:		���
  Author:		Guo Dengjia
  Date :		2014-10-30
  Last Time Modify:  None
**************************************************/
CListItemManager * CListBuffer::ListItemManager()
{
	static CListItemManager sListItemManager;
	return &sListItemManager;
}

/*************************************************
  Function:		CListBuffer	
  Description:	���캯��
  Input:		tBufferLength -- ����������
				tNumBufferLimit -- ��������������
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-28
  Last Time Modify:  None
**************************************************/
CListBuffer::CListBuffer( HSInt tBufferLength, HSInt tNumBufferLimit, HSInt tRefCount )
	: mBufferLength( tBufferLength ), mNumBufferLimit( tNumBufferLimit ), mListItemDefaultRefCount( tRefCount )
{	
	mFreeBufferMutex = CreateEvent( NULL, false, true, NULL );
	mBufferCount = 0;

	EnsureBufferCount( DEFAULT_BUFFER_COUNT );

	mHead = new CListItem( mListItemDefaultRefCount, GetFreeBuffer(), BUFFER_WRITING );

	mTail = mHead;	
	mCurWriteItem = mHead;
	mCurReadItem = mHead;	

	mFileObserver = NULL;

	mCardIdentifier = 0;
}

/*************************************************
  Function:		~CListBuffer	
  Description:	��������
  Input:		None
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-28
  Last Time Modify:  None
**************************************************/
CListBuffer::~CListBuffer()
{
	ListItemManager()->BeginReset();

	CListItem *pItem = this->mHead;
	while ( pItem != NULL )
	{
		CListItem *pTmpItem = pItem;
		pItem = pItem->mNext;
	
		delete pTmpItem;		
	}

	list< HSChar * >::iterator pIterator = mReleasedBuffers.begin();
	while ( pIterator != mReleasedBuffers.end() )
	{
		delete[] *pIterator;
		pIterator++;		
	}

	CloseHandle( mFreeBufferMutex );
}

/*************************************************
  Function:		BeginWrite	
  Description:	��ʼд����
  Input:		pBuf -- Ŀ������
  Output:		None  					
  Return:		0 ok, 0 failed
  Author:		Guo Dengjia
  Date :		2014-02-28
  Last Time Modify:  None
**************************************************/	
HSBool CListBuffer::BeginWrite( HSChar * &pBuf, HSUInt tLength )
{
	if ( BUFFER_WRITING != mCurWriteItem->State() )
	{
		return HSFalse;
	}	

	if ( !mCurWriteItem->EnoughSpaceForLength( tLength, mBufferLength ) )
	{
		HSUInt tIndex = 0;
		HSUInt tOffset = 0;
		FinishWrite( 0, tIndex, tOffset, HSTrue );
	}

	WaitForSingleObject( mCurWriteItem->mMutex, INFINITE );		

	pBuf = &mCurWriteItem->mBuffer[ mCurWriteItem->mBufferPos ];

	SetEvent( mCurWriteItem->mMutex );

	return HSTrue;
}

/*************************************************
  Function:		FinishWrite	
  Description:	���д����
  Input:		pBuf -- Ŀ������
				tLength -- Ŀ�����ݳ���
  Output:		None  					
  Return:		0 ok, 0 failed
  Author:		Guo Dengjia
  Date :		2014-02-28
  Last Time Modify:  None
**************************************************/	
HSVoid CListBuffer::FinishWrite( HSInt tLength, HSUInt &tIndex, HSUInt &tOffset, HSBool tForceFinish )
{
	CListItem *pCurWriteItem = mCurWriteItem;

	WaitForSingleObject( pCurWriteItem->mMutex, INFINITE );	
	
	tIndex = mCurWriteItem->mIndex;
	tOffset = mCurWriteItem->mBufferPos;
	mCurWriteItem->mBufferPos += tLength;

	if ( mCurWriteItem->mBufferPos == mBufferLength || tForceFinish )
	{		
		mCurWriteItem->mState = BUFFER_READY;
		mCurWriteItem->mRefCount--;

		CListItem *pNewItem = new CListItem( mListItemDefaultRefCount, GetFreeBuffer( 1 ), BUFFER_WRITING, mCurWriteItem );
		if ( pNewItem->mBuffer != NULL )
		{			
			mTail = pNewItem;
			mCurWriteItem = mTail;				
		}		
		else
		{
			delete pNewItem;
		}
	}

	SetEvent( pCurWriteItem->mMutex );
}

/*************************************************
  Function:		BeginReadBuffer	
  Description:	��ʼ������
  Input:		pBuf -- ���ݻ�������ַ				
  Output:		pBuf -- Ŀ������
				tLength -- Ŀ�����ݳ���
  Return:		���������, ���� FinishReadBuffer
  Author:		Guo Dengjia
  Date :		2014-02-28
  Last Time Modify:  None
**************************************************/	
HSInt CListBuffer::BeginReadBuffer( HSChar * &pBuf, HSInt &tLength )
{
	if ( mCurReadItem->State() == BUFFER_READY )
	{
		pBuf = mCurReadItem->mBuffer;
		tLength = mCurReadItem->mBufferPos;

		return 1;
	}

	return -1;
}

/*************************************************
  Function:		FinishReadBuffer	
  Description:	�����ݽ���
  Input:		tBufferIndex -- Ҫ�����Ļ�����				
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-28
  Last Time Modify:  None
**************************************************/
HSVoid CListBuffer::FinishReadBuffer( HSInt tBufferIndex )
{
	CListItem *pCurReadItem = mCurReadItem;
	WaitForSingleObject( pCurReadItem->mMutex, INFINITE );

	mCurReadItem->mState = BUFFER_SAVED;
	mCurReadItem->mRefCount--;
	if ( mCurReadItem != mTail )
	{
		mCurReadItem = mCurReadItem->mNext;
	}

	SetEvent( pCurReadItem->mMutex );

	ReleaseBuffer( pCurReadItem );
}

/*************************************************
  Function:		BuffersWithSize	
  Description:	���ڲ���ֹͣʱ������Ӧ��С�����ݱ��浽�ļ�
  Input:		tSize -- �ļ���С
  Output:		tBufferItems -- �������б�					
  Return:		1 ok, 0 failed
  Author:		Guo Dengjia
  Date :		2014-02-28
  Last Time Modify:  None
**************************************************/	
HSBool CListBuffer::BuffersWithSize( list< BufferItem > &tBufferItems, HSInt tSize )
{	
	CListItem *pItem = mCurWriteItem;
	while ( pItem != NULL && tSize > 0 )
	{
		if ( pItem->mBufferPos > 0 )
		{			
			HSInt tBeginPos = 0;
			HSInt tBufferFetchSize = pItem->mBufferPos;
			if ( pItem->mBufferPos > tSize )
			{
				tBeginPos = pItem->mBufferPos - tSize;
				tBufferFetchSize = tSize;
			}
						
			tSize -= tBufferFetchSize;

			if ( pItem->State() == BUFFER_RELEASED && this->LoadData( pItem ) == 0 )
			{				
				return HSFalse;
			}

			BufferItem tItem = { &( pItem->mBuffer[ tBeginPos ] ), tBufferFetchSize };
			tBufferItems.push_front( tItem );				
		}

		pItem = pItem->mPrev;
	}	
	
	return ( tBufferItems.size() > 0 ? HSTrue : HSFalse );
}	

/*************************************************
  Function:		IsBufferReady	
  Description:	�������Ƿ����
  Input:		None
  Output:		None  					
  Return:		1 ok, 0 failed
  Author:		Guo Dengjia
  Date :		2014-02-28
  Last Time Modify:  None
**************************************************/
HSBool CListBuffer::IsBufferReady()
{
	return ( mHead->mBuffer != NULL ? HSTrue : HSFalse );
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
HSBool CListBuffer::WritingBuffer( HSChar * &pBuf, HSInt &tLength )
{
	if ( mCurWriteItem->State() != BUFFER_WRITING )
	{
		return HSFalse;
	}

	pBuf = mCurWriteItem->mBuffer;
	tLength = mCurWriteItem->mBufferPos;

	return HSTrue;
}

/*************************************************
  Function:		Head	
  Description:	����ͷ�ڵ�
  Input:		None
  Output:		None  					
  Return:		���
  Author:		Guo Dengjia
  Date :		2014-03-03
  Last Time Modify:  None
**************************************************/
CListBuffer::CListItem * CListBuffer::Head()
{
	WaitForSingleObject( mHead->mMutex, INFINITE );	
	mHead->mRefCount++;
	SetEvent( mHead->mMutex );

	return mHead;
}

/*************************************************
  Function:		SetFileObserver	
  Description:	�����ļ����ڵ㻺���ͷź���Ҫʱ�����ļ�����
  Input:		pFileObserver -- �ļ�
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-03-03
  Last Time Modify:  None
**************************************************/
HSVoid CListBuffer::SetFileObserver( CFileObserver *pFileObserver )
{
	mFileObserver = pFileObserver;
}

/*************************************************
  Function:		SetCardIdentifier	
  Description:	each card have a listbuffer
  Input:		tCardIdentifier -- card identifier
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-04-24
  Last Time Modify:  None
**************************************************/
HSVoid CListBuffer::SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier )
{
	mCardIdentifier = tCardIdentifier;
}

/*************************************************
  Function:		SetDefaultRefCount	
  Description:	���ýڵ���������������Ϊ0ʱ�����������ܻᱻ�ͷ�
  Input:		tRefCount -- ������
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-03-03
  Last Time Modify:  None
**************************************************/
HSVoid CListBuffer::SetDefaultRefCount( HSInt tRefCount )
{
	mListItemDefaultRefCount = tRefCount;
	mHead->mRefCount = tRefCount;
}


/*************************************************
  Function:		ReadData	
  Description:	��Ŀ��ڵ��ȡ���ݣ��˽ڵ���ܻ���ǰ(length<0)��( length > 0 )�ƶ�
  Input:		pItem -- Ҫ���Ľڵ�
				pBuf -- �洢����
				tOffset -- �ڵ㻺����ƫ��
				tLength -- Ҫ�������ݳ���
  Output:		None  					
  Return:		���
  Author:		Guo Dengjia
  Date :		2014-03-03
  Last Time Modify:  None
**************************************************/
HSBool CListBuffer::ReadData( HSInt tIndex, HSInt tOffset, CListItem * &pItem, HSChar * &pBuf )
{
	SeekToPosition( pItem, tIndex );

	WaitForSingleObject( pItem->mMutex, INFINITE );	
	
	if ( pItem->mState == BUFFER_RELEASED && !LoadData( pItem ) )
	{
		SetEvent( pItem->mMutex );
		
		return HSFalse;
	}	

	pBuf = &pItem->mBuffer[ tOffset ];

	SetEvent( pItem->mMutex );

	return HSTrue;
}


/*************************************************
  Function:		SeekToPosition	
  Description:	�ض�λ��ĳ���ڵ�
  Input:		tPosition -- ֡λ��
  Output:		pItem -- Ŀ��ڵ�  	
				tOffset -- Ŀ��ڵ㻺����ƫ��ֵ
  Return:		0 failed, 1 ok
  Author:		Guo Dengjia
  Date :		2014-03-03
  Last Time Modify:  None
**************************************************/
HSBool CListBuffer::SeekToPosition( CListItem * &pItem, HSInt tIndex )
{	
	if ( pItem && pItem->mIndex == tIndex )
	{		
		return HSTrue;
	}

	CListItem *pCurItem = mHead;
	while ( tIndex > 0 && pCurItem != mTail )
	{
		pCurItem = pCurItem->mNext;
		tIndex--;
	}
	
	if ( tIndex == 0 )
	{		
		pCurItem->AddReference();

		if ( pItem != NULL )
		{
			pItem->ReleaseReference();
			ReleaseBuffer( pItem );
		}

		pItem = pCurItem;

		return HSTrue;
	}

	return HSFalse;
}

/*************************************************
  Function:		ReleaseBuffer	
  Description:	�ͷŲ��õ��ڴ�ռ䣬�����湩�Ժ�ʹ��
  Input:		tStopWhenInUse -- ����ĳ���ڵ����õ�ʱ���Ƿ�ֹͣ�ѡ���
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-03-03
  Last Time Modify:  None
**************************************************/
HSVoid CListBuffer::ReleaseBuffer( CListItem *pItem )
{	
	if ( pItem == NULL || WAIT_TIMEOUT == WaitForSingleObject( pItem->mMutex, 10 ) )
	{
		return;
	}

	/* could relead this node */
	if ( pItem->mRefCount == 0 && pItem->mBuffer != NULL )
	{
		WaitForSingleObject( mFreeBufferMutex, INFINITE );	
				
		mReleasedBuffers.push_back( pItem->mBuffer );
		pItem->mBuffer = NULL;
		pItem->mState = BUFFER_RELEASED;
			
		SetEvent( mFreeBufferMutex );
	}

	SetEvent( pItem->mMutex );

	EnsureBufferCount( MIN_BUFFER_COUNT );
}

/*************************************************
  Function:		GetFreeBuffer	
  Description:	��ȡ�ڴ�ռ�
  Input:		tWithLimit -- �Ƿ�ʹ�û�������������
  Output:		None  					
  Return:		���
  Author:		Guo Dengjia
  Date :		2014-03-03
  Last Time Modify:  None
**************************************************/
HSChar * CListBuffer::GetFreeBuffer( HSInt tWithLimit )
{
	HSChar *pBuffer = NULL;

	WaitForSingleObject( mFreeBufferMutex, INFINITE );		

	list< HSChar * >::iterator pIterator = mReleasedBuffers.begin();
	if ( pIterator != mReleasedBuffers.end() )
	{
		pBuffer = *pIterator;
		mReleasedBuffers.erase( pIterator );		
	}	

	/* could alloc new memory */
	if ( pBuffer == NULL && ( tWithLimit == 0 || mNumBufferLimit == 0 || mBufferCount < mNumBufferLimit ) )
	{
		pBuffer = new HSChar[ mBufferLength ];
		mBufferCount++;			
	}		

	SetEvent( mFreeBufferMutex );

	return pBuffer;
}

/*************************************************
  Function:		LoadData	
  Description:	Ϊ�ڵ��������
  Input:		pItem -- �ڵ�
  Output:		None  					
  Return:		1 ok, 0 failed
  Author:		Guo Dengjia
  Date :		2014-03-03
  Last Time Modify:  None
**************************************************/
HSBool CListBuffer::LoadData( CListItem *pItem )
{	
	/* ever read failed */
	if ( pItem->mBuffer == NULL )
	{
		pItem->mBuffer = GetFreeBuffer( 0 );
	}

	if ( pItem->mBuffer == NULL || mFileObserver == NULL )
	{
		return HSFalse;
	}	

	/* read data until success */
	HSInt tCount = 16;
	CFileObserver::READ_DATA_RES tReadRes = CFileObserver::READ_SUCCESS;
	while ( ( tReadRes = mFileObserver->ReadData( mCardIdentifier, pItem->mIndex, pItem->mBuffer, pItem->mBufferPos ) ) != CFileObserver::READ_SUCCESS && tCount > 0 )
	{
		//HS_ERROR( "Index : %d, Load Data Failed!\n", pItem->mIndex );
		tCount--;
		Sleep( 1 );
	}

	if ( tReadRes == CFileObserver::READ_SUCCESS )
	{
		pItem->mState = BUFFER_SAVED;
		return HSTrue;
	}
	else
	{
		return HSFalse;
	}	
}

/*************************************************
  Function:		InitWithFile	
  Description:	���ڻط�
  Input:		None
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-08-19
  Last Time Modify:  None
**************************************************/
HSVoid CListBuffer::InitWithFile()
{
	this->Reset();

	mHead = new CListItem( 0, NULL, BUFFER_RELEASED, NULL );
	mTail = mHead;	
	mCurWriteItem = mHead;
	mCurReadItem = mHead;

	vector< HSUInt > tItemsLength;
	HSInt tItemCount = mFileObserver->ItemsLengthWithType( mCardIdentifier, tItemsLength );

	mHead->mBufferPos = 0;
	if ( tItemCount > 0 )
	{
		mHead->mBufferPos = tItemsLength[ 0 ];
	}

	CListItem *pPrevItem = mHead;
	for ( HSInt i = 1; i < tItemCount; i++ )
	{
		 CListItem *pNewItem = new CListItem( 0, NULL, BUFFER_RELEASED, pPrevItem );
		 pNewItem->mBufferPos = tItemsLength[ i ];
		 pPrevItem = pNewItem;
	}
	
	mTail = pPrevItem;
}

/*************************************************
  Function:		Reset	
  Description:	��������
  Input:		tCreateNewHead -- �Ƿ��ؽ�����ͷ
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-03-06
  Last Time Modify:  None
**************************************************/
HSVoid CListBuffer::Reset( HSInt tCreateNewHead )
{
	CListItem *pItem = this->mHead;
	while ( pItem != NULL )
	{
		CListItem *pTmpItem = pItem;
		pTmpItem->mRefCount = 0;
		ReleaseBuffer( pTmpItem );

		pItem = pItem->mNext;
	
		delete pTmpItem;
	}
		
	EnsureBufferCount( DEFAULT_BUFFER_COUNT );

	if ( tCreateNewHead == 1 )
	{
		mHead = new CListItem( mListItemDefaultRefCount, GetFreeBuffer(), BUFFER_WRITING );
		mTail = mHead;	
		mCurWriteItem = mHead;
		mCurReadItem = mHead;
	}

	ListItemManager()->BeginReset();
}

/*************************************************
  Function:		EnsureBufferCount	
  Description:	��֤����������
  Input:		tCountType -- ����������
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-03-03
  Last Time Modify:  None
**************************************************/
HSVoid CListBuffer::EnsureBufferCount( BUFFER_COUNT_TYPE tCountType )
{	
	WaitForSingleObject( mFreeBufferMutex, INFINITE );	
	HSInt tSize = tCountType - mReleasedBuffers.size();
	SetEvent( mFreeBufferMutex );		

	if ( tSize < 1 )
	{
		return;
	}	
		
	list< HSChar * > tBuffers;
	for ( HSInt i = 0; i < tSize; i++ )
	{
		HSChar *pBuf = new HSChar[ mBufferLength ];
		if ( pBuf != NULL )
		{
			tBuffers.push_back( pBuf );
		}
	}	

	WaitForSingleObject( mFreeBufferMutex, INFINITE );	
	list< HSChar * >::iterator pIterator = tBuffers.begin();
	while ( pIterator != tBuffers.end() )
	{
		mReleasedBuffers.push_back( *pIterator );		
		pIterator++;

		mBufferCount++;		
	}

	SetEvent( mFreeBufferMutex );	
}