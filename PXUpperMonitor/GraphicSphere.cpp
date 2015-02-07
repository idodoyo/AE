// GraphicSphere.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GraphicSphere.h"
#include "afxdialogex.h"
#include "HSLogProtocol.h"
#include "resource.h"
#include "GLDataTypes.h"
#include "MainFrm.h"
#include "SpherePosition.h"


// CGraphicSphere dialog

CGraphicSphere::CGraphicSphere(CWnd* pParent /*=NULL*/)	
{	
	mFocusedIndex = -1;

	mParent = dynamic_cast< CSpherePosition * >( pParent );

	m3DParent = mParent;

	mHitRadius = 3.0;

	mInitOrientation = CGLQuaternion();
}

CGraphicSphere::~CGraphicSphere()
{
}

HSVoid CGraphicSphere::CoordWithAngles( HSDouble tDirectionAngle, HSDouble tUpAngle, HSDouble tRadius, CGLVector3 &tCoord )
{
	tCoord.X = tRadius * sin( tUpAngle ) * cos( tDirectionAngle );
	tCoord.Y = tRadius * sin( tUpAngle ) * sin( tDirectionAngle );
	tCoord.Z = tRadius * cos( tDirectionAngle );
}

HSVoid CGraphicSphere::DrawHit( HSDouble tDirectionAngle, HSDouble tUpAngle, HSDouble tRadius )
{   
	glPushMatrix();	   
		
	glRotatef( tDirectionAngle, 0.0, 1.0, 0 );
	glRotatef( tUpAngle - 90, 1.0, 0.0, 0 );
	glTranslatef( 0.0, 0.0, tRadius + 0.6 );

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

HSVoid CGraphicSphere::DrawSensor( HSDouble tDirectionAngle, HSDouble tUpAngle, HSDouble tRadius, HSInt tIndex )
{
	glPushMatrix();	   
		
	glRotatef( tDirectionAngle, 0.0, 1.0, 0 );
	glRotatef( tUpAngle - 90, 1.0, 0.0, 0 );
	glTranslatef( 0.0, 0.0, tRadius + 0.6 );		
	
	gluCylinder( mCylinder, 1.5, 1.5, 1.5, 10, 10 );
	
	glColor4f( 0.1f, 0.1f, 0.1f, 0.8 );	
	CString tStrAngle;
	tStrAngle.Format( "S%d", tIndex + 1 );	
	
	glPushMatrix();
	//DrawString( ( LPCSTR )tStrAngle, CGLVector3( -2, 0, 1.5 ) );

	glTranslatef( 0.0, 0.0, 1.0 );		
	DrawString( ( LPCSTR )tStrAngle, CGLVector3( 5, 5, 1 ) );

	glPopMatrix();

	glPopMatrix();
}

HSVoid CGraphicSphere::DrawDirectionAngle( HSDouble tDirectionAngle, HSDouble tRadius )
{
	glPushMatrix();

	glRotatef( tDirectionAngle, 0.0, 1.0, 0 );	
	glTranslatef( 0.0, 0.0, tRadius + 5 );

	CString tStrAngle;
	tStrAngle.Format( "%.0f", tDirectionAngle );

	//DrawString( ( LPCSTR )tStrAngle, CGLVector3( 0, 0, 0 ) );	

	DrawString( ( LPCSTR )tStrAngle, CGLVector3( 7, 7, 1 ) );

	glPopMatrix();
}

HSVoid CGraphicSphere::DrawUpAngle( HSDouble tDirectionAngle, HSDouble tUpAngle, CGLVector3 tAdjustTrans, CGLVector3 tAdjustRotation )
{
	glPushMatrix();

	glRotatef( tDirectionAngle, 0.0, 1.0, 0 );
	glRotatef( tUpAngle - 90, 1.0, 0.0, 0 );	
	//glTranslatef( 0.0, -2.0, tRadius + 2 );
	glTranslatef( tAdjustTrans.X, tAdjustTrans.Y, tAdjustTrans.Z );

	CString tStrAngle;
	tStrAngle.Format( "%.0f", tUpAngle );

	//DrawString( ( LPCSTR )tStrAngle, CGLVector3( 0, 0, 0 ) );

	glPushMatrix();		
	glRotatef( tAdjustRotation.X, 1.0, 0.0, 0 );		
	glRotatef( tAdjustRotation.Y, 0.0, 1.0, 0 );
	glRotatef( tAdjustRotation.Z, 0.0, 0.0, 1.0 );

	DrawString( ( LPCSTR )tStrAngle, CGLVector3( 7, 7, 1 ) );

	glPopMatrix();

	glPopMatrix();
}

HSVoid CGraphicSphere::DrawGraphic()
{
	wglMakeCurrent( mGLDC, mRC );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	HSDouble tRadius = 80;
	
	HSDouble tUpAngleDelta = 180 / 6;
	glColor4f( 0.1f, 0.1f, 0.1f, 1.0 );	
	for ( HSInt j = 1; j < 6; j++ )
	{
		DrawUpAngle( 270, j * tUpAngleDelta, CGLVector3( 0, -2.0, tRadius + 15 ), CGLVector3( 0, 90, 0 ) );
	}
	
	glPushMatrix();

	HSDouble tAngle = 0;
	CGLVector3 tRotation;
	mOrientation.GetAxisAngle( tRotation, tAngle );	

	glRotatef( tAngle * 180 / 3.14159265, tRotation.X,  tRotation.Y,  tRotation.Z );	
	

	glDisable( GL_TEXTURE_2D );	
	glColor4f( 0.9f, 0.9f, 0.9f, 0.5 );	
	DrawSphere( tRadius );


	glColor4f( 0.5f, 0.1f, 0.5f, 0.5 );
	HSDouble tDirectionAngles[] = { 0, 90, 180, 270 };
	for ( HSUInt i = 0; i < sizeof( tDirectionAngles ) / sizeof( HSDouble ); i++ )	
	{
		DrawDirectionAngle( tDirectionAngles[ i ], tRadius );		
	}
	
	glColor4f( 0.1f, 0.1f, 0.1f, 1.0 );
	DrawUpAngle( 0, 0 * tUpAngleDelta, CGLVector3( -2, -2.0, tRadius + 8 ), CGLVector3( 90, 0, 0 ) );
	DrawUpAngle( 0, 6 * tUpAngleDelta, CGLVector3( -5.0, -2.0, tRadius + 12 ), CGLVector3( 270, 0, 0 ) );
	
	list< HitPosition >::iterator pHitIterator = mHitsPosition->begin();
	while ( pHitIterator != mHitsPosition->end() )
	{		
		glColor4f( GetRValue( pHitIterator->Color ) / 255.0, GetGValue( pHitIterator->Color ) / 255.0, GetBValue( pHitIterator->Color ) / 255.0, 1.0 );
		DrawHit( pHitIterator->AngleDirection, pHitIterator->AngleUp, tRadius );
		pHitIterator++;
	}	

	map< HSInt, SensorInfo >::iterator pSensorIterator = mPositionSensors->begin();
	while ( pSensorIterator != mPositionSensors->end() )
	{
		if ( pSensorIterator->second.Index == mFocusedIndex )
		{
			glColor4f( 0.0f, 0.0f, 1.0f, 1.0 );
		}
		else
		{
			glColor4f( 0.4f, 0.7f, 0.4f, 1.0 );
		}

		DrawSensor( pSensorIterator->second.AngleDirection, pSensorIterator->second.AngleUp, tRadius, pSensorIterator->second.Index );
		pSensorIterator++;
	}

	
	glPopMatrix();
	
    SwapBuffers( mGLDC );
}

HSVoid CGraphicSphere::DrawCircle( HSDouble tRadius, HSString tStrAxis )
{
	HSInt N = 600;
	HSDouble PI = 3.14159265;
	glBegin( GL_LINE_LOOP );   
	for ( HSInt i = 0; i < N; i++ )     
	{
		if ( tStrAxis == "XY" )
		{
			glVertex3f( tRadius * cos( 2 * PI / N * i ), tRadius * sin( 2 * PI / N * i ), 0.0 );  
		}
		else if ( tStrAxis == "XZ" )
		{
			glVertex3f( tRadius * cos( 2 * PI / N * i ), 0.0,  tRadius * sin( 2 * PI / N * i ) );  
		}
		else
		{
			glVertex3f( 0.0, tRadius * cos( 2 * PI / N * i ), tRadius * sin( 2 * PI / N * i ) );  
		}
	}

	glEnd();    
}

HSVoid CGraphicSphere::DrawSphere( HSDouble tRadius )
{
	glPushMatrix();

	glRotatef( 90, 1.0, 0.0, 0 );

	glLineWidth ( 2.0 );
	gluQuadricDrawStyle( mSphere, GLU_FILL );
	
	gluSphere( mSphere, tRadius, 60, 60 );

	glPopMatrix();
	
	glLineWidth ( 1.0 );

	HSDouble tCircleDelta = 0.4;
	HSDouble PI = 3.14159265;
	HSDouble tUpAngleDelta = 180 / 36;	
	for ( HSInt j = 1; j < 36; j++ )
	{
		HSDouble tHight = tRadius * cos( PI / 180 * j * tUpAngleDelta );
		HSDouble tWidth = tRadius * sin( PI / 180 * j * tUpAngleDelta );

		if ( j % 6 == 0 )
		{
			glColor4f( 0.5f, 0.1f, 0.5f, 1.0 );	
		}
		else
		{
			glColor4f( 0.6f, 0.6f, 0.6f, 1.0 );
		}

		glPushMatrix();	

		glTranslatef( 0.0, tHight, 0.0 );
		DrawCircle( tWidth + tCircleDelta, "XZ" );

		glPopMatrix();		
	}	

	HSDouble tDirectionAngleDelta = 360 / 36;	
	for ( HSInt j = 0; j < 18; j++ )
	{
		if ( j % 9 == 0 )
		{
			glColor4f( 0.5f, 0.1f, 0.5f, 1.0 );	
		}
		else
		{
			glColor4f( 0.6f, 0.6f, 0.6f, 1.0 );
		}

		glPushMatrix();	

		glRotatef( j * tDirectionAngleDelta, 0.0, 1.0, 0 );
		DrawCircle( tRadius + tCircleDelta, "XY" );

		glPopMatrix();
	}		
}

HSVoid CGraphicSphere::InitGraphic()
{		
	glEnable( GL_DEPTH_TEST );
	
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat lightAmbient[] = { 0.8, 0.8, 0.8, 1.0};
	GLfloat lightDiffuse[] = { 0.2, 0.2, 0.2, 1.0};
	GLfloat lightSpecular[] = { 0.4, 0.4, 0.4, 1.0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);	
	
	glEnable( GL_COLOR_MATERIAL );

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_CULL_FACE );
	glFrontFace( GL_CCW );

	mSphere = gluNewQuadric();
	//gluQuadricDrawStyle( mSphere, GLU_FILL );
	gluQuadricDrawStyle( mSphere, GLU_LINE );
	//gluQuadricDrawStyle( mSphere, GL_POINT );
	gluQuadricNormals( mSphere, GLU_SMOOTH );
	//gluQuadricNormals( mSphere, GLU_NONE );
	gluQuadricTexture( mSphere, GLU_TRUE );	

	mCylinder = gluNewQuadric();
	gluQuadricDrawStyle( mCylinder, GLU_LINE );	
	gluQuadricNormals( mCylinder, GLU_SMOOTH );
	gluQuadricTexture( mCylinder, GLU_TRUE );	

}

HSVoid CGraphicSphere::DestroyGraphic()
{
	gluDeleteQuadric( mSphere );
	gluDeleteQuadric( mCylinder );
}

HSVoid CGraphicSphere::SetHitRadius( HSInt tRadius )
{
	mHitRadius = tRadius;
	DrawGraphic();
}