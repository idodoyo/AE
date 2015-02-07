#pragma once

#include "GL\glut.h"
#include "ThreadControlEx.h"
#include "GLDataTypes.h"

// CGraphicNormal3D dialog

const int WM_REFRESH_GRAPHIC = WM_USER + 30;

class IGraphic3DProtocol
{
public:
	virtual HSVoid MouseMove( UINT nFlags, CPoint point ) = 0;
	virtual HSVoid MouseUp( UINT nFlags, CPoint point ) = 0;
	virtual HSVoid FocusWnd() = 0;

	virtual ~IGraphic3DProtocol(){}
};

class CGraphicNormal3D : public CDialogEx
{
	DECLARE_DYNAMIC(CGraphicNormal3D)

public:
	CGraphicNormal3D(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicNormal3D();

// Dialog Data
	enum { IDD = IDD_GRAPHICNORMAL3D };

	void OnOK(){}
	void OnCancel(){}

public:
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

	HSVoid CalRotateVars( HSDouble tStartX, HSDouble tStartY, HSDouble tEndX, HSDouble tEndY, HSDouble &tResAngle, HSDouble &tResVX, HSDouble &tResVY, HSDouble &tResVZ );
	HSVoid Map2Sphere( HSDouble tPx, HSDouble tPy, HSDouble &tVx, HSDouble &tVy, HSDouble &tVz );

public:
	HSVoid Reset(){ mOrientation = mInitOrientation; mRefreshThread->Stop(); }
	HSVoid Refresh(){ DrawGraphic(); }
	HSVoid InDirectRefresh(){ this->PostMessage( WM_REFRESH_GRAPHIC, 1, 0 ); }	
	HSVoid AutoRatate(){ mRefreshThread->Start(); }

	HSVoid ResizeViewport( CRect &tRect );

protected:
	virtual HSVoid InitGraphic(){}
	virtual HSVoid DrawGraphic(){}
	virtual HSVoid DestroyGraphic(){}

	HSVoid InitWnd();
	HSVoid HorRotate( HSInt tDirection, HSDouble tAngle );
	HSVoid DrawString( const HSChar *pTxt, CGLVector3 tScale = CGLVector3( 5, 5, 1 ) );		
	HSVoid DrawAxisFace( CGLVector3 tRotation, HSDouble tAngle, CGLVector3 tTranslate, CGLVector3 tNormal );

	HSVoid DrawCustomSquare( CGLVector3 tPoints[], HSBool tDrawLine = HSTrue );
	HSVoid DrawCube( HSDouble tLength, HSDouble tWidth, HSDouble tHeight, HSDouble tAngle, CGLVector3 tTranslate, HSBool tDrawLine = HSTrue );

protected:
	IGraphic3DProtocol *m3DParent;

	HDC mGLDC;		
	HGLRC mRC;

	GLuint mLists;
	HSBool mIsFirstCall;	

	CGLQuaternion mOrientation;		
	CGLQuaternion mInitOrientation;		
	
	HSBool mIsBeginDrag;

	CPoint mPrevPoint;

	HSBool mIsInit;

	CThreadControlEx< CGraphicNormal3D > *mRefreshThread;	

	HFONT mFont;

	enum { GRAPHIC_RADIUS = 60 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	LRESULT OnRefreshGraphic( UINT wParam, LPARAM lParam );	

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
};
