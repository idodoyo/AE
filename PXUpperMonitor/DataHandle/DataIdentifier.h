#pragma once

#include "DataTypes.h"

class DEVICE_CH_IDENTIFIER
{
public:
	enum { CHANNEL_DATA_TYPE_ARG = 1, CHANNEL_DATA_TYPE_TIME = 16 };

public:
	HSUShort CARD_IDENTIFIER;
	HSUChar CARD_INDEX;
	HSUChar TYPE;	
	HSUInt CH_IDENTIFIER[ 4 ];	

public:
	DEVICE_CH_IDENTIFIER( HSUShort tCardIdentifier = 0, 
							HSUChar tCardIndex = 0, 
							HSUChar tType = 0,
							HSUInt tChannelIdentifier1 = 0, 
							HSUInt tChannelIdentifier2 = 0, 
							HSUInt tChannelIdentifier3 = 0, 
							HSUInt tChannelIdentifier4 = 0 )
	{
		CARD_IDENTIFIER = tCardIdentifier;
		CARD_INDEX = tCardIndex;		
		TYPE = tType;
		CH_IDENTIFIER[ 0 ] = tChannelIdentifier1;
		CH_IDENTIFIER[ 1 ] = tChannelIdentifier2;
		CH_IDENTIFIER[ 2 ] = tChannelIdentifier3;
		CH_IDENTIFIER[ 3 ] = tChannelIdentifier4;
	}

	HSVoid InitChannels( vector< HSInt > &tChannels )
	{		
		CH_IDENTIFIER[ 0 ] = 0;
		CH_IDENTIFIER[ 1 ] = 0;
		CH_IDENTIFIER[ 2 ] = 0;
		CH_IDENTIFIER[ 3 ] = 0;

		for ( HSUInt i = 0; i < tChannels.size(); i++ )
		{
			if ( tChannels[ i ] < 32 )
			{
				CH_IDENTIFIER[ 0 ] |= ( 1 << tChannels[ i ] );
			}
			else if ( tChannels[ i ] < 64 )
			{
				CH_IDENTIFIER[ 1 ] |= ( 1 << ( tChannels[ i ] - 32 ) );
			}
			else if ( tChannels[ i ] < 96 )
			{
				CH_IDENTIFIER[ 2 ] |= ( 1 << ( tChannels[ i ] - 64 ) );
			}
			else if ( tChannels[ i ] < 128 )
			{
				CH_IDENTIFIER[ 3 ] |= ( 1 << ( tChannels[ i ] - 96 ) );
			}			
		}		
	}
	
	bool operator<( const DEVICE_CH_IDENTIFIER &tValue ) const
	{
		if ( CARD_IDENTIFIER != tValue.CARD_IDENTIFIER )
		{
			return CARD_IDENTIFIER < tValue.CARD_IDENTIFIER;
		}
		else if ( CARD_INDEX != tValue.CARD_INDEX )
		{
			return CARD_INDEX < tValue.CARD_INDEX;
		}
		else if ( TYPE != tValue.TYPE )
		{
			return TYPE < tValue.TYPE;
		}
		else if ( CH_IDENTIFIER[ 0 ] != tValue.CH_IDENTIFIER[ 0 ] )
		{
			return CH_IDENTIFIER[ 0 ] < tValue.CH_IDENTIFIER[ 0 ];
		}
		else if ( CH_IDENTIFIER[ 1 ] != tValue.CH_IDENTIFIER[ 1 ] )
		{
			return CH_IDENTIFIER[ 1 ] < tValue.CH_IDENTIFIER[ 1 ];
		}       
		else if ( CH_IDENTIFIER[ 2 ] != tValue.CH_IDENTIFIER[ 2 ] )
		{
			return CH_IDENTIFIER[ 2 ] < tValue.CH_IDENTIFIER[ 2 ];
		}
		else
		{
			return CH_IDENTIFIER[ 3 ] < tValue.CH_IDENTIFIER[ 3 ];
		}       
    }

	bool operator==( const DEVICE_CH_IDENTIFIER &tValue ) const
	{		
		return CARD_IDENTIFIER == tValue.CARD_IDENTIFIER 
				&& CARD_INDEX == tValue.CARD_INDEX 
				&& TYPE == tValue.TYPE 	
				&& CH_IDENTIFIER[ 0 ] == tValue.CH_IDENTIFIER[ 0 ] 
				&& CH_IDENTIFIER[ 1 ] == tValue.CH_IDENTIFIER[ 1 ] 
				&& CH_IDENTIFIER[ 2 ] == tValue.CH_IDENTIFIER[ 2 ] 
				&& CH_IDENTIFIER[ 3 ] == tValue.CH_IDENTIFIER[ 3 ];
	}  
	
	HSVoid InitChannel( HSInt tChannel )
	{		
		vector< HSInt > tChannels;
		tChannels.push_back( tChannel );
		InitChannels( tChannels );
	}
	
	HSVoid InitChannel()
	{		
		CH_IDENTIFIER[ 0 ] = 0;
		CH_IDENTIFIER[ 1 ] = 0;
		CH_IDENTIFIER[ 2 ] = 0;
		CH_IDENTIFIER[ 3 ] = 0;
		TYPE = 0;
	}

	HSVoid InitWithString( HSString tInfo )
	{
		HSInt tCardIndex = 0;
		HSInt tType = 0;
		istringstream tStream( tInfo );
		tStream >> CARD_IDENTIFIER >> tCardIndex >> tType;		
		tStream >> CH_IDENTIFIER[ 0 ] >> CH_IDENTIFIER[ 1 ] >> CH_IDENTIFIER[ 2 ] >> CH_IDENTIFIER[ 3 ];

		CARD_INDEX = ( HSChar )tCardIndex;
		TYPE = ( HSChar )tType;
	}

	HSString ToString()
	{
		HSChar tBuf[ 1024 ];
		sprintf_s( tBuf, "%d %d %d %d %d %d %d", CARD_IDENTIFIER, CARD_INDEX, TYPE, CH_IDENTIFIER[ 0 ], CH_IDENTIFIER[ 1 ], CH_IDENTIFIER[ 2 ], CH_IDENTIFIER[ 3 ] );
		return tBuf;
	}

	HSVoid ActionAnd( DEVICE_CH_IDENTIFIER tIdentifier )
	{
		if ( this->CARD_IDENTIFIER != tIdentifier.CARD_IDENTIFIER || this->CARD_INDEX != tIdentifier.CARD_INDEX )
		{
			CH_IDENTIFIER[ 0 ] = 0;
			CH_IDENTIFIER[ 1 ] = 0;
			CH_IDENTIFIER[ 2 ] = 0;
			CH_IDENTIFIER[ 3 ] = 0;
		}
		else
		{
			CH_IDENTIFIER[ 0 ] &= tIdentifier.CH_IDENTIFIER[ 0 ];
			CH_IDENTIFIER[ 1 ] &= tIdentifier.CH_IDENTIFIER[ 1 ];
			CH_IDENTIFIER[ 2 ] &= tIdentifier.CH_IDENTIFIER[ 2 ];
			CH_IDENTIFIER[ 3 ] &= tIdentifier.CH_IDENTIFIER[ 3 ];			
		}
	}

	HSInt NumOfChannels( vector< HSUInt > *pChannels = NULL )
	{
		HSInt tChannels = 0;
		for ( HSInt tIndex = 0; tIndex < 4; tIndex++ )
		{
			for ( HSUInt i = 0; i < sizeof( HSUInt ); i++ )
			{
				if ( ( CH_IDENTIFIER[ tIndex ] & ( 1 << i ) ) )
				{
					tChannels++;		

					if ( pChannels != NULL )
					{
						pChannels->push_back( i + tIndex * sizeof( HSUInt ) );
					}
				}
			}
		}		
		
		return tChannels;
	}

	HSInt ChannelIndex()
	{					
		for ( HSInt tIndex = 0; tIndex < 4; tIndex++ )
		{
			for ( HSUInt i = 0; i < sizeof( HSUInt ); i++ )
			{
				if ( ( CH_IDENTIFIER[ tIndex ] & ( 1 << i ) ) )
				{
					return i + tIndex * sizeof( HSUInt );					
				}
			}
		}				

		return 0;
	}

	HSVoid ChannelInfoWithIdentifier( DEVICE_CH_IDENTIFIER tIdentifier, HSInt &tStart, HSInt &tLength )
	{
		tStart = 0;
		tLength = 0;

		int tDataStartChannel = this->ChannelIndex();		

		for ( HSInt tIndex = 0; tIndex < 4; tIndex++ )
		{
			for ( HSUInt i = 0; i < sizeof( HSUInt ); i++ )
			{
				if ( ( tIdentifier.CH_IDENTIFIER[ tIndex ] & ( 1 << i ) ) )
				{
					if ( tLength == 0 )
					{
						tStart = i + tIndex * sizeof( HSUInt );	
					}

					tLength++;							
				}
			}
		}						

		tStart -= tDataStartChannel;		
	}	

};
