#pragma once

#include "GL\glut.h"
#include "ThreadControlEx.h"
#include "GLDataTypes.h"

class CCylinderPosition;
// CGraphicSphere dialog

const int WM_REFRESH_GRAPHIC_CYLINDER = WM_USER + 3;

// CGraphicCylinder dialog

class CGraphicCylinder : public CDialogEx
{
	DECLARE_DYNAMIC(CGraphicCylinder)

public:
	CGraphicCylinder(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicCylinder();

// Dialog Data
	enum { IDD = IDD_GRAPHICCYLINDER };

	void OnOK(){}
	void OnCancel(){}

public:
	typedef struct SensorInfo
	{
		HSDouble PosX;
		HSDouble PosY;
		HSDouble PosZ;
		HSInt Index;
	} SensorInfo;	

	typedef struct HitPosition
	{
		HSDouble PosX;
		HSDouble PosY;
		HSDouble PosZ;
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
	HSVoid InDirectRefresh(){ this->PostMessage( WM_REFRESH_GRAPHIC_CYLINDER, 1, 0 ); }
	HSVoid AutoRatate(){ mRefreshThread->Start(); }

	HSVoid SetHitRadius( HSInt tRadius );
	HSVoid SetXYZ( HSDouble tX, HSDouble tY, HSDouble tZ );
	HSVoid SetMaterialXYZ( HSDouble tX, HSDouble tY, HSDouble tZ );

	HSVoid ResizeViewport( CRect &tRect );

private:	
	HSVoid InitGraphic();
	HSVoid DrawGraphic();	
	HSVoid DrawAxisFace( CGLVector3 tRotation, HSDouble tAngle, CGLVector3 tTranslate, CGLVector3 tNormal );
	HSVoid DrawXAxis( CGLVector3 tPos, HSDouble tXDelta, HSInt tCount, HSDouble tXValueBegin, HSDouble tXValueDelta, CGLVector3 tCenterPos );
	HSVoid DrawYAxis( CGLVector3 tPos, HSDouble tYDelta, HSInt tCount, HSDouble tYValueBegin, HSDouble tYValueDelta, CGLVector3 tCenterPos );
	HSVoid DrawZAxis( CGLVector3 tPos, HSDouble tZDelta, HSInt tCount, HSDouble tZValueBegin, HSDouble tZValueDelta, CGLVector3 tCenterPos );
	HSVoid DrawSquare( HSDouble tLength, HSDouble tHeight, HSDouble tWidth );
	HSVoid DrawString( const HSChar *pTxt, CGLVector3 tScale = CGLVector3( 5, 5, 1 ) );		
	HSVoid DrawHit( HSDouble tPosX, HSDouble tPosY, HSDouble tPosZ );
	HSVoid DrawSensor( HSDouble tPosX, HSDouble tPosY, HSDouble tPosZ, HSInt tIndex );	
	HSVoid HorRotate( HSInt tDirection, HSDouble tAngle );

	HSVoid GetCorrectAxis( HSDouble tValue, HSDouble &tResBegin, HSDouble &tResDelta, HSDouble &tDigitPerValue );
	

private:
	HDC mGLDC;		
	HGLRC mRC;

	GLuint mLists;
	HSBool mIsFirstCall;	

	CGLQuaternion mOrientation;		
	CGLQuaternion mInitOrientation;		
	
	HSBool mIsBeginDrag;

	CPoint mPrevPoint;

	HSBool mIsInit;

	CThreadControlEx< CGraphicCylinder > *mRefreshThread;	

	HSInt mFocusedIndex;

	map< HSInt, SensorInfo > *mPositionSensors;
	list< HitPosition > *mHitsPosition;

	CCylinderPosition *mParent;

	HSDouble mHitRadius;
	HSDouble mXLength;
	HSDouble mYLength;
	HSDouble mZLength;

	HSDouble mMaterialXLength;
	HSDouble mMaterialYLength;
	HSDouble mMaterialZLength;

	HFONT mFont;

	typedef struct AXIS_INFO
	{
		HSDouble Begin;
		HSDouble Delta;
		HSDouble DigitPerValue;
	} AXIS_INFO;

	AXIS_INFO mAxisInfo[ 3 ];

	enum { GRAPHIC_RADIUS = 60 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	LRESULT OnRefreshGraphic( UINT wParam, LPARAM lParam );	

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
