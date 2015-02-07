#pragma once
#include "afxwin.h"

#include "DataHandle\DataTypes.h"
#include "Static3DCrackPosition.h"
#include "DataHandler.h"
#include "LinearTransfer.h"
#include "ThreadControlEx.h"
#include "resource.h"
#include "GraphicManager.h"
#include "ArgListTransfer.h"
#include "ArgDataFectcher.h"
#include "GraphicCylinder.h"
#include "CrackPosition.h"
#include "NormalGraphic.h"
#include "GraphicNormal3D.h"


// C3DCrackPosition dialog

class C3DCrackPosition : public CNormalGraphic
{
public:
	C3DCrackPosition(CWnd* pParent = NULL);   // standard constructor
	virtual ~C3DCrackPosition();		

public:
	static IGraphicProtocol * CreateProto(){ return new C3DCrackPosition; }
	
	virtual HSString Identifier(){ return "GRAPHIC_3DCRACK_POSITION"; }
	virtual HSString Name(){ return "3DÁÑ·ìÍ¼"; }

	virtual HSBool Start();

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup );
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup );		

	virtual HSInt LargeIcon(){ return IDB_BITMAP_3DCRACK_POSITION; }	
	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent );

	virtual IGraphicProtocol * Clone(){ return new C3DCrackPosition; }

	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier );

public:
	virtual HSBool GraphicNeedRefresh();	

	virtual HSVoid InitDialog();	
	virtual HSVoid ViewResized( CRect &tRect );
	
	virtual HSVoid ResetData( CMainFrame* pMainFrame );		

	virtual HSVoid Graphic3DRefresh( HSBool tNeedInvalidate );	

public:	
	virtual HSVoid FocusWnd();
	
public:		
	HSVoid SetFocusSensor( HSInt tIndex );	

	HSVoid SetXAxisLength( HSDouble tX );
	HSVoid SetYAxisLength( HSDouble tY );	
	HSVoid GetAxisLength( HSDouble &tX, HSDouble &tY ){ tX = mAxisXLength; tY = mAxisZLength; }
	
	HSVoid RefreshSensors();	

	HSInt HitRadius(){ return mHitRadius; }
	HSVoid SetHitRadius( HSInt tHitRadius );	

	CCrackPosition * RefCrackPosition();

private:

	map< HSInt, CGraphicCylinder::SensorInfo > mSensors;
	list< CGraphicCylinder::HitPosition > mHitsPosition;
	list< HSDouble > mRandonHeights;

	CGraphicCylinder *mCylinder;
	
	HSDouble mAxisXLength;
	HSDouble mAxisYLength;
	HSDouble mAxisZLength;	

	HSInt mHitRadius;

	enum { DEFALUT_SENSOR_NUM = 6 };	

	CStatic3DCrackPosition mStaticRelation;
};
