#pragma once
#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"
#include "NormalProperty.h"

class CPlanePosition;

// CPlanePositionProperty dialog

class CPlanePositionProperty : public CNormalProperty
{
public:
	CPlanePositionProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPlanePositionProperty();

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

public:
	virtual HSBool IsDataHandlerMachedWith( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSVoid *pSensorInfo );
	virtual HSVoid InitDialog();
	virtual HSVoid PropertyChanged( CMFCPropertyGridProperty * pProperty );	
	virtual HSVoid SensorClicked( HSInt tIndex );

public:
	enum { PROPERTY_SENSOR_COUNT = 1, PROPERTY_WIDTH, PROPERTY_HEIGHT, PROPERTY_SENSOR_FORBID, PROPERTY_SENSOR_X, PROPERTY_SENSOR_Y, PROPERTY_SENSOR_DATA_SOURCE, PROPERTY_X_AXIS_LENGTH, PROPERTY_Y_AXIS_LENGTH, 
			PROPERTY_HIT_RADUS, PROPERTY_HIT_COLOR, PROPERTY_CHECK_CIRCLE_RADIUS, PROPERTY_CHECK_CIRCLE_X, PROPERTY_CHECK_CIRCLE_Y };
	enum { POSITION_SENSOR_MAX_NUM = 100 };

	CPlanePosition *mParent;
};