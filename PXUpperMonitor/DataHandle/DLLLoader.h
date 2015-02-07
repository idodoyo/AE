#pragma once

#include "DataTypes.h"

class CDllLoader
{
public:
	static CDllLoader * SharedInstance();

public:
	CDllLoader( HSString tDllName = "", HSString tFuncName = "" );
	~CDllLoader();

	HSVoid ReleaseLib( HSString tDllName = "" );	

public:
	HSVoid AddDLL( HSString tDllName, HSString tFuncName = "GetHSInstance" );
	FARPROC FuncAddress( HSString tDllName = "" );

	HSBool Init();

	template< typename T >
	HSBool InstanceWithDLL( T &tInstance, HSString tDllName = "" )
	{		
		FARPROC tAddress = FuncAddress( tDllName );
		if ( tAddress == NULL )
		{
			return HSFalse;
		}

		typedef T ( *GetHSInstance )();
		GetHSInstance fGetHSInstance = ( GetHSInstance )tAddress;
		tInstance = fGetHSInstance();

		return HSTrue;	
	}

private:
	typedef struct DLL_INFO
	{
		HINSTANCE LibInstance;
		HSString FuncName;
		FARPROC FuncAddress;
	} DLL_INFO;

	map< HSString, DLL_INFO > mDllsInfo;
};