#pragma once

#include "DataTypes.h"

class IWaveGenProtocol
{
public:
	virtual HSInt Open() = 0;
	virtual HSVoid Close() = 0;
	virtual HSVoid SetWaveformType( HSInt tType ) = 0;
	virtual HSVoid SetAMwaveformControls( HSInt tFrequency, HSInt tCarrierType, HSInt tEnvelope, HSInt tDurationUnits, HSInt tDuration ) = 0;
	virtual HSVoid SetFrequencySweepControls( HSInt tStartFrequency, HSInt tEndFrequency, HSInt tDuration, HSInt tMinPtsPerCycle ) = 0;
	virtual HSVoid SetAEwaveformControls( HSInt tFrequency, HSInt tRiseTime, HSInt tFallTime ) = 0;
	virtual HSVoid SetArbitraryWaveformControls( HSInt tA, HSShort *pB, HSInt tC ) = 0;
	virtual HSVoid SetSignalControls( HSInt tCoupling, HSInt tAmplitude, HSInt tDCoffset, HSInt tFilter, HSInt tDelay ) = 0;
	virtual HSVoid SetHighVoltageControls( HSInt tA, HSInt tB, HSInt tC ) = 0;
	virtual HSVoid SetTriggerControls( HSInt tMode, HSInt tRepRate, HSInt tOutPolarity, HSInt tOutWidth, HSInt tInType, HSInt tInSlope ) = 0;
	virtual HSVoid TriggerSingleShot() = 0;
	virtual HSVoid SetOutputMux( HSInt tA, HSInt tB, HSInt tC, HSInt tD ) = 0;
	virtual HSVoid ActivateControlSettings() = 0;
	virtual HSVoid SetOutputEnable( HSInt tEnable ) = 0;	

	virtual ~IWaveGenProtocol(){}

public:
	static IWaveGenProtocol * InstanceWithAddress( FARPROC pAddress )
	{
		typedef IWaveGenProtocol * ( *GetHSInstance )();
		if ( pAddress == NULL )
		{
			return NULL;
		}

		GetHSInstance fGetInstance = ( GetHSInstance )pAddress;
		return fGetInstance();
	}	
};