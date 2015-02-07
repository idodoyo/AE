#ifndef DATA_HANDLER_HEADER_FILE
#define DATA_HANDLER_HEADER_FILE

#include <vector>
#include "DataTypes.h"
#include "ArgTableManager.h"
#include "DataIdentifier.h"

class CListBuffer;
class CLinearTransfer;

using std::vector;

typedef enum { DEVICE_USB_CARD, DEVICE_PCI_CARD } DEVICE_TYPE;

class IArgFetcher
{
public:
	virtual HSVoid FetchArg( DEVICE_CH_IDENTIFIER tChannelIdentifier, CLinearTransfer *pLinearTransfer, HSDouble tTime ) = 0;
	virtual HSInt BufferSize() = 0;

	virtual ~IArgFetcher(){}
};


class IDataHandler
{
public:
	virtual HSBool Start() = 0;
	virtual HSVoid Pause() = 0;
	virtual HSVoid Stop() = 0;

	virtual HSString Name() = 0;
	virtual DEVICE_CH_IDENTIFIER Identifier() = 0;	
	virtual HSUInt SampleRate( DEVICE_CH_IDENTIFIER tChannelIdentifier ) = 0;
	virtual HSUInt EachSampleSize() = 0;

	virtual DEVICE_CH_IDENTIFIER DataIdentifier( DEVICE_CH_IDENTIFIER tChannelIdentifier = 0 ) = 0;
	virtual HSUInt SamplesInFrame( DEVICE_CH_IDENTIFIER tChannelIdentifier ) = 0;
	virtual HSUInt GetChannelData( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSChar *pSrcBuf, HSChar *pDestBuf, HSInt tOffset, HSInt tSamples ) = 0;

	virtual HSBool GetArgList( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSUInt64 tNBeginTime, HSUInt64 tNDuration, vector< CArgTableManager::ArgItem > &tArgList, HSDouble &tLastSecond, HSDouble &tLastArgSecond ) = 0;
	virtual HSVoid AddArg( DEVICE_CH_IDENTIFIER tChannelIdentifier, CArgTableManager::ArgItem *pItem, HSDouble tLastSecond ) = 0;
	virtual HSUInt64 GetSampleNSecond( HSUInt tSampleReadIndex, HSUInt tSampleReadOffset, DEVICE_CH_IDENTIFIER tChannelIdentifier ) = 0;

	virtual HSDouble VoltageWithPointer( HSChar *pPointer ) = 0;
	virtual HSDouble BypassVoltage( HSUInt tChannel ) = 0;
	
	virtual HSInt ChannelNum() = 0;
	virtual DEVICE_TYPE Type() = 0;

	virtual HSBool OpenChannel( vector< HSInt > &tChannels, vector< HSInt > *pSampleRates = NULL, HSString *pErrorMsg = NULL ) = 0;
	virtual HSBool CloseAllChannel() = 0;

	virtual CListBuffer * ListBuffer() = 0;

	virtual IArgFetcher * ArgFetcher() = 0;

	virtual HSBool DataIsInSegent() = 0;

	virtual HSBool IsOn() = 0;

	virtual ~IDataHandler(){}
};


#endif

