#pragma once

#include "DataTypes.h"
#include "FileObserver.h"
#include "DataHandler.h"

#include <map>
#include <list>
#include <fstream>
#include <vector>

using std::map;
using std::list;
using std::vector;
using std::ifstream;
using std::ofstream;

class CIndexManager
{
public:
	CIndexManager();
	virtual ~CIndexManager();

	HSVoid AddIndex( DEVICE_CH_IDENTIFIER tType, HSUInt tIndex, HSUInt tOffset, HSUInt tLength );
	HSBool GetIndex( DEVICE_CH_IDENTIFIER tType, HSUInt tIndex, HSUInt &tResIndex, HSUInt &tResOffset, HSUInt &tResLength );
	
	HSBool Load( CFileObserver *pFile, HSUInt tIndexCount );
	HSBool Save( CFileObserver *pFile, DEVICE_CH_IDENTIFIER *pSaveType = NULL );

	HSUInt Count();
	HSUInt IndexCountWithType( DEVICE_CH_IDENTIFIER tType );
	DEVICE_CH_IDENTIFIER TypeWithIndex( HSUInt tIndex );

	HSInt64 TotalSizeWithType( DEVICE_CH_IDENTIFIER tType );
	HSInt64 TotalSizeOfAllType();

	HSVoid Reset();

	HSInt64 FilePos();

private:
	typedef struct IndexItem
	{ 
		HSUInt Index;
		HSUInt Offset;
		HSUInt Length;	
	} IndexItem;

	typedef vector< IndexItem > IndexItemList;

	map< DEVICE_CH_IDENTIFIER, IndexItemList > mIndexs;

	HANDLE mMutex;

	HSInt64 mFilePos;
};