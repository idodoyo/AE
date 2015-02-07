#include "StdAfx.h"
#include "DeviceInfo.h"


CDeviceInfo::CDeviceInfo()
{
}

CDeviceInfo::~CDeviceInfo()
{
}

HSBool CDeviceInfo::Load( CFileObserver *pFile, HSUInt tDeviceCount )
{
	Reset();

	DEVICE_CH_IDENTIFIER tDevice;
	HSUInt8 tChannelCount; 
	HSUInt tDeviceIndex = 0;
	HSInt64 tDeviceSavePos = 0;
	while ( tDeviceIndex < tDeviceCount && pFile->ReadData( ( HSChar * )&tDevice, sizeof tDevice ) && pFile->ReadData( ( HSChar * )&tDeviceSavePos, 8 ) && pFile->ReadData( ( HSChar * )&tChannelCount, 1 ) )
	{
		for ( HSUInt8 i = 0; i < tChannelCount; i++ )
		{
			ChannelInfo tChannelInfo;
			if ( pFile->ReadData( ( HSChar * )&tChannelInfo.Channel, 1 ) == 0 )
			{
				break;
			}

			if ( pFile->ReadData( ( HSChar * )&tChannelInfo.SampleRate, 4 ) == 0 )
			{
				break;
			}

			if ( pFile->ReadData( ( HSChar * )&tChannelInfo.Switch, 1 ) == 0 )
			{
				break;
			}

			AddChannel( tDevice, tChannelInfo.Channel, tChannelInfo.SampleRate, tChannelInfo.Switch );
		}

		SetDeviceSavePos( tDevice, tDeviceSavePos );

		tDeviceIndex++;
	}	

	if ( tDeviceIndex < tDeviceCount )
	{
		mDeviceInfo.clear();
		return HSFalse;
	}

	return HSTrue;
}

HSVoid CDeviceInfo::AddChannel( DEVICE_CH_IDENTIFIER tDevice, HSUInt8 tChannel, HSUInt tSampleRate, HSBool tOn )
{
	ChannelInfo tChannelInfo = { tChannel, tSampleRate, tOn };
	mDeviceInfo[ tDevice ].ChannelVector.push_back( tChannelInfo );
}

HSVoid CDeviceInfo::SetDeviceSavePos( DEVICE_CH_IDENTIFIER tDevice, HSInt64 tDeviceSavePos )
{
	mDeviceInfo[ tDevice ].DeviceInfoPos = tDeviceSavePos;
}

HSBool CDeviceInfo::Save( CFileObserver *pFile )
{
	mFilePos = pFile->FilePosition();

	map< DEVICE_CH_IDENTIFIER, DeviceInfo >::iterator pIterator = mDeviceInfo.begin();
	while ( pIterator != mDeviceInfo.end() )
	{			
		DEVICE_CH_IDENTIFIER tDevice = pIterator->first;
		HSUInt8 tChannelCount = pIterator->second.ChannelVector.size();
		HSInt64 tDeviceSavePos = pIterator->second.DeviceInfoPos;
		pFile->Update( ( HSChar * )&tDevice, sizeof tDevice );
		pFile->Update( ( HSChar * )&tDeviceSavePos, sizeof tDeviceSavePos );
		pFile->Update( ( HSChar * )&tChannelCount, 1 );

		vector< ChannelInfo > *pItems = &( ( *pIterator ).second.ChannelVector );
		vector< ChannelInfo >::iterator pItemIterator = pItems->begin();
		while ( pItemIterator != pItems->end() )
		{
			pFile->Update( ( HSChar * )&( pItemIterator->Channel ), 1 );
			pFile->Update( ( HSChar * )&( pItemIterator->SampleRate ), 4 );
			pFile->Update( ( HSChar * )&( pItemIterator->Switch ), 1 );

			pItemIterator++;
		}	

		pIterator++;
	}

	return HSTrue;
}

DEVICE_CH_IDENTIFIER CDeviceInfo::DeviceWithIndex( HSInt tIndex )
{
	map< DEVICE_CH_IDENTIFIER, DeviceInfo >::iterator pIterator = mDeviceInfo.begin();
	while ( pIterator != mDeviceInfo.end() && tIndex > 0 )
	{
		pIterator++;
		tIndex--;
	}

	if ( pIterator == mDeviceInfo.end() )
	{
		return 0;
	}
	
	return pIterator->first;
}

HSUInt CDeviceInfo::DeviceCount()
{
	return mDeviceInfo.size();
}

HSVoid CDeviceInfo::Reset()
{
	mDeviceInfo.clear();
}

HSUInt CDeviceInfo::ChannelCountInDevice( DEVICE_CH_IDENTIFIER tDevice )
{
	return mDeviceInfo[ tDevice ].ChannelVector.size();
}

CDeviceInfo::ChannelInfo CDeviceInfo::ChannelInfoWithIndex( DEVICE_CH_IDENTIFIER tDevice, HSUInt tIndex )
{
	return ( mDeviceInfo[ tDevice ] ).ChannelVector[ tIndex ];
}

HSInt64 CDeviceInfo::DeviceSavePos( DEVICE_CH_IDENTIFIER tDevice )
{
	return mDeviceInfo[ tDevice ].DeviceInfoPos;
}

HSInt64 CDeviceInfo::FilePos()
{
	return mFilePos;
}