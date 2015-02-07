#pragma once

#include "DataTypes.h"


class CArgTableManager
{
public:
	typedef struct ArgItem
	{ 
		HSUInt64 NBeginTime;
		HSUInt64 IncreaseNTime;
		HSUInt64 NDuration;
		HSDouble Energy;
		HSUInt   RingCount;
		HSDouble Amplitude;
		HSDouble HRMS;
		HSDouble HASL;
	} ArgItem;

public:
	CArgTableManager();
	virtual ~CArgTableManager();

	HSVoid AddArg( ArgItem &tItem );
	HSDouble GetArg( HSUInt64 tNBeginTime, HSUInt64 tNDuration, vector< ArgItem > &tArgList );

	HSVoid Reset(){ mArgList.clear(); }

private:
	list< ArgItem > mArgList;
};