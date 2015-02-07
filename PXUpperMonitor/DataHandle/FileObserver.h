#pragma once

#include "ObserverProtocol.h"
#include "DataTypes.h"
#include "DataHandler.h"

#include <fstream>
#include <string>
#include <map>

using std::ofstream;
using std::ifstream;
using std::string;
using std::map;

class CIndexManager;

class CFileObserver : public IObserverProtocol
{
public:
	CFileObserver( OBSERVER_TYPE tObserverType = OBSERVER_BACKGROUND, const HSChar *pFile = NULL, HSLong tFileSize = 0, CWnd *pParent = NULL );
	~CFileObserver();

public:
	HSVoid Update( HSChar *pBuf, HSInt tLength );
	HSVoid Update( DEVICE_CH_IDENTIFIER tType, HSChar *pBuf, HSInt tLength );
	HSLong FinalSize() const;
	OBSERVER_TYPE ObserverType() const;

public:
	HSBool SetFile( const HSChar *pFile, HSBool tForRead = HSFalse, HSInt64 tDisperseFileSize = 0 );
	string File(){ return mFile; }
	HSVoid SetDisperseFileSize( HSInt64 tSize );

public:	
	HSInt ReadData( HSChar *pBuf, HSULong tLength );

	enum READ_DATA_RES{ READ_SUCCESS, READ_FAILED, READ_NO_DATA };
	READ_DATA_RES ReadData( DEVICE_CH_IDENTIFIER tType, HSUInt tIndex, HSChar *pBuf, HSInt &tLength );
	HSBool SetFilePosition( HSInt64 tPosition );
	HSBool SetFileSavePosition( HSInt64 tPosition );
	HSInt64 FilePosition();
	HSVoid Close();

	HSInt FileBlockSize();
	CIndexManager * IndexManager(){ return mIndexManager; }

	HSInt ItemsLengthWithType( DEVICE_CH_IDENTIFIER tType, vector< HSUInt > &tItemsLength );
	
private:
	FILE * GetFileHandle( DEVICE_CH_IDENTIFIER tType );
	HSVoid CloseAllReadHandle();
	
private:
	FILE * mWriteStream;	
	OBSERVER_TYPE mObserverType;
	string mFile;
	HSLong mFileSize;		

	HANDLE mFileMutex;
	map< DEVICE_CH_IDENTIFIER, FILE * > mFileHandles;
	map< DEVICE_CH_IDENTIFIER, HSInt64 > mDisperseFileIndexs;

	HANDLE mWriteMutex;	
	FILE *mDisperseWriteStream;
	HSInt64 mCurDisperseWriteStreamPos;
	HSInt64 mDisperseWriteStreamSize;
	HSInt64 mDisperseFileLimit;
	HSString mDisperseFileFormat;
	HSInt mDisperseWriteStreamIndex;

	CIndexManager *mIndexManager;

	HSUInt mDataBlockSize;

	CWnd *mParent;
};

