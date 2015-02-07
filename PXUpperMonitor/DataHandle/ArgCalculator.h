#pragma once

#include "LinearTransfer.h"
#include "ArgTableManager.h"
#include "ThreadControlEx.h"
#include "IndexManager.h"
#include "DeviceManager.h"
#include "DataHandler.h"

class CArgCalculator
{
public:
	static CArgCalculator * SharedInstance();
	enum MATERIAL_TYPE{ MATERIAL_KINE_ONE, MATERIAL_KINE_TWO, MATERIAL_KINE_THREE, MATERIAL_KINE_FOUR, MATERIAL_KINE_FIVE };

private:
	CArgCalculator();

public:
	~CArgCalculator();	
	
public:
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

	HSVoid SetIndexManager( CIndexManager *pIndexManager, CDeviceManager *pDeviceManager );

	HSVoid Start();
	HSVoid Reset();
	HSVoid AddChannel( DEVICE_CH_IDENTIFIER tIdentifier );

	HSDouble GetMaterialVelocity( MATERIAL_TYPE tType = MATERIAL_KINE_FOUR ){ return mCommonMaterialWaveArgs[ tType ].VELOCITY; }
	HSUInt GetHitFullTime(){ return /*mCommonMaterialWaveArgs[ MATERIAL_KINE_FOUR ].PDT +*/ mCommonMaterialWaveArgs[ MATERIAL_KINE_FOUR ].HDT + mCommonMaterialWaveArgs[ MATERIAL_KINE_FOUR ].HLT; }

private:
	typedef struct ArgHandleInfo
	{
		CLinearTransfer *DataTransfer;	
		IDataHandler *DataHandler;		
		IArgFetcher *ArgFetcher;
		CThreadControlEx< CArgCalculator > *ArgThread;
		HSUInt64 HLTEndNTime;
		HSChar *Buffer;		
		HSUInt SampleIndex;
		HSUInt SampleOffset;
		HSUInt64 SampleNSecond;
	} ArgHandleInfo;

	map< DEVICE_CH_IDENTIFIER, ArgHandleInfo > mArgHandles;	
	map< HSInt, DEVICE_CH_IDENTIFIER > mArgThreadsID;

	CIndexManager *mIndexManager;
	CDeviceManager *mDeviceManager;

	HANDLE mMutex;

	typedef struct WAVE_ITEM
	{
		HSUInt64 NTime;
		HSUInt64 NBeginTime;
		HSUInt64 NEndTime;
		HSDouble Amplitude;
		HSDouble Energy;
		HSUInt EndIndex;
		HSDouble RMS;
	}WAVE_ITEM; 

	HSBool CalculateArgs( HSInt tLength, HSDouble tLastSecond, DEVICE_CH_IDENTIFIER tIdentifier );
	HSInt GetWaveItem( HSInt tLength, vector< WAVE_ITEM > &tWaves, DEVICE_CH_IDENTIFIER tIdentifier );

	typedef struct COMMON_MATERIAL_DEF
	{
		HSUInt PDT;
		HSUInt HDT;
		HSUInt HLT;
		HSDouble VELOCITY;
	} COMMON_MATERIAL_DEF;	

	map< MATERIAL_TYPE, COMMON_MATERIAL_DEF > mCommonMaterialWaveArgs;	
};

