// LinearPosition.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "LinearPosition.h"
#include "afxdialogex.h"
#include "LinearPositionProperty.h"
#include "MainFrm.h"
#include "HSLogProtocol.h"
#include "ArgCalculator.h"
#include "DataExportHelper.h"

// CLinearPosition dialog

CLinearPosition::CLinearPosition(CWnd* pParent /*=NULL*/)
	: CNormalGraphic( pParent )
{
	mDragingSensorIndex = -1;
	mMaterialLength = 1000.0;

	mHitRadius = 3;
	mXAxisLength = 2.0;

	mCheckRadius = 20;
	mDragCheckCircle = HSFalse;
	mCheckCircleEnable = HSFalse;

	mStopWhenMouseDown = HSFalse;
}

CLinearPosition::~CLinearPosition()
{
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.ArgTransfer != NULL )
		{
			delete pIterator->second.ArgTransfer;					
		}
		pIterator++;
	}
}

HSBool CLinearPosition::Start()
{
	if ( mPositionSensors.size() > 0 )
	{		
		mRefreshThread->Start();
		return HSTrue;
	}
	return HSFalse;
}

HSBool CLinearPosition::Save( CINIConfig *pIniConfig, string tGroup )
{
	pIniConfig->SetValue( "MaterialLength", this->mMaterialLength, tGroup );
	pIniConfig->SetValue( "XAxisLength", this->mXAxisLength, tGroup );
	pIniConfig->SetValue( "HitRadius", this->mHitRadius, tGroup );
		
	HSChar tBuf[ 1024 ];
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn )
		{
			sprintf_s( tBuf, "Sensor%d_DataIdentifier", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.DataIdentifier.ToString(), tGroup );

			sprintf_s( tBuf, "Sensor%d_Position", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Pos, tGroup );

			sprintf_s( tBuf, "Sensor%d_Forbid", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Forbid, tGroup );
		}
		pIterator++;
	}	
	
	return HSTrue;
}

HSBool CLinearPosition::Load( CINIConfig *pIniConfig, string tGroup )
{		
	pIniConfig->ValueWithKey( "MaterialLength", this->mMaterialLength, tGroup );
	pIniConfig->ValueWithKey( "XAxisLength", this->mXAxisLength, tGroup );
	pIniConfig->ValueWithKey( "HitRadius", this->mHitRadius, tGroup );
	
	HSString tDataIdentifier = "";
	HSDouble tPosition = 0;
	HSBool tForbid = HSFalse;

	HSChar tBufId[ 1024 ];
	HSChar tBufPos[ 1024 ];
	HSChar tBufForbid[ 1024 ];

	for ( HSInt i = 0; i < CLinearPositionProperty::POSITION_SENSOR_MAX_NUM; i++ )
	{
		sprintf_s( tBufId, "Sensor%d_DataIdentifier", i );
		sprintf_s( tBufPos, "Sensor%d_Position", i );
		sprintf_s( tBufForbid, "Sensor%d_Forbid", i );
		if ( pIniConfig->ValueWithKey( tBufId, tDataIdentifier, tGroup )
			&& pIniConfig->ValueWithKey( tBufPos, tPosition, tGroup )
			&& pIniConfig->ValueWithKey( tBufForbid, tForbid, tGroup ) )
		{			
			mPositionSensors[ i ].Pos = tPosition;
			mPositionSensors[ i ].Forbid = tForbid;
			mPositionSensors[ i ].IsOn = HSTrue;	
			mPositionSensors[ i ].DataIdentifier.InitWithString( tDataIdentifier );
			mPositionSensors[ i ].ArgTransfer = NULL;			
			mPositionSensors[ i ].Index = 0;
		}
	}	
	
	return HSTrue;
}

	
IGraphicPropertyProtocol * CLinearPosition::CreatePropertyDlg( CWnd *pParent )
{
	CLinearPositionProperty *pPositionProperty = new CLinearPositionProperty;
	pPositionProperty->Create( IDD_NORMALPROPERTY, pParent );
	return pPositionProperty;
}

HSVoid CLinearPosition::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	mHitsPosition.clear();

	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		this->UpdatePositionSensor( pIterator->first, pIterator->second.Pos, pIterator->second.Forbid, pIterator->second.IsOn, pIterator->second.DataIdentifier );
		pIterator++;
	}

	SetEvent( mDataMutex );	
}

HSVoid CLinearPosition::SetCheckRadius( HSDouble tValue )
{
	mCheckRadius = tValue;
	mStaticRelation.SetCheckRadius( tValue, this->mCheckCircleEnable );
	mStaticRelation.Invalidate();
}

HSVoid CLinearPosition::SetCheckPosX( HSDouble tValue )
{
	mStaticRelation.SetCheckPos( tValue, mStaticRelation.YCheckPos() );
	mStaticRelation.Invalidate();
}

HSVoid CLinearPosition::SetXAxisLength( HSDouble tLength )
{
	mXAxisLength = tLength;
	mStaticRelation.SetXValue( mXAxisLength, 0 );
	mStaticRelation.ResizeWave();
	mStaticRelation.Invalidate();
}

HSVoid CLinearPosition::SetHitRadius( HSInt tHitRadius )
{
	mHitRadius = tHitRadius;
	mStaticRelation.SetHitRadius( mHitRadius );
	mStaticRelation.Invalidate();
}

HSDouble CLinearPosition::DistanceWithX( HSDouble tXPosOne, HSDouble tXPosTue )
{
	HSDouble tX = tXPosOne - tXPosTue;

	return sqrt( tX * tX );
}

HSBool CLinearPosition::CalcHitPosition( HSDouble tPos, HSUInt64 tTime, HSUInt64 tIncreaseTime, HSDouble tEnergy )
{
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	vector< CArgTableManager::ArgItem > *pArgList = NULL;
	HSDouble tNextItemPos = this->mMaterialLength;	
	HSInt tNextSensorIndex = 0;	
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.ArgTransfer != NULL && pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			if ( pIterator->second.Pos > tPos && pIterator->second.Pos < tNextItemPos )
			{
				tNextItemPos = pIterator->second.Pos;
				pArgList = &( pIterator->second.ArgList );

				tNextSensorIndex = pIterator->first;
			}			
		}
		pIterator++;
	}	

	if ( pArgList == NULL )
	{
		return HSTrue;
	}

	HSDouble tVelocity = CArgCalculator::SharedInstance()->GetMaterialVelocity();
	HSDouble tCenterDelta = 25;
	HSDouble tCenterPos = ( tPos + tNextItemPos ) / 2.0;
	
	HSBool tGotPos = HSFalse;
	for ( HSUInt i = 0; i < pArgList->size(); i++ )
	{		
		HSDouble tDuration = ( ( HSInt64 )( tTime - ( *pArgList )[ i ].NBeginTime ) ) / 1000000000.0;
		HSDouble tResPos = ( tDuration * tVelocity + tPos + tNextItemPos ) / 2.0;
		/*
		HSDouble tEnergyOffset = sqrt( abs( tEnergy - ( *pArgList )[ i ].Energy ) );		

		if ( tResPos < tCenterPos - tCenterDelta || tResPos > tCenterPos + tCenterDelta )
		{
			tResPos += ( tEnergyOffset / 2.0 * ( tResPos < tCenterPos ? -1 : 1 ) );		
		}
		*/
		if ( tResPos > tPos && tResPos < tNextItemPos )
		{
			//HS_INFO( "Energy Offset: %f, Dist Offset: %f, tRes Pos: %f, tDistToCenter: %f, Energy: %f", tEnergyOffset, tDistOffset, tResPos, tDistToCenter, ( *pArgList )[ i ].Energy );
			CStaticLinearPosition::HitPosition tHit = { tResPos, CGraphicManager::SharedInstance()->ColorWithEnergy( max( tEnergy, ( *pArgList )[ i ].Energy ) ) };
			mHitsPosition.push_back( tHit );
			tGotPos = HSTrue;
		}
		else if ( tTime < ( *pArgList )[ i ].NBeginTime && i < pArgList->size() )
		{
			return HSTrue;
		}
	}

	return tGotPos;
}

HSVoid CLinearPosition::SetMaterialLength( HSDouble tLength )
{
	mMaterialLength = max( tLength, 10 );
	mStaticRelation.SetMaterialLength( mMaterialLength );
	
	this->AutoSetSensors();
}

HSVoid CLinearPosition::InitPositionSensors()
{
	HSDouble tPosPercent = 0;	
	HSInt tIndex = 0;
	while ( mPositionSensors.size() < 2 )
	{
		if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
		{
			this->UpdatePositionSensor( tIndex, tPosPercent * mMaterialLength, HSFalse, HSTrue, 0 );		
			tPosPercent = 1;
		}
		tIndex++;
	}

	this->SetDataIdentifier( 0 );
}

HSVoid CLinearPosition::SetFocusSensor( HSInt tIndex )
{
	mStaticRelation.SetFocusSensor( tIndex );
	mStaticRelation.Invalidate();
}

HSVoid CLinearPosition::UpdatePositionSensor( HSInt tIndex, HSDouble tPosition, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier )
{
	if ( mPositionSensors.find( tIndex ) != mPositionSensors.end() )
	{
		if ( mPositionSensors[ tIndex ].ArgTransfer != NULL )
		{
			delete mPositionSensors[ tIndex ].ArgTransfer;
			mPositionSensors[ tIndex ].ArgList.clear();
			mPositionSensors[ tIndex ].ArgHandled.clear();			
		}
	}
	else if ( !tIsOn )
	{
		return;
	}

	mPositionSensors[ tIndex ].Pos = tPosition;
	mPositionSensors[ tIndex ].Forbid = tForbid;
	mPositionSensors[ tIndex ].IsOn = tIsOn;	
	mPositionSensors[ tIndex ].DataIdentifier = tDataIdentifier;
	mPositionSensors[ tIndex ].ArgTransfer = NULL;	
	mPositionSensors[ tIndex ].Index = 0;

	if ( tIsOn )
	{
		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

		DEVICE_CH_IDENTIFIER tCardIdentifer( tDataIdentifier.CARD_IDENTIFIER, tDataIdentifier.CARD_INDEX );
		IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( tCardIdentifer );	

		if ( pDataHandler != NULL )
		{			
			mPositionSensors[ tIndex ].ArgTransfer = new CArgListTransfer( pDataHandler, tDataIdentifier );
		}
	}

	RefreshSensors();

	mStaticRelation.Invalidate();	
}

HSVoid CLinearPosition::RefreshSensors()
{
	mSensors.clear();
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			CStaticLinearPosition::SensorInfo tSensor;
			tSensor.Index = pIterator->first;
			tSensor.Pos = pIterator->second.Pos;
			mSensors.push_back( tSensor );
		}
		pIterator++;
	}
	
	mStaticRelation.SetSensors( &mSensors );	
}

CLinearPosition::PositionSensor * CLinearPosition::GetPositionSensor( HSInt tIndex )
{
	if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
	{
		return NULL;
	}

	return &( mPositionSensors[ tIndex ] );
}

HSVoid CLinearPosition::InitDialog()
{
	mPopupMenu.LoadMenu( IDR_POPUP_LINEAR_POSITION_C );

	CRect tRect;
	this->GetClientRect( &tRect );
	mStaticRelation.Create( "", WS_CHILD | WS_VISIBLE, tRect, this );

	mStaticGraphic = &mStaticRelation;	

	mStaticRelation.SetHitsPosition( &mHitsPosition );
	mStaticRelation.SetSensors( &mSensors );
	mStaticRelation.SetMaterialLength( this->mMaterialLength );
	mStaticRelation.SetXValue( mXAxisLength, 0 );
	mStaticRelation.SetHitRadius( mHitRadius );

	mStaticRelation.SetCheckRadius( mCheckRadius, HSFalse );
	mStaticRelation.SetCheckPos( mMaterialLength / 2, 0 );

	InitPositionSensors();
}

HSBool CLinearPosition::GraphicNeedRefresh()
{
	HSDouble tHitDuration = CArgCalculator::SharedInstance()->GetHitFullTime() / 1000000000.0 + 0.0005;
	
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.ArgTransfer != NULL && pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			pIterator->second.ArgTransfer->GetArg( pIterator->second.ArgList );	
			for ( HSUInt i = pIterator->second.Index; i < pIterator->second.ArgList.size(); i++ )
			{
				pIterator->second.ArgHandled.push_back( HSFalse );
			}
			pIterator->second.Index = pIterator->second.ArgList.size();
		}
		pIterator++;
	}

	pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.ArgTransfer != NULL && pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			for ( HSUInt i = 0; i < pIterator->second.ArgList.size(); i++ )
			{								
				if ( pIterator->second.ArgHandled[ i ] )
				{					
					continue;
				}
				else 
				{
					HSDouble tDuration = ( i > 0 ? ( pIterator->second.ArgList[ i ].NBeginTime - pIterator->second.ArgList[ i - 1 ].NBeginTime ) / 1000000000.0 : 1.0 );
					if ( tDuration < tHitDuration )
					{
						pIterator->second.ArgHandled[ i ] = HSTrue;
						continue;
					}
				}
				
				if ( CalcHitPosition( pIterator->second.Pos, pIterator->second.ArgList[ i ].NBeginTime, pIterator->second.ArgList[ i ].IncreaseNTime, pIterator->second.ArgList[ i ].Energy ) )
				{					
					pIterator->second.ArgHandled[ i ] = HSTrue;
				}
			}			
		}
		pIterator++;	
	}	

	mStaticRelation.Invalidate();

	return HSTrue;
}

HSVoid CLinearPosition::StepGraphic( HSInt tDelta )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() + tDelta * mStaticRelation.TotalXValue() );
	mStaticRelation.Invalidate();

	SetEvent( mDataMutex );				
}

HSVoid CLinearPosition::ZoomInGraphic( CRect &tViewRect )
{		
	WaitForSingleObject( mDataMutex, INFINITE );			

	HSDouble tXValueOffset = max( 0.0, mZoomInRect.left - mStaticRelation.LeftDelta() ) / tViewRect.Width() * mStaticRelation.TotalXValue();
	HSDouble tNewXValue = ( HSDouble )( mZoomInRect.Width() ) / tViewRect.Width() * mStaticRelation.TotalXValue();			

	//HSDouble tBeginYValue = mStaticRelation.BeginYValue() - max( 0.0, mZoomInRect.top - mStaticRelation.TopDelta() ) / tViewRect.Height() * mStaticRelation.TotalYValue();
	//HSDouble tTotalYValue = ( HSDouble )mZoomInRect.Height() / tViewRect.Height() * mStaticRelation.TotalYValue();

	mStaticRelation.SetXValue( tNewXValue, mStaticRelation.BeginXValue() + tXValueOffset );
	//mStaticRelation.SetYValue( tTotalYValue, tBeginYValue );		
	mStaticRelation.Invalidate();

	SetEvent( mDataMutex );
}

HSVoid CLinearPosition::DragGraphic( CRect &tViewRect, CPoint tPoint )
{
	WaitForSingleObject( mDataMutex, INFINITE );	

	//HSInt tYOffset = mPrevPoint.y - point.y;			
	//HSDouble tYValuePerDigit = mStaticRelation.TotalYValue() / tViewRect.Height();
	//HSDouble tBeginYValue = mStaticRelation.BeginYValue() - tYOffset * tYValuePerDigit;			
	//mStaticRelation.SetYValue( mStaticRelation.TotalYValue(), tBeginYValue );		

	HSInt tXOffset = mPrevPoint.x - tPoint.x;
	HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticRelation.TotalXValue();		
	mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() + tXOffsetValue );
	mStaticRelation.Invalidate();

	SetEvent( mDataMutex );			
}


HSBool CLinearPosition::IsDataExportNeeded()
{
	return HSTrue;
}

HSVoid CLinearPosition::ExportData( CDataExportHelper &tDataExport )
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	HSChar tBuf[ 1024 ];

	sprintf_s( tBuf, "长度：%.2f (m)\n", mMaterialLength / 1000.0 );
	tDataExport.Write( tBuf ); 

	sprintf_s( tBuf, "\n% 10s\t% 18s\t% 16s\t% 16s\n", "传感器编号", "X(m)", "采集卡", "通道" );
	tDataExport.Write( tBuf ); 

	map< HSInt, PositionSensor >::iterator pSensorIterator = mPositionSensors.begin();
	while ( pSensorIterator != mPositionSensors.end() )
	{
		if ( pSensorIterator->second.IsOn && !pSensorIterator->second.Forbid )
		{
			IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( pSensorIterator->second.DataIdentifier );
			sprintf_s( tBuf, "% 10d\t% 18f\t% 16s\t% 16d\n", 
				pSensorIterator->first + 1, 
				pSensorIterator->second.Pos / 1000.0, 					
				( pDataHandler == NULL ? "" : pDataHandler->Name().c_str() ),
				( pDataHandler == NULL ? -1 : pSensorIterator->second.DataIdentifier.ChannelIndex() + 1 ) );

			tDataExport.Write( tBuf ); 
		}

		pSensorIterator++;
	}		

	sprintf_s( tBuf, "\n% 10s\t% 18s\n", "撞击编号", "X(m)" );
	tDataExport.Write( tBuf ); 

	HSInt tIndex = 1;
	list< CStaticLinearPosition::HitPosition >::iterator pHitIterator = mHitsPosition.begin();
	while ( pHitIterator != mHitsPosition.end() )
	{
		sprintf_s( tBuf, "% 10d\t% 18f\n", tIndex++, ( *pHitIterator ).XPos / 1000.0 );
		tDataExport.Write( tBuf ); 

		pHitIterator++;
	}
}

HSVoid CLinearPosition::ResetData( CMainFrame* pMainFrame )
{
	WaitForSingleObject( mDataMutex, INFINITE );	
	
	mStaticRelation.SetXValue( mXAxisLength, 0 );	

	mStaticRelation.SetYValue( 10, 5 );

	mStaticRelation.Invalidate();

	SetEvent( mDataMutex );
}

HSVoid CLinearPosition::MouseDownWhenPointer(  CRect &tRect, CPoint tPoint )
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	if ( !mDragCheckCircle )
	{
		mDragCheckCircle = mStaticRelation.PositionWithinCheckCircle( tPoint, tRect );
	}

	if ( !pMainFrame->IsStart() && !mDragCheckCircle && mDragingSensorIndex < 0 )
	{
		mDragingSensorIndex = mStaticRelation.SensorInPosition( tPoint, tRect );
	}

	mPrevPoint = tPoint;
}

HSBool CLinearPosition::CustomMouseUpCheck()
{
	if ( mDragCheckCircle )
	{
		mDragCheckCircle = HSFalse;
		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
		pMainFrame->SetFocusedGraphic( this, HSTrue );

		return HSTrue;
	}
	else if ( mDragingSensorIndex >= 0 )
	{
		PositionSensor *pPositionSensor = GetPositionSensor( mDragingSensorIndex );
		if ( pPositionSensor != NULL )
		{
			if ( pPositionSensor->Pos < 0 || pPositionSensor->Pos > this->mMaterialLength )
			{
				pPositionSensor->Forbid = HSTrue;
			}
			else
			{
				pPositionSensor->Forbid = HSFalse;
			}

			this->UpdatePositionSensor( mDragingSensorIndex, pPositionSensor->Pos, pPositionSensor->Forbid, pPositionSensor->IsOn, pPositionSensor->DataIdentifier );
		}

		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
		pMainFrame->SetFocusedGraphic( this, HSTrue );

		return HSTrue;
	}

	return HSFalse;
}

HSVoid CLinearPosition::MouseUpFinish()
{
	mDragingSensorIndex = -1;
}	

HSBool CLinearPosition::CustomMouseMoveCheck( CPoint tPoint )
{
	if ( mDragCheckCircle )
	{
		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		tViewRect = CRect( tViewRect.left + mStaticRelation.LeftDelta(), tViewRect.top + mStaticRelation.TopDelta(), tViewRect.right - mStaticRelation.LeftDelta(), tViewRect.bottom - mStaticRelation.TopDelta() );

		HSInt tXOffset = mPrevPoint.x - tPoint.x;
		HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticRelation.TotalXValue();
		
		mStaticRelation.SetCheckPos( mStaticRelation.XCheckPos() - tXOffsetValue * 1000.0, mStaticRelation.YCheckPos() );
		mStaticRelation.Invalidate();

		mPrevPoint = tPoint;

		return HSTrue;
	}
	else if ( mDragingSensorIndex >= 0 )
	{
		PositionSensor *pPositionSensor = GetPositionSensor( mDragingSensorIndex );
		if ( pPositionSensor != NULL )
		{
			CRect tViewRect;
			this->GetClientRect( &tViewRect );
			tViewRect = CRect( tViewRect.left + mStaticRelation.LeftDelta(), tViewRect.top + mStaticRelation.TopDelta(), tViewRect.right - mStaticRelation.LeftDelta(), tViewRect.bottom - mStaticRelation.TopDelta() );

			HSInt tXOffset = mPrevPoint.x - tPoint.x;
			HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticRelation.TotalXValue();
			this->UpdatePositionSensor( mDragingSensorIndex, pPositionSensor->Pos - tXOffsetValue * 1000.0, pPositionSensor->Forbid, pPositionSensor->IsOn, pPositionSensor->DataIdentifier );			
		}

		mPrevPoint = tPoint;

		return HSTrue;
	}

	return HSFalse;
}

HSVoid CLinearPosition::AutoSetSensors()
{
	HSInt tSensorCount = 0;
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			tSensorCount++;
		}
		pIterator++;
	}

	if ( tSensorCount < 2 )
	{
		return;
	}	

	HSDouble tDistance = this->mMaterialLength / ( tSensorCount - 1 );
	HSDouble tPosition = 0.0;
	pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			pIterator->second.Pos = tPosition;
			tPosition += tDistance;
		}
		pIterator++;
	}

	RefreshSensors();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
	pMainFrame->SetFocusedGraphic( this, HSTrue );
	
	mStaticRelation.Invalidate();	
}

HSVoid CLinearPosition::SwitchCheckCircle()
{
	mCheckCircleEnable = !mCheckCircleEnable;
	mStaticRelation.SetCheckRadius( this->mCheckRadius, mCheckCircleEnable );
	mStaticRelation.SetCheckPos( mMaterialLength / 2, mStaticRelation.YCheckPos() );
	mStaticRelation.Invalidate();
}
