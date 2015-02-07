#pragma once

#include "GL\glut.h"
#include "ThreadControlEx.h"
#include "GLDataTypes.h"
#include "GraphicNormal3D.h"

class CGantryCranePosition;

class CGraphicGantryCrane : public CGraphicNormal3D
{
public:
	CGraphicGantryCrane(CWnd* pParent = NULL, HSBool tIs3DCrack = HSFalse );   // standard constructor
	virtual ~CGraphicGantryCrane();

public:
	typedef struct SensorInfo
	{
		HSDouble PosX;
		HSInt Index;
	} SensorInfo;	

	typedef struct HitPosition
	{
		HSDouble PosX;		
		COLORREF Color;
	} HitPosition;

public:
	HSVoid SetSensor( map< HSInt, SensorInfo > *pPositionSensors ){ mPositionSensors = pPositionSensors; }
	HSVoid SetFocusdSensor( HSInt tIndex ){ mFocusedIndex = tIndex; }
	HSVoid SetHits( list< HitPosition > *pHitsPosition ){ mHitsPosition = pHitsPosition; }	
	
	HSVoid SetMaterialLength( HSDouble tX );

private:	
	virtual HSVoid InitGraphic();
	virtual HSVoid DrawGraphic();
	virtual HSVoid DestroyGraphic();		
	
	HSVoid DrawPlank( HSDouble tAngle, CGLVector3 tTranslate );	
	HSVoid DrawRope();
	HSVoid DrawHit( HSDouble tPosX, vector< HSDouble > &tAreas );
	HSVoid DrawSensor( HSDouble tPosX, HSInt tIndex );	

private:	
	GLUquadricObj *mCylinder;

	HSInt mFocusedIndex;

	map< HSInt, SensorInfo > *mPositionSensors;
	list< HitPosition > *mHitsPosition;

	CGantryCranePosition *mParent;	

	HSDouble mMaterialXLength;	
};
