// GraphicScatter.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GraphicScatter.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "ScatterProperty.h"
#include "HSLogProtocol.h"


// CGraphicScatter dialog


CGraphicScatter::CGraphicScatter(CWnd* pParent /*=NULL*/)
	: CNormalGraphic( pParent ), mXArg( 0, HSTrue ), mYArg( 2, HSFalse )
{
}

CGraphicScatter::~CGraphicScatter()
{
	CleanChannelArgs();
}

HSBool CGraphicScatter::Start()
{
	if ( mChannelArgs.size() > 0 )
	{		
		mRefreshThread->Start();
		return HSTrue;
	}
	return HSFalse;
}


HSBool CGraphicScatter::Save( CINIConfig *pIniConfig, string tGroup )
{
	pIniConfig->SetValue( "XAxis", this->mXArg.ArgName(), tGroup );
	pIniConfig->SetValue( "YAxis", this->mYArg.ArgName(), tGroup );

	return HSTrue;
}

HSBool CGraphicScatter::Load( CINIConfig *pIniConfig, string tGroup )
{
	HSString tXArgName = "";
	HSString tYArgName = "";
	pIniConfig->ValueWithKey( "XAxis", tXArgName, tGroup );
	pIniConfig->ValueWithKey( "YAxis", tYArgName, tGroup );

	mXArg.SetArg( tXArgName );
	mYArg.SetArg( tYArgName );

	return HSTrue;
}
	
IGraphicPropertyProtocol * CGraphicScatter::CreatePropertyDlg( CWnd *pParent )
{
	CScatterProperty *pScatterProperty = new CScatterProperty;
	pScatterProperty->Create( IDD_NORMALPROPERTY, pParent );
	return pScatterProperty;
}

HSVoid CGraphicScatter::SetRelationTitle()
{	
	CString tTitle = "";
	vector< HSUInt > tChannels;
	HSInt tChannelCount = mDataIdentifier.NumOfChannels( &tChannels );
	if ( mDataHandler && tChannelCount > 0 )
	{		
		CString tStrChannels = "所有通道";
		if ( tChannelCount != mDataHandler->ChannelNum() )
		{
			tStrChannels = "";
			for ( HSUInt i = 0; i < tChannels.size(); i++ )
			{
				CString tStrChannel;
				tStrChannel.Format( ( tStrChannels == "" ? "%d" : ", %d" ), tChannels[ i ] + 1 );
				tStrChannels += tStrChannel;
			}
		}

		tTitle.Format( "%s < %s > ", mDataHandler->Name().c_str(), tStrChannels );
	}
	else
	{
		tTitle = "DEVICE < CHANNEL > ";
	}

	CString tStrRelation;
	tStrRelation.Format( "%s%s 对 %s%s", mYArg.ArgName().c_str(), mYArg.ArgUnit().c_str(), mXArg.ArgName().c_str(), mXArg.ArgUnit().c_str() );

	mStaticScatter.SetTitle( tTitle + tStrRelation );
}

HSVoid CGraphicScatter::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	CleanChannelArgs();

	this->mDataIdentifier = tIdentifier;

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	DEVICE_CH_IDENTIFIER tCardIdentifer( tIdentifier.CARD_IDENTIFIER, tIdentifier.CARD_INDEX );
	mDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( tCardIdentifer );

	HSDouble tDuration = 0;
	HSDouble tMinValue = 0;
	mXArg.GetDesiredShowParam( tDuration, tMinValue );
	mStaticScatter.SetXValue( tDuration, 0 );		
	mCurrentXValue = tDuration;

	mYArg.GetDesiredShowParam( tDuration, tMinValue );
	mStaticScatter.SetYValue( tDuration, tDuration );
	mCurrentYValue = tDuration;

	SetRelationTitle();
	mStaticScatter.Invalidate();

	vector< HSUInt > tChannels;
	if ( mDataHandler && tIdentifier.NumOfChannels( &tChannels ) > 0 )
	{				
		for ( HSUInt i = 0; i < tChannels.size(); i++ )
		{
			tCardIdentifer.InitChannel( tChannels[ i ] );
			ChannelArgInfo tChannelArgInfo;			
			tChannelArgInfo.ArgTransfer = new CArgListTransfer( mDataHandler, tCardIdentifer );
			tChannelArgInfo.Index = 0;
			tChannelArgInfo.Channel = tChannels[ i ];

			mChannelArgs.push_back( tChannelArgInfo );
		}		

		if ( pMainFrame->IsStart() )
		{
			this->Start();
		}
	}	

	SetEvent( mDataMutex );	
}

HSVoid CGraphicScatter::SetXArg( HSString tArgName )
{
	mXArg.SetArg( tArgName );
	CleanChannelArgs();

	SetRelationTitle();

	HSDouble tDuration = 0;
	HSDouble tMinValue = 0;
	mXArg.GetDesiredShowParam( tDuration, tMinValue );

	mStaticScatter.SetXValue( tDuration, 0 );	
	mStaticScatter.SetMinXValue( tMinValue );

	mCurrentXValue = tDuration;

	mStaticScatter.SetIsValueAdded( mXArg.IsValueAdded() );	

	mStaticScatter.Invalidate();
}

HSVoid CGraphicScatter::SetYArg( HSString tArgName )
{
	mYArg.SetArg( tArgName );
	CleanChannelArgs();

	SetRelationTitle();

	HSDouble tDuration = 0;
	HSDouble tMinValue = 0;
	mYArg.GetDesiredShowParam( tDuration, tMinValue );

	mCurrentYValue = tDuration;

	mStaticScatter.SetYValue( tDuration, tDuration );	
	mStaticScatter.SetMinYValue( tMinValue );
	mStaticScatter.Invalidate();

	mCurrentYValue = tDuration;
}

HSVoid CGraphicScatter::CleanChannelArgs()
{
	for ( HSUInt i = 0; i < mChannelArgs.size(); i++ )
	{
		delete mChannelArgs[ i ].ArgTransfer;
	}

	mChannelArgs.clear();
	mArgsValue.clear();
}


HSVoid CGraphicScatter::InitDialog()
{
	mPopupMenu.LoadMenu( IDR_POPUP_SCATTER_C );

	CRect tRect;
	this->GetClientRect( &tRect );
	mStaticScatter.Create( "", WS_CHILD | WS_VISIBLE, tRect, this );

	mStaticGraphic = &mStaticScatter;	

	HSDouble tDuration = 0;
	HSDouble tMinYValue = 0;
	mYArg.GetDesiredShowParam( mCurrentYValue, tMinYValue );	
	mStaticScatter.SetMinYValue( tMinYValue );
	mStaticScatter.SetYValue( mCurrentYValue, mCurrentYValue );

	HSDouble tMinXValue = 0;

	mXArg.GetDesiredShowParam( tDuration, tMinXValue );	
	mStaticScatter.SetXValue( tDuration, 0 );
	mStaticScatter.SetMinXValue( tMinXValue );

	mStaticScatter.SetArgsValue( &mArgsValue );
}

HSBool CGraphicScatter::GraphicNeedRefresh()
{
	for ( HSUInt i = 0; i < mChannelArgs.size(); i++ )	
	{
		mChannelArgs[ i ].ArgTransfer->GetArg( mChannelArgs[ i ].ArgList );
		for ( HSUInt j = mChannelArgs[ i ].Index; j < mChannelArgs[ i ].ArgList.size(); j++ )  
		{
			CStaticScatter::ArgValueInfo tArgValue;

			tArgValue.XValue = mXArg.FetchArg( mChannelArgs[ i ].ArgList[ j ], j, mChannelArgs[ i ].Channel );
			tArgValue.YValue = mYArg.FetchArg( mChannelArgs[ i ].ArgList[ j ], j, mChannelArgs[ i ].Channel );			
			mArgsValue.push_back( tArgValue );
		}

		mChannelArgs[ i ].Index = mChannelArgs[ i ].ArgList.size();
	}
	
	HSDouble tValue = 0;
	list< CStaticScatter::ArgValueInfo >::iterator pIterator = mArgsValue.begin();
	while ( pIterator != mArgsValue.end() )
	{
		tValue = ( mXArg.IsValueAdded() ? tValue + pIterator->YValue : pIterator->YValue );
		if ( tValue > mStaticScatter.TotalYValue() )
		{
			HSDouble tYValue = HSInt( tValue / mCurrentYValue ) * 2 * mCurrentYValue;			
			mStaticScatter.SetYValue( tYValue, tYValue );		

			mCurrentYValue = mStaticScatter.TotalYValue();
		}

		HSDouble tCurXValue = pIterator->XValue;
		if ( tCurXValue > mStaticScatter.TotalXValue() )
		{
			HSDouble tXValue = HSInt( tCurXValue / mCurrentXValue ) * 2 * mCurrentXValue;			
			mStaticScatter.SetXValue( tXValue, 0 );		

			mCurrentXValue = mStaticScatter.TotalXValue();
		}
		
		pIterator++;
	}

	mStaticScatter.Invalidate();

	return HSTrue;
}

HSVoid CGraphicScatter::StepGraphic( HSInt tDelta )
{
	WaitForSingleObject( mDataMutex, INFINITE );		

	mStaticScatter.SetXValue( mStaticScatter.TotalXValue(), mStaticScatter.BeginXValue() + tDelta * mStaticScatter.TotalXValue() );
	mStaticScatter.Invalidate();

	SetEvent( mDataMutex );				
}

HSVoid CGraphicScatter::ZoomInGraphic( CRect &tViewRect )
{		
	WaitForSingleObject( mDataMutex, INFINITE );			

	HSDouble tXValueOffset = max( 0.0, mZoomInRect.left - mStaticScatter.LeftDelta() ) / tViewRect.Width() * mStaticScatter.TotalXValue();
	HSDouble tNewXValue = ( HSDouble )( mZoomInRect.Width() ) / tViewRect.Width() * mStaticScatter.TotalXValue();			

	HSDouble tBeginYValue = mStaticScatter.BeginYValue() - max( 0.0, mZoomInRect.top - mStaticScatter.TopDelta() ) / tViewRect.Height() * mStaticScatter.TotalYValue();
	HSDouble tTotalYValue = ( HSDouble )mZoomInRect.Height() / tViewRect.Height() * mStaticScatter.TotalYValue();

	mStaticScatter.SetXValue( tNewXValue, mStaticScatter.BeginXValue() + tXValueOffset );
	mStaticScatter.SetYValue( tTotalYValue, tBeginYValue );		
	mStaticScatter.Invalidate();

	SetEvent( mDataMutex );
}

HSVoid CGraphicScatter::DragGraphic( CRect &tViewRect, CPoint tPoint )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	HSInt tYOffset = mPrevPoint.y - tPoint.y;			
	HSDouble tYValuePerDigit = mStaticScatter.TotalYValue() / tViewRect.Height();
	HSDouble tBeginYValue = mStaticScatter.BeginYValue() - tYOffset * tYValuePerDigit;			
	mStaticScatter.SetYValue( mStaticScatter.TotalYValue(), tBeginYValue );		

	HSInt tXOffset = mPrevPoint.x - tPoint.x;
	HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticScatter.TotalXValue();		
	mStaticScatter.SetXValue( mStaticScatter.TotalXValue(), mStaticScatter.BeginXValue() + tXOffsetValue );
	mStaticScatter.Invalidate();

	SetEvent( mDataMutex );			
}


HSVoid CGraphicScatter::ResetData( CMainFrame* pMainFrame )
{
	WaitForSingleObject( mDataMutex, INFINITE );		

	mStaticScatter.SetXValue( mCurrentXValue, 0 );
	mStaticScatter.SetYValue( mCurrentYValue, mCurrentYValue );		

	mStaticScatter.Invalidate();

	if ( pMainFrame->IsStart() )
	{
		Start();
	}

	SetEvent( mDataMutex );	
}