// PlanePosition.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "PlanePosition.h"
#include "afxdialogex.h"
#include "PlanePositionProperty.h"
#include "MainFrm.h"
#include <algorithm>
#include "ArgCalculator.h"
#include "HSLogProtocol.h"
#include "DataExportHelper.h"

// CPlanePosition dialog

CPlanePosition::CPlanePosition(CWnd* pParent /*=NULL*/)
	: CNormalGraphic( pParent )
{	
	mDragingSensorIndex = -1;
	mMaterialLength = 1000.0;
	mMaterialWidth = 1000.0;

	mXAxisLength = 2;
	mYAxisLength = 1;
	mHitRadius = 3;

	mCheckRadius = 20;
	mDragCheckCircle = HSFalse;
	mCheckCircleEnable = HSFalse;

	mStopWhenMouseDown = HSFalse;
}

CPlanePosition::~CPlanePosition()
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

HSBool CPlanePosition::Start()
{
	if ( mPositionSensors.size() > 0 )
	{		
		mRefreshThread->Start();
		return HSTrue;
	}
	return HSFalse;
}

HSBool CPlanePosition::Save( CINIConfig *pIniConfig, string tGroup )
{
	pIniConfig->SetValue( "MaterialLength", this->mMaterialLength, tGroup );
	pIniConfig->SetValue( "MaterialWidth", this->mMaterialWidth, tGroup );

	pIniConfig->SetValue( "XAxisLength", this->mXAxisLength, tGroup );
	pIniConfig->SetValue( "YAxisLength", this->mYAxisLength, tGroup );
	pIniConfig->SetValue( "HitRadius", this->mHitRadius, tGroup );
		
	HSChar tBuf[ 1024 ];
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn )
		{
			sprintf_s( tBuf, "Sensor%d_DataIdentifier", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.DataIdentifier.ToString(), tGroup );

			sprintf_s( tBuf, "Sensor%d_XPosition", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.XPos, tGroup );

			sprintf_s( tBuf, "Sensor%d_YPosition", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.YPos, tGroup );

			sprintf_s( tBuf, "Sensor%d_Forbid", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Forbid, tGroup );
		}
		pIterator++;
	}	
	
	return HSTrue;
}

HSBool CPlanePosition::Load( CINIConfig *pIniConfig, string tGroup )
{		
	pIniConfig->ValueWithKey( "MaterialLength", this->mMaterialLength, tGroup );
	pIniConfig->ValueWithKey( "MaterialWidth", this->mMaterialWidth, tGroup );

	pIniConfig->ValueWithKey( "XAxisLength", this->mXAxisLength, tGroup );
	pIniConfig->ValueWithKey( "YAxisLength", this->mYAxisLength, tGroup );
	pIniConfig->ValueWithKey( "HitRadius", this->mHitRadius, tGroup );	
	
	HSString tDataIdentifier = "";
	HSDouble tXPosition = 0;
	HSDouble tYPosition = 0;
	HSBool tForbid = HSFalse;

	HSChar tBufId[ 1024 ];
	HSChar tBufXPos[ 1024 ];
	HSChar tBufYPos[ 1024 ];
	HSChar tBufForbid[ 1024 ];

	for ( HSInt i = 0; i < CPlanePositionProperty::POSITION_SENSOR_MAX_NUM; i++ )
	{
		sprintf_s( tBufId, "Sensor%d_DataIdentifier", i );
		sprintf_s( tBufXPos, "Sensor%d_XPosition", i );
		sprintf_s( tBufYPos, "Sensor%d_YPosition", i );
		sprintf_s( tBufForbid, "Sensor%d_Forbid", i );
		if ( pIniConfig->ValueWithKey( tBufId, tDataIdentifier, tGroup )
			&& pIniConfig->ValueWithKey( tBufXPos, tXPosition, tGroup )
			&& pIniConfig->ValueWithKey( tBufYPos, tYPosition, tGroup )
			&& pIniConfig->ValueWithKey( tBufForbid, tForbid, tGroup ) )
		{			
			mPositionSensors[ i ].XPos = tXPosition;
			mPositionSensors[ i ].YPos = tYPosition;
			mPositionSensors[ i ].Forbid = tForbid;
			mPositionSensors[ i ].IsOn = HSTrue;	
			mPositionSensors[ i ].DataIdentifier.InitWithString( tDataIdentifier );
			mPositionSensors[ i ].ArgTransfer = NULL;			
		}
	}		
	
	return HSTrue;
}	
	
IGraphicPropertyProtocol * CPlanePosition::CreatePropertyDlg( CWnd *pParent )
{
	CPlanePositionProperty *pPositionProperty = new CPlanePositionProperty;
	pPositionProperty->Create( IDD_NORMALPROPERTY, pParent );
	return pPositionProperty;
}

HSVoid CPlanePosition::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		this->UpdatePositionSensor( pIterator->first, pIterator->second.XPos, pIterator->second.YPos, pIterator->second.Forbid, pIterator->second.IsOn, pIterator->second.DataIdentifier );
		pIterator++;
	}

	this->mHitsPosition.clear();

	RefreshSensors();

	SetEvent( mDataMutex );	
}

HSVoid CPlanePosition::SetCheckRadius( HSDouble tValue )
{
	mCheckRadius = tValue;
	mStaticRelation.SetCheckRadius( tValue, this->mCheckCircleEnable );
	mStaticRelation.Invalidate();
}

HSVoid CPlanePosition::SetCheckPosX( HSDouble tValue )
{
	mStaticRelation.SetCheckPos( tValue, mStaticRelation.YCheckPos() );
	mStaticRelation.Invalidate();
}

HSVoid CPlanePosition::SetCheckPosY( HSDouble tValue )
{
	mStaticRelation.SetCheckPos( mStaticRelation.XCheckPos(), tValue );
	mStaticRelation.Invalidate();
}

HSDouble CPlanePosition::DistanceFromSensor( HSDouble tPosX, HSDouble tPosY, HSInt tSensor )
{	
	HSDouble tX = tPosX - mPositionSensors[ tSensor ].XPos;
	HSDouble tY = tPosY - mPositionSensors[ tSensor ].YPos;

	return sqrt( tX * tX + tY * tY );
}

HSDouble CPlanePosition::DistanceBetweenSensors( HSInt tSensorOne, HSInt tSensorTwo )
{		
	return DistanceFromSensor( mPositionSensors[ tSensorOne ].XPos, mPositionSensors[ tSensorOne ].YPos, tSensorTwo );
}

HSBool CPlanePosition::IsHitQualified( HSInt64 tDuration, HSDouble tVelocity, HSInt tSensorOne, HSInt tSensorTwo )
{
	HSDouble tDistanceVoice = abs( tDuration / 1000000000.0 * tVelocity );	

	HSDouble tPhyDistance = DistanceBetweenSensors( tSensorOne, tSensorTwo );

	return tDistanceVoice < tPhyDistance;
}


HSBool CPlanePosition::IsResQualified( HSDouble tXResPos, HSDouble tYResPos, vector< CPlanePositionCalulator::HIT_CALC_INFO > &tHitSensors, HSDouble &tEnergy )
{
/*	HS_INFO( "ResHitPos:( %lf, %lf )", tXResPos, tYResPos );
	HS_INFO( "HitSensors: ( %d, %lld ), ( %d, %lld ), ( %d, %lld )", tHitSensors[ 0 ].SensorIndex, mPositionSensors[ tHitSensors[ 0 ].SensorIndex ].ArgList[ tHitSensors[ 0 ].ArgIndex ].NBeginTime,
																	tHitSensors[ 1 ].SensorIndex, mPositionSensors[ tHitSensors[ 1 ].SensorIndex ].ArgList[ tHitSensors[ 1 ].ArgIndex ].NBeginTime,
																	tHitSensors[ 2 ].SensorIndex, mPositionSensors[ tHitSensors[ 2 ].SensorIndex ].ArgList[ tHitSensors[ 2 ].ArgIndex ].NBeginTime );
																	*/
	if ( tXResPos < 0 || tXResPos > this->mMaterialWidth || tYResPos < 0 || tYResPos > this->mMaterialLength )
	{
		return HSFalse;
	}

	std::sort( tHitSensors.begin(), tHitSensors.end(), CPlanePositionCalulator::HitSensorComapre );
	/*
	HS_INFO( "HitSensors: ( %d, %lld ), ( %d, %lld ), ( %d, %lld )", tHitSensors[ 0 ].SensorIndex, mPositionSensors[ tHitSensors[ 0 ].SensorIndex ].ArgList[ tHitSensors[ 0 ].ArgIndex ].NBeginTime,
																	tHitSensors[ 1 ].SensorIndex, mPositionSensors[ tHitSensors[ 1 ].SensorIndex ].ArgList[ tHitSensors[ 1 ].ArgIndex ].NBeginTime,
																	tHitSensors[ 2 ].SensorIndex, mPositionSensors[ tHitSensors[ 2 ].SensorIndex ].ArgList[ tHitSensors[ 2 ].ArgIndex ].NBeginTime );
																	*/	
	vector< CPlanePositionCalulator::SENSOR_DIST_INFO > tPosDistance;
	for ( HSUInt i = 0; i < tHitSensors.size(); i++ )
	{
		CPlanePositionCalulator::SENSOR_DIST_INFO tDistanceInfo = { tHitSensors[ i ].SensorIndex, DistanceFromSensor( tXResPos, tYResPos, tHitSensors[ i ].SensorIndex ) };
		tPosDistance.push_back( tDistanceInfo );

		tEnergy = max( tEnergy, mPositionSensors[ tHitSensors[ i ].SensorIndex ].ArgList[ tHitSensors[ i ].ArgIndex ].Energy );
	}

	std::sort( tPosDistance.begin(), tPosDistance.end(), CPlanePositionCalulator::HitSensorDistanceComapre );
/*
	HS_INFO( "Distance: ( %d, %f ), ( %d, %f ), ( %d, %f )", tPosDistance[ 0 ].SensorIndex, tPosDistance[ 0 ].Distance, 
															tPosDistance[ 1 ].SensorIndex, tPosDistance[ 1 ].Distance, 
															tPosDistance[ 2 ].SensorIndex, tPosDistance[ 2 ].Distance );
															*/
	for ( HSUInt i = 0; i < tHitSensors.size(); i++ )
	{
		if ( tHitSensors[ i ].SensorIndex != tPosDistance[ i ].SensorIndex )
		{
			return HSFalse;
		}
	}

	return HSTrue;
}

HSBool CPlanePosition::CalcHitPosition( HSInt tSensor, HSUInt tArgIndex )
{
	vector< CPlanePositionCalulator::HIT_CALC_INFO > tHitSensors;
	
	CPlanePositionCalulator::HIT_CALC_INFO tHit = { tSensor, tArgIndex, this };
	tHitSensors.push_back( tHit );	

	HSInt tXLowIndex = 0;
	HSDouble tXLowPos = mPositionSensors[ tSensor ].XPos;
	HSInt tXBigIndex = 0;
	HSDouble tXBigPos = tXLowPos;
	
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();		
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.ArgTransfer != NULL && pIterator->second.IsOn && !pIterator->second.Forbid )
		{			
			for ( HSUInt i = 0; i < pIterator->second.ArgList.size(); i++ )
			{
				if ( pIterator->first != tSensor 
					&& IsHitQualified( mPositionSensors[ tSensor ].ArgList[ tArgIndex ].NBeginTime - pIterator->second.ArgList[ i ].NBeginTime, CArgCalculator::SharedInstance()->GetMaterialVelocity(), tSensor, pIterator->first ) )
				{				
					CPlanePositionCalulator::HIT_CALC_INFO tHit = { pIterator->first, i, this };
					tHitSensors.push_back( tHit );

					if ( pIterator->second.XPos > tXBigPos )
					{
						tXBigPos = pIterator->second.XPos;
						tXBigIndex = tHitSensors.size() - 1;
					}

					if ( pIterator->second.XPos < tXLowPos )
					{
						tXLowPos = pIterator->second.XPos;
						tXLowIndex = tHitSensors.size() - 1;
					}

					break;
				}
			}
		}
		pIterator++;
	}

	if ( tHitSensors.size() < 3 )
	{
		//HS_INFO( "Not Enough Sensor: %d", tHitSensors.size() );
		mPositionSensors[ tSensor ].ArgHandled[ tArgIndex ] = HSTrue;
		return HSFalse;
	}

	vector< CPlanePositionCalulator::HIT_POS > tResHitPos;
	if ( !mPositionCalulator.CalcHit( tHitSensors, tResHitPos, this ) )
	{
		return HSFalse;
	}

	HSBool tRes = HSFalse;
	for ( HSUInt i = 0; i < tResHitPos.size(); i++ )
	{		
		HSDouble tEnergy = 0;
		if ( IsResQualified( tResHitPos[ i ].PosX, tResHitPos[ i ].PosY, tHitSensors, tEnergy ) )
		{			
			CStaticPlanePosition::HitPosition tHitPos = { tResHitPos[ i ].PosX, tResHitPos[ i ].PosY, CGraphicManager::SharedInstance()->ColorWithEnergy( tEnergy ) };
			mHitsPosition.push_back( tHitPos );
			tRes = HSTrue;
		}
	}

	if ( !tRes )
	{
		return HSFalse;
	}

	for ( HSUInt i = 0; i < tHitSensors.size(); i++ )
	{
		mPositionSensors[ tHitSensors[ i ].SensorIndex ].ArgHandled[ tHitSensors[ i ].ArgIndex ] = HSTrue;
	}
	
	return HSTrue;
}

HSVoid CPlanePosition::SetMaterialLength( HSDouble tLength )
{
	mMaterialLength = max( tLength, 10 );
	mStaticRelation.SetMaterialLength( mMaterialLength );
	
	this->AutoSetSensors();
}

HSVoid CPlanePosition::SetMaterialWidth( HSDouble tWidth )
{
	mMaterialWidth = max( tWidth, 10 );
	mStaticRelation.SetMaterialWidth( mMaterialWidth );
	
	this->AutoSetSensors();
}

HSVoid CPlanePosition::InitPositionSensors()
{
	HSDouble tPosPercent = 0.1;	
	HSInt tIndex = 0;
	while ( mPositionSensors.size() < 3 )
	{
		if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
		{
			this->UpdatePositionSensor( tIndex, tPosPercent * mMaterialWidth, tPosPercent * mMaterialLength, HSFalse, HSTrue, 0 );
			tPosPercent += 0.2;
		}
		tIndex++;
	}

	this->RefreshSensors();
}

HSVoid CPlanePosition::SetFocusSensor( HSInt tIndex )
{
	mStaticRelation.SetFocusSensor( tIndex );
	mStaticRelation.Invalidate();
}

HSVoid CPlanePosition::SetXAxisLength( HSDouble tLength )
{
	mXAxisLength = tLength;
	mStaticRelation.SetXValue( mXAxisLength, 0 );
	mStaticRelation.ResizeWave();
	mStaticRelation.Invalidate();
}

HSVoid CPlanePosition::SetYAxisLength( HSDouble tLength )
{
	mYAxisLength = tLength;
	mStaticRelation.SetYValue( mYAxisLength, mYAxisLength );
	mStaticRelation.ResizeWave();
	mStaticRelation.Invalidate();
}

HSVoid CPlanePosition::SetHitRadius( HSInt tHitRadius )
{
	mHitRadius = tHitRadius;
	mStaticRelation.SetHitRadius( mHitRadius );
	mStaticRelation.Invalidate();
}

HSVoid CPlanePosition::UpdatePositionSensor( HSInt tIndex, HSDouble tPositionX, HSDouble tPositionY, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier )
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

	mPositionSensors[ tIndex ].XPos = tPositionX;
	mPositionSensors[ tIndex ].YPos = tPositionY;
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
}

HSVoid CPlanePosition::RefreshSensors()
{
	mSensors.clear();
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			CStaticPlanePosition::SensorInfo tSensor;
			tSensor.Index = pIterator->first;
			tSensor.XPos = pIterator->second.XPos;
			tSensor.YPos = pIterator->second.YPos;
			mSensors.push_back( tSensor );
		}
		pIterator++;
	}
	
	mStaticRelation.SetSensors( &mSensors );
	mStaticRelation.Invalidate();
}

CPlanePosition::PositionSensor * CPlanePosition::GetPositionSensor( HSInt tIndex )
{
	if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
	{
		return NULL;
	}

	return &( mPositionSensors[ tIndex ] );
}

HSVoid CPlanePosition::InitDialog()
{
	mPopupMenu.LoadMenu( IDR_POPUP_PLANE_POSITION_C );

	CRect tRect;
	this->GetClientRect( &tRect );
	mStaticRelation.Create( "", WS_CHILD | WS_VISIBLE, tRect, this );

	mStaticGraphic = &mStaticRelation;
	
	mStaticRelation.SetHitsPosition( &mHitsPosition );
	mStaticRelation.SetSensors( &mSensors );
	mStaticRelation.SetMaterialLength( mMaterialLength );
	mStaticRelation.SetMaterialWidth( mMaterialWidth );

	mStaticRelation.SetXValue( mXAxisLength, 0 );	
	mStaticRelation.SetYValue( mYAxisLength, mYAxisLength );
	mStaticRelation.SetHitRadius( mHitRadius );

	mStaticRelation.SetCheckRadius( mCheckRadius, HSFalse );
	mStaticRelation.SetCheckPos( mMaterialWidth / 2, mMaterialLength / 2 );

	InitPositionSensors();
}

HSBool CPlanePosition::GraphicNeedRefresh()
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
				
				CalcHitPosition( pIterator->first, i );
			}			
		}
		pIterator++;	
	}

	mStaticRelation.Invalidate();

	return HSTrue;
}

HSVoid CPlanePosition::StepGraphic( HSInt tDelta )
{
	WaitForSingleObject( mDataMutex, INFINITE );		

	mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() + tDelta * mStaticRelation.TotalXValue() );
	mStaticRelation.Invalidate();

	SetEvent( mDataMutex );				
}

HSVoid CPlanePosition::ZoomInGraphic( CRect &tViewRect )
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

HSVoid CPlanePosition::DragGraphic( CRect &tViewRect, CPoint tPoint )
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


HSBool CPlanePosition::IsDataExportNeeded()
{
	return HSTrue;
}

HSVoid CPlanePosition::ExportData( CDataExportHelper &tDataExport )
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	HSChar tBuf[ 1024 ];	

	sprintf_s( tBuf, "宽度：%.2f (m)\n", mMaterialWidth / 1000.0 );
	tDataExport.Write( tBuf ); 

	sprintf_s( tBuf, "高度：%.2f (m)\n", mMaterialLength / 1000.0 );
	tDataExport.Write( tBuf ); 

	sprintf_s( tBuf, "\n% 10s\t% 18s\t% 18s\t% 16s\t% 16s\n", "传感器编号", "X(m)", "Y(m)", "采集卡", "通道" );
	tDataExport.Write( tBuf ); 

	map< HSInt, PositionSensor >::iterator pSensorIterator = mPositionSensors.begin();
	while ( pSensorIterator != mPositionSensors.end() )
	{
		if ( pSensorIterator->second.IsOn && !pSensorIterator->second.Forbid )
		{
			IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( pSensorIterator->second.DataIdentifier );
			sprintf_s( tBuf, "% 10d\t% 18f\t% 18f\t% 16s\t% 16d\n", 
				pSensorIterator->first + 1, 
				pSensorIterator->second.XPos / 1000.0, 
				pSensorIterator->second.YPos / 1000.0, 
				( pDataHandler == NULL ? "" : pDataHandler->Name().c_str() ),
				( pDataHandler == NULL ? -1 : pSensorIterator->second.DataIdentifier.ChannelIndex() + 1 ) );

			tDataExport.Write( tBuf ); 
		}

		pSensorIterator++;
	}		

	sprintf_s( tBuf, "\n% 10s\t% 18s\t% 18s\n", "撞击编号", "X(m)", "Y(m)" );
	tDataExport.Write( tBuf ); 

	HSInt tIndex = 1;
	list< CStaticPlanePosition::HitPosition >::iterator pHitIterator = mHitsPosition.begin();
	while ( pHitIterator != mHitsPosition.end() )
	{
		sprintf_s( tBuf, "% 10d\t% 18f\t% 18f\n", tIndex++, pHitIterator->XPos / 1000.0, pHitIterator->YPos / 1000.0 );
		tDataExport.Write( tBuf ); 

		pHitIterator++;
	}
}

HSVoid CPlanePosition::ResetData( CMainFrame* pMainFrame )
{
	WaitForSingleObject( mDataMutex, INFINITE );	
	
	mStaticRelation.SetXValue( mXAxisLength, 0 );	

	mStaticRelation.SetYValue( mYAxisLength, mYAxisLength );

	mStaticRelation.Invalidate();

	SetEvent( mDataMutex );
}

HSVoid CPlanePosition::MouseDownWhenPointer(  CRect &tRect, CPoint tPoint )
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

HSBool CPlanePosition::CustomMouseUpCheck()
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
			if ( pPositionSensor->XPos < 0 || pPositionSensor->XPos > this->mMaterialWidth || pPositionSensor->YPos < 0 || pPositionSensor->YPos > this->mMaterialLength )
			{
				pPositionSensor->Forbid = HSTrue;
			}
			else
			{
				pPositionSensor->Forbid = HSFalse;
			}

			this->UpdatePositionSensor( mDragingSensorIndex, pPositionSensor->XPos, pPositionSensor->YPos, pPositionSensor->Forbid, pPositionSensor->IsOn, pPositionSensor->DataIdentifier );
			RefreshSensors();
		}

		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
		pMainFrame->SetFocusedGraphic( this, HSTrue );

		return HSTrue;
	}

	return HSFalse;
}

HSVoid CPlanePosition::MouseUpFinish()
{
	mDragingSensorIndex = -1;
}	

HSBool CPlanePosition::CustomMouseMoveCheck( CPoint tPoint )
{
	if ( mDragCheckCircle )
	{
		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		tViewRect = CRect( tViewRect.left + mStaticRelation.LeftDelta(), tViewRect.top + mStaticRelation.TopDelta(), tViewRect.right - mStaticRelation.LeftDelta(), tViewRect.bottom - mStaticRelation.TopDelta() );

		HSInt tXOffset = mPrevPoint.x - tPoint.x;
		HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticRelation.TotalXValue();

		HSInt tYOffset = mPrevPoint.y - tPoint.y;
		HSDouble tYOffsetValue = ( HSDouble )tYOffset / tViewRect.Height() * mStaticRelation.TotalYValue();
		mStaticRelation.SetCheckPos( mStaticRelation.XCheckPos() - tXOffsetValue * 1000.0, mStaticRelation.YCheckPos() + tYOffsetValue * 1000.0 );
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

			HSInt tYOffset = mPrevPoint.y - tPoint.y;
			HSDouble tYOffsetValue = ( HSDouble )tYOffset / tViewRect.Height() * mStaticRelation.TotalYValue();

			this->UpdatePositionSensor( mDragingSensorIndex, pPositionSensor->XPos - tXOffsetValue * 1000.0, pPositionSensor->YPos + tYOffsetValue * 1000.0, pPositionSensor->Forbid, pPositionSensor->IsOn, pPositionSensor->DataIdentifier );			
			RefreshSensors();
		}

		mPrevPoint = tPoint;

		return HSTrue;
	}

	return HSFalse;
}

HSVoid CPlanePosition::AutoSetSensors()
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

	if ( tSensorCount < 3 )
	{
		return;
	}	
	
	//HSDouble tYDistance = HSInt( ( this->mMaterialLength - this->mMaterialLength / 10.0 * 2.0 ) / ( tSensorCount - 1 ) );
	HSDouble tXDistance = HSInt( ( this->mMaterialWidth - this->mMaterialLength / 10.0 * 2.0 ) / ( tSensorCount - 1 ) );
	HSDouble tPositionY = mMaterialLength - HSInt( this->mMaterialLength / 10.0 );
	HSDouble tPositionX = HSInt( this->mMaterialWidth / 10.0 );
	pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			pIterator->second.XPos = tPositionX;
			pIterator->second.YPos = tPositionY;
			tPositionX += tXDistance;
			tPositionY = mMaterialLength - tPositionY;
		}
		pIterator++;
	}

	this->RefreshSensors();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
	pMainFrame->SetFocusedGraphic( this, HSTrue );	
}

HSVoid CPlanePosition::SwitchCheckCircle()
{
	mCheckCircleEnable = !mCheckCircleEnable;
	mStaticRelation.SetCheckRadius( this->mCheckRadius, mCheckCircleEnable );
	mStaticRelation.SetCheckPos( mMaterialWidth / 2, mMaterialLength / 2 );
	mStaticRelation.Invalidate();
}
