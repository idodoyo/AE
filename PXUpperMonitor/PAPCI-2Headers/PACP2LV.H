#define AMPLITUDE       1
#define ENERGY          2
#define COUNTS          3
#define DURATION        4
#define RMS             5
#define ASL             6
#define THRESHOLD       7
#define RISETIME        8
#define COUNTS_TO_PEAK  9
#define AVGFREQ         10
#define REV_FREQ        11
#define INIT_FREQ       12
#define SIG_STRENGTH    13
#define ABS_ENERGY      14
#define TIME            15
#define CHANNEL         16
#define PART_POWERS     17
#define FREQPP1         18
#define FREQPP2         19
#define FREQPP3         20
#define FREQPP4         21
#define FREQCENT        22
#define FREQPEAK        23
#define PARAM1          30
#define PARAM2          31
#define PARAM3          32
#define PARAM4          33
#define PARAM5          34
#define PARAM6          35
#define PARAM7          36
#define PARAM8          37
#define CYCLES          38

#define MAX_FREATURES  CYCLES+1

#define GOOD	0


#define STREAMTRIG_EXT      1   // external input triggers all streaming channels
#define STREAMTRIG_HIT      2   // hit on any stream channel causes all to stream
#define STREAMTRIG_TIMED    4   // automatic streaming after an interval has elapsed (start times are a multiple of this #)
#define STREAMTRIG_USER     8   // ignored by low end, high end must start the trigger when F11 is pressed
#define STREAMING_USER_END 16   // true streaming (PCole's definition), user starts and ends, only valid with STREAMTRIG_USER


#define ENABLE  1
#define DISABLE 0