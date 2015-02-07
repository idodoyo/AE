#pragma once
#include "afxwin.h"

#include "DataHandle\DataTypes.h"
#include "StaticSpherePosition.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "ArgListTransfer.h"
#include "ArgDataFectcher.h"
#include "GraphicSphere.h"
#include "NewtonIterator.h"


// CSpherePosition dialog

class CSpherePosition : public CDialogEx, public IGraphicProtocol, public INewtonIteratorProtocol< 3 >
{
	DECLARE_DYNAMIC(CSpherePosition)

public:
	CSpherePosition(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSpherePosition();

// Dialog Data
	enum { IDD = IDD_SPHEREPOSITION };

public:	
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );	

public:
	static IGraphicProtocol * CreateProto(){ return new CSpherePosition; }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier(){ return 0; }
	virtual HSString Identifier(){ return "GRAPHIC_SPHERE_POSITION"; }
	virtual HSString Name(){ return "«Ú√Ê∂®Œª";  }

	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );
	
	virtual HSVoid FocusGraphic( HSBool tIsFocused );
	virtual CDialog * Wnd(){ return this; }
	virtual HSInt LargeIcon(){ return IDB_BITMAP_SPHERE; }
	virtual HSInt SmallIcon(){ return IDB_BITMAP_SPHERE_LIST; }

	virtual HSInt ResourceID(){ return IDD_SPHEREPOSITION; }	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CSpherePosition; }

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
		HSDouble AngleDirction;
		HSDouble AngleUp;
		HSBool Forbid;
		HSBool IsOn;
		DEVICE_CH_IDENTIFIER DataIdentifier;	
		CArgListTransfer *ArgTransfer;		
		vector< CArgTableManager::ArgItem > ArgList;
		vector< HSBool > ArgHandled;
		HSInt Index;
	} PositionSensor;

public:	
	HSVoid UpdatePositionSensor( HSInt tIndex, HSDouble tAngleD, HSDouble tAngleU, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier );
	PositionSensor * GetPositionSensor( HSInt tIndex );

	HSVoid SetFocusSensor( HSInt tIndex );
	HSVoid SetMaterialRadius( HSDouble tRadius ){ mMaterialRadius = tRadius; }
	HSDouble MaterialRadius(){ return mMaterialRadius; }
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
	HSBool IsResQualified( HSDouble tAngleD, HSDouble tAngleU, vector< CSpherePosition::HitCalcInfo > &tHitSensors, HSDouble &tEnergy );	
	HSBool PositionWithHitSensor( HSDouble tInitX0[ 3 ], vector< HitCalcInfo > &tHitSensors, vector< HSInt > &tHitSensorsIndex, vector< CGraphicSphere::HitPosition > &tHitPos );
	HSDouble DistanceWithSensorIndex( HSDouble tAngleD, HSDouble tAngleU, HSInt tSensor );			
	HSDouble CorrectAngle( HSDouble tAngle, HSDouble tTop );

	static HSDouble AtoR( HSDouble tAngle ){ return 3.14159265 * tAngle / 180.0; }
	static HSDouble RtoA( HSDouble tRadius ){ return tRadius * 180.0 / 3.14159265; }

	virtual HSVoid GetFunctionMValue( HSDouble tX0[ 3 ], HSDouble *tPoints[ 3 ], HSDouble tRes[ 3 ][ 3 ] );	
	virtual HSVoid GetFunctionValue( HSDouble tX0[ 3 ], HSDouble *tPoints[ 3 ], HSDouble tRes[ 3 ] );

	static bool HitSensorComapre( HitCalcInfo &t1, HitCalcInfo &t2 );
	static bool HitSensorDistanceComapre( SensorDistanceInfo &t1, SensorDistanceInfo &t2 );

private:
	HSBool mInit;
	
	HSBool mIsDraging;
	CPoint mPrevPoint;

	HSBool mIsZoomIn;
	CPoint mZoomInBeginPoint;
	CRect mZoomInRect;
	HSBool mFirstTimeDrawZoomRect;

	IGraphicWindowProtocol *mParent;

	CThreadControlEx< CSpherePosition > *mUpdateRelationThread;	

	HSBool mIsFullScreen;

	CMenu mPopupMenu;

	HANDLE mRelationMutex;	
	
	HSInt mDragingSensorIndex;

	map< HSInt, PositionSensor > mPositionSensors;	

	CGraphicSphere *mSphere;

	map< HSInt, CGraphicSphere::SensorInfo > mSensors;
	list< CGraphicSphere::HitPosition > mHitsPosition;

	HSDouble mMaterialRadius;

	HSInt mHitRadius;

	enum { MAX_SENSOR_NUM_IN_CENTER_LINE = 8, DEFALUT_SENSOR_NUM = 21 };

	CNewtonIterator< 3 > mNewtonIterator;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CStaticSpherePosition mStaticSphere;
	afx_msg void OnDestroy();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSpherePositionPopupAuto();
	afx_msg void OnSpherePositionPopupClose();
	afx_msg void OnSpherePositionPopupFullScreen();
	afx_msg void OnUpdateSpherePositionPopupFullScreen(CCmdUI *pCmdUI);
	afx_msg void OnSpherePositionPopupReset();
	afx_msg void OnSphereePositionPopupSetdatasource();
	afx_msg void OnSphereePositionPopupRotate();
	afx_msg void OnSphereePositionPopupExportData();
};
