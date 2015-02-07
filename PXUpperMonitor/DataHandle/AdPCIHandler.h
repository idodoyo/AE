#pragma once

#include "../PCIHeaders/Driver.h"
#include "ThreadControlEx.h"
#include "DataHandler.h"
#include "ListBuffer.h"
#include "IndexManager.h"
#include "DeviceTypes.h"
#include "FileObserver.h"
#include "ArgTableManager.h"
#include "ArgHandler.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

class CAdPCIHandler : public CArgHandler
{
public:
	CAdPCIHandler( CIndexManager *pIndexManager, CFileObserver *pFileObserver );
	virtual ~CAdPCIHandler();			
	
	HSVoid SetParam( HSLong tDeviceNum = -1, HSInt tSampleRate = -1, HSFloat tTrigerVoltage = -1 );

	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual CListBuffer * ListBuffer();

	virtual HSUInt SampleRate( DEVICE_CH_IDENTIFIER tChannelIdentifier ){ return mSampleRate; }

	virtual DEVICE_TYPE Type(){ return DEVICE_PCI_CARD; }	
	virtual HSUInt EachSampleSize(){ return sizeof( HSUShort ); }

	virtual HSDouble VoltageWithPointer( HSChar *pPointer ){ return ( ( ( *( HSUShort* )pPointer ) & 0xFFF ) - 2048 ) / 2048.0 * 5.0; }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier( DEVICE_CH_IDENTIFIER tChannelIdentifier = 0 ){ return mIdentifier; }

	virtual HSUInt SamplesInFrame( DEVICE_CH_IDENTIFIER tChannelIdentifier );
	virtual HSUInt GetChannelData( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSChar *pSrcBuf, HSChar *pDestBuf, HSInt tOffset, HSInt tSamples );

	virtual HSBool IsOn(){ return mNumChannel > 0; }	

	virtual HSInt64 Save(){ return 0; }

	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

	static HSInt GetDevice( vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver );
	static HSBool GetDevice( DEVICE_CH_IDENTIFIER tIdentifier, HSUInt tVersion, HSInt64 tDeviceSavePos, vector< HSInt > &tChannels, vector< IDataHandler * > *pDevices, CIndexManager *pIndexManager, CFileObserver *pFileObserver );

private:
	HSVoid BufChangeEvent();
	HSVoid OverrunEvent();
	HSVoid TerminateEvent();

	HSBool EffectiveTest( HSChar *pBuffer );	

protected:
	HSLong mDeviceHandle;

	HSChar *mUserBuffer;
	HSInt mSampleCount;

	HSFloat mTrigerVoltage;
	HSInt mStartChannel;
	HSInt mNumChannel;
	HSInt mSampleRate;

	HSULong mDeviceNum;

	HSUShort mRecorderGainCode[ 64 ];

	DEVICE_CH_IDENTIFIER mIdentifier;

	enum { DATA_THREAD_ID, FILE_THREAD_ID };
	CThreadControlEx< CAdPCIHandler > *mDataThread;
	CThreadControlEx< CAdPCIHandler > *mFileThread;

	CListBuffer *mListBuffer;
	CIndexManager *mIndexManager;	
};

