#pragma once

#include "DataTypes.h"

class CGraham
{
public:
	typedef struct GPoint
	{
		HSDouble X;
		HSDouble Y;
		HSDouble Angle;
	} GPoint;
	
	static HSDouble VectorCos( HSDouble tX1, HSDouble tY1, HSDouble tX2, HSDouble tY2 )
	{
		return ( tX1 * tX2 + tY1 * tY2 ) / ( sqrt( tX1 * tX1 + tY1 * tY1 ) * sqrt( tX2 * tX2 + tY2 * tY2 ) );
	}
	
	static HSInt Comp( GPoint &tX1, GPoint &tX2 )
	{
		return tX1.Angle > tX2.Angle;
	}
	
	HSBool LeftRotated( GPoint *p1, GPoint *p2, GPoint *p3 )
	{
		return ( p1->X * ( p2->Y - p3->Y ) + p3->X * ( p1->Y - p2->Y ) + p2->X * ( p3->Y - p1->Y ) ) > 0;		
	}	
	
	void Cal( list< GPoint > *pList )
	{
		if ( pList->size() < 3 )
		{
			return;
		}

		list< GPoint >::iterator pIt1 = pList->begin();		
		list< GPoint >::iterator pIt3 = pIt1;
		HSDouble tX0 = pIt1->X;
		HSDouble tY0 = pIt1->Y;
		
		while ( pIt1 != pList->end() )
		{
			if ( pIt1->Y < tY0 )
			{
				tY0 = pIt1->Y;
				tX0 = pIt1->X;
				pIt3 = pIt1;
			}
			else if ( !( pIt1->Y > tY0 ) && pIt1->X < tX0 )
			{
				tX0 = pIt1->X;
				pIt3 = pIt1;
			}

			pIt1++;
		}
		
		for ( pIt1 = pList->begin(); pIt1 != pList->end(); pIt1++ )
		{
			pIt1->Angle = VectorCos( 1, 0, pIt1->X - pIt3->X, pIt1->Y - pIt3->Y );
		}
		
		pIt3->Angle = 1016996;
				
		pList->sort( Comp );
		
		pIt3 = pList->begin();
		pIt1 = pIt3;
		pIt3++;
		list< GPoint >::iterator pIt2 = pIt3;
		pIt3++;
		pList->push_back( *pIt1 );
		do
		{
			if ( LeftRotated( &( *pIt1 ), &( *pIt2 ), &( *pIt3 ) ) )
			{
				pIt1++;
				pIt2++;
				pIt3++;
			}
			else
			{
				pList->erase( pIt2 );				
				pIt2 = pIt1;
				pIt1--;
			}
		} while ( pIt3 != pList->end() );

		pList->pop_back();
	}	
};
