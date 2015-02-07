#pragma once

#include "DataHandle\DataTypes.h"
#include "afxwin.h"
#include "StaticPlanePosition.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "ArgListTransfer.h"
#include "ArgDataFectcher.h"
#include "PlanePositionCalulator.h"
#include "NormalGraphic.h"

// CPlanePosition dialog

class CPlanePosition : public CNormalGraphic, public IPlanePositionCalulatorProtocol
{
public:
	CPlanePosition(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPlanePosition();

public:
	static IGraphicProtocol * CreateProto(){ return new CPlanePosition; }
	
	virtual HSString Identifier(){ return "GRAPHIC_PLANE_POSITION"; }
	virtual HSString Name(){ return "平面定位";  }

	virtual HSBool Start();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );	
	
	virtual HSInt LargeIcon(){ return IDB_BITMAP_PLANE_POSITION; }

	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CPlanePosition; }
	
	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );	

public:
	virtual HSVoid GetSensorPosition( HSInt tSensor, HSDouble &tPosX, HSDouble &tPosY ){ tPosX = mPositionSensors[ tSensor ].XPos; tPosY = mPositionSensors[ tSensor ].YPos; }
	virtual HSUInt64 GetSensorHitTime( HSInt tSensor, HSInt tArgIndex ){ return mPositionSensors[ tSensor ].ArgList[ tArgIndex ].NBeginTime; }
	virtual HSDouble DistanceBetweenSensors( HSInt tSensorOne, HSInt tSensorTwo );
	virtual HSDouble DistanceFromSensor( HSDouble tPosX, HSDouble tPosY, HSInt tSensor );

public:
	virtual HSBool GraphicNeedRefresh();	

	virtual HSVoid InitDialog();

	virtual HSVoid StepGraphic( HSInt tDelta );
	virtual HSVoid ZoomInGraphic( CRect &tViewRect );
	virtual HSVoid DragGraphic( CRect &tViewRect, CPoint tPoint );

	virtual HSBool IsDataExportNeeded();
	virtual HSVoid ExportData( CDataExportHelper &tDataExport );

	virtual HSVoid ResetData( CMainFrame* pMainFrame );

	virtual HSVoid MouseDownWhenPointer(  CRect &tRect, CPoint tPoint );	
	virtual HSBool CustomMouseUpCheck();
	virtual HSVoid MouseUpFinish();
	virtual HSBool CustomMouseMoveCheck( CPoint tPoint );

	virtual HSVoid AutoSetSensors();
	virtual HSVoid SwitchCheckCircle();
	virtual HSBool SwitchCheckCircleEnabled(){ return mCheckCircleEnable; }	

public:
	typedef struct PositionSensor
	{
		HSDouble XPos;
		HSDouble YPos;
		HSBool Forbid;
		HSBool IsOn;
		DEVICE_CH_IDENTIFIER DataIdentifier;	
		CArgListTransfer *ArgTransfer;		
		vector< CArgTableManager::ArgItem > ArgList;
		vector< HSBool > ArgHandled;
		HSInt Index;
	} PositionSensor;

public:	
	HSDouble MaterialLength(){ return mMaterialLength; }
	HSVoid SetMaterialLength( HSDouble tLength );
	HSDouble MaterialWidth(){ return mMaterialWidth; }
	HSVoid SetMaterialWidth( HSDouble tWidth );

	HSVoid UpdatePositionSensor( HSInt tIndex, HSDouble tPositionX, HSDouble tPositionY, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier );
	PositionSensor * GetPositionSensor( HSInt tIndex );

	HSVoid SetFocusSensor( HSInt tIndex );	
	
	HSDouble XAxisLength(){ return mXAxisLength; }
	HSVoid SetXAxisLength( HSDouble tLength );

	HSDouble YAxisLength(){ return mYAxisLength; }
	HSVoid SetYAxisLength( HSDouble tLength );

	HSInt HitRadius(){ return mHitRadius; }
	HSVoid SetHitRadius( HSInt tHitRadius );	

	HSVoid RefreshSensors();

	HSDouble GetCheckRadius(){ return mCheckRadius; }
	HSVoid SetCheckRadius( HSDouble tValue );
	HSVoid GetCheckPos( HSDouble &tX, HSDouble &tY ){ tX = mStaticRelation.XCheckPos(); tY = mStaticRelation.YCheckPos(); }
	HSVoid SetCheckPosX( HSDouble tValue );
	HSVoid SetCheckPosY( HSDouble tValue );

private:	
	HSVoid InitPositionSensors();
	HSBool CalcHitPosition( HSInt tSensor, HSUInt tArgIndex );
	HSBool IsHitQualified( HSInt64 tDuration, HSDouble tVelocity, HSInt tSensorOne, HSInt tSensorTwo );
	HSBool IsResQualified( HSDouble tXResPos, HSDouble tYResPos, vector< CPlanePositionCalulator::HIT_CALC_INFO > &tHitSensors, HSDouble &tEnergy );	

private:
	list< CStaticPlanePosition::HitPosition > mHitsPosition;
	vector< CStaticPlanePosition::SensorInfo > mSensors;

	map< HSInt, PositionSensor > mPositionSensors;
	HSDouble mMaterialLength;
	HSDouble mMaterialWidth;
	
	HSInt mDragingSensorIndex;

	HSDouble mXAxisLength;
	HSDouble mYAxisLength;
	HSInt mHitRadius;	

	CPlanePositionCalulator mPositionCalulator;

	HSDouble mCheckRadius;
	HSBool mDragCheckCircle;
	HSBool mCheckCircleEnable;	

	CStaticPlanePosition mStaticRelation;
};
