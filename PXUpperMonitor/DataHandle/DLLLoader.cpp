#include "stdafx.h"
#include "DLLLoader.h"

CDllLoader * CDllLoader::SharedInstance()
{
	static CDllLoader sDllLoader;
	return &sDllLoader;
}

CDllLoader::CDllLoader( HSString tDllName, HSString tFuncName )
{	
	if ( tDllName != "" && tFuncName != "" )
	{
		AddDLL( tDllName, tFuncName );
	}
}

CDllLoader::~CDllLoader()
{
	ReleaseLib();
}

HSVoid CDllLoader::ReleaseLib( HSString tDllName )
{
	if ( tDllName == "" )
	{
		map< HSString, DLL_INFO >::iterator pIterator = mDllsInfo.begin();
		while ( pIterator != mDllsInfo.end() )
		{
			if ( pIterator->second.LibInstance != NULL )
			{
				FreeLibrary( pIterator->second.LibInstance );
			}

			pIterator->second.LibInstance = NULL;

			pIterator++;
		}
	}
	else if ( mDllsInfo.find( tDllName ) != mDllsInfo.end() )
	{
		if ( mDllsInfo[ tDllName ].LibInstance != NULL )
		{
			FreeLibrary( mDllsInfo[ tDllName ].LibInstance );
		}

		mDllsInfo[ tDllName ].LibInstance = NULL;
	}
}

HSVoid CDllLoader::AddDLL( HSString tDllName, HSString tFuncName )
{
	DLL_INFO tDll = { NULL, tFuncName, NULL };
	mDllsInfo[ tDllName ] = tDll;
}

FARPROC CDllLoader::FuncAddress( HSString tDllName )
{
	if ( tDllName == "" )
	{
		map< HSString, DLL_INFO >::iterator pIterator = mDllsInfo.begin();
		while ( pIterator != mDllsInfo.end() )
		{
			if ( pIterator->second.FuncAddress != NULL )
			{
				return pIterator->second.FuncAddress;
			}

			pIterator++;
		}
	}
	else if ( mDllsInfo.find( tDllName ) != mDllsInfo.end() )
	{
		return mDllsInfo[ tDllName ].FuncAddress;		
	}

	return NULL;	
}

HSBool CDllLoader::Init()
{
	map< HSString, DLL_INFO >::iterator pIterator = mDllsInfo.begin();
	while ( pIterator != mDllsInfo.end() )
	{
		if ( pIterator->second.LibInstance != NULL )
		{
			pIterator++;
			continue;
		}

		HSString tLibName = pIterator->first + ".dll";
		pIterator->second.LibInstance = LoadLibrary( tLibName.c_str() );
		if ( pIterator->second.LibInstance == NULL )
		{			
			ReleaseLib( pIterator->first );
			return HSFalse;
		}
		
		pIterator->second.FuncAddress = GetProcAddress( pIterator->second.LibInstance, pIterator->second.FuncName.c_str() );
		if ( pIterator->second.FuncAddress == NULL )
		{			
			ReleaseLib( pIterator->first );
			return HSFalse;
		}				

		pIterator++;
	}

	return HSTrue;
}