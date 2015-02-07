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

	} ChannelInfo;

public:
	CDeviceInfo();
	~CDeviceInfo();

	HSBool Load( CFileObserver *pFile, HSUInt tDeviceCount );
	HSVoid AddSamplate( DEVICE_CH_IDENTIFIER tDevice, HSUInt8 tChannel, HSUInt tSampleRate );
	HSBool Save( CFileObserver *pFile );
	DEVICE_CH_IDENTIFIER DeviceWithIndex( HSInt tIndex );
	HSUInt DeviceCount();
	HSUInt ChannelCountInDevice( DEVICE_CH_IDENTIFIER tDevice );
	ChannelInfo ChannelInfoWithIndex( DEVICE_CH_IDENTIFIER tDevice, HSUInt tIndex );
	HSVoid Reset();

	HSInt64 FilePos();

private:

	typedef vector< ChannelInfo > ChannelVector;

	map< DEVICE_CH_IDENTIFIER, ChannelVector > mDeviceInfo;

	HSInt64 mFilePos;
};

