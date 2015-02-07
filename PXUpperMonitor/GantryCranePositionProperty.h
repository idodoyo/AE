#pragma once

#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"
#include "NormalProperty.h"

class CGantryCranePosition;

class CGantryCranePositionProperty : public CNormalProperty
{
public:
	CGantryCranePositionProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGantryCranePositionProperty();

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

public:
	virtual HSBool IsDataHandlerMachedWith( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSVoid *pSensorInfo );
	virtual HSVoid InitDialog();
	virtual HSVoid PropertyChanged( CMFCPropertyGridProperty * pProperty );	
	virtual HSVoid SensorClicked( HSInt tIndex );

public:
	enum { PROPERTY_SENSOR_COUNT = 1, PROPERTY_WIDTH, PROPERTY_NAME, PROPERTY_SENSOR_FORBID, PROPERTY_SENSOR_X, PROPERTY_SENSOR_DATA_SOURCE, PROPERTY_HIT_COLOR };
	enum { POSITION_SENSOR_MAX_NUM = 16 };

	CGantryCranePosition *mParent;
};
