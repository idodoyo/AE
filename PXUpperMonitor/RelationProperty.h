#pragma once


#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"
#include "NormalProperty.h"

class CGraphicRelation;

// CRelationProperty dialog

class CRelationProperty : public CNormalProperty
{
public:
	CRelationProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRelationProperty();
public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();
	
public:
	virtual HSBool IsDataHandlerMachedWith( IDataHandler *pDataHandler, HSUInt tChannel );
	virtual HSVoid InitDialog();
	virtual HSVoid PropertyChanged( CMFCPropertyGridProperty * pProperty );	

private:
	enum { PROPERTY_X = 1, PROPERTY_Y, PROPERTY_FILL_RECT, PROPERTY_SQUARE_LENGTH, PROPERTY_DATA_SOURCE };

	CGraphicRelation *mParent;
};
