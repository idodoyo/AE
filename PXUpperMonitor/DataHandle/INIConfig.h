// INIConfig.h : main header file for the INIConfig DLL
//

#pragma once

#include "DataTypes.h"

#include <fstream>
#include <string>
#include <map>
#include <sstream>

using std::ifstream;
using std::ofstream;
using std::string;
using std::map;
using std::ostringstream;
using std::istringstream;


// CINIConfig

class CINIConfig
{
public:
	CINIConfig();

	HSBool LoadFile( string tFileName );
	HSBool Save( const HSChar *pFileName = NULL );

	template < typename T >
	HSVoid SetValue( string tKey, T tValue, string tGroup = "" )
	{
		ostringstream tStream;
		tStream.precision( 10 );
		tStream << tValue;
		( mConfigs[ tGroup ] )[ tKey ] = tStream.str();
	}

	template < typename T >
	HSBool ValueWithKey( string tKey, T &tValue, string tGroup )
	{
		map< string, ConfigItem >::iterator pGroupIterator = mConfigs.find( tGroup );
		if ( pGroupIterator != mConfigs.end() )
		{
			map< string, string > *pItems = &( *pGroupIterator ).second;
			map< string, string >::iterator pItemIterator = pItems->find( tKey );
			if ( pItemIterator != pItems->end() )
			{				
				istringstream tStream( pItemIterator->second );
				tStream >> tValue;	
				
				return HSTrue;
			}
		}

		return HSFalse;
	}

	HSBool ValueWithKey( string tKey, string &tValue, string tGroup )
	{
		map< string, ConfigItem >::iterator pGroupIterator = mConfigs.find( tGroup );
		if ( pGroupIterator != mConfigs.end() )
		{
			map< string, string > *pItems = &( *pGroupIterator ).second;
			map< string, string >::iterator pItemIterator = pItems->find( tKey );
			if ( pItemIterator != pItems->end() )
			{
				tValue = pItemIterator->second;				
				return HSTrue;
			}
		}

		return HSFalse;
	}

	HSBool GroupNameWithIndex( HSInt tIndex, string &tName );
	HSVoid ClearGroup( string tGroup );

private:
	HSBool IsGroup( string &tLine, string &tGroup );
	HSBool GetValueAndKey( string &tLine, string &tKey, string &tValue );
	string & Trim( string &tStr );
	HSVoid CreateSubDir( string tPath );

private:
	string mFileName;

	typedef map< string, string > ConfigItem;

	map< string, ConfigItem > mConfigs;
};

