#pragma once

#include "DataTypes.h"
#include "HSLogProtocol.h"

class CGLVector3
{
public:
	CGLVector3( HSDouble tX = 0, HSDouble tY = 0, HSDouble tZ = 0 ): X( tX ), Y( tY ), Z( tZ ){}

	HSDouble Norm(){ return sqrt( X * X + Y * Y + Z * Z ); }
	HSBool IsZero(){ return abs( Norm() ) < 0.00001; }
	HSVoid Normalize()
	{
		HSDouble tNorm = Norm();
		if ( abs( tNorm ) > 0.00001 )
		{
			X /= tNorm;
			Y /= tNorm;
			Z /= tNorm;	
		}
	}

	CGLVector3 operator ^ ( const CGLVector3 &tVec3d ) const
	{
		return CGLVector3( Y * tVec3d.Z - tVec3d.Y * Z, tVec3d.X * Z - X * tVec3d.Z, X * tVec3d.Y - tVec3d.X * Y );
	}

	HSDouble operator * ( const CGLVector3 &tVec3d ) const
	{
		return ( X * tVec3d.X + Y * tVec3d.Y + Z * tVec3d.Z );
	}

public:
	HSDouble X;
	HSDouble Y;
	HSDouble Z;
};

class CGLQuaternion
{
public:
	CGLQuaternion( HSDouble tX = 0, HSDouble tY = 0, HSDouble tZ = 0, HSDouble tW = 1 ): X( tX ), Y( tY ), Z( tZ ), W( tW ){}
	CGLQuaternion( CGLVector3 tAxis, HSDouble tAngle )
	{	
		CGLVector3 tVn( tAxis );
		tVn.Normalize();

		tAngle *= 0.5;
		HSDouble tSinAngle = sin( tAngle );

		X = ( tVn.X * tSinAngle );
		Y = ( tVn.Y * tSinAngle );
		Z = ( tVn.Z * tSinAngle );
		W = cos( tAngle );
	}

	HSVoid Normalize()
	{		
		HSDouble tMag2 = W * W + X * X + Y * Y + Z * Z;		
		if ( abs( tMag2 - 1.0f ) > 0.00001 ) 
		{			
			HSDouble tMag = sqrt( tMag2 );

			W /= tMag;
			X /= tMag;
			Y /= tMag;
			Z /= tMag;
		}
	}

	HSVoid GetAxisAngle( CGLVector3 &tAxis, HSDouble &tAngle )
	{		
		tAngle = acos( W );	
		HSDouble tSinAngle = sin( tAngle );
		if ( abs( tSinAngle ) > 0.00001 )
		{
			HSDouble tSinAngleValue = 1.0 / tSinAngle;
		
			tAxis.X = X * tSinAngleValue;
			tAxis.Y = Y * tSinAngleValue;
			tAxis.Z = Z * tSinAngleValue;
			tAngle *= 2;
		}
		else
		{
			tAxis.X = 0;
			tAxis.Y = 0;
			tAxis.Z = 0;
			tAngle = 0;

			//X = 0;
			//Y = 0;
			//Z = 0;
			//W = 1;

			//HS_INFO( "X %f, Y: %f, Z:%f, W:%f", X, Y, Z, W );
		}
	}

	CGLQuaternion operator * ( const CGLQuaternion &tRq ) const
	{		
		return CGLQuaternion ( W * tRq.X + X * tRq.W + Y * tRq.Z - Z * tRq.Y,
							   W * tRq.Y + Y * tRq.W + Z * tRq.X - X * tRq.Z,
							   W * tRq.Z + Z * tRq.W + X * tRq.Y - Y * tRq.X,
							   W * tRq.W - X * tRq.X - Y * tRq.Y - Z * tRq.Z );
	}

public:
	HSDouble X;
	HSDouble Y;
	HSDouble Z;
	HSDouble W;
};
