#pragma once
#include "GL\glut.h"
#include "ThreadControlEx.h"
#include "GLDataTypes.h"

class CCylinderSurfacePosition;

// CGraphicCylinderSurface dialog

const int WM_REFRESH_GRAPHIC_CYLINDER_SURFACE = WM_USER + 4;

class CGraphicCylinderSurface : public CDialogEx
{
	DECLARE_DYNAMIC(CGraphicCylinderSurface)

public:
	CGraphicCylinderSurface(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicCylinderSurface();

// Dialog Data
	enum { IDD = IDD_GRAPHICCYLINDERSURFACE };

	void OnOK(){}
	void OnCancel(){}

public:
	typedef struct SensorInfo
	{
		HSDouble Angle;
		HSDouble Height;		
		HSInt Index;
	} SensorInfo;	

	typedef struct HitPosition
	{
		HSDouble Angle;
		HSDouble Height;		
		COLORREF Color;
	} HitPosition;

public:
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

	HSVoid SetSensor( map< HSInt, SensorInfo > *pPositionSensors ){ mPositionSensors = pPositionSensors; }
	HSVoid SetFocusdSensor( HSInt tIndex ){ mFocusedIndex = tIndex; }
	HSVoid SetHits( list< HitPosition > *pHitsPosition ){ mHitsPosition = pHitsPosition; }
	HSVoid Reset(){ mOrientation = mInitOrientation; mRefreshThread->Stop(); }
	HSVoid Refresh(){ DrawGraphic(); }
	HSVoid InDirectRefresh(){ this->PostMessage( WM_REFRESH_GRAPHIC_CYLINDER_SURFACE, 1, 0 ); }
	HSVoid AutoRatate(){ mRefreshThread->Start(); }

	HSVoid SetHitRadius( HSInt tRadius );
	HSVoid SetHeight( HSDouble tHeight );
	HSVoid SetMaterialHeight( HSDouble tHeight );

	HSVoid ResizeViewport( CRect &tRect );

private:	
	HSVoid InitGraphic();
	HSVoid DrawGraphic();	
	HSVoid DrawCylinder( HSDouble tHeight, HSDouble tRadius );
	HSVoid DrawCircle( HSDouble tRadius );
	HSVoid DrawAxisFace( CGLVector3 tRotation, HSDouble tAngle, CGLVector3 tTranslate, CGLVector3 tNormal );
	HSVoid DrawXAxis( CGLVector3 tCenterPos );
	HSVoid DrawYAxis( CGLVector3 tPos, HSDouble tYDelta, HSInt tCount, HSDouble tYValueBegin, HSDouble tYValueDelta, CGLVector3 tCenterPos );
	HSVoid DrawZAxis( CGLVector3 tCenterPos );	
	HSVoid DrawString( const HSChar *pTxt, CGLVector3 tScale = CGLVector3( 5, 5, 1 ) );		
	HSVoid DrawHit( HSDouble tAngle, HSDouble tHeight, HSDouble tRadius  );
	HSVoid DrawSensor( HSDouble tAngle, HSDouble tHeight, HSDouble tRadius , HSInt tIndex );	
	HSVoid HorRotate( HSInt tDirection, HSDouble tAngle );

	HSVoid GetCorrectAxis( HSDouble tValue, HSDouble &tResBegin, HSDouble &tResDelta, HSDouble &tDigitPerValue );
	

private:
	HDC mGLDC;	
	GLUquadricObj *mCylinder;
	HGLRC mRC;

	GLuint mLists;
	HSBool mIsFirstCall;	

	CGLQuaternion mOrientation;		
	CGLQuaternion mInitOrientation;		
	
	HSBool mIsBeginDrag;

	CPoint mPrevPoint;

	HSBool mIsInit;

	CThreadControlEx< CGraphicCylinderSurface > *mRefreshThread;	

	HSInt mFocusedIndex;

	map< HSInt, SensorInfo > *mPositionSensors;
	list< HitPosition > *mHitsPosition;

	CCylinderSurfacePosition *mParent;

	HSDouble mHitRadius;
	
	HSDouble mYLength;
	
	HSDouble mMaterialYLength;

	HFONT mFont;

	typedef struct AXIS_INFO
	{
		HSDouble Begin;
		HSDouble Delta;
		HSDouble DigitPerValue;
	} AXIS_INFO;

	AXIS_INFO mYAxisInfo;

	enum { GRAPHIC_RADIUS = 60 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	LRESULT OnRefreshGraphic( UINT wParam, LPARAM lParam );	

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
};
