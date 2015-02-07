// GraphicRelation.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GraphicRelation.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "RelationProperty.h"
#include "HSLogProtocol.h"

// CGraphicRelation dialog


CGraphicRelation::CGraphicRelation(CWnd* pParent /*=NULL*/)
	: CNormalGraphic( pParent ), mXArg( 0, HSTrue ), mYArg( 2, HSFalse )
{	
	mUnitDelta = 1;

	mFillRectangle = HSTrue;	
}

CGraphicRelation::~CGraphicRelation()
{
	CleanChannelArgs();
}

HSBool CGraphicRelation::Start()
{
	if ( mChannelArgs.size() > 0 )
	{		
		mRefreshThread->Start();
		return HSTrue;
	}
	return HSFalse;
}

HSBool CGraphicRelation::Save( CINIConfig *pIniConfig, string tGroup )
{
	pIniConfig->SetValue( "XAxis", this->mXArg.ArgName(), tGroup );
	pIniConfig->SetValue( "YAxis", this->mYArg.ArgName(), tGroup );
	pIniConfig->SetValue( "FillRect", mFillRectangle, tGroup );	

	return HSTrue;
}

HSBool CGraphicRelation::Load( CINIConfig *pIniConfig, string tGroup )
{
	HSString tXArgName = "";
	HSString tYArgName = "";
	pIniConfig->ValueWithKey( "XAxis", tXArgName, tGroup );
	pIniConfig->ValueWithKey( "YAxis", tYArgName, tGroup );
	pIniConfig->ValueWithKey( "FillRect", mFillRectangle, tGroup );

	mXArg.SetArg( tXArgName );
	mYArg.SetArg( tYArgName );

	return HSTrue;
}	
	
IGraphicPropertyProtocol * CGraphicRelation::CreatePropertyDlg( CWnd *pParent )
{
	CRelationProperty *pRelationProperty = new CRelationProperty;
	pRelationProperty->Create( IDD_NORMALPROPERTY, pParent );
	return pRelationProperty;
}

HSVoid CGraphicRelation::SetRelationTitle()
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

	mStaticRelation.SetTitle( tTitle + tStrRelation );
}

HSVoid CGraphicRelation::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
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
	mStaticRelation.SetXValue( tDuration, 0 );		
	mCurrentXValue = tDuration;

	mYArg.GetDesiredShowParam( tDuration, tMinValue );
	mStaticRelation.SetYValue( tDuration, tDuration );
	mCurrentYValue = tDuration;

	SetRelationTitle();
	mStaticRelation.Invalidate();

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

HSVoid CGraphicRelation::SetXArg( HSString tArgName )
{
	mXArg.SetArg( tArgName );
	CleanChannelArgs();

	SetRelationTitle();

	HSDouble tDuration = 0;
	HSDouble tMinValue = 0;
	mXArg.GetDesiredShowParam( tDuration, tMinValue );

	mStaticRelation.SetXValue( tDuration, 0 );	
	mStaticRelation.SetMinXValue( tMinValue );

	mCurrentXValue = tDuration;

	mStaticRelation.SetIsValueAdded( mXArg.IsValueAdded() );
	mStaticRelation.SetPeriodValue( &mPeriodValues, mUnitDelta );

	mStaticRelation.Invalidate();
}

HSVoid CGraphicRelation::SetYArg( HSString tArgName )
{
	mYArg.SetArg( tArgName );
	CleanChannelArgs();

	SetRelationTitle();

	HSDouble tDuration = 0;
	HSDouble tMinValue = 0;
	mYArg.GetDesiredShowParam( tDuration, tMinValue );

	mCurrentYValue = tDuration;

	mStaticRelation.SetYValue( tDuration, tDuration );	
	mStaticRelation.SetMinYValue( tMinValue );
	mStaticRelation.Invalidate();

	mCurrentYValue = tDuration;
}

HSVoid CGraphicRelation::SetIsSolidRect( HSBool tValue )
{
	mFillRectangle = tValue;
	mStaticRelation.SetFillRectangle( mFillRectangle );
	mStaticRelation.Invalidate();
}

HSVoid CGraphicRelation::SetUnitDelta( HSDouble tValue )
{
	mUnitDelta = tValue;
	mStaticRelation.SetPeriodValue( &mPeriodValues, mUnitDelta );
	CleanChannelArgs();
	mStaticRelation.Invalidate();
}


HSVoid CGraphicRelation::CleanChannelArgs()
{
	for ( HSUInt i = 0; i < mChannelArgs.size(); i++ )
	{
		delete mChannelArgs[ i ].ArgTransfer;		
	}

	mChannelArgs.clear();
	mPeriodValues.clear();
}


HSVoid CGraphicRelation::InitDialog()
{
	mPopupMenu.LoadMenu( IDR_POPUP_RELATION_C );

	CRect tRect;
	this->GetClientRect( &tRect );
	mStaticRelation.Create( "", WS_CHILD | WS_VISIBLE, tRect, this );

	mStaticGraphic = &mStaticRelation;	

	HSDouble tMinYValue = 0;
	mYArg.GetDesiredShowParam( mCurrentYValue, tMinYValue );	
	mStaticRelation.SetMinYValue( tMinYValue );
	mStaticRelation.SetYValue( mCurrentYValue, mCurrentYValue );

	HSDouble tMinXValue = 0;
	HSDouble tDuration = 0;
	mXArg.GetDesiredShowParam( tDuration, tMinXValue );	
	mStaticRelation.SetXValue( tDuration, 0 );
	mStaticRelation.SetMinXValue( tMinXValue );

	mStaticRelation.SetPeriodValue( &mPeriodValues, mUnitDelta );
	mStaticRelation.SetFillRectangle( mFillRectangle );	
}

HSBool CGraphicRelation::GraphicNeedRefresh()
{
	for ( HSUInt i = 0; i < mChannelArgs.size(); i++ )	
	{
		mChannelArgs[ i ].ArgTransfer->GetArg( mChannelArgs[ i ].ArgList );
		for ( HSUInt j = mChannelArgs[ i ].Index; j < mChannelArgs[ i ].ArgList.size(); j++ )  
		{
			HSDouble tXValue = mXArg.FetchArg( mChannelArgs[ i ].ArgList[ j ], j, mChannelArgs[ i ].Channel );			
			HSInt tIntXValue = ( HSInt )( tXValue / mUnitDelta );
			if ( mPeriodValues.find( tIntXValue ) == mPeriodValues.end() )
			{
				mPeriodValues[ tIntXValue ] = 0;
			}

			mPeriodValues[ tIntXValue ] += mYArg.FetchArg( mChannelArgs[ i ].ArgList[ j ], j, mChannelArgs[ i ].Channel );
		}

		mChannelArgs[ i ].Index = mChannelArgs[ i ].ArgList.size();		
	}	

	HSDouble tValue = 0;
	map< HSInt, HSDouble >::iterator pIterator = mPeriodValues.begin();
	while ( pIterator != mPeriodValues.end() )
	{
		tValue = ( mXArg.IsValueAdded() ? tValue + pIterator->second : pIterator->second );
		if ( tValue > mStaticRelation.TotalYValue() )
		{
			HSDouble tYValue = HSInt( tValue / mCurrentYValue ) * 2 * mCurrentYValue;			
			mStaticRelation.SetYValue( tYValue, tYValue );		

			mCurrentYValue = mStaticRelation.TotalYValue();			
		}
		
		pIterator++;
	}

	pIterator = mPeriodValues.end();
	if ( pIterator != mPeriodValues.begin() )
	{
		pIterator--;

		if ( pIterator->first > mStaticRelation.TotalXValue() )
		{		
			HSDouble tXValue = HSInt( pIterator->first / mCurrentXValue ) * 2 * mCurrentXValue;			
			mStaticRelation.SetXValue( tXValue, 0 );		

			mCurrentXValue = mStaticRelation.TotalXValue();
		}		
	}

	mStaticRelation.Invalidate();

	return HSTrue;
}

HSVoid CGraphicRelation::StepGraphic( HSInt tDelta )
{
	WaitForSingleObject( mDataMutex, INFINITE );		

	mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() + tDelta * mStaticRelation.TotalXValue() );
	mStaticRelation.Invalidate();

	SetEvent( mDataMutex );				
}

HSVoid CGraphicRelation::ZoomInGraphic( CRect &tViewRect )
{		
	WaitForSingleObject( mDataMutex, INFINITE );			

	HSDouble tXValueOffset = max( 0.0, mZoomInRect.left - mStaticRelation.LeftDelta() ) / tViewRect.Width() * mStaticRelation.TotalXValue();
	HSDouble tNewXValue = ( HSDouble )( mZoomInRect.Width() ) / tViewRect.Width() * mStaticRelation.TotalXValue();			

	HSDouble tBeginYValue = mStaticRelation.BeginYValue() - max( 0.0, mZoomInRect.top - mStaticRelation.TopDelta() ) / tViewRect.Height() * mStaticRelation.TotalYValue();
	HSDouble tTotalYValue = ( HSDouble )mZoomInRect.Height() / tViewRect.Height() * mStaticRelation.TotalYValue();

	mStaticRelation.SetXValue( tNewXValue, mStaticRelation.BeginXValue() + tXValueOffset );
	mStaticRelation.SetYValue( tTotalYValue, tBeginYValue );		
	mStaticRelation.Invalidate();

	SetEvent( mDataMutex );
}

HSVoid CGraphicRelation::DragGraphic( CRect &tViewRect, CPoint tPoint )
{
	WaitForSingleObject( mDataMutex, INFINITE );		

	HSInt tYOffset = mPrevPoint.y - tPoint.y;			
	HSDouble tYValuePerDigit = mStaticRelation.TotalYValue() / tViewRect.Height();
	HSDouble tBeginYValue = mStaticRelation.BeginYValue() - tYOffset * tYValuePerDigit;			
	mStaticRelation.SetYValue( mStaticRelation.TotalYValue(), tBeginYValue );		

	HSInt tXOffset = mPrevPoint.x - tPoint.x;
	HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticRelation.TotalXValue();		
	mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() + tXOffsetValue );
	mStaticRelation.Invalidate();

	SetEvent( mDataMutex );			
}


HSVoid CGraphicRelation::ResetData( CMainFrame* pMainFrame )
{
	WaitForSingleObject( mDataMutex, INFINITE );		
			
	mStaticRelation.SetXValue( mCurrentXValue, 0 );	
	mStaticRelation.SetYValue( mCurrentYValue, mCurrentYValue );		

	mStaticRelation.Invalidate();
	
	if ( pMainFrame->IsStart() )
	{
		Start();
	}

	SetEvent( mDataMutex );		
}