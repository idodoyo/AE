#pragma once

#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"
#include "NormalProperty.h"

class CGraphicFFT;

// CGraphicFFTProperty dialog

class CGraphicFFTProperty : public CNormalProperty
{
public:
	CGraphicFFTProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicFFTProperty();

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

public:
	virtual HSBool IsDataHandlerMachedWith( IDataHandler *pDataHandler, HSUInt tChannel );
	virtual HSVoid InitDialog();
	virtual HSVoid PropertyChanged( CMFCPropertyGridProperty * pProperty );	

private:
	enum { PROPERTY_AMPLITUDE = 1, PROPERTY_BEGIN_AMPLITUDE, PROPERTY_POINT_LENGTH, PROPERTY_DATA_SOURCE };

	CGraphicFFT *mParent;
};
