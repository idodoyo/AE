#pragma once

#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"
#include "NormalProperty.h"

class CArgTable;

// CArgProperty dialog

class CArgProperty : public CNormalProperty
{
public:
	CArgProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CArgProperty();

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

public:
	virtual HSBool IsDataHandlerMachedWith( IDataHandler *pDataHandler, HSUInt tChannel );
	virtual HSVoid InitDialog();
	virtual HSVoid PropertyChanged( CMFCPropertyGridProperty * pProperty );	

private:
	enum { PROPERTY_BYPASS_VOLTAGE = 1, PROPERTY_DATA_SOURCE };

	CArgTable *mParent;
};
