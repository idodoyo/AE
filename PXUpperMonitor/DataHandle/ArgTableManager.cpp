#include "stdafx.h"
#include "ArgTableManager.h"
#include "HSLogProtocol.h"

CArgTableManager::CArgTableManager()
{
}

CArgTableManager::~CArgTableManager()
{
}

HSVoid CArgTableManager::AddArg( ArgItem &tItem )
{
	list< ArgItem >::iterator pIterator = mArgList.begin();
	while ( pIterator != mArgList.end() )
	{
		if ( pIterator->NBeginTime == tItem.NBeginTime )
		{
			return;
		}
		else if ( pIterator->NBeginTime > tItem.NBeginTime )
		{
			break;
		}
		pIterator++;
	}	

	mArgList.push_back( tItem );
}

HSDouble CArgTableManager::GetArg( HSUInt64 tNBeginTime, HSUInt64 tNDuration, vector< ArgItem > &tArgList )
{	
	list< ArgItem >::iterator pIterator = mArgList.begin();
	while ( pIterator != mArgList.end() )
	{
		if ( pIterator->NBeginTime >= tNBeginTime && pIterator->NBeginTime <= ( tNBeginTime + tNDuration ) )
		{
			tArgList.push_back( *pIterator );
		}
		else if (  pIterator->NBeginTime > ( tNBeginTime + tNDuration ) )
		{
			break;
		}		

		pIterator++;
	}	

	HSDouble tValue = 0;	
	if ( mArgList.size() > 0 )
	{
		pIterator = mArgList.end();
		pIterator--;
		tValue =( HSDouble ) pIterator->NBeginTime + pIterator->NDuration + 1;
	}

	tValue /= 1000000000.0;
	return tValue;
}