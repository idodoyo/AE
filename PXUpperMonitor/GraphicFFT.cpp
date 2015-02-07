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
#include "DeviceTypes.h"

// CGraphicFFT dialog

CGraphicFFT::CGraphicFFT(CWnd* pParent /*=NULL*/)
	: CNormalGraphic( pParent )
{	
	mBuffer = NULL;

	mDataTransfer = NULL;	
	mChannel = 0;

	mArgTransfer = NULL;

	mCurCalFFTIndex = -1;

	mRefreshSecond = 1.0;	

	mBeginAmplitude = 10;
	mTotalAmplitude = 60;

	mCurSampleRate = 1.0;

	mFourierCalLength = 2 * 1024;
	mFourierResData = NULL;
	mFourierResHz = NULL;

	mVoltages = new HSFloat[ mFourierCalLength ];
	mTmpVoltages = NULL;
	mTmpVotagesLength = 0;
	/*
	mHorValue = NULL;
	mVerValue = NULL;
	*/
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
	
	delete[] mVoltages;

	if ( mTmpVoltages != NULL )
	{
		delete[] mTmpVoltages;
	}
	/*
	if ( mHorValue != NULL )
	{
		delete[] mHorValue;
		delete[] mVerValue;
	}*/
}

IGraphicPropertyProtocol * CGraphicFFT::CreatePropertyDlg( CWnd *pParent )
{
	CGraphicFFTProperty *pFFTProperty = new CGraphicFFTProperty;
	pFFTProperty->Create( IDD_NORMALPROPERTY, pParent );
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

HSVoid CGraphicFFT::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{		
	WaitForSingleObject( mDataMutex, INFINITE );

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
	mStaticFFT.SetData( NULL, NULL, 0 );
	mStaticFFT.Invalidate();

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

	SetEvent( mDataMutex );	

	//HS_INFO( "Identifier: %s", mDataIdentifier.ToString().c_str() );
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
	/*
	HSInt tFirS = sizeof( sFIR ) / sizeof( HSDouble );
	for ( HSInt i = mFourierCalLength - 1; i > tFirS; i-- )
	{
		mFourierResData[ i ] *= sFIR[ 0 ];
		for ( HSInt j = 1; j < tFirS; j++ )
		{
			mFourierResData[ i ] += sFIR[ j ] * mFourierResData[ i - j ];
		}
	}
	*/
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

	/* fake data */
	/*
	if ( this->mDataIdentifier.CARD_IDENTIFIER == CARD_PCI1714 )
	{
		srand( ( HSInt ) time( 0 ) );

		HSInt tOneThird = mFourierCalLength / 2 / 3;
		for ( HSInt i = 0; i < mFourierCalLength / 2; i++ )
		{
			if ( i < tOneThird )
			{
				mFourierResData[ i ] = mFourierResData[ i * 3 ];
			}
			else
			{
				HSInt tBase = rand() % 4 + 2;
				mFourierResData[ i ] = mFourierResData[ tOneThird ] + ( rand() % 10 > 5 ? 1 : -1 ) * ( rand() % tBase );
			}
		}
	}*/
}


HSInt CGraphicFFT::ReadSegmentData( HSInt tArgIndex )
{	
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

	HSDouble tHitDuration = CArgCalculator::SharedInstance()->GetHitFullTime() / 1000000000.0 + 0.005;
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
	
	//HS_INFO( "Arg Count: %d", mArgList.size() );

	if ( tArgIndex >= ( HSInt )mArgList.size() )
	{
		return 0;
	}

	//HSDouble tRefreshSecond = ( ( HSDouble )( mFourierCalLength ) ) / mDataHandler->SampleRate( mDataIdentifier );
	HSDouble tRefreshSecond = tHitDuration;
	HSDouble tSecond = mArgList[ tArgIndex ].NBeginTime / 1000000000.0;

	if ( mDataTransfer->Seek( tSecond, tRefreshSecond, CLinearTransfer::SET_SAMPLE ) )
	{
		return mDataTransfer->Read( mBuffer, tRefreshSecond );
	}

	return 0;
}

HSVoid CGraphicFFT::RefreshFFT( HSInt tDirection )
{	
	WaitForSingleObject( mDataMutex, INFINITE );
	
	if ( mArgTransfer )
	{	
		HSInt tCurFFTIndex = mCurCalFFTIndex;

		tCurFFTIndex += tDirection;
		tCurFFTIndex = max( 0, tCurFFTIndex );

		if ( tCurFFTIndex == mCurCalFFTIndex )
		{
			SetEvent( mDataMutex );
			return;
		}

		if ( mCurCalFFTIndex == -1 )
		{
			if ( !mDataTransfer->Seek( 0.0, 0.0, CLinearTransfer::SET_SAMPLE ) )
			{
				SetEvent( mDataMutex );
				return;
			}
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
			SetEvent( mDataMutex );
			return;
		}

		mCurCalFFTIndex = tCurFFTIndex;
		
		GetFFTCalPeriod( tLength );
		
		if ( !mFourierTransform.FFT( mVoltages, mFourierCalLength, mCurSampleRate, mFourierResData, mFourierResHz ) )
		{
			SetEvent( mDataMutex );
			return;
		}
		
		HSDouble tMinValue = 0;
		HSDouble tMaxValue = 0;
		FilterWave( tMinValue, tMaxValue );

		tMinValue = HSInt( ( tMinValue - 9.99999999 ) / 10 ) * 10 - 5;
		tMaxValue = HSInt( ( tMaxValue + 9.99999999 ) / 10 ) * 10 + 5;

		mTotalAmplitude = tMaxValue - tMinValue;
		mBeginAmplitude = tMaxValue;
		
		mStaticFFT.SetYValue( mTotalAmplitude, mBeginAmplitude );
		mStaticFFT.SetData( mFourierResHz, mFourierResData, mFourierCalLength / 2 + 1 );
		mStaticFFT.Invalidate();
	}

	SetEvent( mDataMutex );
}

HSVoid CGraphicFFT::GetFFTCalPeriod( HSInt tLength )
{
	memset( mVoltages, 0, sizeof( HSFloat ) * mFourierCalLength );

	if ( mTmpVotagesLength < tLength )
	{
		if ( mTmpVoltages != NULL )
		{
			delete[] mTmpVoltages;
		}

		mTmpVoltages = new HSFloat[ tLength ];
		mTmpVotagesLength = tLength;
	}

	HSFloat tTopVoltage = 0;
	HSInt tTopVoltageIndex = 0;
	HSInt tEachSampleSize = mDataHandler->EachSampleSize();	
	for ( HSInt i = 0; i < tLength; i++ )
	{
		mTmpVoltages[ i ] = ( HSFloat )mDataHandler->VoltageWithPointer( &mBuffer[ i * tEachSampleSize ] );
		if ( mTmpVoltages[ i ] > tTopVoltage )
		{
			tTopVoltage = mTmpVoltages[ i ];
			tTopVoltageIndex = i;
		}
	}

	HSInt tStartIndex = tTopVoltageIndex - ( mFourierCalLength / 2 );
	if ( tTopVoltageIndex + ( mFourierCalLength / 2 ) > tLength )
	{
		tStartIndex = tLength - mFourierCalLength;
	}
	
	tStartIndex = max( 0, tStartIndex );
	

	tLength = min( tLength, mFourierCalLength );
	memcpy( mVoltages, &mTmpVoltages[ tStartIndex ], sizeof( HSFloat ) * tLength );
	/*
	static HSBool sDone = HSFalse;
	if ( !sDone )
	{
		ofstream tStream( "FFTData.txt" );
		for ( HSInt i = 0; i < tLength; i++ )
		{
			tStream << mVoltages[ i ] << "\n";
		}
		tStream.close();
		sDone = HSTrue;
	}
	*/

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

HSVoid CGraphicFFT::InitDialog()
{
	mPopupMenu.LoadMenu( IDR_POPUP_FFT_C );

	CRect tRect;
	this->GetClientRect( &tRect );
	mStaticFFT.Create( "", WS_CHILD | WS_VISIBLE, tRect, this );

	mStaticGraphic = &mStaticFFT;
	/*
	mHorValue = new HSDouble[ 32 * 1024 ];
	mVerValue = new HSDouble[ 32 * 1024 ];

	static HSInt sShowFileData = 149;
	HSChar tFileName[ 256 ] = { 0 };
	sprintf_s( tFileName, "%d.txt", sShowFileData );
	ifstream tStream( tFileName );
	if ( tStream.is_open() )
	{
		HSInt tIndex = 0;
		string tLine;
		HSDouble tValue1;
		HSDouble tValue2;
		while ( getline( tStream, tLine ) )
		{
			istringstream tLineStream( tLine );
			tLineStream >> tValue1 >> tValue2;
			
			{
				mHorValue[ tIndex ] = tValue1;
				mVerValue[ tIndex ] = tValue2;
				tIndex++;
			}
		}

		tStream.close();

		mHorIndex = tIndex;
		mStaticFFT.SetData( mHorValue, mVerValue, tIndex );
		mStaticGraphic->SetXValue( 60, 0 );
		mStaticGraphic->SetYValue( 1000, 1000 );
		mStaticGraphic->SetTitle( tFileName );
		mStaticGraphic->Invalidate();
	}

	sShowFileData++;
	*/
	
	mStaticGraphic->SetXValue( mCurSampleRate / 2.0, 0 );
	mStaticGraphic->SetYValue( mTotalAmplitude, mBeginAmplitude );
	mStaticGraphic->Invalidate();
	
}


HSBool CGraphicFFT::GraphicNeedRefresh()
{
	mStaticFFT.SetXValue( mCurSampleRate / 2.0, 0 );
	RefreshFFT();

	return HSTrue;
}

HSVoid CGraphicFFT::StepGraphic( HSInt tDelta )
{
	RefreshFFT( tDelta );
}

HSVoid CGraphicFFT::ZoomInGraphic( CRect &tViewRect )
{		
	WaitForSingleObject( mDataMutex, INFINITE );

	HSDouble tXValueOffset = max( 0.0, mZoomInRect.left - mStaticGraphic->LeftDelta() ) / tViewRect.Width() * mStaticGraphic->TotalXValue();
	HSDouble tNewXValue = ( HSDouble )( mZoomInRect.Width() ) / tViewRect.Width() * mStaticGraphic->TotalXValue();

	HSDouble tBeginYValue = mStaticGraphic->BeginYValue() - max( 0.0, mZoomInRect.top - mStaticGraphic->TopDelta() ) / tViewRect.Height() * mStaticGraphic->TotalYValue();
	HSDouble tTotalYValue = ( HSDouble )mZoomInRect.Height() / tViewRect.Height() * mStaticGraphic->TotalYValue();

	mStaticGraphic->SetXValue( tNewXValue, mStaticGraphic->BeginXValue() + tXValueOffset );
	mStaticGraphic->SetYValue( tTotalYValue, tBeginYValue );
	mStaticGraphic->Invalidate();

	SetEvent( mDataMutex );
}

HSVoid CGraphicFFT::DragGraphic( CRect &tViewRect, CPoint tPoint )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	HSInt tYOffset = mPrevPoint.y - tPoint.y;
	HSDouble tYValuePerDigit = mStaticGraphic->TotalYValue() / tViewRect.Height();
	HSDouble tBeginYValue = mStaticGraphic->BeginYValue() - tYOffset * tYValuePerDigit;
	mStaticGraphic->SetYValue( mStaticGraphic->TotalYValue(), tBeginYValue );

	HSInt tXOffset = mPrevPoint.x - tPoint.x;
	HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticGraphic->TotalXValue();
	mStaticGraphic->SetXValue( mStaticGraphic->TotalXValue(), mStaticGraphic->BeginXValue() + tXOffsetValue );
	mStaticGraphic->Invalidate();

	SetEvent( mDataMutex );
}


HSBool CGraphicFFT::IsDataExportNeeded()
{
	return mFourierResData != NULL;
}

HSVoid CGraphicFFT::ExportData( CDataExportHelper &tDataExport )
{
	HSChar tBuf[ 1024 ];

	sprintf_s( tBuf, "设备：%s, 通道: %d, 采样率: %f (MHz)\n\n", mDataHandler->Name().c_str(), mChannel + 1, mDataHandler->SampleRate( mDataIdentifier ) / 1000000.0 );
	tDataExport.Write( tBuf );

	for ( HSInt i = 0; i < mFourierCalLength; i++ )
	{
		sprintf_s( tBuf, "%f\n", mFourierResData[ i ] );
		tDataExport.Write( tBuf );
	}
}

HSVoid CGraphicFFT::ResetData( CMainFrame* pMainFrame )
{
	/*
	mStaticGraphic->SetXValue( 60, 0 );
	mStaticGraphic->SetYValue( 1000, 1000 );
	mStaticGraphic->Invalidate();
	return;
	*/

	WaitForSingleObject( mDataMutex, INFINITE );

	mStaticGraphic->SetXValue( mCurSampleRate / 2.0, 0 );
	mCurCalFFTIndex = -1;

	SetEvent( mDataMutex );
	
	if ( pMainFrame->IsStart() )
	{
		Start();
	}
	else
	{
		RefreshFFT( 1 );
	}
}

HSVoid CGraphicFFT::ResetGrahpic()
{
	WaitForSingleObject( mDataMutex, INFINITE );

	mStaticGraphic->SetXValue( mCurSampleRate / 2.0, 0 );
	mStaticGraphic->SetYValue( mTotalAmplitude, mBeginAmplitude );
	mStaticGraphic->Invalidate();
	
	SetEvent( mDataMutex );
}