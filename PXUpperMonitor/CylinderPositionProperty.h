#pragma once
#include "afxpropertygridctrl.h"
#include "GraphicManager.h"
#include "DataTypes.h"
#include "DataHandler.h"
#include "NormalProperty.h"

class CCylinderPosition;


// CCylinderPositionProperty dialog

class CCylinderPositionProperty : public CNormalProperty
{
public:
	CCylinderPositionProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCylinderPositionProperty();

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();
	
public:
	virtual HSBool IsDataHandlerMachedWith( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSVoid *pSensorInfo );
	virtual HSVoid InitDialog();
	virtual HSVoid PropertyChanged( CMFCPropertyGridProperty * pProperty );	
	virtual HSVoid SensorClicked( HSInt tIndex );

public:
	enum { PROPERTY_SENSOR_COUNT = 1, PROPERTY_X, PROPERTY_Y, PROPERTY_Z, PROPERTY_MATERIAL_X, PROPERTY_MATERIAL_Y, PROPERTY_MATERIAL_Z, PROPERTY_SENSOR_FORBID, PROPERTY_SENSOR_FACE, PROPERTY_SENSOR_FIRST_COORD, PROPERTY_SENSOR_SECOND_COORD, PROPERTY_SENSOR_DATA_SOURCE, PROPERTY_HIT_RADUS };
	enum { POSITION_SENSOR_MAX_NUM = 100 };

	CCylinderPosition *mParent;
};
