// CylinderPosition.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "GantryCranePosition.h"
#include "afxdialogex.h"
#include "GantryCranePositionProperty.h"
#include <algorithm>
#include "MainFrm.h"
#include "DataExportHelper.h"
#include "HSLogProtocol.h"
#include "ArgCalculator.h"


// CGantryCranePosition dialog

CGantryCranePosition::CGantryCranePosition(CWnd* pParent /*=NULL*/)
	: CNormalGraphic( pParent )
{		
	mMaterialLength = 10.0;
	mGantryCraneName = "龙门吊1";	
	mLastHitTime = 0;
}

CGantryCranePosition::~CGantryCranePosition()
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

HSBool CGantryCranePosition::Start()
{
	if ( mPositionSensors.size() > 0 )
	{		
		mRefreshThread->Start();
		return HSTrue;
	}

	return HSFalse;
}

HSBool CGantryCranePosition::Save( CINIConfig *pIniConfig, string tGroup )
{	
	pIniConfig->SetValue( "MaterialLength", this->mMaterialLength, tGroup );
	pIniConfig->SetValue( "GantryCraneName", this->mGantryCraneName, tGroup );

	HSChar tBuf[ 1024 ];
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn )
		{
			sprintf_s( tBuf, "Sensor%d_DataIdentifier", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.DataIdentifier.ToString(), tGroup );

			sprintf_s( tBuf, "Sensor%d_Pos", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Pos, tGroup );	

			sprintf_s( tBuf, "Sensor%d_Forbid", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Forbid, tGroup );
		}
		pIterator++;
	}	
	
	return HSTrue;
}

HSBool CGantryCranePosition::Load( CINIConfig *pIniConfig, string tGroup )
{			
	pIniConfig->ValueWithKey( "MaterialLength", this->mMaterialLength, tGroup );
	pIniConfig->ValueWithKey( "GantryCraneName", this->mGantryCraneName, tGroup );

	HSString tDataIdentifier = "";
	HSDouble tPosition = 0;
	HSBool tForbid = HSFalse;

	HSChar tBufId[ 1024 ];
	HSChar tBufPos[ 1024 ];
	HSChar tBufForbid[ 1024 ];

	for ( HSInt i = 0; i < CGantryCranePositionProperty::POSITION_SENSOR_MAX_NUM; i++ )
	{
		sprintf_s( tBufId, "Sensor%d_DataIdentifier", i );
		sprintf_s( tBufPos, "Sensor%d_Pos", i );
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
		}
	}	
	
	return HSTrue;
}
	
IGraphicPropertyProtocol * CGantryCranePosition::CreatePropertyDlg( CWnd *pParent )
{
	CGantryCranePositionProperty *pPositionProperty = new CGantryCranePositionProperty;
	pPositionProperty->Create( IDD_NORMALPROPERTY, pParent );
	return pPositionProperty;
}

HSVoid CGantryCranePosition::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mDataMutex, INFINITE );

	mHitsPosition.clear();
	mLastHitTime = 0;
	mPositionArgs.clear();

	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		this->UpdatePositionSensor( pIterator->first, pIterator->second.Pos, pIterator->second.Forbid, pIterator->second.IsOn, pIterator->second.DataIdentifier );
		pIterator++;
	}

	SetEvent( mDataMutex );	
}

HSVoid CGantryCranePosition::InitPositionSensors()
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
	
	RefreshSensors();
	mCylinder->Refresh();	
}

HSVoid CGantryCranePosition::SetFocusSensor( HSInt tIndex )
{
	mCylinder->SetFocusdSensor( tIndex );
	mCylinder->Refresh();	
}

HSVoid CGantryCranePosition::SetMaterialLength( HSDouble tLength )
{
	mMaterialLength = max( tLength, 0.2 );
	mCylinder->SetMaterialLength( mMaterialLength  );
	this->RefreshSensors();
}

HSVoid CGantryCranePosition::SetGantryCraneName( HSString tName )
{
	this->mGantryCraneName = tName;

	CString tFullName = mGantryCraneName.c_str();
	tFullName += " 尺寸(米)";

	mStaticCylinder.SetTitle( tFullName );
	mStaticCylinder.Invalidate();
	mCylinder->Invalidate();
}

HSVoid CGantryCranePosition::UpdatePositionSensor( HSInt tIndex, HSDouble tPosition, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier )
{
	if ( mPositionSensors.find( tIndex ) != mPositionSensors.end() )
	{
		if ( mPositionSensors[ tIndex ].ArgTransfer != NULL )
		{
			delete mPositionSensors[ tIndex ].ArgTransfer;				
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
}

HSVoid CGantryCranePosition::RefreshSensors()
{
	mSensors.clear();
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			CGraphicGantryCrane::SensorInfo tSensor;
			tSensor.Index = pIterator->first;
			tSensor.PosX = pIterator->second.Pos;
			mSensors[ tSensor.Index ] = tSensor;
		}
		pIterator++;
	}

	mCylinder->Refresh();
}

CGantryCranePosition::PositionSensor * CGantryCranePosition::GetPositionSensor( HSInt tIndex )
{
	if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
	{
		return NULL;
	}

	return &( mPositionSensors[ tIndex ] );
}

HSVoid CGantryCranePosition::CalcHitPosition( vector< PositionInfo > &tPositionArgs )
{
	if ( tPositionArgs.size() < 1 )
	{
		return;
	}

	HSDouble tVelocity = CArgCalculator::SharedInstance()->GetMaterialVelocity();
	HSInt64 tHitDuration = CArgCalculator::SharedInstance()->GetHitFullTime() + 1 * 1000000;
	HSInt64 tMaxTime = HSInt64( mMaterialLength / ( tVelocity / 1000.0 ) * 1000000000.0 ) * 10;

	HSInt tIndex = 0;
	HSInt tStartIndex = 0;
	HSInt64 tTime = tPositionArgs[ 0 ].Time;
	for ( HSUInt i = 0; i < tPositionArgs.size(); i++ )
	{
		if ( tPositionArgs[ i ].Time - tTime > tMaxTime )
		{			
			//HS_INFO( "Begin Cal Hit Between ( %d, %lf ) - ( %d, %lf ), Duration: %d", tStartIndex, mPositionSensors[ tPositionArgs[ tStartIndex ].Sensor ].Pos, i - 1, mPositionSensors[ tPositionArgs[ i - 1 ].Sensor ].Pos, HSInt( ( tTime - mLastHitTime ) / 1000000 ) );
			
			if ( ( tPositionArgs[ i - 1 ].Sensor != tPositionArgs[ tStartIndex ].Sensor ) && ( tTime - mLastHitTime > tHitDuration ) )
			{
				HSDouble tSensorOnePos = mPositionSensors[ tPositionArgs[ tStartIndex ].Sensor ].Pos;
				HSDouble tSensorTuePos = mPositionSensors[ tPositionArgs[ i - 1 ].Sensor ].Pos;

				HSDouble tDuration = ( tTime - tPositionArgs[ i - 1 ].Time ) / 1000000000.0 * ( tSensorTuePos > tSensorOnePos ? 1 : -1 );
				HSDouble tResPos = ( tDuration * tVelocity / 1000.0 + tSensorOnePos + tSensorTuePos ) / 2.0;
				CGraphicGantryCrane::HitPosition tHit = { tResPos, CGraphicManager::SharedInstance()->ColorWithEnergy( max( tPositionArgs[ i - 1 ].Energy, tPositionArgs[ tStartIndex ].Energy ) ) };
				if ( !( ( tResPos > tSensorOnePos && tResPos < tSensorTuePos ) || ( tResPos > tSensorTuePos && tResPos < tSensorOnePos ) ) )
				{
					tHit.PosX = tSensorOnePos + 0.0001 * ( ( tSensorOnePos + tSensorTuePos ) / 2 > tSensorOnePos ? 1 : -1 );
				}
				mHitsPosition.push_back( tHit );

				//HS_INFO( "ResPos: %lf, tDuration: %lf", tHit.PosX, tDuration );
			}

			mLastHitTime = tPositionArgs[ i - 1 ].Time;

			tIndex = i;
			tStartIndex = i;
			tTime = tPositionArgs[ i ].Time;
		}
	}

	for ( HSUInt i = tIndex; i < tPositionArgs.size(); i++ )
	{
		mPositionArgs.push_back( tPositionArgs[ i ] );
	}
}

HSVoid CGantryCranePosition::FocusWnd()
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	pMainFrame->SetFocusedGraphic( this );
	if ( !mStaticCylinder.Focused() )
	{			
		Sleep( 10 );

		mStaticCylinder.SetFocused( true );
		mStaticCylinder.ResizeWave();
		mStaticCylinder.Invalidate();
		mCylinder->Invalidate();
	}		
}

HSVoid CGantryCranePosition::InitDialog()
{
	mPopupMenu.LoadMenu( IDR_POPUP_GANTRY_CRANE_POSITION_C );

	CRect tRect;
	this->GetClientRect( &tRect );
	mStaticCylinder.Create( "", WS_CHILD | WS_VISIBLE, tRect, this );

	CString tName = mGantryCraneName.c_str();
	tName += " 尺寸(米)";

	mStaticGraphic = &mStaticCylinder;
	mStaticGraphic->SetTitle( tName );

	mCylinder = new CGraphicGantryCrane( this );
	mCylinder->SetHits( &mHitsPosition );
	mCylinder->SetSensor( &mSensors );		

	mCylinder->SetMaterialLength( mMaterialLength );

	mCylinder->Create( IDD_GRAPHICNORMAL3D, this );
	mCylinder->ShowWindow( SW_SHOW );

	InitPositionSensors();
}

HSVoid CGantryCranePosition::ViewResized( CRect &tRect )
{
	mStaticCylinder.MoveWindow( tRect, true );
	mStaticCylinder.SetXValue( mStaticCylinder.TotalXValue(), mStaticCylinder.BeginXValue() );
	mStaticCylinder.SetYValue( mStaticCylinder.TotalYValue(), mStaticCylinder.BeginYValue() );
	mStaticCylinder.ResizeWave();
	mStaticCylinder.Invalidate();

	CRect tSphereRect( tRect.left + mStaticCylinder.LeftDelta(), tRect.top + mStaticCylinder.TopDelta(), tRect.right - mStaticCylinder.LeftDelta(), tRect.bottom - mStaticCylinder.TopDelta() );
	mCylinder->MoveWindow( tSphereRect );
}


HSBool CGantryCranePosition::GraphicNeedRefresh()
{	
	vector< PositionInfo > tPositionArgs = mPositionArgs;
	mPositionArgs.clear();

	vector< CArgTableManager::ArgItem > tArgList;
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.ArgTransfer != NULL && pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			pIterator->second.ArgTransfer->GetArg( tArgList );
			for ( HSUInt i = 0; i < tArgList.size(); i++ )
			{
				PositionInfo tPositionInfo = { pIterator->first, tArgList[ i ].NBeginTime, tArgList[ i ].Energy };
				tPositionArgs.push_back( tPositionInfo );
			}

			tArgList.clear();
		}
		pIterator++;
	}			
	
	std::sort( tPositionArgs.begin(), tPositionArgs.end(), PositionInfoCompare );

	CalcHitPosition( tPositionArgs );

	mCylinder->InDirectRefresh();

	return HSTrue;
}


HSBool CGantryCranePosition::IsDataExportNeeded()
{
	return HSTrue;
}

HSVoid CGantryCranePosition::ExportData( CDataExportHelper &tDataExport )
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	HSChar tBuf[ 1024 ];

	sprintf_s( tBuf, "长度：%.2f (m)\n", mMaterialLength );
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
				pSensorIterator->second.Pos, 					
				( pDataHandler == NULL ? "" : pDataHandler->Name().c_str() ),
				( pDataHandler == NULL ? -1 : pSensorIterator->second.DataIdentifier.ChannelIndex() + 1 ) );

			tDataExport.Write( tBuf ); 
		}

		pSensorIterator++;
	}		

	sprintf_s( tBuf, "\n% 10s\t% 18s\n", "撞击编号", "X(m)" );
	tDataExport.Write( tBuf ); 

	HSInt tIndex = 1;
	list< CGraphicGantryCrane::HitPosition >::iterator pHitIterator = mHitsPosition.begin();
	while ( pHitIterator != mHitsPosition.end() )
	{
		sprintf_s( tBuf, "% 10d\t% 18f\n", tIndex++, ( *pHitIterator ).PosX );
		tDataExport.Write( tBuf ); 

		pHitIterator++;
	}
}

HSVoid CGantryCranePosition::ResetData( CMainFrame* pMainFrame )
{
	mCylinder->Reset();
	mCylinder->Refresh();
}

HSVoid CGantryCranePosition::AutoSetSensors()
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
}

HSVoid CGantryCranePosition::Graphic3DRefresh( HSBool tNeedInvalidate )
{
	if ( tNeedInvalidate )
	{
		mCylinder->Invalidate();
	}
	else
	{
		mCylinder->Refresh();
	}
}