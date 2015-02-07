// NormalGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "NormalGraphic.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "resource.h"
#include "HSLogProtocol.h"
#include "INIConfig.h"
#include "DataExportHelper.h"
#include "ArgCalculator.h"


// CNormalGraphic dialog

IMPLEMENT_DYNAMIC(CNormalGraphic, CDialogEx)

CNormalGraphic::CNormalGraphic(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNormalGraphic::IDD, pParent)
{
	mIsDraging = HSFalse;	
	mIsFinishInit = HSFalse;

	mIsZoomIn = HSFalse;	

	mDataIdentifier = 0;		
	mDataHandler = NULL;

	mDataMutex = CreateEvent( NULL, FALSE, TRUE, NULL );	
	mRefreshThread = NULL;

	mIsFullScreen = HSFalse;	

	mZoomBrush.CreateSolidBrush( RGB( 200, 200, 200 ) );

	mStopWhenMouseDown = HSTrue;

	mRefreshMiniSecond = 800;
}

CNormalGraphic::~CNormalGraphic()
{	
	if ( mRefreshThread != NULL )
	{
		delete mRefreshThread;
	}
	
	CloseHandle( mDataMutex );	
}

HSVoid CNormalGraphic::Pause()
{
	mRefreshThread->Pause();
}

HSVoid CNormalGraphic::Stop()
{
	mRefreshThread->Stop();
}

HSVoid CNormalGraphic::FocusGraphic( HSBool tIsFocused )
{
	if ( tIsFocused && !mStaticGraphic->Focused() )
	{
		mStaticGraphic->SetFocused( true );
		mStaticGraphic->ResizeWave();
		mStaticGraphic->Invalidate();

		Graphic3DRefresh( HSTrue );
	}
	else if ( !tIsFocused && mStaticGraphic->Focused() )
	{
		mStaticGraphic->SetFocused( false );
		mStaticGraphic->ResizeWave();
		mStaticGraphic->Invalidate();

		Graphic3DRefresh( HSTrue );
	}
}

HSBool CNormalGraphic::ThreadRuning( HSInt tThreadID )
{		
	return GraphicNeedRefresh();
}

HSVoid CNormalGraphic::ThreadWillStop( HSInt tThreadID )
{
	GraphicStopRefresh();
}

void CNormalGraphic::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNormalGraphic, CDialogEx)
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	ON_WM_INITMENUPOPUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_NORMAL_POPUP_CLOSE, &CNormalGraphic::OnNormalPopupClose)
	ON_COMMAND(ID_NORMAL_POPUP_EXPORTDATA, &CNormalGraphic::OnNormalPopupExportdata)
	ON_COMMAND(ID_NORMAL_POPUP_FULL_SCREEN, &CNormalGraphic::OnNormalPopupFullScreen)
	ON_UPDATE_COMMAND_UI(ID_NORMAL_POPUP_FULL_SCREEN, &CNormalGraphic::OnUpdateNormalPopupFullScreen)
	ON_COMMAND(ID_NORMAL_POPUP_RESET, &CNormalGraphic::OnNormalPopupReset)
	ON_COMMAND(ID_NORMAL_POPUP_RESET_GRAPHIC, &CNormalGraphic::OnNormalPopupResetGraphic)
	ON_COMMAND(ID_NORMAL_POPUP_SETDATASOURCE, &CNormalGraphic::OnNormalPopupSetdatasource)
	ON_COMMAND(ID_POSITION_POPUP_AUTO, &CNormalGraphic::OnPositionPopupAuto)
	ON_COMMAND(ID_POSITION_POPUP_CHECK_CIRCLE, &CNormalGraphic::OnPositionPopupCheckCircle)
	ON_UPDATE_COMMAND_UI(ID_POSITION_POPUP_CHECK_CIRCLE, &CNormalGraphic::OnUpdatePositionPopupCheckCircle)
	ON_COMMAND(ID_POSITION_POPUP_ROTATE, &CNormalGraphic::OnPositionPopupRotate)
	ON_COMMAND(ID_POSITION_POPUP_CHECK_ANGLE, &CNormalGraphic::OnPositionPopupCheckAngle)
	ON_UPDATE_COMMAND_UI(ID_POSITION_POPUP_CHECK_ANGLE, &CNormalGraphic::OnUpdatePositionPopupCheckAngle)
	ON_COMMAND(ID_POSITION_POPUP_OUTLINE, &CNormalGraphic::OnPositionPopupOutline)
	ON_UPDATE_COMMAND_UI(ID_POSITION_POPUP_OUTLINE, &CNormalGraphic::OnUpdatePositionPopupOutline)
	ON_COMMAND(ID_POSITION_POPUP_CAL_COORD, &CNormalGraphic::OnPositionPopupCalCoord)
END_MESSAGE_MAP()


// CNormalGraphic message handlers


BOOL CNormalGraphic::OnInitDialog()
{
	__super::OnInitDialog();

	// TODO:  Add extra initialization here	

	mRefreshThread = new CThreadControlEx< CNormalGraphic >( this, mRefreshMiniSecond, 0 );

	InitDialog();

	mIsFinishInit = HSTrue;	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CNormalGraphic::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: Add your message handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->SetGrahpicCheckType( CMainFrame::GRAPHIC_CHECK_POINTER );
	
	mPopupMenu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this );
}


void CNormalGraphic::OnDestroy()
{
	__super::OnDestroy();

	// TODO: Add your message handler code here
	mRefreshThread->Stop();
}


void CNormalGraphic::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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


void CNormalGraphic::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	CPoint tPoint = point;
	
	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticGraphic->LeftDelta() && tPoint.x < tRect.right - mStaticGraphic->LeftDelta() && tPoint.y > tRect.top + mStaticGraphic->TopDelta() && tPoint.y < tRect.bottom - mStaticGraphic->TopDelta() )
	{		
		if ( pMainFrame->GetGrahpicCheckType() != CMainFrame::GRAPHIC_CHECK_POINTER )
		{			
			if ( mStopWhenMouseDown && mRefreshThread->IsStart() )
			{
				this->Stop();
			}
		}		
		else
		{
			MouseDownWhenPointer( tRect, point );
		}
			
		if ( !mStaticGraphic->Focused() )
		{
			pMainFrame->SetFocusedGraphic( this );
			mStaticGraphic->SetFocused( true );
			mStaticGraphic->ResizeWave();
			mStaticGraphic->Invalidate();
		}

		if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_STEP_FRAME )
		{
			int tDelta = 1;
			if ( tPoint.x < tRect.right / 2 )
			{
				tDelta = -1;
			}

			StepGraphic( tDelta );
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


void CNormalGraphic::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if ( CustomMouseUpCheck() )
	{
	}
	else if ( mIsDraging )
	{
	}
	else if ( mIsZoomIn )
	{
		CDC *pDC = this->GetDC();
		CRect tRect( 0, 0, 0, 0 );
		
		pDC->DrawDragRect( tRect, CSize( 1, 1 ), mZoomInRect, CSize( 1, 1 ) , &mZoomBrush, &mZoomBrush );

		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		tViewRect = CRect( tViewRect.left + mStaticGraphic->LeftDelta(), tViewRect.top + mStaticGraphic->TopDelta(), tViewRect.right - mStaticGraphic->LeftDelta(), tViewRect.bottom - mStaticGraphic->TopDelta() );
		
		ZoomInGraphic( tViewRect );
	}
	else
	{
		mParent->EndMoveSubView( nFlags, point);
	}

	mIsDraging = HSFalse;
	mIsZoomIn = HSFalse;

	MouseUpFinish();

	__super::OnLButtonUp(nFlags, point);
}


void CNormalGraphic::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if ( CustomMouseMoveCheck( point ) )
	{
	}
	else if ( mIsDraging )
	{
		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		tViewRect = CRect( tViewRect.left + mStaticGraphic->LeftDelta(), tViewRect.top + mStaticGraphic->TopDelta(), tViewRect.right - mStaticGraphic->LeftDelta(), tViewRect.bottom - mStaticGraphic->TopDelta() );

		DragGraphic( tViewRect, point );

		mPrevPoint = point;		
	}
	else if ( mIsZoomIn )
	{
		CRect tRect = CRect( mZoomInBeginPoint, point );		
		tRect.NormalizeRect();

		CDC *pDC = this->GetDC();		
		
		if ( mFirstTimeDrawZoomRect )
		{
			pDC->DrawDragRect( tRect, CSize( 1, 1 ), NULL, CSize( 1, 1 ), &mZoomBrush, &mZoomBrush );
			mFirstTimeDrawZoomRect = HSFalse;
		}
		else
		{
			pDC->DrawDragRect( tRect, CSize( 1, 1 ), mZoomInRect, CSize( 1, 1 ), &mZoomBrush, &mZoomBrush );
		}

		mZoomInRect = tRect;
	}
	else
	{
		MouseMoveOtherWork( point );

		mParent->MoveingSubView( nFlags, point);
	}

	__super::OnMouseMove(nFlags, point);
}

HSVoid CNormalGraphic::ViewResized( CRect &tRect )
{
	mStaticGraphic->MoveWindow( tRect, true );	
	mStaticGraphic->SetXValue( mStaticGraphic->TotalXValue(), mStaticGraphic->BeginXValue() );
	mStaticGraphic->SetYValue( mStaticGraphic->TotalYValue(), mStaticGraphic->BeginYValue() );
	mStaticGraphic->ResizeWave();
	mStaticGraphic->Invalidate();
}


void CNormalGraphic::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( mIsFinishInit )
	{
		CRect tRect;
		this->GetClientRect( &tRect );
		if ( tRect.Width() > mStaticGraphic->LeftDelta() * 3 && tRect.Height() > mStaticGraphic->TopDelta() * 3 )
		{
			ViewResized( tRect );
		}
	}	
}


BOOL CNormalGraphic::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );		

	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticGraphic->LeftDelta() && tPoint.x < tRect.right - mStaticGraphic->LeftDelta() && tPoint.y > tRect.top + mStaticGraphic->TopDelta() && tPoint.y < tRect.bottom - mStaticGraphic->TopDelta() )
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


void CNormalGraphic::OnNormalPopupClose()
{
	// TODO: Add your command handler code here

	CDialogEx::OnOK();

	mParent->SubViewClosed( this );
}


void CNormalGraphic::OnNormalPopupExportdata()
{
	// TODO: Add your command handler code here
	
	if ( !IsDataExportNeeded() )
	{
		MessageBox( "没有数据!", "警告" );
		return;
	}

	HSBool tIsStart = mRefreshThread->IsStart();
	if ( tIsStart )
	{
		this->Stop();
	}			

	HSChar tBuf[ 1024 ];

	CDataExportHelper tDataExport( this );
	if ( tDataExport.GetFilePath() )
	{
		sprintf_s( tBuf, "%s\n", this->Name().c_str() );
		tDataExport.Write( tBuf );

		sprintf_s( tBuf, "日期: %s %s\n", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
		tDataExport.Write( tBuf );

		ExportData( tDataExport );

		tDataExport.Finish();
	}

	if ( tIsStart )
	{
		this->Start();
	}
}


void CNormalGraphic::OnNormalPopupFullScreen()
{
	// TODO: Add your command handler code here

	mIsFullScreen = !mIsFullScreen;
	mParent->SubViewFullScreen( this, mIsFullScreen );

	Graphic3DRefresh( HSFalse );
}


void CNormalGraphic::OnUpdateNormalPopupFullScreen(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->SetCheck( mIsFullScreen );	

	Graphic3DRefresh( HSFalse );
}


void CNormalGraphic::OnNormalPopupReset()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	ResetData( pMainFrame );
}


void CNormalGraphic::OnNormalPopupResetGraphic()
{
	// TODO: Add your command handler code here

	ResetGrahpic();
}


void CNormalGraphic::OnNormalPopupSetdatasource()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->OnViewGraphic();	
	pMainFrame->SetFocusedGraphic( this );

	if ( !mStaticGraphic->Focused() || pMainFrame->FocusedGraphic() != this )
	{		
		mStaticGraphic->SetFocused( true );
		mStaticGraphic->ResizeWave();
		mStaticGraphic->Invalidate();

		Graphic3DRefresh( HSTrue );
	}
}


void CNormalGraphic::OnPositionPopupAuto()
{
	// TODO: Add your command handler code here
	AutoSetSensors();
}


void CNormalGraphic::OnPositionPopupCheckCircle()
{
	// TODO: Add your command handler code here
	SwitchCheckCircle();
}


void CNormalGraphic::OnUpdatePositionPopupCheckCircle(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->SetCheck( SwitchCheckCircleEnabled() );	
}


void CNormalGraphic::OnPositionPopupRotate()
{
	// TODO: Add your command handler code here

	AutoRatate();
}


void CNormalGraphic::OnPositionPopupCheckAngle()
{
	// TODO: Add your command handler code here
	SwitchCheckAngle();
}


void CNormalGraphic::OnUpdatePositionPopupCheckAngle(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( SwitchCheckAngleEnabled() );	
}


void CNormalGraphic::OnPositionPopupOutline()
{
	// TODO: Add your command handler code here
	SwitchOutLine();
}


void CNormalGraphic::OnUpdatePositionPopupOutline(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( SwitchOutLineEnabled() );	
}


void CNormalGraphic::OnPositionPopupCalCoord()
{
	// TODO: Add your command handler code here
	AutoSetSensors();
}
