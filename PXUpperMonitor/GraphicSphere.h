#pragma once

#include "GL\glut.h"
#include "ThreadControlEx.h"
#include "GLDataTypes.h"

class CSpherePosition;
// CGraphicSphere dialog

const int WM_REFRESH_GRAPHIC = WM_USER + 2;


class CGraphicSphere : public CDialogEx
{
	DECLARE_DYNAMIC(CGraphicSphere)

public:
	CGraphicSphere(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicSphere();

// Dialog Data
	enum { IDD = IDD_GRAPHICSPHERE };

	void OnOK(){}
	void OnCancel(){}

public:
	typedef struct SensorInfo
	{
		HSDouble AngleDirection;
		HSDouble AngleUp;
		HSInt Index;
	} SensorInfo;	

	typedef struct HitPosition
	{
		HSDouble AngleDirection;
		HSDouble AngleUp;
		COLORREF Color;
	} HitPosition;

public:
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

	HSVoid SetSensor( map< HSInt, SensorInfo > *pPositionSensors ){ mPositionSensors = pPositionSensors; }
	HSVoid SetFocusdSensor( HSInt tIndex ){ mFocusedIndex = tIndex; }
	HSVoid SetHits( list< HitPosition > *pHitsPosition ){ mHitsPosition = pHitsPosition; }
	HSVoid Reset(){ mOrientation = CGLQuaternion(); mRefreshThread->Stop(); }
	HSVoid Refresh(){ DrawGraphic(); }
	HSVoid InDirectRefresh(){ this->PostMessage( WM_REFRESH_GRAPHIC, 1, 0 ); }
	HSVoid AutoRatate(){ mRefreshThread->Start(); }

	HSVoid SetHitRadius( HSInt tRadius );

	static HSVoid CalRotateVars( HSDouble tStartX, HSDouble tStartY, HSDouble tEndX, HSDouble tEndY, HSDouble &tResAngle, HSDouble &tResVX, HSDouble &tResVY, HSDouble &tResVZ );

	static HSVoid Map2Sphere( HSDouble tPx, HSDouble tPy, HSDouble &tVx, HSDouble &tVy, HSDouble &tVz );

private:
	HSVoid ResizeViewport( CRect &tRect );
	HSVoid InitSphere();
	HSVoid DrawGraphic();
	HSVoid DrawSphere( HSDouble tRadius );	
	HSVoid DrawString( const HSChar *pTxt, CGLVector3 tScale );
	HSVoid DrawDirectionAngle( HSDouble tDirectionAngle, HSDouble tRadius );
	HSVoid DrawUpAngle( HSDouble tDirectionAngle, HSDouble tUpAngle, CGLVector3 tAdjustTrans, CGLVector3 tAdjustRotation );
	HSVoid DrawHit( HSDouble tDirectionAngle, HSDouble tUpAngle, HSDouble tRadius );
	HSVoid DrawSensor( HSDouble tDirectionAngle, HSDouble tUpAngle, HSDouble tRadius, HSInt tIndex );	

	HSVoid CoordWithAngles( HSDouble tDirectionAngle, HSDouble tUpAngle, HSDouble tRadius, CGLVector3 &tCoord );
	HSVoid DrawCircle( HSDouble tRadius, HSString tStrAxis );

	HSVoid HorRotate( HSInt tDirection, HSDouble tAngle );

private:
	HDC mGLDC;
	GLUquadricObj *mSphere;
	GLUquadricObj *mCylinder;
	HGLRC mRC;

	GLuint mLists;
	HSBool mIsFirstCall;	

	HSChar *mTextureBits;

	CGLQuaternion mOrientation;		
	
	HSBool mIsBeginDrag;

	CPoint mPrevPoint;

	HSBool mIsInit;

	CThreadControlEx< CGraphicSphere > *mRefreshThread;	

	HSInt mFocusedIndex;

	map< HSInt, SensorInfo > *mPositionSensors;
	list< HitPosition > *mHitsPosition;

	CSpherePosition *mParent;

	HSDouble mHitRadius;

	HFONT mFont;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	LRESULT OnRefreshGraphic( UINT wParam, LPARAM lParam );	

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();	
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
