
#include "stdafx.h"
#include "mainfrm.h"
#include "DeviceView.h"
#include "Resource.h"
#include "PXUpperMonitor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeviceView

CDeviceView::CDeviceView()
{
	mIsChooseDeviceEnable = HSTrue;
}

CDeviceView::~CDeviceView()
{
}


BEGIN_MESSAGE_MAP(CDeviceView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()	
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_DEVICE_REFRESH, OnRefreshDevices)
	ON_COMMAND(ID_EXPLORER_HIDE, OnViewDevices)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(NM_CLICK, ID_DEVICE_TREE, &CDeviceView::OnClickDevices)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

int CDeviceView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if ( !mWndDeviceView.Create(dwViewStyle, rectDummy, this, ID_DEVICE_TREE ) )
	{
		TRACE0("Failed to create file view\n");
		return -1;      // fail to create
	}

	mWndDeviceView.ModifyStyle( 0, TVS_CHECKBOXES );

	// Load view images:
	mDeviceViewImages.Create( IDB_DEVICE_VIEW, 16, 0, RGB(255, 0, 255));
	mWndDeviceView.SetImageList(&mDeviceViewImages, TVSIL_NORMAL);

	mWndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	mWndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	mWndToolBar.SetPaneStyle(mWndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	mWndToolBar.SetPaneStyle(mWndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	mWndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	mWndToolBar.SetRouteCommandsViaFrame(FALSE);

	// Fill in some static tree view data (dummy code, nothing magic here)
	FillDeviceView();
	AdjustLayout();

	return 0;
}

void CDeviceView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CDeviceView::FillDeviceView( HSBool tScanNewDevices, vector< vector< HSInt > > *pDevicesOpenChannels )
{		
	mWndDeviceView.DeleteAllItems( );

	HTREEITEM hRoot = mWndDeviceView.InsertItem(_T("设备"), 0, 0 );
	mWndDeviceView.SetItemState( hRoot, TVIS_BOLD, TVIS_BOLD );

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( tScanNewDevices )
	{
		pMainFrame->DeviceManager()->ScanDevices();
	}

	vector< IDataHandler * > &pDevices = *( pMainFrame->DeviceManager()->Devices() );
	for ( HSUInt i = 0; i < pDevices.size(); i++ )
	{
		HSInt tGraphicIndex = ( pDevices[ i ]->Type() == DEVICE_USB_CARD ? 1 : 2 );
		HTREEITEM hDevice = mWndDeviceView.InsertItem( pDevices[ i ]->Name().c_str(), tGraphicIndex, tGraphicIndex, hRoot );		
		mWndDeviceView.SetItemState( hDevice, TVIS_BOLD, TVIS_BOLD );		

		char tBuf[ 16 ];
		for ( HSInt j = 0; j < pDevices[ i ]->ChannelNum(); j++ )
		{
			sprintf_s( tBuf, "通道%d", j + 1 );
			mWndDeviceView.InsertItem( tBuf, 3, 3, hDevice );
		}		

		mWndDeviceView.Expand( hDevice, TVE_EXPAND );

		if ( i == 0 && tScanNewDevices )
		{
			mWndDeviceView.SetCheck( hRoot, true );
			mWndDeviceView.SetCheck( hDevice, true );
			SetChildCheck( hDevice, true );	
		}
	}	

	mWndDeviceView.Expand( hRoot, TVE_EXPAND );

	if ( !tScanNewDevices )
	{
		mWndDeviceView.SetCheck( hRoot, true );
		CheckSelectChannels( hRoot, pDevicesOpenChannels );

		this->EnableChooseDevice( FALSE );
	}
}

void CDeviceView::CheckSelectChannels( HTREEITEM tRoot, vector< vector< HSInt > > *pDevicesOpenChannels )
{
	vector< vector< HSInt > >::iterator pDeviceIterator = pDevicesOpenChannels->begin();
	HTREEITEM tDeviceNode = mWndDeviceView.GetChildItem( tRoot );  
	while ( tDeviceNode )
	{
		mWndDeviceView.SetCheck( tDeviceNode, true );		
		for ( HSUInt i = 0; i < pDeviceIterator->size(); i++ )
		{
			HTREEITEM tChannelNode = mWndDeviceView.GetChildItem( tDeviceNode );  
			HSInt tIndex = 0;
			while ( tChannelNode && tIndex < ( *pDeviceIterator )[ i ] )
			{				
				tIndex++;
				tChannelNode = mWndDeviceView.GetNextSiblingItem( tChannelNode ); 
			}
			mWndDeviceView.SetCheck( tChannelNode, true );		
		}

		pDeviceIterator++;
		tDeviceNode = mWndDeviceView.GetNextSiblingItem( tDeviceNode ); 
	}
}

void CDeviceView::OnContextMenu(CWnd* pWnd, CPoint point)
{
/*	CTreeCtrl* pWndTree = (CTreeCtrl*) &mWndDeviceView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER_C, point.x, point.y, this, TRUE);
	*/
}

void CDeviceView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = mWndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	mWndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	mWndDeviceView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CDeviceView::OnRefreshDevices()
{
	FillDeviceView();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->ResetAllGraphic();
}

void CDeviceView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	mWndDeviceView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CDeviceView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	mWndDeviceView.SetFocus();
}

void CDeviceView::OnChangeVisualStyle()
{
	mWndToolBar.CleanUpLockedImages();
	mWndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);

	mDeviceViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	mDeviceViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	mDeviceViewImages.Add(&bmp, RGB(255, 0, 255));

	mWndDeviceView.SetImageList(&mDeviceViewImages, TVSIL_NORMAL);
}


void CDeviceView::OnClickDevices(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	CPoint tPoint;	
	GetCursorPos( &tPoint );    
	mWndDeviceView.ScreenToClient( &tPoint ); 		

	UINT tFlag;   	
	HTREEITEM tNode = mWndDeviceView.HitTest( tPoint, &tFlag );  

	if ( tNode && ( TVHT_ONITEMSTATEICON & tFlag) ) 
	{
		BOOL tCheck = mWndDeviceView.GetCheck( tNode );   
		if ( mIsChooseDeviceEnable )
		{			 
			SetChildCheck( tNode, !tCheck );          

			HTREEITEM tParentNode = tNode;
			while ( !tCheck && ( tParentNode = mWndDeviceView.GetParentItem( tParentNode ) ) )
			{
				mWndDeviceView.SetCheck( tParentNode, true );
			}
		}
		else
		{
			mWndDeviceView.SetCheck( tNode, !tCheck );
		}
	}
	else if ( tNode )
	{
		HTREEITEM tRootNode = mWndDeviceView.GetRootItem();
		HTREEITEM tParentNode = mWndDeviceView.GetParentItem( tNode );		  

		if ( tParentNode == tRootNode )
		{
			HSInt tIndex = 0;
			HTREEITEM tCurrentNode = mWndDeviceView.GetChildItem( tRootNode );
			while ( tCurrentNode != tNode )
			{
				tCurrentNode = mWndDeviceView.GetNextSiblingItem( tCurrentNode ); 
				tIndex++;
			}

			CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
			vector< IDataHandler * > &pDevices = *( pMainFrame->DeviceManager()->Devices() );

			theApp.OpenDeviceSettingDlg( pDevices[ tIndex ]->Identifier() );
		}
	}
	
	*pResult = 0;
}

void CDeviceView::SetChildCheck( HTREEITEM tNode, BOOL tCheck )
{
	tNode = mWndDeviceView.GetChildItem( tNode );  
	while ( tNode )
	{
		mWndDeviceView.SetCheck( tNode, tCheck );
		SetChildCheck( tNode, tCheck ); 
		tNode = mWndDeviceView.GetNextSiblingItem( tNode ); 
	}
}

void CDeviceView::OnViewDevices()
{	
	this->ShowPane( FALSE, TRUE, FALSE );

	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

	pMainFrame->SetFocus();		
	pMainFrame->RecalcLayout();
}


vector< HSInt > CDeviceView::OpenChannelsWithDevice( HSInt tIndex )
{
	vector< HSInt > tChannels;

	HTREEITEM tRootItem = mWndDeviceView.GetRootItem();
	HTREEITEM tDeviceItem = mWndDeviceView.GetChildItem( tRootItem );
	while ( tIndex > 0 && tDeviceItem != NULL )
	{
		tDeviceItem = mWndDeviceView.GetNextItem( tDeviceItem, TVGN_NEXT );
		tIndex--;
	}

	HTREEITEM tChannelItem = mWndDeviceView.GetChildItem( tDeviceItem );
	tIndex = 0;
	while ( tChannelItem != NULL )
	{
		if ( mWndDeviceView.GetCheck( tChannelItem ) != 0 )
		{
			tChannels.push_back( tIndex );
		}

		tChannelItem = mWndDeviceView.GetNextItem( tChannelItem, TVGN_NEXT );
		tIndex++;
	}

	return tChannels;
}

HSVoid CDeviceView::EnableChooseDevice( HSBool tEnable )
{
	mIsChooseDeviceEnable = tEnable;
	//mWndDeviceView.EnableWindow( tEnable );
	mWndToolBar.EnableWindow( tEnable );
}