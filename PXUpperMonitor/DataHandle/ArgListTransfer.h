#pragma once

#include "ArgTableManager.h"
#include "LinearTransfer.h"

class CArgListTransfer
{
public:
	CArgListTransfer( IDataHandler *pDataHandler, DEVICE_CH_IDENTIFIER tIdentifier );
	~CArgListTransfer();

	HSVoid GetArg( vector< CArgTableManager::ArgItem > &tArgList );	
	
	HSVoid Reset();	

private:
	IDataHandler *mDataHandler;
	DEVICE_CH_IDENTIFIER mIdentifier;		

	HSUInt64 mNSecondBegin;
	HSUInt64 mNSecondEnd;
	HSUInt64 mNDuration;	
};

