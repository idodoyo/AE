// PCI2SettingView.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "PCI2SettingView.h"
#include "PAPCI-2Handler.h"
#include "MainFrm.h"
#include "PCI2SettingDoc.h"
#include "HSLogProtocol.h"

// CPCI2SettingView

CString CPCI2SettingView::mStrRate[ 9 ] = { "100k", "200k", "500k", "1M", "2M", "5M", "10M", "20M", "40M" };
CString CPCI2SettingView::mStrHighFilter[ 4 ] = { "1k", "20k", "100k", "200k" };
CString CPCI2SettingView::mStrLowFilter[ 6 ] = { "3M", "100k", "200k", "400k", "1M", "2M" };

IMPLEMENT_DYNCREATE(CPCI2SettingView, CFormView)

CPCI2SettingView::CPCI2SettingView()
	: CFormView(CPCI2SettingView::IDD), mAutoAlignManager( this, CRect( 0, 0, 100, 100 ) )
{	
}

CPCI2SettingView::~CPCI2SettingView()
{
}

void CPCI2SettingView::SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier )
{
	mGridProperty.RemoveAll();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	mCardIdentifier = tCardIdentifier; 

	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( tCardIdentifier );	
	CPAPCI2Handler *pPCI2Handler = dynamic_cast< CPAPCI2Handler * >( pDataHandler );	

	mGridProperty.EnableHeaderCtrl(FALSE);
	mGridProperty.EnableDescriptionArea();
	mGridProperty.SetVSDotNetLook();
	mGridProperty.MarkModifiedProperties();	

	CString tStr;

	CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("AE通道设置"));	

	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(_T("采样率"), _T( "" ), _T( "选择采样率!" ), 1000 * 1 + PROPERTY_PCI_RATE );

	for ( int j = 0; j < sizeof( mStrRate ) / sizeof( CString ); j++ )
	{
		pProp->AddOption( mStrRate[ j ] );
	}

	if ( pPCI2Handler->mChannels[ 1 ].SampleRate < 1000 )
	{
		tStr.Format( "%dk", pPCI2Handler->mChannels[ 1 ].SampleRate );
	}
	else
	{
		tStr.Format( "%dM", pPCI2Handler->mChannels[ 1 ].SampleRate / 1000 );
	}

	pProp->SetValue(tStr);
	pProp->AllowEdit( FALSE );
	pGroup->AddSubItem( pProp );

	for ( HSUInt i = 1; i <= pPCI2Handler->mChannels.size() ; i++ )
	{
		CString tStrSensorName;
		tStrSensorName.Format( "AE通道%d", i );
		CMFCPropertyGridProperty *pSensor = new CMFCPropertyGridProperty( tStrSensorName );	
		
		pProp = new CMFCPropertyGridProperty(_T("门槛类型"), _T( "" ), _T( "设置门槛类型!" ), 1000 * i + PROPERTY_PCI_THRESHOLD_TYPE );
		pProp->AddOption( _T( "固定" ) );
		pProp->AddOption( _T( "浮动" ) );
		pProp->SetValue( pPCI2Handler->mChannels[ i ].ThresholdType == 0 ? _T( "固定" ) : _T( "浮动" ) );
		pProp->AllowEdit( FALSE );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("门槛"), _T( "" ), _T( "选择门槛大小" ), 1000 * i  + PROPERTY_PCI_THRESHOLD_VALUE  );
		for ( HSInt j = 10; j < 100; j++ )
		{
			tStr.Format("%ddb", j);

			pProp->AddOption( tStr );
		}
		tStr.Empty();
		tStr.Format("%ddb", pPCI2Handler->mChannels[ i ].Threshold );
		pProp->SetValue( tStr );
		pProp->AllowEdit( false );	
		pSensor->AddSubItem( pProp );	


		pProp = new CMFCPropertyGridProperty(_T("浮动门宽"), _T( "" ), _T( "选择门槛浮动门宽!" ), 1000 * i  + PROPERTY_PCI_THRESHOLD_FLOADBAND);
		for ( HSInt k = 0; k < 100; k++ )
		{
			tStr.Format("%ddb", k);

			pProp->AddOption( tStr );
		}   
		tStr.Empty();
		tStr.Format("%ddb", pPCI2Handler->mChannels[ i ].ThresholdDeadband );
		pProp->SetValue( tStr );
		pProp->AllowEdit( false );	
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("内部增益"), _T( "" ), _T( "选择内部增益" ), 1000 * i  + PROPERTY_PCI_INPUT_GAIN );	
		pProp->AddOption( _T( "0db" ) );
		pProp->AddOption( _T( "6db" ) );
		tStr.Empty();
		tStr.Format("%ddb", pPCI2Handler->mChannels[ i ].Gain);
		pProp->SetValue( tStr );
		pProp->AllowEdit( false );
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("HDT ( us )"), _T( "" ), _T( "设置HDT，取值范围（2 － 65534 ），有效值为 2 的倍数!" ), 1000 * i  + PROPERTY_PCI_HDT ); 
		tStr.Empty();
		tStr.Format("%d", pPCI2Handler->mChannels[ i ].HDT );
		pProp->SetValue( tStr );
		pProp->AllowEdit( true );	
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("PDT ( us )"), _T( "" ), _T( "设置PDT, 取值范围（2 － 65534 )!" ), 1000 * i  + PROPERTY_PCI_PDT );
		tStr.Empty();
		tStr.Format("%d", pPCI2Handler->mChannels[ i ].PDT );
		pProp->SetValue( tStr );
		pProp->AllowEdit( true );	
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("HLT ( us )"), _T( "" ), _T( "设置HLT, 取值范围（2 － 65534 ），有效值为 2 的倍数!" ), 1000 * i  + PROPERTY_PCI_HLT );
		tStr.Empty();
		tStr.Format("%d", pPCI2Handler->mChannels[ i ].HLT );
		pProp->SetValue( tStr );
		pProp->AllowEdit( true );	
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("最大持续时间 ( us )"), _T( "" ), _T( "设置最多持续时间, 取值范围（1 － 1000 )!" ), 1000 * i  + PROPERTY_PCI_MAXDURA );
		tStr.Empty();
		tStr.Format("%d", pPCI2Handler->mChannels[ i ].MaxDuration );
		pProp->SetValue( tStr );
		pProp->AllowEdit( true );	
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("预触发采样点"), _T( "" ), _T( "设置预触发采样点数!" ), 1000 * i  + PROPERTY_PCI_PRETRIGGER ); 
		tStr.Empty();
		tStr.Format("%d", pPCI2Handler->mChannels[ i ].PreTrigger );
		pProp->SetValue( tStr );
		pProp->AllowEdit( true );	
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("预触发长度 (k = 1024 samples )"), _T( "" ), _T( "选择一帧波形的长度!" ), 1000 * i  + PROPERTY_PCI_WAVEFROM_LENGTH ); 
		for ( HSInt k = 1; k <= 15; k++ )
		{
			tStr.Format("%dk", k);

			pProp->AddOption( tStr );
		}
		tStr.Empty();
		tStr.Format("%dk", pPCI2Handler->mChannels[ i ].WaveformLength );
		pProp->SetValue( tStr );
		pProp->AllowEdit( false );	
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("模拟滤波器 下限 "), _T( "" ), _T( "选择模拟低通滤波!" ), 1000 * i  + PROPERTY_PCI_LOW_FILTER );
		for ( int j = 0; j < sizeof( mStrLowFilter ) / sizeof( CString ); j++ )
		{
			pProp->AddOption( mStrLowFilter[ j ] );
		}
		
		pProp->SetValue( mStrLowFilter[ pPCI2Handler->mChannels[ i ].LowFilter ] );
		pProp->AllowEdit( false );	
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("模拟滤波器 上限 "), _T( "" ), _T( "选择高通滤波!" ), 1000 * i  + PROPERTY_PCI_HIGH_FILTER ); 
		for ( int j = 0; j < sizeof( mStrHighFilter ) / sizeof( CString ); j++ )
		{
			pProp->AddOption( mStrHighFilter[ j ] );
		}		
		
		pProp->SetValue( mStrHighFilter[ pPCI2Handler->mChannels[ i ].HighFilter ] );
		pProp->AllowEdit( false );	
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("传感器增益"), _T( "" ), _T( "选择幅值增益!" ), 1000 * i  + PROPERTY_PCI_AMP_GAIN );
		for ( HSInt k = 0; k <= 60; k++ )
		{
			tStr.Format("%ddb", k);

			pProp->AddOption( tStr );
		}
		tStr.Empty();
		tStr.Format("%ddb", pPCI2Handler->mChannels[ i ].AmpGain );
		pProp->SetValue( tStr );
		pProp->AllowEdit( false );	
		pSensor->AddSubItem( pProp );

		pProp = new CMFCPropertyGridProperty(_T("RMS_ASL_Time ( ms )"), _T( "" ), _T( "设置RMS_ASL_TIME 取值范围（10 - 1000 )，有效值为10的倍数!" ), 1000 * i  + PROPERTY_PCI_RMS_ASL_TIME );
		tStr.Empty();
		tStr.Format("%d", pPCI2Handler->mChannels[ i ].RMS_ASL_Time );
		pProp->SetValue( tStr );
		pProp->AllowEdit( true );	
		pSensor->AddSubItem( pProp );


		pProp = new CMFCPropertyGridProperty(_T("周期计数"), _T( "" ), _T( "设置循环计数次数, 取值范围（－10000 - +10000 )!" ), 1000 * i  + PROPERTY_PCI_CYCLE_COUNTER ); 
		tStr.Empty();
		tStr.Format("%d", pPCI2Handler->mChannels[ i ].CounterThreshold );
		pProp->SetValue( tStr );
		pProp->AllowEdit( true );	
		pSensor->AddSubItem( pProp );
			
		pGroup->AddSubItem( pSensor );
	}	

	mGridProperty.AddProperty( pGroup );	

	CString tTitle;
	tTitle.Format( "%s 设置", pDataHandler->Name().c_str() );

	CPCI2SettingDoc *pDoc = dynamic_cast< CPCI2SettingDoc * >( this->GetDocument() );
	pDoc->SetCardIdentifier( mCardIdentifier, tTitle );
	
}

void CPCI2SettingView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID_PCI2, mGridProperty);
}

BEGIN_MESSAGE_MAP(CPCI2SettingView, CFormView)
	ON_MESSAGE( WM_VALUE_INCORRCET_MESSAGE, OnValueIncorrect )
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BTN_OK, &CPCI2SettingView::OnBnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CPCI2SettingView::OnBnClickedBtnCancel)
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
END_MESSAGE_MAP()


// CPCI2SettingView diagnostics

#ifdef _DEBUG
void CPCI2SettingView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPCI2SettingView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPCI2SettingView message handlers


void CPCI2SettingView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class

	mAutoAlignManager.AddControl( IDC_BTN_OK, CRect( -1, -1, 100, 15 ) );
	mAutoAlignManager.AddControl( IDC_BTN_CANCEL, CRect( -1, -1, 10, 15 ) );
	mAutoAlignManager.AddControl( IDC_MFCPROPERTYGRID_PCI2, CRect( 0, 0, 0, 100 ) );
}


void CPCI2SettingView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	mAutoAlignManager.Resize();
}


void CPCI2SettingView::OnLButtonUp(UINT nFlags, CPoint point)
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


void CPCI2SettingView::OnBnClickedBtnOk()
{
	// TODO: Add your control notification handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( mCardIdentifier );	
	if ( pDataHandler == NULL )
	{
		MessageBox( "设置失败!", "警告" );
		return;
	}

	if ( !pMainFrame->IsSettingArgEnabled() )
	{
		MessageBox( "正在采样或回放中，参数设置失败!", "警告" );
		return;
	}

	CPAPCI2Handler *pPCI2Handler = dynamic_cast< CPAPCI2Handler * >( pDataHandler );

	HSInt tSampleRate = 0;

	CMFCPropertyGridProperty* pProp = mGridProperty.FindItemByData( 1000 * 1 + PROPERTY_PCI_RATE );
	if ( pProp != NULL )
	{		
		CString tStr = pProp->GetValue();	

		istringstream tStream( ( LPCSTR )tStr );
		tStream >> tSampleRate;

		if ( tStr.Find( 'M' ) >= 0 )
		{
			tSampleRate *= 1000;
		}
	}

	for ( HSUInt i = 1; i <= ( pPCI2Handler->mChannels.size() ); i++ )
	{
		pPCI2Handler->mChannels[ i ].SampleRate = tSampleRate;

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_THRESHOLD_TYPE );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();				
			istringstream tStream( ( LPCSTR )tStr );
			tStream >> ( pPCI2Handler->mChannels[ i ].ThresholdType );
		}		

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_THRESHOLD_FLOADBAND );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();				
			istringstream tStream( ( LPCSTR )tStr );
			tStream >> ( pPCI2Handler->mChannels[ i ].ThresholdDeadband );
		}

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_INPUT_GAIN );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();				
			istringstream tStream( ( LPCSTR )tStr );
			tStream >> ( pPCI2Handler->mChannels[ i ].Gain );
		}

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_MAXDURA );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();				
			istringstream tStream( ( LPCSTR )tStr );
			tStream >> ( pPCI2Handler->mChannels[ i ].MaxDuration );
		}

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_PDT );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();				
			istringstream tStream( ( LPCSTR )tStr );
			tStream >> ( pPCI2Handler->mChannels[ i ].PDT );
		}

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_HDT );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();				
			istringstream tStream( ( LPCSTR )tStr );
			tStream >> ( pPCI2Handler->mChannels[ i ].HDT );
		}

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_HLT );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();				
			istringstream tStream( ( LPCSTR )tStr );
			tStream >> ( pPCI2Handler->mChannels[ i ].HLT );
		}

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_PRETRIGGER );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();				
			istringstream tStream( ( LPCSTR )tStr );
			tStream >> ( pPCI2Handler->mChannels[ i ].PreTrigger );
		}

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_LOW_FILTER );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();		
			for ( HSInt j = 0; j < sizeof( mStrLowFilter ) / sizeof( CString ); j++ )
			{
				if ( tStr == mStrLowFilter[ j ] )
				{
					pPCI2Handler->mChannels[ i ].LowFilter = j;
					break;
				}
			}			
		}

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_HIGH_FILTER );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();	
			for ( HSInt j = 0; j < sizeof( mStrLowFilter ) / sizeof( CString ); j++ )
			{
				if ( tStr == mStrHighFilter[ j ] )
				{
					pPCI2Handler->mChannels[ i ].HighFilter = j;
					break;
				}
			}			
		}

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_AMP_GAIN );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();				
			istringstream tStream( ( LPCSTR )tStr );
			tStream >> ( pPCI2Handler->mChannels[ i ].AmpGain );
		}

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_THRESHOLD_VALUE );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();		
			istringstream tStream( ( LPCSTR )tStr );
			tStream >> ( pPCI2Handler->mChannels[ i ].Threshold );		

			HSDouble tUnit = 1000000.0;
			HSDouble tTimes = pPCI2Handler->mChannels[ i ].AmpGain;
			pPCI2Handler->SetBypassVoltage( i - 1, pow( 10.0, ( pPCI2Handler->mChannels[ i ].Threshold  ) / 20.0 ) / tUnit * tTimes );
		}


		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_WAVEFROM_LENGTH );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();				
			istringstream tStream( ( LPCSTR )tStr );
			tStream >> ( pPCI2Handler->mChannels[ i ].WaveformLength );
		}

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_RMS_ASL_TIME );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();				
			istringstream tStream( ( LPCSTR )tStr );
			tStream >> ( pPCI2Handler->mChannels[ i ].RMS_ASL_Time );
		}

		pProp = mGridProperty.FindItemByData( 1000 * i + PROPERTY_PCI_CYCLE_COUNTER );
		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();				
			istringstream tStream( ( LPCSTR )tStr );
			tStream >> ( pPCI2Handler->mChannels[ i ].CounterThreshold );
		}

		pPCI2Handler->SetChannelInfo( i, pPCI2Handler->mChannels[ i ] );

		/*
		HS_INFO( "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
			pPCI2Handler->mChannels[ i ].SampleLength,
			pPCI2Handler->mChannels[ i ].SampleRate,
			pPCI2Handler->mChannels[ i ].ThresholdType,
			pPCI2Handler->mChannels[ i ].Threshold,
			pPCI2Handler->mChannels[ i ].ThresholdDeadband,
			pPCI2Handler->mChannels[ i ].Gain,
			pPCI2Handler->mChannels[ i ].HDT,
			pPCI2Handler->mChannels[ i ].PDT,
			pPCI2Handler->mChannels[ i ].HLT,
			pPCI2Handler->mChannels[ i ].MaxDuration,
			pPCI2Handler->mChannels[ i ].WaveformLength,
			pPCI2Handler->mChannels[ i ].PreTrigger,
			pPCI2Handler->mChannels[ i ].LowFilter,
			pPCI2Handler->mChannels[ i ].HighFilter,
			pPCI2Handler->mChannels[ i ].AmpGain,
			pPCI2Handler->mChannels[ i ].DrivenRate,
			pPCI2Handler->mChannels[ i ].RMS_ASL_Time,
			pPCI2Handler->mChannels[ i ].CounterThreshold );
			*/
	}

	MessageBox( "参数已设置!", "警告" );
}


void CPCI2SettingView::OnBnClickedBtnCancel()
{
	// TODO: Add your control notification handler code here

	this->GetDocument()->OnCloseDocument();
}

HSBool CPCI2SettingView::DigitOnlyCheck( CString &tValue )
{
	HSInt tLength = tValue.GetLength();
	if ( tLength == 0 )
	{
		return HSFalse;
	}

	for ( HSInt i = 0; i < tLength; i++ )
	{
		if ( tValue[ i ] < '0' || tValue[ i ] > '9' )
		{
			return HSFalse;
		}
	}

	return HSTrue;
}

LRESULT CPCI2SettingView::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{	
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( mCardIdentifier );	
	CPAPCI2Handler *pPCI2Handler = dynamic_cast< CPAPCI2Handler * >( pDataHandler );

	CMFCPropertyGridProperty *pProperty = ( CMFCPropertyGridProperty * ) lparam;
	CMFCPropertyGridProperty* pProp;
	std::stringstream string;
	HSUInt tVal;
	HSUInt tWaveLength;
	CString str_val;
	
	HSUInt tID = pProperty->GetData() % 1000;
	CString tValue = pProperty->GetValue();
	
	HSBool tDigitOnly = this->DigitOnlyCheck( tValue );

	string.clear();
	string << tValue;
	string >> tVal;

	mIncorrectMsg = "";

	switch ( tID )
	{
	case  PROPERTY_PCI_MAXDURA:
		if ( tVal > 1000 || tVal < 1 || !tDigitOnly )
		{
			pProp = mGridProperty.FindItemByData( 1000 *  (pProperty->GetData() / 1000 ) + PROPERTY_PCI_MAXDURA );
			str_val.Empty();
			str_val.Format( "%d", pPCI2Handler->mChannels[ pProperty->GetData() / 1000 ].MaxDuration );
			pProp->SetValue( str_val );
			mIncorrectMsg = "持续时间设置错误";
		}
		break;

	case  PROPERTY_PCI_PDT:
		if (  tVal > 65534 || tVal < 2 || !tDigitOnly )
		{
			pProp = mGridProperty.FindItemByData( 1000 *  (pProperty->GetData() / 1000 ) + PROPERTY_PCI_PDT );
			str_val.Empty();
			str_val.Format("%d", pPCI2Handler->mChannels[ pProperty->GetData() / 1000 ].PDT );
			pProp->SetValue( str_val );
			mIncorrectMsg = "PDT 设置错误!";
		}
		break;

	case  PROPERTY_PCI_HDT:
		if ( tVal > 65534 || tVal < 2 || !tDigitOnly || ( tVal % 2 ) != 0 )
		{
			pProp = mGridProperty.FindItemByData( 1000 *  (pProperty->GetData() / 1000 ) + PROPERTY_PCI_HDT );
			str_val.Empty();
			str_val.Format("%d", pPCI2Handler->mChannels[ pProperty->GetData() / 1000 ].HDT );
			pProp->SetValue( str_val );
			mIncorrectMsg = "HDT 设置错误!";
		}
		break;

	case PROPERTY_PCI_HLT:
		if ( tVal > 65534 || tVal < 2 || !tDigitOnly || ( tVal % 2 ) != 0 )
		{
			pProp = mGridProperty.FindItemByData( 1000 *  (pProperty->GetData() / 1000 ) + PROPERTY_PCI_HLT );
			str_val.Empty();
			str_val.Format("%d", pPCI2Handler->mChannels[ pProperty->GetData() / 1000 ].HLT );
			pProp->SetValue( str_val );
			mIncorrectMsg = "HLT 设置错误!";
		}
		break;

	case PROPERTY_PCI_PRETRIGGER:
		pProp = mGridProperty.FindItemByData( 1000 *  (pProperty->GetData() / 1000 ) + PROPERTY_PCI_WAVEFROM_LENGTH );

		if ( pProp != NULL )
		{	
			CString tStr = pProp->GetValue();				
			string.clear();
			string << tStr;
			string >> tWaveLength;
		}

		if ( tVal > tWaveLength * 1024 || !tDigitOnly )
		{
			pProp = mGridProperty.FindItemByData( 1000 *  (pProperty->GetData() / 1000 ) + PROPERTY_PCI_PRETRIGGER );
			str_val.Empty();
			str_val.Format("%d", pPCI2Handler->mChannels[ pProperty->GetData() / 1000 ].PreTrigger );
			pProp->SetValue( str_val );

			pProp = mGridProperty.FindItemByData( 1000 *  (pProperty->GetData() / 1000 ) + PROPERTY_PCI_WAVEFROM_LENGTH );
			str_val.Empty();
			str_val.Format("%d", pPCI2Handler->mChannels[ pProperty->GetData() / 1000 ].WaveformLength );
			pProp->SetValue( str_val );

			mIncorrectMsg = "预触发采样点长度设置错误!";
		}
		break;

	case  PROPERTY_PCI_RMS_ASL_TIME:
		if ( tVal < 10 || tVal > 1000 || tVal % 10 != 0 || !tDigitOnly )
		{
			pProp = mGridProperty.FindItemByData( 1000 *  (pProperty->GetData() / 1000 ) + PROPERTY_PCI_RMS_ASL_TIME );
			str_val.Empty();
			str_val.Format("%d", pPCI2Handler->mChannels[ pProperty->GetData() / 1000 ].RMS_ASL_Time );
			pProp->SetValue( str_val );

			mIncorrectMsg = "RMS_ASL_TIME 设置错误!";
		}
		break;

	case PROPERTY_PCI_CYCLE_COUNTER:
		if ( tVal > 10000 || !tDigitOnly )
		{
			pProp = mGridProperty.FindItemByData( 1000 *  (pProperty->GetData() / 1000 ) + PROPERTY_PCI_CYCLE_COUNTER );
			str_val.Empty();
			str_val.Format("%d", pPCI2Handler->mChannels[ pProperty->GetData() / 1000 ].CounterThreshold );
			pProp->SetValue( str_val );

			mIncorrectMsg = "周期计数设置错误！";
		}
		break;

	default:
		break;
	}

	if ( mIncorrectMsg == "" )
	{
		return 0;
	}

	this->PostMessageA( WM_VALUE_INCORRCET_MESSAGE, 0, 0 );


	return 1;
}


LRESULT CPCI2SettingView::OnValueIncorrect( WPARAM wparam, LPARAM lparam )
{
	AfxMessageBox( mIncorrectMsg );

	return 0;
}
