#pragma once

#include "DataHandle\DataTypes.h"
#include "afxwin.h"
#include "StaticGraphic.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "WaveProperty.h"


// CGraphicWave dialog

class CGraphicWave : public CDialogEx, public IGraphicProtocol
{
	DECLARE_DYNAMIC(CGraphicWave)

public:
	CGraphicWave(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicWave();

// Dialog Data
	enum { IDD = IDD_GRAPHICWAVE };

public:	
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

	HSVoid SetTime( HSDouble tSecond );
	HSVoid SetVoltage( HSDouble tVoltage );

	HSDouble RefreshTime(){ return mTotalSecond; }
	HSDouble TotalVoltage(){ return mTotalVoltage; }
	IDataHandler * DataHandler(){ return mDataHandler; }

public:
	static IGraphicProtocol * CreateProto();

	virtual DEVICE_CH_IDENTIFIER DataIdentifier();
	virtual HSString Identifier();
	virtual HSString Name();

	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );
	
	virtual HSVoid FocusGraphic( HSBool tIsFocused );
	virtual CDialog * Wnd();
	virtual HSInt LargeIcon();
	virtual HSInt SmallIcon();

	virtual HSInt ResourceID(){ return IDD_GRAPHICWAVE; }	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone();	

	virtual HSVoid SetParent( IGraphicWindowProtocol *pParent );	
	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );
	virtual HSBool ShowFullScreen( HSBool tFullScreen = HSFalse, CWnd *pWnd = NULL )
	{ 
		mIsFullScreen = ( pWnd != NULL ? tFullScreen : mIsFullScreen );
		return mIsFullScreen;
	}

	void OnOK(){}
	void OnCancel(){}

private:
	HSVoid RefreshWave();

private:
	HSBool mIsDraging;
	CPoint mPrevPoint;

	HSBool mIsZoomIn;
	CPoint mZoomInBeginPoint;
	CRect mZoomInRect;
	HSBool mFirstTimeDrawZoomRect;

	IGraphicWindowProtocol *mParent;

	HSBool mIsFinishInit;

	CLinearTransfer *mDataTransfer;
	IDataHandler *mDataHandler;
	HSUInt mChannel;

	HSChar *mBuffer;	

	CThreadControlEx< CGraphicWave > *mUpdateWaveThread;

	DEVICE_CH_IDENTIFIER mDataIdentifier;

	HANDLE mDataHandlerMutex;
	HSDouble mRefreshSecond;	

	HSDouble mTotalVoltage;
	HSDouble mTotalSecond;

	HSBool mIsFullScreen;

	CMenu mPopupMenu;

	HSDouble mBeginSecondOffset;
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizeClipboard(CWnd* pClipAppWnd, HGLOBAL hRect);
private:
	CStaticGraphic mStaticWave;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnWavepopupExportdata();
	afx_msg void OnWavepopupClose();
	afx_msg void OnWavepopupSetdatasource();
	afx_msg void OnWavepopupZoomin();
	afx_msg void OnWavepopupZoomout();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnWavepopupReset();	
	afx_msg void OnWavepopupFullScreen();
	afx_msg void OnUpdateWavepopupFullScreen(CCmdUI *pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
};
