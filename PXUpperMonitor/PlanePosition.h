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

// CPlanePosition dialog

class CPlanePosition : public CDialogEx, public IGraphicProtocol, public IPlanePositionCalulatorProtocol
{
	DECLARE_DYNAMIC(CPlanePosition)

public:
	CPlanePosition(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPlanePosition();

// Dialog Data
	enum { IDD = IDD_PLANEPOSITION };

public:	
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );	

public:
	static IGraphicProtocol * CreateProto(){ return new CPlanePosition; }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier(){ return 0; }
	virtual HSString Identifier(){ return "GRAPHIC_PLANE_POSITION"; }
	virtual HSString Name(){ return "平面定位";  }

	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );
	
	virtual HSVoid FocusGraphic( HSBool tIsFocused );
	virtual CDialog * Wnd(){ return this; }
	virtual HSInt LargeIcon(){ return IDB_BITMAP_PLANE_POSITION; }
	virtual HSInt SmallIcon(){ return IDB_BITMAP_PLANE_POSITION_LIST; }

	virtual HSInt ResourceID(){ return IDD_PLANEPOSITION; }	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CPlanePosition; }

	virtual HSVoid SetParent( IGraphicWindowProtocol *pParent ){ mParent = pParent; }
	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );
	virtual HSBool ShowFullScreen( HSBool tFullScreen = HSFalse, CWnd *pWnd = NULL )
	{ 
		mIsFullScreen = ( pWnd != NULL ? tFullScreen : mIsFullScreen );
		return mIsFullScreen;
	}

	virtual HSVoid GetSensorPosition( HSInt tSensor, HSDouble &tPosX, HSDouble &tPosY ){ tPosX = mPositionSensors[ tSensor ].XPos; tPosY = mPositionSensors[ tSensor ].YPos; }
	virtual HSUInt64 GetSensorHitTime( HSInt tSensor, HSInt tArgIndex ){ return mPositionSensors[ tSensor ].ArgList[ tArgIndex ].NBeginTime; }
	virtual HSDouble DistanceBetweenSensors( HSInt tSensorOne, HSInt tSensorTwo );
	virtual HSDouble DistanceFromSensor( HSDouble tPosX, HSDouble tPosY, HSInt tSensor );

public:
	void OnOK(){}
	void OnCancel(){}

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
	HSBool mInit;
	
	HSBool mIsDraging;
	CPoint mPrevPoint;

	HSBool mIsZoomIn;
	CPoint mZoomInBeginPoint;
	CRect mZoomInRect;
	HSBool mFirstTimeDrawZoomRect;

	IGraphicWindowProtocol *mParent;

	CThreadControlEx< CPlanePosition > *mUpdateRelationThread;	

	HSBool mIsFullScreen;

	CMenu mPopupMenu;

	HANDLE mRelationMutex;

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

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CStaticPlanePosition mStaticRelation;
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnPlanePositionPopupAuto();
	afx_msg void OnPlanePositionPopupClose();
	afx_msg void OnPlanePositionPopupFullScreen();
	afx_msg void OnUpdatePlanePositionPopupFullScreen(CCmdUI *pCmdUI);
	afx_msg void OnPlanePositionPopupReset();
	afx_msg void OnPlanePositionPopupSetdatasource();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPlanePositionPopupExportData();
	afx_msg void OnPlanePositionPopupCheckCircle();
	afx_msg void OnUpdatePlanePositionPopupCheckCircle(CCmdUI *pCmdUI);
};
