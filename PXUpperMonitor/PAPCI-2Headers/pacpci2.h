#ifndef _PACPCI2_H_
#define _PACPCI2_H_

//***********************************************************************
//
// DLL function prototypes for functions exported from the DLL
//
//***********************************************************************

#include "dll.h"    // for DLL_API, definition for using the DLL

#ifdef __cplusplus
extern "C" {
#endif

DLL_API short openPCI2(void);
DLL_API void  closePCI2(void);

DLL_API bool  checkChannelHardwarePresent(short chanID);
DLL_API bool  checkChannelParametricPresent(short ParametricId); 

DLL_API short setTimeDrivenRate(unsigned short tdRateMs);
DLL_API short setRMS_ASL_TimeConstant(unsigned short tcMs);
DLL_API short setChannelGain(unsigned short chanId, unsigned short gain_db); 
DLL_API short setPreAmpGain(short channel, short dB); 
DLL_API short setChannel(unsigned short chanId, unsigned short on_off);
DLL_API short setChannelThresholdType(unsigned short chanId, unsigned short type);
DLL_API short setChannelThreshold(unsigned short chanId, unsigned short thresh);
DLL_API short setChannelFloatingThresholdDeadband(short chanId, short dB);
DLL_API short setChannelHLT(unsigned short chanId, unsigned short hlt);
DLL_API short setChannelHDT(unsigned short chanId, unsigned short hdt);
DLL_API short setChannelPDT(unsigned short chanId, unsigned short pdt);
DLL_API short setChannelMaxDuration(unsigned short chanId, unsigned short duration_ms); 
DLL_API short setAnalogFilter(unsigned short chanId, unsigned short lp, unsigned short hp);
DLL_API short setSampleRate(unsigned short chanId, unsigned short rate_kHz);
DLL_API short setWaveformLength(unsigned short chanId, unsigned short lengthK);
DLL_API short setWaveformPreTrigger(unsigned short chanId, short samples);
DLL_API short setWaveformTransfer(unsigned short on_off);
DLL_API short stopWaveformTransfer(void);
DLL_API short startWaveformTransfer(void);
DLL_API short setHitFeature(unsigned short featureId, unsigned short on_off);
DLL_API short setHitParametric(unsigned short parametricId, unsigned short on_off);
DLL_API short setTimeDrivenFeature(unsigned short featureId, unsigned short on_off);
DLL_API short setTimeDrivenParametric(unsigned short parametricId, unsigned short on_off);
DLL_API short setHitFftFrequencySpan(int totalStart, int totalEnd, int numberSegments, int *startSegment, int *endSegment); 
DLL_API short setParametricGain(unsigned short parametricId, unsigned short gain);
DLL_API short setParametricFilter(unsigned short parametricId, unsigned short in_out);
DLL_API short setCycleCounterSource(unsigned short parametricId);
DLL_API short setCycleCounterThreshold(short thresholdmV);
DLL_API short setCycleCounterFilter(unsigned short in_out);
DLL_API short validateSetup(void);
DLL_API unsigned short copySetupMessagesToBuffer(unsigned char *buffer);
DLL_API short startTest(void);
DLL_API short stopTest(void);
DLL_API short abortTest(void);
DLL_API short pauseTest(void);
DLL_API short resumeTest(void);
DLL_API short sendTimeMark(void);          
DLL_API short readTimeOfTest(double *elasped_seconds);
DLL_API short pulseChannelAST(unsigned short chanId);
DLL_API short startAST(short pulse_count, short pulse_width_us, short pulse_interval_ms);
DLL_API short startAST_Ex(short *channel_list,
                   short channel_count,
                   short pulse_count,
                   short pulse_width_us,
                   short pulse_interval_ms,
                   short generate_output_file,
                   char *output_file_name,
                   short compare_reference_file,
                   char *compare_file_name,
                   unsigned short failPercent,
                   int *complete_flag);
DLL_API short forceTrigger(unsigned short channelId);

DLL_API void  poll(void);
DLL_API short getMessage(unsigned char *msg, unsigned short maxLength);
DLL_API double readForcedParametric(short parametricId);
DLL_API short getHitDataSetValue(unsigned char *msg, unsigned short q, float *fVal);
DLL_API short getTddDataSetValue(unsigned char *msg, unsigned short chanId, unsigned short q, float *fVal);
DLL_API short getWaveformValue(unsigned char *msg, unsigned short q, float *fVal);
DLL_API short getTimeOfTest(unsigned char *msg, unsigned short offset, float *fVal);
DLL_API short getTimeOfCommand(unsigned char *msg, float *fVal);
DLL_API short calculateCentroidAndPeak(unsigned short SampleRatekHz);
DLL_API short getCentroidAndPeak(unsigned short *centroid, unsigned short *peak); 
DLL_API short setFftFrequencySpan(int totalStart, int totalEnd, int numberSegments, int *startSegment, int *endSegment); 
DLL_API short getFftPartialPowers(int index, short *percentage);
DLL_API short calculateWaveformMsgFftPartialPowers(unsigned char *msg);
DLL_API short scaleWaveform(unsigned char *msg, float scale_factor, float *scaled_waveform, short output_length);

DLL_API short setAudioChannel(short channel, short enabled);

DLL_API void  setDigitalOutput(int bitId, bool on_off); 
DLL_API short readDigitalInput(int bitId); 



DLL_API short enableWaveformStreaming(int on_off);
DLL_API short setWaveformStreamingMode(int mode);
DLL_API short setWaveformStreamingPeriod(unsigned long period_sec);
DLL_API short setWaveformStreamingFilePrefix(char *prefix);
DLL_API short setWaveformStreamingChannel(unsigned short chanId, int on_off);
DLL_API short setWaveformStreamingLength(unsigned short chanId, long preTrigger_kSamples, unsigned long length_kSamples);

DLL_API unsigned long getRequiredSampleBufferLen(int channel);
DLL_API float *setStreamingBuffer(int channel, float *f, unsigned long nElements, short enable);
DLL_API void setStreamingEvents(HANDLE hEventStart, HANDLE hEventStop);
DLL_API void enablePolling(short enable);
DLL_API void enableWFSOutput(short enable);
DLL_API short isStreamingStarted(void);
DLL_API short isStreamingFinished(void);
DLL_API void rearmStreaming(void);
DLL_API short startWaveformStreaming(void);
DLL_API short stopWaveformStreaming(void);

DLL_API const byte *getSetupBuffer();
DLL_API unsigned short getSetupBufferSize();



#ifdef __cplusplus
}
#endif

#endif  // _PACPCI2_H_


