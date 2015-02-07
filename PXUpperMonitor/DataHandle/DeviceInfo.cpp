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
	while ( tDeviceIndex < tDeviceCount && pFile->ReadData( ( HSChar * )&tDevice, sizeof tDevice ) && pFile->ReadData( ( HSChar * )&tChannelCount, 1 ) )
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

			AddSamplate( tDevice, tChannelInfo.Channel, tChannelInfo.SampleRate );
		}

		tDeviceIndex++;
	}	

	if ( tDeviceIndex < tDeviceCount )
	{
		mDeviceInfo.clear();
		return HSFalse;
	}

	return HSTrue;
}

HSVoid CDeviceInfo::AddSamplate( DEVICE_CH_IDENTIFIER tDevice, HSUInt8 tChannel, HSUInt tSampleRate )
{
	ChannelInfo tChannelInfo = { tChannel, tSampleRate };
	mDeviceInfo[ tDevice ].push_back( tChannelInfo );
}

HSBool CDeviceInfo::Save( CFileObserver *pFile )
{
	mFilePos = pFile->FilePosition();

	map< DEVICE_CH_IDENTIFIER, ChannelVector >::iterator pIterator = mDeviceInfo.begin();
	while ( pIterator != mDeviceInfo.end() )
	{			
		DEVICE_CH_IDENTIFIER tDevice = pIterator->first;
		HSUInt8 tChannelCount = pIterator->second.size();
		pFile->Update( ( HSChar * )&tDevice, sizeof tDevice );
		pFile->Update( ( HSChar * )&tChannelCount, 1 );

		ChannelVector *pItems = &( *pIterator ).second;		
		ChannelVector::iterator pItemIterator = pItems->begin();
		while ( pItemIterator != pItems->end() )
		{
			pFile->Update( ( HSChar * )&( pItemIterator->Channel ), 1 );
			pFile->Update( ( HSChar * )&( pItemIterator->SampleRate ), 4 );

			pItemIterator++;
		}	

		pIterator++;
	}

	return HSTrue;
}

DEVICE_CH_IDENTIFIER CDeviceInfo::DeviceWithIndex( HSInt tIndex )
{
	map< DEVICE_CH_IDENTIFIER, ChannelVector >::iterator pIterator = mDeviceInfo.begin();
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
	return mDeviceInfo[ tDevice ].size();
}

CDeviceInfo::ChannelInfo CDeviceInfo::ChannelInfoWithIndex( DEVICE_CH_IDENTIFIER tDevice, HSUInt tIndex )
{
	return ( mDeviceInfo[ tDevice ] )[ tIndex ];
}

HSInt64 CDeviceInfo::FilePos()
{
	return mFilePos;
}