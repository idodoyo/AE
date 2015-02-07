#pragma once

#include <vector>
#include "FileObserver.h"

using std::vector;

class CDataFileHeader
{
private:
	enum { DATA_FILE_IDENTIFIER = 1619 };

public:
	CDataFileHeader();
	~CDataFileHeader();

	HSBool ParseHeader( CFileObserver *pFile );
	HSVoid SetDeviceInfo( HSUInt64 tDevicePos, HSUInt tDeviceCount );
	HSVoid SetIndexInfo( HSUInt64 tIndexPos, HSUInt tIndexCount );
	HSVoid SetDeviceIndexInfo( HSUInt64 tDeviceIndexPos, HSUInt tDeviceIndexCount );
	HSUInt64 DevicePos();
	HSUInt DeviceCount();
	HSUInt64 IndexPos();
	HSUInt IndexCount();
	HSUInt64 DeviceIndexPos();
	HSUInt DeviceIndexCount();
	HSInt Size();	
	HSUShort Version();
	HSInt64 DisperseFileLimit();
	HSVoid SetDisperseFileLimit( HSInt64 tSize );
	HSBool Save( CFileObserver *pFile );

private:
	HSUShort mVersion;
	HSUInt64 mDevicePos;
	HSUInt mDeviceCount;
	HSUInt64 mIndexPos;
	HSUInt mIndexCount;	
	HSUInt64 mDeviceIndexPos;
	HSUInt mDeviceIndexCount;	
	HSUInt mSize;
	HSInt64 mDisperseFileLimit;
};

