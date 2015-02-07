#pragma once

#include "DataTypes.h"
#include "CyAPI.h"

class CUSBControlCommand
{
private:
	CUSBControlCommand();

public:
	static CUSBControlCommand * SharedInstance();

	HSBool OpenChannel( CCyControlEndPoint *pControlEndPoint, HSInt tChannel, HSInt tSampleRate );
	HSBool CloseChannel( CCyControlEndPoint *pControlEndPoint, HSInt tChannel );

private:
	typedef struct USBControlArg
	{
		CTL_XFER_TGT_TYPE Target;
		CTL_XFER_REQ_TYPE ReqType;
		CTL_XFER_DIR_TYPE Direction;
		HSUChar ReqCode;
		HSUShort Value;
		HSUShort Index;
	} USBControlArg;

	HSBool ControlPointExecuteCommand( CCyControlEndPoint *pControlEndPoint, USBControlArg &tControlArg );

private:
	map< HSInt, vector< USBControlArg > > mOpenChannelFlow;
	map< HSInt, vector< USBControlArg > > mCloseChannelFlow;
};