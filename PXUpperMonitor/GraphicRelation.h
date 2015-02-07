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


// CGraphicRelation dialog

class CGraphicRelation : public CDialogEx, public IGraphicProtocol
{
	DECLARE_DYNAMIC(CGraphicRelation)

public:
	CGraphicRelation(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicRelation();

// Dialog Data
	enum { IDD = IDD_GRAPHICRELATION };

public:	
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );
	
	IDataHandler * DataHandler(){ return mDataHandler; }

public:
	static IGraphicProtocol * CreateProto(){ return new CGraphicRelation; }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier(){ return mDataIdentifier; }
	virtual HSString Identifier(){ return "GRAPHIC_HISTOGRAM"; }
	virtual HSString Name(){ return "Öù×´Í¼";  }

	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );
	
	virtual HSVoid FocusGraphic( HSBool tIsFocused );
	virtual CDialog * Wnd(){ return this; }
	virtual HSInt LargeIcon(){ return IDB_BITMAP_RELATION; }
	virtual HSInt SmallIcon(){ return IDB_BITMAP_RELATION_LIST; }

	virtual HSInt ResourceID(){ return IDD_GRAPHICRELATION; }	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CGraphicRelation; }

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
	HSBool mInit;

	HSBool mIsDraging;
	CPoint mPrevPoint;

	HSBool mIsZoomIn;
	CPoint mZoomInBeginPoint;
	CRect mZoomInRect;
	HSBool mFirstTimeDrawZoomRect;

	IGraphicWindowProtocol *mParent;	
	
	IDataHandler *mDataHandler;	

	CThreadControlEx< CGraphicRelation > *mUpdateRelationThread;

	DEVICE_CH_IDENTIFIER mDataIdentifier;	

	HSBool mIsFullScreen;

	CMenu mPopupMenu;

	HANDLE mRelationMutex;

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

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:	
	CStaticRelation mStaticRelation;
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRelationpopupClose();
	afx_msg void OnRelationpopupFullScreen();
	afx_msg void OnUpdateRelationpopupFullScreen(CCmdUI *pCmdUI);
	afx_msg void OnRelationpopupSetdatasource();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnDestroy();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnRelationpopupReset();
};
