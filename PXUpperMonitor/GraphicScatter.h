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

// CGraphicScatter dialog

class CGraphicScatter : public CDialogEx, public IGraphicProtocol
{
	DECLARE_DYNAMIC(CGraphicScatter)

public:
	CGraphicScatter(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicScatter();

// Dialog Data
	enum { IDD = IDD_GRAPHICSCATTER };

public:	
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );
	
	IDataHandler * DataHandler(){ return mDataHandler; }

public:
	static IGraphicProtocol * CreateProto(){ return new CGraphicScatter; }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier(){ return mDataIdentifier; }
	virtual HSString Identifier(){ return "GRAPHIC_SCATTER"; }
	virtual HSString Name(){ return "…¢µ„Õº";  }

	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );
	
	virtual HSVoid FocusGraphic( HSBool tIsFocused );
	virtual CDialog * Wnd(){ return this; }
	virtual HSInt LargeIcon(){ return IDB_BITMAP_SCATTER; }
	virtual HSInt SmallIcon(){ return IDB_BITMAP_SCATTER_LIST; }

	virtual HSInt ResourceID(){ return IDD_GRAPHICSCATTER; }	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CGraphicScatter; }

	virtual HSVoid SetParent( IGraphicWindowProtocol *pParent ){ mParent = pParent; }
	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );
	virtual HSBool ShowFullScreen( HSBool tFullScreen = HSFalse, CWnd *pWnd = NULL )
	{ 
		mIsFullScreen = ( pWnd != NULL ? tFullScreen : mIsFullScreen );
		return mIsFullScreen;
	}

	void OnOK(){}
	void OnCancel(){}

	CArgDataFectcher * XArg(){ return &mXArg; }
	CArgDataFectcher * YArg(){ return &mYArg; }
	HSVoid SetXArg( HSString tArgName );
	HSVoid SetYArg( HSString tArgName );

private:
	HSVoid SetRelationTitle();
	HSVoid CleanChannelArgs();

private:
	HSBool mInit;

	HSBool mIsDraging;
	CPoint mPrevPoint;

	HSBool mIsZoomIn;
	CPoint mZoomInBeginPoint;
	CRect mZoomInRect;
	HSBool mFirstTimeDrawZoomRect;

	IGraphicWindowProtocol *mParent;	
	
	IDataHandler *mDataHandler;
	HSUInt mChannel;

	HSChar *mBuffer;	

	CThreadControlEx< CGraphicScatter > *mUpdateRelationThread;

	DEVICE_CH_IDENTIFIER mDataIdentifier;	

	HSBool mIsFullScreen;

	CArgListTransfer *mArgTransfer;
	CLinearTransfer *mDataTransfer;

	HANDLE mRelationMutex;

	CMenu mPopupMenu;

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

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CStaticScatter mStaticScatter;
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnScatterpopupClose();
	afx_msg void OnScatterpopupFullScreen();
	afx_msg void OnUpdateScatterpopupFullScreen(CCmdUI *pCmdUI);
	afx_msg void OnScatterpopupSetdatasource();
	afx_msg void OnScatterpopupReset();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};
