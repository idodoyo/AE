#pragma once

#include "DataHandler.h"
#include "ThreadControlEx.h"
#include "DeviceInfo.h"
#include "IndexManager.h"
#include "FileObserver.h"

#include <vector>

using std::vector;

class CDeviceManager
{
public:
	CDeviceManager( CFileObserver *pFileObserver, CIndexManager *pIndexManager, CWnd *pParentWnd );
	virtual ~CDeviceManager();

	HSBool ScanDevices();
	HSBool OpenDevice( HSUInt tIndex, vector< HSInt > &tChannels );	
	HSBool CloseAllDevice();
	HSBool AddDevice( DEVICE_CH_IDENTIFIER tType, vector< HSInt > &tChannels, vector< HSInt > &tSampleRates );
	HSBool Start();
	HSVoid Stop();
	HSVoid Pause();

	HSBool IsPaused();
	HSBool IsStart();	

	vector< IDataHandler * > * Devices();

	IDataHandler * DataHandlerWithIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );	
	IDataHandler * DataHandlerWithIndex( HSUInt tIndex );

	HSVoid ClearAllDevices();

private:
	vector< IDataHandler * > mDevices;	

	CFileObserver *mFileObserver;
	CIndexManager *mIndexManager;	

	HSBool mIsPaused;
	HSBool mIsStart;

	CWnd *mParentWnd;
};

