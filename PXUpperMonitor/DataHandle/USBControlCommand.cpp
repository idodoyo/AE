#include "stdafx.h"
#include "USBControlCommand.h"
#include "HSLogProtocol.h"

CUSBControlCommand * CUSBControlCommand::SharedInstance()
{
	static CUSBControlCommand sUSBControlCommand;
	return &sUSBControlCommand;
}

CUSBControlCommand::CUSBControlCommand()
{
	USBControlArg tChannelOneOpenControlArgs[] = { { TGT_DEVICE, REQ_VENDOR, DIR_TO_DEVICE, 0xB3, 0x14, 0x0A },
												  // { TGT_DEVICE, REQ_VENDOR, DIR_TO_DEVICE, 0xB3, 0xFFFF, 0x03 },
												   { TGT_DEVICE, REQ_VENDOR, DIR_TO_DEVICE, 0xB3, 0x01, 0x08 } };

	USBControlArg tChannelOneCloseControlArg = { TGT_DEVICE, REQ_VENDOR, DIR_TO_DEVICE, 0xB3, 0x00, 0x08 };


	USBControlArg tChannelTueOpenControlArgs[] = { { TGT_DEVICE, REQ_VENDOR, DIR_TO_DEVICE, 0xB3, 0x14, 0x0B },
												  // { TGT_DEVICE, REQ_VENDOR, DIR_TO_DEVICE, 0xB3, 0xFFFF, 0x02 },
												   { TGT_DEVICE, REQ_VENDOR, DIR_TO_DEVICE, 0xB3, 0x01, 0x09 } };

	USBControlArg tChannelTueCloseControlArg = { TGT_DEVICE, REQ_VENDOR, DIR_TO_DEVICE, 0xB3, 0x00, 0x09 };

	for ( HSInt i = 0; i < sizeof( tChannelOneOpenControlArgs ) / sizeof( USBControlArg ); i++ )
	{
		mOpenChannelFlow[ 0 ].push_back( tChannelOneOpenControlArgs[ i ] );
		mOpenChannelFlow[ 1 ].push_back( tChannelTueOpenControlArgs[ i ] );
	}
	
	mCloseChannelFlow[ 0 ].push_back( tChannelOneCloseControlArg );
	mCloseChannelFlow[ 1 ].push_back( tChannelTueCloseControlArg );	
}

HSBool CUSBControlCommand::OpenChannel( CCyControlEndPoint *pControlEndPoint, HSInt tChannel, HSInt tSampleRate )
{
	if ( pControlEndPoint == NULL )
	{
		return HSFalse;
	}

	( mOpenChannelFlow[ tChannel ] )[ 0 ].Value = 40000000 / tSampleRate;	

	for ( HSUInt i = 0; i < mOpenChannelFlow[ tChannel ].size(); i++ )
	{
		if ( !ControlPointExecuteCommand( pControlEndPoint, ( mOpenChannelFlow[ tChannel ] )[ i ] ) )
		{
			return HSFalse;
		}
	}

	return HSTrue;
}

HSBool CUSBControlCommand::CloseChannel( CCyControlEndPoint *pControlEndPoint, HSInt tChannel )
{
	if ( pControlEndPoint == NULL )
	{
		return HSFalse;
	}

	for ( HSUInt i = 0; i < mCloseChannelFlow[ tChannel ].size(); i++ )
	{
		if ( !ControlPointExecuteCommand( pControlEndPoint, ( mCloseChannelFlow[ tChannel ] )[ i ] ) )
		{
			return HSFalse;
		}
	}

	return HSTrue;
}

HSBool CUSBControlCommand::ControlPointExecuteCommand( CCyControlEndPoint *pControlEndPoint, USBControlArg &tControlArg )
{	
	HSUChar tBuffer[ 16 ]; 
	memset( tBuffer, 0, sizeof tBuffer );
	LONG tLength = sizeof tBuffer; 
	pControlEndPoint->Target = tControlArg.Target; 
	pControlEndPoint->ReqType = tControlArg.ReqType; 
	pControlEndPoint->Direction = tControlArg.Direction; 
	pControlEndPoint->ReqCode = tControlArg.ReqCode; 
	pControlEndPoint->Value = tControlArg.Value; 
	pControlEndPoint->Index = tControlArg.Index; 

	if ( !pControlEndPoint->XferData( tBuffer, tLength ) )
	{
		return HSFalse;
	}

	return HSTrue;
}


