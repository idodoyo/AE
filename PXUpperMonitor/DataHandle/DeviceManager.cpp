#include "StdAfx.h"
#include "DeviceManager.h"
#include "AdPCIHandler.h"
#include "HSLogProtocol.h"
#include "USBHandler.h"
#include "PAPCI-2Handler.h"
#include "ARB1410-Handler.h"
#include "UA306Handler.h"
#include "AdPCI1714Handler.h"
#include "PXUpperMonitor.h"

CDeviceManager::CDeviceManager( CFileObserver *pFileObserver, CIndexManager *pIndexManager, CWnd *pParentWnd )
	: mFileObserver( pFileObserver ), mIndexManager( pIndexManager ), mParentWnd( pParentWnd )
{
	mIsPaused = HSFalse;
	mIsStart = HSFalse;
}

CDeviceManager::~CDeviceManager()
{
	ClearAllDevices();

	CPAPCI2Handler::CloseDevice();
}

HSVoid CDeviceManager::ClearAllDevices()
{
	for ( HSUInt i = 0; i < mDevices.size(); i++ )
	{
		delete mDevices[ i ];
	}

	mDevices.clear();	
}

HSBool CDeviceManager::ScanDevices()
{
	ClearAllDevices();

	if ( theApp.License()->DeviceEnabled( CARD_PCI1714 ) )
	{
		CAdPCIHandler::GetDevice( &mDevices, mIndexManager, mFileObserver );
	}

	if ( theApp.License()->DeviceEnabled( CARD_EASYUSB ) )
	{
		CUSBHandler::GetDevice( &mDevices, mIndexManager, mFileObserver );
	}

	if ( theApp.License()->DeviceEnabled( CARD_PAPCI2 ) )
	{
		CPAPCI2Handler::GetDevice( &mDevices, mIndexManager, mFileObserver );
	}

	if ( theApp.License()->DeviceEnabled( CARD_ARB1410 ) )
	{
		CARB1410Handler::GetDevice( &mDevices, mIndexManager, mFileObserver );
	}

	if ( theApp.License()->DeviceEnabled( CARD_UA306 ) )
	{
		CUA306Handler::GetDevice( &mDevices, mIndexManager, mFileObserver );
	}

	if ( mDevices.size() > 0 )
	{
		return HSTrue;
	}

	return HSFalse;
}

HSBool CDeviceManager::OpenDevice( HSUInt tIndex, vector< HSInt > &tChannels )
{
	return mDevices[ tIndex ]->OpenChannel( tChannels );
}

HSBool CDeviceManager::CloseAllDevice()
{
	for ( HSUInt i = 0; i < mDevices.size(); i++ )
	{
		mDevices[ i ]->CloseAllChannel();
	}

	return HSTrue;
}

HSBool CDeviceManager::AddDevice( DEVICE_CH_IDENTIFIER tType, HSUInt tVersion, HSInt64 tDeviceSavePos, vector< HSInt > &tChannels, vector< HSInt > &tSampleRates )
{
	HSInt tLastDevice = - 1;
	if ( CAdPCIHandler::GetDevice( tType, tVersion, tDeviceSavePos, tChannels, &mDevices, mIndexManager, mFileObserver ) )
	{
		tLastDevice = mDevices.size() - 1;			
	}
	else if ( CUSBHandler::GetDevice( tType, tVersion, tDeviceSavePos, tChannels, &mDevices, mIndexManager, mFileObserver ) )
	{
		tLastDevice = mDevices.size() - 1;		
	}
	else if ( CPAPCI2Handler::GetDevice( tType, tVersion, tDeviceSavePos, tChannels, &mDevices, mIndexManager, mFileObserver ) )
	{
		tLastDevice = mDevices.size() - 1;		
	}
	else if ( CARB1410Handler::GetDevice( tType, tVersion, tDeviceSavePos, tChannels, &mDevices, mIndexManager, mFileObserver ) )
	{
		tLastDevice = mDevices.size() - 1;		
	}
	else if ( CUA306Handler::GetDevice( tType, tVersion, tDeviceSavePos, tChannels, &mDevices, mIndexManager, mFileObserver ) )
	{
		tLastDevice = mDevices.size() - 1;		
	}

	if ( tLastDevice >= 0 )
	{
		mDevices[ tLastDevice ]->OpenChannel( tChannels, &tSampleRates );		
		return HSTrue;
	}

	return HSFalse;
}

HSVoid CDeviceManager::Stop()
{
	for ( HSUInt i = 0; i < mDevices.size(); i++ )
	{
		if ( mDevices[ i ]->IsOn() )
		{
			mDevices[ i ]->Stop();
		}
	}

	mIsPaused = HSFalse;
	mIsStart = HSFalse;

	HS_INFO( "Stop Sample" );
}

HSBool CDeviceManager::Start()
{
	HSInt mSuccessCount = 0;
	for ( HSUInt i = 0; i < mDevices.size(); i++ )
	{
		if ( mDevices[ i ]->Start() )
		{
			mSuccessCount++;
		}
		else if ( mDevices[ i ]->IsOn() )
		{
			CString tStrMsg;
			tStrMsg.Format( "设备 %s 启动失败!", mDevices[ i ]->Name().c_str() );
			mParentWnd->MessageBox( tStrMsg, "警告" );
		}
	}

	if ( mSuccessCount < 1 )
	{
		return HSFalse;
	}

	mIsPaused = HSFalse;
	mIsStart = HSTrue;

	HS_INFO( "Start Sample, Device: %d", mSuccessCount );

	return HSTrue;
}

HSVoid CDeviceManager::Pause()
{
	for ( HSUInt i = 0; i < mDevices.size(); i++ )
	{
		if ( mDevices[ i ]->IsOn() )
		{
			mDevices[ i ]->Pause();
		}
	}

	mIsPaused = HSTrue;
	mIsStart = HSFalse;

	HS_INFO( "Pause Sample" );
}

IDataHandler * CDeviceManager::DataHandlerWithIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	DEVICE_CH_IDENTIFIER tCardIdentifier( tIdentifier.CARD_IDENTIFIER, tIdentifier.CARD_INDEX );
	for ( HSUInt i = 0; i < mDevices.size(); i++ )
	{
		if ( mDevices[ i ]->Identifier() == tCardIdentifier )
		{
			return mDevices[ i ];
		}
	}

	return NULL;
}

IDataHandler * CDeviceManager::DataHandlerWithIndex( HSUInt tIndex )
{
	if ( tIndex < mDevices.size() )
	{
		return mDevices[ tIndex ];
	}

	return NULL;	
}

vector< IDataHandler * > * CDeviceManager::Devices()
{
	return &mDevices;
}

HSBool CDeviceManager::IsPaused()
{
	return mIsPaused;
}

HSBool CDeviceManager::IsStart()
{
	return mIsStart;
}

HSVoid CDeviceManager::Load( CINIConfig *pIniConfig )
{
	CAdPCI1714Handler::Load( pIniConfig );
	CPAPCI2Handler::Load( pIniConfig );
	CUA306Handler::Load( pIniConfig );

}

HSVoid CDeviceManager::Save( CINIConfig *pIniConfig )
{
	CAdPCI1714Handler::Save( pIniConfig );
	CPAPCI2Handler::Save( pIniConfig );
	CUA306Handler::Save( pIniConfig );
}