// GraphicWave.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GraphicWave.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "resource.h"
#include "HSLogProtocol.h"
#include "INIConfig.h"
// CGraphicWave dialog

IMPLEMENT_DYNAMIC(CGraphicWave, CDialogEx)

CGraphicWave::CGraphicWave(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGraphicWave::IDD, pParent)
{
	mIsDraging = HSFalse;	
	mIsFinishInit = HSFalse;

	mIsZoomIn = HSFalse;

	mBuffer = NULL;

	mDataTransfer = NULL;
	mDataHandler = NULL;
	mChannel = 0;

	mDataIdentifier = 0;

	mUpdateWaveThread = new CThreadControlEx< CGraphicWave >( this, 800, 0 );

	mDataHandlerMutex = CreateEvent( NULL, FALSE, TRUE, NULL );

	mRefreshSecond = 1.0;

	mTotalVoltage = 10.0;
	mTotalSecond = 1.0;

	mIsFullScreen = HSFalse;

	mBeginSecondOffset = 0;


}

CGraphicWave::~CGraphicWave()
{
	if ( mBuffer != NULL )
	{
		delete[] mBuffer;
	}

	if ( mDataTransfer != NULL )
	{
		delete mDataTransfer;
	}

	delete mUpdateWaveThread;

	CloseHandle( mDataHandlerMutex );
}

void CGraphicWave::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_WAVE, mStaticWave);
}


BEGIN_MESSAGE_MAP(CGraphicWave, CDialogEx)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SIZING()
	ON_WM_SIZE()
	ON_WM_SIZECLIPBOARD()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_WAVEPOPUP_EXPORTDATA, &CGraphicWave::OnWavepopupExportdata)
	ON_COMMAND(ID_WAVEPOPUP_CLOSE, &CGraphicWave::OnWavepopupClose)
	ON_COMMAND(ID_WAVEPOPUP_SETDATASOURCE, &CGraphicWave::OnWavepopupSetdatasource)
	ON_COMMAND(ID_WAVEPOPUP_ZOOMIN, &CGraphicWave::OnWavepopupZoomin)
	ON_COMMAND(ID_WAVEPOPUP_ZOOMOUT, &CGraphicWave::OnWavepopupZoomout)
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_WAVEPOPUP_RESET, &CGraphicWave::OnWavepopupReset)	
	ON_COMMAND(ID_WAVEPOPUP_FULL_SCREEN, &CGraphicWave::OnWavepopupFullScreen)
	ON_UPDATE_COMMAND_UI(ID_WAVEPOPUP_FULL_SCREEN, &CGraphicWave::OnUpdateWavepopupFullScreen)
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()


// CGraphicWave message handlers
IGraphicProtocol * CGraphicWave::CreateProto()
{
	return new CGraphicWave;
}

HSString CGraphicWave::Identifier()
{
	return "GRAPHIC_WAVE";
}

HSString CGraphicWave::Name()
{
	return "波形图";
}

IGraphicPropertyProtocol * CGraphicWave::CreatePropertyDlg( CWnd *pParent )
{
	CWaveProperty *pWaveProperty = new CWaveProperty;
	pWaveProperty->Create( IDD_WAVEPROPERTY, pParent );
	return pWaveProperty;
}

HSBool CGraphicWave::Start()
{
	if ( mDataTransfer != NULL && mDataHandler != NULL )
	{		
		mUpdateWaveThread->Start();
		return HSTrue;
	}

	return HSFalse;	
}

HSVoid CGraphicWave::Pause()
{
	mUpdateWaveThread->Pause();
}

HSVoid CGraphicWave::Stop()
{
	mUpdateWaveThread->Stop();
	//HS_INFO( "GraphicWave stoped!" );
}

CDialog * CGraphicWave::Wnd()
{
	return this;
}

HSInt CGraphicWave::LargeIcon()
{
	return IDB_BITMAP_WAVE;
}

HSInt CGraphicWave::SmallIcon()
{
	return IDB_BITMAP_WAVE_LIST;
}

IGraphicProtocol * CGraphicWave::Clone()
{
	return new CGraphicWave;
}

void CGraphicWave::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if ( mIsDraging )
	{
		WaitForSingleObject( mDataHandlerMutex, INFINITE );		

		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		tViewRect = CRect( tViewRect.left + mStaticWave.LeftDelta(), tViewRect.top + mStaticWave.TopDelta(), tViewRect.right - mStaticWave.LeftDelta(), tViewRect.bottom - mStaticWave.TopDelta() );

		HSInt tYOffset = mPrevPoint.y - point.y;			
		HSDouble tVoltagePerDigit = mStaticWave.TotalVoltage() / tViewRect.Height();
		HSDouble tBeginVoltage = mStaticWave.BeginVoltage() - tYOffset * tVoltagePerDigit;			
		mStaticWave.SetVoltage( mStaticWave.TotalVoltage(), tBeginVoltage );		

		HSInt tXOffset = mPrevPoint.x - point.x;
		HSDouble tSeekSecond = ( HSDouble )tXOffset / tViewRect.Width() * mRefreshSecond;		
		mStaticWave.SetSeconds( mRefreshSecond, mStaticWave.BeginSecond() + tSeekSecond );

		RefreshWave();

		mPrevPoint = point;
		
		SetEvent( mDataHandlerMutex );	
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
	
	CDialogEx::OnMouseMove(nFlags, point);
}


void CGraphicWave::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	

	CPoint tPoint = point;
	
	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticWave.LeftDelta() && tPoint.x < tRect.right - mStaticWave.LeftDelta() && tPoint.y > tRect.top + mStaticWave.TopDelta() && tPoint.y < tRect.bottom - mStaticWave.TopDelta() )
	{		
		if ( pMainFrame->GetGrahpicCheckType() != CMainFrame::GRAPHIC_CHECK_POINTER )
		{			
			if ( mUpdateWaveThread->IsStart() )
			{
				this->Stop();
			}
		}
		else if ( pMainFrame->IsStart() && !mUpdateWaveThread->IsStart() )
		{			
			mStaticWave.SetSeconds( mRefreshSecond, 0 );
			this->Start();					
		}	
		

		if ( !mStaticWave.Focused() )
		{
			pMainFrame->SetFocusedGraphic( this );
			mStaticWave.SetFocused( true );
			mStaticWave.ResizeWave();
			mStaticWave.Invalidate();
		}

		if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_STEP_FRAME )
		{
			int tDelta = 1;
			if ( tPoint.x < tRect.right / 2 )
			{
				tDelta = -1;
			}

			WaitForSingleObject( mDataHandlerMutex, INFINITE );		

			mStaticWave.SetSeconds( mRefreshSecond, mStaticWave.BeginSecond() + tDelta * mRefreshSecond );

			RefreshWave();

			SetEvent( mDataHandlerMutex );			
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

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CGraphicWave::OnLButtonUp(UINT nFlags, CPoint point)
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
		tViewRect = CRect( tViewRect.left + mStaticWave.LeftDelta(), tViewRect.top + mStaticWave.TopDelta(), tViewRect.right - mStaticWave.LeftDelta(), tViewRect.bottom - mStaticWave.TopDelta() );

		WaitForSingleObject( mDataHandlerMutex, INFINITE );			
		
		HSDouble tSeekSecond = max( 0.0, mZoomInRect.left - mStaticWave.LeftDelta() ) / tViewRect.Width() * mRefreshSecond;
		HSDouble tNewRefreshSecond = ( HSDouble )( mZoomInRect.Width() ) / tViewRect.Width() * mRefreshSecond;				

		HSDouble tBeginVoltage = mStaticWave.BeginVoltage() - max( 0.0, mZoomInRect.top - mStaticWave.TopDelta() ) / tViewRect.Height() * mStaticWave.TotalVoltage();
		HSDouble tTotalVoltage = ( HSDouble )mZoomInRect.Height() / tViewRect.Height() * mStaticWave.TotalVoltage();
		
		mStaticWave.SetSeconds( tNewRefreshSecond, mStaticWave.BeginSecond() + tSeekSecond );
		mStaticWave.SetVoltage( tTotalVoltage, tBeginVoltage );

		mRefreshSecond = mStaticWave.TotalSecond();	

		RefreshWave();
		
		SetEvent( mDataHandlerMutex );	
	}
	else
	{
		mParent->EndMoveSubView( nFlags, point);
	}

	mIsDraging = HSFalse;
	mIsZoomIn = HSFalse;	

	CDialogEx::OnLButtonUp(nFlags, point);
}

HSVoid CGraphicWave::SetParent( IGraphicWindowProtocol *pParent )
{
	mParent = pParent;
}

HSVoid CGraphicWave::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{	
	WaitForSingleObject( mDataHandlerMutex, INFINITE );

	mDataIdentifier = tIdentifier;

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	DEVICE_CH_IDENTIFIER tCardIdentifer( tIdentifier.CARD_IDENTIFIER, tIdentifier.CARD_INDEX );
	mDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( tCardIdentifer );
	mChannel = tIdentifier.ChannelIndex();	

	mStaticWave.SetDataHandler( mDataHandler, mDataIdentifier );

	this->mRefreshSecond = mTotalSecond;
	mStaticWave.SetVoltage( mTotalVoltage, mTotalVoltage / 2 );
	mStaticWave.SetSeconds( mRefreshSecond, 0 );

	mStaticWave.Invalidate();	

	if ( mDataHandler != NULL )
	{
		if ( mBuffer != NULL )
		{
			delete[] mBuffer;
		}

		mBuffer = new HSChar[ mDataHandler->SampleRate( tIdentifier ) * mDataHandler->EachSampleSize() ];

		if ( mDataTransfer != NULL )
		{
			delete mDataTransfer;
		}
		
		mDataTransfer = new CLinearTransfer( pMainFrame->IndexManager(), mDataHandler, mDataIdentifier );		
		
		if ( pMainFrame->IsStart() )
		{
			this->Start();
		}
	}	

	SetEvent( mDataHandlerMutex );
}

DEVICE_CH_IDENTIFIER CGraphicWave::DataIdentifier()
{
	return mDataIdentifier;
}

HSBool CGraphicWave::Save( CINIConfig *pIniConfig, string tGroup )
{
	pIniConfig->SetValue( "Voltage", this->mTotalVoltage, tGroup );
	pIniConfig->SetValue( "Second", this->mTotalSecond, tGroup );

	return HSTrue;
}

HSBool CGraphicWave::Load( CINIConfig *pIniConfig, string tGroup )
{	
	pIniConfig->ValueWithKey( "Voltage", this->mTotalVoltage, tGroup );
	pIniConfig->ValueWithKey( "Second", this->mTotalSecond, tGroup );

	return HSTrue;
}

void CGraphicWave::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnSizing(fwSide, pRect);

	// TODO: Add your message handler code here
	//OutputDebugStringA( "Sizing...\n" );
}


void CGraphicWave::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	//OutputDebugStringA( "Sized...\n" );
	if ( mIsFinishInit )
	{
		CRect tRect;
		this->GetClientRect( &tRect );
		if ( tRect.Width() > mStaticWave.LeftDelta() * 3 && tRect.Height() > mStaticWave.TopDelta() * 3 )
		{
			mStaticWave.MoveWindow( tRect, true );
			mStaticWave.SetVoltage( mStaticWave.TotalVoltage(), mStaticWave.BeginVoltage() );
			mStaticWave.SetSeconds( mRefreshSecond, mStaticWave.BeginSecond() );
			mStaticWave.ResizeWave();
			mStaticWave.Invalidate();
		}
	}
}


void CGraphicWave::OnSizeClipboard(CWnd* pClipAppWnd, HGLOBAL hRect)
{
	CDialogEx::OnSizeClipboard(pClipAppWnd, hRect);

	// TODO: Add your message handler code here
	//OutputDebugStringA( "OnSizeClipboard...\n" );
}


BOOL CGraphicWave::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	mIsFinishInit = HSTrue;

	mPopupMenu.LoadMenu( IDR_POPUP_WAVE_CH );

	this->mRefreshSecond = mTotalSecond;
	mStaticWave.SetVoltage( mTotalVoltage, mTotalVoltage / 2 );
	mStaticWave.SetSeconds( mRefreshSecond, 0 );
	mStaticWave.Invalidate();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CGraphicWave::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	mUpdateWaveThread->Stop();
}

HSBool CGraphicWave::ThreadRuning( HSInt tThreadID )
{
	WaitForSingleObject( mDataHandlerMutex, INFINITE );

	if ( this->mDataHandler && mDataTransfer->Seek( mRefreshSecond, mRefreshSecond ) && mDataTransfer->NeedRefresh() )
	{				
		HSUInt tSampleOffsest = 0;
		HSUInt tSampleIndex = 0;
		HSInt64 tSampleSecond = mDataTransfer->GetSampleReadInfo( tSampleIndex, tSampleOffsest );
		if ( tSampleSecond != 0 )
		{
			mBeginSecondOffset = tSampleSecond / 1000000000.0;
		}
		else
		{
			mBeginSecondOffset = mDataHandler->GetSampleNSecond( tSampleIndex, tSampleOffsest, mDataIdentifier ) / 1000000000.0;
		}

		HSInt tLength = mDataTransfer->Read( mBuffer, mRefreshSecond );
		mStaticWave.SetData( mBuffer, tLength, 0 );
		mStaticWave.Invalidate();		
	}

	SetEvent( mDataHandlerMutex );

	return HSTrue;
}

HSVoid CGraphicWave::ThreadWillStop( HSInt tThreadID )
{
}

void CGraphicWave::OnContextMenu(CWnd *pWnd, CPoint point)
{
	// TODO: Add your message handler code here	

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->SetGrahpicCheckType( CMainFrame::GRAPHIC_CHECK_POINTER );
	
	mPopupMenu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this );
}


void CGraphicWave::OnWavepopupExportdata()
{
	// TODO: Add your command handler code here

	if ( mStaticWave.DataLength() < 1 )
	{
		MessageBox( "没有数据!", "警告" );
		return;
	}

	HSBool tIsStart = mUpdateWaveThread->IsStart();
	if ( tIsStart )
	{
		this->Stop();
	}	
	
	mStaticWave.ExportDataToFile( this );

	if ( tIsStart )
	{
		this->Start();
	}
}


void CGraphicWave::OnWavepopupClose()
{
	// TODO: Add your command handler code here
	CDialogEx::OnOK();

	mParent->SubViewClosed( this );
}


void CGraphicWave::OnWavepopupSetdatasource()
{
	// TODO: Add your command handler code here	
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->OnViewGraphic();	
	pMainFrame->SetFocusedGraphic( this );

	if ( !mStaticWave.Focused() || pMainFrame->FocusedGraphic() != this )
	{		
		mStaticWave.SetFocused( true );
		mStaticWave.ResizeWave();
		mStaticWave.Invalidate();
	}
}


void CGraphicWave::OnWavepopupZoomin()
{
	// TODO: Add your command handler code here

	WaitForSingleObject( mDataHandlerMutex, INFINITE );

	if ( mRefreshSecond / 10 > 0.000001 )
	{
		mRefreshSecond /= 10;
		mStaticWave.SetSeconds( mRefreshSecond, mStaticWave.BeginSecond() );		

		RefreshWave();
	}

	SetEvent( mDataHandlerMutex );	
}


void CGraphicWave::OnWavepopupZoomout()
{
	// TODO: Add your command handler code here
	WaitForSingleObject( mDataHandlerMutex, INFINITE );

	if ( mRefreshSecond * 10 < 1.00000001 )
	{
		mRefreshSecond *= 10;
		mStaticWave.SetSeconds( mRefreshSecond, mStaticWave.BeginSecond() );

		RefreshWave();		
	}

	SetEvent( mDataHandlerMutex );
}

HSVoid CGraphicWave::RefreshWave()
{
	if ( this->mDataHandler && mDataTransfer->Seek( max( 0, mStaticWave.BeginSecond() + mBeginSecondOffset ), mRefreshSecond, CLinearTransfer::SET_SAMPLE ) )
	{		
		HSUInt tSampleOffsest = 0;
		HSUInt tSampleIndex = 0;

		HSInt64 tSampleSecond = mDataTransfer->GetSampleReadInfo( tSampleIndex, tSampleOffsest );
		HSDouble tBeginSecond = tSampleSecond / 1000000000.0;
		if ( tSampleSecond == 0 )
		{		
			tBeginSecond = mDataHandler->GetSampleNSecond( tSampleIndex, tSampleOffsest, mDataIdentifier ) / 1000000000.0;
		}		

		HSInt tLength = mDataTransfer->Read( mBuffer, max( mRefreshSecond, 0.0001 ) );
		mStaticWave.SetData( mBuffer, tLength, tBeginSecond - mBeginSecondOffset );
	}
	else
	{
		mStaticWave.SetData( NULL, 0, 0 );				
	}

	mStaticWave.Invalidate();	
}

HSVoid CGraphicWave::FocusGraphic( HSBool tIsFocused )
{
	if ( tIsFocused && !mStaticWave.Focused() )
	{
		mStaticWave.SetFocused( true );
		mStaticWave.ResizeWave();
		mStaticWave.Invalidate();		
	}
	else if ( !tIsFocused && mStaticWave.Focused() )
	{
		mStaticWave.SetFocused( false );
		mStaticWave.ResizeWave();
		mStaticWave.Invalidate();		
	}	
}

BOOL CGraphicWave::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );		

	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticWave.LeftDelta() && tPoint.x < tRect.right - mStaticWave.LeftDelta() && tPoint.y > tRect.top + mStaticWave.TopDelta() && tPoint.y < tRect.bottom - mStaticWave.TopDelta() )
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


void CGraphicWave::OnWavepopupReset()
{
	// TODO: Add your command handler code here

	WaitForSingleObject( mDataHandlerMutex, INFINITE );

	this->mRefreshSecond = mTotalSecond;
	mStaticWave.SetVoltage( mTotalVoltage, mTotalVoltage / 2 );
	mStaticWave.SetSeconds( mRefreshSecond, 0 );

	mBeginSecondOffset = 0;
	
	if ( this->mDataHandler && mDataTransfer->Seek( 0, mRefreshSecond, CLinearTransfer::SET_SAMPLE ) )
	{
		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
		if ( pMainFrame->IsStart() )
		{
			Start();
		}
		else
		{
			HSInt tLength = mDataTransfer->Read( mBuffer, mRefreshSecond );
			mStaticWave.SetData( mBuffer, tLength, 0 );
			mStaticWave.Invalidate();		
		}
	}
	else
	{
		mStaticWave.Invalidate();
	}
	
	SetEvent( mDataHandlerMutex );
}

HSVoid CGraphicWave::SetTime( HSDouble tSecond )
{
	mTotalSecond = tSecond;
	OnWavepopupReset();
}

HSVoid CGraphicWave::SetVoltage( HSDouble tVoltage )
{
	WaitForSingleObject( mDataHandlerMutex, INFINITE );

	HSDouble tCenterVoltage = mStaticWave.BeginVoltage() - mStaticWave.TotalVoltage() / 2;

	mTotalVoltage = tVoltage;		
	mStaticWave.SetVoltage( mTotalVoltage, tCenterVoltage + mTotalVoltage / 2 );
	mStaticWave.Invalidate();

	SetEvent( mDataHandlerMutex );

	//OnWavepopupReset();

}

void CGraphicWave::OnWavepopupFullScreen()
{
	// TODO: Add your command handler code here
	mIsFullScreen = !mIsFullScreen;
	mParent->SubViewFullScreen( this, mIsFullScreen );
}


void CGraphicWave::OnUpdateWavepopupFullScreen(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( mIsFullScreen );	
	
}


void CGraphicWave::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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
