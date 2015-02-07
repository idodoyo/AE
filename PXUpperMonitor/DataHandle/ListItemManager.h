#pragma once

#include "DataTypes.h"
#include "ListBuffer.h"

class CListItemManager
{
public:
	CListItemManager();
	~CListItemManager();

	HSVoid ResetList( CListBuffer *pListBuffer );
	HSVoid AddListBuffer( CListBuffer *pListBuffer );
	HSVoid ResetItem( CListBuffer *pListBuffer, CListBuffer::CListItem * &pItem );

	HSVoid BeginReset();
	HSVoid EndReset();

private:	
	map< CListBuffer *, HSInt > mListBuffers;
	HANDLE mMutex;
	HSBool mEndReset;
};

