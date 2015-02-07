#pragma once

#include "DataTypes.h"
#include "DeviceTypes.h"
#include "DataHandler.h"
#include "ListBuffer.h"
#include "ThreadControlEx.h"
#include "FileObserver.h"
#include "IndexManager.h"
#include "PACP2LV.H"
#include "Pacpci2.h"
#include "ArgHandler.h"
#include "PAPCI-2ArgFetcher.h"

class CPAPCI2Handler : public CArgHandler
{	
public:
	CPAPCI2Handler(  CIndexManager *pIndexManager, CFileObserver *pFileObserver, HSInt tIndex );
	~CPAPCI2Handler();		

public:	
	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual HSString Name();
	virtual DEVICE_CH_IDENTIFIER Identifier(){ return DEVICE_CH_IDENTIFIER( CARD_PAPCI2, mDataIdentifier.CARD_INDEX ); }
	virtual HSUInt SampleRate( DEVICE_CH_IDENTIFIER tChannelIdentifier ){ return mChannels[ tChannelIdentifier.ChannelIndex() + 1 ].SampleRate * 1000; }
	virtual HSUInt EachSampleSize(){ return sizeof( HSFloat ); }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier( DEVICE_CH_IDENTIFIER tChannelIdentifier = 0 ){ tChannelIdentifier.InitChannel( tChannelIdentifier.ChannelIndex() + 1 ); return tChannelIdentifier; }
	virtual HSUInt SamplesInFrame( DEVICE_CH_IDENTIFIER tChannelIdentifier ){ return mSampleInOneFrame; }
	virtual HSUInt GetChannelData( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSChar *pSrcBuf, HSChar *pDestBuf, HSInt tOffset, HSInt tSamples );

	virtual HSDouble VoltageWithPointer( HSChar *pPointer ){ return *( HSFloat* )pPointer * 10.0 / 32768.0; }
	virtual HSDouble BypassVoltage( HSUInt tChannel ){ return 0.001; }
	
	virtual HSInt ChannelNum(){ return mChannels.size(); }
	virtual DEVICE_TYPE Type(){ return DEVICE_PCI_CARD; }

	virtual HSBool OpenChannel( vector< HSInt > &tChannels, vector< HSInt > *pSampleRates = NULL, HSString *pErrorMsg = NULL );
	virtual HSBool CloseAllChannel();

	virtual CListBuffer * ListBuffer(){ return mListBuffer; }

	virtual IArgFetcher * ArgFetcher(){ return new CPAPCI2ArgFetcher( this ); }

	virtual HSBool IsOn();	

	virtual HSBool DataIsInSegent(){ return HSTrue; }

public:
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

	static HSInt GetDevice( vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver );
	static HSBool GetDevice( DEVICE_CH_IDENTIFIER tIdentifier, vector< HSInt > &tChannels, vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver );
	static HSVoid CloseDevice();

	HSBool EffectiveTest( HSChar *pBuffer );	

public:
	typedef struct CHANNEL_INFO
	{
		HSBool  On;
		HSULong SampleLength;
		HSFloat *Buffer;
		HSUInt  ThresholdType;
		HSUInt  PreTrigger;
		HSUInt	SampleRate;	
		HSUInt	ThresholdDeadband;
		HSUInt	Threshold;
		HSUInt	Gain;
		HSUInt	HDT;
		HSUInt	PDT;
		HSUInt	HLT;
		HSUInt	MaxDuration;
		HSUInt	WaveformLength;
		HSUInt	LowFilter;
		HSUInt	HighFilter;
		HSUInt	AmpGain;
		HSUInt	DrivenRate;
		HSUInt	RMS_ASL_Time;
		HSUInt	CounterThreshold;
	} CHANNEL_INFO;

	enum { MAX_AE_CHANNELS = 100, MAX_MSG_LEN = 32768 };

	
	CHANNEL_INFO mDefaultSeting;
	map< HSUInt, CHANNEL_INFO > mChannels;

private:
	HSVoid AddChannel( HSInt tChannel, CHANNEL_INFO tChanneInfo );
	HSVoid SetupPCI2Channel( HSInt tChannel );
	HSVoid HandleMessage();
	HSVoid InitBuffer();
	HSVoid SetupChannelBuffer( HSInt tChannel );

private:
	static HSBool mSuccessInit;

private:	
	CListBuffer *mListBuffer;	

	DEVICE_CH_IDENTIFIER mDataIdentifier;

	CIndexManager *mIndexManager;	

	enum { DATA_STREAM_THREAD_ID = 100, DATA_MESSAGE_THREAD_ID, FILE_THREAD_ID };
	CThreadControlEx< CPAPCI2Handler > *mDataStreamThread;
	CThreadControlEx< CPAPCI2Handler > *mDataMessageThread;
	CThreadControlEx< CPAPCI2Handler > *mFileThread;

	HANDLE mExitEvent;
	HANDLE mStartStreamEvent;
	HANDLE mStopStreamEvent;	

	HSByte mMessage[ MAX_MSG_LEN ];

	HANDLE mWriteDataMutex;

	HSInt mSampleInOneFrame;

	HSUInt64 mWaveStreamTime;
};

