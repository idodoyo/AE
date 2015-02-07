#pragma once

#include "GL\glut.h"
#include "ThreadControlEx.h"
#include "GLDataTypes.h"
#include "GraphicNormal3D.h"

class CSpherePosition;
// CGraphicSphere dialog

class CGraphicSphere : public CGraphicNormal3D
{
public:
	CGraphicSphere(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicSphere();

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
	HSVoid SetSensor( map< HSInt, SensorInfo > *pPositionSensors ){ mPositionSensors = pPositionSensors; }
	HSVoid SetFocusdSensor( HSInt tIndex ){ mFocusedIndex = tIndex; }
	HSVoid SetHits( list< HitPosition > *pHitsPosition ){ mHitsPosition = pHitsPosition; }	
	HSVoid SetHitRadius( HSInt tRadius );

private:	
	virtual HSVoid InitGraphic();
	virtual HSVoid DrawGraphic();
	virtual HSVoid DestroyGraphic();

	HSVoid DrawSphere( HSDouble tRadius );		
	HSVoid DrawDirectionAngle( HSDouble tDirectionAngle, HSDouble tRadius );
	HSVoid DrawUpAngle( HSDouble tDirectionAngle, HSDouble tUpAngle, CGLVector3 tAdjustTrans, CGLVector3 tAdjustRotation );
	HSVoid DrawHit( HSDouble tDirectionAngle, HSDouble tUpAngle, HSDouble tRadius );
	HSVoid DrawSensor( HSDouble tDirectionAngle, HSDouble tUpAngle, HSDouble tRadius, HSInt tIndex );	

	HSVoid CoordWithAngles( HSDouble tDirectionAngle, HSDouble tUpAngle, HSDouble tRadius, CGLVector3 &tCoord );
	HSVoid DrawCircle( HSDouble tRadius, HSString tStrAxis );	

private:
	GLUquadricObj *mSphere;
	GLUquadricObj *mCylinder;

	HSInt mFocusedIndex;

	map< HSInt, SensorInfo > *mPositionSensors;
	list< HitPosition > *mHitsPosition;

	CSpherePosition *mParent;

	HSDouble mHitRadius;
};
