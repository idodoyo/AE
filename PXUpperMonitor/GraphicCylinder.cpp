// GraphicCylinder.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GraphicCylinder.h"
#include "afxdialogex.h"

#include "HSLogProtocol.h"
#include "resource.h"
#include "GLDataTypes.h"
#include "MainFrm.h"
#include "CylinderPosition.h"
#include "C3DCrackPosition.h"
#include "GraphicSphere.h"
#include "GlFont.h"

// CGraphicCylinder dialog

CGraphicCylinder::CGraphicCylinder(CWnd* pParent /*=NULL*/, HSBool tIs3DCrack )
{	
	mFocusedIndex = -1;

	m3DCrackParent = NULL;
	mParent = NULL;

	mIs3DCrackParent = tIs3DCrack;
	if ( mIs3DCrackParent )
	{
		m3DCrackParent = dynamic_cast< C3DCrackPosition * >( pParent );
		m3DParent = m3DCrackParent;
	}
	else
	{
		mParent = dynamic_cast< CCylinderPosition * >( pParent );
		m3DParent = mParent;
	}

	mHitRadius = 2.0;
	mXLength = 60.0;
	mYLength = 50.0;
	mZLength = 40.0;

	mMaterialXLength = 40.0;
	mMaterialYLength = 35.0;
	mMaterialZLength = 30.0;

	mInitOrientation = CGLQuaternion( CGLVector3( 0.1, -1, -0.3 ), 155 * 3.14159265 / 180 );	
	mInitOrientation.Normalize();
	
	mOrientation = mInitOrientation;
}

CGraphicCylinder::~CGraphicCylinder()
{
}

HSVoid CGraphicCylinder::DrawHit( HSDouble tPosX, HSDouble tPosY, HSDouble tPosZ )
{   
	glPushMatrix();
	glTranslatef( tPosX ,tPosY, tPosZ );

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

HSVoid CGraphicCylinder::DrawSensor( HSDouble tPosX, HSDouble tPosY, HSDouble tPosZ, HSInt tIndex )
{
	glPushMatrix();
	glTranslatef( tPosX , tPosY, tPosZ );
		
//	glRotatef( tDirectionAngle, 0.0, 1.0, 0 );

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

	glRotatef( 135.0, 0.0, 1.0, 0 );

	CString tStrAngle;
	tStrAngle.Format( "S%d", tIndex + 1 );

	DrawString( ( LPCSTR )tStrAngle, CGLVector3( 4, 4, 1 ) );
	glPopMatrix();

	glPopMatrix();
}

HSVoid CGraphicCylinder::DrawXAxis( CGLVector3 tPos, HSDouble tXDelta, HSInt tCount, HSDouble tXValueBegin, HSDouble tXValueDelta, CGLVector3 tCenterPos )
{	
	CString tStr;
	for ( HSInt i = 0; i <= tCount; i++ )
	{
		glPushMatrix();
		
		glTranslatef( tPos.X, tPos.Y, tPos.Z );
		glRotatef( 90, 0.0, 1.0, 0.0 );	
		glRotatef( 90, -1.0, 0.0, 0.0 );		
		glRotatef( 90, 0.0, 0.0, 1.0 );			
		
		tStr.Format( "%5.01f", tXValueBegin );

		DrawString( ( LPCSTR )tStr );

		glPopMatrix();

		tXValueBegin += tXValueDelta;
		tPos.Z += tXDelta;
	}

	glPushMatrix();
		
		glTranslatef( tCenterPos.X, tCenterPos.Y, tCenterPos.Z );		
		glRotatef( 90, 1.0, 0.0, 0.0 );			
		
		DrawString( "X", CGLVector3( 10, 10, 1 ) );

	glPopMatrix();
}

HSVoid CGraphicCylinder::DrawYAxis( CGLVector3 tPos, HSDouble tYDelta, HSInt tCount, HSDouble tYValueBegin, HSDouble tYValueDelta, CGLVector3 tCenterPos )
{
	if ( m3DCrackParent == NULL )
	{		
		CString tStr;
		for ( HSInt i = 0; i < tCount; i++ )
		{
			tPos.Y += tYDelta;
			tYValueBegin += tYValueDelta;

			glPushMatrix();

			glTranslatef( tPos.X, tPos.Y, tPos.Z );
			glRotatef( 180, 0.0, 1.0, 0.0 );				

			tStr.Format( "%5.01f", tYValueBegin );
			DrawString( ( LPCSTR )tStr );

			glPopMatrix();		
		}

		glPushMatrix();

		glTranslatef( tCenterPos.X, tCenterPos.Y, tCenterPos.Z );				

		DrawString( "Y", CGLVector3( 10, 10, 1 ) );

		glPopMatrix();
	}
	else
	{
		glPushMatrix();

		glTranslatef( tCenterPos.X, tCenterPos.Y, tCenterPos.Z );	
		glRotatef( 180, 0.0, 1.0, 0.0 );		

		DrawString( "Z", CGLVector3( 10, 10, 1 ) );

		glPopMatrix();
	}
}

HSVoid CGraphicCylinder::DrawZAxis( CGLVector3 tPos, HSDouble tZDelta, HSInt tCount, HSDouble tZValueBegin, HSDouble tZValueDelta, CGLVector3 tCenterPos )
{
	CString tStr;
	for ( HSInt i = 0; i <= tCount; i++ )
	{
		glPushMatrix();
		
		glTranslatef( tPos.X, tPos.Y, tPos.Z );
		glRotatef( 90, 0.0, 0.0, 1.0 );		
		glRotatef( 90, 0.0, 1.0, 0.0 );		
	
		tStr.Format( "%5.01f", tZValueBegin );
		DrawString( ( LPCSTR )tStr );

		glPopMatrix();

		tZValueBegin += tZValueDelta;
		tPos.X += tZDelta;
	}

	glPushMatrix();

	glTranslatef( tCenterPos.X, tCenterPos.Y, tCenterPos.Z );			
	glRotatef( 90, 0.0, 0.0, 1.0 );		
	glRotatef( 90, 0.0, 1.0, 0.0 );	
		
	DrawString( m3DCrackParent == NULL ? "Z" : "Y", CGLVector3( 10, 10, 1 ) );		

	glPopMatrix();
}

HSVoid CGraphicCylinder::DrawSquare( HSDouble tLength, HSDouble tHeight, HSDouble tWidth )
{	
	glBegin( GL_QUADS );  
		glNormal3f( 0.0F, 0.0F, 1.0F);  
		glVertex3f( tWidth, tHeight, tLength);  
		glVertex3f(-tWidth, tHeight, tLength);  
		glVertex3f(-tWidth,-tHeight, tLength);  
		glVertex3f( tWidth,-tHeight, tLength);  
		//1----------------------------   
		glNormal3f( 0.0F, 0.0F,-1.0F);  
		glVertex3f(-tWidth,-tHeight,-tLength);  
		glVertex3f(-tWidth, tHeight,-tLength);  
		glVertex3f( tWidth, tHeight,-tLength);  
		glVertex3f( tWidth,-tHeight,-tLength);  
		//2----------------------------   
		glNormal3f( 0.0F, 1.0F, 0.0F);  
		glVertex3f( tWidth, tHeight, tLength);  
		glVertex3f( tWidth, tHeight,-tLength);  
		glVertex3f(-tWidth, tHeight,-tLength);  
		glVertex3f(-tWidth, tHeight, tLength);  
		//3----------------------------   
		glNormal3f( 0.0F,-1.0F, 0.0F);  
		glVertex3f(-tWidth,-tHeight,-tLength);  
		glVertex3f( tWidth,-tHeight,-tLength);  
		glVertex3f( tWidth,-tHeight, tLength);  
		glVertex3f(-tWidth,-tHeight, tLength);  
		//4----------------------------   
		glNormal3f( 1.0F, 0.0F, 0.0F);  
		glVertex3f( tWidth, tHeight, tLength);  
		glVertex3f( tWidth,-tHeight, tLength);  
		glVertex3f( tWidth,-tHeight,-tLength);
		glVertex3f( tWidth, tHeight,-tLength);
		//5----------------------------   
		glNormal3f(-1.0F, 0.0F, 0.0F);  
		glVertex3f(-tWidth,-tHeight,-tLength);  
		glVertex3f(-tWidth,-tHeight, tLength);  
		glVertex3f(-tWidth, tHeight, tLength);  
		glVertex3f(-tWidth, tHeight,-tLength);
		//6----------------------------*/  

    glEnd();  	

	glLineWidth( 1.0 );
	glColor4f( 0.2f, 0.2f, 0.2f, 0.8 );	
	glBegin( GL_LINE_LOOP );		
		glVertex3f( tWidth, tHeight, tLength ); 
		glVertex3f( -tWidth, tHeight, tLength ); 
		glVertex3f( -tWidth,-tHeight, tLength ); 
		glVertex3f( tWidth,-tHeight, tLength ); 
	glEnd();

	glBegin( GL_LINE_LOOP );
		glVertex3f( tWidth, tHeight, -tLength ); 
		glVertex3f( -tWidth, tHeight, -tLength ); 
		glVertex3f( -tWidth,-tHeight, -tLength ); 
		glVertex3f( tWidth,-tHeight, -tLength ); 
	glEnd();

	glBegin( GL_LINE_STRIP );
		glVertex3f( -tWidth, -tHeight, tLength ); 
		glVertex3f( -tWidth, -tHeight, -tLength ); 			
	glEnd();

	glBegin( GL_LINE_STRIP );	
		glVertex3f( -tWidth, tHeight, tLength ); 
		glVertex3f( -tWidth, tHeight, -tLength ); 			
	glEnd();

	glBegin( GL_LINE_STRIP );
		glVertex3f( tWidth, tHeight, tLength ); 
		glVertex3f( tWidth, tHeight, -tLength ); 			
	glEnd();

	glBegin( GL_LINE_STRIP );
		glVertex3f( tWidth, -tHeight, tLength ); 
		glVertex3f( tWidth, -tHeight, -tLength ); 			
	glEnd();
}

HSVoid CGraphicCylinder::DrawArrow()
{
	glColor4f( 0.5f, 0.5f, 0.5f, 0.1 );	
	glPushMatrix();
		glTranslatef( 0, -60, 0 );

		glPushMatrix();
			glTranslatef( 0, 0, -15 );
			gluCylinder( mCylinder, 0.1, 0.1, 30, 20, 20 );
		glPopMatrix();

		glColor4f( 0.4f, 0.6f, 0.4f, 0.6 );		
		glPushMatrix();			
			glTranslatef( 0, 0, -16 );
			gluCylinder( mCylinder, 0.01, 1.0, 3, 20, 10 );
		glPopMatrix();

		glPushMatrix();			
			glTranslatef( 0, 0, 12 );
			gluCylinder( mCylinder, 1.0, 0.01, 3, 20, 10 );
		glPopMatrix();

		glColor4f( 0.5f, 0.8f, 0.5f, 0.8 );	
		glPushMatrix();				
			glTranslatef( 1.75, 0, -18 );
			glRotatef( 90, 0.0, 0.0, 1.0 );
			glRotatef( 90, 0.0, 1.0, 0.0 );
			DrawString( "E", CGLVector3( 7, 7, 1 ) );	
		glPopMatrix();

		glPushMatrix();
			glTranslatef( 1.75, 0, 23 );
			glRotatef( 90, 0.0, 0.0, 1.0 );
			glRotatef( 90, 0.0, 1.0, 0.0 );
			DrawString( "W", CGLVector3( 7, 7, 1 ) );
		glPopMatrix();

		glColor4f( 0.5f, 0.5f, 0.5f, 0.1 );	
		glPushMatrix();	
			glTranslatef( -15, 0, 0 );
			glRotatef( 90, 0.0, 1.0, 0.0 );
			gluCylinder( mCylinder, 0.1, 0.1, 30, 20, 20 );				
		glPopMatrix();

		glColor4f( 0.4f, 0.6f, 0.4f, 0.6 );		
		glPushMatrix();			
			glTranslatef( -15, 0, 0 );
			glRotatef( 90, 0.0, 1.0, 0.0 );
			gluCylinder( mCylinder, 0.01, 1.0, 3, 20, 10 );
		glPopMatrix();

		glPushMatrix();			
			glTranslatef( 12, 0, 0 );
			glRotatef( 90, 0.0, 1.0, 0.0 );
			gluCylinder( mCylinder, 1.0, 0.01, 3, 20, 10 );
		glPopMatrix();

		glColor4f( 0.5f, 0.8f, 0.5f, 0.8 );	
		glPushMatrix();		
			glTranslatef( -18, 0, 1.8 );			
			glRotatef( 90, 0.0, 0.0, 1.0 );
			glRotatef( 90, 0.0, 1.0, 0.0 );
			DrawString( "N", CGLVector3( 7, 7, 1 ) );		
		glPopMatrix();

		glPushMatrix();	
			glTranslatef( 22, 0, 1.8 );			
			glRotatef( 90, 0.0, 0.0, 1.0 );
			glRotatef( 90, 0.0, 1.0, 0.0 );
			DrawString( "S", CGLVector3( 7, 7, 1 ) );
		glPopMatrix();	

	glPopMatrix();	
}

HSVoid CGraphicCylinder::DrawGraphic()
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

	DrawXAxis( CGLVector3( GRAPHIC_RADIUS + 13, -GRAPHIC_RADIUS, GRAPHIC_RADIUS - 1.5 ), -GRAPHIC_RADIUS / 2.5, 5, mAxisInfo[ 0 ].Begin, mAxisInfo[ 0 ].Delta, CGLVector3( GRAPHIC_RADIUS + 15, -GRAPHIC_RADIUS, 0 ) );
	DrawYAxis( CGLVector3( GRAPHIC_RADIUS + 13, -GRAPHIC_RADIUS - 1.5, GRAPHIC_RADIUS ), GRAPHIC_RADIUS / 2.5, 5, mAxisInfo[ 1 ].Begin, mAxisInfo[ 1 ].Delta,  CGLVector3( GRAPHIC_RADIUS + 15, 0, GRAPHIC_RADIUS ) );
	DrawZAxis( CGLVector3( GRAPHIC_RADIUS + 1.5, -GRAPHIC_RADIUS, -GRAPHIC_RADIUS - 3 ), -GRAPHIC_RADIUS / 2.5, 5, mAxisInfo[ 2 ].Begin, mAxisInfo[ 2 ].Delta,  CGLVector3( 0, -GRAPHIC_RADIUS, -GRAPHIC_RADIUS - 20 ) );

	HSDouble tPosXOffset = ( 2 * mAxisInfo[ 2 ].Begin + mAxisInfo[ 2 ].Delta * 5.0 ) / 2.0;
	HSDouble tPosYOffset = ( 2 * mAxisInfo[ 1 ].Begin + mAxisInfo[ 1 ].Delta * 5.0 ) / 2.0;
	HSDouble tPosZOffset = ( 2 * mAxisInfo[ 0 ].Begin + mAxisInfo[ 0 ].Delta * 5.0 ) / 2.0;

	glPushMatrix();
	
	glTranslatef( ( tPosXOffset - mMaterialZLength / 2.0 ) * mAxisInfo[ 2 ].DigitPerValue,
				-( tPosYOffset - mMaterialYLength / 2.0 ) * mAxisInfo[ 1 ].DigitPerValue,
				( tPosZOffset - mMaterialXLength / 2.0 ) * mAxisInfo[ 0 ].DigitPerValue );

	if ( m3DCrackParent != NULL )
	{
		glTranslatef( mMaterialZLength / 2.0 * mAxisInfo[ 2 ].DigitPerValue, -mMaterialYLength / 2.0 * mAxisInfo[ 1 ].DigitPerValue, mMaterialXLength / 2.0 * mAxisInfo[ 0 ].DigitPerValue );

		glColor4f( 0.1f, 0.8f, 0.2f, 0.6 );	
		glPushMatrix();
		glTranslatef( 0, mMaterialYLength / 2.0 * mAxisInfo[ 1 ].DigitPerValue, 0 );
		glRotatef( 90, 1.0, 0.0, 0.0 );
		gluCylinder( mCylinder, 2, 2, mMaterialYLength * mAxisInfo[ 1 ].DigitPerValue, 40, 20 );	
		glPopMatrix();	

		DrawArrow();
	}

	glColor4f( 0.5f, 0.1f, 1.0f, 0.5 );	
	DrawSquare( mMaterialXLength * mAxisInfo[ 0 ].DigitPerValue / 2.0, mMaterialYLength * mAxisInfo[ 1 ].DigitPerValue / 2.0, mMaterialZLength * mAxisInfo[ 2 ].DigitPerValue / 2.0 );
	
	glPopMatrix();	

	list< HitPosition >::iterator pHitIterator = mHitsPosition->begin();
	while ( pHitIterator != mHitsPosition->end() )
	{
		glColor4f( GetRValue( pHitIterator->Color ) / 255.0, GetGValue( pHitIterator->Color ) / 255.0, GetBValue( pHitIterator->Color ) / 255.0, 0.8 );
		DrawHit( ( tPosXOffset - pHitIterator->PosZ ) * mAxisInfo[ 2 ].DigitPerValue,
				-( tPosYOffset - pHitIterator->PosY ) * mAxisInfo[ 1 ].DigitPerValue,
				( tPosZOffset - pHitIterator->PosX ) * mAxisInfo[ 0 ].DigitPerValue );

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
	
		DrawSensor( ( tPosXOffset - pSensorIterator->second.PosZ ) * mAxisInfo[ 2 ].DigitPerValue,
					-( tPosYOffset - pSensorIterator->second.PosY ) * mAxisInfo[ 1 ].DigitPerValue,
					( tPosZOffset - pSensorIterator->second.PosX ) * mAxisInfo[ 0 ].DigitPerValue,
					pSensorIterator->second.Index );

		pSensorIterator++;
	}

	
	glPopMatrix();
	
    SwapBuffers( mGLDC );
}

HSVoid CGraphicCylinder::InitGraphic()
{	
	//glEnable( GL_TEXTURE_2D );
	glEnable( GL_CULL_FACE );
	//glFrontFace( GL_CCW );		

	mCylinder = gluNewQuadric();
	gluQuadricDrawStyle( mCylinder, GLU_LINE );	
	//gluQuadricNormals( mCylinder, GLU_SMOOTH );
	//gluQuadricTexture( mCylinder, GLU_FALSE );	
	
}

HSVoid CGraphicCylinder::DestroyGraphic()
{	
	gluDeleteQuadric( mCylinder );	
}


HSVoid CGraphicCylinder::SetHitRadius( HSInt tRadius )
{
	mHitRadius = tRadius;
	DrawGraphic();
}

HSVoid CGraphicCylinder::SetMaterialXYZ( HSDouble tX, HSDouble tY, HSDouble tZ )
{
	mMaterialXLength = tX;
	mMaterialYLength = tY;
	mMaterialZLength = tZ;
}

HSVoid CGraphicCylinder::SetXYZ( HSDouble tX, HSDouble tY, HSDouble tZ )
{
	mXLength = tX;
	mYLength = tY;
	mZLength = tZ;
		
	GetCorrectAxis( mXLength, mAxisInfo[ 0 ].Begin, mAxisInfo[ 0 ].Delta, mAxisInfo[ 0 ].DigitPerValue );
	GetCorrectAxis( mYLength, mAxisInfo[ 1 ].Begin, mAxisInfo[ 1 ].Delta, mAxisInfo[ 1 ].DigitPerValue );
	GetCorrectAxis( mZLength, mAxisInfo[ 2 ].Begin, mAxisInfo[ 2 ].Delta, mAxisInfo[ 2 ].DigitPerValue );

	if ( m3DCrackParent == NULL )
	{
		HSDouble tBigBegin = max( mAxisInfo[ 0 ].Begin, mAxisInfo[ 1 ].Begin );
		HSDouble tMinBegin = min( mAxisInfo[ 0 ].Begin, mAxisInfo[ 1 ].Begin );
		mAxisInfo[ 0 ].Begin = ( tBigBegin < 0.0 ? tBigBegin : tMinBegin );
		mAxisInfo[ 1 ].Begin = mAxisInfo[ 0 ].Begin;
	}
}

HSVoid CGraphicCylinder::GetCorrectAxis( HSDouble tValue, HSDouble &tResBegin, HSDouble &tResDelta, HSDouble &tDigitPerValue )
{
	if ( m3DCrackParent )
	{
		tResDelta = tValue / 5.0;	
		tResBegin = -tValue / 2;
	}
	else
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
	}

	tDigitPerValue = GRAPHIC_RADIUS * 2.0;
	tDigitPerValue /= ( tResDelta * 5.0 );
}
