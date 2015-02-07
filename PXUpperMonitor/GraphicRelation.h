#pragma once

#include "DataHandle\DataTypes.h"
#include "afxwin.h"
#include "StaticRelation.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "ArgListTransfer.h"
#include "ArgDataFectcher.h"
#include "NormalGraphic.h"

// CGraphicRelation dialog

class CGraphicRelation : public CNormalGraphic
{
public:
	CGraphicRelation(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicRelation();

public:
	static IGraphicProtocol * CreateProto(){ return new CGraphicRelation; }
	
	virtual HSString Identifier(){ return "GRAPHIC_HISTOGRAM"; }
	virtual HSString Name(){ return "Öù×´Í¼";  }

	virtual HSBool Start();	

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );	
	
	virtual HSInt LargeIcon(){ return IDB_BITMAP_RELATION; }
	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CGraphicRelation; }
	
	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );	

public:
	virtual HSBool GraphicNeedRefresh();	

	virtual HSVoid InitDialog();

	virtual HSVoid StepGraphic( HSInt tDelta );
	virtual HSVoid ZoomInGraphic( CRect &tViewRect );
	virtual HSVoid DragGraphic( CRect &tViewRect, CPoint tPoint );

	virtual HSVoid ResetData( CMainFrame* pMainFrame );

public:
	
	CArgDataFectcher * XArg(){ return &mXArg; }
	CArgDataFectcher * YArg(){ return &mYArg; }
	HSVoid SetXArg( HSString tArgName );
	HSVoid SetYArg( HSString tArgName );

	HSBool IsSolidRect(){ return mFillRectangle; }
	HSDouble UnitDelta(){ return mUnitDelta; }

	HSVoid SetIsSolidRect( HSBool tValue );
	HSVoid SetUnitDelta( HSDouble tValue );
 
private:
	HSVoid CleanChannelArgs();
	HSVoid SetRelationTitle();

private:
	typedef struct ChannelArgInfo
	{
		CArgListTransfer *ArgTransfer;		
		vector< CArgTableManager::ArgItem > ArgList;
		HSInt Index;
		HSInt Channel;
	} ChannelArgInfo;

	vector< ChannelArgInfo > mChannelArgs;

	map< HSInt, HSDouble > mPeriodValues;

	HSDouble mUnitDelta;
	CArgDataFectcher mXArg;
	CArgDataFectcher mYArg;

	HSBool mFillRectangle;	

	HSDouble mCurrentYValue;
	HSDouble mCurrentXValue;

	CStaticRelation mStaticRelation;
};
