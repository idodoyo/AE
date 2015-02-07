#pragma once

#include "DataTransferProtocol.h"
#include "ListBuffer.h"
#include "DataHandler.h"
#include "IndexManager.h"

class CLinearTransfer
{
public:
	enum SAMPLE_SEEK_TYPE{ OFFSET_SAMPLE, SET_SAMPLE };

public:
	CLinearTransfer( CIndexManager *pIndexManager, IDataHandler *pDataHandler, DEVICE_CH_IDENTIFIER tIdentifier );
	~CLinearTransfer();

public:
	HSInt Read( HSUInt tSampleReadIndex, HSChar * &pBuf );
	HSInt  Read( HSChar *pBuf, HSDouble tSecond );
	HSBool Seek( HSDouble tSecond, HSDouble tSecondLengthTip = 0, SAMPLE_SEEK_TYPE tType = OFFSET_SAMPLE );
	HSVoid Reset();	
	HSBool GotToEnd();
	HSBool NeedRefresh(){ return mNeedRefresh; }	
	HSInt64 GetSampleReadInfo( HSUInt &tSampleIndex, HSUInt &tSampleOffset );

private:
	HSBool Init();
	HSBool LinearSeek( HSDouble tSecond, HSDouble tSecondLengthTip, SAMPLE_SEEK_TYPE tType );
	HSBool SegmentSeek( HSDouble tSecond, HSDouble tSecondLengthTip, SAMPLE_SEEK_TYPE tType );

	HSInt LinearRead( HSChar *pBuf, HSDouble tSecond );
	HSInt SegmentRead( HSChar *pBuf, HSDouble tSecond );

	HSBool SampleReadInfoWithTime( HSInt64 tTime, HSInt &tSampleReadIndex, HSInt &tSampleReadOffset );
	HSInt64 TotalSampleNSecondLength();
	HSBool GetIndexItemTime( HSUInt tIndex, HSInt64 &tTime, HSInt64 &tTimeLength );

private:
	CIndexManager *mIndexManager;
	IDataHandler *mDataHandler;
	DEVICE_CH_IDENTIFIER mIdentifier;
	DEVICE_CH_IDENTIFIER mCHTimeIdentifier;

	HSUInt mFrameIndex;
	HSUInt mFrameOffset;
	HSUInt mFrameLength;

	HSInt mSampleReadIndex;
	HSInt mSampleReadOffset;	
	HSInt mSamplesInOneFrame;

	HSInt64 mCurReadTime;

	HSBool mGotToEnd;
	HSBool mNeedRefresh;

	CListBuffer *mListBuffer;		
	CListBuffer::CListItem *mCurItem;
};

