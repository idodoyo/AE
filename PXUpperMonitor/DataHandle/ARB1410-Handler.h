#pragma once

#include "DataTypes.h"
#include "DeviceTypes.h"
#include "DataHandler.h"
#include "ListBuffer.h"
#include "ThreadControlEx.h"
#include "FileObserver.h"
#include "IndexManager.h"
#include "Arb1410-Headers\awgapi.h"
#include "ArgHandler.h"

class CARB1410Handler : public CArgHandler
{	
public:
	CARB1410Handler(  CIndexManager *pIndexManager, CFileObserver *pFileObserver, HSInt tIndex );
	~CARB1410Handler();		

public:	
	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();
	   

	virtual HSString Name();
	virtual DEVICE_CH_IDENTIFIER Identifier(){ return DEVICE_CH_IDENTIFIER( CARD_ARB1410, mDataIdentifier.CARD_INDEX ); }
	virtual HSUInt SampleRate( DEVICE_CH_IDENTIFIER tChannelIdentifier ){ return mSampleRate; }
	virtual HSUInt EachSampleSize(){ return sizeof( HSFloat ); }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier( DEVICE_CH_IDENTIFIER tChannelIdentifier = 0 ){ return tChannelIdentifier; }
	virtual HSUInt SamplesInFrame( DEVICE_CH_IDENTIFIER tChannelIdentifier ){ return mSampleInOneFrame; }
	virtual HSUInt GetChannelData( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSChar *pSrcBuf, HSChar *pDestBuf, HSInt tOffset, HSInt tSamples );

	virtual HSDouble VoltageWithPointer( HSChar *pPointer ){ return *( HSFloat* )pPointer * 10.0 / 32768.0; }	
	
	virtual HSInt ChannelNum(){ return 1; }
	virtual DEVICE_TYPE Type(){ return DEVICE_PCI_CARD; }

	virtual HSBool OpenChannel( vector< HSInt > &tChannels, vector< HSInt > *pSampleRates = NULL, HSString *pErrorMsg = NULL );
	virtual HSBool CloseAllChannel();

	virtual CListBuffer * ListBuffer(){ return mListBuffer; }	

	virtual HSBool IsOn();

public:
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

	static HSInt GetDevice( vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver );
	static HSBool GetDevice( DEVICE_CH_IDENTIFIER tIdentifier, vector< HSInt > &tChannels, vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver );	

	HSBool EffectiveTest( HSChar *pBuffer );	
	

private:	
	CListBuffer *mListBuffer;

	DEVICE_CH_IDENTIFIER mDataIdentifier;

	CIndexManager *mIndexManager;

	typedef struct AMWAVEFORM_T
	{
		HSInt frequency;
		HSInt carrierType;
		HSInt envelope;
		HSInt durationUnits;
		HSInt duration;
	} AMWAVEFORM ;

	typedef struct FREQUENCYSWEEP_T
	{
		HSInt startFrequency;
		HSInt endFrequency;
		HSInt duration;
		HSInt minPtsPerCycle;
	} FREQUENCYSWEEP;
	
	typedef struct AEWAVEFORM_T
	{
		HSInt frequency;
		HSInt riseTime;
		HSInt fallTime;
	} AEWAVEFORM;

	typedef struct SIGNAL_T
	{
		HSInt coupling;
		HSInt amplitude;
		HSInt DCoffset;
		HSInt filter;
		HSInt delay;
	} SIGNAL;
	
	typedef struct TRIGGER_T
	{
		HSInt mode;
		HSInt repRate;
		HSInt outPolarity;
		HSInt outWidth;
		HSInt inType;
		HSInt inSlope;
	} TRIGGER;
	
	typedef struct  ARB1410_SET 
	{
		AMWAVEFORM  tAMwaveform;
		FREQUENCYSWEEP tFrequencySweep;
		AEWAVEFORM  tAEwaveform;
		SIGNAL tSignal;
		TRIGGER tTrigger;
		HSInt WaveformType;
	} ARB1410_SETINFO;

	enum { DATA_STREAM_THREAD_ID = 100, FILE_THREAD_ID };
	CThreadControlEx< CARB1410Handler > *mDataStreamThread;	
	CThreadControlEx< CARB1410Handler > *mFileThread;	
	
	HSInt mSampleInOneFrame;	

	HSInt mSampleRate;

	HSBool mIsOn;
public:
	ARB1410_SETINFO mARB1410_SETINFO;
};

