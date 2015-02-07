#pragma once
#include "afxpropertygridctrl.h"
#include "GraphicManager.h"
#include "DataTypes.h"
#include "DataHandler.h"
#include "NormalProperty.h"

class CCylinderSurfacePosition;


// CCylinderSurfacePositionProperty dialog

class CCylinderSurfacePositionProperty : public CNormalProperty
{
public:
	CCylinderSurfacePositionProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCylinderSurfacePositionProperty();

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

public:
	virtual HSBool IsDataHandlerMachedWith( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSVoid *pSensorInfo );
	virtual HSVoid InitDialog();
	virtual HSVoid PropertyChanged( CMFCPropertyGridProperty * pProperty );	
	virtual HSVoid SensorClicked( HSInt tIndex );

public:
	enum { PROPERTY_SENSOR_COUNT = 1, PROPERTY_Y, PROPERTY_MATERIAL_Y, PROPERTY_MATERIAL_RADIUS, PROPERTY_SENSOR_FORBID, PROPERTY_SENSOR_ANGLE, PROPERTY_SENSOR_HEIGHT, PROPERTY_SENSOR_DATA_SOURCE, PROPERTY_HIT_RADUS };
	enum { POSITION_SENSOR_MAX_NUM = 100 };

	CCylinderSurfacePosition *mParent;
};
