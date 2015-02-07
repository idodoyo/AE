#pragma once
#include "afxpropertygridctrl.h"
#include "GraphicManager.h"
#include "DataTypes.h"
#include "DataHandler.h"

class CCylinderPosition;


// CCylinderPositionProperty dialog

class CCylinderPositionProperty : public CDialogEx, public IGraphicPropertyProtocol
{
	DECLARE_DYNAMIC(CCylinderPositionProperty)

public:
	CCylinderPositionProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCylinderPositionProperty();

// Dialog Data
	enum { IDD = IDD_CYLINDERPOSITIONPROPERTY };

	void OnOK(){}
	void OnCancel(){}

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

	virtual CDialogEx * Wnd(){ return this; }	

public:
	enum { PROPERTY_SENSOR_COUNT = 1, PROPERTY_X, PROPERTY_Y, PROPERTY_Z, PROPERTY_MATERIAL_X, PROPERTY_MATERIAL_Y, PROPERTY_MATERIAL_Z, PROPERTY_SENSOR_FORBID, PROPERTY_SENSOR_FACE, PROPERTY_SENSOR_FIRST_COORD, PROPERTY_SENSOR_SECOND_COORD, PROPERTY_SENSOR_DATA_SOURCE, PROPERTY_HIT_RADUS };
	enum { POSITION_SENSOR_MAX_NUM = 100 };

	CCylinderPosition *mParent;

	bool mIsInit;
	
	map< DEVICE_CH_IDENTIFIER, vector< HSInt > > mDeviceChannelInfo;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);	
	CMFCPropertyGridCtrl mGridCylinder;
	afx_msg void OnStnClickedMfcpropertygridCylinder();

	LRESULT OnPropertyChanged( WPARAM wparam, LPARAM lparam );
};
