#include "StdAfx.h"
#include "AdPCI1714Handler.h"
#include "HSLogProtocol.h"

/* keep default values */
map< HSUInt, HSDouble > CAdPCI1714Handler::mAd1714BypassVoltages;
HSInt CAdPCI1714Handler::mDefaultSampleRate = 1000000;

/*************************************************
  Function:		CAdPCI1714Handler	
  Description:	Constructor
  Input:		pIndexManager -- Use for save data	
				pFileObserver -- File use for CListBuffer;
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
CAdPCI1714Handler::CAdPCI1714Handler( CIndexManager *pIndexManager, CFileObserver *pFileObserver )
	: CAdPCIHandler( pIndexManager, pFileObserver )
{
	mIdentifier.CARD_IDENTIFIER = CARD_PCI1714;
	mIdentifier.CARD_INDEX = 0;	
	
	mBypassVoltages[ 0 ] = mAd1714BypassVoltages[ 0 ];
	mBypassVoltages[ 1 ] = mAd1714BypassVoltages[ 1 ];
	mBypassVoltages[ 2 ] = mAd1714BypassVoltages[ 2 ];
	mBypassVoltages[ 3 ] = mAd1714BypassVoltages[ 3 ];

	mSampleRate = mDefaultSampleRate;
}

/*************************************************
  Function:		~CAdPCI1714Handler	
  Description:	Destructor
  Input:		pIndexManager -- Use for save data	
				pFileObserver -- File use for CListBuffer;
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
CAdPCI1714Handler::~CAdPCI1714Handler()
{	
}

/*************************************************
  Function:		Name
  Description:	get the name of the card
  Input:		None
  Output:		None
  Return:		Result
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
HSString CAdPCI1714Handler::Name()
{
	HSChar tNameBuf[ 32 ];
	sprintf_s( tNameBuf, "PXDAQ12204-%02d", mDeviceNum + 1 );

	return tNameBuf;
}

/*************************************************
  Function:		OpenChannel
  Description:	Open channels, used for acquisition data, and save data.
  Input:		tChannels -- channels to open
				pSampleRates -- samplerates of each channel, used in review mode.				
  Output:		pErrorMsg -- errors when open channel, or "" no error.
  Return:		Result
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
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

/*************************************************
  Function:		CloseAllChannel
  Description:	Close all channels;
  Input:		None		
  Output:		None
  Return:		Result
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
HSBool CAdPCI1714Handler::CloseAllChannel()
{
	mIdentifier.CH_IDENTIFIER[ 0 ] = 0;
	mStartChannel = 0;
	mNumChannel = 0;

	return HSTrue;
}

/*************************************************
  Function:		SetBypassVoltage
  Description:	Inherit from CArgHander, used to keep default values when program exit;
  Input:		tChannel -- Channel to change threshhold.		
				tVoltage -- value to change
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
HSVoid CAdPCI1714Handler::SetBypassVoltage( HSUInt tChannel, HSDouble tVoltage )
{
	mDefaultSampleRate = mSampleRate;

	mAd1714BypassVoltages[ tChannel ] = tVoltage;
	CArgHandler::SetBypassVoltage( tChannel, tVoltage );	
}

/*************************************************
  Function:		Save
  Description:	Save default value to file.
  Input:		pIniConfig -- config file to save.
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
HSVoid CAdPCI1714Handler::Save( CINIConfig *pIniConfig )
{
	pIniConfig->SetValue( "CHANNEL1-ThreadHold", mAd1714BypassVoltages[ 0 ], "PXDAQ12204 Setting" );
	pIniConfig->SetValue( "CHANNEL2-ThreadHold", mAd1714BypassVoltages[ 1 ], "PXDAQ12204 Setting" );
	pIniConfig->SetValue( "CHANNEL3-ThreadHold", mAd1714BypassVoltages[ 2 ], "PXDAQ12204 Setting" );
	pIniConfig->SetValue( "CHANNEL4-ThreadHold", mAd1714BypassVoltages[ 3 ], "PXDAQ12204 Setting" );
	pIniConfig->SetValue( "SampleRate", mDefaultSampleRate, "PXDAQ12204 Setting" );
}

/*************************************************
  Function:		Load
  Description:	Load default value from file.
  Input:		pIniConfig -- config file to Load.
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
HSVoid CAdPCI1714Handler::Load( CINIConfig *pIniConfig )
{	
	mAd1714BypassVoltages[ 0 ] = 0.0501191;
	mAd1714BypassVoltages[ 1 ] = 0.0501191;
	mAd1714BypassVoltages[ 2 ] = 0.0501191;
	mAd1714BypassVoltages[ 3 ] = 0.0501191;	

	pIniConfig->ValueWithKey( "CHANNEL1-ThreadHold", mAd1714BypassVoltages[ 0 ], "PXDAQ12204 Setting" );
	pIniConfig->ValueWithKey( "CHANNEL2-ThreadHold", mAd1714BypassVoltages[ 1 ], "PXDAQ12204 Setting" );
	pIniConfig->ValueWithKey( "CHANNEL3-ThreadHold", mAd1714BypassVoltages[ 2 ], "PXDAQ12204 Setting" );
	pIniConfig->ValueWithKey( "CHANNEL4-ThreadHold", mAd1714BypassVoltages[ 3 ], "PXDAQ12204 Setting" );
	pIniConfig->ValueWithKey( "SampleRate", mDefaultSampleRate, "PXDAQ12204 Setting" );
}
