#pragma once

#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"
#include "NormalProperty.h"

class CGraphicWave;

// CWaveProperty dialog

class CWaveProperty : public CNormalProperty
{
public:
	CWaveProperty(CWnd* pParent = NULL);
	virtual ~CWaveProperty();

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();	

public:
	virtual HSBool IsDataHandlerMachedWith( IDataHandler *pDataHandler, HSUInt tChannel );
	virtual HSVoid InitDialog();
	virtual HSVoid PropertyChanged( CMFCPropertyGridProperty * pProperty );	

private:
	enum { PROPERTY_VOLTAGE = 1, PROPERTY_TIME, PROPERTY_DISCARD_DATA, PROPERTY_OFFSET_VOLTAGE, PROPERTY_DATA_SOURCE, PROPERTY_BYPASS_VOLTAGE };

	CGraphicWave *mParent;

	static int mVoltages[ 11 ];
	static int mUSeconds[ 12 ];
	static CString mStrVoltages[ 11 ];
	static CString mStrTimes[ 12 ];
};
