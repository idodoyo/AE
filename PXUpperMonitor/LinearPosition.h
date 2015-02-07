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

// CLinearPosition dialog

class CLinearPosition : public CDialogEx, public IGraphicProtocol
{
	DECLARE_DYNAMIC(CLinearPosition)

public:
	CLinearPosition(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLinearPosition();

// Dialog Data
	enum { IDD = IDD_LINEARPOSITION };

public:	
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );	

public:
	static IGraphicProtocol * CreateProto(){ return new CLinearPosition; }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier(){ return 0; }
	virtual HSString Identifier(){ return "GRAPHIC_LINEAR_POSITION"; }
	virtual HSString Name(){ return "线性定位";  }

	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );
	
	virtual HSVoid FocusGraphic( HSBool tIsFocused );
	virtual CDialog * Wnd(){ return this; }
	virtual HSInt LargeIcon(){ return IDB_BITMAP_LINEAR_POSITION; }
	virtual HSInt SmallIcon(){ return IDB_BITMAP_LINEAR_POSITION_LIST; }

	virtual HSInt ResourceID(){ return IDD_LINEARPOSITION; }	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CLinearPosition; }

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
	HSBool mInit;
	
	HSBool mIsDraging;
	CPoint mPrevPoint;

	HSBool mIsZoomIn;
	CPoint mZoomInBeginPoint;
	CRect mZoomInRect;
	HSBool mFirstTimeDrawZoomRect;

	IGraphicWindowProtocol *mParent;

	CThreadControlEx< CLinearPosition > *mUpdateRelationThread;	

	HSBool mIsFullScreen;

	CMenu mPopupMenu;

	HANDLE mRelationMutex;

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

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CStaticLinearPosition mStaticRelation;
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLinearPositionPopupClose();
	afx_msg void OnLinearPositionPopupFullScreen();
	afx_msg void OnUpdateLinearPositionPopupFullScreen(CCmdUI *pCmdUI);
	afx_msg void OnLinearPositionPopupReset();
	afx_msg void OnLinearPositionPopupSetdatasource();
	afx_msg void OnLinearPositionPopupAuto();
	afx_msg void OnLinearPositionPopupExportData();
	afx_msg void OnLinearPositionPopupCheckCircle();
	afx_msg void OnUpdateLinearPositionPopupCheckCircle(CCmdUI *pCmdUI);
};
