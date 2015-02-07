#pragma once
#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"

class CSpherePosition;


// CSphereProperty dialog

class CSphereProperty : public CDialogEx, public IGraphicPropertyProtocol
{
	DECLARE_DYNAMIC(CSphereProperty)

public:
	CSphereProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSphereProperty();

// Dialog Data
	enum { IDD = IDD_SPHEREPROPERTY };

	void OnOK(){}
	void OnCancel(){}

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

	virtual CDialogEx * Wnd(){ return this; }	

public:
	enum { PROPERTY_SENSOR_COUNT = 1, PROPERTY_RADIUS, PROPERTY_SENSOR_FORBID, PROPERTY_SENSOR_ANGLE_D, PROPERTY_SENSOR_ANGLE_U, PROPERTY_SENSOR_DATA_SOURCE, PROPERTY_HIT_RADUS };
	enum { POSITION_SENSOR_MAX_NUM = 100 };

	CSpherePosition *mParent;

	bool mIsInit;
	
	map< DEVICE_CH_IDENTIFIER, vector< HSInt > > mDeviceChannelInfo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CMFCPropertyGridCtrl mPropertyGrid;
	afx_msg void OnStnClickedMfcpropertygridSphere();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();

	LRESULT OnPropertyChanged( WPARAM wparam, LPARAM lparam );
};
