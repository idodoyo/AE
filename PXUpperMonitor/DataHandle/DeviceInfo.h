#pragma once

#include <vector>
#include <map>
#include "FileObserver.h"
#include "DataHandler.h"

using std::vector;
using std::map;

class CDeviceInfo
{
public:
	typedef struct ChannelInfo
	{
		HSUInt8 Channel;
		HSUInt SampleRate;
		HSInt8 Switch;
	} ChannelInfo;

	typedef struct DeviceInfo
	{
		HSInt64 DeviceInfoPos;
		vector< ChannelInfo > ChannelVector;
	} DeviceInfo;

public:
	CDeviceInfo();
	~CDeviceInfo();

	HSBool Load( CFileObserver *pFile, HSUInt tDeviceCount );
	HSVoid AddChannel( DEVICE_CH_IDENTIFIER tDevice, HSUInt8 tChannel, HSUInt tSampleRate, HSBool tOn );
	HSBool Save( CFileObserver *pFile );
	DEVICE_CH_IDENTIFIER DeviceWithIndex( HSInt tIndex );
	HSUInt DeviceCount();
	HSUInt ChannelCountInDevice( DEVICE_CH_IDENTIFIER tDevice );
	ChannelInfo ChannelInfoWithIndex( DEVICE_CH_IDENTIFIER tDevice, HSUInt tIndex );
	HSInt64 DeviceSavePos( DEVICE_CH_IDENTIFIER tDevice );
	HSVoid SetDeviceSavePos( DEVICE_CH_IDENTIFIER tDevice, HSInt64 tDeviceSavePos );
	HSVoid Reset();

	HSInt64 FilePos();

private:	

	map< DEVICE_CH_IDENTIFIER, DeviceInfo > mDeviceInfo;

	HSInt64 mFilePos;
};

