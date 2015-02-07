#pragma once

#include "BufferProtocol.h"

#include <list>
#include <vector>

using std::list;
using std::vector;

class CArrayBuffer : public IBufferProtocol
{
public:
	enum BUFFER_TYPE { BUFFER_WRITE_ONLY, BUFFER_READ_WRITE };	

private:
	enum BUFFER_STATE { BUFFER_READY, BUFFER_NEED_READ };
	typedef struct BufferInfo
	{
		HANDLE Mutex;
		HSChar *Buffer;
		HSInt Position;
		BUFFER_STATE State;
	} BufferInfo;

public:
	CArrayBuffer( BUFFER_TYPE tType = BUFFER_READ_WRITE, HSInt tBufferCount = 2, HSInt tBufferSize = 50 * 1024 * 1024 );
	~CArrayBuffer();

public:
	virtual HSBool IsBufferReady();
	virtual HSInt Write( HSChar *pBuf, HSInt tLength );
	virtual HSInt BeginReadBuffer( HSChar * &pBuf, HSInt &tLength );
	virtual HSVoid FinishReadBuffer( HSInt tBufferIndex );
	virtual HSBool WritingBuffer( HSChar * &pBuf, HSInt &tLength );
	virtual HSBool BuffersWithSize( list< BufferItem > &tBufferItems, HSInt tSize );	

private:	
	HSVoid SetBufferState( HSInt tIndex, BUFFER_STATE tState );
	BUFFER_STATE GetBufferState( HSInt tIndex );
	HSInt GetWriteBuffer( HSInt tLength );

private:
	BUFFER_TYPE mBufferType;
	HSInt mBufferCount;
	HSInt mBufferSize;
	vector< BufferInfo > mBuffers;
	HSInt mCurrentWriteIndex;
	HSInt mCurrentReadIndex;	
};

