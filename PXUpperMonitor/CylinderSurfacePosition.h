#pragma once
#include "afxwin.h"
#include "DataHandle\DataTypes.h"
#include "StaticCylinderSurfacePosition.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "ArgListTransfer.h"
#include "ArgDataFectcher.h"
#include "GraphicCylinderSurface.h"
#include "NewtonIterator.h"
#include "PlanePositionCalulator.h"
#include "NormalGraphic.h"

// CCylinderSurfacePosition dialog

class CCylinderSurfacePosition : public CNormalGraphic, public IPlanePositionCalulatorProtocol
{
public:
	CCylinderSurfacePosition(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCylinderSurfacePosition();


public:
	static IGraphicProtocol * CreateProto(){ return new CCylinderSurfacePosition; }
	
	virtual HSString Identifier(){ return "GRAPHIC_CYLINDER_SURFACE_POSITION"; }
	virtual HSString Name(){ return "ÖùÃæ¶¨Î»";  }

	virtual HSBool Start();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );	

	virtual HSInt LargeIcon(){ return IDB_BITMAP_CYLINDER_SURFACE_POSITION; }

	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CCylinderSurfacePosition; }
	
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
	HSDouble XPosWithAngle( HSDouble tAngle ){ return 3.14159265 * mMaterialRadius / 180.0 * tAngle; }
	HSDouble AngleWithXPos( HSDouble tXPos ){ return tXPos / ( 3.14159265 * mMaterialRadius ) * 180.0; }

public:
	virtual HSVoid GetSensorPosition( HSInt tSensor, HSDouble &tPosX, HSDouble &tPosY ){ tPosX = XPosWithAngle( mPositionSensors[ tSensor ].Angle ); tPosY = mPositionSensors[ tSensor ].Height; }
	virtual HSUInt64 GetSensorHitTime( HSInt tSensor, HSInt tArgIndex ){ return mPositionSensors[ tSensor ].ArgList[ tArgIndex ].NBeginTime; }
	virtual HSDouble DistanceBetweenSensors( HSInt tSensorOne, HSInt tSensorTwo );
	virtual HSDouble DistanceFromSensor( HSDouble tPosX, HSDouble tPosY, HSInt tSensor );

public:
	virtual HSVoid FocusWnd();

public:
	typedef struct PositionSensor
	{		
		HSDouble Angle;
		HSDouble Height;			
		HSBool Forbid;
		HSBool IsOn;
		DEVICE_CH_IDENTIFIER DataIdentifier;	
		CArgListTransfer *ArgTransfer;		
		vector< CArgTableManager::ArgItem > ArgList;
		vector< HSBool > ArgHandled;
		HSInt Index;
	} PositionSensor;

public:	
	HSVoid UpdatePositionSensor( HSInt tIndex, HSDouble tAngle, HSDouble tHeight, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier );
	PositionSensor * GetPositionSensor( HSInt tIndex );

	HSVoid SetFocusSensor( HSInt tIndex );
	
	HSVoid SetMaterialYLength( HSDouble tLength );
	HSVoid SetMaterialRadius( HSDouble tRadius );	
	HSDouble MaterialYLength(){ return mMaterialYLength; }
	HSDouble MaterialRadius(){ return mMaterialRadius; }
	
	HSVoid SetYAxisLength( HSDouble tY );

	HSDouble GetAxisLength(){ return mAxisYLength; }
	

	HSVoid RefreshSensors();	

	HSInt HitRadius(){ return mHitRadius; }
	HSVoid SetHitRadius( HSInt tHitRadius );

private:	
	HSVoid InitPositionSensors();
	HSBool CalcHitPosition( HSInt tSensor, HSUInt tArgIndex );
	HSBool IsHitQualified( HSInt64 tDuration, HSDouble tVelocity, HSInt tSensorOne, HSInt tSensorTwo );
	HSBool IsResQualified( HSDouble tPosX, HSDouble tPosY, vector< CPlanePositionCalulator::HIT_CALC_INFO > &tHitSensors, HSDouble &tEnergy );		
	

private:
	map< HSInt, PositionSensor > mPositionSensors;	

	CGraphicCylinderSurface *mCylinder;

	map< HSInt, CGraphicCylinderSurface::SensorInfo > mSensors;
	list< CGraphicCylinderSurface::HitPosition > mHitsPosition;
	
	HSDouble mMaterialYLength;
	HSDouble mMaterialRadius;

	HSDouble mAxisYLength;

	HSInt mHitRadius;

	enum { DEFALUT_SENSOR_NUM = 3 };	

	CPlanePositionCalulator mPositionCalulator;

	CStaticCylinderSurfacePosition mStaticCylinderSurface;
};
