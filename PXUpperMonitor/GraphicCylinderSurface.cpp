// GraphicCylinderSurface.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GraphicCylinderSurface.h"
#include "afxdialogex.h"


#include "HSLogProtocol.h"
#include "resource.h"
#include "GLDataTypes.h"
#include "MainFrm.h"
#include "CylinderSurfacePosition.h"
#include "GraphicSphere.h"
#include "GlFont.h"

// CGraphicCylinderSurface dialog


CGraphicCylinderSurface::CGraphicCylinderSurface(CWnd* pParent /*=NULL*/)
{
	mFocusedIndex = -1;

	mParent = dynamic_cast< CCylinderSurfacePosition * >( pParent );
	m3DParent = mParent;

	mHitRadius = 2.0;
	
	mYLength = 50.0;

	mMaterialYLength = 40.0;

	mInitOrientation = CGLQuaternion( CGLVector3( 0.1, -1, -0.3 ), 155 * 3.14159265 / 180 );
	mInitOrientation.Normalize();

	mOrientation = mInitOrientation;
}

CGraphicCylinderSurface::~CGraphicCylinderSurface()
{	
}

HSVoid CGraphicCylinderSurface::DrawHit( HSDouble tAngle, HSDouble tHeight, HSDouble tRadius )
{   
	glPushMatrix();
	
	glRotatef( tAngle + 90, 0.0, 1.0, 0 );
	glTranslatef( 0, tHeight, tRadius );


	GLfloat tValue = 0.65f * ( mHitRadius / 3.0 );

    glBegin( GL_QUADS );  
		glNormal3f( 0.0F, 0.0F, 1.0F);  
		glVertex3f( tValue, tValue, tValue);  
		glVertex3f(-tValue, tValue, tValue);  
		glVertex3f(-tValue,-tValue, tValue);  
		glVertex3f( tValue,-tValue, tValue);  
		//1----------------------------   
		glNormal3f( 0.0F, 0.0F,-1.0F);  
		glVertex3f(-tValue,-tValue,-tValue);  
		glVertex3f(-tValue, tValue,-tValue);  
		glVertex3f( tValue, tValue,-tValue);  
		glVertex3f( tValue,-tValue,-tValue);  
		//2----------------------------   
		glNormal3f( 0.0F, 1.0F, 0.0F);  
		glVertex3f( tValue, tValue, tValue);  
		glVertex3f( tValue, tValue,-tValue);  
		glVertex3f(-tValue, tValue,-tValue);  
		glVertex3f(-tValue, tValue, tValue);  
		//3----------------------------   
		glNormal3f( 0.0F,-1.0F, 0.0F);  
		glVertex3f(-tValue,-tValue,-tValue);  
		glVertex3f( tValue,-tValue,-tValue);  
		glVertex3f( tValue,-tValue, tValue);  
		glVertex3f(-tValue,-tValue, tValue);  
		//4----------------------------   
		glNormal3f( 1.0F, 0.0F, 0.0F);  
		glVertex3f( tValue, tValue, tValue);  
		glVertex3f( tValue,-tValue, tValue);  
		glVertex3f( tValue,-tValue,-tValue);  
		glVertex3f( tValue, tValue,-tValue);  
		//5----------------------------   
		glNormal3f(-1.0F, 0.0F, 0.0F);  
		glVertex3f(-tValue,-tValue,-tValue);  
		glVertex3f(-tValue,-tValue, tValue);  
		glVertex3f(-tValue, tValue, tValue);  
		glVertex3f(-tValue, tValue,-tValue);  
		//6----------------------------*/   
    glEnd();  	

	glPopMatrix();
}

HSVoid CGraphicCylinderSurface::DrawSensor( HSDouble tAngle, HSDouble tHeight, HSDouble tRadius, HSInt tIndex )
{
	glPushMatrix();
	glRotatef( tAngle + 90, 0.0, 1.0, 0 );
	glTranslatef( 0, tHeight, tRadius );	

	HSDouble tValue = 1.0;
	glBegin( GL_QUADS );  
		glNormal3f( 0.0F, 0.0F, 1.0F);  
		glVertex3f( tValue, tValue, tValue);  
		glVertex3f(-tValue, tValue, tValue);  
		glVertex3f(-tValue,-tValue, tValue);  
		glVertex3f( tValue,-tValue, tValue);  
		//1----------------------------   
		glNormal3f( 0.0F, 0.0F,-1.0F);  
		glVertex3f(-tValue,-tValue,-tValue);  
		glVertex3f(-tValue, tValue,-tValue);  
		glVertex3f( tValue, tValue,-tValue);  
		glVertex3f( tValue,-tValue,-tValue);  
		//2----------------------------   
		glNormal3f( 0.0F, 1.0F, 0.0F);  
		glVertex3f( tValue, tValue, tValue);  
		glVertex3f( tValue, tValue,-tValue);  
		glVertex3f(-tValue, tValue,-tValue);  
		glVertex3f(-tValue, tValue, tValue);  
		//3----------------------------   
		glNormal3f( 0.0F,-1.0F, 0.0F);  
		glVertex3f(-tValue,-tValue,-tValue);  
		glVertex3f( tValue,-tValue,-tValue);  
		glVertex3f( tValue,-tValue, tValue);  
		glVertex3f(-tValue,-tValue, tValue);  
		//4----------------------------   
		glNormal3f( 1.0F, 0.0F, 0.0F);  
		glVertex3f( tValue, tValue, tValue);  
		glVertex3f( tValue,-tValue, tValue);  
		glVertex3f( tValue,-tValue,-tValue);  
		glVertex3f( tValue, tValue,-tValue);  
		//5----------------------------   
		glNormal3f(-1.0F, 0.0F, 0.0F);  
		glVertex3f(-tValue,-tValue,-tValue);  
		glVertex3f(-tValue,-tValue, tValue);  
		glVertex3f(-tValue, tValue, tValue);  
		glVertex3f(-tValue, tValue,-tValue);  
		//6----------------------------*/   
    glEnd();  	

	glPushMatrix();
	
	glColor4f( 0.1f, 0.1f, 0.1f, 0.8 );	

	//glRotatef( 155.0, 0.0, 1.0, 0 );

	CString tStrAngle;
	tStrAngle.Format( "S%d", tIndex + 1 );

	DrawString( ( LPCSTR )tStrAngle, CGLVector3( 4, 4, 1 ) );
	glPopMatrix();

	glPopMatrix();
}

HSVoid CGraphicCylinderSurface::DrawXAxis( CGLVector3 tCenterPos )
{	
	glPushMatrix();
		
		glTranslatef( tCenterPos.X + 20, tCenterPos.Y, tCenterPos.Z - 3 );
		glRotatef( 90, 1.0, 0.0, 0.0 );			
		glRotatef( 180, 0.0, 1.0, 0.0 );			
		
		DrawString( "Angle 0", CGLVector3( 10, 10, 1 ) );

	glPopMatrix();
}

HSVoid CGraphicCylinderSurface::DrawYAxis( CGLVector3 tPos, HSDouble tYDelta, HSInt tCount, HSDouble tYValueBegin, HSDouble tYValueDelta, CGLVector3 tCenterPos )
{
	CString tStr;
	for ( HSInt i = 0; i <= tCount; i++ )
	{
		glPushMatrix();
		
		glTranslatef( tPos.X, tPos.Y, tPos.Z );
		glRotatef( 180, 0.0, 1.0, 0.0 );				
	
		tStr.Format( "%5.01f", tYValueBegin );
		DrawString( ( LPCSTR )tStr );

		glPopMatrix();		

		tPos.Y += tYDelta;
		tYValueBegin += tYValueDelta;
	}

	glPushMatrix();
		
		glTranslatef( tCenterPos.X, tCenterPos.Y, tCenterPos.Z );	
		
		DrawString( "Y", CGLVector3( 10, 10, 1 ) );

	glPopMatrix();
}

HSVoid CGraphicCylinderSurface::DrawZAxis( CGLVector3 tCenterPos )
{	
	glPushMatrix();

	glTranslatef( tCenterPos.X + 1, tCenterPos.Y, tCenterPos.Z + 15 );			
	glRotatef( 90, 0.0, 0.0, 1.0 );		
	glRotatef( 90, 0.0, 1.0, 0.0 );	
		
	DrawString( "Angle 90", CGLVector3( 10, 10, 1 ) );		

	glPopMatrix();
}

HSVoid CGraphicCylinderSurface::DrawCircle( HSDouble tRadius )
{
	HSInt N = 600;
	HSDouble PI = 3.14159265;
	glBegin( GL_LINE_LOOP );   
	for ( HSInt i = 0; i < N; i++ )     
	{	
		glVertex3f( tRadius * cos( 2 * PI / N * i ), 0.0,  tRadius * sin( 2 * PI / N * i ) ); 	
	}

	glEnd();    
}

HSVoid CGraphicCylinderSurface::DrawCylinder( HSDouble tHeight, HSDouble tRadius )
{
	glPushMatrix();

	glTranslatef( 0, -tHeight / 2, 0 );
	glRotatef( 90, -1, 0, 0 );
	gluCylinder( mCylinder, tRadius, tRadius, tHeight, 60, 60 );

	glPopMatrix();

	glLineWidth( 1.0 );
	HSDouble tHeightDelta = tHeight / 4;
	HSDouble tCircleHeight = -tHeight / 2;
	glColor4f( 0.5f, 0.1f, 0.5f, 0.8 );
	for ( HSInt i = 0; i <= 4; i++ )
	{
		glPushMatrix();
		glTranslatef( 0, tCircleHeight, 0 );
		DrawCircle( tRadius );
		glPopMatrix();

		tCircleHeight += tHeightDelta;
	}

	HSDouble tDirectionAngleDelta = 360 / 36;
	for ( HSInt j = 0; j < 36; j++ )
	{
		if ( j % 9 == 0 )
		{
			glLineWidth( 1.0 );
			glColor4f( 0.5f, 0.1f, 0.5f, 0.8 );
		}
		else
		{
			glLineWidth( 1.0 );
			glColor4f( 0.6f, 0.6f, 0.6f, 0.0 );
		}

		glPushMatrix();
		
		glRotatef( j * tDirectionAngleDelta, 0.0, 1.0, 0 );
		glTranslatef( 0, 0, tRadius );

		glBegin( GL_LINE_STRIP );
		glVertex3f( 0.0, -tHeight / 2.0,  0.0 );
		glVertex3f( 0.0, tHeight / 2.0,  0.0 );
		glEnd();    

		glPopMatrix();
	}		
}

HSVoid CGraphicCylinderSurface::DrawGraphic()
{
	wglMakeCurrent( mGLDC, mRC );
	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	
	
	glPushMatrix();

	HSDouble tAngle = 0;
	CGLVector3 tRotation;
	mOrientation.GetAxisAngle( tRotation, tAngle );	

	glRotatef( tAngle * 180 / 3.14159265, tRotation.X,  tRotation.Y,  tRotation.Z );	
	
	DrawAxisFace( CGLVector3( 0, 0, 0 ), 0, CGLVector3( 0, 0, GRAPHIC_RADIUS ), CGLVector3( 0, 0, -1 ) );
	DrawAxisFace( CGLVector3( 1, 0, 0 ), 90, CGLVector3( 0, -GRAPHIC_RADIUS, 0 ), CGLVector3( 0, 0, 1 ) );
	DrawAxisFace( CGLVector3( 0, -1, 0 ), 90, CGLVector3( -GRAPHIC_RADIUS, 0, 0 ), CGLVector3( 0, 0, -1 ) );

	glColor4f( 0.1f, 0.1f, 0.1f, 0.8 );		

	DrawXAxis( CGLVector3( GRAPHIC_RADIUS + 15, -GRAPHIC_RADIUS, 0 ) );
	DrawYAxis( CGLVector3( GRAPHIC_RADIUS + 13, -GRAPHIC_RADIUS - 1.5, GRAPHIC_RADIUS ), GRAPHIC_RADIUS / 2.5, 5, mYAxisInfo.Begin, mYAxisInfo.Delta,  CGLVector3( GRAPHIC_RADIUS + 15, 0, GRAPHIC_RADIUS ) );
	DrawZAxis( CGLVector3( 0, -GRAPHIC_RADIUS, -GRAPHIC_RADIUS - 20 ) );
	
	HSDouble tRadius = GRAPHIC_RADIUS - 5;
	
	glPushMatrix();
	
	glTranslatef( 0.0, -( ( 2 * mYAxisInfo.Begin + mYAxisInfo.Delta * 5.0 ) / 2.0 - mMaterialYLength / 2.0 ) * mYAxisInfo.DigitPerValue, 0.0 );

	glColor4f( 0.3f, 0.1f, 1.0f, 0.3 );
	DrawCylinder( mYAxisInfo.DigitPerValue * mMaterialYLength, tRadius );

	glPopMatrix();
	
	list< HitPosition >::iterator pHitIterator = mHitsPosition->begin();
	while ( pHitIterator != mHitsPosition->end() )
	{
		glColor4f( GetRValue( pHitIterator->Color ) / 255.0, GetGValue( pHitIterator->Color ) / 255.0, GetBValue( pHitIterator->Color ) / 255.0, 0.8 );
		DrawHit( pHitIterator->Angle, -( ( 2 * mYAxisInfo.Begin + mYAxisInfo.Delta * 5.0 ) / 2.0 - pHitIterator->Height / 1000.0 ) * mYAxisInfo.DigitPerValue, tRadius );

		pHitIterator++;
	}
	
	map< HSInt, SensorInfo >::iterator pSensorIterator = mPositionSensors->begin();
	while ( pSensorIterator != mPositionSensors->end() )
	{
		if ( pSensorIterator->second.Index == mFocusedIndex )
		{
			glColor4f( 0.0f, 0.0f, 1.0f, 0.8 );
		}
		else
		{
			glColor4f( 0.4f, 0.7f, 0.4f, 0.8 );
		}
	
		DrawSensor( pSensorIterator->second.Angle,
					-( ( 2 * mYAxisInfo.Begin + mYAxisInfo.Delta * 5.0 ) / 2.0 - pSensorIterator->second.Height / 1000.0 ) * mYAxisInfo.DigitPerValue,
					tRadius,
					pSensorIterator->second.Index );

		pSensorIterator++;
	}
	
	glPopMatrix();
	
    SwapBuffers( mGLDC );
}

HSVoid CGraphicCylinderSurface::InitGraphic()
{
	mCylinder = gluNewQuadric();
	gluQuadricDrawStyle( mCylinder, GLU_FILL );
	gluQuadricNormals( mCylinder, GLU_SMOOTH );
}

HSVoid CGraphicCylinderSurface::DestroyGraphic()
{	
	gluDeleteQuadric( mCylinder );
}


HSVoid CGraphicCylinderSurface::SetHitRadius( HSInt tRadius )
{
	mHitRadius = tRadius;
	DrawGraphic();
}

HSVoid CGraphicCylinderSurface::SetMaterialHeight( HSDouble tHeight )
{
	mMaterialYLength = tHeight;
}

HSVoid CGraphicCylinderSurface::SetHeight( HSDouble tHeight )
{	
	mYLength = tHeight;		
	
	GetCorrectAxis( mYLength, mYAxisInfo.Begin, mYAxisInfo.Delta, mYAxisInfo.DigitPerValue );
}

HSVoid CGraphicCylinderSurface::GetCorrectAxis( HSDouble tValue, HSDouble &tResBegin, HSDouble &tResDelta, HSDouble &tDigitPerValue )
{
	HSInt tBase = 1;
	if ( tValue < 5.0 )
	{
		tValue *= 1000.0;
		tBase = 1000;
	}

	tValue = ( ( HSInt )( ( tValue + 4.9999 ) / 5.0 ) ) * 5.0;

	tResDelta = tValue / 5.0;
	
	tResBegin = -( HSInt )( tValue / 10 );
	
	if ( tBase != 1 )
	{
		tResDelta /= tBase;
		tResBegin /= tBase;
	}			

	tDigitPerValue = GRAPHIC_RADIUS * 2.0;
	tDigitPerValue /= ( tResDelta * 5.0 );
}