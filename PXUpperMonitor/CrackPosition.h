#pragma once

#include "DataHandle\DataTypes.h"
#include "afxwin.h"
#include "StaticCrackPosition.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "ArgListTransfer.h"
#include "ArgDataFectcher.h"
#include "GrahamCalLine.h"
#include "NormalGraphic.h"

// CCrackPosition dialog

class CCrackPosition : public CNormalGraphic
{
public:
	CCrackPosition(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCrackPosition();

public:
	static IGraphicProtocol * CreateProto(){ return new CCrackPosition; }
	
	virtual HSString Identifier(){ return "GRAPHIC_CRACK_POSITION"; }
	virtual HSString Name(){ return "�ѷ춨λ";  }

	virtual HSBool Start();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );	
	
	virtual HSInt LargeIcon(){ return IDB_BITMAP_CRACK_POSITION; }
	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CCrackPosition; }

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
	virtual HSVoid MouseMoveOtherWork( CPoint tPoint );

	virtual HSVoid AutoSetSensors();
	virtual HSVoid SwitchCheckCircle();
	virtual HSBool SwitchCheckCircleEnabled(){ return mCheckCircleEnable; }	

	virtual HSVoid SwitchOutLine();
	virtual HSBool SwitchOutLineEnabled(){ return mIsShowOutLine; }	

	virtual HSVoid SwitchCheckAngle();
	virtual HSBool SwitchCheckAngleEnabled(){ return mIsShowAngleLine; }	

public:
	typedef struct PositionSensor
	{
		HSDouble XPos;
		HSDouble YPos;		
		HSDouble Radius;
		HSDouble Angle;
		HSDouble AxisPosX;
		HSDouble AxisPosY;
		HSBool Forbid;
		HSBool IsOn;
		DEVICE_CH_IDENTIFIER DataIdentifier;	
		CArgListTransfer *ArgTransfer;		
		vector< CArgTableManager::ArgItem > ArgList;
		vector< HSBool > ArgHandled;
		HSInt Index;
	} PositionSensor;

	typedef struct HIT_CALC_INFO
	{
		HSInt Sensor;
		HSInt ArgIndex;
	} HIT_CALC_INFO;

public:	
	HSDouble MaterialLength(){ return mMaterialLength; }
	HSVoid SetMaterialLength( HSDouble tLength );
	HSDouble MaterialWidth(){ return mMaterialWidth; }
	HSVoid SetMaterialWidth( HSDouble tWidth );

	HSVoid UpdatePositionSensor( HSInt tIndex, HSDouble tPositionX, HSDouble tPositionY, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier );
	PositionSensor * GetPositionSensor( HSInt tIndex );

	list< CStaticCrackPosition::HitPosition > * HitsPosition(){ return &mHitsPosition; }

	HSVoid SetFocusSensor( HSInt tIndex );	
	
	HSDouble XAxisLength(){ return mXAxisLength; }
	HSVoid SetXAxisLength( HSDouble tLength );

	HSDouble YAxisLength(){ return mYAxisLength; }
	HSVoid SetYAxisLength( HSDouble tLength );

	HSDouble CenterXCoord(){ return mCenterXCoord; }
	HSVoid SetCenterXCoord( HSDouble tValue ){ mCenterXCoord = tValue; }

	HSDouble CenterYCoord(){ return mCenterYCoord; }
	HSVoid SetCenterYCoord( HSDouble tValue ){ mCenterYCoord = tValue; }

	HSInt HitRadius(){ return mHitRadius; }
	HSVoid SetHitRadius( HSInt tHitRadius );

	HSVoid RefreshSensors();

	HSDouble GetCheckRadius(){ return mCheckRadius; }
	HSVoid SetCheckRadius( HSDouble tValue );
	HSVoid GetCheckPos( HSDouble &tX, HSDouble &tY ){ tX = mStaticRelation.XCheckPos(); tY = mStaticRelation.YCheckPos(); }
	HSVoid SetCheckPosX( HSDouble tValue );
	HSVoid SetCheckPosY( HSDouble tValue );

	HSVoid GetSensorCircleInfo( HSDouble &tFirstCircle, HSDouble &tSecondCircle, HSDouble &tStartAngle );
	HSVoid SetSensorCircleInfo( HSDouble tFirstCircle, HSDouble tSecondCircle, HSDouble tStartAngle );

	HSVoid PosWithGPSInfo( HSDouble tLongitude, HSDouble tLatitude, HSDouble tAngle, HSDouble tDistance, HSDouble &tResLongitude, HSDouble &tResLatitude );

	HSVoid SetLastGPSModifiedSensor( HSInt tIndex ){ mLastSensorSetGPS = tIndex; }

private:	
	HSVoid InitPositionSensors();
	HSBool CrossPosition( HSDouble tPosX, HSDouble tPosY, CStaticCrackPosition::OutLinePos tResVerPos[ 2 ], CStaticCrackPosition::OutLinePos tResHorPos[ 2 ] );	
	HSBool CalcHitPosition( HSInt tSensor, HSUInt tArgIndex );
	HSDouble DistanceBetweenSensors( HSInt tSensorOne, HSInt tSensorTwo );
	HSBool IsHitQualified( HSInt64 tDuration, HSDouble tVelocity, HSInt tSensorOne, HSInt tSensorTwo );
	HSDouble DistanceFromSensor( HSDouble tPosX, HSDouble tPosY, HSInt tSensor );
	HSBool GetHitPosition( vector< HIT_CALC_INFO > &tHitSensors, CStaticCrackPosition::HitPosition &tResHitPos );
	HSVoid CreateVirtualHits();
	HSVoid GetTheNearest3Sensor( HSDouble tX, HSDouble tY, HSInt &tChannel1, HSInt &tChannel2, HSInt &tChannel3 );

private:
	list< CStaticCrackPosition::HitPosition > mHitsPosition;
	vector< CStaticCrackPosition::SensorInfo > mSensors;

	map< HSInt, PositionSensor > mPositionSensors;
	HSDouble mMaterialLength;
	HSDouble mMaterialWidth;
	
	HSInt mDragingSensorIndex;

	HSDouble mXAxisLength;
	HSDouble mYAxisLength;
	HSInt mHitRadius;

	HSDouble mCenterXCoord;
	HSDouble mCenterYCoord;

	HSDouble mCheckRadius;
	HSBool mDragCheckCircle;
	HSBool mCheckCircleEnable;

	HSDouble mSensorFirstCircle;
	HSDouble mSensorSecondCircle;
	HSDouble mSensorStartAngle;

	HSInt mLastSensorSetGPS;

	list< CStaticCrackPosition::OutLinePos > mOutLinePoints;
	HSBool mIsShowOutLine;

	HSBool mIsShowAngleLine;

	CStaticCrackPosition mStaticRelation;
};
