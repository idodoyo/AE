#pragma once

#include "AdPCIHandler.h"
#include "INIConfig.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

class CAdPCI1714Handler : public CAdPCIHandler
{
public:
	CAdPCI1714Handler( CIndexManager *pIndexManager, CFileObserver *pFileObserver );
	virtual ~CAdPCI1714Handler();
	
	virtual HSString Name();
	virtual DEVICE_CH_IDENTIFIER Identifier(){ return DEVICE_CH_IDENTIFIER( CARD_PCI1714, ( HSUChar )mDeviceNum ); }

	virtual HSInt ChannelNum(){ return 4; }
	virtual HSBool OpenChannel( vector< HSInt > &tChannels, vector< HSInt > *pSampleRates = NULL, HSString *pErrorMsg = NULL );
	virtual HSBool CloseAllChannel();	

	static HSVoid Save( CINIConfig *pIniConfig );
	static HSVoid Load( CINIConfig *pIniConfig, string tGroup );

private:
	static map< HSUInt, HSDouble > mAd1714BypassVoltages;
	static HSBool mInitBypassVoltages;
};

