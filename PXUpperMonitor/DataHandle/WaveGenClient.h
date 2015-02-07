#pragma once

#include "DataTypes.h"
#include "DLLLoader.h"
#include "HSNetStreamProtocol.h"

class CWaveGenClient
{
public:
	CWaveGenClient();
	~CWaveGenClient();

	HSBool StartServer();
	HSBool Init();
	HSVoid Close();

public:
	static CWaveGenClient * SharedInstance();

public:
	HSInt OpenAWG();
	HSVoid CloseAWG();
	HSVoid SetWaveformType( HSInt tType );
	HSVoid SetAMwaveformControls( HSInt tFrequency, HSInt tCarrierType, HSInt tEnvelope, HSInt tDurationUnits, HSInt tDuration );
	HSVoid SetFrequencySweepControls( HSInt tStartFrequency, HSInt tEndFrequency, HSInt tDuration, HSInt tMinPtsPerCycle );
	HSVoid SetAEwaveformControls( HSInt tFrequency, HSInt tRiseTime, HSInt tFallTime );
	HSVoid SetArbitraryWaveformControls( HSInt tA, HSShort *pB, HSInt tC );
	HSVoid SetSignalControls( HSInt tCoupling, HSInt tAmplitude, HSInt tDCoffset, HSInt tFilter, HSInt tDelay );
	HSVoid SetHighVoltageControls( HSInt tA, HSInt tB, HSInt tC );
	HSVoid SetTriggerControls( HSInt tMode, HSInt tRepRate, HSInt tOutPolarity, HSInt tOutWidth, HSInt tInType, HSInt tInSlope );
	HSVoid TriggerSingleShot();
	HSVoid SetOutputMux( HSInt tA, HSInt tB, HSInt tC, HSInt tD );
	HSVoid ActivateControlSettings();
	HSVoid SetOutputEnable( HSInt tEnable );

private:
	HSInt ExecuteApi( HSString tCommand );
	HSString StringWithCommand( HSString tCommand, HSInt *pArg1 = NULL, HSInt *pArg2 = NULL, HSInt *pArg3 = NULL, HSInt *pArg4 = NULL, HSInt *pArg5 = NULL, HSInt *pArg6 = NULL );

private:
	CDllLoader mDllNetStream;
	IHSNetStreamProtocol *mNetStream;
	HSBool mIsServerOn;
};
