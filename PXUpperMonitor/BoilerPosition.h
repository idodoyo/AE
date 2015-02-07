#pragma once

#include "DataHandle\DataTypes.h"
#include "afxwin.h"
#include "StaticBoilerPosition.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "ArgListTransfer.h"
#include "ArgDataFectcher.h"
#include "NormalGraphic.h"

// CBoilerPosition dialog

class CBoilerPosition : public CNormalGraphic
{
public:
	CBoilerPosition(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBoilerPosition();

public:
	static IGraphicProtocol * CreateProto(){ return new CBoilerPosition; }
	
	virtual HSString Identifier(){ return "GRAPHIC_BOILER_POSITION"; }
	virtual HSString Name(){ return "¹øÂ¯¶¨Î»";  }

	virtual HSBool Start();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );	
	
	virtual HSInt LargeIcon(){ return IDB_BITMAP_GL; }

	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CBoilerPosition; }
	
	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );

public:
	virtual HSBool GraphicNeedRefresh();	

	virtual HSVoid InitDialog();

	virtual HSVoid StepGraphic( HSInt tDelta );
	virtual HSVoid ZoomInGraphic( CRect &tViewRect );
	virtual HSVoid DragGraphic( CRect &tViewRect, CPoint tPoint );

	virtual HSBool IsDataExportNeeded();
	virtual HSVoid ExportData( CDataExportHelper &tDataExport );

	virtual HSVoid ResetData( CMainFrame* pMainFrame );		

public:

	enum MATERIAL_TYPE{ MATERIAL_IRON_STONE, MATERIAL_COKE };

	typedef struct ChannelFeedMaterial
	{
		HSUInt64 Time;
		MATERIAL_TYPE Material;
		HSInt Sensor;
		HSDouble Energy;
	} ChannelFeedMaterial;

	typedef struct PositionSensor
	{
		HSDouble XPos;
		HSDouble YPos;
		DEVICE_CH_IDENTIFIER DataIdentifier;
		CArgListTransfer *ArgTransfer;
		vector< CArgTableManager::ArgItem > ArgList;
		HSInt Index;
	} PositionSensor;

	typedef struct BoilerFeedMaterial
	{
		HSUInt64 BeginTime;
		HSUInt64 EndTime;
		MATERIAL_TYPE Material;
		HSInt BeginSensor;		
		vector< HSDouble > SensorsEnergy[ 4 ];
	} BoilerFeedMaterial;	

public:
	static bool FeedMaterialComapre( ChannelFeedMaterial &t1, ChannelFeedMaterial &t2 )
	{
		return t1.Time < t2.Time;
	}

public:	
	HSVoid UpdatePositionSensor( HSInt tIndex, DEVICE_CH_IDENTIFIER tDataIdentifier );
	PositionSensor * GetPositionSensor( HSInt tIndex );

	HSVoid SetFocusSensor( HSInt tIndex );	

	HSVoid RefreshSensors();

	HSDouble MaterailRadius(){ return mMaterialRadius; }
	HSVoid SetMaterailRadius( HSDouble tRadius );

	HSDouble XAxisLength(){ return mXAxisLength; }
	HSVoid SetXAxisLength( HSDouble tValue );

	HSDouble YAxisLength(){ return mYAxisLength; }
	HSVoid SetYAxisLength( HSDouble tValue );

	HSDouble PillarLength( HSInt tIndex ){ return mPillarsLength[ tIndex ]; }
	HSVoid SetPillarLength( HSInt tIndex, HSDouble tValue );

	HSVoid IronStoneInfo( HSDouble &tCycleNum, HSDouble &tCycleTime ){ tCycleNum = mIronStoneCycleNum; tCycleTime = mIronStoneCycleTime; }
	HSVoid SetIronStoneInfo( HSDouble tCycleNum, HSDouble tCycleTime );

	HSVoid CokeInfo( HSDouble &tCycleNum, HSDouble &tCycleTime ){ tCycleNum = mCokeCycleNum; tCycleTime = mCokeCycleTime; }
	HSVoid SetCokeInfo( HSDouble tCycleNum, HSDouble tCycleTime );

	HSString BoilerName(){ return mBoilerName; }
	HSVoid SetBoilerName( HSString tName );

private:	
	HSVoid InitPositionSensors();
	HSVoid FindChannelFeedMaterialInfo( HSInt tSensor, vector< ChannelFeedMaterial > &tChannelsFeedMaterial );
	HSVoid FindBoilerFeedMaterialInfo( vector< ChannelFeedMaterial > &tChannelsFeedMaterial );
	HSVoid ShowFeedMaterial( HSInt tIndex );
	HSBool IsFeedMaterialOK( BoilerFeedMaterial *pFeedMaterial, HSDouble *pRealCycleNum = NULL );

private:	
	vector< CStaticBoilerPosition::SensorInfo > mSensors;

	map< HSInt, PositionSensor > mPositionSensors;
	list< BoilerFeedMaterial > mBoilerFeedMaterialInfo;	

	vector< ChannelFeedMaterial > mChannelsFeedMaterial;	

	CStaticBoilerPosition mStaticRelation;

	HSDouble mMaterialRadius;

	HSDouble mXAxisLength;
	HSDouble mYAxisLength;

	vector< HSDouble > mPillarsLength;

	HSDouble mIronStoneCycleNum;
	HSDouble mIronStoneCycleTime;

	HSDouble mCokeCycleNum;
	HSDouble mCokeCycleTime;

	HSString mBoilerName;

	HSBool mStartFeedMaterial;

	HSInt mCurShowFeedMaterialIndex;	

	HSInt mCurFeedMaterialStartSensor;
};
