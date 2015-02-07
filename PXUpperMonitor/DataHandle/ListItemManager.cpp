#include "StdAfx.h"
#include "HSLogProtocol.h"
#include "ListItemManager.h"
#include "ListBuffer.h"

CListItemManager::CListItemManager()
{
	mMutex = CreateEvent( NULL, false, true, NULL );
	mEndReset = HSTrue;
}

CListItemManager::~CListItemManager()
{
	CloseHandle( mMutex );
}

HSVoid CListItemManager::ResetList( CListBuffer *pListBuffer )
{
	WaitForSingleObject( mMutex, INFINITE );

	mListBuffers[ pListBuffer ] = 0;

	SetEvent( mMutex );
}

HSVoid CListItemManager::AddListBuffer( CListBuffer *pListBuffer )
{
	if ( pListBuffer == NULL )
	{
		return;
	}

	WaitForSingleObject( mMutex, INFINITE );

	mListBuffers[ pListBuffer ] = 1;	

	SetEvent( mMutex );
}

HSVoid CListItemManager::ResetItem( CListBuffer *pListBuffer, CListBuffer::CListItem * &pItem )
{
	if ( !mEndReset || pListBuffer == NULL || pItem == NULL )
	{
		return;
	}

	WaitForSingleObject( mMutex, INFINITE );

	if ( mListBuffers.find( pListBuffer ) != mListBuffers.end() && mListBuffers[ pListBuffer ] == 1 )
	{		
		pListBuffer->SeekToPosition( pItem, 0 );		
	}

	SetEvent( mMutex );
}

HSVoid CListItemManager::BeginReset()
{
	mListBuffers.clear();
	mEndReset = HSFalse;
}

HSVoid CListItemManager::EndReset()
{
	mEndReset = HSTrue;
}