#pragma once
#include "afxcmn.h"

#include "DataTypes.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "afxwin.h"
#include "StaticArg.h"
#include "ArgListTransfer.h"
#include "ArgListCtrl.h"


// CArgTable dialog

class CArgTable : public CDialogEx, public IGraphicProtocol
{
	DECLARE_DYNAMIC(CArgTable)

public:
	CArgTable(CWnd* pParent = NULL);   // standard constructor
	virtual ~CArgTable();

// Dialog Data
	enum { IDD = IDD_ARGTABLE };

public:	
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );	

public:
	static IGraphicProtocol * CreateProto(){ return new CArgTable; }

	virtual DEVICE_CH_IDENTIFIER DataIdentifier(){ return mDataIdentifier; }
	virtual HSString Identifier(){ return "GRAPHIC_WAVE_ARG_TAB"; }
	virtual HSString Name(){ return "²ÎÊý±í"; }

	virtual HSBool Start();
	virtual HSVoid Pause();
	virtual HSVoid Stop();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );
	
	virtual HSVoid FocusGraphic( HSBool tIsFocused );
	virtual CDialog * Wnd(){ return this; }
	virtual HSInt LargeIcon(){ return IDB_BITMAP_ARG; }
	virtual HSInt SmallIcon(){ return IDB_BITMAP_ARROW_LIST; }

	virtual HSInt ResourceID(){ return IDD_ARGTABLE; }	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new CArgTable; }

	virtual HSVoid SetParent( IGraphicWindowProtocol *pParent ){ mParent = pParent; }
	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );
	virtual HSBool ShowFullScreen( HSBool tFullScreen = HSFalse, CWnd *pWnd = NULL );
	
	void OnOK(){}
	void OnCancel(){}

	IDataHandler * DataHandler(){ return mDataHandler; }
	HSVoid FocusWnd();

private:
	CString GetFormatTime( HSUInt64 tNSecond );
	HSVoid CleanChannelArgs();
	HSVoid SetRelationTitle();

	HSBool GetNextArg( CArgTableManager::ArgItem * &pArg, HSInt &tChannel );

private:
	CImageList mImageList;
	HSBool mInit;

	IGraphicWindowProtocol *mParent;	
	
	IDataHandler *mDataHandler;
	HSUInt mChannel;

	HSChar *mBuffer;	

	CThreadControlEx< CArgTable > *mUpdateArgThread;

	DEVICE_CH_IDENTIFIER mDataIdentifier;	

	HSBool mIsFullScreen;

	typedef struct ChannelArgInfo
	{
		CArgListTransfer *ArgTransfer;		
		HSInt Channel;
		vector< CArgTableManager::ArgItem > ArgList;
		HSInt Index;
	} ChannelArgInfo;

	vector< ChannelArgInfo > mChannelArgs;

	HANDLE mArgMutex;

	CMenu mPopupMenu;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CArgListCtrl mArgList;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	CStaticArg mStaticTitle;	
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnArgpopupClose();
	afx_msg void OnArgpopupExportdata();
	afx_msg void OnArgpopupFullScreen();
	afx_msg void OnUpdateArgpopupFullScreen(CCmdUI *pCmdUI);
	afx_msg void OnArgpopupSetdatasource();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);	
	afx_msg void OnDestroy();
	afx_msg void OnUpdateArgpopupExportdata(CCmdUI *pCmdUI);
};
