// ArgTable.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "ArgTable.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "ArgProperty.h"
#include "HSLogProtocol.h"
#include "DataExportHelper.h"

// CArgTable dialog

IMPLEMENT_DYNAMIC(CArgTable, CDialogEx)

CArgTable::CArgTable(CWnd* pParent /*=NULL*/)
	: CDialogEx(CArgTable::IDD, pParent)
{
	mInit = HSFalse;
	
	mDataHandler = NULL;
	mChannel = 0;

	mDataIdentifier = 0;	

	mIsFullScreen = HSFalse;

	mUpdateArgThread = new CThreadControlEx< CArgTable >( this, 800, 0, HSTrue );	

	mArgMutex = CreateEvent( NULL, FALSE, TRUE, NULL );	
}

CArgTable::~CArgTable()
{
	CleanChannelArgs();

	delete mUpdateArgThread;

	CloseHandle( mArgMutex );
}

void CArgTable::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ARGT, mArgList);
	DDX_Control(pDX, IDC_STATIC_TITLE, mStaticTitle);
}


BEGIN_MESSAGE_MAP(CArgTable, CDialogEx)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()	
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_ARGPOPUP_CLOSE, &CArgTable::OnArgpopupClose)
	ON_COMMAND(ID_ARGPOPUP_EXPORTDATA, &CArgTable::OnArgpopupExportdata)
	ON_COMMAND(ID_ARGPOPUP_FULL_SCREEN, &CArgTable::OnArgpopupFullScreen)
	ON_UPDATE_COMMAND_UI(ID_ARGPOPUP_FULL_SCREEN, &CArgTable::OnUpdateArgpopupFullScreen)
	ON_COMMAND(ID_ARGPOPUP_SETDATASOURCE, &CArgTable::OnArgpopupSetdatasource)
	ON_WM_CONTEXTMENU()	
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CArgTable message handlers


BOOL CArgTable::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	
	mArgList.SetExtendedStyle( mArgList.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );	
	mArgList.InsertColumn( 0, "", LVCFMT_CENTER, 0 );
	mArgList.InsertColumn( 1, "序号", LVCFMT_CENTER, 40 );
	mArgList.InsertColumn( 2, "通道", LVCFMT_CENTER, 40 );
	mArgList.InsertColumn( 3, "时间( hh:mm:ss mmm uuu n )", LVCFMT_CENTER, 200 );
	mArgList.InsertColumn( 4, "上升时间( us )", LVCFMT_CENTER, 150 );
	mArgList.InsertColumn( 5, "持续时间( us )", LVCFMT_CENTER, 150 );
	mArgList.InsertColumn( 6, "振铃计数", LVCFMT_CENTER, 100 );
	mArgList.InsertColumn( 7, "能量", LVCFMT_CENTER, 100 );
	mArgList.InsertColumn( 8, "幅度( dB )", LVCFMT_CENTER, 100 );
	//mArgList.InsertColumn( 9, "ASL", LVCFMT_CENTER, 100 );
	//mArgList.InsertColumn( 10, "RMS", LVCFMT_CENTER, 100 );

	mImageList.Create( 24, 24, 0, 1, 1 );
	mArgList.SetImageList( &mImageList, 1 );

	mArgList.SetParent( this );

	mInit = HSTrue;

	mPopupMenu.LoadMenu( IDR_POPUP_ARG_C );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CArgTable::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if ( !mInit )
	{
		return;
	}

	CRect tRect;
	this->GetClientRect( &tRect );	

	CRect tTitleRect = CRect( tRect.left, tRect.top, tRect.right, tRect.top + 24 );

	mStaticTitle.MoveWindow( &tTitleRect );
	mStaticTitle.ResizeWave();
	mStaticTitle.Invalidate();

	CRect tListRect = CRect( tRect.left, tRect.top + 24, tRect.right, tRect.bottom );	
	mArgList.MoveWindow( tListRect );	

}

HSBool CArgTable::GetNextArg( CArgTableManager::ArgItem * &pArg, HSInt &tChannel )
{	
	pArg = NULL;
	HSUInt64 tTime = 0;
	HSInt tChannelIndex = -1;
	for ( HSUInt i = 0; i < mChannelArgs.size(); i++ )	
	{
		HSUInt tIndex = mChannelArgs[ i ].Index;
		if ( mChannelArgs[ i ].ArgList.size() > tIndex && ( pArg == NULL || mChannelArgs[ i ].ArgList[ tIndex ].NBeginTime < tTime ) )
		{
			pArg = &( mChannelArgs[ i ].ArgList[ tIndex ] );
			tChannel = mChannelArgs[ i ].Channel;
			tTime = mChannelArgs[ i ].ArgList[ tIndex ].NBeginTime;
			tChannelIndex = i;
		}
	}				

	if ( tChannelIndex < 0 )
	{
		return HSFalse;
	}

	mChannelArgs[ tChannelIndex ].Index++;

	return HSTrue;
}

HSBool CArgTable::ThreadRuning( HSInt tThreadID )
{
	WaitForSingleObject( mArgMutex, INFINITE );	

	for ( HSUInt i = 0; i < mChannelArgs.size(); i++ )	
	{
		mChannelArgs[ i ].Index = 0;
		mChannelArgs[ i ].ArgList.clear();
		mChannelArgs[ i ].ArgTransfer->GetArg( mChannelArgs[ i ].ArgList );	
	}

	CArgTableManager::ArgItem *pArg = NULL;
	HSInt tChannel = 0;

	while ( !mUpdateArgThread->IsStoped() && GetNextArg( pArg, tChannel ) )
	{		
		HSInt tIndex = mArgList.GetItemCount();			

		mArgList.InsertItem( tIndex, "", 0 );
		mArgList.SetItemText( tIndex, 0, "" );

		CString tStr;
		tStr.Format( "%d", tIndex + 1 );
		mArgList.SetItemText( tIndex, 1, tStr );		

		tStr.Format( "%d", tChannel + 1 );
		mArgList.SetItemText( tIndex, 2, tStr );		

		tStr.Format( "%lld", pArg->NBeginTime / 1000 );
		mArgList.SetItemText( tIndex, 0, tStr );
		mArgList.SetItemText( tIndex, 3, GetFormatTime(  pArg->NBeginTime ) );

		tStr.Format( "%d",  pArg->IncreaseNTime / 1000 );
		mArgList.SetItemText( tIndex, 4, tStr );

		tStr.Format( "%d",  pArg->NDuration / 1000 );
		mArgList.SetItemText( tIndex, 5, tStr );

		tStr.Format( "%d",  pArg->RingCount );
		mArgList.SetItemText( tIndex, 6, tStr );

		tStr.Format( "%.5f",  pArg->Energy );
		mArgList.SetItemText( tIndex, 7, tStr );

		tStr.Format( "%.5f",  pArg->Amplitude );
		mArgList.SetItemText( tIndex, 8, tStr );
/*
		tStr.Format( "%.5f",  pArg->ASL );
		mArgList.SetItemText( tIndex, 9, tStr );

		tStr.Format( "%.5f",  pArg->RMS );
		mArgList.SetItemText( tIndex, 10, tStr );	*/	
	}		

	SetEvent( mArgMutex );	

	return !mUpdateArgThread->IsStoped();
}

HSVoid CArgTable::ThreadWillStop( HSInt tThreadID )
{	
}

CString CArgTable::GetFormatTime( HSUInt64 tNSecond )
{
	HSUInt64 tOneSecond = 1000 * 1000 * 1000;

	HSUInt64 tOneHour = tOneSecond;
	tOneHour *= 3600;	

	HSUInt64 tOneMinute = tOneSecond;
	tOneMinute *= 60;	

	HSUInt tResHour = ( HSUInt )( tNSecond / tOneHour );	
	tNSecond = tNSecond % tOneHour;

	HSUInt tResMinute = ( HSUInt )( tNSecond / tOneMinute );	
	tNSecond = tNSecond % tOneMinute;

	HSUInt tResSecond = ( HSUInt )( tNSecond / tOneSecond );
	tNSecond = tNSecond % tOneSecond;

	HSUInt tResMSecond = ( HSUInt )( tNSecond / 1000 / 1000 );
	tNSecond = tNSecond % 1000000;

	HSUInt tResUSecond = ( HSUInt )( tNSecond / 1000 );
	tNSecond = tNSecond % 1000;

	CString tStr;
	tStr.Format( "%02d:%02d:%02d %03d %03d %d", tResHour, tResMinute, tResSecond, tResMSecond, tResUSecond, ( HSUInt )( tNSecond / 100 ) );

	return tStr;
}

HSVoid CArgTable::CleanChannelArgs()
{
	for ( HSUInt i = 0; i < mChannelArgs.size(); i++ )
	{
		delete mChannelArgs[ i ].ArgTransfer;		
	}

	mChannelArgs.clear();	
}

HSVoid CArgTable::SetRelationTitle()
{	
	CString tTitle = "";
	vector< HSUInt > tChannels;
	HSInt tChannelCount = mDataIdentifier.NumOfChannels( &tChannels );
	if ( mDataHandler && tChannelCount > 0 )
	{		
		CString tStrChannels = "所有通道";
		if ( tChannelCount != mDataHandler->ChannelNum() )
		{
			tStrChannels = "";
			for ( HSUInt i = 0; i < tChannels.size(); i++ )
			{
				CString tStrChannel;
				tStrChannel.Format( ( tStrChannels == "" ? "%d" : ", %d" ), tChannels[ i ] + 1 );
				tStrChannels += tStrChannel;
			}
		}

		tTitle.Format( "%s < %s > ", mDataHandler->Name().c_str(), tStrChannels );
	}
	else
	{
		tTitle = "DEVICE < CHANNEL > ";
	}	

	mStaticTitle.SetTitle( tTitle );
}

HSVoid CArgTable::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mArgMutex, INFINITE );

	CleanChannelArgs();

	this->mDataIdentifier = tIdentifier;

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	DEVICE_CH_IDENTIFIER tCardIdentifer( tIdentifier.CARD_IDENTIFIER, tIdentifier.CARD_INDEX );
	mDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( tCardIdentifer );	

	SetRelationTitle();
	mStaticTitle.Invalidate();

	vector< HSUInt > tChannels;
	if ( mDataHandler && tIdentifier.NumOfChannels( &tChannels ) > 0 )
	{				
		for ( HSUInt i = 0; i < tChannels.size(); i++ )
		{
			tCardIdentifer.InitChannel( tChannels[ i ] );
			ChannelArgInfo tChannelArgInfo;			
			tChannelArgInfo.ArgTransfer = new CArgListTransfer( mDataHandler, tCardIdentifer );			
			tChannelArgInfo.Channel = tChannels[ i ];

			mChannelArgs.push_back( tChannelArgInfo );	
		}		

		mArgList.DeleteAllItems();

		if ( pMainFrame->IsStart() )
		{
			this->Start();
		}		
	}	

	SetEvent( mArgMutex );
}

HSBool CArgTable::ShowFullScreen( HSBool tFullScreen, CWnd *pWnd )
{ 
	mIsFullScreen = ( pWnd != NULL ? tFullScreen : mIsFullScreen );
	return mIsFullScreen;
}

IGraphicPropertyProtocol * CArgTable::CreatePropertyDlg( CWnd *pParent )
{
	CArgProperty *pArgProperty = new CArgProperty;
	pArgProperty->Create( IDD_ARGPROPERTY, pParent );
	return pArgProperty;
}

HSBool CArgTable::Start()
{
	if ( mChannelArgs.size() > 0 && mDataHandler != NULL )
	{		
		mUpdateArgThread->Start();
		return HSTrue;
	}

	return HSFalse;	
}

HSVoid CArgTable::Pause()
{
	mUpdateArgThread->Pause();
}

HSVoid CArgTable::Stop()
{	
	mUpdateArgThread->Stop();
}

HSBool CArgTable::Save( CINIConfig *pIniConfig, string tGroup )
{
	return HSTrue; 
}

HSBool CArgTable::Load( CINIConfig *pIniConfig, string tGroup )
{
	return HSTrue; 
}

void CArgTable::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	/*CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( !mStaticTitle.Focused() )
	{
		pMainFrame->SetFocusedGraphic( this );
		mStaticTitle.SetFocused( true );
		mStaticTitle.ResizeWave();
		mStaticTitle.Invalidate();		
	}
	*/

	mParent->BeginMoveSubView( this, nFlags, point);

	__super::OnLButtonDown(nFlags, point);
}


void CArgTable::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	mParent->EndMoveSubView( nFlags, point);

	__super::OnLButtonUp(nFlags, point);
}


void CArgTable::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	mParent->MoveingSubView( nFlags, point);

	__super::OnMouseMove(nFlags, point);
}

HSVoid CArgTable::FocusGraphic( HSBool tIsFocused )
{
	if ( tIsFocused && !mStaticTitle.Focused() )
	{
		mStaticTitle.SetFocused( true );
		mStaticTitle.ResizeWave();
		mStaticTitle.Invalidate();
	}
	else if ( !tIsFocused && mStaticTitle.Focused() )
	{
		mStaticTitle.SetFocused( false );
		mStaticTitle.ResizeWave();
		mStaticTitle.Invalidate();
	}
}


void CArgTable::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	__super::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// TODO: Add your message handler code here
	 ASSERT(pPopupMenu != NULL);     
    // Check the enabled state of various menu items.     
    
    CCmdUI state;     
    state.m_pMenu = pPopupMenu;     
    ASSERT(state.m_pOther == NULL);     
    ASSERT(state.m_pParentMenu == NULL);     
    
    // Determine if menu is popup in top-level menu and set m_pOther to     
    // it if so (m_pParentMenu == NULL indicates that it is secondary popup).     
    HMENU hParentMenu;     
    if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)     
        state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.     
    else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)     
    {     
        CWnd* pParent = this;     
           // Child windows don't have menus--need to go to the top!     
        if (pParent != NULL &&     
           (hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)     
        {     
           int nIndexMax = ::GetMenuItemCount(hParentMenu);     
           for (int nIndex = 0; nIndex < nIndexMax; nIndex++)     
           {     
            if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)     
            {     
                // When popup is found, m_pParentMenu is containing menu.     
                state.m_pParentMenu = CMenu::FromHandle(hParentMenu);     
                break;     
            }     
           }     
        }     
    }     
    
    state.m_nIndexMax = pPopupMenu->GetMenuItemCount();     
    for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;     
      state.m_nIndex++)     
    {     
        state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);     
        if (state.m_nID == 0)     
           continue; // Menu separator or invalid cmd - ignore it.     
    
        ASSERT(state.m_pOther == NULL);     
        ASSERT(state.m_pMenu != NULL);     
        if (state.m_nID == (UINT)-1)     
        {     
           // Possibly a popup menu, route to first item of that popup.     
           state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);     
           if (state.m_pSubMenu == NULL ||     
            (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||     
            state.m_nID == (UINT)-1)     
           {     
            continue;       // First item of popup can't be routed to.     
           }     
           state.DoUpdate(this, TRUE);   // Popups are never auto disabled.     
        }     
        else    
        {     
           // Normal menu item.     
           // Auto enable/disable if frame window has m_bAutoMenuEnable     
           // set and command is _not_ a system command.     
           state.m_pSubMenu = NULL;     
           state.DoUpdate(this, FALSE);     
        }     
    
        // Adjust for menu deletions and additions.     
        UINT nCount = pPopupMenu->GetMenuItemCount();     
        if (nCount < state.m_nIndexMax)     
        {     
           state.m_nIndex -= (state.m_nIndexMax - nCount);     
           while (state.m_nIndex < nCount &&     
            pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)     
           {     
            state.m_nIndex++;     
           }     
        }     
        state.m_nIndexMax = nCount;     
    } 
}


void CArgTable::OnArgpopupClose()
{
	// TODO: Add your command handler code here

	CDialogEx::OnOK();

	mParent->SubViewClosed( this );
}


void CArgTable::OnArgpopupExportdata()
{
	// TODO: Add your command handler code here
	
	if ( mArgList.GetItemCount() < 1 )
	{
		MessageBox( "没有数据!", "警告" );
		return;
	}		

	HSBool tIsStart = mUpdateArgThread->IsStart();
	if ( tIsStart )
	{
		this->Stop();
	}
	
	CDataExportHelper tDataExport( this );
	if ( tDataExport.GetFilePath() )
	{
		HSChar tBuf[ 2048 ];

		tDataExport.Write( "参数表\n" );
		sprintf_s( tBuf, "日期: %s %s\n\n", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
		tDataExport.Write( tBuf ); 

		//sprintf_s( tBuf, "% 10s\t% 10s\t% 28s\t% 16s\t% 16s\t% 10s\t% 10s\t% 10s\t% 10s\t% 10s\n", "序号", "通道", "时间( hh:mm:ss mmm uuu n )", "上升时间( us )", "持续时间( us )", "振铃计数", "能量", "幅度( dB )", "ASL", "RMS" );
		sprintf_s( tBuf, "% 10s\t% 10s\t% 28s\t% 16s\t% 16s\t% 10s\t% 10s\t% 10s\n", "序号", "通道", "时间( hh:mm:ss mmm uuu n )", "上升时间( us )", "持续时间( us )", "振铃计数", "能量", "幅度( dB )" );
		tDataExport.Write( tBuf ); 

		for ( HSInt i = 0; i < mArgList.GetItemCount(); i++ )
		{	
			sprintf_s( tBuf, "% 10s\t% 10s\t% 28s\t% 16s\t% 16s\t% 10s\t% 10s\t% 10s\n",
				mArgList.GetItemText( i, 1 ),
				mArgList.GetItemText( i, 2 ), 
				mArgList.GetItemText( i, 3 ),
				mArgList.GetItemText( i, 4 ),
				mArgList.GetItemText( i, 5 ),
				mArgList.GetItemText( i, 6 ),
				mArgList.GetItemText( i, 7 ),
				mArgList.GetItemText( i, 8 ) /*,
											 mArgList.GetItemText( i, 9 ),
											 mArgList.GetItemText( i, 10 )*/ );
			tDataExport.Write( tBuf ); 
		}

		tDataExport.Finish();
	}	

	if ( tIsStart )
	{
		this->Start();
	}
}


void CArgTable::OnArgpopupFullScreen()
{
	// TODO: Add your command handler code here

	mIsFullScreen = !mIsFullScreen;
	mParent->SubViewFullScreen( this, mIsFullScreen );
}


void CArgTable::OnUpdateArgpopupFullScreen(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( mIsFullScreen );	
}


void CArgTable::OnArgpopupSetdatasource()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->OnViewGraphic();	

	if ( !mStaticTitle.Focused() || pMainFrame->FocusedGraphic() != this )
	{
		pMainFrame->SetFocusedGraphic( this );
		mStaticTitle.SetFocused( true );
		mStaticTitle.ResizeWave();
		mStaticTitle.Invalidate();
	}
}


void CArgTable::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: Add your message handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->SetGrahpicCheckType( CMainFrame::GRAPHIC_CHECK_POINTER );
	
	CPoint tPoint;
	::GetCursorPos( &tPoint );
	
	mPopupMenu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, tPoint.x, tPoint.y, this );
}

HSVoid CArgTable::FocusWnd()
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( !mStaticTitle.Focused() )
	{
		pMainFrame->SetFocusedGraphic( this );
		mStaticTitle.SetFocused( true );
		mStaticTitle.ResizeWave();
		mStaticTitle.Invalidate();
	}
}


void CArgTable::OnDestroy()
{
	__super::OnDestroy();

	// TODO: Add your message handler code here

	mUpdateArgThread->Stop();
}
