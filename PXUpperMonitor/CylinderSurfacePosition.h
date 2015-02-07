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


// CCylinderSurfacePosition dialog

class CCylinderSurfacePosition : public CDialogEx, public IGraphicProtocol, public IPlanePositionCalulatorProtocol
{
	DECLARE_DYNAMIC(CCylinderSurfacePosition)

public:
	CCylinderSurfacePosition(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCylinderSurfacePosition();

// Dialog Data
	enum { IDD = IDD_CYLINDERSURFACEPOSITION };

	public:	
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );	

public:
	static IGraphicProtocol * CreateProto(){ return new CCylinderSurfacePosition; }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier(){ return 0; }
	virtual HSString Identifier(){ return "GRAPHIC_CYLINDER_SURFACE_POSITION"; }
	virtual HSString Name(){ return "ÖùÃæ¶¨Î»";  }

	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );
	
	virtual HSVoid FocusGraphic( HSBool tIsFocused );
	virtual CDialog * Wnd(){ return this; }
	virtual HSInt LargeIcon(){ return IDB_BITMAP_CYLINDER_SURFACE_POSITION; }
	virtual HSInt SmallIcon(){ return IDB_BITMAP_CYLINDER_SURFACE_POSITION_LIST; }

	virtual HSInt ResourceID(){ return IDD_CYLINDERSURFACEPOSITION; }	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CCylinderSurfacePosition; }

	virtual HSVoid SetParent( IGraphicWindowProtocol *pParent ){ mParent = pParent; }
	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );
	virtual HSBool ShowFullScreen( HSBool tFullScreen = HSFalse, CWnd *pWnd = NULL )
	{ 
		mIsFullScreen = ( pWnd != NULL ? tFullScreen : mIsFullScreen );
		return mIsFullScreen;
	}

	HSDouble XPosWithAngle( HSDouble tAngle ){ return 3.14159265 * mMaterialRadius / 180.0 * tAngle; }
	HSDouble AngleWithXPos( HSDouble tXPos ){ return tXPos / ( 3.14159265 * mMaterialRadius ) * 180.0; }

	virtual HSVoid GetSensorPosition( HSInt tSensor, HSDouble &tPosX, HSDouble &tPosY ){ tPosX = XPosWithAngle( mPositionSensors[ tSensor ].Angle ); tPosY = mPositionSensors[ tSensor ].Height; }
	virtual HSUInt64 GetSensorHitTime( HSInt tSensor, HSInt tArgIndex ){ return mPositionSensors[ tSensor ].ArgList[ tArgIndex ].NBeginTime; }
	virtual HSDouble DistanceBetweenSensors( HSInt tSensorOne, HSInt tSensorTwo );
	virtual HSDouble DistanceFromSensor( HSDouble tPosX, HSDouble tPosY, HSInt tSensor );

public:
	void OnOK(){}
	void OnCancel(){}

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
	HSVoid FocusWnd();

	HSInt HitRadius(){ return mHitRadius; }
	HSVoid SetHitRadius( HSInt tHitRadius );

private:
	
	
	HSVoid InitPositionSensors();
	HSBool CalcHitPosition( HSInt tSensor, HSUInt tArgIndex );
	HSBool IsHitQualified( HSInt64 tDuration, HSDouble tVelocity, HSInt tSensorOne, HSInt tSensorTwo );
	HSBool IsResQualified( HSDouble tPosX, HSDouble tPosY, vector< CPlanePositionCalulator::HIT_CALC_INFO > &tHitSensors, HSDouble &tEnergy );		

	

private:
	HSBool mInit;
	
	HSBool mIsDraging;
	CPoint mPrevPoint;

	HSBool mIsZoomIn;
	CPoint mZoomInBeginPoint;
	CRect mZoomInRect;
	HSBool mFirstTimeDrawZoomRect;

	IGraphicWindowProtocol *mParent;

	CThreadControlEx< CCylinderSurfacePosition > *mUpdateRelationThread;	

	HSBool mIsFullScreen;

	CMenu mPopupMenu;

	HANDLE mRelationMutex;	
	
	HSInt mDragingSensorIndex;

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

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CStaticCylinderSurfacePosition mStaticCylinderSurface;
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnCylinderSurfacePositionPopupAuto();
	afx_msg void OnCylinderSurfacePositionPopupClose();
	afx_msg void OnCylinderSurfacePositionPopupExportData();
	afx_msg void OnCylinderSurfacePositionPopupFullScreen();
	afx_msg void OnUpdateCylinderSurfacePositionPopupFullScreen(CCmdUI *pCmdUI);
	afx_msg void OnCylinderSurfacePositionPopupReset();
	afx_msg void OnCylinderSurfacePositionPopupSetdatasource();
};
