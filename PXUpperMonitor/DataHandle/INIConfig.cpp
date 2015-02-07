#include "stdafx.h"
#include "INIConfig.h"


// CINIConfig construction


CINIConfig::CINIConfig()
{
}

HSBool CINIConfig::LoadFile( string tFileName )
{
	ifstream tFile( tFileName.c_str() );
	if ( !tFile.is_open() )
	{
		return HSFalse;
	}

	string tLine;
	string tGroup;
	string tKey;
	string tValue;	
	while ( getline( tFile, tLine ) )
	{
		Trim( tLine );
		if ( tLine.length() < 2 )
		{
			continue;
		}
		else if ( IsGroup( tLine, tGroup ) )
		{
			continue;
		}
		else if ( GetValueAndKey( tLine, tKey, tValue ) )
		{
			SetValue( tKey, tValue, tGroup );
		}
	}

	mFileName = tFileName;

	tFile.close();

	return HSTrue;
}

HSBool CINIConfig::Save( const HSChar *pFileName )
{
	string tFileName = mFileName;
	if ( pFileName != NULL )
	{
		tFileName = pFileName;
	}

	ofstream tFile( tFileName );
	if ( !tFile.is_open() )
	{		
		CreateSubDir( tFileName );

		tFile.open( tFileName );
		if ( !tFile.is_open() )
		{		
			return HSFalse;
		}
	}	

	map< string, ConfigItem >::iterator pGroupIterator = mConfigs.begin();
	while ( pGroupIterator != mConfigs.end() )
	{	
		tFile << "[" << ( *pGroupIterator ).first << "]\n";
		map< string, string > *pItems = &( *pGroupIterator ).second;
		map< string, string >::iterator pItemIterator = pItems->begin();
		while ( pItemIterator != pItems->end() )
		{
			tFile << ( *pItemIterator ).first << "=" << ( *pItemIterator ).second << "\n";

			pItemIterator++;
		}

		tFile << "\n";

		pGroupIterator++;
	}

	tFile.close();
	
	return HSTrue;
}

HSVoid CINIConfig::CreateSubDir( string tPath )
{	
	tPath = tPath.substr( 0, tPath.find_last_of( '\\' ) + 1 );
	string tTmpPath = "";
	while ( tPath.length() > 1 )
	{
		tTmpPath += tPath.substr( 0, tPath.find( '\\' ) + 1 );		
		tPath = tPath.substr( tPath.find( '\\' ) + 1 );

		CreateDirectory( tTmpPath.c_str(), NULL );
	}
}

HSBool CINIConfig::IsGroup( string &tLine, string &tGroup )
{
	if ( tLine.length() > 1 && tLine[ 0 ] == '[' && tLine[ tLine.length() - 1 ] == ']' )
	{
		tGroup = tLine.substr( 1, tLine.length() - 2 );
		return HSTrue;
	}

	return HSFalse;
}

HSBool CINIConfig::GetValueAndKey( string &tLine, string &tKey, string &tValue )
{
	std::size_t tPos = tLine.find( '=' );
	if ( tPos == string::npos )
	{
		return HSFalse;
	}

	tKey = Trim( tLine.substr( 0, tPos ) );
	tValue = Trim( tLine.substr( tPos + 1 ) );	

	return HSTrue;
}

HSBool CINIConfig::GroupNameWithIndex( HSInt tIndex, string &tName )
{
	map< string, ConfigItem >::iterator pGroupIterator = mConfigs.begin();
	while ( pGroupIterator != mConfigs.end() && tIndex > 0 )
	{
		pGroupIterator++;
		tIndex--;
	}

	if ( pGroupIterator == mConfigs.end() )
	{
		return HSFalse;
	}

	tName = pGroupIterator->first;

	return HSTrue;
}

HSVoid CINIConfig::ClearGroup( string tGroup )
{
	map< string, ConfigItem >::iterator pGroupIterator = mConfigs.find( tGroup );
	if ( pGroupIterator != mConfigs.end() )
	{
		mConfigs[ tGroup ].clear();
	}
}

string & CINIConfig::Trim( string &tStr )
{
	int tPos = tStr.find_first_not_of( ' ' );
	if ( tPos != std::string::npos )
	{
		tStr.erase( 0, tPos );
	}

	tStr.erase( tStr.find_last_not_of( ' ' ) + 1 );

	return tStr;
}
