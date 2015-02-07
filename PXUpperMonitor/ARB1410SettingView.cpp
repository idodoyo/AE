// ARB1410SettingView.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "ARB1410SettingView.h"
#include "ARB1410-Handler.h"
#include "MainFrm.h"
#include "ARB1410SettingDoc.h"


// CARB1410SettingView

IMPLEMENT_DYNCREATE(CARB1410SettingView, CFormView)

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
	
	CARB1410Handler::FREQUENCY_MOD *pFrequencyMod = pArb1410Handler->GetFrequcyModulate();

	CString tStr;
	tStr.Format( "%d", pFrequencyMod->StartFrequency / 1000 );
	this->mBeginRate.SetWindowTextA( tStr ) ;

	tStr.Format( "%d", pFrequencyMod->EndFrequency / 1000 );
	this->mEndRate.SetWindowTextA( tStr );

	tStr.Format( "%d", pFrequencyMod->Duration / 1000 );
	this->mDuration.SetWindowTextA( tStr );

	tStr.Format( "%d", pFrequencyMod->MinPtsPerCycle );
	this->mCycleCount.SetWindowTextA( tStr );

	this->mRateStep.SetCheck( 1 );
	

	CARB1410Handler::SET_SIGNAL *pSignalSet = pArb1410Handler->GetSignalSet();	
	
	::CheckRadioButton( this->GetSafeHwnd(), IDC_RADIO_AC, IDC_RADIO_DC, pSignalSet->Coupling == 0 ? IDC_RADIO_AC : IDC_RADIO_DC );	

	tStr.Format( "%d", pSignalSet->Amplitude / 1000 );
	this->mMinAmplitude.SetWindowTextA( tStr );

	tStr.Format( "%d", pSignalSet->DCoffset );
	this->mSignalOffset.SetWindowTextA( tStr );
	
	this->mBypassRate.SetCurSel( pSignalSet->Filter );

	tStr.Format( "%d", pSignalSet->Delay );
	this->mSignalLate.SetWindowTextA( tStr );
	
	CARB1410Handler::SET_TRIGGER *pSetTrigger = pArb1410Handler->GetTriggerSet();	

	HSInt tTriggerIDs[] = { IDC_RADIO_TRIGGER_CONTINUE, IDC_RADIO_TRIGGER_REPEAT, IDC_RADIO_TRIGGER_STEP, IDC_RADIO_TRIGGER_OUTSIDE };
	::CheckRadioButton( this->GetSafeHwnd(), tTriggerIDs[ 0 ], tTriggerIDs[ 3 ], tTriggerIDs[ pSetTrigger->Mode ] );


	CString tTitle;
	tTitle.Format( "%s 设置", pDataHandler->Name().c_str() );

	CARB1410SettingDoc *pDoc = dynamic_cast< CARB1410SettingDoc * >( this->GetDocument() );
	pDoc->SetCardIdentifier( mCardIdentifier, tTitle );	

	mInitValue = HSTrue;
}

void CARB1410SettingView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_START_RATE, mBeginRate);
	DDX_Control(pDX, IDC_EDIT_END_RATE, mEndRate);
	DDX_Control(pDX, IDC_EDIT_DURATION, mDuration);
	DDX_Control(pDX, IDC_EDIT_CYCLE_COUNT, mCycleCount);
	DDX_Control(pDX, IDC_COMBO_BYPASS_RATE, mBypassRate);
	DDX_Control(pDX, IDC_EDIT_LATE, mSignalLate);
	DDX_Control(pDX, IDC_EDIT_MIN_AMPLITUDE, mMinAmplitude);
	DDX_Control(pDX, IDC_EDIT_OFFSET, mSignalOffset);
	DDX_Control(pDX, IDC_RADIO_AC, mAC);
	DDX_Control(pDX, IDC_RADIO_STEP, mRateStep);
	DDX_Control(pDX, IDC_RADIO_TRIGGER_CONTINUE, mTriggerContinue);
}

BEGIN_MESSAGE_MAP(CARB1410SettingView, CFormView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_OK, &CARB1410SettingView::OnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CARB1410SettingView::OnClickedBtnCancel)
	ON_WM_LBUTTONUP()
	ON_EN_KILLFOCUS(IDC_EDIT_START_RATE, &CARB1410SettingView::OnEnKillfocusEditStartRate)
	ON_EN_KILLFOCUS(IDC_EDIT_END_RATE, &CARB1410SettingView::OnEnKillfocusEditEndRate)
	ON_EN_KILLFOCUS(IDC_EDIT_DURATION, &CARB1410SettingView::OnEnKillfocusEditDuration)
	ON_EN_KILLFOCUS(IDC_EDIT_CYCLE_COUNT, &CARB1410SettingView::OnEnKillfocusEditCycleCount)
	ON_EN_KILLFOCUS(IDC_EDIT_MIN_AMPLITUDE, &CARB1410SettingView::OnEnKillfocusEditMinAmplitude)
	ON_EN_KILLFOCUS(IDC_EDIT_OFFSET, &CARB1410SettingView::OnEnKillfocusEditOffset)
	ON_EN_KILLFOCUS(IDC_EDIT_LATE, &CARB1410SettingView::OnEnKillfocusEditLate)
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

	mAutoAlignManager.AddControl( IDC_BTN_OK, CRect( -1, -1, 100, 15 ) );
	mAutoAlignManager.AddControl( IDC_BTN_CANCEL, CRect( -1, -1, 10, 15 ) );
	//mAutoAlignManager.AddControl( IDC_MFCPROPERTYGRID_PCI2, CRect( 0, 0, 0, 100 ) );

	CString tStrBypassRate[] = { "10kHz", "50kHz", "100kHz", "500kHz", "1M", "5M", "10M", "40M" };
	for ( HSInt i = 0; i < sizeof( tStrBypassRate ) / sizeof( CString ); i++ )
	{
		this->mBypassRate.AddString( tStrBypassRate[ i ] );
	}			

	mInitValue = HSFalse;
}


void CARB1410SettingView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	mAutoAlignManager.Resize();
}

void CARB1410SettingView::OnClickedBtnOk()
{
	// TODO: Add your control notification handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());		

	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( mCardIdentifier );	
	if ( pDataHandler == NULL )
	{
		MessageBox( "设置失败!", "警告" );
		return;
	}

	CARB1410Handler *pArb1410Handler = dynamic_cast< CARB1410Handler * >( pDataHandler );
	/*
	if ( pMainFrame->ControlState() != CMainFrame::GRAPHIC_STOP )
	{
		MessageBox( "正在采样或回放中，参数设置失败!", "警告" );
		return;
	}
	*/

	CARB1410Handler::FREQUENCY_MOD tFrequcencyMod;	

	CString tStr;
	this->mBeginRate.GetWindowTextA( tStr );
	std::istringstream  Istring1( ( LPCSTR )tStr );
	Istring1 >> tFrequcencyMod.StartFrequency;
	tFrequcencyMod.StartFrequency *= 1000;

	this->mEndRate.GetWindowTextA( tStr );
	std::istringstream  Istring2( ( LPCSTR )tStr );
	Istring2 >> tFrequcencyMod.EndFrequency;
	tFrequcencyMod.EndFrequency *= 1000;

	this->mDuration.GetWindowTextA( tStr );
	std::istringstream  Istring3( ( LPCSTR )tStr );
	Istring3 >> tFrequcencyMod.Duration;
	tFrequcencyMod.Duration *= 1000;

	this->mCycleCount.GetWindowTextA( tStr );
	std::istringstream  Istring4( ( LPCSTR )tStr );
	Istring4 >> tFrequcencyMod.MinPtsPerCycle;

	pArb1410Handler->SetFrequencyMod( tFrequcencyMod );

	CARB1410Handler::SET_SIGNAL tSinalSet;
	tSinalSet.Coupling = mAC.GetCheck() ? 0 : 1;

	this->mMinAmplitude.GetWindowTextA( tStr );
	std::istringstream  Istring5( ( LPCSTR )tStr );
	Istring5 >> tSinalSet.Amplitude;
	tSinalSet.Amplitude *= 1000;

	this->mSignalOffset.GetWindowTextA( tStr );
	std::istringstream  Istring6( ( LPCSTR )tStr );
	Istring6 >> tSinalSet.DCoffset;

	tSinalSet.Filter = this->mBypassRate.GetCurSel();

	this->mSignalLate.GetWindowTextA( tStr );
	std::istringstream  Istring8( ( LPCSTR )tStr );
	Istring8 >> tSinalSet.Delay;

	pArb1410Handler->SetSignal( tSinalSet );

	CARB1410Handler::SET_TRIGGER tTrigger;

	if ( ( ( CButton * )GetDlgItem( IDC_RADIO_TRIGGER_CONTINUE ) )->GetCheck() )
	{
		tTrigger.Mode = 0;
		tTrigger.RepRate = 100 *1000;
		tTrigger.OutPolarity = 0;
		tTrigger.OutWidth = 10;
		tTrigger.Intype = 0;
		tTrigger.InSlope = 0;
	}
	else if ( ( ( CButton * )GetDlgItem( IDC_RADIO_TRIGGER_REPEAT ) )->GetCheck() )
	{
		tTrigger.Mode = 1;
		tTrigger.RepRate = 100 *1000;
		tTrigger.OutPolarity = 0;
		tTrigger.OutWidth = 10;
		tTrigger.Intype = 0;
		tTrigger.InSlope = 0;
	}
	else if ( ( ( CButton * )GetDlgItem( IDC_RADIO_TRIGGER_STEP ) )->GetCheck() )
	{
		tTrigger.Mode = 2;
		tTrigger.RepRate = 100 *1000;
		tTrigger.OutPolarity = 0;
		tTrigger.OutWidth = 10;
		tTrigger.Intype = 0;
		tTrigger.InSlope = 0;
	}
	else
	{
		tTrigger.Mode = 3;
		tTrigger.RepRate = 100 * 1000;
		tTrigger.OutPolarity = 0;
		tTrigger.OutWidth = 10;
		tTrigger.Intype = 0;
		tTrigger.InSlope = 0;
	}

	pArb1410Handler->SetTrigger( tTrigger );

	pArb1410Handler->EffectiveSettings();

	MessageBox( "参数已设置!", "警告" );
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


void CARB1410SettingView::OnEnKillfocusEditStartRate()
{
	// TODO: Add your control notification handler code here

	if ( !mInitValue )
	{
		return;
	}

	CString tStr;
	HSDouble tValue;
	mBeginRate.GetWindowTextA( tStr );
	std::istringstream istring( ( LPCSTR )tStr );
	istring >> tValue;

	if ( tStr == "" || ( tValue > 100000.0  || tValue < 0.001 ) )
	{		
		MessageBox( "StartFrequency Must be between 0.001Hz dan 100MHz!", "WARNING" );		
		mBeginRate.SetFocus();			
	}
}


void CARB1410SettingView::OnEnKillfocusEditEndRate()
{
	// TODO: Add your control notification handler code here
	if ( !mInitValue )
	{
		return;
	}

	CString tStr;
	HSDouble tValue;
	mEndRate.GetWindowTextA( tStr );
	std::istringstream istring( ( LPCSTR )tStr );
	istring >> tValue;

	if ( tStr == "" || ( tValue > 100000.0 || tValue < 0.001 ) )
	{
		MessageBox( "EndFrequency Must be between 0.001Hz dan 100MHz!", "WARNING" );		
		mEndRate.SetFocus();			
	}
}


void CARB1410SettingView::OnEnKillfocusEditDuration()
{
	// TODO: Add your control notification handler code here
	if ( !mInitValue )
	{
		return;
	}

	CString tStr;
	HSDouble tValue;
	mDuration.GetWindowTextA( tStr );

	std::istringstream istring( ( LPCSTR )tStr );
	istring >> tValue;

	if (  tStr == "" || tValue < 0.00001 )
	{		
		MessageBox( "Milisecond Duration must be greater than 0.00001!", "WARNING" );		
		mDuration.SetFocus();			
	}
}


void CARB1410SettingView::OnEnKillfocusEditCycleCount()
{
	// TODO: Add your control notification handler code here
	if ( !mInitValue )
	{
		return;
	}

	CString tStr;
	HSDouble tValue;
	mCycleCount.GetWindowTextA( tStr );

	std::istringstream istring( ( LPCSTR )tStr );
	istring >> tValue;

	if ( tStr == "" || tValue < 2.0 )
	{
		MessageBox( "Minimum Points per Cycle Must be greater than 2!", "WARNING" );	
		mCycleCount.SetFocus();			
	}
}


void CARB1410SettingView::OnEnKillfocusEditMinAmplitude()
{
	// TODO: Add your control notification handler code here
	if ( !mInitValue )
	{
		return;
	}

	CString tStr;
	HSDouble tValue;
	mMinAmplitude.GetWindowTextA( tStr );

	std::istringstream istring( ( LPCSTR )tStr );
	istring >> tValue;

	if ( tStr == "" || ( tValue < 0 || tValue > 10 ) )
	{		
		MessageBox( "AMP Must be between 0 and 10!", "WARNING" );		
		mMinAmplitude.SetFocus();			
	}
}


void CARB1410SettingView::OnEnKillfocusEditOffset()
{
	// TODO: Add your control notification handler code here
	if ( !mInitValue )
	{
		return;
	}

	CString tStr;
	mSignalOffset.GetWindowTextA( tStr );

	HSDouble tValue;
	std::istringstream istring( ( LPCSTR )tStr );
	istring >> tValue;

	if( tStr == "" || ( tValue < 0 || tValue > 5.0 ) )
	{			
		MessageBox( "DC offset must be between 0 and 5.0 at gain of 0.000!", "WARNING" );
		mSignalOffset.SetFocus();		
	}
}


void CARB1410SettingView::OnEnKillfocusEditLate()
{
	// TODO: Add your control notification handler code here
	if ( !mInitValue )
	{
		return;
	}

	CString tStr;
	mSignalLate.GetWindowTextA( tStr );

	if ( tStr == "" )
	{		
		MessageBox( "Enter a number!", "WARNING" );
		mSignalLate.SetFocus();		
	}
}
