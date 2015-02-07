
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "PXUpperMonitor.h"

#include "MainFrm.h"
#include "DataHandle\DataTypes.h"
#include "PXUpperMonitorDoc.h"
#include "DataExportToTXT.h"
#include "INIConfig.h"
#include "ArgCalculator.h"
#include "WaveGenClient.h"
#include "ListItemManager.h"

#include <fstream>


#include <map>
#include <vector>
#include <list>
#include <string>
#include "HSLogProtocol.h"

using std::map;
using std::vector;
using std::list;
using std::string;

using std::ofstream;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_MESSAGE( WM_UPDATE_FILE_SIZE_MESSAGE, OnUpdateFileSize )
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	ON_WM_SETTINGCHANGE()
	ON_COMMAND(ID_VIEW_DEVICE, &CMainFrame::OnViewDevice)
	ON_COMMAND(ID_VIEW_GRAPHIC, &CMainFrame::OnViewGraphic)
	ON_COMMAND(ID_VIEW_OUTPUT, &CMainFrame::OnViewOutput)
	ON_COMMAND(ID_VIEW_STANDARD_TOOLBAR, &CMainFrame::OnViewStandardToolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STANDARD_TOOLBAR, &CMainFrame::OnUpdateViewStandardToolbar)
	ON_COMMAND(ID_INDICATOR_SAMPLE, &CMainFrame::OnIndicatorSample)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SAMPLE, &CMainFrame::OnUpdateIndicatorSample)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_REPLAY, &CMainFrame::OnUpdateIndicatorReplay)
	ON_COMMAND(ID_INDICATOR_REPLAY, &CMainFrame::OnIndicatorReplay)
	ON_COMMAND(ID_CONTROL_PAUSE, &CMainFrame::OnControlPause)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_PAUSE, &CMainFrame::OnUpdateControlPause)
	ON_COMMAND(ID_CONTROL_START, &CMainFrame::OnControlStart)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_START, &CMainFrame::OnUpdateControlStart)
	ON_COMMAND(ID_CONTROL_STOP, &CMainFrame::OnControlStop)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_STOP, &CMainFrame::OnUpdateControlStop)
	ON_COMMAND(ID_FILE_OPEN, &CMainFrame::OnFileOpen)
	ON_COMMAND(ID_DATAFILE_CLOSE, &CMainFrame::OnFileClose)
	ON_UPDATE_COMMAND_UI(ID_DATAFILE_CLOSE, &CMainFrame::OnUpdateFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &CMainFrame::OnUpdateFileOpen)
	ON_COMMAND(ID_OPTIONS_EXPORTTOTXT, &CMainFrame::OnOptionsExporttotxt)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_EXPORTTOTXT, &CMainFrame::OnUpdateOptionsExporttotxt)
	ON_WM_CLOSE()	
	ON_COMMAND(ID_VIEW_GRAPHICTOOLBAR, &CMainFrame::OnViewGraphictoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GRAPHICTOOLBAR, &CMainFrame::OnUpdateViewGraphictoolbar)
	ON_COMMAND(ID_VIEW_GRAPHIC_POINTER, &CMainFrame::OnViewGraphicPointer)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GRAPHIC_POINTER, &CMainFrame::OnUpdateViewGraphicPointer)
	ON_COMMAND(ID_VIEW_GRAPHIC_NEXT_FRAME, &CMainFrame::OnViewGraphicNextFrame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GRAPHIC_NEXT_FRAME, &CMainFrame::OnUpdateViewGraphicNextFrame)
	ON_COMMAND(ID_VIEW_GRAPHIC_RECTANGLE, &CMainFrame::OnViewGraphicRectangle)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GRAPHIC_RECTANGLE, &CMainFrame::OnUpdateViewGraphicRectangle)
	ON_COMMAND(ID_VIEW_GRAPHIC_DRAG, &CMainFrame::OnViewGraphicDrag)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GRAPHIC_DRAG, &CMainFrame::OnUpdateViewGraphicDrag)
END_MESSAGE_MAP()

/*
static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};
*/

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_SAMPLE,
	ID_INDICATOR_REPLAY,	
};

// CMainFrame construction/destruction

string CMainFrame::mConfigFilePath = "Configs\\config.ini";

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);

	mSelGraphicName = "";
	mDragingGraphicName = "";	

	mFileObserver = new CFileObserver( OBSERVER_BACKGROUND, NULL, 0, this );
	mDeviceManager = new CDeviceManager( mFileObserver, &mIndexManager, this );	

	mIsReviewMode = HSFalse;

	mGraphicState = GRAPHIC_STOP;

	mGrapicCheckType = GRAPHIC_CHECK_POINTER;

	mFocusedGrahpic = NULL;		

	CArgCalculator::SharedInstance()->SetIndexManager( &mIndexManager, mDeviceManager );
}

CMainFrame::~CMainFrame()
{
	delete mDeviceManager;
	delete mFileObserver;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	CINIConfig tIniConfig;
	string tFilePath = "";
	if ( tIniConfig.LoadFile( mConfigFilePath.c_str() ) )
	{				
		tIniConfig.ValueWithKey( "Save File Path", tFilePath, "DATA FILE" );

		CDeviceManager::Load( &tIniConfig );	
	}
	

	BOOL bNameValid;
	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);	
	
	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // other styles available...
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // set to FALSE to place close button at right of tab area
	mdiTabParams.m_bTabIcons = FALSE;    // set to TRUE to enable document icons on MDI taba
	mdiTabParams.m_bAutoColor = TRUE;    // set to FALSE to disable auto-coloring of MDI tabs
	mdiTabParams.m_bDocumentMenu = TRUE; // enable the document menu at the right edge of the tab area
	EnableMDITabbedGroups(TRUE, mdiTabParams);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC |  CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);	

	if (!mWaveContrlToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!mWaveContrlToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_WAVE_CONTROL : IDR_WAVE_CONTROL))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}	
	mWaveContrlToolBar.SetWindowText( "图形操作" );	

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}	

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
//	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	// Allow user-defined toolbars operations:
	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable

	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);			
	mWaveContrlToolBar.EnableDocking( CBRS_ALIGN_ANY );	
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);

	EnableDocking(CBRS_ALIGN_ANY);	

	DockPane(&m_wndMenuBar);		
	DockPane(&m_wndToolBar);
	DockPane( &mWaveContrlToolBar );
	
	

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Load menu item image (not placed on any standard toolbars):
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

	// create docking windows
	if (!CreateDockingWindows())
	{
		TRACE0("Failed to create docking windows\n");
		return -1;
	}

	mWndDeviceView.EnableDocking(CBRS_ALIGN_ANY);	
	mWndGraphic.EnableDocking(CBRS_ALIGN_ANY);
	mWndOutput.EnableDocking(CBRS_ALIGN_ANY);

	EnableDocking(CBRS_ALIGN_ANY);

	DockPane(&mWndDeviceView);	
	DockPane(&mWndGraphic);		
	DockPane(&mWndOutput);

	// Enable enhanced windows management dialog
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);

	// Enable toolbar and docking window menu replacement
//	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

	if (CMFCToolBar::GetUserImages() == NULL)
	{
		// load user-defined toolbar images
		if (m_UserImages.Load(_T(".\\UserImages.bmp")))
		{
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}

	// enable menu personalization (most-recently used commands)
	// TODO: define your own basic commands, ensuring that each pulldown menu has at least one basic command.

	// Switch the order of document name and application name on the window title bar. This
	// improves the usability of the taskbar because the document name is visible with the thumbnail.
	ModifyStyle(0, FWS_PREFIXTITLE);
	/*
	if (!mCustomToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!mCustomToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_258 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}	
	*/

	HICON tIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	SetIcon( tIcon,TRUE );
	SetIcon( tIcon,FALSE );

	if ( tFilePath != "" )
	{
		this->mWndGraphic.SetDefaultFilePath( tFilePath.c_str() );
		this->mWndGraphic.SetFilePath( tFilePath.c_str() );
	}

	return 0;

}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
	CRect tViewRect;
	this->GetClientRect( &tViewRect );	

	// Create device view
	CRect tDeviceRect( tViewRect.left, tViewRect.top, 200, tViewRect.bottom );
	if (!mWndDeviceView.Create( "设备", this, tDeviceRect, TRUE, ID_VIEW_FILEVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create File View window\n");
		return FALSE; // failed to create
	}

	// Create graphic window
	CRect tGraphicRect( tViewRect.right - 250, tViewRect.top, tViewRect.right, tViewRect.bottom );
	if (!mWndGraphic.Create("图像 && 数据文件", this, tGraphicRect, TRUE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE; // failed to create
	}

	// Create output window
	CRect tOutputRect( tViewRect.left + tDeviceRect.Width(), tViewRect.bottom - 100, tViewRect.right - tGraphicRect.Width(), tViewRect.bottom );
	if (!mWndOutput.Create( "输出", this, tOutputRect, TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Output window\n");
		return FALSE; // failed to create
	}		

	SetDockingWindowIcons(theApp.m_bHiColorIcons);

	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hFileViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	mWndDeviceView.SetIcon(hFileViewIcon, FALSE);

	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	mWndOutput.SetIcon(hOutputBarIcon, FALSE);

	HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	mWndGraphic.SetIcon(hPropertiesBarIcon, FALSE);

	UpdateMDITabbedBarsIcons();
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// base class does the real work

	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	// enable customization button for all user toolbars
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	return TRUE;
}


void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CMDIFrameWndEx::OnSettingChange(uFlags, lpszSection);
	mWndOutput.UpdateFonts();
}


void CMainFrame::OnViewDevice()
{
	// TODO: Add your command handler code here
	if ( !mWndDeviceView.IsVisible() )
	{
		mWndDeviceView.ShowPane( TRUE, TRUE, TRUE );	
		this->RecalcLayout();
	}
}

void CMainFrame::OnViewGraphic()
{
	// TODO: Add your command handler code here
	if ( !mWndGraphic.IsVisible() )
	{
		mWndGraphic.ShowPane( TRUE, TRUE, TRUE );	
		this->RecalcLayout();
	}	
}

void CMainFrame::OnViewOutput()
{
	// TODO: Add your command handler code here
	if ( !mWndOutput.IsVisible() )
	{
		mWndOutput.ShowPane( TRUE, TRUE, TRUE );	
		this->RecalcLayout();
	}
}

void CMainFrame::OnViewStandardToolbar()
{
	// TODO: Add your command handler code here
	m_wndToolBar.ShowPane( !m_wndToolBar.IsVisible(), TRUE, TRUE );	
	this->RecalcLayout();
}


void CMainFrame::OnUpdateViewStandardToolbar(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( m_wndToolBar.IsVisible() );	
}


void CMainFrame::OnIndicatorSample()
{
	// TODO: Add your command handler code here	
}


void CMainFrame::OnUpdateIndicatorSample(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable( !mIsReviewMode && mGraphicState != GRAPHIC_STOP );
}


void CMainFrame::OnUpdateIndicatorReplay(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable( mIsReviewMode );	
}


void CMainFrame::OnIndicatorReplay()
{
	// TODO: Add your command handler code here
}

void CMainFrame::OnControlPause()
{
	// TODO: Add your command handler code here
	CString tStrInfo;
	if ( mIsReviewMode )
	{
		SetAllGraphicState( GRAPHIC_PAUSE );
		
		tStrInfo.Format( " %s %s 暂停回放...", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
		mWndOutput.PrintInfo( tStrInfo );	

		return;
	}

	mDeviceManager->Pause();
	SetAllGraphicState( GRAPHIC_PAUSE );	
	
	tStrInfo.Format( " %s %s 暂停采样...", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
	mWndOutput.PrintInfo( tStrInfo );	
}


void CMainFrame::OnUpdateControlPause(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable( mGraphicState == GRAPHIC_START );
}

HSString CMainFrame::GetNextDataFileName( HSString &tFileName )
{
	HSString tNewFileName = tFileName.substr( 0, tFileName.find_last_of( '.' ) );
	std::size_t tDigitPos = tNewFileName.find_last_not_of( "0123456789" );
	if ( tDigitPos == string::npos || tDigitPos == tNewFileName.length() - 1 )
	{
		return tNewFileName + "0.pxd";
	}
	
	HSString tStrNum = tNewFileName.substr( tDigitPos + 1 );
	
	HSChar tBuf[ 64 ];
	sprintf_s( tBuf, "%d.pxd", atoi( tStrNum.c_str() ) + 1 );

	return tNewFileName.substr( 0, tDigitPos + 1 ) + tBuf;
}

void CMainFrame::OnControlStart()
{
	// TODO: Add your command handler code here

	CString tStrInfo;
	if ( mIsReviewMode )
	{		
		tStrInfo.Format( " %s %s 开始回放...", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
		mWndOutput.PrintInfo( tStrInfo );	

		if ( GRAPHIC_STOP == mGraphicState )
		{
			ResetAllGraphic();
		}

		SetAllGraphicState( GRAPHIC_START );
		return;
	}

	string tFile = mWndGraphic.FilePath();

	vector< string > tVecStartInfo;		
	HSBool tChoosedAnyDevice = HSFalse;
	if ( !mDeviceManager->IsPaused() )
	{
		mDeviceInfo.Reset();

		/* open devices */		
		vector< IDataHandler * > &pDevices = *( mDeviceManager->Devices() );
		for ( HSUInt i = 0; i < pDevices.size(); i++ )
		{
			pDevices[ i ]->CloseAllChannel();
			vector< HSInt > tChannels = mWndDeviceView.OpenChannelsWithDevice( i );
			if ( tChannels.size() > 0 )
			{				
				tChoosedAnyDevice = HSTrue;
				HSString tErrorMsg = "";
				if ( !pDevices[ i ]->OpenChannel( tChannels, NULL, &tErrorMsg ) )
				{										
					MessageBox( tErrorMsg.c_str(), "警告" );
					return;
				}
				else
				{
					string tStartInfo = "设备 -  ";
					tStartInfo += pDevices[ i ]->Name().c_str();
					for ( HSUInt j = 0; j < tChannels.size(); j++ )
					{						
						HSDouble tChannelSampleRate = pDevices[ i ]->SampleRate( tChannels[ j ] );
						CString tStrSampleRate;
						HSDouble tMHZ = 1000000.0 - 0.000001;
						tStrSampleRate.Format( "%.0f %s", tChannelSampleRate > tMHZ ? tChannelSampleRate / 1000000.0 : tChannelSampleRate / 1000.0, 
															tChannelSampleRate > tMHZ ? "MHZ" : "KHZ" );
						CString tChannelStr;
						tChannelStr.Format( ", 通道%d 采样率: %s", tChannels[ j ] + 1, ( LPCSTR )tStrSampleRate );
						tStartInfo += tChannelStr;
					}

					for ( HSInt j = 0; j < pDevices[ i ]->ChannelNum(); j++ )
					{
						HSBool tIsOn = HSFalse;
						for ( HSUInt k = 0; k < tChannels.size(); k++ )
						{
							if ( j == tChannels[ k ] )
							{
								tIsOn = HSTrue;
								break;
							}
						}

						DEVICE_CH_IDENTIFIER tChannelIdentifier = pDevices[ i ]->Identifier();
						tChannelIdentifier.InitChannel( j );
						mDeviceInfo.AddChannel( pDevices[ i ]->Identifier(), j, pDevices[ i ]->SampleRate( tChannelIdentifier ), tIsOn );
					}

					tVecStartInfo.push_back( tStartInfo );
				}
			}			
		}

		
		if ( theApp.DataFileIndex() > 0 )
		{
			tFile = GetNextDataFileName( tFile );
			mWndGraphic.SetFilePath( tFile.c_str() );
		}

		HSInt64 tDisperseFileLimit = mWndGraphic.GetDisperseFileLimit();
		if ( !mFileObserver->SetFile( tFile.c_str(), HSFalse, tDisperseFileLimit ) )
		{
			MessageBox( "数据文件打开错误!", "警告" );
			return;
		}

		mDataFileHeader.SetDisperseFileLimit( tDisperseFileLimit );
		mDataFileHeader.Save( mFileObserver );
		mIndexManager.Reset();
	}

	if ( !mDeviceManager->Start() )
	{
		if ( !tChoosedAnyDevice )
		{
			MessageBox( "请选择采集设备!", "警告", MB_ICONWARNING );
		}

		mFileObserver->Close();		
		return;
	}	

	/* restart, should reset linear transfer. */
	if ( GRAPHIC_STOP == mGraphicState )
	{				
		ResetAllGraphic();
		mWndGraphic.UpdateFileSize( 0 );
	}

	time_t tTime;
	time( &tTime );
	theApp.StartTime( tTime );
	mDataFileHeader.SetStartTime( tTime );
		
	SetAllGraphicState( GRAPHIC_START );
	
	for ( HSUInt i = 0; i < tVecStartInfo.size(); i++ )
	{
		mWndOutput.PrintInfo( tVecStartInfo[ i ].c_str() );
	}
	
	tStrInfo.Format( " %s %s 开始采样...", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
	mWndOutput.PrintInfo( tStrInfo );

	mWndDeviceView.EnableChooseDevice( FALSE );
	mWndGraphic.EnableFileInfo( FALSE );	
}


void CMainFrame::OnUpdateControlStart(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable( mGraphicState != GRAPHIC_START );	
}


void CMainFrame::OnControlStop()
{
	// TODO: Add your command handler code here	
	if ( mGraphicState == GRAPHIC_STOP )
	{
		return;
	}

	CString tStrInfo;
	if ( mIsReviewMode )
	{
		SetAllGraphicState( GRAPHIC_STOP );
		
		tStrInfo.Format( " %s %s 停止回放...", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
		mWndOutput.PrintInfo( tStrInfo );	
		
		return;
	}

	if ( IDYES != MessageBox( "确认停止数据采集?", "警告", MB_ICONQUESTION | MB_YESNO ) )
	{
		return;
	}

	StopSampleData();

	tStrInfo.Format( " %s %s 停止采样...", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
	mWndOutput.PrintInfo( tStrInfo );	

	theApp.DataFileAddIndex();
}

void CMainFrame::StopSampleData()
{
	mDeviceManager->Stop();

	SetAllGraphicState( GRAPHIC_STOP );

	HSUInt tDeviceCount = mDeviceInfo.DeviceCount();
	for ( HSUInt i = 0; i < tDeviceCount; i++ )
	{
		IDataHandler *pDataHandler = mDeviceManager->DataHandlerWithIdentifier( mDeviceInfo.DeviceWithIndex( i ) );
		mDeviceInfo.SetDeviceSavePos( pDataHandler->Identifier(), pDataHandler->Save() );
	}	
	
	mDeviceInfo.Save( mFileObserver );
	mIndexManager.Save( mFileObserver );
	mFileObserver->IndexManager()->Save( mFileObserver );

	mDataFileHeader.SetDeviceIndexInfo( mFileObserver->IndexManager()->FilePos(), mFileObserver->IndexManager()->Count() );
	mDataFileHeader.SetDeviceInfo( mDeviceInfo.FilePos(), mDeviceInfo.DeviceCount() );
	mDataFileHeader.SetIndexInfo( mIndexManager.FilePos(), mIndexManager.Count() );
	mDataFileHeader.Save( mFileObserver );

	mFileObserver->Close();

	mWndDeviceView.EnableChooseDevice( TRUE );
	mWndGraphic.EnableFileInfo( TRUE );	
}

void CMainFrame::OnUpdateControlStop(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable( mGraphicState != GRAPHIC_STOP );
}


void CMainFrame::OnFileOpen()
{
	// TODO: Add your command handler code here	

	CFileDialog tFileDialog( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Data Files (*.pxd)|*.pxd|" );
	if ( IDOK != tFileDialog.DoModal() )
	{
		return;
	}

	mFileObserver->SetFile( ( LPCSTR )tFileDialog.GetPathName(), HSTrue, 0 );

	if ( !mDataFileHeader.ParseHeader( mFileObserver ) )
	{
		HS_ERROR( "Parse Header Error!" );
		MessageBox( "文件解析错误!", "警告" );		
		return;
	}

	mFileObserver->SetDisperseFileSize( mDataFileHeader.DisperseFileLimit() );

	//mFileObserver->SetFilePosition( mDataFileHeader.IndexPos() );
	if ( !mFileObserver->SetFilePosition( mDataFileHeader.IndexPos() ) )
	{
		HS_ERROR( "Set File Position Error!" );
	}

	if ( !mIndexManager.Load( mFileObserver, mDataFileHeader.IndexCount() ) )
	{
		HS_ERROR( "IndexManager Load Error, File Position: %lld!", mDataFileHeader.IndexPos() );
		MessageBox( "文件解析错误!", "警告" );
		return;
	}

	mFileObserver->SetFilePosition( mDataFileHeader.DevicePos() );
	if ( !mDeviceInfo.Load( mFileObserver, mDataFileHeader.DeviceCount() ) )
	{
		HS_ERROR( "DeviceInfo Load Error!" );
		MessageBox( "文件解析错误!", "警告" );
		return;
	}

	mFileObserver->SetFilePosition( mDataFileHeader.DeviceIndexPos() );
	if ( !mFileObserver->IndexManager()->Load( mFileObserver, mDataFileHeader.DeviceIndexCount() ) )
	{
		HS_ERROR( "FileObserver IndexManager Load Error!" );
		MessageBox( "文件解析错误!", "警告" );
		return;
	}
	
	vector< vector< HSInt > > tDevicesOpenChannels;
	mDeviceManager->ClearAllDevices();
	for ( HSUInt i = 0; i < mDeviceInfo.DeviceCount(); i++ )
	{
		vector< HSInt > tChannels;
		vector< HSInt > tSampleRates;
		vector< HSInt > tOpenChannels;
		DEVICE_CH_IDENTIFIER tType = mDeviceInfo.DeviceWithIndex( i );
		for ( HSUInt j = 0; j < mDeviceInfo.ChannelCountInDevice( tType ); j++ )
		{
			tChannels.push_back( mDeviceInfo.ChannelInfoWithIndex( tType, j ).Channel );
			tSampleRates.push_back( mDeviceInfo.ChannelInfoWithIndex( tType, j ).SampleRate );

			if ( mDeviceInfo.ChannelInfoWithIndex( tType, j ).Switch )
			{
				tOpenChannels.push_back( mDeviceInfo.ChannelInfoWithIndex( tType, j ).Channel );
			}
		}

		mDeviceManager->AddDevice( tType, mDataFileHeader.Version(), mDeviceInfo.DeviceSavePos( tType ), tOpenChannels, tSampleRates );
		tDevicesOpenChannels.push_back( tOpenChannels );
	}

	theApp.StartTime( mDataFileHeader.StartTime() );
	
	mWndGraphic.UpdateFileSize( ( HSDouble )mFileObserver->IndexManager()->TotalSizeOfAllType() );
	mWndGraphic.SetFilePath( tFileDialog.GetPathName() );
	mWndGraphic.SetDisperseFileLimit( mDataFileHeader.DisperseFileLimit() );
	mWndGraphic.SetFocusedGraphic( NULL );

	mWndDeviceView.FillDeviceView( HSFalse, &tDevicesOpenChannels );

	mIsReviewMode = HSTrue;	

	ResetAllGraphic();

	mWndGraphic.EnableFileInfo( FALSE );

	CString tInfo;
	tInfo.Format( "打开数据文件：%s", ( LPCSTR )tFileDialog.GetPathName() );
	mWndOutput.PrintInfo( tInfo );

	HS_INFO( "Open Data File：%s", ( LPCSTR )tFileDialog.GetPathName() );
}

LRESULT CMainFrame::OnUpdateFileSize( UINT wParam, LPARAM lParam )
{
	HSDouble tValue = wParam;
	tValue *= mFileObserver->FileBlockSize();
	tValue += ( HSInt )lParam;
	mWndGraphic.UpdateFileSize( tValue );
	return 0;
}

void CMainFrame::OnFileClose()
{
	// TODO: Add your command handler code here
	SetAllGraphicState( GRAPHIC_STOP );

	mWndGraphic.UpdateFileSize( 0 );
	mWndGraphic.SetFilePath( "" );	

	mWndDeviceView.FillDeviceView();

	mWndGraphic.SetFocusedGraphic( NULL );

	mIsReviewMode = HSFalse;	

	ResetAllGraphic();	

	mWndDeviceView.EnableChooseDevice( TRUE );
	mWndGraphic.EnableFileInfo( TRUE );

	mWndOutput.PrintInfo( "关闭数据文件..." );
}


void CMainFrame::OnUpdateFileClose(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable( this->mIsReviewMode );	
}

void CMainFrame::SetDocTemplate( CDocTemplate *pDocTemplate )
{
	this->mDocTemplate = pDocTemplate;
}

void CMainFrame::SetAllGraphicState( GRAPHIC_STATE tState )
{
	//HS_INFO( "Begin Set All Grahpic State!" );

	if ( tState == GRAPHIC_STOP )
	{
		CArgCalculator::SharedInstance()->Reset();
	}
	else if ( tState == GRAPHIC_START )
	{
		CArgCalculator::SharedInstance()->Start();
	}

	//HS_INFO( "Arg Calculator Stoped!" );

	POSITION tPosition = this->mDocTemplate->GetFirstDocPosition();	
	while( tPosition )
	{
		CDocument *pDoc = mDocTemplate->GetNextDoc( tPosition );
		IDocInfoProtocol *pGeneralDoc = dynamic_cast< IDocInfoProtocol * >( pDoc );
		if ( pGeneralDoc->DocType() == GRAPHIC_DOC )
		{		
			CPXUpperMonitorDoc *pPXGraphicDoc = dynamic_cast< CPXUpperMonitorDoc * >( pDoc );
			if( tState == GRAPHIC_START )
			{
				pPXGraphicDoc->StartView();
			}
			else if( tState == GRAPHIC_PAUSE )
			{
				pPXGraphicDoc->PauseView();
			}
			else
			{
				pPXGraphicDoc->StopView();
				//HS_INFO( "Stop Doc View!" );
			}
		}
	}

	mGraphicState = tState;	

	//HS_INFO( "Finish Set All Grahpic State!" );
}

void CMainFrame::ResetAllGraphic()
{
	CArgCalculator::SharedInstance()->Reset();

	POSITION tPosition = this->mDocTemplate->GetFirstDocPosition();	
	while( tPosition )
	{
		CDocument *pDoc = mDocTemplate->GetNextDoc( tPosition );
		IDocInfoProtocol *pGeneralDoc = dynamic_cast< IDocInfoProtocol * >( pDoc );
		if ( pGeneralDoc->DocType() == GRAPHIC_DOC )
		{		
			CPXUpperMonitorDoc *pPXGraphicDoc =  dynamic_cast< CPXUpperMonitorDoc * >( pDoc );		
			pPXGraphicDoc->Reset();	
		}
	}

	CListBuffer::ListItemManager()->EndReset();
}

void CMainFrame::OnUpdateFileOpen(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable( mGraphicState == GRAPHIC_STOP );
}


void CMainFrame::OnOptionsExporttotxt()
{
	// TODO: Add your command handler code here
	if ( mIndexManager.Count() < 1 )
	{
		MessageBox( "没有数据!", "警告" );
		return;
	}

	CDataExportToTXT tDataExportToTXT;
	tDataExportToTXT.SetDeviceInfo( mDeviceManager, &mIndexManager, mFileObserver->File() );
	tDataExportToTXT.DoModal();	
}


void CMainFrame::OnUpdateOptionsExporttotxt(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable( mGraphicState == GRAPHIC_STOP && theApp.DataExportEnabled() );
}


void CMainFrame::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	if ( mGraphicState != GRAPHIC_STOP )
	{
		CString tMsg = "数据回放中，确认退出?";
		if ( !mIsReviewMode )
		{
			tMsg = "数据采集中，确认退出?";
		}

		if ( IDYES != MessageBox( tMsg, "警告", MB_ICONQUESTION | MB_YESNO ) )
		{
			return;
		}

		if ( mIsReviewMode )
		{
			this->OnControlStop();
		}
		else
		{
			this->StopSampleData();
		}
	}	

	if ( IDYES == MessageBox( "是否保存已命名的布局?", "警告", MB_ICONQUESTION | MB_YESNO ) )
	{
		SaveAllLayout();
	}
	else
	{
		CINIConfig tIniConfig;
		tIniConfig.LoadFile( mConfigFilePath.c_str() );	

		CDeviceManager::Save( &tIniConfig );
		tIniConfig.SetValue( "Save File Path", ( LPCSTR )this->mWndGraphic.FilePath(), "DATA FILE" );

		tIniConfig.Save( mConfigFilePath.c_str() );	
	}

	CWaveGenClient::SharedInstance()->Close();

	CMDIFrameWndEx::OnClose();
}

void CMainFrame::SaveAllLayout()
{
	CINIConfig tIniConfig;
	tIniConfig.LoadFile( mConfigFilePath.c_str() );	
	tIniConfig.ClearGroup( "INIT LAYOUT" );

	HSInt tIndex = 0;
	HSChar tBuf[ 32 ];
	POSITION tPosition = this->mDocTemplate->GetFirstDocPosition();	
	while( tPosition )
	{
		CDocument *pDoc = mDocTemplate->GetNextDoc( tPosition );
		IDocInfoProtocol *pGeneralDoc = dynamic_cast< IDocInfoProtocol * >( pDoc );
		if ( pGeneralDoc->DocType() == GRAPHIC_DOC )
		{		
			CPXUpperMonitorDoc *pPXGraphicDoc =  dynamic_cast< CPXUpperMonitorDoc * >( pDoc );
			if ( pPXGraphicDoc->GetLayout() != "" )
			{
				pPXGraphicDoc->SaveLayout();
				sprintf_s( tBuf, "Layout%d", ++tIndex );
				tIniConfig.SetValue( tBuf, ( LPCSTR )pPXGraphicDoc->GetLayout(), "INIT LAYOUT" );
			}
		}
	}

	CDeviceManager::Save( &tIniConfig );

	tIniConfig.SetValue( "Save File Path", ( LPCSTR )this->mWndGraphic.FilePath(), "DATA FILE" );

	tIniConfig.Save( mConfigFilePath.c_str() );	
}

void CMainFrame::OnViewGraphictoolbar()
{
	// TODO: Add your command handler code here
	mWaveContrlToolBar.ShowPane( !mWaveContrlToolBar.IsVisible(), TRUE, TRUE );	
	this->RecalcLayout();
}


void CMainFrame::OnUpdateViewGraphictoolbar(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( mWaveContrlToolBar.IsVisible() );	
}


void CMainFrame::OnViewGraphicPointer()
{
	// TODO: Add your command handler code here
	mGrapicCheckType = GRAPHIC_CHECK_POINTER;
}


void CMainFrame::OnUpdateViewGraphicPointer(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( mGrapicCheckType == GRAPHIC_CHECK_POINTER );
}


void CMainFrame::OnViewGraphicNextFrame()
{
	// TODO: Add your command handler code here
	mGrapicCheckType = GRAPHIC_CHECK_STEP_FRAME;
}


void CMainFrame::OnUpdateViewGraphicNextFrame(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( mGrapicCheckType == GRAPHIC_CHECK_STEP_FRAME );
}


void CMainFrame::OnViewGraphicRectangle()
{
	// TODO: Add your command handler code here
	mGrapicCheckType = GRAPHIC_CHECK_RECTANGE;
}


void CMainFrame::OnUpdateViewGraphicRectangle(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( mGrapicCheckType == GRAPHIC_CHECK_RECTANGE );
}


void CMainFrame::OnViewGraphicDrag()
{
	// TODO: Add your command handler code here
	mGrapicCheckType = GRAPHIC_CHECK_DRAG;
}


void CMainFrame::SetFocusedGraphic( IGraphicProtocol * pGraphic, HSBool tForceRefresh )
{	
	if ( mFocusedGrahpic != NULL && mFocusedGrahpic != pGraphic )
	{
		mFocusedGrahpic->FocusGraphic( HSFalse );
	}

	mFocusedGrahpic = pGraphic;
	this->mWndGraphic.SetFocusedGraphic( mFocusedGrahpic, tForceRefresh );	
}

void CMainFrame::OnUpdateViewGraphicDrag(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( mGrapicCheckType == GRAPHIC_CHECK_DRAG );
}

HSBool CMainFrame::IsSettingArgEnabled()
{
	if ( this->mIsReviewMode || mGraphicState != GRAPHIC_STOP )
	{
		return HSFalse;
	}

	return HSTrue;
}


void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// TODO: Add your specialized code here and/or call the base class
	this->SetWindowText( "PXAES数字化全波形声发射检测系统-鹏翔" );

	//CMDIFrameWndEx::OnUpdateFrameTitle(bAddToTitle);
}
