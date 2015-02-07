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
#include "GraphicSphere.h"
#include "GlFont.h"

// CGraphicCylinder dialog

IMPLEMENT_DYNAMIC(CGraphicCylinder, CDialogEx)

CGraphicCylinder::CGraphicCylinder(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGraphicCylinder::IDD, pParent)
{
	mRefreshThread = new CThreadControlEx< CGraphicCylinder >( this, 50, 0 );
	mIsInit = HSFalse;

	mIsBeginDrag = HSFalse;		

	mIsFirstCall = HSTrue;

	mFocusedIndex = -1;

	mParent = dynamic_cast< CCylinderPosition * >( pParent );

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
	delete mRefreshThread;

}

HSVoid CGraphicCylinder::ResizeViewport( CRect &tRect )
{
	wglMakeCurrent( mGLDC, mRC );

	GLfloat tRange = 110.0;
	double tW = tRect.Width();
	double tH = tRect.Height();

	// Prevent a divide by zero
	tH = max( 1, tH );

	// Set Viewport to window dimensions
	glViewport( 0, 0, tW, tH );

	// Reset projection matrix stack
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	// Establish clipping volume (left, right, bottom, top, near, far)
	if ( tW <= tH ) 
	{
		glOrtho( -tRange, tRange, -tRange * tH / tW, tRange * tH / tW, -tRange, tRange );
	}
	else 
	{
		glOrtho( -tRange * tW / tH, tRange * tW / tH, -tRange, tRange, -tRange, tRange );
	}

	// Reset Model view matrix stack
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();			

	//glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
}

HSVoid CGraphicCylinder::DrawString( const HSChar *pTxt, CGLVector3 tScale ) 
{	
	static const HSInt MAX_CHAR = 256;

	if( mIsFirstCall ) 
	{ 
		mFont = CreateFont(
		32,									//< lfHeight 字体高度
		32,										//< lfWidth 字体宽度 
		0,										//< lfEscapement 字体的旋转角度 Angle Of Escapement 
		0,										//< lfOrientation 字体底线的旋转角度Orientation Angle 
		FW_BOLD,								//< lfWeight 字体的重量 
		FALSE,									//< lfItalic 是否使用斜体 
		FALSE,									//< lfUnderline 是否使用下划线 
		FALSE,									//< lfStrikeOut 是否使用删除线 
		ANSI_CHARSET,							//< lfCharSet 设置字符集 
		OUT_TT_PRECIS,							//< lfOutPrecision 输出精度 
		CLIP_DEFAULT_PRECIS,					//< lfClipPrecision 裁剪精度 
		ANTIALIASED_QUALITY,					//< lfQuality 输出质量 
		FF_DONTCARE|DEFAULT_PITCH,				//< lfPitchAndFamily Family And Pitch 
		"Arial");								//< lfFaceName 字体名称

		SelectObject( mGLDC, mFont );

		mIsFirstCall = HSFalse;

		mLists = glGenLists( MAX_CHAR );		

		GLYPHMETRICSFLOAT agmf[256]; 
		wglUseFontOutlines( mGLDC, 0, 256, mLists, 0.0f, 0.1f, WGL_FONT_POLYGONS, agmf ); 

		glListBase( mLists );		
	}
	
	glPushMatrix();		

		glScaled( tScale.X, tScale.Y, tScale.Z );
	
		glCallLists( strlen( pTxt ), GL_UNSIGNED_BYTE, pTxt );

	glPopMatrix();
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
		
	DrawString( "Z", CGLVector3( 10, 10, 1 ) );		

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

HSVoid CGraphicCylinder::DrawAxisFace( CGLVector3 tRotation, HSDouble tAngle, CGLVector3 tTranslate, CGLVector3 tNormal )
{
	HSDouble tValue = GRAPHIC_RADIUS;

	glColor4f( 0.9f, 0.9f, 0.9f, 0.2 );	

	glPushMatrix();						
	glTranslatef( tTranslate.X, tTranslate.Y, tTranslate.Z );
	glRotatef( tAngle, tRotation.X,  tRotation.Y,  tRotation.Z );
	
	glBegin( GL_QUADS );  
		glNormal3f( tNormal.X, tNormal.Y, tNormal.Z );  
		glVertex3f( tValue, tValue, 0 );  
		glVertex3f(-tValue, tValue, 0 );  
		glVertex3f(-tValue,-tValue, 0 );  
		glVertex3f( tValue,-tValue, 0 );  
	glEnd();

	glLineWidth( 1.5 );
	glColor4f( 0.1f, 0.1f, 0.1f, 0.8 );	
	glBegin( GL_LINE_LOOP );
			glVertex3f( tValue, tValue, 0 ); 
			glVertex3f( -tValue, tValue, 0 ); 
			glVertex3f( -tValue,-tValue, 0 ); 
			glVertex3f( tValue,-tValue, 0 ); 
	glEnd();

	glLineWidth( 1.5 );
	glColor4f( 0.7f, 0.7f, 0.7f, 0.3 );	
	HSDouble tDelta = 2 * tValue / 5;
	HSDouble tTempValue = -tValue + tDelta;
	while ( tTempValue < tValue )
	{			
		glBegin( GL_LINE_STRIP );
			glVertex3f( -tValue, tTempValue, 0 ); 
			glVertex3f( tValue, tTempValue, 0 ); 				
		glEnd();

		glBegin( GL_LINE_STRIP );
			glVertex3f( tTempValue, -tValue, 0 ); 
			glVertex3f( tTempValue, tValue, 0 ); 				
		glEnd();

		tTempValue += tDelta;
	}			

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

	glPushMatrix();
	
	glTranslatef( ( ( 2 * mAxisInfo[ 2 ].Begin + mAxisInfo[ 2 ].Delta * 5.0 ) / 2.0 - mMaterialZLength / 2.0 ) * mAxisInfo[ 2 ].DigitPerValue,
				-( ( 2 * mAxisInfo[ 1 ].Begin + mAxisInfo[ 1 ].Delta * 5.0 ) / 2.0 - mMaterialYLength / 2.0 ) * mAxisInfo[ 1 ].DigitPerValue,
				( ( 2 * mAxisInfo[ 0 ].Begin + mAxisInfo[ 0 ].Delta * 5.0 ) / 2.0 - mMaterialXLength / 2.0 ) * mAxisInfo[ 0 ].DigitPerValue );

	glColor4f( 0.5f, 0.1f, 1.0f, 0.5 );	
	DrawSquare( mMaterialXLength * mAxisInfo[ 0 ].DigitPerValue / 2.0, mMaterialYLength * mAxisInfo[ 1 ].DigitPerValue / 2.0, mMaterialZLength * mAxisInfo[ 2 ].DigitPerValue / 2.0 );

	glPopMatrix();	

	list< HitPosition >::iterator pHitIterator = mHitsPosition->begin();
	while ( pHitIterator != mHitsPosition->end() )
	{
		glColor4f( GetRValue( pHitIterator->Color ) / 255.0, GetGValue( pHitIterator->Color ) / 255.0, GetBValue( pHitIterator->Color ) / 255.0, 0.8 );
		DrawHit( ( ( 2 * mAxisInfo[ 2 ].Begin + mAxisInfo[ 2 ].Delta * 5.0 ) / 2.0 - pHitIterator->PosZ / 1000.0 ) * mAxisInfo[ 2 ].DigitPerValue,
				-( ( 2 * mAxisInfo[ 1 ].Begin + mAxisInfo[ 1 ].Delta * 5.0 ) / 2.0 - pHitIterator->PosY / 1000.0 ) * mAxisInfo[ 1 ].DigitPerValue,
				( ( 2 * mAxisInfo[ 0 ].Begin + mAxisInfo[ 0 ].Delta * 5.0 ) / 2.0 - pHitIterator->PosX / 1000.0 ) * mAxisInfo[ 0 ].DigitPerValue );

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
	
		DrawSensor( ( ( 2 * mAxisInfo[ 2 ].Begin + mAxisInfo[ 2 ].Delta * 5.0 ) / 2.0 - pSensorIterator->second.PosZ ) * mAxisInfo[ 2 ].DigitPerValue,
					-( ( 2 * mAxisInfo[ 1 ].Begin + mAxisInfo[ 1 ].Delta * 5.0 ) / 2.0 - pSensorIterator->second.PosY ) * mAxisInfo[ 1 ].DigitPerValue,
					( ( 2 * mAxisInfo[ 0 ].Begin + mAxisInfo[ 0 ].Delta * 5.0 ) / 2.0 - pSensorIterator->second.PosX ) * mAxisInfo[ 0 ].DigitPerValue,
					pSensorIterator->second.Index );

		pSensorIterator++;
	}		

	
	glPopMatrix();
	
    SwapBuffers( mGLDC );
}

HSVoid CGraphicCylinder::InitGraphic()
{
	static PIXELFORMATDESCRIPTOR tPfd = { sizeof(PIXELFORMATDESCRIPTOR),  
		1,  
		PFD_DRAW_TO_WINDOW |  
		PFD_SUPPORT_OPENGL |  
		PFD_DOUBLEBUFFER,  
		PFD_TYPE_RGBA,  
		24 ,  
		0, 0, 0, 0, 0, 0,  
		0,  
		0,  
		0,  
		0, 0, 0, 0,  
		32 ,  
		0,  
		0,  
		PFD_MAIN_PLANE,  
		0,  
		0, 0, 0  
	};  

    int tPixelFormat;     
    if ( !( tPixelFormat = ChoosePixelFormat( mGLDC, &tPfd ) ) )  
    {          
        return;  
    }  

    if ( !SetPixelFormat( mGLDC, tPixelFormat, &tPfd ) )  
    {          
        return;  
    }  

	
    mRC = wglCreateContext( mGLDC );
    wglMakeCurrent( mGLDC, mRC );

    //glutInit(&g_AppParaCount, &g_AppPara);

    glClearColor( 0.94, 0.94, 0.94, 1.0 ); 
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 

	//glShadeModel( GL_FLAT ); 
	glShadeModel( GL_SMOOTH ); 	

	glEnable( GL_ALPHA_TEST );
	//glEnable( GL_DEPTH_TEST );
	glEnable( GL_BLEND );  
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );  
	
	
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

	//glEnable( GL_TEXTURE_2D );
	glEnable( GL_CULL_FACE );
	glFrontFace( GL_CCW );		
	
}

HSBool CGraphicCylinder::ThreadRuning( HSInt tThreadID )
{		
	this->PostMessage( WM_REFRESH_GRAPHIC_CYLINDER, 1, 1 );
	
	return HSTrue;
}

HSVoid CGraphicCylinder::ThreadWillStop( HSInt tThreadID )
{
}

void CGraphicCylinder::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGraphicCylinder, CDialogEx)
	ON_MESSAGE( WM_REFRESH_GRAPHIC_CYLINDER, OnRefreshGraphic )
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CGraphicCylinder message handlers


BOOL CGraphicCylinder::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	mIsInit = HSTrue;

	mGLDC = ::GetDC( m_hWnd );

	InitGraphic();

	CRect tRect;
	this->GetClientRect( tRect );

	ResizeViewport( tRect );

	this->SetTimer( 1, 1, NULL );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CGraphicCylinder::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( !mIsInit )
	{
		return;
	}

	CRect tRect;
	this->GetClientRect( tRect );
		
	ResizeViewport( tRect );
	
	this->Invalidate();
}


void CGraphicCylinder::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here

	mRefreshThread->Stop();

	wglMakeCurrent( NULL, NULL );
    wglDeleteContext( mRC );			

	glDeleteLists( mLists, 1 );
}


void CGraphicCylinder::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialogEx::OnPaint() for painting messages

	DrawGraphic();
}


void CGraphicCylinder::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if ( mIsBeginDrag )
	{
		mIsBeginDrag = HSFalse;
	}
	else
	{
		mParent->OnLButtonUp( nFlags, point );
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CGraphicCylinder::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CRect tRect;
	this->GetClientRect( &tRect );

	CPoint tPoint = point;	

	mParent->FocusWnd();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_DRAG )
	{
		mRefreshThread->Stop();
		if ( !mIsBeginDrag )
		{
			mIsBeginDrag = HSTrue;
			mPrevPoint = point;			
		}
	}
	else if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_STEP_FRAME )
	{
		mRefreshThread->Stop();

		int tDelta = 1;
		if ( tPoint.x < tRect.right / 2 )
		{
			tDelta = -1;
		}

		HorRotate( tDelta, 30 );		
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CGraphicCylinder::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if ( mIsBeginDrag )
	{		
		wglMakeCurrent( mGLDC, mRC );

		HSDouble tAngle = 0;
		CGLVector3 tRotation;
		CGraphicSphere::CalRotateVars( mPrevPoint.x, mPrevPoint.y, point.x, point.y, tAngle, tRotation.X, tRotation.Y, tRotation.Z );

		CGLQuaternion tQ( tRotation, tAngle / 10 );
		mOrientation = tQ * mOrientation;
		mOrientation.Normalize();

		DrawGraphic();

		mPrevPoint = point;
	}
	else
	{
		mParent->OnMouseMove( nFlags, point );
	}
	
	CDialogEx::OnMouseMove(nFlags, point);
}

LRESULT CGraphicCylinder::OnRefreshGraphic( UINT wParam, LPARAM lParam )
{
	HSInt tDirection = ( HSInt )wParam;
	HSInt tAngle = ( HSInt )lParam;
//	HorRotate( tDirection, tAngle );

	return 1;
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

	HSDouble tBigBegin = max( mAxisInfo[ 0 ].Begin, mAxisInfo[ 1 ].Begin );
	HSDouble tMinBegin = min( mAxisInfo[ 0 ].Begin, mAxisInfo[ 1 ].Begin );
	mAxisInfo[ 0 ].Begin = ( tBigBegin < 0.0 ? tBigBegin : tMinBegin );
	mAxisInfo[ 1 ].Begin = mAxisInfo[ 0 ].Begin;
}

HSVoid CGraphicCylinder::GetCorrectAxis( HSDouble tValue, HSDouble &tResBegin, HSDouble &tResDelta, HSDouble &tDigitPerValue )
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

HSVoid CGraphicCylinder::HorRotate( HSInt tDirection, HSDouble tAngle )
{
	wglMakeCurrent( mGLDC, mRC );

	CRect tRect;
	this->GetClientRect( &tRect );
	HSDouble tCenterY = tRect.Height() / 2;
	HSDouble tCenterX = tRect.Width() / 2;

	HSDouble tStartX = tCenterX + 10 * tDirection;
	HSDouble tEndX = tCenterX;

	HSDouble tTmpAngle = 90;
	CGLVector3 tRotation;
	CGraphicSphere::CalRotateVars( tStartX, tCenterY, tEndX, tCenterY, tTmpAngle, tRotation.X, tRotation.Y, tRotation.Z );

	CGLQuaternion tQ( tRotation, tAngle * 3.14159265 / 180.0 );
	mOrientation = tQ * mOrientation;	
	mOrientation.Normalize();

	DrawGraphic();
}

void CGraphicCylinder::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	KillTimer( nIDEvent );

	this->Invalidate();	

	CDialogEx::OnTimer(nIDEvent);
}
