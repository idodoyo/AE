// DataExportToTXT.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "DataExportToTXT.h"
#include "afxdialogex.h"
#include "GraghicWnd.h"
#include "DataHandler.h"



// CDataExportToTXT dialog

IMPLEMENT_DYNAMIC(CDataExportToTXT, CDialogEx)

CDataExportToTXT::CDataExportToTXT(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDataExportToTXT::IDD, pParent), mExportThread( this, 0, 0 )
{
	mDeviceManager = NULL;
	mIndexManager = NULL;

	mIsStart = HSFalse;
}

CDataExportToTXT::~CDataExportToTXT()
{
	ClearDataExportInfo();
}

void CDataExportToTXT::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDataExportToTXT, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &CDataExportToTXT::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CDataExportToTXT::OnBnClickedOk)
	ON_BN_CLICKED(ID_BTN_CHOOSE_DIR, &CDataExportToTXT::OnBnClickedBtnChooseDir)
END_MESSAGE_MAP()


// CDataExportToTXT message handlers
HSVoid CDataExportToTXT::SetDeviceInfo( CDeviceManager *pDeviceManager, CIndexManager *pIndexManager, HSString tDataFilePath )
{
	mDeviceManager = pDeviceManager;
	mIndexManager = pIndexManager;
	mDataFilePath = tDataFilePath;
}

void CDataExportToTXT::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	if ( mIsStart )
	{
		if ( IDYES != MessageBox( "数据导出中，确认退去?", "警告", MB_YESNO ) )
		{
			return;
		}
	}

	mExportThread.Stop();

	CDialogEx::OnCancel();
}


void CDataExportToTXT::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here	

	CString tResFile;
	this->GetDlgItem( IDC_EDIT_SAVE_PATH )->GetWindowText( tResFile );
	if ( tResFile == "" )
	{
		OnBnClickedBtnChooseDir();

		this->GetDlgItem( IDC_EDIT_SAVE_PATH )->GetWindowText( tResFile );
		if ( tResFile == "" )
		{
			return;
		}
	}

	ClearDataExportInfo();

	mSplitToSmallFile = ( ( ( CButton * )GetDlgItem( IDC_CHECK_SPLIT_FILE ) )->GetCheck() ? HSTrue : HSFalse );
	mTotalDataSize = 0;
	mHandledDataSize = 0;

	CListCtrl *pListCtrl = ( CListCtrl * )this->GetDlgItem( IDC_LIST_DEVICES );
	IDataHandler *pLastHandler = NULL;
	for ( HSInt i = 0; i < pListCtrl->GetItemCount(); i++ )
	{
		if ( pListCtrl->GetCheck( i ) )
		{
			IDataHandler *pHandler = NULL;
			DEVICE_CH_IDENTIFIER tChannelIdentifier = 0;
			DataHandlerWithListIndex( i, pHandler, tChannelIdentifier );
			mTotalDataSize += mItemDataSize[ i ];
			HSUInt tSamplesInOneFrame = pHandler->SamplesInFrame( tChannelIdentifier );
			CLinearTransfer *pLinearTransfer = new CLinearTransfer( mIndexManager, pHandler, tChannelIdentifier );
			HSChar *pBuffer = new HSChar[ tSamplesInOneFrame * pHandler->EachSampleSize() ];
			HSDouble tSeekSeconds = ( HSDouble )tSamplesInOneFrame / pHandler->SampleRate( tChannelIdentifier );
			if ( pHandler == pLastHandler )
			{
				HSUInt tLastIndex = mDataExportInfo.size() - 1;
				mDataExportInfo[ tLastIndex ].LinearTransfer.push_back( pLinearTransfer );
				mDataExportInfo[ tLastIndex ].Buffer.push_back( pBuffer );
				mDataExportInfo[ tLastIndex ].SeekSeconds.push_back( tSeekSeconds );
				mDataExportInfo[ tLastIndex ].ChannelIdentifier.push_back( tChannelIdentifier );
				mDataExportInfo[ tLastIndex ].GotData.push_back( HSFalse );
			}
			else
			{
				DATA_EXPORT_INFO tDataExportInfo;
				tDataExportInfo.LinearTransfer.push_back( pLinearTransfer );
				tDataExportInfo.Buffer.push_back( pBuffer );
				tDataExportInfo.SeekSeconds.push_back( tSeekSeconds );
				tDataExportInfo.GotData.push_back( HSFalse );
				tDataExportInfo.Index = 0;				
				tDataExportInfo.SaveStream = new ofstream;
				tDataExportInfo.ChannelIdentifier.push_back( tChannelIdentifier );
				tDataExportInfo.DataHandler = pHandler;
				
				
				CString tFileName = mDataFilePath.c_str();	
				tFileName = tFileName.Mid( tFileName.ReverseFind( '\\' ) + 1 );
				tFileName = tResFile + tFileName.Left( tFileName.ReverseFind( '.' ) );
				tFileName += "_";
				tFileName += pHandler->Name().c_str();
				tDataExportInfo.FileFormat = tFileName + "_%05d.txt";	
				tFileName += ".txt";
				if ( mSplitToSmallFile )
				{					
					tFileName.Format( tDataExportInfo.FileFormat, tDataExportInfo.Index++ );	
					
					tDataExportInfo.SaveStream->open( ( LPCSTR )tFileName );					
				}
				else
				{
					tDataExportInfo.SaveStream->open( ( LPCSTR )tFileName );
				}				

				mDataExportInfo.push_back( tDataExportInfo );
			}

			pLastHandler = pHandler;
		}
	}

	if ( mDataExportInfo.size() < 1 )
	{
		MessageBox( "请选择数据源!", "警告" );
		return;
	}	

	GetDlgItem( IDC_LIST_DEVICES )->EnableWindow( HSFalse );
	GetDlgItem( IDC_CHECK_SPLIT_FILE )->EnableWindow( HSFalse );
	GetDlgItem( ID_BTN_CHOOSE_DIR )->EnableWindow( HSFalse );
	GetDlgItem( IDOK )->EnableWindow( HSFalse );

	mDataExportIndex = 0;

	mExportThread.Start();	

	mIsStart = HSTrue;

	//CDialogEx::OnOK();
}

HSVoid CDataExportToTXT::DataHandlerWithListIndex( HSInt tIndex, IDataHandler * &pHandler, DEVICE_CH_IDENTIFIER &tChannelIdentifier )
{
	HSUInt tChannelIndex = 0;
	map< DEVICE_CH_IDENTIFIER, vector< HSUInt > >::iterator pIterator = mDeviceChannelInfo.begin();
	while ( pIterator != mDeviceChannelInfo.end() )
	{
		if ( pIterator->second.size() > ( HSUInt )tIndex )
		{
			tChannelIndex = tIndex;	
			break;
		}	

		tIndex -= pIterator->second.size();

		pIterator++;
	}

	pHandler = mDeviceManager->DataHandlerWithIdentifier( pIterator->first );
	
	tChannelIdentifier = DEVICE_CH_IDENTIFIER( pIterator->first.CARD_IDENTIFIER, pIterator->first.CARD_INDEX );
	tChannelIdentifier.InitChannel( ( pIterator->second )[ tChannelIndex ] );
}

HSVoid CDataExportToTXT::ClearDataExportInfo()
{
	for ( HSUInt i = 0; i < mDataExportInfo.size(); i++ )
	{
		for ( HSUInt j = 0; j < mDataExportInfo[ i ].LinearTransfer.size(); j++ )
		{
			delete mDataExportInfo[ i ].LinearTransfer[ j ];
			delete[] mDataExportInfo[ i ].Buffer[ j ];
		}
		delete  mDataExportInfo[ i ].SaveStream;
	}

	mDataExportInfo.clear();
}

BOOL CDataExportToTXT::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	CListCtrl *pListCtrl = ( CListCtrl * )this->GetDlgItem( IDC_LIST_DEVICES );
	pListCtrl->SetExtendedStyle( pListCtrl->GetExtendedStyle() | LVS_EX_GRIDLINES| LVS_EX_CHECKBOXES );
	pListCtrl->InsertColumn( 0, "", LVCFMT_CENTER, 20 );
	pListCtrl->InsertColumn( 1, "设备", LVCFMT_CENTER, 150 );
	pListCtrl->InsertColumn( 2, "通道", LVCFMT_CENTER, 150 );
	pListCtrl->InsertColumn( 3, "数据量", LVCFMT_CENTER, 100 );

	mImageList.Create( 24, 24, 0, 1, 1 );
	pListCtrl->SetImageList( &mImageList, 1 );

	HSUInt tIndexCount = mIndexManager->Count();
	for ( HSUInt i = 0; i < tIndexCount; i++ )
	{
		DEVICE_CH_IDENTIFIER tType = mIndexManager->TypeWithIndex( i );		
		DEVICE_CH_IDENTIFIER tCardType( tType.CARD_IDENTIFIER, tType.CARD_INDEX );
		tType.NumOfChannels( &mDeviceChannelInfo[ tCardType ] );		
	}

	map< DEVICE_CH_IDENTIFIER, vector< HSUInt > >::iterator pIterator = mDeviceChannelInfo.begin();
	while ( pIterator != mDeviceChannelInfo.end() )
	{
		for ( HSUInt i = 0; i < pIterator->second.size(); i++ )
		{
			HSInt tItem = pListCtrl->GetItemCount();
			pListCtrl->InsertItem( tItem, "", 0 );
			pListCtrl->SetCheck( tItem, HSTrue );
			pListCtrl->SetItemText( tItem, 1, mDeviceManager->DataHandlerWithIdentifier( pIterator->first )->Name().c_str() );

			CString tStr = "";
			tStr.Format( "通道%d", ( pIterator->second )[ i ] + 1 );
			pListCtrl->SetItemText( tItem, 2, tStr );
			
			DEVICE_CH_IDENTIFIER tChannelIdentifier( pIterator->first.CARD_IDENTIFIER, pIterator->first.CARD_INDEX, pIterator->first.TYPE );
			tChannelIdentifier.InitChannel( ( pIterator->second )[ i ] );
			HSUInt tSamplesInOneFrame =  mDeviceManager->DataHandlerWithIdentifier( pIterator->first )->SamplesInFrame( tChannelIdentifier );
			DEVICE_CH_IDENTIFIER tDataIdentifier = mDeviceManager->DataHandlerWithIdentifier( pIterator->first )->DataIdentifier( tChannelIdentifier );
			HSUInt tFrameCount = mIndexManager->IndexCountWithType( tDataIdentifier );

			HSInt64 tTotalSize = tFrameCount * tSamplesInOneFrame * mDeviceManager->DataHandlerWithIdentifier( pIterator->first )->EachSampleSize();			

			pListCtrl->SetItemText( tItem, 3, CGraghicWnd::GetStrSize( ( HSDouble )tTotalSize ).c_str() );

			mItemDataSize.push_back( tTotalSize );
			
		}
		pIterator++;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDataExportToTXT::OnBnClickedBtnChooseDir()
{
	// TODO: Add your control notification handler code here
	BROWSEINFO bi = { 0 };  
    bi.hwndOwner = this->m_hWnd;
    bi.lpszTitle = "打开文件夹";
    bi.ulFlags = 0x0040 | BIF_EDITBOX;
    bi.lpfn = NULL;
	bi.lParam = 0;

	LPITEMIDLIST pSelected = SHBrowseForFolder( &bi );   
	char tDirPath[ 512 ] = { 0 };
	if ( pSelected == NULL || !SHGetPathFromIDList( pSelected, tDirPath ) )
	{
		return;
	}

	this->GetDlgItem( IDC_EDIT_SAVE_PATH )->SetWindowText( CString( tDirPath ) + "\\" );
}

HSBool CDataExportToTXT::ThreadRuning( HSInt tThreadID )
{
	IDataHandler *pDataHandler = mDataExportInfo[ mDataExportIndex ].DataHandler;
	HSUInt tSamplesCount = 0;
	for ( HSUInt i = 0; i < mDataExportInfo[ mDataExportIndex ].LinearTransfer.size(); i++ )
	{
		mDataExportInfo[ mDataExportIndex ].GotData[ i ] = HSFalse;
		CLinearTransfer *pLinearTransfer = mDataExportInfo[ mDataExportIndex ].LinearTransfer[ i ];		
		DEVICE_CH_IDENTIFIER tChannelIdentifier = mDataExportInfo[ mDataExportIndex ].ChannelIdentifier[ i ];
		HSDouble tSeconds = mDataExportInfo[ mDataExportIndex ].SeekSeconds[ i ];
		tSamplesCount = pDataHandler->SamplesInFrame( tChannelIdentifier );
		if ( !pLinearTransfer->GotToEnd() && pLinearTransfer->Seek( tSeconds, tSeconds ) )
		{			
			pLinearTransfer->Read(  mDataExportInfo[ mDataExportIndex ].Buffer[ i ], tSeconds );			
			mDataExportInfo[ mDataExportIndex ].GotData[ i ] = HSTrue;
		}
	}

	HSBool tFinished = HSTrue;
	for ( HSUInt i = 0; i < mDataExportInfo[ mDataExportIndex ].LinearTransfer.size(); i++ )
	{
		if ( mDataExportInfo[ mDataExportIndex ].GotData[ i ] )
		{
			tFinished = HSFalse;
			break;
		}
	}

	if ( tFinished )
	{
		mDataExportInfo[ mDataExportIndex ].SaveStream->close();
		
		mDataExportIndex++;
		if ( mDataExportIndex >= mDataExportInfo.size() )
		{			
			return HSFalse;			
		}

		return HSTrue;
	}

	HSChar tStrBuf[ 1024 ];	
	for ( HSUInt j = 0; j < tSamplesCount; j++ )
	{				
		for ( HSUInt i = 0; i < mDataExportInfo[ mDataExportIndex ].LinearTransfer.size(); i++ )
		{
			HSDouble tValue = 0.0;
			if ( mDataExportInfo[ mDataExportIndex ].GotData[ i ] )
			{				
				HSChar *pPointer = mDataExportInfo[ mDataExportIndex ].Buffer[ i ];
				tValue = pDataHandler->VoltageWithPointer( &pPointer[ pDataHandler->EachSampleSize() * j ] );	
				mHandledDataSize += pDataHandler->EachSampleSize();
			}
			sprintf_s( tStrBuf, "%f\t", tValue );
			( *mDataExportInfo[ mDataExportIndex ].SaveStream ) << tStrBuf;			
		}
		
		( *mDataExportInfo[ mDataExportIndex ].SaveStream ) << "\n";

		if ( mHandledDataSize % 1048576 == 0 )
		{
			UpdateHandleProgress();
		}
	}

	if ( mSplitToSmallFile && mDataExportInfo[ mDataExportIndex ].SaveStream->tellp() > SMALL_TXT_FILE_SIZE )
	{
		mDataExportInfo[ mDataExportIndex ].SaveStream->close();
		mDataExportInfo[ mDataExportIndex ].SaveStream->clear();

		CString tFile;
		tFile.Format( mDataExportInfo[ mDataExportIndex ].FileFormat, mDataExportInfo[ mDataExportIndex ].Index++ );
		mDataExportInfo[ mDataExportIndex ].SaveStream->open( ( LPCSTR )tFile );
	}

	return HSTrue;
}

HSVoid CDataExportToTXT::ThreadWillStop( HSInt tThreadID )
{
	ClearDataExportInfo();

	GetDlgItem( IDC_LIST_DEVICES )->EnableWindow( HSTrue );
	GetDlgItem( IDC_CHECK_SPLIT_FILE )->EnableWindow( HSTrue );
	GetDlgItem( ID_BTN_CHOOSE_DIR )->EnableWindow( HSTrue );
	GetDlgItem( IDOK )->EnableWindow( HSTrue );

	mIsStart = HSFalse;

	GetDlgItem( IDC_STATIC_PROGRESS )->SetWindowText( "完成" );
}

HSVoid CDataExportToTXT::UpdateHandleProgress()
{	
	CString tStr;
	tStr.Format( "%d %s", ( HSInt )( ( ( HSDouble )mHandledDataSize / mTotalDataSize ) * 100 ), "%" );
	GetDlgItem( IDC_STATIC_PROGRESS )->SetWindowText( tStr );
}