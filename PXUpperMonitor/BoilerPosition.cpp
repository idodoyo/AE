// PlanePosition.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "BoilerPosition.h"
#include "afxdialogex.h"
#include "BoilerPositionProperty.h"
#include "MainFrm.h"
#include <algorithm>
#include "ArgCalculator.h"
#include "HSLogProtocol.h"
#include "DataExportHelper.h"

// CBoilerPosition dialog

CBoilerPosition::CBoilerPosition(CWnd* pParent /*=NULL*/)
	: CNormalGraphic( pParent )
{
	mMaterialRadius = 8.3 / 2.0;

	mXAxisLength = 10;
	mYAxisLength = 10;

	mPillarsLength.push_back( 4.150 );
	for ( HSInt i = 0; i < 3; i++ )
	{
		mPillarsLength.push_back( 3.125 );
	}

	mIronStoneCycleNum = 10.0;
	mIronStoneCycleTime = 7.5;

	mCokeCycleNum = 11.5;
	mCokeCycleTime = 7.5;

	mBoilerName = "锅炉X";	

	mStopWhenMouseDown = HSFalse;
}

CBoilerPosition::~CBoilerPosition()
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

HSBool CBoilerPosition::Start()
{
	if ( mPositionSensors.size() > 0 )
	{				
		mRefreshThread->Start();
		return HSTrue;
	}
	return HSFalse;
}

HSBool CBoilerPosition::Save( CINIConfig *pIniConfig, string tGroup )
{		
	pIniConfig->SetValue( "MaterialRadius", this->mMaterialRadius, tGroup );
	pIniConfig->SetValue( "XAxisLength", this->mXAxisLength, tGroup );
	pIniConfig->SetValue( "YAxisLength", this->mYAxisLength, tGroup );

	pIniConfig->SetValue( "BoilerName", this->mBoilerName, tGroup );
	pIniConfig->SetValue( "IronStoneCycleNum", this->mIronStoneCycleNum, tGroup );
	pIniConfig->SetValue( "IronStoneTime", this->mIronStoneCycleTime, tGroup );
	pIniConfig->SetValue( "CokeCycleNum", this->mCokeCycleNum, tGroup );
	pIniConfig->SetValue( "Coketime", this->mCokeCycleTime, tGroup );

	HSChar tBuf[ 1024 ];
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		sprintf_s( tBuf, "Sensor%d_DataIdentifier", pIterator->first );
		pIniConfig->SetValue( tBuf, pIterator->second.DataIdentifier.ToString(), tGroup );

		sprintf_s( tBuf, "Sensor%d_XPosition", pIterator->first );
		pIniConfig->SetValue( tBuf, pIterator->second.XPos, tGroup );

		sprintf_s( tBuf, "Sensor%d_YPosition", pIterator->first );
		pIniConfig->SetValue( tBuf, pIterator->second.YPos, tGroup );

		pIterator++;
	}

	for ( HSUInt i = 0; i < mPillarsLength.size(); i++ )
	{
		sprintf_s( tBuf, "Pillar%d_Length", i );
		pIniConfig->SetValue( tBuf, mPillarsLength[ i ], tGroup );
	}
	
	return HSTrue;
}

HSBool CBoilerPosition::Load( CINIConfig *pIniConfig, string tGroup )
{			
	pIniConfig->ValueWithKey( "MaterialRadius", this->mMaterialRadius, tGroup );
	pIniConfig->ValueWithKey( "XAxisLength", this->mXAxisLength, tGroup );
	pIniConfig->ValueWithKey( "YAxisLength", this->mYAxisLength, tGroup );

	pIniConfig->ValueWithKey( "BoilerName", this->mBoilerName, tGroup );
	pIniConfig->ValueWithKey( "IronStoneCycleNum", this->mIronStoneCycleNum, tGroup );
	pIniConfig->ValueWithKey( "IronStoneTime", this->mIronStoneCycleTime, tGroup );
	pIniConfig->ValueWithKey( "CokeCycleNum", this->mCokeCycleNum, tGroup );
	pIniConfig->ValueWithKey( "Coketime", this->mCokeCycleTime, tGroup );

	HSString tDataIdentifier = "";
	HSDouble tXPosition = 0;
	HSDouble tYPosition = 0;

	HSChar tBufId[ 1024 ];
	HSChar tBufXPos[ 1024 ];
	HSChar tBufYPos[ 1024 ];

	for ( HSInt i = 0; i < CBoilerPositionProperty::POSITION_SENSOR_MAX_NUM; i++ )
	{
		sprintf_s( tBufId, "Sensor%d_DataIdentifier", i );
		sprintf_s( tBufXPos, "Sensor%d_XPosition", i );
		sprintf_s( tBufYPos, "Sensor%d_YPosition", i );
		if ( pIniConfig->ValueWithKey( tBufId, tDataIdentifier, tGroup )
			&& pIniConfig->ValueWithKey( tBufXPos, tXPosition, tGroup )
			&& pIniConfig->ValueWithKey( tBufYPos, tYPosition, tGroup )	)
		{			
			mPositionSensors[ i ].XPos = tXPosition;
			mPositionSensors[ i ].YPos = tYPosition;			
			mPositionSensors[ i ].DataIdentifier.InitWithString( tDataIdentifier );
			mPositionSensors[ i ].ArgTransfer = NULL;			
		}
	}

	HSChar tBufPillar[ 1024 ];	
	for ( HSUInt i = 0; i < mPillarsLength.size(); i++ )
	{
		sprintf_s( tBufPillar, "Pillar%d_Length", i );
		pIniConfig->ValueWithKey( tBufPillar, mPillarsLength[ i ], tGroup );
	}
	
	return HSTrue;
}	
	
IGraphicPropertyProtocol * CBoilerPosition::CreatePropertyDlg( CWnd *pParent )
{
	CBoilerPositionProperty *pPositionProperty = new CBoilerPositionProperty;
	pPositionProperty->Create( IDD_NORMALPROPERTY, pParent );
	return pPositionProperty;
}

HSVoid CBoilerPosition::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		this->UpdatePositionSensor( pIterator->first, pIterator->second.DataIdentifier );
		pIterator++;
	}

	mBoilerFeedMaterialInfo.clear();

	RefreshSensors();

	mCurFeedMaterialStartSensor = -1;
	mStartFeedMaterial = HSFalse;	

	SetEvent( mDataMutex );	
}

HSVoid CBoilerPosition::InitPositionSensors()
{	
	if ( mPositionSensors.size() < 4 )
	{		
		this->UpdatePositionSensor( 0, 0 );
		this->UpdatePositionSensor( 1, 0 );
		this->UpdatePositionSensor( 2, 0 );
		this->UpdatePositionSensor( 3, 0 );
	}

	this->RefreshSensors();
}

HSVoid CBoilerPosition::SetFocusSensor( HSInt tIndex )
{
	mStaticRelation.SetFocusSensor( tIndex );
	mStaticRelation.Invalidate();
}

HSVoid CBoilerPosition::UpdatePositionSensor( HSInt tIndex, DEVICE_CH_IDENTIFIER tDataIdentifier )
{
	if ( mPositionSensors.find( tIndex ) != mPositionSensors.end() )
	{
		if ( mPositionSensors[ tIndex ].ArgTransfer != NULL )
		{
			delete mPositionSensors[ tIndex ].ArgTransfer;
			mPositionSensors[ tIndex ].ArgList.clear();			 
		}
	}

	HSDouble tDelta = 0.5;
	if ( tIndex == 0 )
	{
		mPositionSensors[ tIndex ].XPos = 0;
		mPositionSensors[ tIndex ].YPos = mMaterialRadius + tDelta;
	}
	else if ( tIndex == 1 )
	{
		mPositionSensors[ tIndex ].XPos = mMaterialRadius + tDelta;
		mPositionSensors[ tIndex ].YPos = 0;
	}
	else if ( tIndex == 2 )
	{
		mPositionSensors[ tIndex ].XPos = 0;
		mPositionSensors[ tIndex ].YPos = -mMaterialRadius - tDelta;
	}
	else
	{
		mPositionSensors[ tIndex ].XPos = -mMaterialRadius - tDelta;
		mPositionSensors[ tIndex ].YPos = 0;
	}
	
	mPositionSensors[ tIndex ].DataIdentifier = tDataIdentifier;
	mPositionSensors[ tIndex ].ArgTransfer = NULL;	
	mPositionSensors[ tIndex ].Index = 0;


	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	DEVICE_CH_IDENTIFIER tCardIdentifer( tDataIdentifier.CARD_IDENTIFIER, tDataIdentifier.CARD_INDEX );
	IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( tCardIdentifer );
	if ( pDataHandler != NULL )
	{
		mPositionSensors[ tIndex ].ArgTransfer = new CArgListTransfer( pDataHandler, tDataIdentifier );
	}
}

HSVoid CBoilerPosition::RefreshSensors()
{
	mSensors.clear();
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{		
		CStaticBoilerPosition::SensorInfo tSensor;
		tSensor.Index = pIterator->first;
		tSensor.XPos = pIterator->second.XPos;
		tSensor.YPos = pIterator->second.YPos;
		mSensors.push_back( tSensor );

		pIterator++;
	}
	
	mStaticRelation.SetSensors( &mSensors );
	mStaticRelation.Invalidate();
}

CBoilerPosition::PositionSensor * CBoilerPosition::GetPositionSensor( HSInt tIndex )
{
	if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
	{
		return NULL;
	}

	return &( mPositionSensors[ tIndex ] );
}

HSVoid CBoilerPosition::SetMaterailRadius( HSDouble tRadius )
{ 
	mMaterialRadius = tRadius;
	
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		this->UpdatePositionSensor( pIterator->first, pIterator->second.DataIdentifier );
		pIterator++;
	}

	mStaticRelation.SetMaterialRadius( mMaterialRadius );

	mXAxisLength = mMaterialRadius * 2 + mMaterialRadius / 2;
	mStaticRelation.SetXValue( mXAxisLength, -mXAxisLength / 2  );

	mYAxisLength = mXAxisLength;	
	mStaticRelation.SetYValue( mYAxisLength, mYAxisLength / 2 );

	RefreshSensors();
}

HSVoid CBoilerPosition::SetXAxisLength( HSDouble tValue )
{ 
	mXAxisLength = tValue;
	mStaticRelation.SetXValue( mXAxisLength, -mXAxisLength / 2  );
	mStaticRelation.ResizeWave();
	mStaticRelation.Invalidate();
}

HSVoid CBoilerPosition::SetYAxisLength( HSDouble tValue )
{ 
	mYAxisLength = tValue;	
	mStaticRelation.SetYValue( mYAxisLength, mYAxisLength / 2 );
	mStaticRelation.ResizeWave();
	mStaticRelation.Invalidate();
}

HSVoid CBoilerPosition::SetPillarLength( HSInt tIndex, HSDouble tValue )
{
	mPillarsLength[ tIndex ] = tValue;
	mStaticRelation.SetPillarLength( mPillarsLength );
	mStaticRelation.ResizeWave();
	mStaticRelation.Invalidate();
}

HSVoid CBoilerPosition::SetIronStoneInfo( HSDouble tCycleNum, HSDouble tCycleTime )
{
	mIronStoneCycleNum = max( mIronStoneCycleNum, tCycleNum );
	mIronStoneCycleTime = max( mIronStoneCycleTime, tCycleTime );
}

HSVoid CBoilerPosition::SetCokeInfo( HSDouble tCycleNum, HSDouble tCycleTime )
{
	mCokeCycleNum = max( mCokeCycleNum, tCycleNum );
	mCokeCycleTime = max( mCokeCycleTime, tCycleTime );	
}

HSVoid CBoilerPosition::SetBoilerName( HSString tName )
{
	mBoilerName = tName;
	mStaticRelation.SetTitle( mBoilerName.c_str() );
	mStaticRelation.ResizeWave();
	mStaticRelation.Invalidate();
}

HSVoid CBoilerPosition::FindChannelFeedMaterialInfo( HSInt tSensor, vector< ChannelFeedMaterial > &tChannelsFeedMaterial )
{
	HSInt tEffectiveEnergy = 1400;
	HSInt tIronEnergy = 4500;	
	ChannelFeedMaterial tFeedMaterial;
	HSUInt64 tMinDuration = 2 * 1000 * 1000;
	tMinDuration *= 1000;

	HSBool tFindFeedMaterial = HSFalse;	
	HSDouble tMaxAmplitude = 0;

	tFeedMaterial.Energy = 0;

	for ( HSUInt i = mPositionSensors[ tSensor ].Index; i < mPositionSensors[ tSensor ].ArgList.size(); i++ )
	{
		if ( mPositionSensors[ tSensor ].ArgList[ i ].Energy > tEffectiveEnergy )
		{	
			if ( mPositionSensors[ tSensor ].ArgList[ i ].Energy > tFeedMaterial.Energy )
			{
				tFeedMaterial.Material = ( mPositionSensors[ tSensor ].ArgList[ i ].Energy > tIronEnergy ? MATERIAL_IRON_STONE : MATERIAL_COKE );
				tFeedMaterial.Sensor = tSensor;
				tFeedMaterial.Time = mPositionSensors[ tSensor ].ArgList[ i ].NBeginTime;
				tFeedMaterial.Energy = mPositionSensors[ tSensor ].ArgList[ i ].Energy;

				tMaxAmplitude = mPositionSensors[ tSensor ].ArgList[ i ].Amplitude;
			}
			
			if ( !tFindFeedMaterial )
			{
				tFeedMaterial.Time = mPositionSensors[ tSensor ].ArgList[ i ].NBeginTime;
			}

			tFindFeedMaterial = HSTrue;
		}
		else if ( tFindFeedMaterial && ( mPositionSensors[ tSensor ].ArgList[ i ].NBeginTime - tFeedMaterial.Time > tMinDuration ) )
		{
			HS_INFO( "Find Feed Material, Time: %d, Energy: %lf, Amplitude: %lf", HSInt( tFeedMaterial.Time / 1000 / 1000 / 1000 ), tFeedMaterial.Energy, tMaxAmplitude );

			tChannelsFeedMaterial.push_back( tFeedMaterial );
			tFindFeedMaterial = HSFalse;

			tFeedMaterial.Energy = 0;

			mPositionSensors[ tSensor ].Index = i;
		}
	}

	if ( !tFindFeedMaterial )
	{
		mPositionSensors[ tSensor ].ArgList.clear();
		mPositionSensors[ tSensor ].Index = 0;
	}
}

HSVoid CBoilerPosition::FindBoilerFeedMaterialInfo( vector< ChannelFeedMaterial > &tChannelsFeedMaterial )
{		
	if ( tChannelsFeedMaterial.size() < 1 )
	{		
		return;
	}
	
	std::sort( tChannelsFeedMaterial.begin(), tChannelsFeedMaterial.end(), FeedMaterialComapre );

	HSUInt tIndex = 0;

	if ( !mStartFeedMaterial )
	{
		BoilerFeedMaterial tFeedMaterial;
		tFeedMaterial.BeginTime = tChannelsFeedMaterial[ 0 ].Time;
		tFeedMaterial.EndTime = tChannelsFeedMaterial[ 0 ].Time;
		tFeedMaterial.BeginSensor = tChannelsFeedMaterial[ 0 ].Sensor;
		tFeedMaterial.Material = tChannelsFeedMaterial[ 0 ].Material;		
		tFeedMaterial.SensorsEnergy[ tChannelsFeedMaterial[ 0 ].Sensor ].push_back( tChannelsFeedMaterial[ 0 ].Energy );

		mBoilerFeedMaterialInfo.push_back( tFeedMaterial );

		mStartFeedMaterial = HSTrue;
		tIndex = 1;

		mCurFeedMaterialStartSensor = tFeedMaterial.BeginSensor;
	}

	list< BoilerFeedMaterial >::iterator pIterator = mBoilerFeedMaterialInfo.end();
	pIterator--;
	BoilerFeedMaterial *pBoilerFeedMaterial = &( *pIterator );

	HSUInt64 tIronStoneTotalTime = HSUInt( mIronStoneCycleNum * ( mIronStoneCycleTime + 0.5 ) * 1000000 );
	tIronStoneTotalTime *= 1000;

	HSUInt64 tCokeTotalTime = HSUInt( mCokeCycleNum * ( mCokeCycleTime + 0.5 ) * 1000000 );
	tCokeTotalTime *= 1000;

	HSUInt64 tTotalTime = ( pBoilerFeedMaterial->Material == MATERIAL_IRON_STONE ? tIronStoneTotalTime : tCokeTotalTime );
	while ( tIndex < tChannelsFeedMaterial.size() )
	{
		if ( /*pBoilerFeedMaterial->Material != tChannelsFeedMaterial[ tIndex ].Material || */ ( tChannelsFeedMaterial[ tIndex ].Time - pBoilerFeedMaterial->BeginTime > tTotalTime ) )
		{
			mStartFeedMaterial = HSFalse;
			break;
		}

		HSInt tSensor = tChannelsFeedMaterial[ tIndex ].Sensor;
		HSInt64 tFullCycleTime = ( HSInt64 )( ( pBoilerFeedMaterial->Material == MATERIAL_IRON_STONE ? mIronStoneCycleTime : mCokeCycleTime ) * 1000 * 1000 * 1000 );
		
		pBoilerFeedMaterial->EndTime = max( tChannelsFeedMaterial[ tIndex ].Time, pBoilerFeedMaterial->EndTime );
		pBoilerFeedMaterial->SensorsEnergy[ tSensor ].push_back( tChannelsFeedMaterial[ tIndex ].Energy );

		tIndex++;
	}

	while ( tIndex < tChannelsFeedMaterial.size() )
	{
		mChannelsFeedMaterial.push_back( tChannelsFeedMaterial[ tIndex ] );
		tIndex++;
	}
}

HSVoid CBoilerPosition::ShowFeedMaterial( HSInt tIndex )
{
	HSInt tCurIndex = mBoilerFeedMaterialInfo.size() - 1;
	list< BoilerFeedMaterial >::iterator pIterator = mBoilerFeedMaterialInfo.end();
	while ( pIterator != mBoilerFeedMaterialInfo.begin() )
	{
		if ( tIndex == tCurIndex )
		{
			break;
		}

		pIterator--;
		tCurIndex--;
	}

	CString tTitleValues[ 6 ] = { "00:00:00", "00:00:00", "", "0", "0", "" };
	BoilerFeedMaterial *pBoilerFeedMaterial = NULL;
	if ( tCurIndex >= 0 )
	{
		pIterator--;
		pBoilerFeedMaterial = &( *pIterator );
		
		HSDouble tRealCycleNum = 0;
		HSBool tOK = IsFeedMaterialOK( pBoilerFeedMaterial, &tRealCycleNum );

		tTitleValues[ 0 ].Format( "%s", theApp.GetLocalTime( theApp.StartTime() + pBoilerFeedMaterial->BeginTime / 1000 / 1000 / 1000 ).c_str() );
		tTitleValues[ 1 ].Format( "%s", theApp.GetLocalTime( theApp.StartTime() + pBoilerFeedMaterial->EndTime / 1000 / 1000 / 1000 ).c_str() );
		tTitleValues[ 2 ].Format( "%s", pBoilerFeedMaterial->Material == MATERIAL_IRON_STONE ? "铁矿石" : "焦炭" );
		tTitleValues[ 3 ].Format( "%.2f", pBoilerFeedMaterial->Material == MATERIAL_IRON_STONE ? mIronStoneCycleNum : mCokeCycleNum );
		tTitleValues[ 4 ].Format( "%.2f", tRealCycleNum );
		tTitleValues[ 5 ].Format( "%s", tOK ? "均匀" : "不均匀" );

		mStaticRelation.SetFeedTitleInfo( tTitleValues, mBoilerFeedMaterialInfo.size(), tCurIndex + 1 );
	}
	else if ( mBoilerFeedMaterialInfo.size() == 0 )
	{
		mStaticRelation.SetFeedTitleInfo( tTitleValues, mBoilerFeedMaterialInfo.size(), 0 );
	}
	
	for ( HSInt i = 0; i < 4; i++ )
	{
		if ( pBoilerFeedMaterial == NULL )
		{
			mStaticRelation.SetPillarArea( i, 0, 0, RGB( 0, 0, 0 ) );
		}
		else
		{
			HSInt tDesiredCount = ( pBoilerFeedMaterial->Material == MATERIAL_IRON_STONE ? mIronStoneCycleNum : mCokeCycleNum ) * 100;
			HSInt tNonFullCycleNum = ( tDesiredCount % 100 ) / 25;
			map< HSInt, HSBool > tNonFullCycleSensors;
			for ( HSInt j = 0; j < tNonFullCycleNum; j++ )
			{
				tNonFullCycleSensors[ ( pBoilerFeedMaterial->BeginSensor + j ) % 4 ] = HSTrue;
			}

			COLORREF tColor = ( pBoilerFeedMaterial->Material == MATERIAL_IRON_STONE ? RGB( 255, 0, 0 ) : RGB( 0, 0, 255 ) );
			
			mStaticRelation.SetPillarArea( i, tDesiredCount / 100 + ( tNonFullCycleSensors.find( i ) == tNonFullCycleSensors.end() ? 0 : 1 ), pBoilerFeedMaterial->SensorsEnergy[ i ].size(), tColor );
		}
	}	

	HSInt tIronGood = 0; 
	HSInt tIronBad = 0;
	HSInt tCokeGood = 0;
	HSInt CokeBad = 0;

	pIterator = mBoilerFeedMaterialInfo.begin();
	while ( pIterator != mBoilerFeedMaterialInfo.end() )
	{
		pBoilerFeedMaterial = &( *pIterator );
		if ( pBoilerFeedMaterial->Material == MATERIAL_IRON_STONE )
		{
			( IsFeedMaterialOK( pBoilerFeedMaterial ) ? tIronGood++ : tIronBad++ );
		}
		else
		{
			( IsFeedMaterialOK( pBoilerFeedMaterial ) ? tCokeGood++ : CokeBad++ );
		}

		pIterator++;		
	}

	mStaticRelation.SetFeedMaterialTotalInfo( tIronGood, tIronBad, tCokeGood, CokeBad );
	mStaticRelation.Invalidate();
}

HSBool CBoilerPosition::IsFeedMaterialOK( BoilerFeedMaterial *pFeedMaterial, HSDouble *pRealCycleNum )
{
	HSDouble tCycleNum = ( pFeedMaterial->Material == MATERIAL_IRON_STONE ? mIronStoneCycleNum : mCokeCycleNum );
		
	HSInt tFullCycle = -1;
	for ( HSInt i = 0; i < 4; i++ )
	{
		if ( HSInt( pFeedMaterial->SensorsEnergy[ i ].size() ) < tFullCycle || tFullCycle < 0 )
		{
			tFullCycle = pFeedMaterial->SensorsEnergy[ i ].size();
		}
	}

	HSDouble tRealCycleNum = 0;
	for ( HSInt i = 0; i < 4; i++ )
	{
		//tRealCycleNum += ( HSInt( pFeedMaterial->SensorsEnergy[ i ].size() ) - tFullCycle );		
		tRealCycleNum += ( ( HSInt( pFeedMaterial->SensorsEnergy[ i ].size() ) - tFullCycle ) > 0 ? 1 : 0 );
	}

	tRealCycleNum /= 4;
	tRealCycleNum += tFullCycle;	
	
	if ( pRealCycleNum != NULL )
	{
		*pRealCycleNum = tRealCycleNum;
	}

	return HSInt( tCycleNum * 100 ) == HSInt( tRealCycleNum * 100 ) || ( tRealCycleNum > tCycleNum );
}

HSVoid CBoilerPosition::InitDialog()
{
	mPopupMenu.LoadMenu( IDR_POPUP_BOILER_POSITION_C );

	CRect tRect;
	this->GetClientRect( &tRect );
	mStaticRelation.Create( "", WS_CHILD | WS_VISIBLE, tRect, this );

	mStaticGraphic = &mStaticRelation;	

	mStaticRelation.SetPillarLength( mPillarsLength );
	mStaticRelation.SetMaterialRadius( mMaterialRadius );
	mStaticRelation.SetXValue( mXAxisLength, -mXAxisLength / 2  );
	mStaticRelation.SetYValue( mYAxisLength, mYAxisLength / 2 );
	mStaticRelation.SetSensors( &mSensors );
	mStaticRelation.SetTitle( mBoilerName.c_str() );

	InitPositionSensors();
}

HSBool CBoilerPosition::GraphicNeedRefresh()
{	
	WaitForSingleObject( mDataMutex, INFINITE );

	vector< ChannelFeedMaterial > tChannelsFeedMaterial = mChannelsFeedMaterial;
	mChannelsFeedMaterial.clear();
	
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.ArgTransfer != NULL )
		{
			pIterator->second.ArgTransfer->GetArg( pIterator->second.ArgList );
			FindChannelFeedMaterialInfo( pIterator->first, tChannelsFeedMaterial );
		}
		pIterator++;
	}	

	FindBoilerFeedMaterialInfo( tChannelsFeedMaterial );
	
	mCurShowFeedMaterialIndex = max( 0, HSInt( mBoilerFeedMaterialInfo.size() ) - 1 );

	ShowFeedMaterial( mCurShowFeedMaterialIndex );

	SetEvent( mDataMutex );

	return HSTrue;
}

HSVoid CBoilerPosition::StepGraphic( HSInt tDelta )
{
	Stop();

	mCurShowFeedMaterialIndex += tDelta;
	mCurShowFeedMaterialIndex = max( min( HSInt( mBoilerFeedMaterialInfo.size() ) - 1, mCurShowFeedMaterialIndex ), 0 );
	ShowFeedMaterial( mCurShowFeedMaterialIndex );
}

HSVoid CBoilerPosition::ZoomInGraphic( CRect &tViewRect )
{		
	mStaticRelation.Invalidate();	
}

HSVoid CBoilerPosition::DragGraphic( CRect &tViewRect, CPoint tPoint )
{
	mStaticRelation.Invalidate();	
}


HSBool CBoilerPosition::IsDataExportNeeded()
{
	return mBoilerFeedMaterialInfo.size() > 0;
}

HSVoid CBoilerPosition::ExportData( CDataExportHelper &tDataExport )
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	HSChar tBuf[ 1024 ];

	sprintf_s( tBuf, "锅炉直径：%.2f (m)\n", mMaterialRadius * 2 );
	tDataExport.Write( tBuf ); 


	sprintf_s( tBuf, "\n% 10s\t% 16s\t% 16s\n", "传感器编号", "采集卡", "通道" );
	tDataExport.Write( tBuf ); 

	map< HSInt, PositionSensor >::iterator pSensorIterator = mPositionSensors.begin();
	while ( pSensorIterator != mPositionSensors.end() )
	{		
		IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( pSensorIterator->second.DataIdentifier );
		sprintf_s( tBuf, "% 10d\t% 16s\t% 16d\n", 
			pSensorIterator->first + 1, 			
			( pDataHandler == NULL ? "" : pDataHandler->Name().c_str() ),
			( pDataHandler == NULL ? -1 : pSensorIterator->second.DataIdentifier.ChannelIndex() + 1 ) );

		tDataExport.Write( tBuf ); 

		pSensorIterator++;
	}

	tDataExport.Write( "\n\n统计信息：\n" ); 
	sprintf_s( tBuf, "\n% 10s\t% 16s\t% 16s\t% 16s\n", "", "均匀次数", "不均匀次数", "总次数" );
	tDataExport.Write( tBuf ); 

	HSInt tIronGood = 0; 
	HSInt tIronBad = 0;
	HSInt tCokeGood = 0;
	HSInt CokeBad = 0;

	list< BoilerFeedMaterial >::iterator pIterator = mBoilerFeedMaterialInfo.begin();
	while ( pIterator != mBoilerFeedMaterialInfo.end() )
	{
		BoilerFeedMaterial *pBoilerFeedMaterial = &( *pIterator );
		if ( pBoilerFeedMaterial->Material == MATERIAL_IRON_STONE )
		{
			( IsFeedMaterialOK( pBoilerFeedMaterial ) ? tIronGood++ : tIronBad++ );
		}
		else
		{
			( IsFeedMaterialOK( pBoilerFeedMaterial ) ? tCokeGood++ : CokeBad++ );
		}

		pIterator++;		
	}

	sprintf_s( tBuf, "% 10s\t% 16d\t% 16d\t% 16d\n", "铁矿石", tIronGood, tIronBad, tIronGood + tIronBad );
	tDataExport.Write( tBuf ); 

	sprintf_s( tBuf, "% 10s\t% 16d\t% 16d\t% 16d\n", "焦  炭", tCokeGood, CokeBad, tCokeGood + CokeBad );
	tDataExport.Write( tBuf ); 

	sprintf_s( tBuf, "% 10s\t% 16d\t% 16d\t% 16d\n", "汇  总", tIronGood + tCokeGood, tIronBad + CokeBad, tIronGood + tIronBad + tCokeGood + CokeBad );
	tDataExport.Write( tBuf ); 

	tDataExport.Write( "\n\n加料信息：\n" ); 
	sprintf_s( tBuf, "\n% 10s\t% 16s\t% 16s\t% 16s\t% 16s\t% 16s\t% 16s\n", "序号", "加料时间", "结束时间", "加料种类", "预期圈数", "实际圈数", "加料结果" );
	tDataExport.Write( tBuf ); 
	pIterator = mBoilerFeedMaterialInfo.begin();
	HSInt tIndex = 1;
	while ( pIterator != mBoilerFeedMaterialInfo.end() )
	{
		BoilerFeedMaterial *pBoilerFeedMaterial = &( *pIterator );

		HSDouble tRealCycleNum = 0;
		HSBool tOK = IsFeedMaterialOK( pBoilerFeedMaterial, &tRealCycleNum );

		sprintf_s( tBuf, "% 10d\t% 16s\t% 16s\t% 16s\t% 16.2f\t% 16.2f\t% 16s\n", 
			tIndex, 
			theApp.GetLocalTime( theApp.StartTime() + pBoilerFeedMaterial->BeginTime / 1000 / 1000 / 1000 ).c_str(), 
			theApp.GetLocalTime( theApp.StartTime() + pBoilerFeedMaterial->EndTime / 1000 / 1000 / 1000 ).c_str(),
			pBoilerFeedMaterial->Material == MATERIAL_IRON_STONE ? "铁矿石" : "焦炭", 
			pBoilerFeedMaterial->Material == MATERIAL_IRON_STONE ? mIronStoneCycleNum : mCokeCycleNum, 
			tRealCycleNum,
			tOK ? "均匀" : "不均匀" );
		tDataExport.Write( tBuf ); 

		tIndex++;
		pIterator++;				
	}
}

HSVoid CBoilerPosition::ResetData( CMainFrame* pMainFrame )
{
	WaitForSingleObject( mDataMutex, INFINITE );
		
	if ( pMainFrame->IsStart() )
	{
		Start();
	}
	else
	{
		mCurShowFeedMaterialIndex = 0;
		this->ShowFeedMaterial( mCurShowFeedMaterialIndex );
	}

	SetEvent( mDataMutex );
}

