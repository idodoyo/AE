#include "StdAfx.h"
#include "AdPCI1714Handler.h"
#include "HSLogProtocol.h"

map< HSUInt, HSDouble > CAdPCI1714Handler::mAd1714BypassVoltages;
HSBool CAdPCI1714Handler::mInitBypassVoltages = HSFalse;

CAdPCI1714Handler::CAdPCI1714Handler( CIndexManager *pIndexManager, CFileObserver *pFileObserver )
	: CAdPCIHandler( pIndexManager, pFileObserver )
{
	mIdentifier.CARD_IDENTIFIER = CARD_PCI1714;
	mIdentifier.CARD_INDEX = 0;	

	if ( !mInitBypassVoltages )
	{
		mAd1714BypassVoltages[ 0 ] = 0.1;
		mAd1714BypassVoltages[ 1 ] = 0.1;
		mAd1714BypassVoltages[ 2 ] = 0.1;
		mAd1714BypassVoltages[ 3 ] = 0.1;

		mInitBypassVoltages = HSTrue;
	}

	mBypassVoltages[ 0 ] = 0.0501191;
	mBypassVoltages[ 1 ] = 0.0501191;
	mBypassVoltages[ 2 ] = 0.0501191;
	mBypassVoltages[ 3 ] = 0.0501191;
}

CAdPCI1714Handler::~CAdPCI1714Handler()
{
}

HSString CAdPCI1714Handler::Name()
{
	HSChar tNameBuf[ 32 ];
	sprintf_s( tNameBuf, "PXDAQ12204-%02d", mDeviceNum + 1 );

	return tNameBuf;
}

HSBool CAdPCI1714Handler::OpenChannel( vector< HSInt > &tChannels, vector< HSInt > *pSampleRates, HSString *pErrorMsg )
{
	HSString tErrorInfo = "";	
	if ( tChannels.size() < 1 )
	{
		tErrorInfo = this->Name() + ", 请选择通道! ";		
	}
	else if ( tChannels[ tChannels.size() - 1 ] - tChannels[ 0 ] + 1 != tChannels.size() )
	{
		tErrorInfo = this->Name() + ", 通道必须以奇数号开始，以1个或偶数个连续通道结束! ";
	}
	else if ( tChannels[ 0 ] % 2 != 0 || tChannels.size() == 3 )
	{
		tErrorInfo = this->Name() + ", 通道必须以奇数号开始，以1个或偶数个连续通道结束! ";
	}

	if ( tErrorInfo != "" )
	{
		if ( pErrorMsg )
		{
			*pErrorMsg = tErrorInfo;
		}
		return HSFalse;
	}	
		
	for ( HSUInt i = 0; i < tChannels.size(); i++ )
	{
		mIdentifier.CH_IDENTIFIER[ 0 ] |= ( 1 << tChannels[ i ] );
	}

	mStartChannel = tChannels[ 0 ];
	mNumChannel = tChannels.size();
	
	if ( pSampleRates != NULL )
	{
		mSampleRate = ( *pSampleRates )[ 0 ];
	}

	return HSTrue;
}

HSBool CAdPCI1714Handler::CloseAllChannel()
{
	mIdentifier.CH_IDENTIFIER[ 0 ] = 0;
	mStartChannel = 0;
	mNumChannel = 0;

	return HSTrue;
}

HSVoid CAdPCI1714Handler::Save( CINIConfig *pIniConfig )
{
	pIniConfig->SetValue( "CHANNEL1-ThreadHold", mAd1714BypassVoltages[ 0 ], "PXDAQ12204 Setting" );
	pIniConfig->SetValue( "CHANNEL2-ThreadHold", mAd1714BypassVoltages[ 1 ], "PXDAQ12204 Setting" );
	pIniConfig->SetValue( "CHANNEL3-ThreadHold", mAd1714BypassVoltages[ 2 ], "PXDAQ12204 Setting" );
	pIniConfig->SetValue( "CHANNEL4-ThreadHold", mAd1714BypassVoltages[ 3 ], "PXDAQ12204 Setting" );
}

HSVoid CAdPCI1714Handler::Load( CINIConfig *pIniConfig, string tGroup )
{	
	pIniConfig->ValueWithKey( "CHANNEL1-ThreadHold", mAd1714BypassVoltages[ 0 ], tGroup );
	pIniConfig->ValueWithKey( "CHANNEL2-ThreadHold", mAd1714BypassVoltages[ 1 ], tGroup );
	pIniConfig->ValueWithKey( "CHANNEL3-ThreadHold", mAd1714BypassVoltages[ 2 ], tGroup );
	pIniConfig->ValueWithKey( "CHANNEL4-ThreadHold", mAd1714BypassVoltages[ 3 ], tGroup );	

	mInitBypassVoltages = HSTrue;
}
