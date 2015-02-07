#include "stdafx.h"
#include "WaveGenClient.h"
#include "HSLogProtocol.h"
#include "PXUpperMonitor.h"
#include "DeviceTypes.h"

CWaveGenClient::CWaveGenClient() : mDllNetStream( "HSNetStream", "GetNetStreamInstance" )
{
	mIsServerOn = HSFalse;
	mNetStream = NULL;
}

CWaveGenClient::~CWaveGenClient()
{
}

HSBool CWaveGenClient::StartServer()
{	
	if ( !theApp.License()->DeviceEnabled( CARD_ARB1410 ) )
	{
		return HSFalse;
	}

	STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
  
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = FALSE;
    TCHAR cmdline[] = TEXT( "WaveGen\\PXWaveGenServer.exe" );
    BOOL bRet = ::CreateProcess( NULL, cmdline, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi );
    
    if( bRet )   
    {   
        ::CloseHandle( pi.hThread );
        ::CloseHandle( pi.hProcess );
    }   
    else  
    {  		
        HS_ERROR( "Server Start Error:%d", GetLastError() );  
		return HSFalse;
    }

	Sleep( 1 );

	mIsServerOn = HSTrue;
	return HSTrue;
}

HSBool CWaveGenClient::Init()
{
	if ( !mIsServerOn )
	{
		return HSFalse;
	}

	if ( !mDllNetStream.Init() )
	{
		HS_ERROR( "Dll Load Error!" );
		return HSFalse;
	}
	
	if ( !mDllNetStream.InstanceWithDLL( mNetStream ) || mNetStream == NULL || !mNetStream->Init( "127.0.0.1", 10169 ) )
	{
		HS_ERROR( "Connect Server Error!" );
		return HSFalse;
	}

	return HSTrue;
}

HSVoid CWaveGenClient::Close()
{
	if ( mIsServerOn && mNetStream != NULL )
	{
		mNetStream->Send( "QUIT", 5 );
		Sleep( 2 );
		mNetStream->Close();
		mNetStream = NULL;
		mIsServerOn = HSFalse;
	}
}

CWaveGenClient * CWaveGenClient::SharedInstance()
{
	static CWaveGenClient sWaveGenClient;
	return &sWaveGenClient;
}

HSInt CWaveGenClient::ExecuteApi( HSString tCommand )
{
	if ( mNetStream )
	{	
		if ( !mNetStream->Send( tCommand.c_str(), tCommand.length() ) )
		{
			return -1;
		}
		
		HSChar tBuf[ 32 ] = { 0 };
		HSInt tLength = 0;
		if ( !mNetStream->Recv( tBuf, tLength ) )
		{
			return -1;
		}

		//HS_INFO( "Command: %s, Res: %s", tCommand.c_str(), tBuf );

		return atoi( tBuf );
	}

	return -1;
}

HSString CWaveGenClient::StringWithCommand( HSString tCommand, HSInt *pArg1, HSInt *pArg2, HSInt *pArg3, HSInt *pArg4, HSInt *pArg5, HSInt *pArg6 )
{	
	ostringstream tStream;
    tStream << tCommand;
	
	HSInt *pArgs[] = { pArg1, pArg2, pArg3, pArg4, pArg5, pArg6 };
	for ( HSUInt i = 0; i < sizeof( pArgs ) / sizeof( HSInt * ); i++ )
	{
		if ( pArgs[ i ] == NULL )
		{
			break;
		}

		tStream << "@@" << *pArgs[ i ];
	}
    
	return tStream.str();
}

HSInt CWaveGenClient::OpenAWG()
{
	if ( !mIsServerOn )
	{
		return 1;
	}

	return ExecuteApi( StringWithCommand( "AWG_open" ) );
}

HSVoid CWaveGenClient::CloseAWG()
{
	ExecuteApi( StringWithCommand( "AWG_close" ) );
}

HSVoid CWaveGenClient::SetWaveformType( HSInt tType )
{
	ExecuteApi( StringWithCommand( "AWG_setWaveformType", &tType ) );
}

HSVoid CWaveGenClient::SetAMwaveformControls( HSInt tFrequency, HSInt tCarrierType, HSInt tEnvelope, HSInt tDurationUnits, HSInt tDuration )
{
	ExecuteApi( StringWithCommand( "AWG_setAMwaveformControls", &tFrequency, &tCarrierType, &tEnvelope, &tDurationUnits, &tDuration ) );
}

HSVoid CWaveGenClient::SetFrequencySweepControls( HSInt tStartFrequency, HSInt tEndFrequency, HSInt tDuration, HSInt tMinPtsPerCycle )
{
	ExecuteApi( StringWithCommand( "AWG_setFrequencySweepControls", &tStartFrequency, &tEndFrequency, &tDuration, &tMinPtsPerCycle ) );
}

HSVoid CWaveGenClient::SetAEwaveformControls( HSInt tFrequency, HSInt tRiseTime, HSInt tFallTime )
{
	ExecuteApi( StringWithCommand( "AWG_setAEwaveformControls", &tFrequency, &tRiseTime, &tFallTime ) );
}

HSVoid CWaveGenClient::SetArbitraryWaveformControls( HSInt tA, HSShort *pB, HSInt tC )
{
	HSInt tB = 0;
	ExecuteApi( StringWithCommand( "AWG_setArbitraryWaveformControls", &tA, &tB, &tC ) );
}

HSVoid CWaveGenClient::SetSignalControls( HSInt tCoupling, HSInt tAmplitude, HSInt tDCoffset, HSInt tFilter, HSInt tDelay )
{
	ExecuteApi( StringWithCommand( "AWG_setSignalControls", &tCoupling, &tAmplitude, &tDCoffset, &tFilter, &tDelay ) );
}

HSVoid CWaveGenClient::SetHighVoltageControls( HSInt tA, HSInt tB, HSInt tC )
{
	ExecuteApi( StringWithCommand( "AWG_setHighVoltageControls", &tA, &tB, &tC ) );
}

HSVoid CWaveGenClient::SetTriggerControls( HSInt tMode, HSInt tRepRate, HSInt tOutPolarity, HSInt tOutWidth, HSInt tInType, HSInt tInSlope )
{
	ExecuteApi( StringWithCommand( "AWG_setTriggerControls", &tMode, &tRepRate, &tOutPolarity, &tOutWidth, &tInType, &tInSlope ) );
}

HSVoid CWaveGenClient::TriggerSingleShot()
{
	ExecuteApi( StringWithCommand( "AWG_triggerSingleShot" ) );
}

HSVoid CWaveGenClient::SetOutputMux( HSInt tA, HSInt tB, HSInt tC, HSInt tD )
{
	ExecuteApi( StringWithCommand( "AWG_setOutputMux", &tA, &tB, &tC, &tD ) );
}

HSVoid CWaveGenClient::ActivateControlSettings()
{
	ExecuteApi( StringWithCommand( "AWG_activateControlSettings" ) );
}

HSVoid CWaveGenClient::SetOutputEnable( HSInt tEnable )
{
	ExecuteApi( StringWithCommand( "AWG_setOutputEnable", &tEnable ) );
}