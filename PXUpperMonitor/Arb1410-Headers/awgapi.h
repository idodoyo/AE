
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the AWGAPI_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// AWGAPI_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef AWGAPI_EXPORTS
#define AWG_DLL_API __declspec(dllexport)
#else
#define AWG_DLL_API __declspec(dllimport)
#endif


AWG_DLL_API int  AWG_open(void);
AWG_DLL_API void AWG_close(void);

AWG_DLL_API void AWG_setWaveformType(int);
AWG_DLL_API void AWG_setAMwaveformControls(int,int,int,int,int);
AWG_DLL_API void AWG_setFrequencySweepControls(int,int,int,int);
AWG_DLL_API void AWG_setAEwaveformControls(int,int,int);
AWG_DLL_API void AWG_setArbitraryWaveformControls(int,short*,int);

AWG_DLL_API void AWG_setSignalControls(int,int,int,int,int);

AWG_DLL_API void AWG_setHighVoltageControls(int,int,int);

AWG_DLL_API void AWG_setTriggerControls(int,int,int,int,int,int);
AWG_DLL_API void AWG_triggerSingleShot(void);

AWG_DLL_API void AWG_setOutputMux(int,int,int,int);

AWG_DLL_API void AWG_activateControlSettings(void);

AWG_DLL_API void AWG_setOutputEnable(int);

AWG_DLL_API void AWG_triggerSingleShot(void);
