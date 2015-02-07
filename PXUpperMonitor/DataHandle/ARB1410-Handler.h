#pragma once

#include "DataTypes.h"
#include "DeviceTypes.h"
#include "DataHandler.h"
#include "ListBuffer.h"
#include "ThreadControlEx.h"
#include "FileObserver.h"
#include "IndexManager.h"
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

	virtual HSInt64 Save(){ return 0; }

public:
	typedef struct FREQUENCY_MOD
	{
		HSInt StartFrequency;
		HSInt EndFrequency;
		HSInt Duration;
		HSInt MinPtsPerCycle;
	} FREQUENCY_MOD;

	typedef struct SET_SIGNAL
	{
		HSInt Coupling;
		HSInt Amplitude;
		HSInt DCoffset;
		HSInt Filter;
		HSInt Delay;
	} SET_SIGNAL;

	typedef struct SET_TRIGGER
	{
		HSInt Mode;
		HSInt RepRate;
		HSInt OutPolarity;
		HSInt OutWidth;
		HSInt Intype;
		HSInt InSlope;
	} SET_TRIGGER;	

public:
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

	static HSInt GetDevice( vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver );
	static HSBool GetDevice( DEVICE_CH_IDENTIFIER tIdentifier, HSUInt tVersion, HSInt64 tDeviceSavePos, vector< HSInt > &tChannels, vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver );	

	HSBool EffectiveTest( HSChar *pBuffer );	

	HSVoid SetFrequencyMod( FREQUENCY_MOD tMod ){ mFrequcencyMod = tMod; }
	HSVoid SetSignal( SET_SIGNAL tSignal ){ mSignalSet = tSignal; }
	HSVoid SetTrigger( SET_TRIGGER tTrigger ){ mTriggerSet = tTrigger; }
	FREQUENCY_MOD * GetFrequcyModulate(){ return &mFrequcencyMod; }
	SET_SIGNAL * GetSignalSet(){ return &mSignalSet; }
	SET_TRIGGER * GetTriggerSet(){ return &mTriggerSet; }

	HSBool EffectiveSettings();

private:	
	CListBuffer *mListBuffer;

	DEVICE_CH_IDENTIFIER mDataIdentifier;

	CIndexManager *mIndexManager;	

	enum { DATA_STREAM_THREAD_ID = 100, FILE_THREAD_ID };
	CThreadControlEx< CARB1410Handler > *mDataStreamThread;	
	CThreadControlEx< CARB1410Handler > *mFileThread;	
	
	HSInt mSampleInOneFrame;	

	HSInt mSampleRate;

	HSBool mIsOn;

	FREQUENCY_MOD mFrequcencyMod;
	SET_SIGNAL mSignalSet;
	SET_TRIGGER mTriggerSet;
};

