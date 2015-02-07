// ARB1410SettingView.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "ARB1410SettingView.h"
#include "ARB1410-Handler.h"
#include "MainFrm.h"
#include "ARB1410SettingDoc.h"


// CARB1410SettingView

CString CARB1410SettingView::mStrRate[ 9 ] = { "100k", "200k", "500k", "1M", "2M", "5M", "10M", "20M", "40M" };
CString CARB1410SettingView::mStrRiseRate[ 9 ] = { "100k", "200k", "500k", "1M", "2M", "5M", "10M", "20M", "40M" };
CString CARB1410SettingView::mStrreRate[ 9 ] = { "100k", "200k", "500k", "1M", "2M", "5M", "10M", "20M", "40M" };
CString CARB1410SettingView::mStartFrequency[ 4 ] = { "1k", "20k", "100k", "200k" };
CString CARB1410SettingView::mStrEndFrequency[ 6 ] = { "3M", "100k", "200k", "400k", "1M", "2M" };

IMPLEMENT_DYNCREATE(CARB1410SettingView, CFormView)

const int WM_VALUE_INCORRCET_MESSAGE = WM_USER + 7;

CARB1410SettingView::CARB1410SettingView()
	: CFormView(CARB1410SettingView::IDD), mAutoAlignManager( this, CRect( 0, 0, 100, 100 ) )
{

}

CARB1410SettingView::~CARB1410SettingView()
{
}

void CARB1410SettingView::SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier )
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	mCardIdentifier = tCardIdentifier; 

	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( tCardIdentifier );	
	CARB1410Handler *pArb1410Handler = dynamic_cast< CARB1410Handler * >( pDataHandler );	

	this->mPropertyGrid.RemoveAll();
	this->mPropertyGrid.EnableHeaderCtrl(FALSE);
	this->mPropertyGrid.EnableDescriptionArea();
	this->mPropertyGrid.SetVSDotNetLook();
	this->mPropertyGrid.MarkModifiedProperties();

	CString str;

	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("Wave Gen 波形设置"));	

	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("采样率"), _T( "" ), _T( "选择采样率!" ), IDD_ARB1410SETTINGVIEW );

	for ( int j = 0; j < sizeof( mStrRate ) / sizeof( CString ); j++ )
	{
		pProp->AddOption( mStrRate[ j ] );
	}
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tAMwaveform.frequency );
	pProp->SetValue(str );
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );	

	pProp = new CMFCPropertyGridProperty(_T("载波类型"), _T( "方波" ), _T( "设置载波的类型" ), PROPERTY_ARB1410_CARRIERTYPE );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tAMwaveform.carrierType );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("包络线的类型"), _T( "正玄波" ), _T( "设置包络线的类型" ), PROPERTY_ARB1410_ENVELOPE );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tAMwaveform.envelope );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("持续时间单位"), _T( "微妙" ), _T( "设置持续时间的单位" ), PROPERTY_ARB1410_DURATIONUNITS );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tAMwaveform.durationUnits );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("持续时间"), _T( "10" ), _T( "设置持续时间" ), PROPERTY_ARB1410_DURATION );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tAMwaveform.duration );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("开始采样率"), _T( "" ), _T( "选择开始采样率!" ), PROPERTY_ARB1410_STARTFREQUENCY );

	for ( int j = 0; j < sizeof( mStartFrequency ) / sizeof( CString ); j++ )
	{
		pProp->AddOption( mStartFrequency[ j ] );
	}
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tFrequencySweep.startFrequency );
	pProp->SetValue( str );
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("结束采样率"), _T( "" ), _T( "选择结束采样率!" ), PROPERTY_ARB1410_ENDFREQUENCY );

	for ( int j = 0; j < sizeof( mStrEndFrequency ) / sizeof( CString ); j++ )
	{
		pProp->AddOption( mStrEndFrequency[ j ] );
	}
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tFrequencySweep.endFrequency );
	pProp->SetValue( str );
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("频率的持续时间"), _T( "10" ), _T( "设置频率持续时间" ), PROPERTY_ARB1410_FREDURATION );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tFrequencySweep.duration );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("触发前循环的时间"), _T( "100" ), _T( "设置触发前持续时间" ), PROPERTY_ARB1410_PRECYCLE );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tFrequencySweep.minPtsPerCycle );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );


	pProp = new CMFCPropertyGridProperty(_T("上升采样率"), _T( "" ), _T( "选择上升采样率!" ), PROPERTY_ARB1410_AEFREQUENCY );

	for ( int j = 0; j < sizeof( mStrRiseRate ) / sizeof( CString ); j++ )
	{
		pProp->AddOption( mStrRiseRate[ j ] );
	}
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tAEwaveform.frequency );
	pProp->SetValue( str );
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("上升时间"), _T( "10" ), _T( "设置上升时间" ), PROPERTY_ARB1410_RISERATE );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tAEwaveform.riseTime );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("下降时间"), _T( "100" ), _T( "设置下降时间" ), PROPERTY_ARB1410_FALLRATE );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tAEwaveform.fallTime );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("耦合"), _T( "0" ), _T( "设置耦合" ), PROPERTY_ARB1410_COUPLING );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tFrequencySweep.minPtsPerCycle );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("振幅"), _T( "100000" ), _T( "设置振幅" ), PROPERTY_ARB1410_AMPLITUDE );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tSignal.coupling );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("偏移"), _T( "0" ), _T( "设置偏移" ), PROPERTY_ARB1410_DCOFFSET );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tSignal.DCoffset );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("过滤频率"), _T( "2" ), _T( "设置过滤频率" ), PROPERTY_ARB1410_FILTER );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tSignal.filter );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("延时"), _T( "0" ), _T( "设置延时" ), PROPERTY_ARB1410_DELAY );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tSignal.delay );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("模式"), _T( "1" ), _T( "设置模式" ), PROPERTY_ARB1410_MODE );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tTrigger.mode );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("重复采样率"), _T( "" ), _T( "选择重复采样率!" ), PROPERTY_ARB1410_RERATE );

	for ( int j = 0; j < sizeof( mStrreRate ) / sizeof( CString ); j++ )
	{
		pProp->AddOption( mStrreRate[ j ] );
	}
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tTrigger.repRate );
	pProp->SetValue( str );
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("二次曲线"), _T( "0" ), _T( "设置二次曲线" ), PROPERTY_ARB1410_OUTPOLARITY );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tFrequencySweep.minPtsPerCycle );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("波形宽度"), _T( "10" ), _T( "设置波形宽度" ), PROPERTY_ARB1410_OUTWIDTH );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tTrigger.outPolarity );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("输入类型"), _T( "0" ), _T( "设置输入类型" ), PROPERTY_ARB1410_INTYPE );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tTrigger.inType );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("输入斜率"), _T( "0" ), _T( "设置输入斜率" ), PROPERTY_ARB1410_INSLOPE );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.tTrigger.inSlope );
	pProp->SetValue( str );
	pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	pProp = new CMFCPropertyGridProperty(_T("波形类型"), _T( "0" ), _T( "设置波形类型" ), PROPERTY_ARB1410_WAVEFROM_TYPE );
	str.Empty();
	str.Format("%d", pArb1410Handler->mARB1410_SETINFO.WaveformType );
	pProp->SetValue( str );pProp->AllowEdit( true );	
	pGroup->AddSubItem( pProp );

	this->mPropertyGrid.AddProperty( pGroup );

	CString tTitle;
	tTitle.Format( "%s 设置", pDataHandler->Name().c_str() );

	CARB1410SettingDoc *pDoc = dynamic_cast< CARB1410SettingDoc * >( this->GetDocument() );
	pDoc->SetCardIdentifier( mCardIdentifier, tTitle );

}

void CARB1410SettingView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID1, mPropertyGrid);
}

BEGIN_MESSAGE_MAP(CARB1410SettingView, CFormView)
	ON_MESSAGE( WM_VALUE_INCORRCET_MESSAGE, OnValueIncorrect )
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_OK, &CARB1410SettingView::OnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CARB1410SettingView::OnClickedBtnCancel)
	ON_WM_LBUTTONUP()
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
END_MESSAGE_MAP()


// CARB1410SettingView diagnostics

#ifdef _DEBUG
void CARB1410SettingView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CARB1410SettingView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CARB1410SettingView message handlers


void CARB1410SettingView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class

	mAutoAlignManager.AddControl( IDC_BTN_OK, CRect( -1, -1, 10, 15 ) );
	mAutoAlignManager.AddControl( IDC_BTN_CANCEL, CRect( -1, -1, 100, 15 ) );
	this->mAutoAlignManager.AddControl( IDC_MFCPROPERTYGRID1, CRect(0, 0, 0, 100 ));

}


void CARB1410SettingView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	mAutoAlignManager.Resize();
}

void CARB1410SettingView::OnClickedBtnOk()
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST( CMainFrame, GetTopLevelFrame() );
	IDataHandler* pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( this->mCardIdentifier );
	CARB1410Handler* pCARB1410Handler = dynamic_cast< CARB1410Handler *> ( pDataHandler ) ;

	CString tStr;
	CMFCPropertyGridProperty *pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_RATE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr);
		Istring >>  ( pCARB1410Handler->mARB1410_SETINFO.tAMwaveform.frequency );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_CARRIERTYPE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tAMwaveform.carrierType );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_ENVELOPE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tAMwaveform.envelope );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_DURATIONUNITS );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tAMwaveform.durationUnits ) ;
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_DURATION );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tAMwaveform.duration ) ;
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_STARTFREQUENCY );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tFrequencySweep.startFrequency );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_ENDFREQUENCY );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tFrequencySweep.endFrequency );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_FREDURATION );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tFrequencySweep.duration ) ;
	}


	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_PRECYCLE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tFrequencySweep.minPtsPerCycle );
	}


	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_AEFREQUENCY );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tAEwaveform.frequency );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_RISERATE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tAEwaveform.riseTime );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_FALLRATE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tAEwaveform.fallTime );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_COUPLING );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tSignal.coupling );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_AMPLITUDE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tSignal.amplitude );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_DCOFFSET );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tSignal.DCoffset );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_FILTER );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tSignal.filter );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_DELAY );
	tStr = pProp->GetValue();
	std::istringstream Istring( ( LPCSTR )tStr );
	Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tSignal.delay );

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_MODE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tTrigger.mode );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_RERATE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tTrigger.repRate );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_OUTPOLARITY );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tTrigger.outPolarity );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_OUTWIDTH );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tTrigger.outWidth );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_INTYPE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tTrigger.inType );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_INSLOPE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.tTrigger.inSlope );
	}

	pProp = this->mPropertyGrid.FindItemByData( PROPERTY_ARB1410_WAVEFROM_TYPE );
	if ( pProp != NULL )
	{
		tStr = pProp->GetValue();
		std::istringstream Istring( ( LPCSTR )tStr );
		Istring >> ( pCARB1410Handler->mARB1410_SETINFO.WaveformType );
	}
}
void CARB1410SettingView::OnClickedBtnCancel()
{
	// TODO: Add your control notification handler code here

	this->GetDocument()->OnCloseDocument();
}


void CARB1410SettingView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( pMainFrame->DragingGraphic() != "" )
	{		
		pMainFrame->SetSelectedGraphic( "" );
		pMainFrame->SetDragingGraphic( "" );
	}

	__super::OnLButtonUp(nFlags, point);
}

BOOL CARB1410SettingView::DigcheckOnly( CString tStr )
{
	if ( tStr.GetLength() < 0 )
	{
		return 0;
	}

	for ( int i = 0; i < tStr.GetLength(); i++ )
	{
		if ( tStr[ i ] < '0' || tStr[ i ] > '9' )
		{
			return 0;
		}
	}

	return 1;
}

LRESULT CARB1410SettingView::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{
	CMFCPropertyGridProperty *pProp = ( CMFCPropertyGridProperty *)lparam;
	HSInt tARB1410SETDATA = pProp->GetData();
	CString tStr;
	tStr = pProp->GetValue();
	
	if ( DigcheckOnly( tStr ) == 0 )
	{
		this->mIncorrectMsg = "字符不匹配";

		this->PostMessageA( WM_VALUE_INCORRCET_MESSAGE, 0, 0 );

		return 1;
	}

	switch ( tARB1410SETDATA )
	{
		case PROPERTY_ARB1410_RATE:
			this->mIncorrectMsg = "PROPERTY_ARB1410_RATE";
			break;

		case PROPERTY_ARB1410_CARRIERTYPE:
			this->mIncorrectMsg = "PROPERTY_ARB1410_CARRIERTYPE";
			break;

		case PROPERTY_ARB1410_ENVELOPE:
			this->mIncorrectMsg = "PROPERTY_ARB1410_ENVELOPE";
			break;

		case PROPERTY_ARB1410_DURATIONUNITS:
			this->mIncorrectMsg = "PROPERTY_ARB1410_DURATIONUNITS";
			break;

		case PROPERTY_ARB1410_DURATION:
			this->mIncorrectMsg = "PROPERTY_ARB1410_DURATION";
			break;

		case PROPERTY_ARB1410_STARTFREQUENCY:
			this->mIncorrectMsg = "PROPERTY_ARB1410_STARTFREQUENCY";
			break;

		case PROPERTY_ARB1410_ENDFREQUENCY:
			this->mIncorrectMsg = "PROPERTY_ARB1410_ENDFREQUENCY";
			break;

		case PROPERTY_ARB1410_FREDURATION:
			this->mIncorrectMsg = "PROPERTY_ARB1410_FREDURATION";
			break;
		case PROPERTY_ARB1410_PRECYCLE:
			this->mIncorrectMsg = "PROPERTY_ARB1410_PRECYCLE";
			break;

		case PROPERTY_ARB1410_AEFREQUENCY:
			this->mIncorrectMsg = "PROPERTY_ARB1410_AEFREQUENCY";
			break;

		case PROPERTY_ARB1410_RISERATE:
			this->mIncorrectMsg = "PROPERTY_ARB1410_RISERATE";
			break;
		case PROPERTY_ARB1410_FALLRATE:
			this->mIncorrectMsg = "PROPERTY_ARB1410_FALLRATE";
			break;

		case PROPERTY_ARB1410_COUPLING:
			this->mIncorrectMsg = "PROPERTY_ARB1410_COUPLING";
			break;

		case PROPERTY_ARB1410_AMPLITUDE:
			this->mIncorrectMsg = "PROPERTY_ARB1410_AMPLITUDE";
			break;

		case PROPERTY_ARB1410_DCOFFSET:
			this->mIncorrectMsg = "PROPERTY_ARB1410_DCOFFSET";
			break;

		case PROPERTY_ARB1410_FILTER:
			this->mIncorrectMsg = "PROPERTY_ARB1410_FILTER";
			break;

		case PROPERTY_ARB1410_DELAY:
			this->mIncorrectMsg = "PROPERTY_ARB1410_DELAY";
			break;

		case PROPERTY_ARB1410_MODE:
			this->mIncorrectMsg = "PROPERTY_ARB1410_MODE";
			break;

		case PROPERTY_ARB1410_RERATE:
			this->mIncorrectMsg = "PROPERTY_ARB1410_RERATE";
			break;

		case PROPERTY_ARB1410_OUTPOLARITY:
			this->mIncorrectMsg = "PROPERTY_ARB1410_OUTPOLARITY";
			break;

		case PROPERTY_ARB1410_INTYPE:
			this->mIncorrectMsg = "PROPERTY_ARB1410_INTYPE";
			break;

		case PROPERTY_ARB1410_INSLOPE:
			this->mIncorrectMsg = "PROPERTY_ARB1410_INSLOPE";
			break;

		case PROPERTY_ARB1410_WAVEFROM_TYPE:
			this->mIncorrectMsg = "PROPERTY_ARB1410_WAVEFROM_TYPE";
			break;

		default:
			break;
	}

	if ( this->mIncorrectMsg == "" )
	{
		return 0;
	}

	this->PostMessageA( WM_VALUE_INCORRCET_MESSAGE, 0, 0 );

	return 1;
}

LRESULT CARB1410SettingView::OnValueIncorrect( WPARAM wparam, LPARAM lparam )
{
	AfxMessageBox( mIncorrectMsg );

	return 0;
}