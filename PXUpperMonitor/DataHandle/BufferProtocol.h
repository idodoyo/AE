#pragma once

#include <list>
#include "DataTypes.h"

using std::list;

typedef struct BufferItem
{
	HSChar *Buffer;
	HSInt Length;
} BufferItem;

class IBufferProtocol
{
public:
	virtual HSBool BeginWrite( HSChar * &pBuf ) = 0;
	virtual HSVoid FinishWrite( HSInt tLength, HSUInt &tIndex, HSUInt &tOffset ) = 0;
	virtual HSInt BeginReadBuffer( HSChar * &pBuf, HSInt &tLength ) = 0;
	virtual HSVoid FinishReadBuffer( HSInt tBufferIndex ) = 0;
	virtual HSBool BuffersWithSize( list< BufferItem > &tBufferItems, HSInt tSize ) = 0;	
	virtual HSBool WritingBuffer( HSChar * &pBuf, HSInt &tLength ) = 0;
	virtual HSBool IsBufferReady() = 0;

	virtual ~IBufferProtocol(){}
};

