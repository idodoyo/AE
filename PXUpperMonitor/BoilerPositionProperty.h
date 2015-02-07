#pragma once
#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"
#include "NormalProperty.h"

class CBoilerPosition;

// CPlanePositionProperty dialog

class CBoilerPositionProperty : public CNormalProperty
{
public:
	CBoilerPositionProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBoilerPositionProperty();

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

public:
	virtual HSBool IsDataHandlerMachedWith( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSVoid *pSensorInfo );
	virtual HSVoid InitDialog();
	virtual HSVoid PropertyChanged( CMFCPropertyGridProperty * pProperty );	
	virtual HSVoid SensorClicked( HSInt tIndex );

public:
	enum { PROPERTY_SENSOR_COUNT = 1, PROPERTY_RADIUS, PROPERTY_PILLAR1_LENGTH, PROPERTY_PILLAR2_LENGTH, PROPERTY_PILLAR3_LENGTH, PROPERTY_PILLAR4_LENGTH, PROPERTY_SENSOR_DATA_SOURCE, PROPERTY_X_AXIS_LENGTH, PROPERTY_Y_AXIS_LENGTH,
		   PROPERTY_BOILER_NAME, PROPERTY_IRONSTONE_CIRLE_COUNT, PROPERTY_IRONSTONE_TIME, PROPERTY_COKE_CIRLE_COUNT, PROPERTY_COKE_TIME, PROPERTY_IRONSTONE_COLOR, PROPERTY_COKE_COLOR  };
	enum { POSITION_SENSOR_MAX_NUM = 4 };

	CBoilerPosition *mParent;
};
