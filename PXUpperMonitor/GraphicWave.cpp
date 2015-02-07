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
#include "ArgCalculator.h"
// CGraphicWave dialog


CGraphicWave::CGraphicWave(CWnd* pParent /*=NULL*/)
	: CNormalGraphic( pParent )
{	
	mBuffer = NULL;
	mSegmentBuffer = NULL;

	mDataTransfer = NULL;
	mChannel = 0;

	mRefreshSecond = 1.0;

	mTotalVoltage = 10.0;
	mTotalSecond = 1.0;

	mBeginSecondOffset = 0;

	mArgTransfer = NULL;

	mCurWaveIndex = -1;

	mOnlyHitWaves = HSFalse;
	mVoltageOffset = 0;
}

CGraphicWave::~CGraphicWave()
{
	if ( mBuffer != NULL )
	{
		delete[] mBuffer;
		delete[] mSegmentBuffer;
	}

	if ( mDataTransfer != NULL )
	{
		delete mDataTransfer;
	}

	if ( mArgTransfer != NULL )
	{
		delete mArgTransfer;		
	}
}

IGraphicPropertyProtocol * CGraphicWave::CreatePropertyDlg( CWnd *pParent )
{
	CWaveProperty *pWaveProperty = new CWaveProperty;
	pWaveProperty->Create( IDD_NORMALPROPERTY, pParent );
	return pWaveProperty;
}

HSBool CGraphicWave::Start()
{	
	if ( mDataTransfer != NULL && mDataHandler != NULL )
	{		
		mRefreshThread->Start();
		return HSTrue;
	}

	return HSFalse;	
}


HSVoid CGraphicWave::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{		
	WaitForSingleObject( mDataMutex, INFINITE );		

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

	mCurWaveIndex = -1;

	if ( mDataHandler != NULL )
	{
		if ( mBuffer != NULL )
		{
			delete[] mBuffer;
			delete[] mSegmentBuffer;
		}

		mBuffer = new HSChar[ mDataHandler->SampleRate( tIdentifier ) * mDataHandler->EachSampleSize() ];
		mSegmentBuffer = new HSChar[ mDataHandler->SampleRate( tIdentifier ) * mDataHandler->EachSampleSize() ];

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
}

HSBool CGraphicWave::Save( CINIConfig *pIniConfig, string tGroup )
{
	pIniConfig->SetValue( "Voltage", this->mTotalVoltage, tGroup );
	pIniConfig->SetValue( "Second", this->mTotalSecond, tGroup );
	pIniConfig->SetValue( "HitWaveOnly", this->mOnlyHitWaves, tGroup );
	pIniConfig->SetValue( "VoltageOffset", this->mVoltageOffset, tGroup );

	return HSTrue;
}

HSBool CGraphicWave::Load( CINIConfig *pIniConfig, string tGroup )
{	
	pIniConfig->ValueWithKey( "Voltage", this->mTotalVoltage, tGroup );
	pIniConfig->ValueWithKey( "Second", this->mTotalSecond, tGroup );
	pIniConfig->ValueWithKey( "HitWaveOnly", this->mOnlyHitWaves, tGroup );
	pIniConfig->ValueWithKey( "VoltageOffset", this->mVoltageOffset, tGroup );

	return HSTrue;
}

HSVoid CGraphicWave::SetVoltageOffset( HSDouble tOffset )
{
	mVoltageOffset = tOffset;
	mStaticWave.SetVoltageOffset( tOffset );
	mStaticWave.Invalidate();
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

HSBool CGraphicWave::RefreshArgWave( HSInt tDirection )
{
	if ( mArgTransfer == NULL )
	{
		return HSFalse;
	}

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

	HSInt tCurWaveIndex = mCurWaveIndex + tDirection;	
	tCurWaveIndex = max( 0, tCurWaveIndex );

	if ( ( tCurWaveIndex == mCurWaveIndex  && tDirection != 0 ) || tCurWaveIndex >= ( HSInt )( mArgList.size() ) )
	{			
		return  HSTrue;
	}

	if ( mCurWaveIndex < 0 && !mDataTransfer->Seek( 0.0, 0.0, CLinearTransfer::SET_SAMPLE ) )
	{
		return HSFalse;
	}
	
	HSDouble tSecond = mArgList[ tCurWaveIndex ].NBeginTime / 1000000000.0 - mRefreshSecond / 4;

	if ( mDataTransfer->Seek( max( 0, tSecond ), mRefreshSecond, CLinearTransfer::SET_SAMPLE ) )
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

	mCurWaveIndex = tCurWaveIndex;

	return HSTrue;
}

HSBool CGraphicWave::RefreshSegmentWave( HSInt tDirection )
{
	if ( mCurWaveIndex < 0 && !mDataTransfer->Seek( 0.0, 0.0, CLinearTransfer::SET_SAMPLE ) )
	{	
		return HSFalse;
	}

	HSInt tCurWaveIndex = mCurWaveIndex + tDirection;	
	tCurWaveIndex = max( 0, tCurWaveIndex );
	if ( tCurWaveIndex == mCurWaveIndex && tDirection != 0 )
	{			
		return HSTrue;
	}	

	HSChar *pBuf = NULL;
	HSInt tLength = mDataTransfer->Read( tCurWaveIndex, pBuf );
	if ( tLength == 0 )
	{
		return HSFalse;
	}

	memcpy( mBuffer, pBuf, tLength );
	memcpy( mSegmentBuffer, pBuf, tLength );

	mBeginSecondOffset = 0;
	mCurWaveIndex = tCurWaveIndex;

	tLength /= mDataHandler->EachSampleSize();
	mSampleLength = tLength;
	
	mRefreshSecond = tLength;
	mRefreshSecond /= mDataHandler->SampleRate( mDataIdentifier );
	mStaticWave.SetVoltage( mTotalVoltage, mTotalVoltage / 2 );
	mStaticWave.SetSeconds( mRefreshSecond, 0 );
	mStaticWave.SetData( mBuffer, tLength, 0 );
	mStaticWave.Invalidate();

	return HSTrue;
}

HSVoid CGraphicWave::AdjustSegmentWave()
{
	HSInt tEachSampleSize = mDataHandler->EachSampleSize();	
	HSDouble tTotalTime = mSampleLength;
	tTotalTime /= mDataHandler->SampleRate( mDataIdentifier );
	HSInt tBeginSample = HSInt( max( 0, mStaticWave.BeginSecond() ) / tTotalTime * mSampleLength );
	if ( tBeginSample >= mSampleLength )
	{
		mStaticWave.SetData( NULL, 0, 0 );
	}
	else
	{		
		memcpy( mBuffer, &mSegmentBuffer[ tBeginSample * tEachSampleSize ], ( mSampleLength - tBeginSample ) * tEachSampleSize );
		mStaticWave.SetData( mBuffer, mSampleLength - tBeginSample, tBeginSample * tTotalTime / mSampleLength );	
	}	
	
	mStaticWave.Invalidate();
}

HSVoid CGraphicWave::SetTime( HSDouble tSecond )
{
	mTotalSecond = tSecond;

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	this->ResetData( pMainFrame );
}

HSVoid CGraphicWave::SetVoltage( HSDouble tVoltage )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	HSDouble tCenterVoltage = mStaticWave.BeginVoltage() - mStaticWave.TotalVoltage() / 2;

	mTotalVoltage = tVoltage;		
	mStaticWave.SetVoltage( mTotalVoltage, tCenterVoltage + mTotalVoltage / 2 );
	mStaticWave.Invalidate();

	SetEvent( mDataMutex );
}


HSVoid CGraphicWave::InitDialog()
{
	mPopupMenu.LoadMenu( IDR_POPUP_WAVE_CH );

	CRect tRect;
	this->GetClientRect( &tRect );
	mStaticWave.Create( "", WS_CHILD | WS_VISIBLE, tRect, this );

	mStaticGraphic = &mStaticWave;	

	this->mRefreshSecond = mTotalSecond;
	mStaticWave.SetVoltage( mTotalVoltage, mTotalVoltage / 2 );
	mStaticWave.SetSeconds( mRefreshSecond, 0 );
	mStaticWave.SetVoltageOffset( this->mVoltageOffset );
	mStaticWave.Invalidate();
}

HSVoid CGraphicWave::ViewResized( CRect &tRect )
{
	mStaticWave.MoveWindow( tRect, true );
	mStaticWave.SetVoltage( mStaticWave.TotalVoltage(), mStaticWave.BeginVoltage() );
	mStaticWave.SetSeconds( mRefreshSecond, mStaticWave.BeginSecond() );
	mStaticWave.ResizeWave();
	mStaticWave.Invalidate();
}

HSBool CGraphicWave::GraphicNeedRefresh()
{
	WaitForSingleObject( mDataMutex, INFINITE );

	if ( mDataHandler == NULL )
	{
		SetEvent( mDataMutex );
		return HSTrue;
	}
	
	if ( mDataHandler->DataIsInSegent() )
	{
		RefreshSegmentWave();
	}
	else if ( this->mOnlyHitWaves )
	{
		RefreshArgWave();
	}
	else
	{		
		if ( mDataTransfer->Seek( mRefreshSecond, mRefreshSecond ) && mDataTransfer->NeedRefresh() )
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
	}		

	SetEvent( mDataMutex );

	return HSTrue;
}

HSVoid CGraphicWave::StepGraphic( HSInt tDelta )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	if ( mDataHandler != NULL && mDataHandler->DataIsInSegent() )
	{				
		this->RefreshSegmentWave( tDelta );
	}
	else if ( this->mOnlyHitWaves )
	{
		mRefreshSecond = mTotalSecond;
		mStaticWave.SetVoltage( mTotalVoltage, mTotalVoltage / 2 );
		mStaticWave.SetSeconds( mRefreshSecond, 0 );

		this->RefreshArgWave( tDelta );
	}
	else
	{
		mStaticWave.SetSeconds( mRefreshSecond, mStaticWave.BeginSecond() + tDelta * mRefreshSecond );
		RefreshWave();
	}

	SetEvent( mDataMutex );
}

HSVoid CGraphicWave::ZoomInGraphic( CRect &tViewRect )
{		
	WaitForSingleObject( mDataMutex, INFINITE );			

	HSDouble tSeekSecond = max( 0.0, mZoomInRect.left - mStaticWave.LeftDelta() ) / tViewRect.Width() * mRefreshSecond;
	HSDouble tNewRefreshSecond = ( HSDouble )( mZoomInRect.Width() ) / tViewRect.Width() * mRefreshSecond;

	HSDouble tBeginVoltage = mStaticWave.BeginVoltage() - max( 0.0, mZoomInRect.top - mStaticWave.TopDelta() ) / tViewRect.Height() * mStaticWave.TotalVoltage();
	HSDouble tTotalVoltage = ( HSDouble )mZoomInRect.Height() / tViewRect.Height() * mStaticWave.TotalVoltage();

	mStaticWave.SetSeconds( tNewRefreshSecond, mStaticWave.BeginSecond() + tSeekSecond );
	mStaticWave.SetVoltage( tTotalVoltage, tBeginVoltage );
	mRefreshSecond = mStaticWave.TotalSecond();

	if ( mDataHandler != NULL && mDataHandler->DataIsInSegent() )
	{			
		this->AdjustSegmentWave();			
	}
	else
	{
		RefreshWave();			
	}

	SetEvent( mDataMutex );
}

HSVoid CGraphicWave::DragGraphic( CRect &tViewRect, CPoint tPoint )
{
	WaitForSingleObject( mDataMutex, INFINITE );		

	HSInt tYOffset = mPrevPoint.y - tPoint.y;			
	HSDouble tVoltagePerDigit = mStaticWave.TotalVoltage() / tViewRect.Height();
	HSDouble tBeginVoltage = mStaticWave.BeginVoltage() - tYOffset * tVoltagePerDigit;			
	mStaticWave.SetVoltage( mStaticWave.TotalVoltage(), tBeginVoltage );		

	HSInt tXOffset = mPrevPoint.x - tPoint.x;
	HSDouble tSeekSecond = ( HSDouble )tXOffset / tViewRect.Width() * mRefreshSecond;		
	mStaticWave.SetSeconds( mRefreshSecond, mStaticWave.BeginSecond() + tSeekSecond );

	if ( mDataHandler != NULL && mDataHandler->DataIsInSegent() )
	{			
		this->AdjustSegmentWave();			
	}
	else
	{
		RefreshWave();			
	}

	SetEvent( mDataMutex );	
}


HSBool CGraphicWave::IsDataExportNeeded()
{
	return mStaticWave.DataLength() > 0;
}

HSVoid CGraphicWave::ExportData( CDataExportHelper &tDataExport )
{
	mStaticWave.ExportDataToFile( tDataExport );
}

HSVoid CGraphicWave::ResetData( CMainFrame* pMainFrame )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	this->mRefreshSecond = mTotalSecond;
	mStaticWave.SetVoltage( mTotalVoltage, mTotalVoltage / 2 );
	mStaticWave.SetSeconds( mRefreshSecond, 0 );

	mBeginSecondOffset = 0;
	mCurWaveIndex = -1;
	if ( this->mDataHandler && mDataTransfer->Seek( 0, mRefreshSecond, CLinearTransfer::SET_SAMPLE ) )
	{		
		if ( pMainFrame->IsStart() )
		{
			Start();
		}
		else if( mDataHandler->DataIsInSegent() )
		{
			RefreshSegmentWave();
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
	
	SetEvent( mDataMutex );
}

HSVoid CGraphicWave::ResetGrahpic()
{
	WaitForSingleObject( mDataMutex, INFINITE );

	if ( mDataHandler != NULL && mDataHandler->DataIsInSegent() )
	{
		this->RefreshSegmentWave( 0 );
	}
	else if ( this->mOnlyHitWaves )
	{
		mRefreshSecond = mTotalSecond;
		mStaticWave.SetVoltage( mTotalVoltage, mTotalVoltage / 2 );
		mStaticWave.SetSeconds( mRefreshSecond, 0 );

		this->RefreshArgWave( 0 );
	}
	else
	{
		mRefreshSecond = mTotalSecond;
		mStaticWave.SetVoltage( mTotalVoltage, mTotalVoltage / 2 );
		mStaticWave.SetSeconds( mRefreshSecond, 0 );
		RefreshWave();
	}

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	if ( pMainFrame->IsStart() )
	{
		Start();
	}

	SetEvent( mDataMutex );			
}