
// PXUpperMonitor.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "PXUpperMonitor.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "PXUpperMonitorDoc.h"
#include "PXUpperMonitorView.h"

#include "PCI1714SettingDoc.h"
#include "PCI1714SettingView.h"
#include "DeviceTypes.h"

#include "INIConfig.h"

#include "HSLogProtocol.h"
#include "GraphicManager.h"

#include "EasyUSBSettingDoc.h"
#include "EasyUSBSettingView.h"

#include "PCI2SettingDoc.h"
#include "PCI2SettingView.h"

#include "ARB1410SettingDoc.h"
#include "ARB1410SettingView.h"

#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPXUpperMonitorApp

BEGIN_MESSAGE_MAP(CPXUpperMonitorApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CPXUpperMonitorApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CPXUpperMonitorApp::OnLayoutNew)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CPXUpperMonitorApp construction

CPXUpperMonitorApp::CPXUpperMonitorApp()
{
	mInit = false;

	m_bHiColorIcons = TRUE;

	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("PXUpperMonitor.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	DWORD tLength = 1025;
	HSChar tBuf[ 1024 ];
	memset( tBuf, 0, sizeof tBuf );
	::GetCurrentDirectory( tLength, tBuf );

	mPropgramFilePath = tBuf;

	strcat_s( tBuf, "\\Configs\\config.ini" );
	CMainFrame::mConfigFilePath = tBuf;
}

// The one and only CPXUpperMonitorApp object

CPXUpperMonitorApp theApp;


// CPXUpperMonitorApp initialization

BOOL CPXUpperMonitorApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction();
	

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	WriteProfileStringA( _T( "Workspace\\Keyboard-0" ), _T( "Accelerators" ),NULL);


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);	

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_PXUpperMonitorTYPE_C,
		RUNTIME_CLASS(CPXUpperMonitorDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CPXUpperMonitorView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	mNormalDocTemplate = pDocTemplate;

	pDocTemplate = new CMultiDocTemplate(IDR_PXUpperMonitorTYPE_C,
		RUNTIME_CLASS(CPCI1714SettingDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CPCI1714SettingView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	mDeviceSettinsDocTemplate[ CARD_PCI1714 ] = pDocTemplate;

	pDocTemplate = new CMultiDocTemplate(IDR_PXUpperMonitorTYPE_C,
		RUNTIME_CLASS(CEasyUSBSettingDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CEasyUSBSettingView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	mDeviceSettinsDocTemplate[ CARD_EASYUSB ] = pDocTemplate;
	
	pDocTemplate = new CMultiDocTemplate(IDR_PXUpperMonitorTYPE_C,
		RUNTIME_CLASS(CPCI2SettingDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CPCI2SettingView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	mDeviceSettinsDocTemplate[ CARD_PAPCI2 ] = pDocTemplate;

	pDocTemplate = new CMultiDocTemplate(IDR_PXUpperMonitorTYPE_C,
		RUNTIME_CLASS(CARB1410SettingDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CARB1410SettingView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	mDeviceSettinsDocTemplate[ CARD_ARB1410 ] = pDocTemplate;
	

	if ( !InitLog() )
	{
		return FALSE;
	}

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME_C))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	pMainFrame->SetDocTemplate( mNormalDocTemplate );
	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

int CPXUpperMonitorApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CPXUpperMonitorApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
	CFont mFont;
	CRect mWebRect;
// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnStnClickedStaticWeb();
	afx_msg void OnStnClickedStaticDesc();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
	ON_WM_SETCURSOR()
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
	ON_STN_CLICKED(IDC_STATIC_WEB, &CAboutDlg::OnStnClickedStaticWeb)
	ON_STN_CLICKED(IDC_STATIC_DESC, &CAboutDlg::OnStnClickedStaticDesc)
END_MESSAGE_MAP()

// App command to run the dialog
void CPXUpperMonitorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CPXUpperMonitorApp customization load/save methods

void CPXUpperMonitorApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CPXUpperMonitorApp::LoadCustomState()
{
}

void CPXUpperMonitorApp::SaveCustomState()
{
}

string CPXUpperMonitorApp::GetLocalTime()
{
	char tDateBuf[ 64 ];
	memset( tDateBuf, 0, sizeof tDateBuf );

	time_t tTime;
	time( &tTime );
	struct tm * tNow = localtime( &tTime );
	sprintf_s( tDateBuf, "%02d:%02d:%02d", tNow->tm_hour, tNow->tm_min, tNow->tm_sec );

	return tDateBuf;
}

string CPXUpperMonitorApp::GetLocalDate()
{
	char tDateBuf[ 64 ];
	memset( tDateBuf, 0, sizeof tDateBuf );

	time_t tTime;
	time( &tTime );
	struct tm * tNow = localtime( &tTime );
	sprintf_s( tDateBuf, "%04d-%02d-%02d", tNow->tm_year + 1900, tNow->tm_mon + 1, tNow->tm_mday );

	return tDateBuf;
}

// CPXUpperMonitorApp message handlers


void CPXUpperMonitorApp::OnLayoutNew()
{
	if ( !mInit )
	{		
		IGraphicProtocol *pGraphic = NULL;
		CINIConfig tIniConfig;
		if ( tIniConfig.LoadFile( CMainFrame::mConfigFilePath.c_str() ) )
		{
			HSInt tIndex = 0;
			HSChar tBuf[ 32 ];
			sprintf_s( tBuf, "Layout%d", ++tIndex );
			string tPath = "";
			while ( tIniConfig.ValueWithKey( tBuf, tPath, "INIT LAYOUT" ) )
			{
				ifstream tFile( tPath.c_str() );
				if ( tFile.is_open() )
				{
					tFile.close();
					CPXUpperMonitorDoc *pDoc = ( CPXUpperMonitorDoc * )mNormalDocTemplate->OpenDocumentFile( NULL );
					pGraphic = pDoc->OpenLayout( tPath.c_str() );	
					mInit = true;
				}

				sprintf_s( tBuf, "Layout%d", ++tIndex );
			}
		}		

		if ( !mInit )
		{
			mInit = true;
			CPXUpperMonitorDoc *pDoc = ( CPXUpperMonitorDoc * )mNormalDocTemplate->OpenDocumentFile( NULL );
			pGraphic = pDoc->OpenLayout( "Layout\\Waves.pxl" );	
		}

		if ( pGraphic )
		{
			pGraphic->FocusGraphic( HSTrue );
			CMainFrame* pMainFrame = dynamic_cast< CMainFrame* >( m_pMainWnd );					
			pMainFrame->SetFocusedGraphic( pGraphic );
		}
	}
	else
	{
		mNormalDocTemplate->OpenDocumentFile( NULL );
	}	
}

HSVoid CPXUpperMonitorApp::OpenDeviceSettingDlg( DEVICE_CH_IDENTIFIER tCardIdentifier )
{
	tCardIdentifier.InitChannel();	

	if ( mDeviceSettins.find( tCardIdentifier ) == mDeviceSettins.end() )
	{
		mDeviceSettins[ tCardIdentifier ] = NULL;
	}

	CDocument *pDoc = NULL;
	if ( mDeviceSettins[ tCardIdentifier ] == NULL )
	{		
		pDoc = mDeviceSettinsDocTemplate[ tCardIdentifier.CARD_IDENTIFIER ]->OpenDocumentFile( NULL );
		mDeviceSettins[ tCardIdentifier ] = pDoc;
	}
	else
	{
		pDoc = mDeviceSettins[ tCardIdentifier ];		
	}

	POSITION pos = pDoc->GetFirstViewPosition();         
	CView *pView = pDoc->GetNextView(pos);  
	pView->GetParentFrame()->ActivateFrame();

	IDeviceViewSetCardIDProtocol *pCardSetView = dynamic_cast< IDeviceViewSetCardIDProtocol * >( pView );
	pCardSetView->SetCardIdentifier( tCardIdentifier );
}

HSVoid CPXUpperMonitorApp::DeviceSettingDlgClosed( DEVICE_CH_IDENTIFIER tCardIdentifier )
{
	mDeviceSettins[ tCardIdentifier ] = NULL;
}
/*
HSVoid CPXUpperMonitorApp::RefreshDataHandler( HSUInt tCardIdentifier, IDataHandler *pDataHandler )
{
	if ( mDeviceSettins.find( tCardIdentifier ) != mDeviceSettins.end() && mDeviceSettins[ tCardIdentifier ] != NULL )
	{
		CDocument *pDoc = mDeviceSettins[ tCardIdentifier ];

		POSITION pos = pDoc->GetFirstViewPosition();         
		CView *pView = pDoc->GetNextView(pos);  

		IDeviceViewRefreshDataHandlerProtocol *pDeviceView = dynamic_cast< IDeviceViewRefreshDataHandlerProtocol * >( pView );
		pDeviceView->RefreshDataHandler( pDataHandler );
	}
}
*/
HSBool CPXUpperMonitorApp::InitLog()
{
	CINIConfig tIniConfig;
	if ( tIniConfig.LoadFile( CMainFrame::mConfigFilePath.c_str() ) )
	{
		string tValue;
		IHSLogProtocol::HSLOG_LEVEL tLogLevel = IHSLogProtocol::HSLOG_INFO;
		if ( tIniConfig.ValueWithKey( "LOG_LEVEL", tValue, "LOG" ) )
		{
			int tIntValue = atoi( tValue.c_str() );
			switch ( tIntValue )
			{
			case 0:
				tLogLevel = IHSLogProtocol::HSLOG_ERROR;
				break;

			case 1:
				tLogLevel = IHSLogProtocol::HSLOG_WARNING;
				break;

			default:
				tLogLevel = IHSLogProtocol::HSLOG_INFO;
				break;
			}			
		}
		
		IHSLogProtocol::HSLOG_PRINT_TYPE tPrintType = IHSLogProtocol::PRINT_TO_BOTH;
		if ( tIniConfig.ValueWithKey( "LOG_OUTPUT", tValue, "LOG" ) )
		{
			int tIntValue = atoi( tValue.c_str() );
			switch ( tIntValue )
			{
			case 0:
				tPrintType = IHSLogProtocol::PRINT_TO_FILE;
				break;

			case 1:
				tPrintType = IHSLogProtocol::PRINT_TO_TERMINAL;
				break;

			default:
				tPrintType = IHSLogProtocol::PRINT_TO_BOTH;
				break;
			}			
		}

		bool tFlush = true;
		if ( tIniConfig.ValueWithKey( "LOG_FILE_IMMEDIATE_FLUSH", tValue, "LOG" ) )
		{
			int tIntValue = atoi( tValue.c_str() );
			switch ( tIntValue )
			{
			case 0:
				tFlush = false;
				break;			

			default:
				tFlush = true;
				break;
			}			
		}

		int tFileSize = 1;
		if ( tIniConfig.ValueWithKey( "LOG_FILE_MAX_SIZE_MB", tValue, "LOG" ) )
		{
			tFileSize = max( atoi( tValue.c_str() ), 1 );
		}
		tFileSize *= ( 1024 * 1024 );

		int tFileIndexLimit = 16;
		if ( tIniConfig.ValueWithKey( "LOG_FILE_MAX_BACKUP_INDEX", tValue, "LOG" ) )
		{
			tFileIndexLimit = max( atoi( tValue.c_str() ), 1 );
		}
		
		if ( !CHSLogLoader::SharedInstance()->Init( tLogLevel, tPrintType, tFlush, tFileSize, tFileIndexLimit, IHSLogProtocol::HSLOG_BLACK_LIST ) )
		{	
			AfxMessageBox( "日志初始化错误!" );
			return FALSE;
		}

		if ( tIniConfig.ValueWithKey( "LOG_ERROR_FILE", tValue, "LOG" ) )
		{
			HS_LOG->AddFormat( IHSLogProtocol::HSLOG_ERROR, NULL, tValue.c_str() );
		}		

		if ( tIniConfig.ValueWithKey( "LOG_ERROR_FORMAT", tValue, "LOG" ) )
		{
			HS_LOG->AddFormat( IHSLogProtocol::HSLOG_ERROR, tValue.c_str(), NULL );
		}		

		if ( tIniConfig.ValueWithKey( "LOG_WARNING_FILE", tValue, "LOG" ) )
		{
			HS_LOG->AddFormat( IHSLogProtocol::HSLOG_WARNING, NULL, tValue.c_str() );
		}		

		if ( tIniConfig.ValueWithKey( "LOG_WARNING_FORMAT", tValue, "LOG" ) )
		{
			HS_LOG->AddFormat( IHSLogProtocol::HSLOG_WARNING, tValue.c_str(), NULL );
		}		

		if ( tIniConfig.ValueWithKey( "LOG_INFO_FILE", tValue, "LOG" ) )
		{
			HS_LOG->AddFormat( IHSLogProtocol::HSLOG_INFO, NULL, tValue.c_str() );
		}		

		if ( tIniConfig.ValueWithKey( "LOG_INFO_FORMAT", tValue, "LOG" ) )
		{
			HS_LOG->AddFormat( IHSLogProtocol::HSLOG_INFO, tValue.c_str(), NULL );
		}		
	}
	else if ( !CHSLogLoader::SharedInstance()->Init() )
	{			
		AfxMessageBox( "日志初始化错误!" );
		return FALSE;		
	}

	return TRUE;
}

void CAboutDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	CClientDC tDC( this );

	GetDlgItem( IDC_STATIC_WEB )->GetWindowRect( &mWebRect );
	ScreenToClient( mWebRect );	
	
	mFont.CreatePointFont( 110, "隶书", &tDC );
	this->GetDlgItem( IDC_STATIC_DESC )->SetFont( &mFont );

	this->GetDlgItem( IDC_STATIC_DESC )->SetWindowText( "    ZHIS 声音和震动探测分析系统软件的著作权、"
														"版权和知识产权属于长沙鹏翔电子科技有限公司所有，"
														"并受《中华人民共和国著作权法》、《计算机软件保护条理》、"
														"《知识产权保护条例》和相关国际版权条约、"
														"法律、法规，以及其它知识产权法律和条约的保护。\n\n"
														"    任何单位和个人未经ZHIS的授权不能使用、修改、再发布本软件的任何部分，"
														"否则将视为非法侵害，我公司保留依法追究其责任的权利，"
														"此条款同样适用于ZHIS拥有完全权利的文字、图片、表格等内容。" );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CAboutDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );

	if ( tPoint.x > mWebRect.left && tPoint.x < mWebRect.right && tPoint.y > mWebRect.top && tPoint.y < mWebRect.bottom )
	{
		SetCursor( AfxGetApp()->LoadStandardCursor( IDC_HAND ) );
		return true;
	}

	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}


HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	if ( pWnd->GetDlgCtrlID() == IDC_STATIC_WEB )
	{
		LOGFONT tFont;
		GetFont()->GetObject(sizeof( tFont ), &tFont );
		tFont.lfUnderline = TRUE;

		CFont tPointFont;
		tPointFont.CreateFontIndirect( &tFont );

		pDC->SelectObject( &tPointFont );
		pDC->SetTextColor( RGB( 0, 0, 255 ) );
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}


void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if ( point.x > mWebRect.left && point.x < mWebRect.right && point.y > mWebRect.top && point.y < mWebRect.bottom )
	{
		ShellExecute( NULL, "open", TEXT( "http://www.zhis.net" ), NULL, NULL, SW_SHOWNORMAL );
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CAboutDlg::OnStnClickedStaticWeb()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CAboutDlg::OnStnClickedStaticDesc()
{
	// TODO: 在此添加控件通知处理程序代码
}
