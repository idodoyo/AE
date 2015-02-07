#pragma once

#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"
#include "NormalProperty.h"

class CLinearPosition;

// CLinearPositionProperty dialog

class CLinearPositionProperty : public CNormalProperty
{
public:
	CLinearPositionProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLinearPositionProperty();

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

public:
	virtual HSBool IsDataHandlerMachedWith( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSVoid *pSensorInfo );
	virtual HSVoid InitDialog();
	virtual HSVoid PropertyChanged( CMFCPropertyGridProperty * pProperty );	
	virtual HSVoid SensorClicked( HSInt tIndex );

public:
	enum { PROPERTY_SENSOR_COUNT = 1, PROPERTY_WIDTH, PROPERTY_SENSOR_FORBID, PROPERTY_SENSOR_X, PROPERTY_SENSOR_DATA_SOURCE, PROPERTY_X_AXIS_LENGTH, PROPERTY_HIT_RADUS, PROPERTY_HIT_COLOR, PROPERTY_CHECK_CIRCLE_RADIUS, PROPERTY_CHECK_CIRCLE_X };
	enum { POSITION_SENSOR_MAX_NUM = 100 };

	CLinearPosition *mParent;
};
