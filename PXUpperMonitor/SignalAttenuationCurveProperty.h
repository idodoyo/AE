#pragma once

#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"
#include "NormalProperty.h"

class CSignalAttenuationCurve;

// CSignalAttenuationCurveProperty dialog

class CSignalAttenuationCurveProperty : public CNormalProperty
{
public:
	CSignalAttenuationCurveProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSignalAttenuationCurveProperty();
	
public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

public:
	virtual HSBool IsDataHandlerMachedWith( IDataHandler *pDataHandler, HSUInt tChannel, HSVoid *pInfo );
	virtual HSVoid InitDialog();
	virtual HSVoid PropertyChanged( CMFCPropertyGridProperty * pProperty );	

private:
	enum { PROPERTY_DATA_SOURCE = 1, PROPERTY_DATA_SOURCE_ORIGIN };

	CSignalAttenuationCurve *mParent;
};
