#pragma once

#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"

class CLinearPosition;


// CLinearPositionProperty dialog

class CLinearPositionProperty : public CDialogEx, public IGraphicPropertyProtocol
{
	DECLARE_DYNAMIC(CLinearPositionProperty)

public:
	CLinearPositionProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLinearPositionProperty();

// Dialog Data
	enum { IDD = IDD_LINEARPOSITIONPROPERTY };

	void OnOK(){}
	void OnCancel(){}

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

	virtual CDialogEx * Wnd(){ return this; }	

public:
	enum { PROPERTY_SENSOR_COUNT = 1, PROPERTY_WIDTH, PROPERTY_SENSOR_FORBID, PROPERTY_SENSOR_X, PROPERTY_SENSOR_DATA_SOURCE, PROPERTY_X_AXIS_LENGTH, PROPERTY_HIT_RADUS, PROPERTY_HIT_COLOR, PROPERTY_CHECK_CIRCLE_RADIUS, PROPERTY_CHECK_CIRCLE_X };
	enum { POSITION_SENSOR_MAX_NUM = 100 };

	CLinearPosition *mParent;

	bool mIsInit;
	
	map< DEVICE_CH_IDENTIFIER, vector< HSInt > > mDeviceChannelInfo;	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CMFCPropertyGridCtrl mGridPosition;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();

	LRESULT OnPropertyChanged( WPARAM wparam, LPARAM lparam );
	afx_msg void OnClickedMfcpropertygridLinearPosition();
};
