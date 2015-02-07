#include "stdafx.h"
#include "IndexManager.h"
#include "HSLogProtocol.h"

CIndexManager::CIndexManager()
{
	mMutex = CreateEvent( NULL, false, true, NULL );
}

CIndexManager::~CIndexManager()
{
	CloseHandle( mMutex );
}

HSVoid CIndexManager::AddIndex( DEVICE_CH_IDENTIFIER tType, HSUInt tIndex, HSUInt tOffset, HSUInt tLength )
{	
	WaitForSingleObject( this->mMutex, INFINITE );
	
	IndexItem tIndexItem;
	tIndexItem.Index = tIndex;
	tIndexItem.Offset = tOffset;
	tIndexItem.Length = tLength;

	mIndexs[ tType ].push_back( tIndexItem );

	SetEvent( this->mMutex );
}

HSBool CIndexManager::GetIndex( DEVICE_CH_IDENTIFIER tType, HSUInt tIndex, HSUInt &tResIndex, HSUInt &tResOffset, HSUInt &tResLength )
{	
	WaitForSingleObject( this->mMutex, INFINITE );

	if ( mIndexs.find( tType ) == mIndexs.end() || tIndex >= mIndexs[ tType ].size() )
	{
		SetEvent( this->mMutex );		

		return HSFalse;
	}

	tResLength = ( mIndexs[ tType ] )[ tIndex ].Length;
	tResIndex = ( mIndexs[ tType ] )[ tIndex ].Index;
	tResOffset = ( mIndexs[ tType ] )[ tIndex ].Offset; 

	SetEvent( this->mMutex );

	return HSTrue;
}
	
HSBool CIndexManager::Load( CFileObserver *pFile, HSUInt tIndexCount )
{	
	Reset();

	DEVICE_CH_IDENTIFIER tType;
	HSUInt tCount = 0;
	
	HSUInt tCountIndex = 0;
	while ( tCountIndex < tIndexCount && pFile->ReadData( ( HSChar * )&tType, sizeof tType ) && pFile->ReadData( ( HSChar * )&tCount, sizeof tCount ) )	
	{
		HSUInt tIndex = 0;
		IndexItem tIndexItem;
		while ( tIndex < tCount && pFile->ReadData( ( HSChar * )&tIndexItem, sizeof tIndexItem ) )
		{
			mIndexs[ tType ].push_back( tIndexItem );
			tIndex++;
		}

		if ( tIndex < tCount )
		{			
			mIndexs.clear();
			return HSFalse;
		}

		tCountIndex++;
	}

	if ( tIndexCount != mIndexs.size() ) 
	{		
		mIndexs.clear();
		return HSFalse;
	}

	return HSTrue;
}

HSBool CIndexManager::Save( CFileObserver *pFile, DEVICE_CH_IDENTIFIER *pSaveType )
{
	HSUInt tBufferLength = 12 * 1024;
	HSChar *pBuffer = new HSChar[ tBufferLength ];
	HSInt tOffset = 0;	

	mFilePos = pFile->FilePosition();	

	map< DEVICE_CH_IDENTIFIER, IndexItemList >::iterator pTypeIterator = mIndexs.begin();
	while ( pTypeIterator != mIndexs.end() )
	{	
		DEVICE_CH_IDENTIFIER tType = pTypeIterator->first;		
		if ( pSaveType != NULL && !( tType == *pSaveType ) )
		{
			pTypeIterator++;
			continue;
		}

		/* type */
		pFile->Update( ( HSChar * )&tType, sizeof tType );

		/* item count */
		HSUInt tCount = pTypeIterator->second.size();
		pFile->Update( ( HSChar * )&tCount, sizeof tCount );		

		/* each item */
		tOffset = 0;
		IndexItemList *pItemList = &pTypeIterator->second;
		IndexItemList::iterator pItemIterator = pItemList->begin();
		while ( pItemIterator != pItemList->end() )
		{
			IndexItem *pItem = ( IndexItem * )&pBuffer[ tOffset ];
			*pItem = *pItemIterator;

			tOffset += sizeof( IndexItem );
			if ( tOffset == tBufferLength )
			{
				pFile->Update( pBuffer, tOffset );
				tOffset = 0;
			}

			pItemIterator++;
		}

		pFile->Update( pBuffer, tOffset );

		pTypeIterator++;
	}	

	delete[] pBuffer;	

	return HSTrue;
}

HSVoid CIndexManager::Reset()
{
	WaitForSingleObject( this->mMutex, INFINITE );

	mIndexs.clear();

	SetEvent( this->mMutex );
}

HSInt64 CIndexManager::FilePos()
{
	return mFilePos;
}

HSUInt CIndexManager::Count()
{
	return mIndexs.size();
}

HSUInt CIndexManager::IndexCountWithType( DEVICE_CH_IDENTIFIER tType )
{
	HSUInt tIndexCount = 0;

	WaitForSingleObject( this->mMutex, INFINITE );

	if ( mIndexs.find( tType ) != mIndexs.end() )
	{
		tIndexCount = mIndexs[ tType ].size();
	}		

	SetEvent( this->mMutex );

	return tIndexCount;
}

DEVICE_CH_IDENTIFIER CIndexManager::TypeWithIndex( HSUInt tIndex )
{
	WaitForSingleObject( this->mMutex, INFINITE );

	map< DEVICE_CH_IDENTIFIER, IndexItemList >::iterator pIterator = mIndexs.begin();
	while ( tIndex > 0 )
	{
		tIndex--;
		pIterator++;
	}	

	DEVICE_CH_IDENTIFIER tType = pIterator->first;
	
	SetEvent( this->mMutex );

	return tType;
}

HSInt64 CIndexManager::TotalSizeWithType( DEVICE_CH_IDENTIFIER tType )
{
	HSInt64 tTotalSize = 0;

	WaitForSingleObject( this->mMutex, INFINITE );

	if ( mIndexs.find( tType ) != mIndexs.end() )
	{		
		for ( HSUInt i = 0; i < mIndexs[ tType ].size(); i++ )
		{
			tTotalSize += ( mIndexs[ tType ] )[ i ].Length;
		}
	}		

	SetEvent( this->mMutex );

	return tTotalSize;
}

HSInt64 CIndexManager::TotalSizeOfAllType()
{
	HSInt64 tTotalSize = 0;

	WaitForSingleObject( this->mMutex, INFINITE );

	map< DEVICE_CH_IDENTIFIER, IndexItemList >::iterator pIterator = mIndexs.begin();
	while ( pIterator != mIndexs.end() )
	{
		for ( HSUInt i = 0; i < pIterator->second.size(); i++ )
		{
			tTotalSize += ( pIterator->second )[ i ].Length;
		}
		pIterator++;
	}		

	SetEvent( this->mMutex );

	return tTotalSize;
}