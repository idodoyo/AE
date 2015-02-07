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

IMPLEMENT_DYNAMIC(CLinearPosition, CDialogEx)

CLinearPosition::CLinearPosition(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLinearPosition::IDD, pParent)
{
	mInit = HSFalse;
	mRelationMutex = CreateEvent( NULL, FALSE, TRUE, NULL );	

	mUpdateRelationThread = new CThreadControlEx< CLinearPosition >( this, 800, 0 );

	mIsFullScreen = HSFalse;
	
	mIsDraging = HSFalse;
	mIsZoomIn = HSFalse;

	mDragingSensorIndex = -1;
	mMaterialLength = 1000.0;

	mHitRadius = 3;
	mXAxisLength = 2.0;

	mCheckRadius = 20;
	mDragCheckCircle = HSFalse;
	mCheckCircleEnable = HSFalse;
}

CLinearPosition::~CLinearPosition()
{
	CloseHandle( mRelationMutex );

	delete mUpdateRelationThread;

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
		mUpdateRelationThread->Start();
		return HSTrue;
	}
	return HSFalse;
}

HSVoid CLinearPosition::Pause()
{
	mUpdateRelationThread->Pause();
}

HSVoid CLinearPosition::Stop()
{
	mUpdateRelationThread->Stop();
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

HSVoid CLinearPosition::FocusGraphic( HSBool tIsFocused )
{
	if ( tIsFocused && !mStaticRelation.Focused() )
	{
		mStaticRelation.SetFocused( true );
		mStaticRelation.ResizeWave();
		mStaticRelation.Invalidate();		
	}
	else if ( !tIsFocused && mStaticRelation.Focused() )
	{
		mStaticRelation.SetFocused( false );
		mStaticRelation.ResizeWave();
		mStaticRelation.Invalidate();		
	}	
}
	
IGraphicPropertyProtocol * CLinearPosition::CreatePropertyDlg( CWnd *pParent )
{
	CLinearPositionProperty *pPositionProperty = new CLinearPositionProperty;
	pPositionProperty->Create( IDD_LINEARPOSITIONPROPERTY, pParent );
	return pPositionProperty;
}

HSVoid CLinearPosition::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mRelationMutex, INFINITE );

	mHitsPosition.clear();

	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		this->UpdatePositionSensor( pIterator->first, pIterator->second.Pos, pIterator->second.Forbid, pIterator->second.IsOn, pIterator->second.DataIdentifier );
		pIterator++;
	}

	SetEvent( mRelationMutex );	
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

HSBool CLinearPosition::ThreadRuning( HSInt tThreadID )
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

HSVoid CLinearPosition::ThreadWillStop( HSInt tThreadID )
{
}

HSVoid CLinearPosition::SetMaterialLength( HSDouble tLength )
{
	mMaterialLength = max( tLength, 10 );
	mStaticRelation.SetMaterialLength( mMaterialLength );
	
	this->OnLinearPositionPopupAuto();
}

HSVoid CLinearPosition::InitPositionSensors()
{
	HSDouble tPosPercent = 0.1;	
	HSInt tIndex = 0;
	while ( mPositionSensors.size() < 2 )
	{
		if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
		{
			this->UpdatePositionSensor( tIndex, tPosPercent * mMaterialLength, HSFalse, HSTrue, 0 );		
			tPosPercent = 0.9;
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

void CLinearPosition::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_POSITION, mStaticRelation);
}


BEGIN_MESSAGE_MAP(CLinearPosition, CDialogEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_WM_INITMENUPOPUP()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_LINEAR_POSITION_POPUP_CLOSE, &CLinearPosition::OnLinearPositionPopupClose)
	ON_COMMAND(ID_LINEAR_POSITION_POPUP_FULL_SCREEN, &CLinearPosition::OnLinearPositionPopupFullScreen)
	ON_UPDATE_COMMAND_UI(ID_LINEAR_POSITION_POPUP_FULL_SCREEN, &CLinearPosition::OnUpdateLinearPositionPopupFullScreen)
	ON_COMMAND(ID_LINEAR_POSITION_POPUP_RESET, &CLinearPosition::OnLinearPositionPopupReset)
	ON_COMMAND(ID_LINEAR_POSITION_POPUP_SETDATASOURCE, &CLinearPosition::OnLinearPositionPopupSetdatasource)
	ON_COMMAND(ID_LINEAR_POSITION_POPUP_AUTO, &CLinearPosition::OnLinearPositionPopupAuto)
	ON_COMMAND(ID_LINEAR_POSITION_POPUP_EXPORT_DATA, &CLinearPosition::OnLinearPositionPopupExportData)
	ON_COMMAND(ID_LINEAR_POSITION_POPUP_CHECK_CIRCLE, &CLinearPosition::OnLinearPositionPopupCheckCircle)
	ON_UPDATE_COMMAND_UI(ID_LINEAR_POSITION_POPUP_CHECK_CIRCLE, &CLinearPosition::OnUpdateLinearPositionPopupCheckCircle)
END_MESSAGE_MAP()


// CLinearPosition message handlers


BOOL CLinearPosition::OnInitDialog()
{
	__super::OnInitDialog();

	// TODO:  Add extra initialization here

	mInit = HSTrue;

	mPopupMenu.LoadMenu( IDR_POPUP_LINEAR_POSITION_C );	

	mStaticRelation.SetHitsPosition( &mHitsPosition );
	mStaticRelation.SetSensors( &mSensors );
	mStaticRelation.SetMaterialLength( this->mMaterialLength );
	mStaticRelation.SetXValue( mXAxisLength, 0 );
	mStaticRelation.SetHitRadius( mHitRadius );

	mStaticRelation.SetCheckRadius( mCheckRadius, HSFalse );
	mStaticRelation.SetCheckPos( mMaterialLength / 2, 0 );

	InitPositionSensors();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CLinearPosition::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	CPoint tPoint = point;
	
	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticRelation.LeftDelta() && tPoint.x < tRect.right - mStaticRelation.LeftDelta() && tPoint.y > tRect.top + mStaticRelation.TopDelta() && tPoint.y < tRect.bottom - mStaticRelation.TopDelta() )
	{
		if ( pMainFrame->GetGrahpicCheckType() != CMainFrame::GRAPHIC_CHECK_POINTER )
		{			
			//if ( mUpdateRelationThread->IsStart() )
			//{
			//	this->Stop();
			//}
		}
		else
		{
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

		pMainFrame->SetFocusedGraphic( this );
		if ( !mStaticRelation.Focused() )
		{			
			mStaticRelation.SetFocused( true );
			mStaticRelation.ResizeWave();
			mStaticRelation.Invalidate();
		}
		
		if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_STEP_FRAME )
		{
			int tDelta = 1;
			if ( tPoint.x < tRect.right / 2 )
			{
				tDelta = -1;
			}

			WaitForSingleObject( mRelationMutex, INFINITE );		

			mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() + tDelta * mStaticRelation.TotalXValue() );
			mStaticRelation.Invalidate();

			SetEvent( mRelationMutex );				
		}
		else if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_RECTANGE )
		{
			mIsDraging = HSFalse;
			mIsZoomIn = HSTrue;
			mZoomInBeginPoint = tPoint;
			mFirstTimeDrawZoomRect = HSTrue;
		}
		else if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_DRAG )
		{
			mIsDraging = HSTrue;
			mIsZoomIn = HSFalse;
			mPrevPoint = tPoint;
		}
	}	
	else
	{	
		mParent->BeginMoveSubView( this, nFlags, point);
	}

	__super::OnLButtonDown(nFlags, point);
}


void CLinearPosition::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default	
	if ( mDragCheckCircle )
	{
		mDragCheckCircle = HSFalse;
		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
		pMainFrame->SetFocusedGraphic( this, HSTrue );
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
	}
	else if ( mIsDraging )
	{
	}
	else if ( mIsZoomIn )
	{
		CDC *pDC = this->GetDC();
		CRect tRect( 0, 0, 0, 0 );
		CBrush tBrush;		
		tBrush.CreateSolidBrush( RGB( 200, 200, 200 ) );
		pDC->DrawDragRect( tRect, CSize( 1, 1 ), mZoomInRect, CSize( 1, 1 ) , &tBrush, &tBrush );

		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		tViewRect = CRect( tViewRect.left + mStaticRelation.LeftDelta(), tViewRect.top + mStaticRelation.TopDelta(), tViewRect.right - mStaticRelation.LeftDelta(), tViewRect.bottom - mStaticRelation.TopDelta() );
		
		WaitForSingleObject( mRelationMutex, INFINITE );			
		
		HSDouble tXValueOffset = max( 0.0, mZoomInRect.left - mStaticRelation.LeftDelta() ) / tViewRect.Width() * mStaticRelation.TotalXValue();
		HSDouble tNewXValue = ( HSDouble )( mZoomInRect.Width() ) / tViewRect.Width() * mStaticRelation.TotalXValue();			

		//HSDouble tBeginYValue = mStaticRelation.BeginYValue() - max( 0.0, mZoomInRect.top - mStaticRelation.TopDelta() ) / tViewRect.Height() * mStaticRelation.TotalYValue();
		//HSDouble tTotalYValue = ( HSDouble )mZoomInRect.Height() / tViewRect.Height() * mStaticRelation.TotalYValue();
		
		mStaticRelation.SetXValue( tNewXValue, mStaticRelation.BeginXValue() + tXValueOffset );
		//mStaticRelation.SetYValue( tTotalYValue, tBeginYValue );		
		mStaticRelation.Invalidate();
		
		SetEvent( mRelationMutex );
	}
	else
	{
		mParent->EndMoveSubView( nFlags, point);
	}

	mIsDraging = HSFalse;
	mIsZoomIn = HSFalse;	
	mDragingSensorIndex = -1;

	__super::OnLButtonUp(nFlags, point);
}


void CLinearPosition::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if ( mDragCheckCircle )
	{
		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		tViewRect = CRect( tViewRect.left + mStaticRelation.LeftDelta(), tViewRect.top + mStaticRelation.TopDelta(), tViewRect.right - mStaticRelation.LeftDelta(), tViewRect.bottom - mStaticRelation.TopDelta() );

		HSInt tXOffset = mPrevPoint.x - point.x;
		HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticRelation.TotalXValue();
		
		mStaticRelation.SetCheckPos( mStaticRelation.XCheckPos() - tXOffsetValue * 1000.0, mStaticRelation.YCheckPos() );
		mStaticRelation.Invalidate();

		mPrevPoint = point;
	}
	else if ( mDragingSensorIndex >= 0 )
	{
		PositionSensor *pPositionSensor = GetPositionSensor( mDragingSensorIndex );
		if ( pPositionSensor != NULL )
		{
			CRect tViewRect;
			this->GetClientRect( &tViewRect );
			tViewRect = CRect( tViewRect.left + mStaticRelation.LeftDelta(), tViewRect.top + mStaticRelation.TopDelta(), tViewRect.right - mStaticRelation.LeftDelta(), tViewRect.bottom - mStaticRelation.TopDelta() );

			HSInt tXOffset = mPrevPoint.x - point.x;
			HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticRelation.TotalXValue();
			this->UpdatePositionSensor( mDragingSensorIndex, pPositionSensor->Pos - tXOffsetValue * 1000.0, pPositionSensor->Forbid, pPositionSensor->IsOn, pPositionSensor->DataIdentifier );			
		}

		mPrevPoint = point;
	}
	else if ( mIsDraging )
	{
		WaitForSingleObject( mRelationMutex, INFINITE );		

		CRect tViewRect;
		this->GetClientRect( &tViewRect );
		tViewRect = CRect( tViewRect.left + mStaticRelation.LeftDelta(), tViewRect.top + mStaticRelation.TopDelta(), tViewRect.right - mStaticRelation.LeftDelta(), tViewRect.bottom - mStaticRelation.TopDelta() );

		//HSInt tYOffset = mPrevPoint.y - point.y;			
		//HSDouble tYValuePerDigit = mStaticRelation.TotalYValue() / tViewRect.Height();
		//HSDouble tBeginYValue = mStaticRelation.BeginYValue() - tYOffset * tYValuePerDigit;			
		//mStaticRelation.SetYValue( mStaticRelation.TotalYValue(), tBeginYValue );		

		HSInt tXOffset = mPrevPoint.x - point.x;
		HSDouble tXOffsetValue = ( HSDouble )tXOffset / tViewRect.Width() * mStaticRelation.TotalXValue();		
		mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() + tXOffsetValue );
		mStaticRelation.Invalidate();
		
		mPrevPoint = point;
		
		SetEvent( mRelationMutex );			
	}
	else if ( mIsZoomIn )
	{
		CRect tRect = CRect( mZoomInBeginPoint, point );		
		tRect.NormalizeRect();

		CDC *pDC = this->GetDC();		
		CBrush tBrush;		
		tBrush.CreateSolidBrush( RGB( 200, 200, 200 ) );
		if ( mFirstTimeDrawZoomRect )
		{
			pDC->DrawDragRect( tRect, CSize( 1, 1 ), NULL, CSize( 1, 1 ) , &tBrush, &tBrush );
			mFirstTimeDrawZoomRect = HSFalse;
		}
		else
		{
			pDC->DrawDragRect( tRect, CSize( 1, 1 ), mZoomInRect, CSize( 1, 1 ) , &tBrush, &tBrush );
		}

		mZoomInRect = tRect;
	}
	else
	{
		mParent->MoveingSubView( nFlags, point);
	}		

	__super::OnMouseMove(nFlags, point);
}


void CLinearPosition::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( mInit )
	{
		CRect tRect;
		this->GetClientRect( &tRect );
		if ( tRect.Width() > mStaticRelation.LeftDelta() * 3 && tRect.Height() > mStaticRelation.TopDelta() * 3 )
		{
			mStaticRelation.MoveWindow( tRect, true );
			mStaticRelation.SetXValue( mStaticRelation.TotalXValue(), mStaticRelation.BeginXValue() );
			mStaticRelation.SetYValue( mStaticRelation.TotalYValue(), mStaticRelation.BeginYValue() );
			mStaticRelation.ResizeWave();
			mStaticRelation.Invalidate();
		}
	}
}


void CLinearPosition::OnDestroy()
{
	__super::OnDestroy();

	// TODO: Add your message handler code here

	mUpdateRelationThread->Stop();
}


void CLinearPosition::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: Add your message handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->SetGrahpicCheckType( CMainFrame::GRAPHIC_CHECK_POINTER );
	
	mPopupMenu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this );
}


void CLinearPosition::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	__super::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// TODO: Add your message handler code here
	ASSERT(pPopupMenu != NULL);     
	// Check the enabled state of various menu items.     

	CCmdUI state;     
	state.m_pMenu = pPopupMenu;     
	ASSERT(state.m_pOther == NULL);     
	ASSERT(state.m_pParentMenu == NULL);     

	// Determine if menu is popup in top-level menu and set m_pOther to     
	// it if so (m_pParentMenu == NULL indicates that it is secondary popup).     
	HMENU hParentMenu;     
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)     
		state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.     
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)     
	{     
		CWnd* pParent = this;     
		// Child windows don't have menus--need to go to the top!     
		if (pParent != NULL &&     
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)     
		{     
			int nIndexMax = ::GetMenuItemCount(hParentMenu);     
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)     
			{     
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)     
				{     
					// When popup is found, m_pParentMenu is containing menu.     
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);     
					break;     
				}     
			}     
		}     
	}     

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();     
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;     
		state.m_nIndex++)     
	{     
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);     
		if (state.m_nID == 0)     
			continue; // Menu separator or invalid cmd - ignore it.     

		ASSERT(state.m_pOther == NULL);     
		ASSERT(state.m_pMenu != NULL);     
		if (state.m_nID == (UINT)-1)     
		{     
			// Possibly a popup menu, route to first item of that popup.     
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);     
			if (state.m_pSubMenu == NULL ||     
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||     
				state.m_nID == (UINT)-1)     
			{     
				continue;       // First item of popup can't be routed to.     
			}     
			state.DoUpdate(this, TRUE);   // Popups are never auto disabled.     
		}     
		else    
		{     
			// Normal menu item.     
			// Auto enable/disable if frame window has m_bAutoMenuEnable     
			// set and command is _not_ a system command.     
			state.m_pSubMenu = NULL;     
			state.DoUpdate(this, FALSE);     
		}     

		// Adjust for menu deletions and additions.     
		UINT nCount = pPopupMenu->GetMenuItemCount();     
		if (nCount < state.m_nIndexMax)     
		{     
			state.m_nIndex -= (state.m_nIndexMax - nCount);     
			while (state.m_nIndex < nCount &&     
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)     
			{     
				state.m_nIndex++;     
			}     
		}     
		state.m_nIndexMax = nCount;     
	} 
}


BOOL CLinearPosition::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );		

	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticRelation.LeftDelta() && tPoint.x < tRect.right - mStaticRelation.LeftDelta() && tPoint.y > tRect.top + mStaticRelation.TopDelta() && tPoint.y < tRect.bottom - mStaticRelation.TopDelta() )
	{		
		CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
		if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_STEP_FRAME )
		{
			if ( tPoint.x < tRect.right / 2 )
			{
				SetCursor( LoadCursor( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDC_CURSOR_PREV_FRAME ) ) );
			}
			else
			{
				SetCursor( LoadCursor( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDC_CURSOR_NEXT_FRAME ) ) );
			}

			return true;
		}
		else if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_RECTANGE )
		{
			SetCursor( AfxGetApp()->LoadStandardCursor( IDC_CROSS ) );
			return true;
		}
		else if ( pMainFrame->GetGrahpicCheckType() == CMainFrame::GRAPHIC_CHECK_DRAG )
		{
			SetCursor( AfxGetApp()->LoadStandardCursor( IDC_HAND ) );
			return true;
		}
	}	

	return __super::OnSetCursor(pWnd, nHitTest, message);
}


void CLinearPosition::OnLinearPositionPopupClose()
{
	// TODO: Add your command handler code here
	CDialogEx::OnOK();

	mParent->SubViewClosed( this );
}


void CLinearPosition::OnLinearPositionPopupFullScreen()
{
	// TODO: Add your command handler code here
	mIsFullScreen = !mIsFullScreen;
	mParent->SubViewFullScreen( this, mIsFullScreen );
}


void CLinearPosition::OnUpdateLinearPositionPopupFullScreen(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->SetCheck( mIsFullScreen );
}


void CLinearPosition::OnLinearPositionPopupReset()
{
	// TODO: Add your command handler code here

	WaitForSingleObject( mRelationMutex, INFINITE );	
	
	mStaticRelation.SetXValue( mXAxisLength, 0 );	

	mStaticRelation.SetYValue( 10, 5 );

	mStaticRelation.Invalidate();

	SetEvent( mRelationMutex );
}


void CLinearPosition::OnLinearPositionPopupSetdatasource()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->OnViewGraphic();	
	pMainFrame->SetFocusedGraphic( this );

	if ( !mStaticRelation.Focused() || pMainFrame->FocusedGraphic() != this )
	{		
		mStaticRelation.SetFocused( true );
		mStaticRelation.ResizeWave();
		mStaticRelation.Invalidate();
	}
}


void CLinearPosition::OnLinearPositionPopupAuto()
{
	// TODO: Add your command handler code here
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

	HSDouble tDistance = HSInt( ( this->mMaterialLength - this->mMaterialLength / 10.0 * 2.0 ) / ( tSensorCount - 1 ) );
	HSDouble tPosition = HSInt( this->mMaterialLength / 10.0 );
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


void CLinearPosition::OnLinearPositionPopupExportData()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	HSChar tBuf[ 1024 ];

	CDataExportHelper tDataExport( this );
	if ( tDataExport.GetFilePath() )
	{
		tDataExport.Write( "线性定位\n" );
		sprintf_s( tBuf, "日期: %s %s\n", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
		tDataExport.Write( tBuf ); 	

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

		tDataExport.Finish();
	}	
}


void CLinearPosition::OnLinearPositionPopupCheckCircle()
{
	// TODO: Add your command handler code here

	mCheckCircleEnable = !mCheckCircleEnable;
	mStaticRelation.SetCheckRadius( this->mCheckRadius, mCheckCircleEnable );
	mStaticRelation.SetCheckPos( mMaterialLength / 2, mStaticRelation.YCheckPos() );
	mStaticRelation.Invalidate();
}


void CLinearPosition::OnUpdateLinearPositionPopupCheckCircle(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( mCheckCircleEnable );	
}
