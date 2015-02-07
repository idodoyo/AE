
#include "stdafx.h"

#include "GraghicWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "PXUpperMonitor.h"
#include "GraphicManager.h"
#include <sstream>
#include "HSLogProtocol.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CGraghicWnd::CGraghicWnd()
{
	mGraphicListType = GRAPHIC_ICON;
	mIconPrevSelectItem = 0;
	mListPrevSelectItem = 0;

	mDefaultFilePath = "C:\\data.pxd";
	mFocusedGrahpic = NULL;
}

CGraghicWnd::~CGraghicWnd()
{
	map< string, IGraphicPropertyProtocol * >::iterator pIterator = mGraphicProperties.begin();
	while ( pIterator != mGraphicProperties.end() )
	{
		delete pIterator->second;
		pIterator++;
	}
}

BEGIN_MESSAGE_MAP(CGraghicWnd, CDockablePane)
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
	ON_WM_CREATE()
	ON_WM_SIZE()	
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)	
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_COMMAND(ID_LIST_ICON, &CGraghicWnd::OnListIcon)
	ON_UPDATE_COMMAND_UI(ID_LIST_ICON, &CGraghicWnd::OnUpdateListIcon)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_ICON_PROPERTIES, &CGraghicWnd::OnIconProperties)
	ON_UPDATE_COMMAND_UI(ID_ICON_PROPERTIES, &CGraghicWnd::OnUpdateIconProperties)
	ON_COMMAND(ID_LIST_ICONLIST, &CGraghicWnd::OnListIconlist)
	ON_UPDATE_COMMAND_UI(ID_LIST_ICONLIST, &CGraghicWnd::OnUpdateListIconlist)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CGraghicWnd::AdjustLayout( bool tAuto )
{		
	if (GetSafeHwnd() == NULL)
	{
		return;
	}
	
	CRect tClientRect;
	GetClientRect( tClientRect );	

	int tGraphicHeight = tClientRect.Height() * 2 / 3;
	
	int tHeight[] = { mGraphicToolBar.CalcFixedLayout( FALSE, TRUE ).cy,
						tGraphicHeight, 
						mGraphicToolBar.CalcFixedLayout( FALSE, TRUE ).cy, 
						0  };
	
	CWnd *pGraphicListTypeWnd = &mGraphicIcon;
	if ( mGraphicListType == GRAPHIC_LIST )
	{
		pGraphicListTypeWnd = &mGraphicList;
	}
	else if ( mGraphicListType == GRAPHIC_PROPERTY )
	{		
		if ( this->mFocusedGrahpic != NULL )
		{				
			pGraphicListTypeWnd = mGraphicProperties[ mFocusedGrahpic->Identifier() ]->Wnd();
			mGraphicProperties[ mFocusedGrahpic->Identifier() ]->SetParent( mFocusedGrahpic );			
		}
		else
		{
			pGraphicListTypeWnd = mGraphicProperties.begin()->second->Wnd();
			mGraphicProperties.begin()->second->SetParent( NULL );
		}
	}
	
	CWnd *pWnd[] = { &mGraphicToolBar, pGraphicListTypeWnd, &mFileToolBar, &mWndPropList };

	int tTop = tClientRect.top;
	for ( int i = 0; i < sizeof( pWnd ) / sizeof( CWnd * ); i++ )
	{
		if ( i == ( sizeof( pWnd ) / sizeof( CWnd * ) - 1 ) )
		{
			tHeight[ i ] = tClientRect.Height() - tTop;
		}
	
		pWnd[ i ]->SetWindowPos( NULL, tClientRect.left, tTop, tClientRect.Width(), tHeight[ i ], SWP_NOACTIVATE | SWP_NOZORDER );		
		tTop += tHeight[ i ];
	}

	mGraphicList.SetColumnWidth( 0, 200 );

	if ( tAuto )
	{
		return;
	}

	if ( mGraphicListType == GRAPHIC_ICON )
	{
		mGraphicIcon.ShowWindow( SW_SHOW );	
		mGraphicList.ShowWindow( SW_HIDE );
		HidePropertyWndExcept();
	}
	else if ( mGraphicListType == GRAPHIC_LIST )
	{
		mGraphicIcon.ShowWindow( SW_HIDE );	
		mGraphicList.ShowWindow( SW_SHOW );
		HidePropertyWndExcept();
	}
	else
	{
		mGraphicIcon.ShowWindow( SW_HIDE );	
		mGraphicList.ShowWindow( SW_HIDE );
		HidePropertyWndExcept( pGraphicListTypeWnd );
	}
}

HSVoid CGraghicWnd::HidePropertyWndExcept( CWnd * pWnd )
{
	map< string, IGraphicPropertyProtocol * >::iterator pIterator = mGraphicProperties.begin();
	while ( pIterator != mGraphicProperties.end() )
	{
		if ( pIterator->second->Wnd() != pWnd )
		{
			pIterator->second->Wnd()->ShowWindow( SW_HIDE );
		}
		else
		{
			pIterator->second->Wnd()->ShowWindow( SW_SHOW );
			pIterator->second->RefreshDataSource();
		}

		pIterator++;
	}
}

HSVoid CGraghicWnd::RefreshPropertyDataSource()
{
	map< string, IGraphicPropertyProtocol * >::iterator pIterator = mGraphicProperties.begin();
	while ( pIterator != mGraphicProperties.end() )
	{					
		pIterator->second->RefreshDataSource();
		pIterator++;
	}
}

int CGraghicWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;	

	CRect tRect( 0, 0, 0, 0 );
	mGraphicIcon.Create( WS_CHILD | /*WS_VISIBLE | */WS_BORDER | LVS_ICON | LVS_AUTOARRANGE | LVS_SHOWSELALWAYS, tRect, this, 1 );
	mGraphicIcon.SetParent( this );

	mGraphicList.Create( WS_CHILD | /*WS_VISIBLE | */WS_BORDER | LVS_REPORT | LVS_AUTOARRANGE | LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS, tRect, this, 1 );
	mGraphicList.SetParent( this );

	if (!mWndPropList.Create( WS_VISIBLE | WS_CHILD, tRect, this, 2 ) )
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}

	CGraphicManager *pGraphicManager = CGraphicManager::SharedInstance();

	mGraphicIconImageList.Create( 64, 45, ILC_COLOR24 | ILC_MASK, 0, pGraphicManager->Count() );
	mGraphicIcon.SetImageList( &mGraphicIconImageList, LVSIL_NORMAL );

	mGraphicListImageList.Create( 32, 24, ILC_COLOR24 | ILC_MASK, 0, pGraphicManager->Count() );
	mGraphicList.SetImageList( &mGraphicListImageList, LVSIL_SMALL );
	mGraphicList.InsertColumn( 0, "", 0 );	

	IGraphicProtocol *pGraphic = NULL;
	HSInt tIndex = 0;
	HSInt tIconIndex = 0;
	while ( ( pGraphic = pGraphicManager->GraphicWithIndex( tIndex, HSTrue ) ) != NULL )
	{
		if ( tIndex != 0 && !theApp.License()->GraphicEnabled( pGraphic->Identifier().c_str() ) )
		{
			tIndex++;
			continue;
		}

		CBitmap tLargeIcon;
		tLargeIcon.LoadBitmap( pGraphic->LargeIcon() );
		mGraphicIconImageList.Add( &tLargeIcon, RGB( 100, 100, 100 ) );
		mGraphicIcon.InsertItem( tIconIndex, pGraphic->Name().c_str(), tIconIndex );

		CBitmap tSmallIcon;
		tSmallIcon.LoadBitmap( pGraphic->SmallIcon() );
		mGraphicListImageList.Add( &tSmallIcon, RGB( 100, 100, 100 ) );
		mGraphicList.InsertItem( tIconIndex, pGraphic->Name().c_str(), tIconIndex );
		tIconIndex++;

		mGrahpicIdentifiers.push_back( pGraphic->Identifier() );

		IGraphicPropertyProtocol *pDlg = pGraphic->CreatePropertyDlg( this );
		if ( pDlg != NULL )
		{			
			mGraphicProperties[ pGraphic->Identifier() ] = pDlg;
		}

		tIndex++;
	}
	
	CFont tFont;
	tFont.CreateFont( 24, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T( "宋体" ) ); 
	if ( theApp.License()->ShowUnBuyItems() )
	{
		tIndex = 1;
		while ( ( pGraphic = pGraphicManager->GraphicWithIndex( tIndex, HSTrue ) ) != NULL )
		{
			if ( theApp.License()->GraphicEnabled( pGraphic->Identifier().c_str() ) || !theApp.License()->IsNormalGraphic( pGraphic->Identifier().c_str() ) )
			{
				tIndex++;
				continue;
			}

			CBitmap tLargeIcon;
			tLargeIcon.LoadBitmap( pGraphic->LargeIcon() );

			CDC tMemDC;
			tMemDC.CreateCompatibleDC( this->GetDC() );
			CBitmap *pOldBitmap = tMemDC.SelectObject( &tLargeIcon );
			tMemDC.SetBkMode( TRANSPARENT );
			tMemDC.SetTextColor( RGB( 0, 0, 255 ) );
			tMemDC.SelectObject( &tFont );
			tMemDC.TextOutA( 40, 22, "￥" );
			tMemDC.SelectObject( pOldBitmap );

			mGraphicIconImageList.Add( &tLargeIcon, RGB( 100, 100, 100 ) );
			mGraphicIcon.InsertItem( tIconIndex, pGraphic->Name().c_str(), tIconIndex );			

			CBitmap tSmallIcon;
			tSmallIcon.LoadBitmap( pGraphic->SmallIcon() );
			mGraphicListImageList.Add( &tSmallIcon, RGB( 100, 100, 100 ) );
			mGraphicList.InsertItem( tIconIndex, pGraphic->Name().c_str(), tIconIndex );
			tIconIndex++;

			mGrahpicIdentifiers.push_back( pGraphic->Identifier() );		

			IGraphicPropertyProtocol *pDlg = pGraphic->CreatePropertyDlg( this );
			if ( pDlg != NULL )
			{
				mGraphicProperties[ pGraphic->Identifier() ] = pDlg;
			}

			tIndex++;
		}
	}

	InitPropList();

	mFileToolBar.Create( this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_FILE_ORDER );
	mFileToolBar.LoadToolBar( IDR_FILE_ORDER, 0, 0, TRUE /* Is locked */);
	mFileToolBar.CleanUpLockedImages();
	mFileToolBar.LoadBitmap( theApp.m_bHiColorIcons ? IDB_FILE_ORDER_HC : IDR_FILE_ORDER, 0, 0, TRUE /* Locked */);

	mFileToolBar.SetPaneStyle(mFileToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	mFileToolBar.SetPaneStyle(mFileToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	mFileToolBar.SetOwner(this);

	mGraphicToolBar.Create( this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_GRAPHIC_ICON_LIST );
	mGraphicToolBar.LoadToolBar( IDR_GRAPHIC_ICON_LIST, 0, 0, TRUE );
	mGraphicToolBar.CleanUpLockedImages();
	mGraphicToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_GRAPHIC_ICON_LIST_HC : IDR_GRAPHIC_ICON_LIST, 0, 0, TRUE );

	mGraphicToolBar.SetPaneStyle(mGraphicToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	mGraphicToolBar.SetPaneStyle(mGraphicToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	mGraphicToolBar.SetOwner(this);


	// All commands will be routed via this control , not via the parent frame:
	mFileToolBar.SetRouteCommandsViaFrame(FALSE);
	mGraphicToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout( false );

	return 0;
}

void CGraghicWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);	

	AdjustLayout();	
	
//	mGraphicIcon.Invalidate();
}

void CGraghicWnd::OnSortProperties()
{
	mWndPropList.SetAlphabeticMode(!mWndPropList.IsAlphabeticMode());
}

void CGraghicWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(mWndPropList.IsAlphabeticMode());
}

void CGraghicWnd::InitPropList()
{
	SetPropListFont();

	mWndPropList.EnableHeaderCtrl(FALSE);
	mWndPropList.EnableDescriptionArea();
	mWndPropList.SetVSDotNetLook();
	mWndPropList.MarkModifiedProperties();		

	CMFCPropertyGridProperty* pGroupDataFile = new CMFCPropertyGridProperty(_T("数据文件"));

	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridFileProperty( _T("路径"), FALSE, mDefaultFilePath, "pxd", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "pxd Files (*.pxd)|*.pxd||", NULL, PROPERTY_GRID_CTRL_FILE_NAME );
	pProp->AllowEdit( FALSE );
	pGroupDataFile->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("大小"), (_variant_t)"0", NULL, PROPERTY_GRID_CTRL_FILE_SIZE );
	pProp->AllowEdit( FALSE );
	pGroupDataFile->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("分割数据文件"),  _T( "否" ), _T( "将数据保存到不同的文件!" ), PROPERTY_GRID_CTRL_SPLIT_FILE );
	pProp->AddOption( _T( "是" ) );
	pProp->AddOption( _T( "否" ) );
	pProp->AllowEdit( FALSE );
	pGroupDataFile->AddSubItem( pProp );
	
	pProp = new CMFCPropertyGridProperty(_T("数据文件大小( 10M )"), ( _variant_t )5, _T( "单个数据文件大小!"), PROPERTY_GRID_CTRL_FILE_LIMIT_SIZE );
	pProp->EnableSpinControl( TRUE, 1, 100 );
	pGroupDataFile->AddSubItem( pProp );

	mWndPropList.AddProperty( pGroupDataFile );	
}

void CGraghicWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	mWndPropList.SetFocus();
}

void CGraghicWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CGraghicWnd::SetPropListFont()
{
	::DeleteObject(mFntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	mFntPropList.CreateFontIndirect(&lf);

	mWndPropList.SetFont( &mFntPropList );
	mGraphicIcon.SetFont( &mFntPropList );
	mGraphicList.SetFont( &mFntPropList );
}


void CGraghicWnd::OnListIcon()
{
	// TODO: Add your command handler code here
	mGraphicListType = GRAPHIC_ICON;

	AdjustLayout( false );			
}

void CGraghicWnd::OnUpdateListIcon(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( mGraphicListType == GRAPHIC_ICON );
}

void CGraghicWnd::OnContextMenu(CWnd* pWnd, CPoint point)
{
}

void CGraghicWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default		
	int nItem = 0;
	UINT uFlags;
	if ( mGraphicListType == GRAPHIC_LIST )
	{
		nItem = mGraphicList.HitTest( point, &uFlags );
	}
	else if ( mGraphicListType == GRAPHIC_ICON )
	{
		nItem = mGraphicIcon.HitTest( point, &uFlags );
	}	
	else
	{
		nItem = -1;
	}

	if ( nItem >= 0 )
	{						
		if ( mGraphicListType == GRAPHIC_LIST )
		{
			mGraphicList.SetItemState( mListPrevSelectItem, ~LVNI_SELECTED, LVNI_SELECTED );
			mGraphicList.SetItemState( nItem, LVNI_SELECTED, LVNI_SELECTED );		
			mGraphicList.SetFocus();
			mListPrevSelectItem = nItem;
		}
		else
		{
			mGraphicIcon.SetItemState( mIconPrevSelectItem, ~LVNI_SELECTED, LVNI_SELECTED );
			mGraphicIcon.SetItemState( nItem, LVNI_SELECTED, LVNI_SELECTED );
			mGraphicIcon.SetFocus();
			mIconPrevSelectItem = nItem;
		}				

		if ( nItem == 0 )
		{
			return;
		}

		CGraphicManager *pGraphicManager = CGraphicManager::SharedInstance();
		if ( pGraphicManager->GraphicWithIdentifier( mGrahpicIdentifiers[ nItem ] ) == NULL )
		{
			return;
		}

		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
		pMainFrame->SetSelectedGraphic( mGrahpicIdentifiers[ nItem ] );
		pMainFrame->SetDragingGraphic( mGrahpicIdentifiers[ nItem ] );	
	}
	else
	{
		CDockablePane::OnLButtonDown(nFlags, point);
	}	
}

void CGraghicWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	int nItem = 0;
	UINT uFlags;
	if ( mGraphicListType == GRAPHIC_LIST )
	{
		nItem = mGraphicList.HitTest( point, &uFlags );
	}
	else if ( mGraphicListType == GRAPHIC_ICON )
	{
		nItem = mGraphicIcon.HitTest( point, &uFlags );
	}	
	else
	{
		nItem = -1;
	}

	CGraphicManager *pGraphicManager = CGraphicManager::SharedInstance();
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( nItem < 1 || ( nItem > 0 && pGraphicManager->GraphicWithIdentifier( mGrahpicIdentifiers[ nItem ] ) == NULL ) )
	{	
		pMainFrame->SetSelectedGraphic( "" );
	}

	pMainFrame->SetDragingGraphic( "" );

	CDockablePane::OnLButtonUp(nFlags, point);
}

LRESULT CGraghicWnd::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{
	CMFCPropertyGridProperty * pProperty = ( CMFCPropertyGridProperty * ) lparam;
	if ( pProperty->GetData() == PROPERTY_GRID_CTRL_FILE_NAME )
	{		
		theApp.DataFileResetIndex();		
	}

	return 0;
}

CString CGraghicWnd::FilePath()
{	
	return mWndPropList.FindItemByData( PROPERTY_GRID_CTRL_FILE_NAME )->GetValue();
}

void CGraghicWnd::SetFilePath( CString tFilePath )
{
	if ( tFilePath == "" )
	{
		tFilePath = mDefaultFilePath;
	}

	mWndPropList.FindItemByData( PROPERTY_GRID_CTRL_FILE_NAME )->SetValue( tFilePath );
}

HSVoid CGraghicWnd::UpdateFileSize( double tSize )
{	
	CMFCPropertyGridProperty *pProp = mWndPropList.FindItemByData( PROPERTY_GRID_CTRL_FILE_SIZE );
	pProp->SetValue( GetStrSize( tSize ).c_str() );
	pProp->Redraw();	
}

HSVoid CGraghicWnd::EnableFileInfo( HSBool tEnable )
{
	CMFCPropertyGridProperty *pProp = mWndPropList.FindItemByData( PROPERTY_GRID_CTRL_FILE_NAME );
	pProp->Enable( tEnable );	

	pProp = mWndPropList.FindItemByData( PROPERTY_GRID_CTRL_SPLIT_FILE );
	pProp->Enable( tEnable );		

	pProp = mWndPropList.FindItemByData( PROPERTY_GRID_CTRL_FILE_LIMIT_SIZE );
	pProp->Enable( tEnable );		
}

string CGraghicWnd::GetStrSize( double tSize )
{
    if ( tSize < 0.000001 )
    {
        return "0";
    }

    unsigned int tIndex = 0;
    string tStrUnit[] = { " KB", " MB", " GB"};
    while ( tIndex < sizeof( tStrUnit ) / sizeof( string ) )
    {
        tIndex++;
        tSize /= 1024;
        if ( tSize < 1000 )
        {
            break;
        }
    }

    tSize = ( int )( tSize * 10 );
    tSize /= 10;

    std::ostringstream tStream;
    tStream << tSize << tStrUnit[ tIndex - 1 ];

    return tStream.str();
}

HSInt64 CGraghicWnd::GetDisperseFileLimit()
{
	CMFCPropertyGridProperty *pProp  = mWndPropList.FindItemByData( PROPERTY_GRID_CTRL_SPLIT_FILE );
	CString tStrSplitFile = pProp->GetValue();
	if ( tStrSplitFile == _T( "否" ) )
	{
		return 0;
	}	

	pProp = mWndPropList.FindItemByData( PROPERTY_GRID_CTRL_FILE_LIMIT_SIZE );
	CString tStrValue = pProp->GetValue();
	HSInt64 tValue = max( 1, atoi( ( LPCSTR )tStrValue ) );
	tValue *= ( 1024 * 1024 * 10 );	
	return tValue;
}

HSVoid CGraghicWnd::SetDisperseFileLimit( HSInt64 tSize )
{	
	CMFCPropertyGridProperty *pProp = mWndPropList.FindItemByData( PROPERTY_GRID_CTRL_SPLIT_FILE );	
	pProp->SetValue( tSize > 0 ? _T( "是" ) : _T( "否" ) );
	pProp->Redraw();		
	
	if ( tSize > 0 )
	{	
		pProp = mWndPropList.FindItemByData( PROPERTY_GRID_CTRL_FILE_LIMIT_SIZE );
		HSInt tTenMSize = ( HSInt )( tSize / ( 1024 * 1024 * 10 ) );	
		pProp->SetValue( ( _variant_t )tTenMSize );
		pProp->Redraw();	
	}
}

void CGraghicWnd::OnIconProperties()
{
	// TODO: Add your command handler code here
	mGraphicListType = GRAPHIC_PROPERTY;
	AdjustLayout( false );	
}


void CGraghicWnd::OnUpdateIconProperties(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( mGraphicListType == GRAPHIC_PROPERTY );
}


void CGraghicWnd::OnListIconlist()
{
	// TODO: Add your command handler code here
	mGraphicListType = GRAPHIC_LIST;
	AdjustLayout( false );	
}


void CGraghicWnd::OnUpdateListIconlist(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( mGraphicListType == GRAPHIC_LIST );
}

HSVoid CGraghicWnd::SetGrahpicListType( CGraghicWnd::GRAPHIC_VIEW_TYPE tGraphicListType )
{	
	mGraphicListType = tGraphicListType;
	AdjustLayout( false );		
}

HSVoid CGraghicWnd::SetFocusedGraphic( IGraphicProtocol *pFocusedGrahpic, HSBool tForceRefresh )
{	
	if ( mFocusedGrahpic == pFocusedGrahpic && !tForceRefresh && mGraphicListType == GRAPHIC_PROPERTY )
	{
		return;
	}	

	mFocusedGrahpic = pFocusedGrahpic;
	if ( mFocusedGrahpic == NULL )
	{
		SetGrahpicListType( GRAPHIC_ICON );
	}
	else
	{
		SetGrahpicListType( GRAPHIC_PROPERTY );
	}
}