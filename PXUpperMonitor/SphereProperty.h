#pragma once
#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"
#include "NormalProperty.h"

class CSpherePosition;


// CSphereProperty dialog

class CSphereProperty : public CNormalProperty
{
public:
	CSphereProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSphereProperty();

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

public:
	virtual HSBool IsDataHandlerMachedWith( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSVoid *pSensorInfo );
	virtual HSVoid InitDialog();
	virtual HSVoid PropertyChanged( CMFCPropertyGridProperty * pProperty );	
	virtual HSVoid SensorClicked( HSInt tIndex );

public:
	enum { PROPERTY_SENSOR_COUNT = 1, PROPERTY_RADIUS, PROPERTY_SENSOR_FORBID, PROPERTY_SENSOR_ANGLE_D, PROPERTY_SENSOR_ANGLE_U, PROPERTY_SENSOR_DATA_SOURCE, PROPERTY_HIT_RADUS };
	enum { POSITION_SENSOR_MAX_NUM = 100 };

	CSpherePosition *mParent;
};
