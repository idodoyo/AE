#pragma once

#include "DataHandle\DataTypes.h"
#include "afxwin.h"
#include "StaticGrid.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "SignalAttenuationCurveProperty.h"
#include "FourierTransform.h"
#include "ArgListTransfer.h"
#include "DataExportHelper.h"
#include "GraphicNormal3D.h"


// CNormalGraphic dialog

class CMainFrame;

class CNormalGraphic : public CDialogEx, public IGraphicProtocol, public IGraphic3DProtocol
{
	DECLARE_DYNAMIC(CNormalGraphic)

public:
	CNormalGraphic(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNormalGraphic();

// Dialog Data
	enum { IDD = IDD_NORMALGRAPHIC };

public:	
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

	IDataHandler * DataHandler(){ return mDataHandler; }

public:	
	virtual DEVICE_CH_IDENTIFIER DataIdentifier(){ return mDataIdentifier; }	
	
	virtual HSVoid Pause();
	virtual HSVoid Stop();
	
	virtual HSVoid FocusGraphic( HSBool tIsFocused );
	virtual CDialog * Wnd(){ return this; }	
	virtual HSInt SmallIcon(){ return IDB_BITMAP_ARROW_LIST; }

	virtual HSInt ResourceID(){ return IDD_NORMALGRAPHIC; }			

	virtual HSVoid SetParent( IGraphicWindowProtocol *pParent ){ mParent = pParent; }

	virtual HSBool ShowFullScreen( HSBool tFullScreen = HSFalse, CWnd *pWnd = NULL )
	{ 
		mIsFullScreen = ( pWnd != NULL ? tFullScreen : mIsFullScreen );
		return mIsFullScreen;
	}

public:
	virtual HSBool GraphicNeedRefresh(){ return HSTrue; }
	virtual HSVoid GraphicStopRefresh(){}

	virtual HSVoid InitDialog(){}
	virtual HSVoid ViewResized( CRect &tRect );

	virtual HSVoid StepGraphic( HSInt tDelta ){}
	virtual HSVoid ZoomInGraphic( CRect &tViewRect ){}
	virtual HSVoid DragGraphic( CRect &tViewRect, CPoint tPoint ){}

	virtual HSBool IsDataExportNeeded(){ return HSFalse; }
	virtual HSVoid ExportData( CDataExportHelper &tDataExport ){}

	virtual HSVoid ResetData( CMainFrame* pMainFrame ){}
	virtual HSVoid ResetGrahpic(){}

	virtual HSVoid MouseDownWhenPointer( CRect &tRect, CPoint tPoint ){}
	virtual HSBool CustomMouseUpCheck(){ return HSFalse; }
	virtual HSVoid MouseUpFinish(){}
	virtual HSBool CustomMouseMoveCheck( CPoint tPoint ){ return HSFalse; }
	virtual HSVoid MouseMoveOtherWork( CPoint tPoint ){}

	virtual HSVoid AutoSetSensors(){}
	virtual HSVoid SwitchCheckCircle(){}
	virtual HSBool SwitchCheckCircleEnabled(){ return HSFalse; }	

	virtual HSVoid SwitchOutLine(){}
	virtual HSBool SwitchOutLineEnabled(){ return HSFalse; }	

	virtual HSVoid SwitchCheckAngle(){}
	virtual HSBool SwitchCheckAngleEnabled(){ return HSFalse; }	

	virtual HSVoid Graphic3DRefresh( HSBool tNeedInvalidate ){}
	virtual HSVoid AutoRatate(){}

public:
	virtual HSVoid MouseMove( UINT nFlags, CPoint point ){ this->OnMouseMove( nFlags, point ); }
	virtual HSVoid MouseUp( UINT nFlags, CPoint point ){ this->OnLButtonUp( nFlags, point ); }
	virtual HSVoid FocusWnd(){}

public:
	void OnOK(){}
	void OnCancel(){}

protected:
	HSBool mIsDraging;
	CPoint mPrevPoint;

	HSBool mIsZoomIn;
	CPoint mZoomInBeginPoint;
	CRect mZoomInRect;
	HSBool mFirstTimeDrawZoomRect;

	IGraphicWindowProtocol *mParent;

	HSBool mIsFinishInit;	

	CThreadControlEx< CNormalGraphic > *mRefreshThread;
	HSUInt mRefreshMiniSecond;

	IDataHandler *mDataHandler;	

	DEVICE_CH_IDENTIFIER mDataIdentifier;

	HANDLE mDataMutex;

	HSBool mIsFullScreen;

	CMenu mPopupMenu;	

	CBrush mZoomBrush;	

	CStaticGrid *mStaticGraphic;

	HSBool mStopWhenMouseDown;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnDestroy();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnNormalPopupClose();
	afx_msg void OnNormalPopupExportdata();
	afx_msg void OnNormalPopupFullScreen();
	afx_msg void OnUpdateNormalPopupFullScreen(CCmdUI *pCmdUI);
	afx_msg void OnNormalPopupReset();
	afx_msg void OnNormalPopupResetGraphic();
	afx_msg void OnNormalPopupSetdatasource();
	afx_msg void OnPositionPopupAuto();
	afx_msg void OnPositionPopupCheckCircle();
	afx_msg void OnUpdatePositionPopupCheckCircle(CCmdUI *pCmdUI);
	afx_msg void OnPositionPopupRotate();
	afx_msg void OnPositionPopupCheckAngle();
	afx_msg void OnUpdatePositionPopupCheckAngle(CCmdUI *pCmdUI);
	afx_msg void OnPositionPopupOutline();
	afx_msg void OnUpdatePositionPopupOutline(CCmdUI *pCmdUI);
	afx_msg void OnPositionPopupCalCoord();
};
