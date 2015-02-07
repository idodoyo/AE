#include "StdAfx.h"
#include "DataFileHeader.h"
#include "HSLogProtocol.h"


CDataFileHeader::CDataFileHeader()
{
	mVersion = 2003;
	mStartTime = 0;
	mSize = 4 + 4 + 2 + 8 + 4 + 8 + 4 + 8 + 4 + 8 + 8;
}

CDataFileHeader::~CDataFileHeader()
{
}

HSBool CDataFileHeader::ParseHeader( CFileObserver *pFile )
{
	/* identifier, 4 byte */	
	HSInt tIdentifer = 0;
	if ( pFile->ReadData( ( HSChar * )&tIdentifer, 4 ) == 0 || tIdentifer != DATA_FILE_IDENTIFIER )
	{
		return HSFalse;
	}

	/* header size, 4 byte */
	if ( pFile->ReadData( ( HSChar * )&mSize, 4 ) == 0 || mSize < 1 )
	{
		return HSFalse;
	}	

	/* version, 2 byte */
	if ( pFile->ReadData( ( HSChar * )&mVersion, 2 ) == 0 || mVersion < 1 )
	{
		return HSFalse;
	}

	/* device pos, 8 byte */
	if ( pFile->ReadData( ( HSChar * )&mDevicePos, 8 ) == 0 || mDevicePos < 1 )
	{
		return HSFalse;
	}	

	/* device count, 4 byte */
	if ( pFile->ReadData( ( HSChar * )&mDeviceCount, 4 ) == 0 || mDeviceCount < 1 )
	{
		return HSFalse;
	}	

	/* index pos, 8 byte */
	if ( pFile->ReadData( ( HSChar * )&mIndexPos, 8 ) == 0 || mIndexPos < 1 )
	{
		return HSFalse;
	}	

	/* index count, 4 byte */
	if ( pFile->ReadData( ( HSChar * )&mIndexCount, 4 ) == 0 || mIndexCount < 1 )
	{
		return HSFalse;
	}		

	/* device index pos, 8 byte */
	if ( pFile->ReadData( ( HSChar * )&mDeviceIndexPos, 8 ) == 0 || mDeviceIndexPos < 1 )
	{
		return HSFalse;
	}	

	/* device index count, 4 byte */
	if ( pFile->ReadData( ( HSChar * )&mDeviceIndexCount, 4 ) == 0 || mDeviceIndexCount < 1 )
	{
		return HSFalse;
	}		

	HSInt64 tTopSize = 4096;
	tTopSize *= 1024 * 1024;
	/* disperse file limit, 8 byte */
	if ( pFile->ReadData( ( HSChar * )&mDisperseFileLimit, 8 ) == 0 || mDisperseFileLimit > tTopSize )
	{		
		return HSFalse;
	}

	/* start time, 8 byte */
	if ( mVersion >= 2003 && pFile->ReadData( ( HSChar * )&mStartTime, 8 ) == 0 )
	{		
		return HSFalse;
	}

	return HSTrue;
}

HSVoid CDataFileHeader::SetDeviceInfo( HSUInt64 tDevicePos, HSUInt tDeviceCount )
{
	mDevicePos = tDevicePos;
	mDeviceCount = tDeviceCount;
}

HSVoid CDataFileHeader::SetIndexInfo( HSUInt64 tIndexPos, HSUInt tIndexCount )
{
	mIndexPos = tIndexPos;
	mIndexCount = tIndexCount;
}

HSVoid CDataFileHeader::SetDeviceIndexInfo( HSUInt64 tDeviceIndexPos, HSUInt tDeviceIndexCount )
{
	mDeviceIndexPos = tDeviceIndexPos;
	mDeviceIndexCount = tDeviceIndexCount;
}

HSUInt64 CDataFileHeader::DeviceIndexPos()
{
	return mDeviceIndexPos;
}

HSUInt CDataFileHeader::DeviceIndexCount()
{
	return mDeviceIndexCount;
}

HSUInt64 CDataFileHeader::DevicePos()
{
	return mDevicePos;
}

HSUInt CDataFileHeader::DeviceCount()
{
	return mDeviceCount;
}

HSUInt64 CDataFileHeader::IndexPos()
{
	return mIndexPos;
}

HSUInt CDataFileHeader::IndexCount()
{
	return mIndexCount;
}

HSInt CDataFileHeader::Size()
{
	return mSize;
}

HSUShort CDataFileHeader::Version()
{
	return mVersion;
}

HSInt64 CDataFileHeader::DisperseFileLimit()
{
	return mDisperseFileLimit;
}

HSVoid CDataFileHeader::SetDisperseFileLimit( HSInt64 tSize )
{
	mDisperseFileLimit = tSize;
}

HSVoid CDataFileHeader::SetStartTime( HSInt64 tTime )
{
	mStartTime = tTime;
}

HSBool CDataFileHeader::Save( CFileObserver *pFile )
{
	pFile->SetFileSavePosition( 0 );

	/* identifier, 4 byte */		
	HSUInt tIdentifer = DATA_FILE_IDENTIFIER;
	pFile->Update( ( HSChar * )&tIdentifer, 4 );	

	/* header size, 4 byte */
	pFile->Update( ( HSChar * )&mSize, 4 );

	/* version, 2 byte */
	pFile->Update( ( HSChar * )&mVersion, 2 );	

	/* device pos, 8 byte */
	pFile->Update( ( HSChar * )&mDevicePos, 8 );	

	/* device count, 4 byte */
	pFile->Update( ( HSChar * )&mDeviceCount, 4 );

	/* index pos, 8 byte */
	pFile->Update( ( HSChar * )&mIndexPos, 8 );	

	/* index count, 4 byte */
	pFile->Update( ( HSChar * )&mIndexCount, 4 );

	/* device index pos, 8 byte */
	pFile->Update( ( HSChar * )&mDeviceIndexPos, 8 );	

	/* device index count, 4 byte */
	pFile->Update( ( HSChar * )&mDeviceIndexCount, 4 );	

	/* disperse file size limit, 8 byte */
	pFile->Update( ( HSChar * )&mDisperseFileLimit, 8 );	

	/* start time, 8 byte */
	pFile->Update( ( HSChar * )&mStartTime, 8 );	

	return HSTrue;
}
