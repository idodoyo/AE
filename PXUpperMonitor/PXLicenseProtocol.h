#pragma once

#include "DataTypes.h"

class IPXLicenseProtocol
{
public:
	virtual HSBool Init() = 0;
	virtual HSBool DataExportEnabled() = 0;
	virtual HSBool ShowUnBuyItems() = 0;
	virtual HSBool IsNormalGraphic( HSCChar *pIdentifier ) = 0;
	virtual HSBool GraphicEnabled( HSCChar *pIdentifier ) = 0;
	virtual HSBool DeviceEnabled( HSUShort tDevice ) = 0;
	virtual HSBool TimeUp() = 0;
	virtual HSBool SetEndTime( HSInt64 tSecond ) = 0;
	virtual HSBool GetSerialNum( HSUChar *pBuf ) = 0;

	virtual ~IPXLicenseProtocol(){}
};