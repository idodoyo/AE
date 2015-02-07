#pragma once

#include "DataTypes.h"

using std::list;

typedef enum{ SET_FRAME, OFFSET_FRAME } SEEK_FRAME_TYPE;
class IDataTransferProtocol
{
public:	
	virtual HSBool Read( HSChar *pBuf, HSInt &tLength ) = 0;
	virtual HSBool Seek( HSInt tFrameIndex, SEEK_FRAME_TYPE tType = OFFSET_FRAME ) = 0;
	virtual HSVoid Reset() = 0;

	virtual ~IDataTransferProtocol(){}
};
