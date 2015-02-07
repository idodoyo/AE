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

IMPLEMENT_DYNAMIC(CGraphicCylinderSurface, CDialogEx)

CGraphicCylinderSurface::CGraphicCylinderSurface(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGraphicCylinderSurface::IDD, pParent)
{
	mRefreshThread = new CThreadControlEx< CGraphicCylinderSurface >( this, 50, 0 );
	mIsInit = HSFalse;

	mIsBeginDrag = HSFalse;		

	mIsFirstCall = HSTrue;

	mFocusedIndex = -1;

	mParent = dynamic_cast< CCylinderSurfacePosition * >( pParent );

	mHitRadius = 2.0;
	
	mYLength = 50.0;

	mMaterialYLength = 40.0;

	mInitOrientation = CGLQuaternion( CGLVector3( 0.1, -1, -0.3 ), 155 * 3.14159265 / 180 );
	mInitOrientation.Normalize();

	mOrientation = mInitOrientation;
}

CGraphicCylinderSurface::~CGraphicCylinderSurface()
{
	delete mRefreshThread;
}

HSVoid CGraphicCylinderSurface::ResizeViewport( CRect &tRect )
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

HSVoid CGraphicCylinderSurface::DrawString( const HSChar *pTxt, CGLVector3 tScale ) 
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

HSVoid CGraphicCylinderSurface::DrawAxisFace( CGLVector3 tRotation, HSDouble tAngle, CGLVector3 tTranslate, CGLVector3 tNormal )
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
		glVertex3f( 0.0, -tHeight / 2.0 ,  0.0 ); 	
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
    //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
	glClear( GL_COLOR_BUFFER_BIT ); 

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

	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);	
	//glEnable( GL_COLOR_MATERIAL );

	//glEnable( GL_TEXTURE_2D );
	//glEnable( GL_CULL_FACE );
	//glFrontFace( GL_CCW );	

	mCylinder = gluNewQuadric();
	gluQuadricDrawStyle( mCylinder, GLU_FILL );	
	gluQuadricNormals( mCylinder, GLU_SMOOTH );
	//gluQuadricTexture( mCylinder, GLU_TRUE );		
	
}

HSBool CGraphicCylinderSurface::ThreadRuning( HSInt tThreadID )
{		
	this->PostMessage( WM_REFRESH_GRAPHIC_CYLINDER_SURFACE, 1, 1 );
	
	return HSTrue;
}

HSVoid CGraphicCylinderSurface::ThreadWillStop( HSInt tThreadID )
{
}

void CGraphicCylinderSurface::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGraphicCylinderSurface, CDialogEx)
	ON_MESSAGE( WM_REFRESH_GRAPHIC_CYLINDER_SURFACE, OnRefreshGraphic )
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CGraphicCylinderSurface message handlers


void CGraphicCylinderSurface::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here

	mRefreshThread->Stop();

	wglMakeCurrent( NULL, NULL );
    wglDeleteContext( mRC );	
	
	gluDeleteQuadric( mCylinder );	

	glDeleteLists( mLists, 1 );
}


void CGraphicCylinderSurface::OnLButtonDown(UINT nFlags, CPoint point)
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


void CGraphicCylinderSurface::OnLButtonUp(UINT nFlags, CPoint point)
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


void CGraphicCylinderSurface::OnMouseMove(UINT nFlags, CPoint point)
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


void CGraphicCylinderSurface::OnSize(UINT nType, int cx, int cy)
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
	//this->InDirectRefresh();
}


void CGraphicCylinderSurface::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	KillTimer( nIDEvent );

	this->Invalidate();	


	CDialogEx::OnTimer(nIDEvent);
}


void CGraphicCylinderSurface::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialogEx::OnPaint() for painting messages

	DrawGraphic();
}


BOOL CGraphicCylinderSurface::OnInitDialog()
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

LRESULT CGraphicCylinderSurface::OnRefreshGraphic( UINT wParam, LPARAM lParam )
{
	HSInt tDirection = ( HSInt )wParam;
	HSInt tAngle = ( HSInt )lParam;
	HorRotate( tDirection, tAngle );

	return 1;
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

HSVoid CGraphicCylinderSurface::HorRotate( HSInt tDirection, HSDouble tAngle )
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
