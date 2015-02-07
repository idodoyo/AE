#pragma once

#include "DataTypes.h"
#include "ArgTableManager.h"


class CArgDataFectcher
{
public:
	CArgDataFectcher( HSInt tArgIndex, HSBool tIsXAxis ) : mArgIndex( tArgIndex ), mIsXAxis( tIsXAxis ){}

	virtual ~CArgDataFectcher(){}

	HSVoid SetArg( HSInt tArgIndex ){ mArgIndex = tArgIndex; }
	HSVoid SetArg( HSString tArgName )
	{
		HSString *pArgs = NULL;
		HSInt tCount = GetArgs( pArgs );
		mArgIndex = ( mIsXAxis ? 0 : 2 );
		for ( HSInt i = 0; i < tCount; i++ )
		{
			if ( pArgs[ i ] == tArgName )
			{
				 mArgIndex = i; 
				 break;
			}
		}
	}

	HSDouble FetchArg( CArgTableManager::ArgItem &tArgItem, HSInt tIndex, HSInt tChannel )
	{		
		switch( mArgIndex )
		{
		case 0:
			return tChannel + 1;			

		case 1:
			return tArgItem.NBeginTime / 1000000.0;

		case 2:
			return mIsXAxis ? tIndex + 1 : 1;

		case 3:
			return tArgItem.IncreaseNTime / 1000.0;

		case 4:
			return tArgItem.NDuration / 1000.0;

		case 5:
			return tArgItem.Energy;

		case 6:
			return tArgItem.RingCount;

		case 7:
			return tArgItem.Amplitude;

		case 8:
			return tArgItem.HRMS;

		case 9:
			return tArgItem.HASL;

		default:
			return 0;

		}		
	}		

	static HSInt GetArgs( HSString * &pArgs )
	{ 
		//static HSString sArgs[] = { "通道", "时间", "撞击", "上升时间", "持续时间", "能量", "振铃计数", "振幅", "RMS", "ASL" };
		static HSString sArgs[] = { "通道", "时间", "撞击", "上升时间", "持续时间", "能量", "振铃计数", "振幅" };
		pArgs = sArgs;
		return sizeof( sArgs ) / sizeof( HSString );
	}

	HSString ArgUnit()
	{
		static HSString sArgsUnit[] = { "", "(ms)", "", "(us)", "(us)", "", "", "(dB)", "", "" };
		return sArgsUnit[ mArgIndex ];
	}

	HSVoid GetDesiredShowParam( HSDouble &tDuration, HSDouble &tLowValue )
	{
		static HSDouble sArgsDuration[] = { 5, 1000, 100, 1000, 3000, 1000, 100, 100, 1000, 100 };
		
		tDuration = sArgsDuration[ mArgIndex ];	

		tLowValue = 0.001;
	}

	HSBool IsValueAdded()
	{
		switch ( mArgIndex )
		{
		case 1:
		//case 3:
		//case 4:
			return HSTrue;

		default:
			return HSFalse;
		}		
	}
	
	HSInt ArgIndex()
	{		
		return mArgIndex;
	}
	
	HSString ArgName()
	{
		HSString *pArgs = NULL;
		GetArgs( pArgs );

		return pArgs[ mArgIndex ];
	}	

private:
	HSInt mArgIndex;
	HSBool mIsXAxis;
};

