// GraphicCylinder.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GraphicGantryCrane.h"
#include "afxdialogex.h"

#include "HSLogProtocol.h"
#include "resource.h"
#include "GLDataTypes.h"
#include "MainFrm.h"
#include "GantryCranePosition.h"
#include "GlFont.h"
#include <algorithm>

// CGraphicGantryCrane dialog

CGraphicGantryCrane::CGraphicGantryCrane(CWnd* pParent /*=NULL*/, HSBool tIs3DCrack )
{	
	mFocusedIndex = -1;
	
	mParent = dynamic_cast< CGantryCranePosition * >( pParent );
	m3DParent = mParent;	

	mMaterialXLength = 40.0;

	mInitOrientation = CGLQuaternion( CGLVector3( 0.407774, 0.908003, 0.096184 ), 0.675292 );	
	mInitOrientation.Normalize();
	
	mOrientation = mInitOrientation;
}

CGraphicGantryCrane::~CGraphicGantryCrane()
{
}

HSVoid CGraphicGantryCrane::DrawHit( HSDouble tPosX, vector< HSDouble > &tAreas )
{
	HSInt tIndex = 0;
	for ( HSUInt i = 0; i < tAreas.size(); i++ )
	{
		if ( tPosX < tAreas[ i ] )
		{
			tIndex = i;
			break;
		}
	}

	if ( tIndex == 0 )
	{
		return;
	}

	HSDouble tLength = ( tAreas[ tIndex ] - tAreas[ tIndex - 1 ] ) / mMaterialXLength * 240;

	glPushMatrix();
	glTranslatef( -240 / 2  + tLength / 2 + tAreas[ tIndex - 1 ] / mMaterialXLength * 240, 0, 0 );

	DrawCube( tLength, 25, 25, 0, CGLVector3( 0, 0, 0 ), HSFalse );	

	glPopMatrix();
}

HSVoid CGraphicGantryCrane::DrawSensor( HSDouble tPosX, HSInt tIndex )
{	
	glPushMatrix();
	glTranslatef( -240 / 2 + tPosX / mMaterialXLength * 240, 13.5, 0 );

	DrawCube( 2, 2, 2, 0, CGLVector3( 0, 0, 0 ), HSFalse );

	glPushMatrix();
	
	glColor4f( 0.1f, 0.1f, 0.1f, 0.8 );

	glTranslatef( -2, 2, 0 );

	CString tStrAngle;
	tStrAngle.Format( "S%d", tIndex + 1 );

	DrawString( ( LPCSTR )tStrAngle, CGLVector3( 4, 4, 1 ) );
	glPopMatrix();

	glPopMatrix();
}

HSVoid CGraphicGantryCrane::DrawPlank( HSDouble tAngle, CGLVector3 tTranslate )
{
	glPushMatrix();
	glTranslatef( tTranslate.X, tTranslate.Y, tTranslate.Z );
	glRotatef( tAngle, 1.0f, 0.0f, 0.0f );

	glColor4f( 0.5f, 0.1f, 1.0f, 0.5 );	

	CGLVector3 tPoints[] = { CGLVector3( 10, 42, 3 ),  CGLVector3( -10, 42, 3 ),  CGLVector3( -2, -42, 3 ),  CGLVector3( 2, -42, 3 ), 
							 CGLVector3( 10, 42,-3 ),  CGLVector3( -10, 42,-3 ),  CGLVector3( -2, -42,-3 ),  CGLVector3( 2, -42,-3 ) };

	DrawCustomSquare( tPoints );

	glPopMatrix();
}

HSVoid CGraphicGantryCrane::DrawRope()
{
	glPushMatrix();
	glTranslatef( 30, 30, 0 );
	glRotatef( 90, 1.0f, 0.0f, 0.0f );

	glColor4f( 0.5f, 0.1f, 1.0f, 0.5 );	

	gluCylinder( mCylinder, 0.5, 0.5, 40, 10, 10 );	

	glPopMatrix();

	DrawCube( 16, 16, 4, 0, CGLVector3( 30, -12, 0 ) );
	DrawCube( 16, 4, 4, 0, CGLVector3( 30, 32, 0 ) );
}

HSVoid CGraphicGantryCrane::DrawGraphic()
{	
	wglMakeCurrent( mGLDC, mRC );
	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	
	
	glPushMatrix();

	HSDouble tAngle = 0;
	CGLVector3 tRotation;
	mOrientation.GetAxisAngle( tRotation, tAngle );

	//HS_INFO( "Angle: %lf, X: %lf, Y: %lf, Z: %lf", tAngle, tRotation.X, tRotation.Y, tRotation.Z );

	glRotatef( tAngle * 180 / 3.14159265, tRotation.X, tRotation.Y, tRotation.Z );
	/*
	//DrawPlank( -20, CGLVector3( -85, -2, 30 ) );
	//DrawPlank( 20, CGLVector3( -85, -2, -30 ) );
	//DrawPlank( -20, CGLVector3( 85, -2, 30 ) );
	//DrawPlank( 20, CGLVector3( 85, -2, -30 ) );
	
	//DrawCube( 110, 10, 10, 90, CGLVector3( -85, -45, 0 ) );
	//DrawCube( 110, 10, 10, 90, CGLVector3( 85, -45, 0 ) );	
	//DrawCube( 190, 4, 2, 0, CGLVector3( 0, 34, 0 ) );
	*/
	DrawCube( 240, 25, 25, 0, CGLVector3( 0, 0, 0 ) );
	/*
	DrawCube( 20, 10, 20, 0, CGLVector3( -85, 45, 15 ) );	
	DrawCube( 20, 10, 20, 0, CGLVector3( -85, 45, -15 ) );	
	DrawCube( 20, 10, 20, 0, CGLVector3( 85, 45, 15 ) );	
	DrawCube( 20, 10, 20, 0, CGLVector3( 85, 45, -15 ) );

	DrawRope();		
	*/
	map< HSInt, SensorInfo >::iterator pSensorIterator = mPositionSensors->begin();
	while ( pSensorIterator != mPositionSensors->end() )
	{
		if ( pSensorIterator->second.Index == mFocusedIndex )
		{
			glColor4f( 0.0f, 0.0f, 1.0f, 0.8 );
		}
		else
		{
			glColor4f( 0.8f, 0.5f, 0.4f, 0.8 );
		}		
	
		DrawSensor( pSensorIterator->second.PosX, pSensorIterator->second.Index );

		pSensorIterator++;
	}

	if ( mPositionSensors->size() > 1 )
	{	
		vector< SensorInfo > tSensors;
		map< HSInt, SensorInfo >::iterator pIterator = mPositionSensors->begin();
		while ( pIterator != mPositionSensors->end() )
		{		
			tSensors.push_back( pIterator->second );

			pIterator++;
		}	

		std::sort( tSensors.begin(), tSensors.end(), CGantryCranePosition::SensorPosComapre );

		vector< HSDouble > tAreas;
		tAreas.push_back( 0 );		

		for ( HSUInt i = 1; i < tSensors.size(); i++ )
		{				
			tAreas.push_back( tSensors[ i - 1 ].PosX + ( tSensors[ i ].PosX - tSensors[ i - 1 ].PosX ) / 3.0 );
			tAreas.push_back( tSensors[ i - 1 ].PosX + ( tSensors[ i ].PosX - tSensors[ i - 1 ].PosX ) / 3.0 * 2.0 );

			if ( i != tSensors.size() - 1 )
			{
				tAreas.push_back( tSensors[ i ].PosX );
			}
		}		

		tAreas.push_back( this->mMaterialXLength );
			
		list< HitPosition >::iterator pHitIterator = mHitsPosition->begin();
		while ( pHitIterator != mHitsPosition->end() )
		{
			glColor4f( GetRValue( pHitIterator->Color ) / 255.0, GetGValue( pHitIterator->Color ) / 255.0, GetBValue( pHitIterator->Color ) / 255.0, 0.8 );
			DrawHit( pHitIterator->PosX, tAreas );

			pHitIterator++;
		}
	}

	glPopMatrix();
	
    SwapBuffers( mGLDC );
}

HSVoid CGraphicGantryCrane::InitGraphic()
{	
	//glEnable( GL_TEXTURE_2D );
	glEnable( GL_CULL_FACE );
	//glFrontFace( GL_CCW );		

	//glEnable( GL_DEPTH_TEST );

	mCylinder = gluNewQuadric();
	gluQuadricDrawStyle( mCylinder, GLU_LINE );	
	//gluQuadricNormals( mCylinder, GLU_SMOOTH );
	//gluQuadricTexture( mCylinder, GLU_FALSE );	
	
}

HSVoid CGraphicGantryCrane::DestroyGraphic()
{	
	gluDeleteQuadric( mCylinder );	
}


HSVoid CGraphicGantryCrane::SetMaterialLength( HSDouble tX )
{
	mMaterialXLength = tX;	
}