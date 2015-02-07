#pragma once

#include "UA306Headers\ua300.h"
#include "ThreadControlEx.h"
#include "DataHandler.h"
#include "ListBuffer.h"
#include "IndexManager.h"
#include "DeviceTypes.h"
#include "FileObserver.h"
#include "ArgTableManager.h"
#include "ArgHandler.h"
#include "PAPCI-2ArgFetcher.h"
#include "INIConfig.h"

class CUA306Handler : public CArgHandler
{
public:
	CUA306Handler( CIndexManager *pIndexManager, CFileObserver *pFileObserver, HSInt tIndex = 0 );
	virtual ~CUA306Handler();
	
	HSVoid SetParam( HSInt tSampleRate = -1 );

	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual HSString Name();
	virtual DEVICE_CH_IDENTIFIER Identifier(){ return DEVICE_CH_IDENTIFIER( CARD_UA306, ( HSUChar )mDeviceNum ); }
	virtual HSInt ChannelNum(){ return 8; }	
	virtual CListBuffer * ListBuffer(){ return mListBuffer; }

	virtual HSUInt SampleRate( DEVICE_CH_IDENTIFIER tChannelIdentifier ){ return mSampleRate; }

	virtual DEVICE_TYPE Type(){ return DEVICE_USB_CARD; }	
	virtual HSUInt EachSampleSize(){ return sizeof( HSShort ); }

	virtual HSDouble VoltageWithPointer( HSChar *pPointer ){ return *( HSShort* )pPointer * 10.0 / 32768.0; }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier( DEVICE_CH_IDENTIFIER tChannelIdentifier = 0 );

	virtual HSUInt SamplesInFrame( DEVICE_CH_IDENTIFIER tChannelIdentifier );
	virtual HSUInt GetChannelData( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSChar *pSrcBuf, HSChar *pDestBuf, HSInt tOffset, HSInt tSamples );

	virtual HSBool IsOn(){ return mNumChannel > 0; }

	virtual HSBool OpenChannel( vector< HSInt > &tChannels, vector< HSInt > *pSampleRates = NULL, HSString *pErrorMsg = NULL );
	virtual HSBool CloseAllChannel();

	virtual HSVoid SetBypassVoltage( HSUInt tChannel, HSDouble tVoltage );

	//virtual IArgFetcher * ArgFetcher(){ return new CPAPCI2ArgFetcher( this ); }

	virtual HSInt64 Save(){ return 0; }

	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

	static HSInt GetDevice( vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver );
	static HSBool GetDevice( DEVICE_CH_IDENTIFIER tIdentifier, HSUInt tVersion, HSInt64 tDeviceSavePos, vector< HSInt > &tChannels, vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver );

	static HSVoid Save( CINIConfig *pIniConfig );
	static HSVoid Load( CINIConfig *pIniConfig );

private:
	HSBool EffectiveTest( HSChar *pBuffer );	
	HSDouble GetSavedValue( HSInt tType, HSInt tIndex, HSInt &tSize );

protected:
	HSInt mSampleCount;	
	HSInt mStartChannel;
	HSInt mNumChannel;
	HSInt mSampleRate;

	HSULong mDeviceNum;

	HANDLE mHandle;

	DEVICE_CH_IDENTIFIER mIdentifier;

	enum { DATA_THREAD_ID, FILE_THREAD_ID };
	CThreadControlEx< CUA306Handler > *mDataThread;
	CThreadControlEx< CUA306Handler > *mFileThread;

	CListBuffer *mListBuffer;
	CIndexManager *mIndexManager;	


	HSUInt mCurVirtualHitIndex;
	HSInt mFrameCount;

	time_t mStartTime;

	static map< HSUInt, HSDouble > mUA306BypassVoltages;	
	static HSInt mDefaultSampleRate;
};

