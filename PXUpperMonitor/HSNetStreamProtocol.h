#pragma once

#include "DataTypes.h"

class IHSNetStreamProtocol;

class IHSServerProtocol
{
public:
	virtual HSVoid NewClientConnected( IHSNetStreamProtocol *pClient ) = 0;
	virtual HSBool DataReceivedFromClient( IHSNetStreamProtocol *pClient, HSChar *pBuffer, HSInt tLength ) = 0;

	virtual ~IHSServerProtocol(){} 
};

class IHSNetStreamProtocol
{
public:
	virtual HSBool Init( HSCChar *pServerIP, HSInt tPort, IHSServerProtocol *pServer = NULL, HSInt tBufferLength = 16 * 1024 ) = 0;
	virtual HSBool Send( HSCChar *pBuffer, HSInt tLength ) = 0;
	virtual HSBool Recv( HSChar *pBuffer, HSInt &tLength ) = 0;
	virtual HSVoid Close() = 0;
	virtual HSUInt Identifier() = 0;
	virtual IHSNetStreamProtocol * CreateProto() = 0;
	virtual HSVoid GetAddress( HSChar *pIP, HSInt &tPort ) = 0;

	virtual ~IHSNetStreamProtocol(){}
};
