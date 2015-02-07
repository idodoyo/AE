#pragma once

#include "GL\glut.h"
#include "ThreadControlEx.h"
#include "GLDataTypes.h"
#include "GraphicNormal3D.h"

class CCylinderPosition;
class C3DCrackPosition;

// CGraphicCylinder dialog

class CGraphicCylinder : public CGraphicNormal3D
{
public:
	CGraphicCylinder(CWnd* pParent = NULL, HSBool tIs3DCrack = HSFalse );   // standard constructor
	virtual ~CGraphicCylinder();

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
	HSVoid SetSensor( map< HSInt, SensorInfo > *pPositionSensors ){ mPositionSensors = pPositionSensors; }
	HSVoid SetFocusdSensor( HSInt tIndex ){ mFocusedIndex = tIndex; }
	HSVoid SetHits( list< HitPosition > *pHitsPosition ){ mHitsPosition = pHitsPosition; }		

	HSVoid SetHitRadius( HSInt tRadius );
	HSVoid SetXYZ( HSDouble tX, HSDouble tY, HSDouble tZ );
	HSVoid SetMaterialXYZ( HSDouble tX, HSDouble tY, HSDouble tZ );

private:	
	virtual HSVoid InitGraphic();
	virtual HSVoid DrawGraphic();
	virtual HSVoid DestroyGraphic();
	
	HSVoid DrawXAxis( CGLVector3 tPos, HSDouble tXDelta, HSInt tCount, HSDouble tXValueBegin, HSDouble tXValueDelta, CGLVector3 tCenterPos );
	HSVoid DrawYAxis( CGLVector3 tPos, HSDouble tYDelta, HSInt tCount, HSDouble tYValueBegin, HSDouble tYValueDelta, CGLVector3 tCenterPos );
	HSVoid DrawZAxis( CGLVector3 tPos, HSDouble tZDelta, HSInt tCount, HSDouble tZValueBegin, HSDouble tZValueDelta, CGLVector3 tCenterPos );
	HSVoid DrawSquare( HSDouble tLength, HSDouble tHeight, HSDouble tWidth );	
	HSVoid DrawHit( HSDouble tPosX, HSDouble tPosY, HSDouble tPosZ );
	HSVoid DrawSensor( HSDouble tPosX, HSDouble tPosY, HSDouble tPosZ, HSInt tIndex );	
	HSVoid DrawArrow();

	HSVoid GetCorrectAxis( HSDouble tValue, HSDouble &tResBegin, HSDouble &tResDelta, HSDouble &tDigitPerValue );	

private:	
	GLUquadricObj *mCylinder;

	HSInt mFocusedIndex;

	map< HSInt, SensorInfo > *mPositionSensors;
	list< HitPosition > *mHitsPosition;

	CCylinderPosition *mParent;
	C3DCrackPosition *m3DCrackParent;
	HSBool mIs3DCrackParent;

	HSDouble mHitRadius;
	HSDouble mXLength;
	HSDouble mYLength;
	HSDouble mZLength;

	HSDouble mMaterialXLength;
	HSDouble mMaterialYLength;
	HSDouble mMaterialZLength;

	typedef struct AXIS_INFO
	{
		HSDouble Begin;
		HSDouble Delta;
		HSDouble DigitPerValue;
	} AXIS_INFO;

	AXIS_INFO mAxisInfo[ 3 ];
};
