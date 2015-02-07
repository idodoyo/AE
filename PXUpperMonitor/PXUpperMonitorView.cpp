
// PXUpperMonitorView.cpp : implementation of the CPXUpperMonitorView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "PXUpperMonitor.h"
#endif

#include "PXUpperMonitorDoc.h"
#include "PXUpperMonitorView.h"
#include "MainFrm.h"
#include "ChooseColumnDlg.h"
#include "DataHandle\INIConfig.h"
#include "DeviceTypes.h"
#include "HSLogProtocol.h"

#include <list>

using std::list;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPXUpperMonitorView

IMPLEMENT_DYNCREATE(CPXUpperMonitorView, CView)

BEGIN_MESSAGE_MAP(CPXUpperMonitorView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CPXUpperMonitorView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_FILE_LOADLAYOUT, &CPXUpperMonitorView::OnFileLoadlayout)
	ON_COMMAND(ID_FILE_SAVELAYOUT, &CPXUpperMonitorView::OnFileSavelayout)	
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_AUTO_ARRANE_MUL_COL, &CPXUpperMonitorView::OnAutoArraneMulCol)
	ON_COMMAND(ID_AUTO_ARRANE_ONE_COL, &CPXUpperMonitorView::OnAutoArraneOneCol)
	ON_COMMAND(ID_AUTO_ARRANE_TUE_COL, &CPXUpperMonitorView::OnAutoArraneTueCol)
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_WM_MOUSELEAVE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CPXUpperMonitorView construction/destruction

CPXUpperMonitorView::CPXUpperMonitorView()
{
	// TODO: add construction code here
	mIsConstructNewGraphic = HSFalse;
	mMoveingWnd = NULL;

	mTracking = HSFalse;		
}

CPXUpperMonitorView::~CPXUpperMonitorView()
{	
	ClearGraphics();
}

BOOL CPXUpperMonitorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CPXUpperMonitorView drawing

void CPXUpperMonitorView::OnDraw(CDC* pDC)
{
	CPXUpperMonitorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here	
}


// CPXUpperMonitorView printing


void CPXUpperMonitorView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CPXUpperMonitorView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CPXUpperMonitorView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CPXUpperMonitorView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CPXUpperMonitorView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CPXUpperMonitorView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->SetSelectedGraphic( "" );
	//pMainFrame->SetDragingGraphic( "" );

//#ifndef SHARED_HANDLERS
//	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_AUTO_ARRANGE_C, point.x, point.y, this, TRUE);
//#endif

	CMenu tMenu;
	tMenu.LoadMenu( IDR_POPUP_AUTO_ARRANGE_C );
	tMenu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this );
}


// CPXUpperMonitorView diagnostics

#ifdef _DEBUG
void CPXUpperMonitorView::AssertValid() const
{
	CView::AssertValid();
}

void CPXUpperMonitorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CPXUpperMonitorDoc* CPXUpperMonitorView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPXUpperMonitorDoc)));
	return (CPXUpperMonitorDoc*)m_pDocument;
}
#endif //_DEBUG


// CPXUpperMonitorView message handlers

void CPXUpperMonitorView::ClearGraphics( HSBool tUnfocusWnd )
{
	CMainFrame* pMainFrame = NULL;
	if ( tUnfocusWnd )
	{
		pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	}

	list< GraphicDetail >::iterator pGraphicIterator = mGraphics.begin();
	while ( pGraphicIterator != mGraphics.end() )	
	{
		if ( tUnfocusWnd && ( *pGraphicIterator ).Graphic == pMainFrame->FocusedGraphic() )
		{
			pMainFrame->SetFocusedGraphic( NULL );
		}

		if ( ( *pGraphicIterator ).Graphic != NULL )
		{			
			delete ( *pGraphicIterator ).Graphic;
		}

		pGraphicIterator++;
	}

	mGraphics.clear();
}

void CPXUpperMonitorView::OnFileLoadlayout()
{
	// TODO: Add your command handler code here
	
	string tFilePath = theApp.PropgramFilePath() + "\\Layout\\Waves.pxl";

	CFileDialog tFileDialog( TRUE, "pxl", tFilePath.c_str(), OFN_HIDEREADONLY, "Layout Files (*.pxl)|*.pxl|" );
	if ( IDOK != tFileDialog.DoModal() )
	{
		return;
	}	

	IGraphicProtocol *pGraphic = LoadLayout( tFileDialog.GetPathName() );
	if ( pGraphic != NULL )
	{
		pGraphic->FocusGraphic( HSTrue );
		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());			
		pMainFrame->SetFocusedGraphic( pGraphic );
	}
}

IGraphicProtocol * CPXUpperMonitorView::LoadLayout( CString tLayout )
{
	CINIConfig tIniConfig;
	if ( !tIniConfig.LoadFile( ( LPCSTR )tLayout ) )
	{
		MessageBox( "加载布局错误!", "警告" );
		return NULL;
	}	

	ClearGraphics( HSTrue );		

	CGraphicManager *pGraphicManager = CGraphicManager::SharedInstance();

	HSInt tIndex = 0;
	string tGroup;	
	while ( tIniConfig.GroupNameWithIndex( tIndex++, tGroup ) )
	{
		GraphicDetail tGraphicDetail;		
		tGraphicDetail.Graphic = NULL;
		HSString tDataIdentifier = "";
		if ( tIniConfig.ValueWithKey( "Identifier", tGraphicDetail.Identifier, tGroup ) 
			&& tIniConfig.ValueWithKey( "DataIdentifier", tDataIdentifier, tGroup )
			&& tIniConfig.ValueWithKey( "Left", tGraphicDetail.LeftPercent, tGroup )
			&& tIniConfig.ValueWithKey( "Top", tGraphicDetail.TopPercent, tGroup )
			&& tIniConfig.ValueWithKey( "Right", tGraphicDetail.RightPercent, tGroup )
			&& tIniConfig.ValueWithKey( "Bottom", tGraphicDetail.BottomPercent, tGroup )
			&& ( tGraphicDetail.Graphic = pGraphicManager->GraphicWithIdentifier( tGraphicDetail.Identifier ) ) != NULL
			&& tGraphicDetail.Graphic->Load( &tIniConfig, tGroup ) )
		{		
			tGraphicDetail.DataIdentifier.InitWithString( tDataIdentifier );
			mGraphics.push_back( tGraphicDetail );
		}		
		else
		{
			ClearGraphics( HSTrue );
			MessageBox( "加载布局错误!", "警告" );
			return NULL;
		}
	}	

	CString tTitle = tLayout;
	tTitle = tTitle.Mid( tTitle.ReverseFind( '\\' ) + 1 );
	tTitle = tTitle.Left( tTitle.ReverseFind( '.' ) );

	CPXUpperMonitorDoc *pDoc = dynamic_cast< CPXUpperMonitorDoc * >( this->GetDocument() );
	pDoc->SetTitle( tTitle );
	pDoc->SetLayout( tLayout );

	CRect tViewRect;
	this->GetClientRect( &tViewRect );
	HSInt tWidth = tViewRect.Width();
	HSInt tHeight = tViewRect.Height();

	IGraphicProtocol *pLastGraphic = NULL;
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
	list< GraphicDetail >::iterator pIterator = mGraphics.begin();	
	while ( pIterator != mGraphics.end() )
	{				
		CRect tRect( ( HSInt )( ( *pIterator ).LeftPercent * tWidth ), 
					( HSInt )( ( *pIterator ).TopPercent * tHeight ), 
					( HSInt )( ( *pIterator ).RightPercent * tWidth ), 
					( HSInt )( ( *pIterator ).BottomPercent * tHeight ) );

		IGraphicProtocol *pGraphic = pIterator->Graphic;		
		pGraphic->SetParent( this );
		pGraphic->Wnd()->Create( pGraphic->ResourceID(), this );
		pGraphic->Wnd()->MoveWindow( tRect );
		pGraphic->Wnd()->ShowWindow( SW_SHOW );	
				
		pGraphic->SetDataIdentifier( ( *pIterator ).DataIdentifier );	

		pLastGraphic = pGraphic;	

		pIterator++;
	}		

	return pLastGraphic;
}


void CPXUpperMonitorView::OnFileSavelayout()
{
	// TODO: Add your command handler code here
	CPXUpperMonitorDoc *pDoc = dynamic_cast< CPXUpperMonitorDoc * >( this->GetDocument() );
	string tLayout = pDoc->GetLayout();
	if ( tLayout == "" )
	{
		tLayout = theApp.PropgramFilePath() + "\\Layout\\Layout.pxl";
	}
	
	CFileDialog tFileDialog( FALSE, "pxl", tLayout.c_str(), OFN_HIDEREADONLY, "Layout Files (*.pxl)|*.pxl|" );
	if ( IDOK != tFileDialog.DoModal() )
	{
		return;
	}	

	if ( !SaveLayout( tFileDialog.GetPathName(), tFileDialog.GetFileTitle() ) )
	{
		MessageBox( "保存失败!", "警告" );
	}
}

bool CPXUpperMonitorView::SaveLayout( CString tLayout, CString tTitle )
{
	CalcGraphicPos();

	CINIConfig tIniConfig;
	
	list< GraphicDetail >::iterator pIterator = mGraphics.begin();

	HSInt tIndex = 1;
	HSChar tGroup[ 32 ];
	while ( pIterator != mGraphics.end() )
	{
		sprintf_s( tGroup, "Graphic %d", tIndex++ );

		tIniConfig.SetValue( "Identifier", ( *pIterator ).Graphic->Identifier(), tGroup );
		tIniConfig.SetValue( "DataIdentifier", ( *pIterator ).Graphic->DataIdentifier().ToString(), tGroup );
		tIniConfig.SetValue( "Left", ( *pIterator ).LeftPercent, tGroup );
		tIniConfig.SetValue( "Top", ( *pIterator ).TopPercent, tGroup );
		tIniConfig.SetValue( "Right", ( *pIterator ).RightPercent, tGroup );
		tIniConfig.SetValue( "Bottom", ( *pIterator ).BottomPercent, tGroup );		
		pIterator->Graphic->Save( &tIniConfig, tGroup );

		pIterator++;
	}	

	if ( !tIniConfig.Save( ( LPCSTR )tLayout ) )
	{		
		return false;
	}	
		
	if ( tTitle != "" )
	{
		CPXUpperMonitorDoc *pDoc = dynamic_cast< CPXUpperMonitorDoc * >( this->GetDocument() );	
		pDoc->SetTitle( tTitle );
		pDoc->SetLayout( tLayout );
	}

	return true;
}

BOOL CPXUpperMonitorView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	CRect tRect;
	GetClientRect( &tRect );	
	pDC->FillSolidRect( &tRect, RGB( 120, 120, 120 ) );

	return TRUE;
}

void CPXUpperMonitorView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default	
	CRect tRect;
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( pMainFrame->DragingGraphic() != "" )
	{
		tRect = CRect( point.x - 200, point.y - 100, point.x + 200, point.y + 100 );

		if ( !mTracking )
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = this->m_hWnd;
			tme.dwHoverTime = 10;

			if (::_TrackMouseEvent(&tme)) 
			{
				mTracking = HSTrue;   
			}
		}
	}
	else if ( mIsConstructNewGraphic )
	{
		tRect = CRect( mBeginDrawPoint, point );
		tRect.OffsetRect( 0, 8 );
		tRect.NormalizeRect();
	}
	else if ( mMoveingWnd != NULL )
	{
		HSInt tXOffset = point.x - mBeginMoveWndPoint.x;
		HSInt tYOffset = point.y - mBeginMoveWndPoint.y;
		CRect tNewRect( mMoveingRect.left + tXOffset, mMoveingRect.top + tYOffset, mMoveingRect.right + tXOffset, mMoveingRect.bottom + tYOffset );

		HSInt tLeft = mMoveingRect.left + tXOffset;
		HSInt tTop = mMoveingRect.top + tYOffset;
		HSInt tRight = mMoveingRect.right + tXOffset;
		HSInt tBottom = mMoveingRect.bottom + tYOffset;

		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		if ( tLeft < tViewRect.left )
		{
			tLeft = tViewRect.left;
			tRight = tLeft + mMoveingRect.Width();
		}
		else if ( tRight > tViewRect.right )
		{
			tRight = tViewRect.right;
			tLeft = tRight - mMoveingRect.Width();
		}

		if ( tTop < tViewRect.top )
		{
			tTop = tViewRect.top;
			tBottom = tTop + mMoveingRect.Height();
		}
		else if ( tBottom > tViewRect.bottom )
		{
			tBottom = tViewRect.bottom;
			tTop = tBottom - mMoveingRect.Height();
		}
		
		tRect = CRect( tLeft, tTop, tRight, tBottom );
	}			
	else
	{
		return;
	}

	CDC *pDC = this->GetDC();
	if ( pDC != NULL )
	{
		CBrush tBrush;		
		tBrush.CreateSolidBrush( RGB( 100, 50, 10 ) );

		if( mFirstDraw )
		{
			mFirstDraw = HSFalse;
			pDC->DrawDragRect( tRect, CSize( 5, 5 ), NULL, CSize( 5, 5 ) , &tBrush, &tBrush );
		}
		else
		{
			pDC->DrawDragRect( tRect, CSize( 5, 5 ), mGraphicRect, CSize( 5, 5 ) , &tBrush, &tBrush );
		}

		mGraphicRect = tRect;
	}

	CView::OnMouseMove(nFlags, point);
}

void CPXUpperMonitorView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default	
	string tNewWndGraphic = "";
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( pMainFrame->DragingGraphic() != "" )
	{
		tNewWndGraphic = pMainFrame->DragingGraphic();
		pMainFrame->SetSelectedGraphic( "" );
		pMainFrame->SetDragingGraphic( "" );

		mFirstDraw = HSTrue;
	}
	else if ( mIsConstructNewGraphic )
	{
		tNewWndGraphic = pMainFrame->SelectedGraphic();
	}
	else if ( mMoveingWnd != NULL )
	{
		if ( mGraphicRect.Width() > 0 && mGraphicRect.Height() > 0 )
		{
			mMoveingWnd->MoveWindow( mGraphicRect, TRUE );	
		
			mMoveingWnd->ShowWindow( SW_HIDE );
			mMoveingWnd->ShowWindow( SW_SHOW );
		}

		mMoveingWnd = NULL;		
	}			
	else
	{
		return;
	}

	CDC *pDC = this->GetDC();
	if ( pDC != NULL )
	{
		CRect tRect( 0, 0, 0, 0 );
		CBrush tBrush;		
		tBrush.CreateSolidBrush( RGB( 100, 50, 10 ) );
		pDC->DrawDragRect( tRect, CSize( 5, 5 ), mGraphicRect, CSize( 5, 5 ) , &tBrush, &tBrush );

		mIsConstructNewGraphic = HSFalse;

		if ( tNewWndGraphic != "" && mGraphicRect.Width() > 0 && mGraphicRect.Height() > 0 )
		{
			tRect = CRect( CPoint( mGraphicRect.left, mGraphicRect.top ), CSize( max( mGraphicRect.Width(), 100 ), max( mGraphicRect.Height(), 80 ) ) );

			CGraphicManager *pGraphicManager = CGraphicManager::SharedInstance();
			IGraphicProtocol *pGraphic = pGraphicManager->GraphicWithIdentifier( tNewWndGraphic );		
			pGraphic->SetParent( this );
			pGraphic->Wnd()->Create( pGraphic->ResourceID(), this );
			pGraphic->Wnd()->MoveWindow( tRect );
			pGraphic->Wnd()->ShowWindow( SW_SHOW );

			pGraphic->FocusGraphic( HSTrue );
			pMainFrame->SetFocusedGraphic( pGraphic );

			GraphicDetail tDetail;
			tDetail.DataIdentifier = pGraphic->DataIdentifier();
			tDetail.Identifier = tNewWndGraphic;
			tDetail.Graphic = pGraphic;
			tDetail.LeftPercent = 0;
			tDetail.TopPercent = 0;
			tDetail.RightPercent = 0;
			tDetail.BottomPercent = 0;
			mGraphics.push_back( tDetail );				
		}

		CalcGraphicPos();
	}

	CView::OnLButtonUp(nFlags, point);
}


void CPXUpperMonitorView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if ( mMoveingWnd != NULL )
	{
		return;
	}

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( pMainFrame->SelectedGraphic() != "" )
	{
		mIsConstructNewGraphic = HSTrue;
		mBeginDrawPoint = point;
		mFirstDraw = HSTrue;

		mGraphicRect = CRect( 0, 0, 0, 0 );
	}	

	CView::OnLButtonDown(nFlags, point);
}

void CPXUpperMonitorView::BeginMoveSubView( CWnd *pWnd, UINT tFlags, CPoint tPoint )
{
	if ( mMoveingWnd != NULL )
	{
		return;
	}

	mGraphicRect = CRect( 0, 0, 0, 0 );

	GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );
	mBeginMoveWndPoint = tPoint;

	mMoveingWnd = pWnd;

	CRect tViewRect;
	pWnd->GetWindowRect( &tViewRect );		
	this->ScreenToClient( &tViewRect );

	mMoveingRect = tViewRect;

	mFirstDraw = HSTrue;

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());		
	pMainFrame->SetSelectedGraphic( "" );
	pMainFrame->SetDragingGraphic( "" );
}

void CPXUpperMonitorView::SubViewClosed( CWnd *pWnd )
{	
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	

	HSBool tShowFullScreen = HSFalse;
	list< GraphicDetail >::iterator pWndIterator = mGraphics.end();
	list< GraphicDetail >::iterator pIterator = mGraphics.begin();
	while ( pIterator != mGraphics.end() )
	{		
		if ( ( *pIterator ).Graphic->Wnd() == pWnd )
		{
			pWndIterator = pIterator;
		}	

		if ( ( *pIterator ).Graphic->ShowFullScreen() )
		{
			tShowFullScreen = HSTrue;
		}

		pIterator++;
	}	

	if ( pWndIterator != mGraphics.end() )
	{
		if ( pWndIterator->Graphic == pMainFrame->FocusedGraphic() )
		{
			pMainFrame->SetFocusedGraphic( NULL );			
		}

		pWndIterator->Graphic->Stop();
		delete pWndIterator->Graphic;
		mGraphics.erase( pWndIterator );
	}

	if ( tShowFullScreen )
	{
		list< GraphicDetail >::iterator pIterator = mGraphics.begin();
		while ( pIterator != mGraphics.end() )
		{					
			( *pIterator ).Graphic->Wnd()->ShowWindow( SW_SHOW );
			pIterator++;
		}	
	}
}

void CPXUpperMonitorView::MoveingSubView( UINT tFlags, CPoint tPoint )
{
	GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );

	OnMouseMove( tFlags, tPoint );
}

void CPXUpperMonitorView::EndMoveSubView( UINT tFlags, CPoint tPoint )
{
	GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );

	OnLButtonUp( tFlags, tPoint );
}

void CPXUpperMonitorView::SubViewFullScreen( CWnd *pWnd, HSBool tIsFullScreen )
{
	list< GraphicDetail >::iterator pGraphicIterator = mGraphics.begin();
	while ( pGraphicIterator != mGraphics.end() )	
	{				
		if ( ( *pGraphicIterator ).Graphic->Wnd() == pWnd )
		{			
			( *pGraphicIterator ).Graphic->Wnd()->ShowWindow( SW_SHOW );			
		}
		else
		{
			( *pGraphicIterator ).Graphic->Wnd()->ShowWindow( tIsFullScreen ? SW_HIDE : SW_SHOW );
		}
		
		pGraphicIterator++;
	}		

	ResizeGraphics();
}

IGraphicProtocol * CPXUpperMonitorView::GraphicWithIdentifier( HSString tIdentifier )
{
	list< GraphicDetail >::iterator pGraphicIterator = mGraphics.begin();
	while ( pGraphicIterator != mGraphics.end() )	
	{				
		if ( ( *pGraphicIterator ).Identifier == tIdentifier )
		{			
			return ( *pGraphicIterator ).Graphic;
		}
		
		pGraphicIterator++;
	}		

	return NULL;
}

void CPXUpperMonitorView::ArrangeGrapics( HSInt tColumnNum )
{
	if ( mGraphics.size() < 1 )
	{
		return;
	}

	struct GraphicInfo
	{
		IGraphicProtocol *Graphic;
		HSInt Value;
	};

	list< struct GraphicInfo > tGraphicsInfo;	

	list< GraphicDetail >::iterator pGraphicIterator = mGraphics.begin();
	while ( pGraphicIterator != mGraphics.end() )	
	{
		CRect tViewRect;
		( *pGraphicIterator ).Graphic->Wnd()->GetWindowRect( &tViewRect );		
		this->ScreenToClient( &tViewRect );

		HSInt tValue = ( tViewRect.left + tViewRect.right ) / 2 + ( tViewRect.top + tViewRect.bottom ) / 2 / 100  * 500;

		list< struct GraphicInfo >::iterator pIterator = tGraphicsInfo.begin();
		while ( pIterator != tGraphicsInfo.end() )
		{
			if ( ( *pIterator ).Value > tValue )
			{				
				break;
			}
			pIterator++;
		}

		struct GraphicInfo tGraphicInfo = { ( *pGraphicIterator ).Graphic, tValue };
		tGraphicsInfo.insert( pIterator, tGraphicInfo );

		pGraphicIterator++;
	}		

	CRect tViewRect;
	this->GetClientRect( &tViewRect );	

	HSInt tRows = ( mGraphics.size() + tColumnNum - 1 ) / tColumnNum;
	HSInt tWidth = tViewRect.Width() / tColumnNum;
	HSInt tHeight = tViewRect.Height() / tRows;

	HSInt tWidthMod = tViewRect.Width() % tColumnNum;
	HSInt tHeightMod = tViewRect.Height() % tRows;

	HSInt tRow = 0;
	HSInt tColumn = 0;
	list< struct GraphicInfo >::iterator pIterator = tGraphicsInfo.begin();
	while ( pIterator != tGraphicsInfo.end() )
	{
		CPoint tPoint( tViewRect.left + tWidth * tColumn, tViewRect.top + tHeight * tRow );
		CSize tSize( tWidth + ( tColumnNum == ( tColumn + 1 ) ? tWidthMod : 0 ), tHeight + ( tRows == ( tRow + 1 ) ? tHeightMod : 0 ) );
		CRect tRect( tPoint, tSize );
		( *pIterator ).Graphic->Wnd()->MoveWindow( tRect );
		( *pIterator ).Graphic->Wnd()->ShowWindow( SW_SHOW );
		( *pIterator ).Graphic->ShowFullScreen( HSFalse, ( *pIterator ).Graphic->Wnd() );

		tColumn++;

		if ( tColumn == tColumnNum )
		{
			tColumn = 0;
			tRow++;
		}

		pIterator++;
	}	
}

void CPXUpperMonitorView::OnAutoArraneMulCol()
{
	// TODO: Add your command handler code here
	CChooseColumnDlg tDlg;
	if ( IDOK == tDlg.DoModal() )
	{
		ArrangeGrapics( tDlg.ColumnNum() );
		CalcGraphicPos();
	}
}


void CPXUpperMonitorView::OnAutoArraneOneCol()
{
	// TODO: Add your command handler code here
	ArrangeGrapics( 1 );
	CalcGraphicPos();
}


void CPXUpperMonitorView::OnAutoArraneTueCol()
{
	// TODO: Add your command handler code here
	ArrangeGrapics( 2 );
	CalcGraphicPos();
}

void CPXUpperMonitorView::CalcGraphicPos()
{
	CRect tViewRect;
	this->GetClientRect( &tViewRect );
	HSInt tWidth = tViewRect.Width();
	HSInt tHeight = tViewRect.Height();

	list< GraphicDetail >::iterator pIterator = mGraphics.begin();
	while ( pIterator != mGraphics.end() )
	{
		if ( ( *pIterator ).Graphic->ShowFullScreen() )
		{
			return;
		}

		pIterator++;
	}	

	pIterator = mGraphics.begin();
	while ( pIterator != mGraphics.end() )
	{		
		CRect tRect;
		( *pIterator ).Graphic->Wnd()->GetWindowRect( &tRect );		
		this->ScreenToClient( &tRect );

		( *pIterator ).LeftPercent = ( HSFloat )tRect.left / tWidth;
		( *pIterator ).TopPercent = ( HSFloat )tRect.top / tHeight;
		( *pIterator ).RightPercent = ( HSFloat )tRect.right / tWidth;
		( *pIterator ).BottomPercent = ( HSFloat )tRect.bottom / tHeight;		

		pIterator++;
	}	
}

void CPXUpperMonitorView::ResizeGraphics()
{
	CRect tViewRect;
	this->GetClientRect( &tViewRect );
	HSInt tWidth = tViewRect.Width();
	HSInt tHeight = tViewRect.Height();
	
	CWnd *pFullScreenWnd = NULL;
	list< GraphicDetail >::iterator pIterator = mGraphics.begin();
	while ( pIterator != mGraphics.end() )
	{
		CRect tRect( ( HSInt )( ( *pIterator ).LeftPercent * tWidth ), 
					( HSInt )( ( *pIterator ).TopPercent * tHeight ), 
					( HSInt )( ( *pIterator ).RightPercent * tWidth ), 
					( HSInt )( ( *pIterator ).BottomPercent * tHeight ) );

		( *pIterator ).Graphic->Wnd()->MoveWindow( tRect );			
		if ( ( *pIterator ).Graphic->ShowFullScreen() )
		{
			pFullScreenWnd = ( *pIterator ).Graphic->Wnd();
		}

		pIterator++;
	}	

	if ( pFullScreenWnd != NULL )
	{
		pFullScreenWnd->MoveWindow( tViewRect );
	}
}

BOOL CPXUpperMonitorView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( pMainFrame->DragingGraphic() != "" )
	{
		SetCursor( LoadCursor( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDC_CURSOR_DRAG ) ) );
		return true;
	}
	else if ( mMoveingWnd != NULL )
	{
		SetCursor( AfxGetApp()->LoadStandardCursor( IDC_HAND ) );
		return true;
	}
	else if ( pMainFrame->SelectedGraphic() != "" )
	{
		SetCursor( LoadCursor( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDC_CURSOR_CROSS ) ) );
		return true;
	}	

	return __super::OnSetCursor(pWnd, nHitTest, message);
}


void CPXUpperMonitorView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	ResizeGraphics();
}


void CPXUpperMonitorView::OnMouseLeave()
{
	// TODO: Add your message handler code here and/or call default	
	CRect tRect( 0, 0, 0, 0 );
	CDC *pDC = this->GetDC();
	if ( pDC != NULL )
	{		
		CBrush tBrush;		
		tBrush.CreateSolidBrush( RGB( 100, 50, 10 ) );
		pDC->DrawDragRect( tRect, CSize( 5, 5 ), mGraphicRect, CSize( 5, 5 ) , &tBrush, &tBrush );
	}

	mGraphicRect = tRect;	

	mTracking = HSFalse;

	__super::OnMouseLeave();
}


void CPXUpperMonitorView::OnDestroy()
{
	__super::OnDestroy();

	// TODO: Add your message handler code here
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	list< GraphicDetail >::iterator pGraphicIterator = mGraphics.begin();
	while ( pGraphicIterator != mGraphics.end() )	
	{
		( *pGraphicIterator ).Graphic->Stop();

		if ( ( *pGraphicIterator ).Graphic == pMainFrame->FocusedGraphic() )
		{
			pMainFrame->SetFocusedGraphic( NULL );			
		}

		pGraphicIterator++;
	}	
}

void CPXUpperMonitorView::Start()
{
	list< GraphicDetail >::iterator pGraphicIterator = mGraphics.begin();
	while ( pGraphicIterator != mGraphics.end() )	
	{
		( *pGraphicIterator ).Graphic->Start();

		pGraphicIterator++;
	}
}

void CPXUpperMonitorView::Pause()
{
	list< GraphicDetail >::iterator pGraphicIterator = mGraphics.begin();
	while ( pGraphicIterator != mGraphics.end() )	
	{
		( *pGraphicIterator ).Graphic->Pause();

		pGraphicIterator++;
	}	
}

void CPXUpperMonitorView::Stop()
{
	list< GraphicDetail >::iterator pGraphicIterator = mGraphics.begin();
	while ( pGraphicIterator != mGraphics.end() )	
	{		
		( *pGraphicIterator ).Graphic->Stop();		

		pGraphicIterator++;
	}	
}

void CPXUpperMonitorView::Reset()
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	list< GraphicDetail >::iterator pGraphicIterator = mGraphics.begin();
	while ( pGraphicIterator != mGraphics.end() )	
	{
		DEVICE_CH_IDENTIFIER tDataIdentifier = ( *pGraphicIterator ).Graphic->DataIdentifier();		
		( *pGraphicIterator ).Graphic->SetDataIdentifier( tDataIdentifier );

		pGraphicIterator++;
	}	
}