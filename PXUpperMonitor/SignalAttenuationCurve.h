#pragma once

#include "DataHandle\DataTypes.h"
#include "afxwin.h"
#include "StaticSignalAttenuation.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "SignalAttenuationCurveProperty.h"
#include "FourierTransform.h"
#include "ArgListTransfer.h"
#include "NormalGraphic.h"

// CSignalAttenuationCurve dialog


class CSignalAttenuationCurve : public CNormalGraphic
{
public:
	CSignalAttenuationCurve(CWnd* pParent = NULL);
	virtual ~CSignalAttenuationCurve();

public:			
	IDataHandler * OrigDataHandler(){ return mDataHandler; }

	DEVICE_CH_IDENTIFIER OrigDataIdentifier(){ return mDataIdentifier; }
	HSVoid SetOrigDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier ){}

public:
	static IGraphicProtocol * CreateProto(){ return new CSignalAttenuationCurve; }
	
	virtual HSString Identifier(){ return "GRAPHIC_SIGNAL_ATTENUATION_CURVE"; }
	virtual HSString Name(){ return "Ñ¹µç²¹³¥ÇúÏß"; }

	virtual HSBool Start();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );		
	
	virtual HSInt LargeIcon(){ return IDB_BITMAP_SIGNAL_ATTENUATION; }
	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CSignalAttenuationCurve; }
	
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

	CStaticSignalAttenuation mStaticSignal;
};
