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
#include "NormalGraphic.h"


// CGraphicFFT dialog

class CGraphicFFT : public CNormalGraphic
{
public:
	CGraphicFFT(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicFFT();

public:
	static IGraphicProtocol * CreateProto(){ return new CGraphicFFT; }
	
	virtual HSString Identifier(){ return "GRAPHIC_FFT"; }
	virtual HSString Name(){ return "¹¦ÂÊÆ×"; }

	virtual HSBool Start();	

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );	
	
	virtual HSInt LargeIcon(){ return IDB_BITMAP_FFT; }
	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CGraphicFFT; }
	
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
	virtual HSVoid ResetGrahpic();

public:
	HSDouble BeginAmplitude(){ return mBeginAmplitude; }
	HSDouble TotalAmplitude(){ return mTotalAmplitude; }

	HSVoid SetBeginAmplitude( HSDouble tAmplitude );
	HSVoid SetTotalAmplitude( HSDouble tAmplitude );

private:
	HSVoid RefreshFFT( HSInt tDirection = 1 );
	HSVoid FilterWave( HSDouble &tMinValue, HSDouble &tMaxValue );	
	HSBool ReadLinearData( HSInt tArgIndex );
	HSBool ReadSegmentData( HSInt tArgIndex );
	HSVoid GetFFTCalPeriod( HSInt tLength );

private:
	CLinearTransfer *mDataTransfer;	
	HSUInt mChannel;

	CArgListTransfer *mArgTransfer;
	vector< CArgTableManager::ArgItem > mTmpArgList;
	vector< CArgTableManager::ArgItem > mArgList;
	HSInt mHandledArgIndex;

	HSInt mCurCalFFTIndex;	

	HSChar *mBuffer;

	HSDouble mRefreshSecond;

	HSDouble mBeginAmplitude;
	HSDouble mTotalAmplitude;

	HSDouble mCurSampleRate;

	CFourierTransform mFourierTransform;

	HSInt mFourierCalLength;
	HSDouble *mFourierResData;
	HSDouble *mFourierResHz;

	HSFloat *mVoltages;	
	HSFloat *mTmpVoltages;
	HSInt mTmpVotagesLength;

	CStaticFFT mStaticFFT;
	/*
	HSDouble *mHorValue;
	HSDouble *mVerValue;
	HSInt mHorIndex;*/
};
