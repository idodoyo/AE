#pragma once

#include "BufferProtocol.h"
#include "FileObserver.h"

#include <list>

using std::list;

class CListBuffer //: public IBufferProtocol
{
public:
	CListBuffer( HSInt tBufferLength, HSInt tNumBufferLimit, HSInt tRefCount = 1 );
	virtual ~CListBuffer();

public:
	typedef enum { BUFFER_WRITING, BUFFER_READY, BUFFER_SAVED, BUFFER_RELEASED } BufferState;

public:
	class CListItem
	{
	public:
		CListItem( HSInt tRefCount, HSChar *pBuf, BufferState tState, CListItem *pPrev = NULL );
		~CListItem();			

	public:
		BufferState State();
		HSVoid SetState( BufferState tState );
		HSVoid AddReference();
		HSVoid ReleaseReference();
		HSBool EnoughSpaceForLength( HSUInt tLength, HSUInt tBufferLength );

	private:
		CListItem *mPrev;
		CListItem *mNext;		
		HANDLE mMutex;
		HSChar *mBuffer;
		HSInt mBufferPos;
		HSInt mRefCount;		
		BufferState mState;
		HSInt mIndex;

		friend class CListBuffer;
	};

public:
	/* IBufferProtocol ½Ó¿Ú */
	virtual HSBool BeginWrite( HSChar * &pBuf, HSUInt tLength = 0 );
	virtual HSVoid FinishWrite( HSInt tLength, HSUInt &tIndex, HSUInt &tOffset, HSBool tForceFinish = HSFalse );
	virtual HSInt BeginReadBuffer( HSChar * &pBuf, HSInt &tLength );
	virtual HSVoid FinishReadBuffer( HSInt tBufferIndex );
	virtual HSBool BuffersWithSize( list< BufferItem > &tBufferItems, HSInt tSize );	
	virtual HSBool WritingBuffer( HSChar * &pBuf, HSInt &tLength );
	virtual HSBool IsBufferReady();

public:		
	CListItem *Head();
	CListItem *Tail();	

	HSBool ReadData( HSInt tIndex, HSInt tOffset, CListItem * &pItem, HSChar * &pBuf );
	HSBool SeekToPosition( CListItem * &pItem, HSInt tIndex );
	HSVoid SetFileObserver( CFileObserver *pFileObserver );
	HSVoid SetDefaultRefCount( HSInt tRefCount );
	HSVoid SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier );

	HSVoid InitWithFile();
	HSVoid Reset( HSInt tCreateNewHead = 0 );

	CFileObserver * FileObserver(){ return mFileObserver; };
	
private:
	HSVoid ReleaseBuffer( CListItem *pItem );
	HSChar * GetFreeBuffer( HSInt tWithLimit = 1 );	

	HSBool LoadData( CListItem *pItem );	

	enum BUFFER_COUNT_TYPE{ MIN_BUFFER_COUNT = 2, DEFAULT_BUFFER_COUNT = 5 };
	HSVoid EnsureBufferCount( BUFFER_COUNT_TYPE tCountType );

private:
	CListItem *mHead;
	CListItem *mTail;

	CListItem *mCurWriteItem;
	CListItem *mCurReadItem;

	HSInt mBufferCount;
	HSInt mBufferLength;
	HSInt mNumBufferLimit;
	
	list< HSChar * > mReleasedBuffers;
	HANDLE mFreeBufferMutex;

	CFileObserver *mFileObserver;

	HSInt mListItemDefaultRefCount;

	DEVICE_CH_IDENTIFIER mCardIdentifier;

};

