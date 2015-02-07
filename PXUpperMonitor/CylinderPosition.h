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

// CCylinderPosition dialog

class CCylinderPosition : public CDialogEx, public IGraphicProtocol, public INewtonIteratorProtocol< 4 >
{
	DECLARE_DYNAMIC(CCylinderPosition)

public:
	CCylinderPosition(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCylinderPosition();

// Dialog Data
	enum { IDD = IDD_CYLINDERPOSITION };

public:	
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );	

public:
	static IGraphicProtocol * CreateProto(){ return new CCylinderPosition; }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier(){ return 0; }
	virtual HSString Identifier(){ return "GRAPHIC_CYLINDER_POSITION"; }
	virtual HSString Name(){ return "方体定位";  }

	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );
	
	virtual HSVoid FocusGraphic( HSBool tIsFocused );
	virtual CDialog * Wnd(){ return this; }
	virtual HSInt LargeIcon(){ return IDB_BITMAP_CYLINDER_POSITION; }
	virtual HSInt SmallIcon(){ return IDB_BITMAP_CYLINDER_POSITION_LIST; }

	virtual HSInt ResourceID(){ return IDD_CYLINDERPOSITION; }	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CCylinderPosition; }

	virtual HSVoid SetParent( IGraphicWindowProtocol *pParent ){ mParent = pParent; }
	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );
	virtual HSBool ShowFullScreen( HSBool tFullScreen = HSFalse, CWnd *pWnd = NULL )
	{ 
		mIsFullScreen = ( pWnd != NULL ? tFullScreen : mIsFullScreen );
		return mIsFullScreen;
	}

public:
	void OnOK(){}
	void OnCancel(){}

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
	HSBool mInit;
	
	HSBool mIsDraging;
	CPoint mPrevPoint;

	HSBool mIsZoomIn;
	CPoint mZoomInBeginPoint;
	CRect mZoomInRect;
	HSBool mFirstTimeDrawZoomRect;

	IGraphicWindowProtocol *mParent;

	CThreadControlEx< CCylinderPosition > *mUpdateRelationThread;	

	HSBool mIsFullScreen;

	CMenu mPopupMenu;

	HANDLE mRelationMutex;	
	
	HSInt mDragingSensorIndex;

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

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CStaticCylinderPosition mStaticCylinder;
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnDestroy();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	virtual BOOL OnInitDialog();
	afx_msg void OnCylinderPositionPopupAuto();
	afx_msg void OnCylinderPositionPopupClose();
	afx_msg void OnCylinderPositionPopupExportData();
	afx_msg void OnCylinderPositionPopupFullScreen();
	afx_msg void OnUpdateCylinderPositionPopupFullScreen(CCmdUI *pCmdUI);
	afx_msg void OnCylinderPositionPopupReset();
	afx_msg void OnCylinderPositionPopupSetdatasource();
};
