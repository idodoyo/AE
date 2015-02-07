// GraphicNormal3D.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GraphicNormal3D.h"
#include "afxdialogex.h"

#include "HSLogProtocol.h"
#include "resource.h"
#include "MainFrm.h"
#include "GlFont.h"

// CGraphicNormal3D dialog

IMPLEMENT_DYNAMIC(CGraphicNormal3D, CDialogEx)

CGraphicNormal3D::CGraphicNormal3D(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGraphicNormal3D::IDD, pParent)
{
	mRefreshThread = new CThreadControlEx< CGraphicNormal3D >( this, 50, 0 );
	mIsInit = HSFalse;

	mIsBeginDrag = HSFalse;		

	mIsFirstCall = HSTrue;

	m3DParent = NULL;
}

CGraphicNormal3D::~CGraphicNormal3D()
{
	delete mRefreshThread;
}

HSVoid CGraphicNormal3D::ResizeViewport( CRect &tRect )
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

HSVoid CGraphicNormal3D::DrawString( const HSChar *pTxt, CGLVector3 tScale ) 
{	
	static const HSInt MAX_CHAR = 256;

	if( mIsFirstCall ) 
	{ 
		mFont = CreateFont(
		32,										//< lfHeight 字体高度
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

HSVoid CGraphicNormal3D::CalRotateVars( HSDouble tStartX, HSDouble tStartY, HSDouble tEndX, HSDouble tEndY, HSDouble &tResAngle, HSDouble &tResVX, HSDouble &tResVY, HSDouble &tResVZ )
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

	if ( !tS_vec.IsZero() )
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
	if ( abs( tResAngle ) > 0.000001 )
	{		
		tResVX = tR_vec.X;
		tResVY = tR_vec.Y;
		tResVZ = tR_vec.Z;
	}
	else
	{
		tResAngle = 0;
		tResVX = 0;
		tResVY = 0;
		tResVZ = 0;
	}
}

HSVoid CGraphicNormal3D::Map2Sphere( HSDouble tPx, HSDouble tPy, HSDouble &tVx, HSDouble &tVy, HSDouble &tVz )
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

HSBool CGraphicNormal3D::ThreadRuning( HSInt tThreadID )
{		
	this->PostMessage( WM_REFRESH_GRAPHIC, 1, 1 );
	
	return HSTrue;
}

HSVoid CGraphicNormal3D::ThreadWillStop( HSInt tThreadID )
{
}

HSVoid CGraphicNormal3D::InitWnd()
{
	static PIXELFORMATDESCRIPTOR tPfd = { sizeof(PIXELFORMATDESCRIPTOR),  
		1,  
		PFD_DRAW_TO_WINDOW |  
		PFD_SUPPORT_OPENGL |  
		PFD_DOUBLEBUFFER,  
		PFD_TYPE_RGBA,  
		24,  
		0, 0, 0, 0, 0, 0,  
		0,  
		0,  
		0,  
		0, 0, 0, 0,  
		32,  
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

    glClearColor( 0.94, 0.94, 0.94, 1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//glShadeModel( GL_FLAT );
	glShadeModel( GL_SMOOTH );

	glEnable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}


void CGraphicNormal3D::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGraphicNormal3D, CDialogEx)
	ON_MESSAGE( WM_REFRESH_GRAPHIC, OnRefreshGraphic )
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CGraphicNormal3D message handlers


BOOL CGraphicNormal3D::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	mIsInit = HSTrue;

	mGLDC = ::GetDC( m_hWnd );

	InitWnd();
	InitGraphic();

	CRect tRect;
	this->GetClientRect( tRect );

	ResizeViewport( tRect );

	this->SetTimer( 1, 1, NULL );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CGraphicNormal3D::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here

	mRefreshThread->Stop();

	wglMakeCurrent( NULL, NULL );
    wglDeleteContext( mRC );

	DestroyGraphic();

	glDeleteLists( mLists, 1 );
}


void CGraphicNormal3D::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CRect tRect;
	this->GetClientRect( &tRect );

	CPoint tPoint = point;	

	m3DParent->FocusWnd();

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


void CGraphicNormal3D::OnMouseMove(UINT nFlags, CPoint point)
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
		m3DParent->MouseMove( nFlags, point );
	}

	CDialogEx::OnMouseMove(nFlags, point);
}


void CGraphicNormal3D::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if ( mIsBeginDrag )
	{
		mIsBeginDrag = HSFalse;
	}
	else
	{
		m3DParent->MouseUp( nFlags, point );
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CGraphicNormal3D::OnSize(UINT nType, int cx, int cy)
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


void CGraphicNormal3D::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	KillTimer( nIDEvent );

	this->Invalidate();	

	CDialogEx::OnTimer(nIDEvent);
}


void CGraphicNormal3D::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialogEx::OnPaint() for painting messages

	DrawGraphic();
}

LRESULT CGraphicNormal3D::OnRefreshGraphic( UINT wParam, LPARAM lParam )
{
	HSInt tDirection = ( HSInt )wParam;
	HSInt tAngle = ( HSInt )lParam;
	HorRotate( tDirection, tAngle );

	return 1;
}

HSVoid CGraphicNormal3D::HorRotate( HSInt tDirection, HSDouble tAngle )
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

HSVoid CGraphicNormal3D::DrawAxisFace( CGLVector3 tRotation, HSDouble tAngle, CGLVector3 tTranslate, CGLVector3 tNormal )
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

HSVoid CGraphicNormal3D::DrawCustomSquare( CGLVector3 tPoints[], HSBool tDrawLine )
{
	glBegin( GL_QUADS );  
		glNormal3f( 0.0F, 0.0F, 1.0F);  
		glVertex3f( tPoints[ 0 ].X, tPoints[ 0 ].Y, tPoints[ 0 ].Z );  
		glVertex3f( tPoints[ 1 ].X, tPoints[ 1 ].Y, tPoints[ 1 ].Z );  
		glVertex3f( tPoints[ 2 ].X, tPoints[ 2 ].Y, tPoints[ 2 ].Z );
		glVertex3f( tPoints[ 3 ].X, tPoints[ 3 ].Y, tPoints[ 3 ].Z );
		//1----------------------------   
		glNormal3f( 0.0F, 0.0F, -1.0F);  
		glVertex3f( tPoints[ 7 ].X, tPoints[ 7 ].Y, tPoints[ 7 ].Z );  
		glVertex3f( tPoints[ 6 ].X, tPoints[ 6 ].Y, tPoints[ 6 ].Z );  
		glVertex3f( tPoints[ 5 ].X, tPoints[ 5 ].Y, tPoints[ 5 ].Z );
		glVertex3f( tPoints[ 4 ].X, tPoints[ 4 ].Y, tPoints[ 4 ].Z );
		//2----------------------------   
		glNormal3f( 0.0F, 1.0F, 0.0F);  
		glVertex3f( tPoints[ 0 ].X, tPoints[ 0 ].Y, tPoints[ 0 ].Z );  
		glVertex3f( tPoints[ 4 ].X, tPoints[ 4 ].Y, tPoints[ 4 ].Z );
		glVertex3f( tPoints[ 5 ].X, tPoints[ 5 ].Y, tPoints[ 5 ].Z );
		glVertex3f( tPoints[ 1 ].X, tPoints[ 1 ].Y, tPoints[ 1 ].Z );		
		//3----------------------------   
		glNormal3f( 0.0F, -1.0F, 0.0F);  				
		glVertex3f( tPoints[ 3 ].X, tPoints[ 3 ].Y, tPoints[ 3 ].Z );  
		glVertex3f( tPoints[ 2 ].X, tPoints[ 2 ].Y, tPoints[ 2 ].Z );		
		glVertex3f( tPoints[ 6 ].X, tPoints[ 6 ].Y, tPoints[ 6 ].Z );
		glVertex3f( tPoints[ 7 ].X, tPoints[ 7 ].Y, tPoints[ 7 ].Z );  		
		//4----------------------------   
		glNormal3f( 1.0F, 0.0F, 0.0F);  
		glVertex3f( tPoints[ 0 ].X, tPoints[ 0 ].Y, tPoints[ 0 ].Z );  
		glVertex3f( tPoints[ 3 ].X, tPoints[ 3 ].Y, tPoints[ 3 ].Z );		
		glVertex3f( tPoints[ 7 ].X, tPoints[ 7 ].Y, tPoints[ 7 ].Z );
		glVertex3f( tPoints[ 4 ].X, tPoints[ 4 ].Y, tPoints[ 4 ].Z );  
		//5----------------------------   
		glNormal3f( -1.0F, 0.0F, 0.0F);		
		glVertex3f( tPoints[ 1 ].X, tPoints[ 1 ].Y, tPoints[ 1 ].Z );  
		glVertex3f( tPoints[ 5 ].X, tPoints[ 5 ].Y, tPoints[ 5 ].Z );		
		glVertex3f( tPoints[ 6 ].X, tPoints[ 6 ].Y, tPoints[ 6 ].Z );
		glVertex3f( tPoints[ 2 ].X, tPoints[ 2 ].Y, tPoints[ 2 ].Z );  
		//6----------------------------

    glEnd();  	
	
	if ( !tDrawLine )
	{
		return;
	}

	glColor4f( 0.1f, 0.1f, 0.1f, 1.0 );

	typedef struct LineAxis
	{
		HSInt One;
		HSInt Tue;
	} LineAxis;

	LineAxis tAxis[ 12 ] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };

	for ( HSInt i = 0; i < sizeof( tAxis ) / sizeof( LineAxis ); i++ )
	{
		glBegin( GL_LINE_STRIP );
			glVertex3f( tPoints[ tAxis[ i ].One ].X, tPoints[ tAxis[ i ].One ].Y, tPoints[ tAxis[ i ].One ].Z );  
			glVertex3f( tPoints[ tAxis[ i ].Tue ].X, tPoints[ tAxis[ i ].Tue ].Y, tPoints[ tAxis[ i ].Tue ].Z );
		glEnd();
	}
}

HSVoid CGraphicNormal3D::DrawCube( HSDouble tLength, HSDouble tWidth, HSDouble tHeight, HSDouble tAngle, CGLVector3 tTranslate, HSBool tDrawLine )
{	
	glPushMatrix();
	glTranslatef( tTranslate.X, tTranslate.Y, tTranslate.Z );
	glRotatef( tAngle, 0.0f, 1.0f, 0.0f );

	if ( tDrawLine )
	{
		glColor4f( 0.5f, 0.1f, 1.0f, 0.5 );	
	}

	CGLVector3 tPoints[] = { CGLVector3( tLength / 2, tHeight / 2 , tWidth / 2 ),  CGLVector3( -tLength / 2, tHeight / 2 , tWidth / 2 ),  CGLVector3( -tLength / 2, -tHeight / 2 , tWidth / 2 ),  CGLVector3( tLength / 2, -tHeight / 2 , tWidth / 2 ), 
							CGLVector3( tLength / 2, tHeight / 2 , -tWidth / 2 ),  CGLVector3( -tLength / 2, tHeight / 2 , -tWidth / 2 ),  CGLVector3( -tLength / 2, -tHeight / 2 , -tWidth / 2 ),  CGLVector3( tLength / 2, -tHeight / 2 , -tWidth / 2 ) };

	DrawCustomSquare( tPoints, tDrawLine );

	glPopMatrix();
}