// GraphicRelation.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GraphicRelation.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "RelationProperty.h"
#include "HSLogProtocol.h"

// CGraphicRelation dialog

IMPLEMENT_DYNAMIC(CGraphicRelation, CDialogEx)

CGraphicRelation::CGraphicRelation(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGraphicRelation::IDD, pParent), mXArg( 0, HSTrue ), mYArg( 2, HSFalse )
{
	mInit = HSFalse;
	mRelationMutex = CreateEvent( NULL, FALSE, TRUE, NULL );
	mDataHandler = NULL;

	mUpdateRelationThread = new CThreadControlEx< CGraphicRelation >( this, 800, 0 );

	mIsFullScreen = HSFalse;
	mUnitDelta = 1;

	mFillRectangle = HSTrue;
	mIsDraging = HSFalse;		
	mIsZoomIn = HSFalse;		
}

CGraphicRelation::~CGraphicRelation()
{
	CloseHandle( mRelationMutex );

	delete mUpdateRelationThread;

	CleanChannelArgs();
}

HSBool CGraphicRelation::Start()
{
	if ( mChannelArgs.size() > 0 )
	{		
		mUpdateRelationThread->Start();
		return HSTrue;
	}
	return HSFalse;
}

HSVoid CGraphicRelation::Pause()
{
	mUpdateRelationThread->Pause();
}

HSVoid CGraphicRelation::Stop()
{
	mUpdateRelationThread->Stop();
	//HS_INFO( "GraphicRelation stoped!" );
}

HSBool CGraphicRelation::Save( CINIConfig *pIniConfig, string tGroup )
{
	pIniConfig->SetValue( "XAxis", this->mXArg.ArgName(), tGroup );
	pIniConfig->SetValue( "YAxis", this->mYArg.ArgName(), tGroup );
	pIniConfig->SetValue( "FillRect", mFillRectangle, tGroup );	

	return HSTrue;
}

HSBool CGraphicRelation::Load( CINIConfig *pIniConfig, string tGroup )
{
	HSString tXArgName = "";
	HSString tYArgName = "";
	pIniConfig->ValueWithKey( "XAxis", tXArgName, tGroup );
	pIniConfig->ValueWithKey( "YAxis", tYArgName, tGroup );
	pIniConfig->ValueWithKey( "FillRect", mFillRectangle, tGroup );

	mXArg.SetArg( tXArgName );
	mYArg.SetArg( tYArgName );

	return HSTrue;
}	


HSVoid CGraphicRelation::FocusGraphic( HSBool tIsFocused )
{
	if ( tIsFocused && !mStaticRelation.Focused() )
	{
		mStaticRelation.SetFocused( true );
		mStaticRelation.ResizeWave();
		mStaticRelation.Invalidate();		
	}
	else if ( !tIsFocused && mStaticRelation.Focused() )
	{
		mStaticRelation.SetFocused( false );
		mStaticRelation.ResizeWave();
		mStaticRelation.Invalidate();		
	}	
}

	
IGraphicPropertyProtocol * CGraphicRelation::CreatePropertyDlg( CWnd *pParent )
{
	CRelationProperty *pRelationProperty = new CRelationProperty;
	pRelationProperty->Create( IDD_RELATIONPROPERTY, pParent );
	return pRelationProperty;
}

HSVoid CGraphicRelation::SetRelationTitle()
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

	mStaticRelation.SetTitle( tTitle + tStrRelation );
}

HSVoid CGraphicRelation::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
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
	mStaticRelation.SetXValue( tDuration, 0 );		
	mCurrentXValue = tDuration;

	mYArg.GetDesiredShowParam( tDuration, tMinValue );
	mStaticRelation.SetYValue( tDuration, tDuration );
	mCurrentYValue = tDuration;

	SetRelationTitle();
	mStaticRelation.Invalidate();

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

HSVoid CGraphicRelation::SetXArg( HSString tArgName )
{
	mXArg.SetArg( tArgName );
	CleanChannelArgs();

	SetRelationTitle();

	HSDouble tDuration = 0;
	HSDouble tMinValue = 0;
	mXArg.GetDesiredShowParam( tDuration, tMinValue );

	mStaticRelation.SetXValue( tDuration, 0 );	
	mStaticRelation.SetMinXValue( tMinValue );

	mCurrentXValue = tDuration;

	mStaticRelation.SetIsValueAdded( mXArg.IsValueAdded() );
	mStaticRelation.SetPeriodValue( &mPeriodValues, mUnitDelta );

	mStaticRelation.Invalidate();
}

HSVoid CGraphicRelation::SetYArg( HSString tArgName )
{
	mYArg.SetArg( tArgName );
	CleanChannelArgs();

	SetRelationTitle();

	HSDouble tDuration = 0;
	HSDouble tMinValue = 0;
	mYArg.GetDesiredShowParam( tDuration, tMinValue );

	mCurrentYValue = tDuration;

	mStaticRelation.SetYValue( tDuration, tDuration );	
	mStaticRelation.SetMinYValue( tMinValue );
	mStaticRelation.Invalidate();

	mCurrentYValue = tDuration;
}

HSVoid CGraphicRelation::SetIsSolidRect( HSBool tValue )
{
	mFillRectangle = tValue;
	mStaticRelation.SetFillRectangle( mFillRectangle );
	mStaticRelation.Invalidate();
}

HSVoid CGraphicRelation::SetUnitDelta( HSDouble tValue )
{
	mUnitDelta = tValue;
	mStaticRelation.SetPeriodValue( &mPeriodValues, mUnitDelta );
	CleanChannelArgs();
	mStaticRelation.Invalidate();
}

HSBool CGraphicRelation::ThreadRuning( HSInt tThreadID )
{
	for ( HSUInt i = 0; i < mChannelArgs.size(); i++ )	
	{
		mChannelArgs[ i ].ArgTransfer->GetArg( mChannelArgs[ i ].ArgList );
		for ( HSUInt j = mChannelArgs[ i ].Index; j < mChannelArgs[ i ].ArgList.size(); j++ )  
		{
			HSDouble tXValue = mXArg.FetchArg( mChannelArgs[ i ].ArgList[ j ], j, mChannelArgs[ i ].Channel );			
			HSInt tIntXValue = ( HSInt )( tXValue / mUnitDelta );
			if ( mPeriodValues.find( tIntXValue ) == mPeriodValues.end() )
			{
				mPeriodValues[ tIntXValue ] = 0;
			}

			mPeriodValues[ tIntXValue ] += mYArg.FetchArg( mChannelArgs[ i ].ArgList[ j ], j, mChannelArgs[ i ].Channel );
		}

		mChannelArgs[ i ].Index = mChannelArgs[ i ].ArgList.size();		
	}	

	HSDouble tValue = 0;
	map< HSInt, HSDouble >::iterator pIterator = mPeriodValues.begin();
	while ( pIterator != mPeriodValues.end() )
	{
		tValue = ( mXArg.IsValueAdded() ? tValue + pIterator->second : pIterator->second );
		if ( tValue > mStaticRelation.TotalYValue() )
		{
			HSDouble tYValue = HSInt( tValue / mCurrentYValue ) * 2 * mCurrentYValue;			
			mStaticRelation.SetYValue( tYValue, tYValue );		

			mCurrentYValue = mStaticRelation.TotalYValue();			
		}
		
		pIterator++;
	}

	pIterator = mPeriodValues.end();
	if ( pIterator != mPeriodValues.begin() )
	{
		pIterator--;

		if ( pIterator->first > mStaticRelation.TotalXValue() )
		{		
			HSDouble tXValue = HSInt( pIterator->first / mCurrentXValue ) * 2 * mCurrentXValue;			
			mStaticRelation.SetXValue( tXValue, 0 );		

			mCurrentXValue = mStaticRelation.TotalXValue();
		}		
	}

	mStaticRelation.Invalidate();

	return HSTrue;
}

HSVoid CGraphicRelation::ThreadWillStop( HSInt tThreadID )
{
}

HSVoid CGraphicRelation::CleanChannelArgs()
{
	for ( HSUInt i = 0; i < mChannelArgs.size(); i++ )
	{
		delete mChannelArgs[ i ].ArgTransfer;		
	}

	mChannelArgs.clear();
	mPeriodValues.clear();
}

void CGraphicRelation::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_RELATION, mStaticRelation);
}



BEGIN_MESSAGE_MAP(CGraphicRelation, CDialogEx)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_RELATIONPOPUP_CLOSE, &CGraphicRelation::OnRelationpopupClose)
	ON_COMMAND(ID_RELATIONPOPUP_FULL_SCREEN, &CGraphicRelation::OnRelationpopupFullScreen)
	ON_UPDATE_COMMAND_UI(ID_RELATIONPOPUP_FULL_SCREEN, &CGraphicRelation::OnUpdateRelationpopupFullScreen)
	ON_COMMAND(ID_RELATIONPOPUP_SETDATASOURCE, &CGraphicRelation::OnRelationpopupSetdatasource)
	ON_WM_CONTEXTMENU()
	ON_WM_INITMENUPOPUP()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_RELATIONPOPUP_RESET, &CGraphicRelation::OnRelationpopupReset)
END_MESSAGE_MAP()


// CGraphicRelation message handlers


BOOL CGraphicRelation::OnInitDialog()
{
	__super::OnInitDialog();

	// TODO:  Add extra initialization here

	mInit = HSTrue;

	mPopupMenu.LoadMenu( IDR_POPUP_RELATION_C );

	HSDouble tMinYValue = 0;
	mYArg.GetDesiredShowParam( mCurrentYValue, tMinYValue );	
	mStaticRelation.SetMinYValue( tMinYValue );
	mStaticRelation.SetYValue( mCurrentYValue, mCurrentYValue );

	HSDouble tMinXValue = 0;
	HSDouble tDuration = 0;
	mXArg.GetDesiredShowParam( tDuration, tMinXValue );	
	mStaticRelation.SetXValue( tDuration, 0 );
	mStaticRelation.SetMinXValue( tMinXValue );

	mStaticRelation.SetPeriodValue( &mPeriodValues, mUnitDelta );
	mStaticRelation.SetFillRectangle( mFillRectangle );	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CGraphicRelation::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( mInit )
	{
		CRect tRect;
		this->GetClientRect( &tRect );
		if ( tRect.Width() > mStaticRelation.LeftDelta() * 3 && tRect.Height() > mStaticRelation.TopDelta() * 3 )
		{
			mStaticRelation.MoveWindow( tRect, true );
			mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() );
			mStaticRelation.SetYValue( mStaticRelation.TotalYValue(), mStaticRelation.BeginYValue() );
			mStaticRelation.ResizeWave();
			mStaticRelation.Invalidate();
		}
	}
}


void CGraphicRelation::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	

	CPoint tPoint = point;
	
	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticRelation.LeftDelta() && tPoint.x < tRect.right - mStaticRelation.LeftDelta() && tPoint.y > tRect.top + mStaticRelation.TopDelta() && tPoint.y < tRect.bottom - mStaticRelation.TopDelta() )
	{		
		if ( pMainFrame->GetGrahpicCheckType() != CMainFrame::GRAPHIC_CHECK_POINTER )
		{			
			if ( mUpdateRelationThread->IsStart() )
			{
				this->Stop();
			}
		}		

		pMainFrame->SetFocusedGraphic( this );
		if ( !mStaticRelation.Focused() )
		{			
			mStaticRelation.SetFocused( true );
			mStaticRelation.ResizeWave();
			mStaticRelation.Invalidate();
		}

		if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_STEP_FRAME )
		{
			int tDelta = 1;
			if ( tPoint.x < tRect.right / 2 )
			{
				tDelta = -1;
			}

			WaitForSingleObject( mRelationMutex, INFINITE );		

			mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() + tDelta * mStaticRelation.TotalXValue() );
			mStaticRelation.Invalidate();

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


void CGraphicRelation::OnLButtonUp(UINT nFlags, CPoint point)
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
		tViewRect = CRect( tViewRect.left + mStaticRelation.LeftDelta(), tViewRect.top + mStaticRelation.TopDelta(), tViewRect.right - mStaticRelation.LeftDelta(), tViewRect.bottom - mStaticRelation.TopDelta() );
		
		WaitForSingleObject( mRelationMutex, INFINITE );			
		
		HSDouble tXValueOffset = max( 0.0, mZoomInRect.left - mStaticRelation.LeftDelta() ) / tViewRect.Width() * mStaticRelation.TotalXValue();
		HSDouble tNewXValue = ( HSDouble )( mZoomInRect.Width() ) / tViewRect.Width() * mStaticRelation.TotalXValue();			

		HSDouble tBeginYValue = mStaticRelation.BeginYValue() - max( 0.0, mZoomInRect.top - mStaticRelation.TopDelta() ) / tViewRect.Height() * mStaticRelation.TotalYValue();
		HSDouble tTotalYValue = ( HSDouble )mZoomInRect.Height() / tViewRect.Height() * mStaticRelation.TotalYValue();
		
		mStaticRelation.SetXValue( tNewXValue, mStaticRelation.BeginXValue() + tXValueOffset );
		mStaticRelation.SetYValue( tTotalYValue, tBeginYValue );		
		mStaticRelation.Invalidate();
		
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


void CGraphicRelation::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if ( mIsDraging )
	{
		WaitForSingleObject( mRelationMutex, INFINITE );		

		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		tViewRect = CRect( tViewRect.left + mStaticRelation.LeftDelta(), tViewRect.top + mStaticRelation.TopDelta(), tViewRect.right - mStaticRelation.LeftDelta(), tViewRect.bottom - mStaticRelation.TopDelta() );

		HSInt tYOffset = mPrevPoint.y - point.y;			
		HSDouble tYValuePerDigit = mStaticRelation.TotalYValue() / tViewRect.Height();
		HSDouble tBeginYValue = mStaticRelation.BeginYValue() - tYOffset * tYValuePerDigit;			
		mStaticRelation.SetYValue( mStaticRelation.TotalYValue(), tBeginYValue );		

		HSInt tXOffset = mPrevPoint.x - point.x;
		HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticRelation.TotalXValue();		
		mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() + tXOffsetValue );
		mStaticRelation.Invalidate();
		
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


void CGraphicRelation::OnRelationpopupClose()
{
	// TODO: Add your command handler code here

	CDialogEx::OnOK();

	mParent->SubViewClosed( this );
}


void CGraphicRelation::OnRelationpopupFullScreen()
{
	// TODO: Add your command handler code here

	mIsFullScreen = !mIsFullScreen;
	mParent->SubViewFullScreen( this, mIsFullScreen );
}


void CGraphicRelation::OnUpdateRelationpopupFullScreen(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->SetCheck( mIsFullScreen );	
}


void CGraphicRelation::OnRelationpopupSetdatasource()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->OnViewGraphic();	
	pMainFrame->SetFocusedGraphic( this );

	if ( !mStaticRelation.Focused() || pMainFrame->FocusedGraphic() != this )
	{		
		mStaticRelation.SetFocused( true );
		mStaticRelation.ResizeWave();
		mStaticRelation.Invalidate();
	}
}


void CGraphicRelation::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: Add your message handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->SetGrahpicCheckType( CMainFrame::GRAPHIC_CHECK_POINTER );
	
	mPopupMenu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this );
}


void CGraphicRelation::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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


void CGraphicRelation::OnDestroy()
{
	__super::OnDestroy();

	// TODO: Add your message handler code here
	mUpdateRelationThread->Stop();
}


BOOL CGraphicRelation::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );		

	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticRelation.LeftDelta() && tPoint.x < tRect.right - mStaticRelation.LeftDelta() && tPoint.y > tRect.top + mStaticRelation.TopDelta() && tPoint.y < tRect.bottom - mStaticRelation.TopDelta() )
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


void CGraphicRelation::OnRelationpopupReset()
{
	// TODO: Add your command handler code here
	
	WaitForSingleObject( mRelationMutex, INFINITE );		
			
	mStaticRelation.SetXValue( mCurrentXValue, 0 );	

	mStaticRelation.SetYValue( mCurrentYValue, mCurrentYValue );		

	mStaticRelation.Invalidate();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( pMainFrame->IsStart() )
	{
		Start();
	}

	SetEvent( mRelationMutex );		
}
