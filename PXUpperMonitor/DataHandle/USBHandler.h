#pragma once

#include "DataTypes.h"
#include "DeviceTypes.h"
#include "DataHandler.h"
#include "CyAPI.h"
#include "ListBuffer.h"
#include "ThreadControlEx.h"
#include "FileObserver.h"
#include "IndexManager.h"
#include "ArgHandler.h"

class CUSBHandler : public CArgHandler
{	
public:
	CUSBHandler(  CIndexManager *pIndexManager, CFileObserver *pFileObserver, HSInt tIndex );
	~CUSBHandler();

	enum { EASY_USB_EACH_FRAME_SAMPLES = 4096 };

public:	
	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual HSString Name();
	virtual DEVICE_CH_IDENTIFIER Identifier(){ return DEVICE_CH_IDENTIFIER( CARD_EASYUSB, ( HSUChar )mDeviceIndex ); }
	virtual HSUInt SampleRate( DEVICE_CH_IDENTIFIER tChannelIdentifier ){ return mSampleRates[ tChannelIdentifier.ChannelIndex() ]; }	
	virtual HSUInt EachSampleSize(){ return sizeof( HSShort ); }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier( DEVICE_CH_IDENTIFIER tChannelIdentifier = 0 ){ return mDataIdentifier[ tChannelIdentifier.ChannelIndex() ]; }
	virtual HSUInt SamplesInFrame( DEVICE_CH_IDENTIFIER tChannelIdentifier ){ return EASY_USB_EACH_FRAME_SAMPLES - 2; }
	virtual HSUInt GetChannelData( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSChar *pSrcBuf, HSChar *pDestBuf, HSInt tOffset, HSInt tSamples );

	virtual HSDouble VoltageWithPointer( HSChar *pPointer ){ return ( ( HSShort )( ( HSUChar )pPointer[ 1 ] << 8 | ( HSUChar )pPointer[ 0 ] ) ) / 32767.0 * 2.0; }
	
	virtual HSInt ChannelNum(){ return 2; }
	virtual DEVICE_TYPE Type(){ return DEVICE_USB_CARD; }

	virtual HSBool OpenChannel( vector< HSInt > &tChannels, vector< HSInt > *pSampleRates = NULL, HSString *pErrorMsg = NULL );
	virtual HSBool CloseAllChannel();

	virtual CListBuffer * ListBuffer(){ return mListBuffer; }

	virtual HSBool IsOn(){ return mChannelOn[ 0 ] || mChannelOn[ 1 ]; }

	//virtual HSUInt64 GetSampleNSecond( HSUInt tSampleReadIndex, HSUInt tSampleReadOffset, DEVICE_CH_IDENTIFIER tChannelIdentifier );

public:
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

	static HSInt GetDevice( vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver );
	static HSBool GetDevice( DEVICE_CH_IDENTIFIER tIdentifier, vector< HSInt > &tChannels, vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver );

	HSBool EffectiveTest( HSChar *pBuffer );
	HSVoid SetSampleRate( HSInt tChannel, HSInt tSampleRate ){ mSampleRates[ tChannel ] = tSampleRate; }

	HSVoid Test( HSInt tFirst, HSInt tSecond );

private:
	enum { DATA_THREAD_ID, FILE_THREAD_ID };
	CThreadControlEx< CUSBHandler > *mDataThread;
	CThreadControlEx< CUSBHandler > *mFileThread;

	CCyUSBDevice *mUSBDevice;

	CListBuffer *mListBuffer;
	HSInt mDeviceIndex;

	HSBool mChannelOn[ 2 ];
	HSInt mSampleRates[ 2 ];

	DEVICE_CH_IDENTIFIER mDataIdentifier[ 2 ];

	CIndexManager *mIndexManager;
};

