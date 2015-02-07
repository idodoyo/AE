#pragma once

#include "DataTypes.h"
#include "DataHandler.h"
#include "ArgTableManager.h"


class CArgHandler : public IDataHandler
{	
public:	
	CArgHandler();
	virtual ~CArgHandler();

	virtual HSDouble BypassVoltage( HSUInt tChannel ){ return mBypassVoltages[ tChannel ]; }
	virtual HSBool GetArgList( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSUInt64 tNBeginTime, HSUInt64 tNDuration, vector< CArgTableManager::ArgItem > &tArgList, HSDouble &tLastSecond, HSDouble &tLastArgSecond );
	virtual HSVoid AddArg( DEVICE_CH_IDENTIFIER tChannelIdentifier, CArgTableManager::ArgItem *pItem, HSDouble tLastSecond );
	virtual HSUInt64 GetSampleNSecond( HSUInt tSampleReadIndex, HSUInt tSampleReadOffset, DEVICE_CH_IDENTIFIER tChannelIdentifier );

	virtual IArgFetcher * ArgFetcher(){ return NULL; }

	virtual HSBool DataIsInSegent(){ return HSFalse; }

	HSVoid SetBypassVoltage( HSUInt tChannel, HSDouble tVoltage );

protected:
	typedef struct ArgInfo
	{
		CArgTableManager Arg;		
		HSDouble LastSecond;
	} ArgInfo;

	map< HSUInt, HSDouble > mBypassVoltages;
	map< DEVICE_CH_IDENTIFIER, ArgInfo > mArgsInfo;
	HANDLE mArgMutex;
};

