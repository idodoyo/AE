#pragma once
#include "GL\glut.h"
#include "ThreadControlEx.h"
#include "GLDataTypes.h"
#include "GraphicNormal3D.h"

class CCylinderSurfacePosition;

// CGraphicCylinderSurface dialog

class CGraphicCylinderSurface : public CGraphicNormal3D
{
public:
	CGraphicCylinderSurface(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicCylinderSurface();

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
	HSVoid SetSensor( map< HSInt, SensorInfo > *pPositionSensors ){ mPositionSensors = pPositionSensors; }
	HSVoid SetFocusdSensor( HSInt tIndex ){ mFocusedIndex = tIndex; }
	HSVoid SetHits( list< HitPosition > *pHitsPosition ){ mHitsPosition = pHitsPosition; }

	HSVoid SetHitRadius( HSInt tRadius );
	HSVoid SetHeight( HSDouble tHeight );
	HSVoid SetMaterialHeight( HSDouble tHeight );

private:	
	virtual HSVoid InitGraphic();
	virtual HSVoid DrawGraphic();
	virtual HSVoid DestroyGraphic();

	HSVoid DrawCylinder( HSDouble tHeight, HSDouble tRadius );
	HSVoid DrawCircle( HSDouble tRadius );
	HSVoid DrawXAxis( CGLVector3 tCenterPos );
	HSVoid DrawYAxis( CGLVector3 tPos, HSDouble tYDelta, HSInt tCount, HSDouble tYValueBegin, HSDouble tYValueDelta, CGLVector3 tCenterPos );
	HSVoid DrawZAxis( CGLVector3 tCenterPos );
	HSVoid DrawHit( HSDouble tAngle, HSDouble tHeight, HSDouble tRadius  );
	HSVoid DrawSensor( HSDouble tAngle, HSDouble tHeight, HSDouble tRadius , HSInt tIndex );

	HSVoid GetCorrectAxis( HSDouble tValue, HSDouble &tResBegin, HSDouble &tResDelta, HSDouble &tDigitPerValue );
	

private:
	GLUquadricObj *mCylinder;
	
	HSInt mFocusedIndex;

	map< HSInt, SensorInfo > *mPositionSensors;
	list< HitPosition > *mHitsPosition;

	CCylinderSurfacePosition *mParent;

	HSDouble mHitRadius;
	
	HSDouble mYLength;
	
	HSDouble mMaterialYLength;

	typedef struct AXIS_INFO
	{
		HSDouble Begin;
		HSDouble Delta;
		HSDouble DigitPerValue;
	} AXIS_INFO;

	AXIS_INFO mYAxisInfo;	
};
