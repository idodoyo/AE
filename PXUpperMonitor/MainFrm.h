
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "DeviceView.h"
#include "OutputWnd.h"
#include "GraghicWnd.h"
#include "ThreadControlEx.h"
#include "DeviceManager.h"
#include "IndexManager.h"
#include "DeviceInfo.h"
#include "DataFileHeader.h"
#include "FileObserver.h"
#include "GraphicManager.h"

#include <string>

using std::string;


class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	
// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	string SelectedGraphic(){ return mSelGraphicName; }
	void SetSelectedGraphic( string tGraphic ){ mSelGraphicName = tGraphic; }
	string DragingGraphic(){ return mDragingGraphicName; }
	void SetDragingGraphic( string tDragingGraphic ){ mDragingGraphicName = tDragingGraphic; }

	IGraphicProtocol * FocusedGraphic(){ return mFocusedGrahpic; }
	void SetFocusedGraphic( IGraphicProtocol * pGraphic, HSBool tForceRefresh = HSFalse );

	CDeviceManager * DeviceManager(){ return mDeviceManager; }
	CIndexManager * IndexManager(){ return &mIndexManager; }
	CDeviceInfo * DeviceInfo(){ return &mDeviceInfo; }

	void SetDocTemplate( CDocTemplate *pDocTemplate );

	enum GRAPHIC_STATE{ GRAPHIC_START, GRAPHIC_PAUSE, GRAPHIC_STOP };
	void SetAllGraphicState( GRAPHIC_STATE tState );
	HSBool IsStart(){ return mGraphicState == GRAPHIC_START; }
	GRAPHIC_STATE ControlState(){ return mGraphicState; }

	HSBool IsSettingArgEnabled();

	void ResetAllGraphic();

	void StopSampleData();

	void SaveAllLayout();

	HSString GetNextDataFileName( HSString &tFileName );

	enum GRAPHIC_CEHCK_TYPE{ GRAPHIC_CHECK_POINTER, GRAPHIC_CHECK_STEP_FRAME, GRAPHIC_CHECK_RECTANGE, GRAPHIC_CHECK_DRAG };
	GRAPHIC_CEHCK_TYPE GetGrahpicCheckType(){ return mGrapicCheckType; }
	void SetGrahpicCheckType( GRAPHIC_CEHCK_TYPE tType ){ mGrapicCheckType = tType; }

	static string mConfigFilePath;	

protected:  // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;

	CMFCToolBar       mCustomToolBar;
	CMFCToolBar mWaveContrlToolBar;

	CDeviceView       mWndDeviceView;
	COutputWnd        mWndOutput;
	CGraghicWnd		  mWndGraphic;

	string mSelGraphicName;
	string mDragingGraphicName;

	CDeviceManager *mDeviceManager;
	CIndexManager mIndexManager;
	CDeviceInfo mDeviceInfo;
	CDataFileHeader mDataFileHeader;
	CFileObserver *mFileObserver;

	CDocTemplate *mDocTemplate;

	HSBool mIsReviewMode;

	GRAPHIC_STATE mGraphicState;
	
	GRAPHIC_CEHCK_TYPE mGrapicCheckType;

	IGraphicProtocol *mFocusedGrahpic;		

// Generated message map functions
protected:
	LRESULT OnUpdateFileSize( UINT wParam, LPARAM lParam );	
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
public:
	afx_msg void OnViewDevice();
	afx_msg void OnViewGraphic();
	afx_msg void OnViewOutput();
	afx_msg void OnViewStandardToolbar();
	afx_msg void OnUpdateViewStandardToolbar(CCmdUI *pCmdUI);
	afx_msg void OnIndicatorSample();
	afx_msg void OnUpdateIndicatorSample(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorReplay(CCmdUI *pCmdUI);
	afx_msg void OnIndicatorReplay();
	afx_msg void OnControlPause();
	afx_msg void OnUpdateControlPause(CCmdUI *pCmdUI);
	afx_msg void OnControlStart();
	afx_msg void OnUpdateControlStart(CCmdUI *pCmdUI);
	afx_msg void OnControlStop();
	afx_msg void OnUpdateControlStop(CCmdUI *pCmdUI);
	afx_msg void OnFileOpen();
	afx_msg void OnFileClose();
	afx_msg void OnUpdateFileClose(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileOpen(CCmdUI *pCmdUI);
	afx_msg void OnOptionsExporttotxt();
	afx_msg void OnUpdateOptionsExporttotxt(CCmdUI *pCmdUI);
	afx_msg void OnClose();	
	afx_msg void OnViewGraphictoolbar();
	afx_msg void OnUpdateViewGraphictoolbar(CCmdUI *pCmdUI);
	afx_msg void OnViewGraphicPointer();
	afx_msg void OnUpdateViewGraphicPointer(CCmdUI *pCmdUI);
	afx_msg void OnViewGraphicNextFrame();
	afx_msg void OnUpdateViewGraphicNextFrame(CCmdUI *pCmdUI);
	afx_msg void OnViewGraphicRectangle();
	afx_msg void OnUpdateViewGraphicRectangle(CCmdUI *pCmdUI);	
	afx_msg void OnViewGraphicDrag();
	afx_msg void OnUpdateViewGraphicDrag(CCmdUI *pCmdUI);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
};


