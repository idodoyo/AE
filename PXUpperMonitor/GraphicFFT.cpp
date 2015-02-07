// GraphicFFT.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GraphicFFT.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "resource.h"
#include "HSLogProtocol.h"
#include "INIConfig.h"
#include "DataExportHelper.h"
#include "ArgCalculator.h"

// CGraphicFFT dialog

IMPLEMENT_DYNAMIC(CGraphicFFT, CDialogEx)

CGraphicFFT::CGraphicFFT(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGraphicFFT::IDD, pParent), mFourierTransform( this )
{
	mIsDraging = HSFalse;	
	mIsFinishInit = HSFalse;

	mIsZoomIn = HSFalse;

	mBuffer = NULL;

	mDataTransfer = NULL;
	mDataHandler = NULL;
	mChannel = 0;

	mDataIdentifier = 0;

	mArgTransfer = NULL;

	mCurCalFFTIndex = -1;

	mRefreshThread = new CThreadControlEx< CGraphicFFT >( this, 800, 0 );

	mDataHandlerMutex = CreateEvent( NULL, FALSE, TRUE, NULL );

	mRefreshSecond = 1.0;	

	mIsFullScreen = HSFalse;

	mBeginAmplitude = 10;
	mTotalAmplitude = 60;

	mCurSampleRate = 1.0;

	mFourierCalLength = 2 * 1024;
	mFourierResData = NULL;
	mFourierResHz = NULL;

	mVoltages = new HSFloat[ mFourierCalLength ];

	mIsFreshSecondChecked = HSFalse;
}

CGraphicFFT::~CGraphicFFT()
{
	if ( mBuffer != NULL )
	{
		delete[] mBuffer;
	}

	if ( mDataTransfer != NULL )
	{
		delete mDataTransfer;
	}

	if ( mArgTransfer != NULL )
	{
		delete mArgTransfer;		
	}

	delete mRefreshThread;
	delete[] mVoltages;
	
	CloseHandle( mDataHandlerMutex );
}

IGraphicPropertyProtocol * CGraphicFFT::CreatePropertyDlg( CWnd *pParent )
{
	CGraphicFFTProperty *pFFTProperty = new CGraphicFFTProperty;
	pFFTProperty->Create( IDD_GRAPHICFFTPROPERTY, pParent );
	return pFFTProperty;
}

HSBool CGraphicFFT::Start()
{
	if ( mDataTransfer != NULL && mDataHandler != NULL )
	{		
		mRefreshThread->Start();
		return HSTrue;
	}

	return HSFalse;	
}

HSVoid CGraphicFFT::Pause()
{
	mRefreshThread->Pause();
}

HSVoid CGraphicFFT::Stop()
{
	mRefreshThread->Stop();
}

HSVoid CGraphicFFT::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{	
	WaitForSingleObject( mDataHandlerMutex, INFINITE );

	mDataIdentifier = tIdentifier;

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	DEVICE_CH_IDENTIFIER tCardIdentifer( tIdentifier.CARD_IDENTIFIER, tIdentifier.CARD_INDEX );
	mDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( tCardIdentifer );
	mChannel = tIdentifier.ChannelIndex();	

	CString tTitle = "";	
	if ( mDataHandler )
	{
		tTitle.Format( "%s < %d > ", mDataHandler->Name().c_str(), mChannel + 1 );
		mCurSampleRate = mDataHandler->SampleRate( mDataIdentifier ) / 1000000.0;
	}
	else
	{
		tTitle = "DEVICE < CHANNEL > ";
	}

	tTitle += "(dB/MHz)";	
	
	mStaticFFT.SetTitle( tTitle );

	mStaticFFT.SetXValue( mCurSampleRate / 2.0, 0 );
	mStaticFFT.Invalidate();

	mIsFreshSecondChecked = HSFalse;
	mRefreshSecond = 1.0;

	mCurCalFFTIndex = -1;

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

		if ( mArgTransfer != NULL )
		{
			delete mArgTransfer;
			mArgList.clear();
			mTmpArgList.clear();
		}

		mHandledArgIndex = 0;

		mArgTransfer = new CArgListTransfer( mDataHandler, mDataIdentifier );
		
		if ( pMainFrame->IsStart() )
		{
			this->Start();
		}
	}	

	SetEvent( mDataHandlerMutex );
}

HSBool CGraphicFFT::Save( CINIConfig *pIniConfig, string tGroup )
{
	pIniConfig->SetValue( "BeginAmplitude", this->mBeginAmplitude, tGroup );
	pIniConfig->SetValue( "TotalAmplitude", this->mTotalAmplitude, tGroup );

	return HSTrue;
}

HSBool CGraphicFFT::Load( CINIConfig *pIniConfig, string tGroup )
{	
	pIniConfig->ValueWithKey( "BeginAmplitude", this->mBeginAmplitude, tGroup );
	pIniConfig->ValueWithKey( "TotalAmplitude", this->mTotalAmplitude, tGroup );

	return HSTrue;
}

HSVoid CGraphicFFT::FilterWave( HSDouble &tMinValue, HSDouble &tMaxValue )
{
	static HSDouble sFIR[] = { 0.000007885183080,                  
								0.000238289390077,                  
								0.001476977287234,                  
								0.005321203511731,                  
								0.014102397933053,                  
								0.029979717808644,                  
								0.053500775585607,                  
								0.082298100098353,                  
								0.110892306856709,                  
								0.132160653157548,                  
								0.140043386375926,                  
								0.132160653157548,                  
								0.110892306856709,                  
								0.082298100098353,                  
								0.053500775585607,                  
								0.029979717808644,                  
								0.014102397933053,                  
								0.005321203511731,                  
								0.001476977287234,                 
								0.000238289390077,                  
								0.000007885183080 };

	HSInt tFirS = sizeof( sFIR ) / sizeof( HSDouble );
	for ( HSInt i = mFourierCalLength - 1; i > tFirS; i-- )
	{
		mFourierResData[ i ] *= sFIR[ 0 ];
		for ( HSInt j = 1; j < tFirS; j++ )
		{
			mFourierResData[ i ] += sFIR[ j ] * mFourierResData[ i - j ];
		}
	}

	tMinValue = mFourierResData[ mFourierCalLength - 1 ];
	tMaxValue = mFourierResData[ mFourierCalLength - 1 ];
	for ( HSInt i = 0; i < mFourierCalLength / 2; i++ )
	{
		mFourierResData[ i ] = mFourierResData[ mFourierCalLength - i - 1 ];

		if ( mFourierResData[ i ] > tMaxValue )
		{
			tMaxValue = mFourierResData[ i ];
		}

		if ( mFourierResData[ i ] < tMinValue )
		{
			tMinValue = mFourierResData[ i ];
		}
	}
}

/*
HSBool CGraphicFFT::DataIsSegment()
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	

	DEVICE_CH_IDENTIFIER tCHTimeIdentifier = mDataHandler->DataIdentifier( mDataIdentifier );
	tCHTimeIdentifier.TYPE = DEVICE_CH_IDENTIFIER::CHANNEL_DATA_TYPE_TIME;

	HS_INFO( "ID: %s", tCHTimeIdentifier.ToString().c_str() );

	HSUInt tFrameIndex = 0;
	HSUInt tFrameOffset = 0;
	HSUInt tFrameLength = 0;
	if ( !pMainFrame->IndexManager()->GetIndex( tCHTimeIdentifier, 0, tFrameIndex, tFrameOffset, tFrameLength ) )
	{
		return HSFalse;
	}

	return HSTrue;
}
*/

HSInt CGraphicFFT::ReadSegmentData( HSInt tArgIndex )
{	
	if ( mCurCalFFTIndex == -1 )
	{
		if ( !mDataTransfer->Seek( 0.0, 0.0, CLinearTransfer::SET_SAMPLE ) )
		{			
			return 0;			
		}

		mCurCalFFTIndex = 0;
	}

	HSChar *pBuf = NULL;
	HSInt tLength = mDataTransfer->Read( tArgIndex, pBuf );
	if ( tLength == 0 )
	{
		return 0;
	}

	memcpy( mBuffer, pBuf, tLength );

	return tLength / mDataHandler->EachSampleSize();
}

HSBool CGraphicFFT::ReadLinearData( HSInt tArgIndex )
{
	mArgTransfer->GetArg( mTmpArgList );

	HSDouble tHitDuration = CArgCalculator::SharedInstance()->GetHitFullTime() / 1000000000.0 + 0.0005;
	HSInt tRegardIndex = 0;
	for ( HSUInt i = mHandledArgIndex; i < mTmpArgList.size(); i++ )
	{
		HSDouble tDuration = ( i > 0 ? ( mTmpArgList[ i ].NBeginTime - mTmpArgList[ i - 1 ].NBeginTime ) / 1000000000.0 : 1.0 );
		if ( tDuration < tHitDuration && tRegardIndex < 10 )
		{
			tRegardIndex++;
			continue;
		}
		else
		{
			tRegardIndex = 0;
			mArgList.push_back( mTmpArgList[ i ] );
		}
	}

	mHandledArgIndex = mTmpArgList.size();

	if ( tArgIndex >= ( HSInt )mArgList.size() )
	{
		return 0;
	}

	HSDouble tRefreshSecond = ( ( HSDouble )( mFourierCalLength ) ) / mDataHandler->SampleRate( mDataIdentifier );
	HSDouble tSecond = mArgList[ tArgIndex ].NBeginTime / 1000000000.0;

	if ( mDataTransfer->Seek( tSecond, tRefreshSecond, CLinearTransfer::SET_SAMPLE ) )
	{			
		return mDataTransfer->Read( mBuffer, tRefreshSecond );
	}

	return 0;
}

HSVoid CGraphicFFT::RefreshFFT( HSInt tDirection )
{	
	WaitForSingleObject( mDataHandlerMutex, INFINITE );

	if ( mArgTransfer )
	{
		HSInt tCurFFTIndex = mCurCalFFTIndex;

		tCurFFTIndex += tDirection;
		tCurFFTIndex = max( 0, tCurFFTIndex );		

		if ( tCurFFTIndex == mCurCalFFTIndex )
		{
			SetEvent( mDataHandlerMutex );
			return;
		}
	
		HSInt tLength = 0;
		if ( mDataHandler->DataIsInSegent() )
		{
			tLength = ReadSegmentData( tCurFFTIndex );
		}
		else
		{
			tLength = ReadLinearData( tCurFFTIndex );
		}		
		
		if ( tLength == 0 )
		{
			SetEvent( mDataHandlerMutex );
			return;
		}

		mCurCalFFTIndex = tCurFFTIndex;

		memset( mVoltages, 0, sizeof( HSFloat ) * mFourierCalLength );

		HSInt tEachSampleSize = mDataHandler->EachSampleSize();
		tLength = min( tLength, mFourierCalLength );
		for ( HSInt i = 0; i < tLength; i++ )
		{
			mVoltages[ i ] = ( HSFloat )mDataHandler->VoltageWithPointer( &mBuffer[ i * tEachSampleSize ] );
		}

		mFourierTransform.FFT( mVoltages, mFourierCalLength, mCurSampleRate, mFourierResData, mFourierResHz );

		HSDouble tMinValue = 0;
		HSDouble tMaxValue = 0;
		FilterWave( tMinValue, tMaxValue );

		tMinValue = HSInt( ( tMinValue - 9.99999999 ) / 10 ) * 10 - 10;
		tMaxValue = HSInt( ( tMaxValue + 9.99999999 ) / 10 ) * 10 + 10;

		mTotalAmplitude = tMaxValue - tMinValue;
		mBeginAmplitude = tMaxValue;

		mStaticFFT.SetYValue( mTotalAmplitude, mBeginAmplitude );
		mStaticFFT.SetData( mFourierResHz, mFourierResData, mFourierCalLength / 2 + 1 );
		mStaticFFT.Invalidate();
	}

	SetEvent( mDataHandlerMutex );
}

HSVoid CGraphicFFT::SetBeginAmplitude( HSDouble tAmplitude )
{
	mBeginAmplitude = tAmplitude;

	mStaticFFT.SetYValue( mTotalAmplitude, mBeginAmplitude + mTotalAmplitude );
	mStaticFFT.ResizeWave();
	mStaticFFT.Invalidate();	
}

HSVoid CGraphicFFT::SetTotalAmplitude( HSDouble tAmplitude )
{
	mTotalAmplitude = tAmplitude;

	mStaticFFT.SetYValue( mTotalAmplitude, mBeginAmplitude + mTotalAmplitude );
	mStaticFFT.ResizeWave();
	mStaticFFT.Invalidate();	
}

HSVoid CGraphicFFT::FocusGraphic( HSBool tIsFocused )
{
	if ( tIsFocused && !mStaticFFT.Focused() )
	{
		mStaticFFT.SetFocused( true );
		mStaticFFT.ResizeWave();
		mStaticFFT.Invalidate();		
	}
	else if ( !tIsFocused && mStaticFFT.Focused() )
	{
		mStaticFFT.SetFocused( false );
		mStaticFFT.ResizeWave();
		mStaticFFT.Invalidate();		
	}	
}

HSBool CGraphicFFT::ThreadRuning( HSInt tThreadID )
{	
	RefreshFFT();	

	return HSTrue;
}

HSVoid CGraphicFFT::ThreadWillStop( HSInt tThreadID )
{
}

void CGraphicFFT::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FFT, mStaticFFT);
}


BEGIN_MESSAGE_MAP(CGraphicFFT, CDialogEx)
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	ON_WM_INITMENUPOPUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_FFT_OPUP_SETDATASOURCE, &CGraphicFFT::OnFftOpupSetdatasource)
	ON_COMMAND(ID_FFT_POPUP_CLOSE, &CGraphicFFT::OnFftPopupClose)
	ON_COMMAND(ID_FFT_POPUP_EXPORTDATA, &CGraphicFFT::OnFftPopupExportdata)
	ON_COMMAND(ID_FFT_POPUP_FULL_SCREEN, &CGraphicFFT::OnFftPopupFullScreen)
	ON_UPDATE_COMMAND_UI(ID_FFT_POPUP_FULL_SCREEN, &CGraphicFFT::OnUpdateFftPopupFullScreen)
	ON_COMMAND(ID_FFT_POPUP_RESET, &CGraphicFFT::OnFftPopupReset)
END_MESSAGE_MAP()


// CGraphicFFT message handlers


BOOL CGraphicFFT::OnInitDialog()
{
	__super::OnInitDialog();

	// TODO:  Add extra initialization here

	mIsFinishInit = HSTrue;

	mPopupMenu.LoadMenu( IDR_POPUP_FFT_C );
	
	mStaticFFT.SetXValue( mCurSampleRate / 2.0, 0 );
	mStaticFFT.SetYValue( mTotalAmplitude, mBeginAmplitude );	
	mStaticFFT.Invalidate();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CGraphicFFT::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: Add your message handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->SetGrahpicCheckType( CMainFrame::GRAPHIC_CHECK_POINTER );
	
	mPopupMenu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this );
}


void CGraphicFFT::OnDestroy()
{
	__super::OnDestroy();

	// TODO: Add your message handler code here

	mRefreshThread->Stop();
}


void CGraphicFFT::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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


void CGraphicFFT::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	CPoint tPoint = point;
	
	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticFFT.LeftDelta() && tPoint.x < tRect.right - mStaticFFT.LeftDelta() && tPoint.y > tRect.top + mStaticFFT.TopDelta() && tPoint.y < tRect.bottom - mStaticFFT.TopDelta() )
	{		
		if ( pMainFrame->GetGrahpicCheckType() != CMainFrame::GRAPHIC_CHECK_POINTER )
		{			
			if ( mRefreshThread->IsStart() )
			{
				this->Stop();
			}
		}
		

		if ( !mStaticFFT.Focused() )
		{
			pMainFrame->SetFocusedGraphic( this );
			mStaticFFT.SetFocused( true );
			mStaticFFT.ResizeWave();
			mStaticFFT.Invalidate();
		}

		if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_STEP_FRAME )
		{
			int tDelta = 1;
			if ( tPoint.x < tRect.right / 2 )
			{
				tDelta = -1;
			}

			RefreshFFT( tDelta );				
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


void CGraphicFFT::OnLButtonUp(UINT nFlags, CPoint point)
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
		tViewRect = CRect( tViewRect.left + mStaticFFT.LeftDelta(), tViewRect.top + mStaticFFT.TopDelta(), tViewRect.right - mStaticFFT.LeftDelta(), tViewRect.bottom - mStaticFFT.TopDelta() );
		
		WaitForSingleObject( mDataHandlerMutex, INFINITE );			
		
		HSDouble tXValueOffset = max( 0.0, mZoomInRect.left - mStaticFFT.LeftDelta() ) / tViewRect.Width() * mStaticFFT.TotalXValue();
		HSDouble tNewXValue = ( HSDouble )( mZoomInRect.Width() ) / tViewRect.Width() * mStaticFFT.TotalXValue();			

		HSDouble tBeginYValue = mStaticFFT.BeginYValue() - max( 0.0, mZoomInRect.top - mStaticFFT.TopDelta() ) / tViewRect.Height() * mStaticFFT.TotalYValue();
		HSDouble tTotalYValue = ( HSDouble )mZoomInRect.Height() / tViewRect.Height() * mStaticFFT.TotalYValue();
		
		mStaticFFT.SetXValue( tNewXValue, mStaticFFT.BeginXValue() + tXValueOffset );
		mStaticFFT.SetYValue( tTotalYValue, tBeginYValue );		
		mStaticFFT.Invalidate();
		
		SetEvent( mDataHandlerMutex );
	}
	else
	{
		mParent->EndMoveSubView( nFlags, point);
	}

	mIsDraging = HSFalse;
	mIsZoomIn = HSFalse;	

	__super::OnLButtonUp(nFlags, point);
}


void CGraphicFFT::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if ( mIsDraging )
	{
		WaitForSingleObject( mDataHandlerMutex, INFINITE );	

		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		tViewRect = CRect( tViewRect.left + mStaticFFT.LeftDelta(), tViewRect.top + mStaticFFT.TopDelta(), tViewRect.right - mStaticFFT.LeftDelta(), tViewRect.bottom - mStaticFFT.TopDelta() );

		HSInt tYOffset = mPrevPoint.y - point.y;			
		HSDouble tYValuePerDigit = mStaticFFT.TotalYValue() / tViewRect.Height();
		HSDouble tBeginYValue = mStaticFFT.BeginYValue() - tYOffset * tYValuePerDigit;			
		mStaticFFT.SetYValue( mStaticFFT.TotalYValue(), tBeginYValue );		

		HSInt tXOffset = mPrevPoint.x - point.x;
		HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticFFT.TotalXValue();		
		mStaticFFT.SetXValue( mStaticFFT.TotalXValue(), mStaticFFT.BeginXValue() + tXOffsetValue );
		mStaticFFT.Invalidate();

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
	

	__super::OnMouseMove(nFlags, point);
}


void CGraphicFFT::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( mIsFinishInit )
	{
		CRect tRect;
		this->GetClientRect( &tRect );
		if ( tRect.Width() > mStaticFFT.LeftDelta() * 3 && tRect.Height() > mStaticFFT.TopDelta() * 3 )
		{
			mStaticFFT.MoveWindow( tRect, true );	
			mStaticFFT.SetXValue( mStaticFFT.TotalXValue(), mStaticFFT.BeginXValue() );
			mStaticFFT.SetYValue( mStaticFFT.TotalYValue(), mStaticFFT.BeginYValue() );
			mStaticFFT.ResizeWave();
			mStaticFFT.Invalidate();
		}
	}
}


BOOL CGraphicFFT::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );		

	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticFFT.LeftDelta() && tPoint.x < tRect.right - mStaticFFT.LeftDelta() && tPoint.y > tRect.top + mStaticFFT.TopDelta() && tPoint.y < tRect.bottom - mStaticFFT.TopDelta() )
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


void CGraphicFFT::OnFftOpupSetdatasource()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->OnViewGraphic();	
	pMainFrame->SetFocusedGraphic( this );

	if ( !mStaticFFT.Focused() || pMainFrame->FocusedGraphic() != this )
	{		
		mStaticFFT.SetFocused( true );
		mStaticFFT.ResizeWave();
		mStaticFFT.Invalidate();
	}
}


void CGraphicFFT::OnFftPopupClose()
{
	// TODO: Add your command handler code here

	CDialogEx::OnOK();

	mParent->SubViewClosed( this );
}


void CGraphicFFT::OnFftPopupExportdata()
{
	// TODO: Add your command handler code here

	if ( mFourierResData == NULL )
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
		tDataExport.Write( "FFT\n" );
		sprintf_s( tBuf, "日期: %s %s\n", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
		tDataExport.Write( tBuf ); 	

		sprintf_s( tBuf, "设备：%s, 通道: %d, 采样率: %f (MHz)\n\n", mDataHandler->Name().c_str(), mChannel + 1, mDataHandler->SampleRate( mDataIdentifier ) / 1000000.0 );
		tDataExport.Write( tBuf );

		for ( HSInt i = 0; i < mFourierCalLength; i++ )
		{
			sprintf_s( tBuf, "%f\n", mFourierResData[ i ] );
			tDataExport.Write( tBuf ); 
		}	

		tDataExport.Finish();
	}

	if ( tIsStart )
	{
		this->Start();
	}
}


void CGraphicFFT::OnFftPopupFullScreen()
{
	// TODO: Add your command handler code here

	mIsFullScreen = !mIsFullScreen;
	mParent->SubViewFullScreen( this, mIsFullScreen );
}


void CGraphicFFT::OnUpdateFftPopupFullScreen(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->SetCheck( mIsFullScreen );	
}


void CGraphicFFT::OnFftPopupReset()
{
	// TODO: Add your command handler code here

	WaitForSingleObject( mDataHandlerMutex, INFINITE );		

	mStaticFFT.SetXValue( mCurSampleRate / 2.0, 0 );
	mStaticFFT.SetYValue( mTotalAmplitude, mBeginAmplitude );
	mStaticFFT.Invalidate();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( pMainFrame->IsStart() )
	{
		Start();
	}

	SetEvent( mDataHandlerMutex );
}
