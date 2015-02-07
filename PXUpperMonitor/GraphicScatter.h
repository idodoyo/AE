#pragma once

#include "DataHandle\DataTypes.h"
#include "afxwin.h"
#include "StaticScatter.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "ArgListTransfer.h"
#include "ArgDataFectcher.h"
#include "NormalGraphic.h"

// CGraphicScatter dialog

class CGraphicScatter : public CNormalGraphic
{
public:
	CGraphicScatter(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicScatter();

public:
	static IGraphicProtocol * CreateProto(){ return new CGraphicScatter; }
	
	virtual HSString Identifier(){ return "GRAPHIC_SCATTER"; }
	virtual HSString Name(){ return "…¢µ„Õº";  }

	virtual HSBool Start();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );	

	virtual HSInt LargeIcon(){ return IDB_BITMAP_SCATTER; }

	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CGraphicScatter; }

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

private:
	HSVoid SetRelationTitle();
	HSVoid CleanChannelArgs();

private:	
	CArgListTransfer *mArgTransfer;
	CLinearTransfer *mDataTransfer;

	typedef struct ChannelArgInfo
	{
		CArgListTransfer *ArgTransfer;		
		vector< CArgTableManager::ArgItem > ArgList;
		HSInt Index;
		HSInt Channel;
	} ChannelArgInfo;

	vector< ChannelArgInfo > mChannelArgs;

	CArgDataFectcher mXArg;
	CArgDataFectcher mYArg;

	HSDouble mCurrentYValue;	
	HSDouble mCurrentXValue;

	list< CStaticScatter::ArgValueInfo > mArgsValue;

	CStaticScatter mStaticScatter;
};
