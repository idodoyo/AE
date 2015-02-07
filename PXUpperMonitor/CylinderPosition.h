#pragma once
#include "afxwin.h"
#include "DataHandle\DataTypes.h"
#include "StaticCylinderPosition.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "ArgListTransfer.h"
#include "ArgDataFectcher.h"
#include "GraphicCylinder.h"
#include "NewtonIterator.h"
#include "PlanePositionCalulator.h"
#include "NormalGraphic.h"

// CCylinderPosition dialog

class CCylinderPosition : public CNormalGraphic, public INewtonIteratorProtocol< 4 >
{
public:
	CCylinderPosition(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCylinderPosition();

public:
	static IGraphicProtocol * CreateProto(){ return new CCylinderPosition; }
	
	virtual HSString Identifier(){ return "GRAPHIC_CYLINDER_POSITION"; }
	virtual HSString Name(){ return "3D¶¨Î»";  }

	virtual HSBool Start();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );	

	virtual HSInt LargeIcon(){ return IDB_BITMAP_CYLINDER_POSITION; }

	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CCylinderPosition; }

	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );

public:
	virtual HSBool GraphicNeedRefresh();	

	virtual HSVoid InitDialog();	
	virtual HSVoid ViewResized( CRect &tRect );

	virtual HSBool IsDataExportNeeded();
	virtual HSVoid ExportData( CDataExportHelper &tDataExport );

	virtual HSVoid ResetData( CMainFrame* pMainFrame );		

	virtual HSVoid AutoSetSensors();

	virtual HSVoid Graphic3DRefresh( HSBool tNeedInvalidate );	


public:
	typedef struct PositionSensor
	{
		HSInt Face;
		HSDouble PosX;
		HSDouble PosY;			
		HSBool Forbid;
		HSBool IsOn;
		DEVICE_CH_IDENTIFIER DataIdentifier;	
		CArgListTransfer *ArgTransfer;		
		vector< CArgTableManager::ArgItem > ArgList;
		vector< HSBool > ArgHandled;
		HSInt Index;
	} PositionSensor;

public:	
	HSVoid UpdatePositionSensor( HSInt tIndex, HSInt tFace, HSDouble tFirstPos, HSDouble tSecondPos, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier );
	PositionSensor * GetPositionSensor( HSInt tIndex );

	HSVoid SetFocusSensor( HSInt tIndex );

	HSVoid SetMaterialXLength( HSDouble tLength );
	HSVoid SetMaterialYLength( HSDouble tLength );
	HSVoid SetMaterialZLength( HSDouble tLength );
	HSDouble MaterialXLength(){ return mMaterialXLength; }
	HSDouble MaterialYLength(){ return mMaterialYLength; }
	HSDouble MaterialZLength(){ return mMaterialZLength; }

	HSVoid SetXAxisLength( HSDouble tX );
	HSVoid SetYAxisLength( HSDouble tY );
	HSVoid SetZAxisLength( HSDouble tZ );
	HSVoid GetAxisLength( HSDouble &tX, HSDouble &tY, HSDouble &tZ ){ tX = mAxisXLength; tY = mAxisYLength; tZ = mAxisZLength; }

	HSString FaceWithIndex( HSInt tFace, HSString &tFirstAxisName, HSString &tSecondAxisName );

	HSVoid RefreshSensors();
	HSVoid FocusWnd();

	HSInt HitRadius(){ return mHitRadius; }
	HSVoid SetHitRadius( HSInt tHitRadius );

private:
	typedef struct HitCalcInfo
	{
		HSInt SensorIndex;
		HSUInt ArgIndex;
		map< HSInt, PositionSensor > *PositionSensors;
	} HitCalcInfo;	
	
	typedef struct SensorDistanceInfo
	{
		HSInt SensorIndex;
		HSDouble Distance;		
	} SensorDistanceInfo;
	
	HSVoid InitPositionSensors();
	HSBool CalcHitPosition( HSInt tSensor, HSUInt tArgIndex );
	HSBool IsHitQualified( HSInt64 tDuration, HSDouble tVelocity, HSInt tSensorOne, HSInt tSensorTwo );
	HSBool IsResQualified( HSDouble tPosX, HSDouble tPosY, HSDouble tPosZ, vector< CCylinderPosition::HitCalcInfo > &tHitSensors, HSDouble &tEnergy );	
	HSBool PositionWithHitSensor( HSDouble tInitX0[ 4 ], vector< HitCalcInfo > &tHitSensors, vector< HSInt > &tHitSensorsIndex, vector< CGraphicCylinder::HitPosition > &tHitPos );
	HSDouble DistanceWithSensorIndex( HSDouble tPosX, HSDouble tPosY, HSDouble tPosZ, HSInt tSensor );			

	virtual HSVoid GetFunctionMValue( HSDouble tX0[ 4 ], HSDouble *tPoints[ 4 ], HSDouble tRes[ 4 ][ 4 ] );
	virtual HSVoid GetFunctionValue( HSDouble tX0[ 4 ], HSDouble *tPoints[ 4 ], HSDouble tRes[ 4 ] );

	static bool HitSensorComapre( HitCalcInfo &t1, HitCalcInfo &t2 );
	static bool HitSensorDistanceComapre( SensorDistanceInfo &t1, SensorDistanceInfo &t2 );

	HSVoid CoordWithFaceInfo( PositionSensor *pSensor, CGraphicCylinder::SensorInfo &tGraphicSensor );	

private:
	map< HSInt, PositionSensor > mPositionSensors;	

	CGraphicCylinder *mCylinder;

	map< HSInt, CGraphicCylinder::SensorInfo > mSensors;
	list< CGraphicCylinder::HitPosition > mHitsPosition;

	HSDouble mMaterialXLength;
	HSDouble mMaterialYLength;
	HSDouble mMaterialZLength;

	HSDouble mAxisXLength;
	HSDouble mAxisYLength;
	HSDouble mAxisZLength;

	HSInt mHitRadius;

	enum { DEFALUT_SENSOR_NUM = 6 };

	CNewtonIterator< 4 > mNewtonIterator;

	CStaticCylinderPosition mStaticCylinder;
};
