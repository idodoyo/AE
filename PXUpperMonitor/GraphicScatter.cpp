// GraphicScatter.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GraphicScatter.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "ScatterProperty.h"
#include "HSLogProtocol.h"


// CGraphicScatter dialog

IMPLEMENT_DYNAMIC(CGraphicScatter, CDialogEx)

CGraphicScatter::CGraphicScatter(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGraphicScatter::IDD, pParent), mXArg( 0, HSTrue ), mYArg( 2, HSFalse )
{
	mInit = HSFalse;
	mRelationMutex = CreateEvent( NULL, FALSE, TRUE, NULL );
	mDataHandler = NULL;

	mUpdateRelationThread = new CThreadControlEx< CGraphicScatter >( this, 800, 0 );

	mIsFullScreen = HSFalse;

	mIsDraging = HSFalse;		
	mIsZoomIn = HSFalse;	
}

CGraphicScatter::~CGraphicScatter()
{
	CloseHandle( mRelationMutex );

	delete mUpdateRelationThread;

	CleanChannelArgs();
}

HSBool CGraphicScatter::Start()
{
	if ( mChannelArgs.size() > 0 )
	{		
		mUpdateRelationThread->Start();
		return HSTrue;
	}
	return HSFalse;
}

HSVoid CGraphicScatter::Pause()
{
	mUpdateRelationThread->Pause();
}

HSVoid CGraphicScatter::Stop()
{
	mUpdateRelationThread->Stop();
	//HS_INFO( "GraphicScatter stoped!" );
}

HSBool CGraphicScatter::Save( CINIConfig *pIniConfig, string tGroup )
{
	pIniConfig->SetValue( "XAxis", this->mXArg.ArgName(), tGroup );
	pIniConfig->SetValue( "YAxis", this->mYArg.ArgName(), tGroup );

	return HSTrue;
}

HSBool CGraphicScatter::Load( CINIConfig *pIniConfig, string tGroup )
{
	HSString tXArgName = "";
	HSString tYArgName = "";
	pIniConfig->ValueWithKey( "XAxis", tXArgName, tGroup );
	pIniConfig->ValueWithKey( "YAxis", tYArgName, tGroup );

	mXArg.SetArg( tXArgName );
	mYArg.SetArg( tYArgName );

	return HSTrue;
}

HSVoid CGraphicScatter::FocusGraphic( HSBool tIsFocused )
{
	if ( tIsFocused && !mStaticScatter.Focused() )
	{
		mStaticScatter.SetFocused( true );
		mStaticScatter.ResizeWave();
		mStaticScatter.Invalidate();		
	}
	else if ( !tIsFocused && mStaticScatter.Focused() )
	{
		mStaticScatter.SetFocused( false );
		mStaticScatter.ResizeWave();
		mStaticScatter.Invalidate();		
	}	
}

	
IGraphicPropertyProtocol * CGraphicScatter::CreatePropertyDlg( CWnd *pParent )
{
	CScatterProperty *pScatterProperty = new CScatterProperty;
	pScatterProperty->Create( IDD_SCATTERPROPERTY, pParent );
	return pScatterProperty;
}

HSVoid CGraphicScatter::SetRelationTitle()
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

	CString tStrRelation;
	tStrRelation.Format( "%s%s 对 %s%s", mYArg.ArgName().c_str(), mYArg.ArgUnit().c_str(), mXArg.ArgName().c_str(), mXArg.ArgUnit().c_str() );

	mStaticScatter.SetTitle( tTitle + tStrRelation );
}

HSVoid CGraphicScatter::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mRelationMutex, INFINITE );

	CleanChannelArgs();

	this->mDataIdentifier = tIdentifier;

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	DEVICE_CH_IDENTIFIER tCardIdentifer( tIdentifier.CARD_IDENTIFIER, tIdentifier.CARD_INDEX );
	mDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( tCardIdentifer );

	HSDouble tDuration = 0;
	HSDouble tMinValue = 0;
	mXArg.GetDesiredShowParam( tDuration, tMinValue );
	mStaticScatter.SetXValue( tDuration, 0 );		
	mCurrentXValue = tDuration;

	mYArg.GetDesiredShowParam( tDuration, tMinValue );
	mStaticScatter.SetYValue( tDuration, tDuration );
	mCurrentYValue = tDuration;

	SetRelationTitle();
	mStaticScatter.Invalidate();

	vector< HSUInt > tChannels;
	if ( mDataHandler && tIdentifier.NumOfChannels( &tChannels ) > 0 )
	{				
		for ( HSUInt i = 0; i < tChannels.size(); i++ )
		{
			tCardIdentifer.InitChannel( tChannels[ i ] );
			ChannelArgInfo tChannelArgInfo;			
			tChannelArgInfo.ArgTransfer = new CArgListTransfer( mDataHandler, tCardIdentifer );
			tChannelArgInfo.Index = 0;
			tChannelArgInfo.Channel = tChannels[ i ];

			mChannelArgs.push_back( tChannelArgInfo );
		}		

		if ( pMainFrame->IsStart() )
		{
			this->Start();
		}
	}
	

	SetEvent( mRelationMutex );	
}

HSVoid CGraphicScatter::SetXArg( HSString tArgName )
{
	mXArg.SetArg( tArgName );
	CleanChannelArgs();

	SetRelationTitle();

	HSDouble tDuration = 0;
	HSDouble tMinValue = 0;
	mXArg.GetDesiredShowParam( tDuration, tMinValue );

	mStaticScatter.SetXValue( tDuration, 0 );	
	mStaticScatter.SetMinXValue( tMinValue );

	mCurrentXValue = tDuration;

	mStaticScatter.SetIsValueAdded( mXArg.IsValueAdded() );	

	mStaticScatter.Invalidate();
}

HSVoid CGraphicScatter::SetYArg( HSString tArgName )
{
	mYArg.SetArg( tArgName );
	CleanChannelArgs();

	SetRelationTitle();

	HSDouble tDuration = 0;
	HSDouble tMinValue = 0;
	mYArg.GetDesiredShowParam( tDuration, tMinValue );

	mCurrentYValue = tDuration;

	mStaticScatter.SetYValue( tDuration, tDuration );	
	mStaticScatter.SetMinYValue( tMinValue );
	mStaticScatter.Invalidate();

	mCurrentYValue = tDuration;
}

HSBool CGraphicScatter::ThreadRuning( HSInt tThreadID )
{
	for ( HSUInt i = 0; i < mChannelArgs.size(); i++ )	
	{
		mChannelArgs[ i ].ArgTransfer->GetArg( mChannelArgs[ i ].ArgList );
		for ( HSUInt j = mChannelArgs[ i ].Index; j < mChannelArgs[ i ].ArgList.size(); j++ )  
		{
			CStaticScatter::ArgValueInfo tArgValue;

			tArgValue.XValue = mXArg.FetchArg( mChannelArgs[ i ].ArgList[ j ], j, mChannelArgs[ i ].Channel );
			tArgValue.YValue = mYArg.FetchArg( mChannelArgs[ i ].ArgList[ j ], j, mChannelArgs[ i ].Channel );			
			mArgsValue.push_back( tArgValue );
		}

		mChannelArgs[ i ].Index = mChannelArgs[ i ].ArgList.size();		
	}	
	
	HSDouble tValue = 0;
	list< CStaticScatter::ArgValueInfo >::iterator pIterator = mArgsValue.begin();
	while ( pIterator != mArgsValue.end() )
	{
		tValue = ( mXArg.IsValueAdded() ? tValue + pIterator->YValue : pIterator->YValue );
		if ( tValue > mStaticScatter.TotalYValue() )
		{
			HSDouble tYValue = HSInt( tValue / mCurrentYValue ) * 2 * mCurrentYValue;			
			mStaticScatter.SetYValue( tYValue, tYValue );		

			mCurrentYValue = mStaticScatter.TotalYValue();
		}

		HSDouble tCurXValue = pIterator->XValue;
		if ( tCurXValue > mStaticScatter.TotalXValue() )
		{
			HSDouble tXValue = HSInt( tCurXValue / mCurrentXValue ) * 2 * mCurrentXValue;			
			mStaticScatter.SetXValue( tXValue, 0 );		

			mCurrentXValue = mStaticScatter.TotalXValue();
		}
		
		pIterator++;
	}

	mStaticScatter.Invalidate();

	return HSTrue;
}

HSVoid CGraphicScatter::ThreadWillStop( HSInt tThreadID )
{
}

HSVoid CGraphicScatter::CleanChannelArgs()
{
	for ( HSUInt i = 0; i < mChannelArgs.size(); i++ )
	{
		delete mChannelArgs[ i ].ArgTransfer;
	}

	mChannelArgs.clear();
	mArgsValue.clear();
}


void CGraphicScatter::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_SCATTER, mStaticScatter);
}


BEGIN_MESSAGE_MAP(CGraphicScatter, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_INITMENUPOPUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SCATTERPOPUP_CLOSE, &CGraphicScatter::OnScatterpopupClose)
	ON_COMMAND(ID_SCATTERPOPUP_FULL_SCREEN, &CGraphicScatter::OnScatterpopupFullScreen)
	ON_UPDATE_COMMAND_UI(ID_SCATTERPOPUP_FULL_SCREEN, &CGraphicScatter::OnUpdateScatterpopupFullScreen)
	ON_COMMAND(ID_SCATTERPOPUP_SETDATASOURCE, &CGraphicScatter::OnScatterpopupSetdatasource)
	ON_COMMAND(ID_SCATTERPOPUP_RESET, &CGraphicScatter::OnScatterpopupReset)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CGraphicScatter message handlers


BOOL CGraphicScatter::OnInitDialog()
{
	__super::OnInitDialog();

	// TODO:  Add extra initialization here

	mInit = HSTrue;

	mPopupMenu.LoadMenu( IDR_POPUP_SCATTER_C );

	HSDouble tDuration = 0;
	HSDouble tMinYValue = 0;
	mYArg.GetDesiredShowParam( mCurrentYValue, tMinYValue );	
	mStaticScatter.SetMinYValue( tMinYValue );
	mStaticScatter.SetYValue( mCurrentYValue, mCurrentYValue );

	HSDouble tMinXValue = 0;
	
	mXArg.GetDesiredShowParam( tDuration, tMinXValue );	
	mStaticScatter.SetXValue( tDuration, 0 );
	mStaticScatter.SetMinXValue( tMinXValue );

	mStaticScatter.SetArgsValue( &mArgsValue );

	//mStaticRelation.SetPeriodValue( &mPeriodValues, mUnitDelta );
	//mStaticRelation.SetFillRectangle( mFillRectangle );	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CGraphicScatter::OnDestroy()
{
	__super::OnDestroy();

	// TODO: Add your message handler code here

	mUpdateRelationThread->Stop();
}


void CGraphicScatter::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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


void CGraphicScatter::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	

	CPoint tPoint = point;
	
	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticScatter.LeftDelta() && tPoint.x < tRect.right - mStaticScatter.LeftDelta() && tPoint.y > tRect.top + mStaticScatter.TopDelta() && tPoint.y < tRect.bottom - mStaticScatter.TopDelta() )
	{		
		if ( pMainFrame->GetGrahpicCheckType() != CMainFrame::GRAPHIC_CHECK_POINTER )
		{			
			if ( mUpdateRelationThread->IsStart() )
			{
				this->Stop();
			}
		}		

		pMainFrame->SetFocusedGraphic( this );
		if ( !mStaticScatter.Focused() )
		{			
			mStaticScatter.SetFocused( true );
			mStaticScatter.ResizeWave();
			mStaticScatter.Invalidate();
		}

		if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_STEP_FRAME )
		{
			int tDelta = 1;
			if ( tPoint.x < tRect.right / 2 )
			{
				tDelta = -1;
			}

			WaitForSingleObject( mRelationMutex, INFINITE );		

			mStaticScatter.SetXValue( mStaticScatter.TotalXValue(), mStaticScatter.BeginXValue() + tDelta * mStaticScatter.TotalXValue() );
			mStaticScatter.Invalidate();

			SetEvent( mRelationMutex );				
		}
		else if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_RECTANGE )
		{
			mIsDraging = HSFalse;
			mIsZoomIn = HSTrue;
			mZoomInBeginPoint = tPoint;
			mFirstTimeDrawZoomRect = HSTrue;
		}
		else if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_DRAG )
		{
			mIsDraging = HSTrue;
			mIsZoomIn = HSFalse;
			mPrevPoint = tPoint;
		}
	}	
	else
	{	
		mParent->BeginMoveSubView( this, nFlags, point);
	}

	__super::OnLButtonDown(nFlags, point);
}


void CGraphicScatter::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if ( mIsDraging )
	{

	}
	else if ( mIsZoomIn )
	{
		CDC *pDC = this->GetDC();
		CRect tRect( 0, 0, 0, 0 );
		CBrush tBrush;		
		tBrush.CreateSolidBrush( RGB( 200, 200, 200 ) );
		pDC->DrawDragRect( tRect, CSize( 1, 1 ), mZoomInRect, CSize( 1, 1 ) , &tBrush, &tBrush );

		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		tViewRect = CRect( tViewRect.left + mStaticScatter.LeftDelta(), tViewRect.top + mStaticScatter.TopDelta(), tViewRect.right - mStaticScatter.LeftDelta(), tViewRect.bottom - mStaticScatter.TopDelta() );
		
		WaitForSingleObject( mRelationMutex, INFINITE );			
		
		HSDouble tXValueOffset = max( 0.0, mZoomInRect.left - mStaticScatter.LeftDelta() ) / tViewRect.Width() * mStaticScatter.TotalXValue();
		HSDouble tNewXValue = ( HSDouble )( mZoomInRect.Width() ) / tViewRect.Width() * mStaticScatter.TotalXValue();			

		HSDouble tBeginYValue = mStaticScatter.BeginYValue() - max( 0.0, mZoomInRect.top - mStaticScatter.TopDelta() ) / tViewRect.Height() * mStaticScatter.TotalYValue();
		HSDouble tTotalYValue = ( HSDouble )mZoomInRect.Height() / tViewRect.Height() * mStaticScatter.TotalYValue();
		
		mStaticScatter.SetXValue( tNewXValue, mStaticScatter.BeginXValue() + tXValueOffset );
		mStaticScatter.SetYValue( tTotalYValue, tBeginYValue );		
		mStaticScatter.Invalidate();
		
		SetEvent( mRelationMutex );
	}
	else
	{
		mParent->EndMoveSubView( nFlags, point);
	}

	mIsDraging = HSFalse;
	mIsZoomIn = HSFalse;	

	__super::OnLButtonUp(nFlags, point);
}


void CGraphicScatter::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if ( mIsDraging )
	{
		WaitForSingleObject( mRelationMutex, INFINITE );		

		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		tViewRect = CRect( tViewRect.left + mStaticScatter.LeftDelta(), tViewRect.top + mStaticScatter.TopDelta(), tViewRect.right - mStaticScatter.LeftDelta(), tViewRect.bottom - mStaticScatter.TopDelta() );

		HSInt tYOffset = mPrevPoint.y - point.y;			
		HSDouble tYValuePerDigit = mStaticScatter.TotalYValue() / tViewRect.Height();
		HSDouble tBeginYValue = mStaticScatter.BeginYValue() - tYOffset * tYValuePerDigit;			
		mStaticScatter.SetYValue( mStaticScatter.TotalYValue(), tBeginYValue );		

		HSInt tXOffset = mPrevPoint.x - point.x;
		HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticScatter.TotalXValue();		
		mStaticScatter.SetXValue( mStaticScatter.TotalXValue(), mStaticScatter.BeginXValue() + tXOffsetValue );
		mStaticScatter.Invalidate();
		
		mPrevPoint = point;
		
		SetEvent( mRelationMutex );			
	}
	else if ( mIsZoomIn )
	{
		CRect tRect = CRect( mZoomInBeginPoint, point );		
		tRect.NormalizeRect();

		CDC *pDC = this->GetDC();		
		CBrush tBrush;		
		tBrush.CreateSolidBrush( RGB( 200, 200, 200 ) );
		if ( mFirstTimeDrawZoomRect )
		{
			pDC->DrawDragRect( tRect, CSize( 1, 1 ), NULL, CSize( 1, 1 ) , &tBrush, &tBrush );
			mFirstTimeDrawZoomRect = HSFalse;
		}
		else
		{
			pDC->DrawDragRect( tRect, CSize( 1, 1 ), mZoomInRect, CSize( 1, 1 ) , &tBrush, &tBrush );
		}

		mZoomInRect = tRect;
	}
	else
	{
		mParent->MoveingSubView( nFlags, point);
	}		

	__super::OnMouseMove(nFlags, point);
}


void CGraphicScatter::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( mInit )
	{
		CRect tRect;
		this->GetClientRect( &tRect );
		if ( tRect.Width() > mStaticScatter.LeftDelta() * 3 && tRect.Height() > mStaticScatter.TopDelta() * 3 )
		{
			mStaticScatter.MoveWindow( tRect, true );
			mStaticScatter.SetXValue( mStaticScatter.TotalXValue(), mStaticScatter.BeginXValue() );
			mStaticScatter.SetYValue( mStaticScatter.TotalYValue(), mStaticScatter.BeginYValue() );
			mStaticScatter.ResizeWave();
			mStaticScatter.Invalidate();
		}
	}
}


void CGraphicScatter::OnContextMenu(CWnd *pWnd, CPoint point )
{
	// TODO: Add your message handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->SetGrahpicCheckType( CMainFrame::GRAPHIC_CHECK_POINTER );
	
	mPopupMenu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this );
}


void CGraphicScatter::OnScatterpopupClose()
{
	// TODO: Add your command handler code here

	CDialogEx::OnOK();

	mParent->SubViewClosed( this );
}


void CGraphicScatter::OnScatterpopupFullScreen()
{
	// TODO: Add your command handler code here

	mIsFullScreen = !mIsFullScreen;
	mParent->SubViewFullScreen( this, mIsFullScreen );
}


void CGraphicScatter::OnUpdateScatterpopupFullScreen(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->SetCheck( mIsFullScreen );
}


void CGraphicScatter::OnScatterpopupSetdatasource()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->OnViewGraphic();	
	pMainFrame->SetFocusedGraphic( this );

	if ( !mStaticScatter.Focused() || pMainFrame->FocusedGraphic() != this )
	{		
		mStaticScatter.SetFocused( true );
		mStaticScatter.ResizeWave();
		mStaticScatter.Invalidate();
	}
}


void CGraphicScatter::OnScatterpopupReset()
{
	// TODO: Add your command handler code here

	WaitForSingleObject( mRelationMutex, INFINITE );		

	mStaticScatter.SetXValue( mCurrentXValue, 0 );
	mStaticScatter.SetYValue( mCurrentYValue, mCurrentYValue );		

	mStaticScatter.Invalidate();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( pMainFrame->IsStart() )
	{
		Start();
	}

	SetEvent( mRelationMutex );
}


BOOL CGraphicScatter::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );		

	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticScatter.LeftDelta() && tPoint.x < tRect.right - mStaticScatter.LeftDelta() && tPoint.y > tRect.top + mStaticScatter.TopDelta() && tPoint.y < tRect.bottom - mStaticScatter.TopDelta() )
	{		
		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
		if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_STEP_FRAME )
		{
			if ( tPoint.x < tRect.right / 2 )
			{
				SetCursor( LoadCursor( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDC_CURSOR_PREV_FRAME ) ) );
			}
			else
			{
				SetCursor( LoadCursor( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDC_CURSOR_NEXT_FRAME ) ) );
			}

			return true;
		}
		else if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_RECTANGE )
		{
			SetCursor( AfxGetApp()->LoadStandardCursor( IDC_CROSS ) );
			return true;
		}
		else if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_DRAG )
		{
			SetCursor( AfxGetApp()->LoadStandardCursor( IDC_HAND ) );
			return true;
		}
	}	

	return __super::OnSetCursor(pWnd, nHitTest, message);
}
