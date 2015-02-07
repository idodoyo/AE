#pragma once
#include "afxwin.h"
#include "DataHandle\DataTypes.h"
#include "StaticGantryCranePosition.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "ArgListTransfer.h"
#include "ArgDataFectcher.h"
#include "GraphicGantryCrane.h"
#include "NewtonIterator.h"
#include "PlanePositionCalulator.h"
#include "NormalGraphic.h"

// CCylinderPosition dialog

class CGantryCranePosition : public CNormalGraphic
{
public:
	CGantryCranePosition(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGantryCranePosition();

public:
	static IGraphicProtocol * CreateProto(){ return new CGantryCranePosition; }
	
	virtual HSString Identifier(){ return "GRAPHIC_GANTRYCRANE_POSITION"; }
	virtual HSString Name(){ return "¡˙√≈µıº‡≤‚"; }

	virtual HSBool Start();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );	

	virtual HSInt LargeIcon(){ return IDB_BITMAP_GANTRY_CRANE; }

	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CGantryCranePosition; }

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
		HSDouble Pos;			
		HSBool Forbid;
		HSBool IsOn;
		DEVICE_CH_IDENTIFIER DataIdentifier;	
		CArgListTransfer *ArgTransfer;				
	} PositionSensor;


	typedef struct PositionInfo
	{
		HSInt Sensor;
		HSInt64 Time;		
		HSDouble Energy;
	} PositionInfo;

	static bool PositionInfoCompare( PositionInfo &t1, PositionInfo &t2 )
	{
		return t1.Time < t2.Time;
	}

public:	
	HSVoid UpdatePositionSensor( HSInt tIndex, HSDouble tPosition, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier );
	PositionSensor * GetPositionSensor( HSInt tIndex );

	HSVoid SetFocusSensor( HSInt tIndex );	

	HSVoid RefreshSensors();
	HSVoid FocusWnd();

	HSVoid SetMaterialLength( HSDouble tLength );
	HSDouble MaterialLength(){ return mMaterialLength; }

	HSString GantryCraneName(){ return mGantryCraneName; }
	HSVoid SetGantryCraneName( HSString tName );

	static bool SensorPosComapre( CGraphicGantryCrane::SensorInfo &t1, CGraphicGantryCrane::SensorInfo &t2 )
	{
		return t1.PosX < t2.PosX;
	}

private:	
	HSVoid InitPositionSensors();	

	HSVoid CalcHitPosition( vector< PositionInfo > &tPositionArgs );

private:
	map< HSInt, PositionSensor > mPositionSensors;

	vector< PositionInfo > mPositionArgs;	
	HSInt64 mLastHitTime;

	CGraphicGantryCrane *mCylinder;

	map< HSInt, CGraphicGantryCrane::SensorInfo > mSensors;
	list< CGraphicGantryCrane::HitPosition > mHitsPosition;

	enum { DEFALUT_SENSOR_NUM = 2 };	

	CStaticGantryCranePosition mStaticCylinder;

	HSDouble mMaterialLength;

	HSString mGantryCraneName;
};
