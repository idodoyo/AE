#pragma once

#include "DataTypes.h"
#include "DeviceTypes.h"
#include "DataHandler.h"
#include "..\PAPCI-2Headers\PACP2LV.H"
#include "..\PAPCI-2Headers\Pacpci2.h"
#include "LinearTransfer.h"

class CPAPCI2Handler;

class CPAPCI2ArgFetcher : public IArgFetcher
{	
public:
	CPAPCI2ArgFetcher( CPAPCI2Handler *pParent );
	~CPAPCI2ArgFetcher();

	virtual HSVoid FetchArg( DEVICE_CH_IDENTIFIER tChannelIdentifier, CLinearTransfer *pLinearTransfer, HSDouble tTime );
	virtual HSInt BufferSize();
	
private:
	HSInt mArgIndex;
	CPAPCI2Handler *mParent;	
};

