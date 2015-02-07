#pragma once

#include "DataHandle\DataTypes.h"
#include "afxwin.h"
#include "StaticFFT.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "GraphicFFTProperty.h"
#include "FourierTransform.h"
#include "ArgListTransfer.h"


// CGraphicFFT dialog

class CGraphicFFT : public CDialogEx, public IGraphicProtocol, public IFourierTransformResHandleProtocol
{
	DECLARE_DYNAMIC(CGraphicFFT)

public:
	CGraphicFFT(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicFFT();

// Dialog Data
	enum { IDD = IDD_GRAPHICFFT };

public:	
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );
	
	IDataHandler * DataHandler(){ return mDataHandler; }

public:
	static IGraphicProtocol * CreateProto(){ return new CGraphicFFT; }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier(){ return mDataIdentifier; }
	virtual HSString Identifier(){ return "GRAPHIC_FFT"; }
	virtual HSString Name(){ return "FFT"; }

	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );
	
	virtual HSVoid FocusGraphic( HSBool tIsFocused );
	virtual CDialog * Wnd(){ return this; }
	virtual HSInt LargeIcon(){ return IDB_BITMAP_FFT; }
	virtual HSInt SmallIcon(){ return IDB_BITMAP_FFT_LIST; }

	virtual HSInt ResourceID(){ return IDD_GRAPHICFFT; }	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CGraphicFFT; }

	virtual HSVoid SetParent( IGraphicWindowProtocol *pParent ){ mParent = pParent; }
	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );
	virtual HSBool ShowFullScreen( HSBool tFullScreen = HSFalse, CWnd *pWnd = NULL )
	{ 
		mIsFullScreen = ( pWnd != NULL ? tFullScreen : mIsFullScreen );
		return mIsFullScreen;
	}

public:
	virtual HSVoid Handle( HSDouble &tValue )
	{
		tValue = 10 * log10( tValue );  
		//tValue = -4.8737009 + 1.2173890 * tValue - 0.0033691 * tValue * tValue;
		//tValue = -0.59207151 + 0.92654998 * tValue;
		//tValue = 72.462262 / ( 1 + 13.709324 * pow( 2.78, -0.067359152 * tValue ) );
	}

	void OnOK(){}
	void OnCancel(){}

public:
	HSDouble BeginAmplitude(){ return mBeginAmplitude; }
	HSDouble TotalAmplitude(){ return mTotalAmplitude; }

	HSVoid SetBeginAmplitude( HSDouble tAmplitude );
	HSVoid SetTotalAmplitude( HSDouble tAmplitude );

private:
	HSVoid RefreshFFT( HSInt tDirection = 1 );
	HSVoid FilterWave( HSDouble &tMinValue, HSDouble &tMaxValue );
	//HSBool DataIsSegment();
	HSBool ReadLinearData( HSInt tArgIndex );
	HSBool ReadSegmentData( HSInt tArgIndex );

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

	CArgListTransfer *mArgTransfer;
	vector< CArgTableManager::ArgItem > mTmpArgList;
	vector< CArgTableManager::ArgItem > mArgList;
	HSInt mHandledArgIndex;

	HSInt mCurCalFFTIndex;
	

	HSChar *mBuffer;	

	CThreadControlEx< CGraphicFFT > *mRefreshThread;

	DEVICE_CH_IDENTIFIER mDataIdentifier;

	HANDLE mDataHandlerMutex;
	HSDouble mRefreshSecond;		

	HSBool mIsFullScreen;

	CMenu mPopupMenu;

	HSDouble mBeginAmplitude;
	HSDouble mTotalAmplitude;

	HSDouble mCurSampleRate;

	CFourierTransform mFourierTransform;

	HSInt mFourierCalLength;
	HSDouble *mFourierResData;
	HSDouble *mFourierResHz;

	HSFloat *mVoltages;	

	HSBool mIsFreshSecondChecked;

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
	afx_msg void OnFftOpupSetdatasource();
	afx_msg void OnFftPopupClose();
	afx_msg void OnFftPopupExportdata();
	afx_msg void OnFftPopupFullScreen();
	afx_msg void OnUpdateFftPopupFullScreen(CCmdUI *pCmdUI);
	afx_msg void OnFftPopupReset();
	CStaticFFT mStaticFFT;
};
