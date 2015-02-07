#pragma once

#include "DataHandle\DataTypes.h"
#include "afxwin.h"
#include "StaticLinearPosition.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "ArgListTransfer.h"
#include "ArgDataFectcher.h"
#include "NormalGraphic.h"

// CLinearPosition dialog

class CLinearPosition : public CNormalGraphic
{
public:
	CLinearPosition(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLinearPosition();	

public:
	static IGraphicProtocol * CreateProto(){ return new CLinearPosition; }
	
	virtual HSString Identifier(){ return "GRAPHIC_LINEAR_POSITION"; }
	virtual HSString Name(){ return "线性定位";  }

	virtual HSBool Start();	

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );	
	
	virtual HSInt LargeIcon(){ return IDB_BITMAP_LINEAR_POSITION; }
	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CLinearPosition; }
	
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
		HSDouble Pos;
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

	HSVoid UpdatePositionSensor( HSInt tIndex, HSDouble tPosition, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier );
	PositionSensor * GetPositionSensor( HSInt tIndex );

	HSVoid SetFocusSensor( HSInt tIndex );
	HSVoid RefreshSensors();

	HSDouble XAxisLength(){ return mXAxisLength; }
	HSVoid SetXAxisLength( HSDouble tLength );

	HSInt HitRadius(){ return mHitRadius; }
	HSVoid SetHitRadius( HSInt tHitRadius );

	HSDouble GetCheckRadius(){ return mCheckRadius; }
	HSVoid SetCheckRadius( HSDouble tValue );
	HSDouble GetCheckPos(){ return mStaticRelation.XCheckPos(); }
	HSVoid SetCheckPosX( HSDouble tValue );	

private:
	HSVoid InitPositionSensors();
	HSBool CalcHitPosition( HSDouble tPos, HSUInt64 tTime, HSUInt64 tIncreaseTime, HSDouble tEnergy );
	HSDouble DistanceWithX( HSDouble tXPosOne, HSDouble tXPosTue );

private:
	list< CStaticLinearPosition::HitPosition > mHitsPosition;
	vector< CStaticLinearPosition::SensorInfo > mSensors;

	map< HSInt, PositionSensor > mPositionSensors;
	HSDouble mMaterialLength;
	
	HSInt mDragingSensorIndex;

	HSDouble mXAxisLength;
	HSInt mHitRadius;

	HSDouble mCheckRadius;
	HSBool mDragCheckCircle;
	HSBool mCheckCircleEnable;

	CStaticLinearPosition mStaticRelation;
};
