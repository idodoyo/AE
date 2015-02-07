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
#include "ArgListTransfer.h"
#include "NormalGraphic.h"


// CGraphicWave dialog

class CGraphicWave : public CNormalGraphic
{
public:
	CGraphicWave(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicWave();

public:
	static IGraphicProtocol * CreateProto(){ return new CGraphicWave; }
	
	virtual HSString Identifier(){ return "GRAPHIC_WAVE"; }
	virtual HSString Name(){ return "²¨ÐÎÍ¼"; }

	virtual HSBool Start();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );	

	virtual HSInt LargeIcon(){ return IDB_BITMAP_WAVE; }	
	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CGraphicWave; }

	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );

public:
	virtual HSBool GraphicNeedRefresh();	

	virtual HSVoid InitDialog();
	virtual HSVoid ViewResized( CRect &tRect );

	virtual HSVoid StepGraphic( HSInt tDelta );
	virtual HSVoid ZoomInGraphic( CRect &tViewRect );
	virtual HSVoid DragGraphic( CRect &tViewRect, CPoint tPoint );

	virtual HSBool IsDataExportNeeded();
	virtual HSVoid ExportData( CDataExportHelper &tDataExport );

	virtual HSVoid ResetData( CMainFrame* pMainFrame );
	virtual HSVoid ResetGrahpic();

public:
	HSVoid SetTime( HSDouble tSecond );
	HSVoid SetVoltage( HSDouble tVoltage );

	HSDouble RefreshTime(){ return mTotalSecond; }
	HSDouble TotalVoltage(){ return mTotalVoltage; }

	HSBool OnlyHitWaves(){ return mOnlyHitWaves; }
	HSVoid SetOnlyHitWaves( HSBool tValue ){ mOnlyHitWaves = tValue; }

	HSDouble VoltageOffset(){ return mVoltageOffset; }
	HSVoid SetVoltageOffset( HSDouble tOffset );

private:
	HSVoid RefreshWave();
	HSBool RefreshArgWave( HSInt tDirection = 1 );
	HSBool RefreshSegmentWave( HSInt tDirection = 1 );
	HSVoid AdjustSegmentWave();

private:
	CLinearTransfer *mDataTransfer;	
	HSUInt mChannel;

	HSChar *mBuffer;
	HSChar *mSegmentBuffer;
	HSInt mSampleLength;	

	HSDouble mRefreshSecond;	

	HSDouble mTotalVoltage;
	HSDouble mTotalSecond;

	HSDouble mBeginSecondOffset;

	CArgListTransfer *mArgTransfer;
	vector< CArgTableManager::ArgItem > mTmpArgList;
	vector< CArgTableManager::ArgItem > mArgList;
	HSInt mHandledArgIndex;

	HSInt mCurWaveIndex;

	HSBool mOnlyHitWaves;
	HSDouble mVoltageOffset;

	CStaticGraphic mStaticWave;

};
