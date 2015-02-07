// CylinderSurfacePosition.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "CylinderSurfacePosition.h"
#include "afxdialogex.h"
#include "CylinderSurfacePositionProperty.h"
#include <algorithm>
#include "MainFrm.h"
#include "DataExportHelper.h"
#include "HSLogProtocol.h"
#include "ArgCalculator.h"


// CCylinderSurfacePosition dialog

IMPLEMENT_DYNAMIC(CCylinderSurfacePosition, CDialogEx)

CCylinderSurfacePosition::CCylinderSurfacePosition(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCylinderSurfacePosition::IDD, pParent)
{
	mInit = HSFalse;
	mRelationMutex = CreateEvent( NULL, FALSE, TRUE, NULL );	

	mUpdateRelationThread = new CThreadControlEx< CCylinderSurfacePosition >( this, 800, 0 );

	mIsFullScreen = HSFalse;
	
	mIsDraging = HSFalse;
	mIsZoomIn = HSFalse;

	mDragingSensorIndex = -1;
	
	mMaterialYLength = 1500;
	mMaterialRadius = 500;

	mAxisYLength = 2000;	

	mHitRadius = 3;
}

CCylinderSurfacePosition::~CCylinderSurfacePosition()
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

HSBool CCylinderSurfacePosition::Start()
{
	if ( mPositionSensors.size() > 0 )
	{		
		mUpdateRelationThread->Start();
		return HSTrue;
	}

	return HSFalse;
}

HSVoid CCylinderSurfacePosition::Pause()
{
	mUpdateRelationThread->Pause();
}

HSVoid CCylinderSurfacePosition::Stop()
{
	mUpdateRelationThread->Stop();
}

HSBool CCylinderSurfacePosition::Save( CINIConfig *pIniConfig, string tGroup )
{				
	pIniConfig->SetValue( "MaterialYLength", this->mMaterialYLength, tGroup );
	pIniConfig->SetValue( "MaterialRadius", this->mMaterialRadius, tGroup );	
	
	pIniConfig->SetValue( "AxisYLength", this->mAxisYLength, tGroup );

	pIniConfig->SetValue( "HitRadius", this->mHitRadius, tGroup );	
		
	HSChar tBuf[ 1024 ];
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn )
		{
			sprintf_s( tBuf, "Sensor%d_DataIdentifier", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.DataIdentifier.ToString(), tGroup );			

			sprintf_s( tBuf, "Sensor%d_Angle", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Angle, tGroup );

			sprintf_s( tBuf, "Sensor%d_Height", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Height, tGroup );

			sprintf_s( tBuf, "Sensor%d_Forbid", pIterator->first );
			pIniConfig->SetValue( tBuf, pIterator->second.Forbid, tGroup );
		}
		pIterator++;
	}	
	
	return HSTrue;
}

HSBool CCylinderSurfacePosition::Load( CINIConfig *pIniConfig, string tGroup )
{		
	pIniConfig->ValueWithKey( "MaterialYLength", this->mMaterialYLength, tGroup );	
	pIniConfig->ValueWithKey( "MaterialRadius", this->mMaterialRadius, tGroup );
	
	pIniConfig->ValueWithKey( "AxisYLength", this->mAxisYLength, tGroup );	

	pIniConfig->ValueWithKey( "HitRadius", this->mHitRadius, tGroup );
	
	HSString tDataIdentifier = "";
	HSDouble tAngle = 0;
	HSDouble tHeight = 0;	
	HSBool tForbid = HSFalse;

	HSChar tBufId[ 1024 ];
	HSChar tBufAngle[ 1024 ];
	HSChar tBufHeight[ 1024 ];	
	HSChar tBufForbid[ 1024 ];

	for ( HSInt i = 0; i < CCylinderSurfacePositionProperty::POSITION_SENSOR_MAX_NUM; i++ )
	{
		sprintf_s( tBufId, "Sensor%d_DataIdentifier", i );
		sprintf_s( tBufAngle, "Sensor%d_Angle", i );
		sprintf_s( tBufHeight, "Sensor%d_Height", i );		
		sprintf_s( tBufForbid, "Sensor%d_Forbid", i );
		if ( pIniConfig->ValueWithKey( tBufId, tDataIdentifier, tGroup )
			&& pIniConfig->ValueWithKey( tBufAngle, tAngle, tGroup )
			&& pIniConfig->ValueWithKey( tBufHeight, tHeight, tGroup )	
			&& pIniConfig->ValueWithKey( tBufForbid, tForbid, tGroup ) )
		{						
			mPositionSensors[ i ].Angle = tAngle;
			mPositionSensors[ i ].Height = tHeight;
			mPositionSensors[ i ].Forbid = tForbid;
			mPositionSensors[ i ].IsOn = HSTrue;
			mPositionSensors[ i ].DataIdentifier.InitWithString( tDataIdentifier );
			mPositionSensors[ i ].ArgTransfer = NULL;
		}
	}	
	
	return HSTrue;
}

HSVoid CCylinderSurfacePosition::FocusGraphic( HSBool tIsFocused )
{
	if ( tIsFocused && !mStaticCylinderSurface.Focused() )
	{
		mStaticCylinderSurface.SetFocused( true );
		mStaticCylinderSurface.ResizeWave();
		mStaticCylinderSurface.Invalidate();
		mCylinder->Invalidate();
	}
	else if ( !tIsFocused && mStaticCylinderSurface.Focused() )
	{
		mStaticCylinderSurface.SetFocused( false );
		mStaticCylinderSurface.ResizeWave();
		mStaticCylinderSurface.Invalidate();
		mCylinder->Invalidate();
	}	
}
	
IGraphicPropertyProtocol * CCylinderSurfacePosition::CreatePropertyDlg( CWnd *pParent )
{
	CCylinderSurfacePositionProperty *pPositionProperty = new CCylinderSurfacePositionProperty;
	pPositionProperty->Create( IDD_CYLINDERSURFACEPOSITIONPROPERTY, pParent );
	return pPositionProperty;	
}

HSVoid CCylinderSurfacePosition::SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier )
{
	WaitForSingleObject( mRelationMutex, INFINITE );
	
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		this->UpdatePositionSensor( pIterator->first, pIterator->second.Angle, pIterator->second.Height, pIterator->second.Forbid, pIterator->second.IsOn, pIterator->second.DataIdentifier );
		pIterator++;
	}
	
	this->mHitsPosition.clear();

	this->OnCylinderSurfacePositionPopupReset();

	SetEvent( mRelationMutex );	
}

HSDouble CCylinderSurfacePosition::DistanceBetweenSensors( HSInt tSensorOne, HSInt tSensorTwo )
{
	HSDouble tX1Angle = mPositionSensors[ tSensorOne ].Angle;
	HSDouble tX2Angle = mPositionSensors[ tSensorTwo ].Angle;
	
	tX2Angle = tX1Angle + min( 360 - abs( tX2Angle - tX1Angle ), abs( tX2Angle - tX1Angle ) );

	HSDouble tX = XPosWithAngle( tX1Angle ) - XPosWithAngle( tX2Angle );
	HSDouble tY = mPositionSensors[ tSensorOne ].Height - mPositionSensors[ tSensorTwo ].Height;
	
	return sqrt( tX * tX + tY * tY );
}

HSDouble CCylinderSurfacePosition::DistanceFromSensor( HSDouble tPosX, HSDouble tPosY, HSInt tSensor )
{
	HSDouble tX1Angle = AngleWithXPos( tPosX );
	HSDouble tX2Angle = mPositionSensors[ tSensor ].Angle;
	
	tX2Angle = tX1Angle + min( 360 - abs( tX2Angle - tX1Angle ), abs( tX2Angle - tX1Angle ) );

	HSDouble tX = tPosX - XPosWithAngle( tX2Angle );
	HSDouble tY = tPosY - mPositionSensors[ tSensor ].Height;
	
	return sqrt( tX * tX + tY * tY );	
}

HSVoid CCylinderSurfacePosition::SetHitRadius( HSInt tHitRadius )
{
	mHitRadius = tHitRadius;
	mCylinder->SetHitRadius( mHitRadius );
}

HSBool CCylinderSurfacePosition::IsHitQualified( HSInt64 tDuration, HSDouble tVelocity, HSInt tSensorOne, HSInt tSensorTwo )
{	
	return abs( tDuration / 1000000000.0 * tVelocity ) < DistanceBetweenSensors( tSensorOne, tSensorTwo );
}

HSBool CCylinderSurfacePosition::IsResQualified( HSDouble tPosX, HSDouble tPosY, vector< CPlanePositionCalulator::HIT_CALC_INFO > &tHitSensors, HSDouble &tEnergy )
{	
	if ( tPosX < 0 || tPosX > XPosWithAngle( 360 ) || tPosY < 0 || tPosY > this->mMaterialYLength )
	{
		return HSFalse;
	}	

	std::sort( tHitSensors.begin(), tHitSensors.end(), CPlanePositionCalulator::HitSensorComapre );	

	vector< CPlanePositionCalulator::SENSOR_DIST_INFO > tPosDistance;
	for ( HSUInt i = 0; i < tHitSensors.size(); i++ )
	{
		CPlanePositionCalulator::SENSOR_DIST_INFO tDistanceInfo = { tHitSensors[ i ].SensorIndex, DistanceFromSensor( tPosX, tPosY, tHitSensors[ i ].SensorIndex ) };
		tPosDistance.push_back( tDistanceInfo );

		tEnergy = max( tEnergy, mPositionSensors[ tHitSensors[ i ].SensorIndex ].ArgList[ tHitSensors[ i ].ArgIndex ].Energy );
	}

	std::sort( tPosDistance.begin(), tPosDistance.end(), CPlanePositionCalulator::HitSensorDistanceComapre );

	for ( HSUInt i = 0; i < tHitSensors.size(); i++ )
	{
		if ( tHitSensors[ i ].SensorIndex != tPosDistance[ i ].SensorIndex )
		{
			return HSFalse;
		}
	}
	
	return HSTrue;
}

HSBool CCylinderSurfacePosition::CalcHitPosition( HSInt tSensor, HSUInt tArgIndex )
{	
	vector< CPlanePositionCalulator::HIT_CALC_INFO > tHitSensors;	
	
	CPlanePositionCalulator::HIT_CALC_INFO tHit = { tSensor, tArgIndex, this };
	tHitSensors.push_back( tHit );

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
			CGraphicCylinderSurface::HitPosition tHitPos = { AngleWithXPos( tResHitPos[ i ].PosX ), tResHitPos[ i ].PosY, CGraphicManager::SharedInstance()->ColorWithEnergy( tEnergy ) };
			
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

HSBool CCylinderSurfacePosition::ThreadRuning( HSInt tThreadID )
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
	
	mCylinder->InDirectRefresh();
	
	return HSTrue;
}

HSVoid CCylinderSurfacePosition::ThreadWillStop( HSInt tThreadID )
{
}

HSVoid CCylinderSurfacePosition::InitPositionSensors()
{		
	HSInt tIndex = 0;		
	HSInt tAngle = 0;
	
	while ( mPositionSensors.size() < DEFALUT_SENSOR_NUM )
	{
		if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
		{
			this->UpdatePositionSensor( tIndex, tAngle, 500, HSFalse, HSTrue, 0 );		
			tAngle = ( tAngle + 10 ) % 360;
		}

		tIndex++;
	}
	
	RefreshSensors();
	mCylinder->Refresh();	
}

HSVoid CCylinderSurfacePosition::SetFocusSensor( HSInt tIndex )
{
	mCylinder->SetFocusdSensor( tIndex );
	mCylinder->Refresh();	
}

HSVoid CCylinderSurfacePosition::SetMaterialYLength( HSDouble tLength )
{
	mMaterialYLength = tLength;
	mCylinder->SetMaterialHeight( mMaterialYLength / 1000.0 );
	this->RefreshSensors();	
}

HSVoid CCylinderSurfacePosition::SetMaterialRadius( HSDouble tRadius )
{
	mMaterialRadius = tRadius;		
}

HSVoid CCylinderSurfacePosition::SetYAxisLength( HSDouble tY )
{
	mAxisYLength = tY;
	mCylinder->SetHeight(  mAxisYLength / 1000.0 );
	mCylinder->Refresh();	
}

HSVoid CCylinderSurfacePosition::UpdatePositionSensor( HSInt tIndex, HSDouble tAngle, HSDouble tHeight, HSBool tForbid, HSBool tIsOn, DEVICE_CH_IDENTIFIER tDataIdentifier )
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

	mPositionSensors[ tIndex ].Angle = tAngle;
	mPositionSensors[ tIndex ].Height = tHeight;
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
}

HSVoid CCylinderSurfacePosition::RefreshSensors()
{
	mSensors.clear();
	map< HSInt, PositionSensor >::iterator pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			CGraphicCylinderSurface::SensorInfo tSensor;
			tSensor.Index = pIterator->first;
			tSensor.Angle = pIterator->second.Angle;
			tSensor.Height = pIterator->second.Height;

			mSensors[ pIterator->first ] = tSensor;
		}
		pIterator++;
	}

	mCylinder->Refresh();
}

CCylinderSurfacePosition::PositionSensor * CCylinderSurfacePosition::GetPositionSensor( HSInt tIndex )
{
	if ( mPositionSensors.find( tIndex ) == mPositionSensors.end() )
	{
		return NULL;
	}

	return &( mPositionSensors[ tIndex ] );
}

void CCylinderSurfacePosition::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CYLINDER_SURFACE, mStaticCylinderSurface);
}


BEGIN_MESSAGE_MAP(CCylinderSurfacePosition, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_WM_INITMENUPOPUP()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_CYLINDER_SURFACE_POSITION_POPUP_AUTO, &CCylinderSurfacePosition::OnCylinderSurfacePositionPopupAuto)
	ON_COMMAND(ID_CYLINDER_SURFACE_POSITION_POPUP_CLOSE, &CCylinderSurfacePosition::OnCylinderSurfacePositionPopupClose)
	ON_COMMAND(ID_CYLINDER_SURFACE_POSITION_POPUP_EXPORT_DATA, &CCylinderSurfacePosition::OnCylinderSurfacePositionPopupExportData)
	ON_COMMAND(ID_CYLINDER_SURFACE_POSITION_POPUP_FULL_SCREEN, &CCylinderSurfacePosition::OnCylinderSurfacePositionPopupFullScreen)
	ON_UPDATE_COMMAND_UI(ID_CYLINDER_SURFACE_POSITION_POPUP_FULL_SCREEN, &CCylinderSurfacePosition::OnUpdateCylinderSurfacePositionPopupFullScreen)
	ON_COMMAND(ID_CYLINDER_SURFACE_POSITION_POPUP_RESET, &CCylinderSurfacePosition::OnCylinderSurfacePositionPopupReset)
	ON_COMMAND(ID_CYLINDER_SURFACE_POSITION_POPUP_SETDATASOURCE, &CCylinderSurfacePosition::OnCylinderSurfacePositionPopupSetdatasource)
END_MESSAGE_MAP()


// CCylinderSurfacePosition message handlers


BOOL CCylinderSurfacePosition::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	mInit = HSTrue;

	mPopupMenu.LoadMenu( IDR_POPUP_CYLINDER_SURFACE_POSITION_C );		

	mCylinder = new CGraphicCylinderSurface( this );
	mCylinder->SetHits( &mHitsPosition );
	mCylinder->SetSensor( &mSensors );	

	mCylinder->SetHeight( mAxisYLength / 1000.0 );
	mCylinder->SetMaterialHeight( mMaterialYLength / 1000.0 );

	mCylinder->Create( IDD_GRAPHICCYLINDERSURFACE, this );
	mCylinder->ShowWindow( SW_SHOW );

	mCylinder->SetHitRadius( mHitRadius );

	InitPositionSensors();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CCylinderSurfacePosition::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	mUpdateRelationThread->Stop();
}


void CCylinderSurfacePosition::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: Add your message handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->SetGrahpicCheckType( CMainFrame::GRAPHIC_CHECK_POINTER );
	
	mPopupMenu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this );
}


void CCylinderSurfacePosition::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialogEx::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

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


void CCylinderSurfacePosition::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	mParent->EndMoveSubView( nFlags, point);

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CCylinderSurfacePosition::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	CPoint tPoint = point;
	
	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticCylinderSurface.LeftDelta() && tPoint.x < tRect.right - mStaticCylinderSurface.LeftDelta() && tPoint.y > tRect.top + mStaticCylinderSurface.TopDelta() && tPoint.y < tRect.bottom - mStaticCylinderSurface.TopDelta() )
	{		
	}	
	else
	{	
		mParent->BeginMoveSubView( this, nFlags, point );
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

HSVoid CCylinderSurfacePosition::FocusWnd()
{
	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	pMainFrame->SetFocusedGraphic( this );
	if ( !mStaticCylinderSurface.Focused() )
	{			
		Sleep( 10 );

		mStaticCylinderSurface.SetFocused( true );
		mStaticCylinderSurface.ResizeWave();
		mStaticCylinderSurface.Invalidate();
		mCylinder->Invalidate();
	}			
}

void CCylinderSurfacePosition::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	mParent->MoveingSubView( nFlags, point);

	CDialogEx::OnMouseMove(nFlags, point);
}


void CCylinderSurfacePosition::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if ( mInit )
	{
		CRect tRect;
		this->GetClientRect( &tRect );
		if ( tRect.Width() > mStaticCylinderSurface.LeftDelta() * 3 && tRect.Height() > mStaticCylinderSurface.TopDelta() * 3 )
		{
			mStaticCylinderSurface.MoveWindow( tRect, true );
			mStaticCylinderSurface.SetXValue( mStaticCylinderSurface.TotalXValue(), mStaticCylinderSurface.BeginXValue() );
			mStaticCylinderSurface.SetYValue( mStaticCylinderSurface.TotalYValue(), mStaticCylinderSurface.BeginYValue() );
			mStaticCylinderSurface.ResizeWave();
			mStaticCylinderSurface.Invalidate();
						
			CRect tCylinderRect( tRect.left + mStaticCylinderSurface.LeftDelta(), tRect.top + mStaticCylinderSurface.TopDelta(), tRect.right - mStaticCylinderSurface.LeftDelta(), tRect.bottom - mStaticCylinderSurface.TopDelta() );
			mCylinder->MoveWindow( tCylinderRect );					
		}
	}
}


BOOL CCylinderSurfacePosition::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

	CPoint tPoint;
	::GetCursorPos( &tPoint );
	this->ScreenToClient( &tPoint );		

	CRect tRect;
	this->GetClientRect( &tRect );
	if ( tPoint.x > tRect.left + mStaticCylinderSurface.LeftDelta() && tPoint.x < tRect.right - mStaticCylinderSurface.LeftDelta() && tPoint.y > tRect.top + mStaticCylinderSurface.TopDelta() && tPoint.y < tRect.bottom - mStaticCylinderSurface.TopDelta() )
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

	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}


void CCylinderSurfacePosition::OnCylinderSurfacePositionPopupAuto()
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

	if ( tSensorCount < 3 )
	{
		return;
	}	

	HSInt tRows = 2;	

	HSDouble tAngleDelta = 360.0 / tSensorCount;
	HSDouble tHeightDelta = this->mMaterialYLength / ( tRows - 1 );

	HSInt tIndex = 0;
	HSInt tRowIndex = 0;
	pIterator = mPositionSensors.begin();
	while ( pIterator != mPositionSensors.end() )
	{
		if ( pIterator->second.IsOn && !pIterator->second.Forbid )
		{
			pIterator->second.Angle = tAngleDelta * tIndex;
			pIterator->second.Height = tRowIndex * tHeightDelta;					

			tIndex++;
			tRowIndex++;

			if ( tRowIndex == tRows )
			{
				tRowIndex = 0;
			}
		}

		pIterator++;
	}

	RefreshSensors();

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());	
	pMainFrame->SetFocusedGraphic( this, HSTrue );
}


void CCylinderSurfacePosition::OnCylinderSurfacePositionPopupClose()
{
	// TODO: Add your command handler code here

	CDialogEx::OnOK();

	mParent->SubViewClosed( this );
}


void CCylinderSurfacePosition::OnCylinderSurfacePositionPopupExportData()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());

	HSChar tBuf[ 1024 ];
	CString tStrChannel;

	CDataExportHelper tDataExport( this );
	if ( tDataExport.GetFilePath() )
	{
		tDataExport.Write( "柱面定位\n" );
		sprintf_s( tBuf, "日期: %s %s\n", theApp.GetLocalDate().c_str(), theApp.GetLocalTime().c_str() );
		tDataExport.Write( tBuf ); 

		sprintf_s( tBuf, "材料尺寸(半径，高度)：%.2f (m), %.2f (m)\n", this->mMaterialRadius / 1000.0, this->mMaterialYLength / 1000.0 );
		tDataExport.Write( tBuf ); 

		sprintf_s( tBuf, "\n% 10s\t% 18s\t% 18s\t% 16s\t% 16s\n", "传感器编号", "角度", "高度(m)", "采集卡", "通道" );
		tDataExport.Write( tBuf );

		map< HSInt, PositionSensor >::iterator pSensorIterator = mPositionSensors.begin();
		while ( pSensorIterator != mPositionSensors.end() )
		{
			if ( pSensorIterator->second.IsOn && !pSensorIterator->second.Forbid )
			{
				tStrChannel.Format( "%d", pSensorIterator->second.DataIdentifier.ChannelIndex() + 1 );
				IDataHandler *pDataHandler = pMainFrame->DeviceManager()->DataHandlerWithIdentifier( pSensorIterator->second.DataIdentifier );
				sprintf_s( tBuf, "% 10d\t% 18f\t% 18f\t% 16s\t% 16s\n", 
					pSensorIterator->first + 1, 
					pSensorIterator->second.Angle, 
					pSensorIterator->second.Height / 1000.0, 					
					( pDataHandler == NULL ? "" : pDataHandler->Name().c_str() ),
					( pDataHandler == NULL ? "" : ( LPCSTR )tStrChannel ) );

				tDataExport.Write( tBuf ); 
			}

			pSensorIterator++;
		}

		sprintf_s( tBuf, "\n% 10s\t% 18s\t% 18s\n", "撞击编号", "角度", "高度(m)" );
		tDataExport.Write( tBuf ); 

		HSInt tIndex = 1;
		list< CGraphicCylinderSurface::HitPosition >::iterator pHitIterator = mHitsPosition.begin();
		while ( pHitIterator != mHitsPosition.end() )
		{
			sprintf_s( tBuf, "% 10d\t% 18f\t% 18f\n", tIndex++, pHitIterator->Angle, pHitIterator->Height / 1000.0 );
			tDataExport.Write( tBuf ); 

			pHitIterator++;
		}

		tDataExport.Finish();
	}	
}


void CCylinderSurfacePosition::OnCylinderSurfacePositionPopupFullScreen()
{
	// TODO: Add your command handler code here

	mIsFullScreen = !mIsFullScreen;
	mParent->SubViewFullScreen( this, mIsFullScreen );

	mCylinder->Refresh();
}


void CCylinderSurfacePosition::OnUpdateCylinderSurfacePositionPopupFullScreen(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->SetCheck( mIsFullScreen );

	mCylinder->Refresh();
}


void CCylinderSurfacePosition::OnCylinderSurfacePositionPopupReset()
{
	// TODO: Add your command handler code here

	mCylinder->Reset();
	mCylinder->Refresh();
}


void CCylinderSurfacePosition::OnCylinderSurfacePositionPopupSetdatasource()
{
	// TODO: Add your command handler code here

	CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
	pMainFrame->OnViewGraphic();	
	pMainFrame->SetFocusedGraphic( this );

	if ( !mStaticCylinderSurface.Focused() || pMainFrame->FocusedGraphic() != this )
	{		
		mStaticCylinderSurface.SetFocused( true );
		mStaticCylinderSurface.ResizeWave();
		mStaticCylinderSurface.Invalidate();
		mCylinder->Invalidate();
	}
}

