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

IMPLEMENT_DYNAMIC(CGraphicSphere, CDialogEx)

CGraphicSphere::CGraphicSphere(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGraphicSphere::IDD, pParent)
{
	mRefreshThread = new CThreadControlEx< CGraphicSphere >( this, 50, 0 );
	mIsInit = HSFalse;
	
	mTextureBits = NULL;

	mIsBeginDrag = HSFalse;		

	mIsFirstCall = HSTrue;

	mFocusedIndex = -1;

	mParent = dynamic_cast< CSpherePosition * >( pParent );

	mHitRadius = 3.0;
}

CGraphicSphere::~CGraphicSphere()
{
	delete mRefreshThread;

	if ( mTextureBits != NULL )
	{
		delete[] mTextureBits;
	}
}

void CGraphicSphere::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGraphicSphere, CDialogEx)
	ON_MESSAGE( WM_REFRESH_GRAPHIC, OnRefreshGraphic )
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()	
	ON_WM_PAINT()	
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CGraphicSphere message handlers

HSVoid CGraphicSphere::CoordWithAngles( HSDouble tDirectionAngle, HSDouble tUpAngle, HSDouble tRadius, CGLVector3 &tCoord )
{
	tCoord.X = tRadius * sin( tUpAngle ) * cos( tDirectionAngle );
	tCoord.Y = tRadius * sin( tUpAngle ) * sin( tDirectionAngle );
	tCoord.Z = tRadius * cos( tDirectionAngle );
}

HSVoid CGraphicSphere::CalRotateVars( HSDouble tStartX, HSDouble tStartY, HSDouble tEndX, HSDouble tEndY, HSDouble &tResAngle, HSDouble &tResVX, HSDouble &tResVY, HSDouble &tResVZ )
{
	const HSDouble PI = 3.14159265;

	GLint tViewport[ 4 ];
	glGetIntegerv( GL_VIEWPORT, tViewport );	

	// 对鼠标获取的点进行归一化处理下（线性变换）
	HSDouble tSx = tStartX * ( 2.0 / ( tViewport[ 2 ] + 1 ) ) - 1.0;
	HSDouble tSy = -1.0 * ( tStartY * ( 2.0 / ( tViewport[ 3 ] + 1 ) ) - 1.0 );
	
	HSDouble tEx = tEndX * ( 2.0 / ( tViewport[ 2 ] + 1 ) ) - 1.0;
	HSDouble tEy = -1.0 * ( tEndY * ( 2.0 / ( tViewport[ 3 ] + 1 ) ) - 1.0 );

	HSDouble tS_vx, tS_vy, tS_vz, tE_vx, tE_vy, tE_vz;
	Map2Sphere( tSx, tSy, tS_vx, tS_vy, tS_vz );
	Map2Sphere( tEx, tEy, tE_vx, tE_vy, tE_vz );

	CGLVector3 tS_vec( tS_vx, tS_vy, tS_vz );
	CGLVector3 tE_vec( tE_vx, tE_vy, tE_vz );

	if( !tS_vec.IsZero() )
	{
		tS_vec.Normalize();
	}

	if ( !tE_vec.IsZero() )
	{
		tE_vec.Normalize();
	}
    
	CGLVector3 tR_vec = tS_vec ^ tE_vec;

	if ( !tR_vec.IsZero() )
	{
		tR_vec.Normalize();
	}

	HSDouble tAngle = acos( tS_vec * tE_vec );

	tResAngle = tAngle * 180.0 / PI;
    tResVX = tR_vec.X;
	tResVY = tR_vec.Y;
	tResVZ = tR_vec.Z;
}

HSVoid CGraphicSphere::Map2Sphere( HSDouble tPx, HSDouble tPy, HSDouble &tVx, HSDouble &tVy, HSDouble &tVz )
{
	HSDouble tRadius = 1.0;
	if ( ( tPx * tPx + tPy * tPy ) > 1.0 )
	{
		tVx = tPx;
		tVy = tPy;
		tVz = 0.0;
	}	
	else
	{
		tVx = tPx;
		tVy = tPy;
		tVz = sqrt( tRadius * tRadius - tPx * tPx - tPy * tPy );	
	}
}

HSVoid CGraphicSphere::ResizeViewport( CRect &tRect )
{
	wglMakeCurrent( mGLDC, mRC );

	GLfloat tRange = 100.0;
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

HSVoid CGraphicSphere::DrawString( const HSChar *pTxt, CGLVector3 tScale ) 
{	/*
	static const HSInt MAX_CHAR = 255;

	if( mIsFirstCall ) 
	{                          
		mIsFirstCall = HSFalse;

		mLists = glGenLists( MAX_CHAR );

		glListBase( mLists );

		wglUseFontBitmaps( wglGetCurrentDC(), 0, MAX_CHAR, mLists );
	}

	glRasterPos3f( tPos.X, tPos.Y, tPos.Z );

	GLsizei tLen = strlen( pTxt );  
	glCallLists( tLen, GL_BYTE, ( GLbyte * )pTxt ); 	
	*/

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

	//HS_INFO( "Angle %f, Axis: %f, %f, %f", tAngle, tRotation.X,  tRotation.Y,  tRotation.Z );
	

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

HSVoid CGraphicSphere::InitSphere()
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
	//glEnable( GL_LIGHTING );
	//glEnable( GL_LIGHT0 );

	glEnable( GL_ALPHA_TEST );
	glEnable( GL_DEPTH_TEST );
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
	/*
	GLfloat light1_ambient[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat light1_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light1_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light1_position[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat spot_direction[] = { -1.0, -1.0, 0.0 };
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.5);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.5);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.2);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 45.0);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0);
	glEnable(GL_LIGHT1);
	*/
	
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

	/*
	
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );	 
	
	CBitmap tTextureBitmap;
	tTextureBitmap.LoadBitmapA( IDB_BITMAP_SPHERE_FACE );	

	BITMAP tBitmap;
	tTextureBitmap.GetBitmap( &tBitmap );

	HSInt tBitmapSize = tBitmap.bmWidth * tBitmap.bmHeight * 4;	

	mTextureBits = new HSChar[ tBitmapSize ];
	tTextureBitmap.GetBitmapBits( tBitmapSize, mTextureBits );		
	
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, tBitmap.bmWidth, tBitmap.bmHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, mTextureBits );
	*/
	
}

HSBool CGraphicSphere::ThreadRuning( HSInt tThreadID )
{		
	this->PostMessage( WM_REFRESH_GRAPHIC, 1, 1 );
	
	return HSTrue;
}

HSVoid CGraphicSphere::ThreadWillStop( HSInt tThreadID )
{
}

BOOL CGraphicSphere::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here	

	mIsInit = HSTrue;

	mGLDC = ::GetDC( m_hWnd );

	InitSphere();

	CRect tRect;
	this->GetClientRect( tRect );

	ResizeViewport( tRect );

	this->SetTimer( 1, 1, NULL );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CGraphicSphere::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here

	mRefreshThread->Stop();

	wglMakeCurrent( NULL, NULL );
    wglDeleteContext( mRC );	

	gluDeleteQuadric( mSphere );
	gluDeleteQuadric( mCylinder );	

	glDeleteLists( mLists, 1 );
}


void CGraphicSphere::OnSize(UINT nType, int cx, int cy)
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


void CGraphicSphere::OnLButtonDown(UINT nFlags, CPoint point)
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


void CGraphicSphere::OnLButtonUp(UINT nFlags, CPoint point)
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


void CGraphicSphere::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if ( mIsBeginDrag )
	{		
		wglMakeCurrent( mGLDC, mRC );

		HSDouble tAngle = 0;
		CGLVector3 tRotation;
		CalRotateVars( mPrevPoint.x, mPrevPoint.y, point.x, point.y, tAngle, tRotation.X, tRotation.Y, tRotation.Z );
		
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

void CGraphicSphere::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialogEx::OnPaint() for painting messages

	DrawGraphic();
}


void CGraphicSphere::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	KillTimer( nIDEvent );

	this->Invalidate();	

	CDialogEx::OnTimer(nIDEvent);
}

LRESULT CGraphicSphere::OnRefreshGraphic( UINT wParam, LPARAM lParam )
{
	HSInt tDirection = ( HSInt )wParam;
	HSInt tAngle = ( HSInt )lParam;
	HorRotate( tDirection, tAngle );

	return 1;
}

HSVoid CGraphicSphere::HorRotate( HSInt tDirection, HSDouble tAngle )
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
	CalRotateVars( tStartX, tCenterY, tEndX, tCenterY, tTmpAngle, tRotation.X, tRotation.Y, tRotation.Z );

	CGLQuaternion tQ( tRotation, tAngle * 3.14159265 / 180.0 );
	mOrientation = tQ * mOrientation;	
	mOrientation.Normalize();

	DrawGraphic();
}

HSVoid CGraphicSphere::SetHitRadius( HSInt tRadius )
{
	mHitRadius = tRadius;
	DrawGraphic();
}