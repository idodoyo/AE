#pragma once
#include "afxpropertygridctrl.h"
#include "GraphicManager.h"
#include "DataTypes.h"
#include "DataHandler.h"

class CCylinderSurfacePosition;


// CCylinderSurfacePositionProperty dialog

class CCylinderSurfacePositionProperty : public CDialogEx, public IGraphicPropertyProtocol
{
	DECLARE_DYNAMIC(CCylinderSurfacePositionProperty)

public:
	CCylinderSurfacePositionProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCylinderSurfacePositionProperty();

// Dialog Data
	enum { IDD = IDD_CYLINDERSURFACEPOSITIONPROPERTY };

	void OnOK(){}
	void OnCancel(){}

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

	virtual CDialogEx * Wnd(){ return this; }	

public:
	enum { PROPERTY_SENSOR_COUNT = 1, PROPERTY_Y, PROPERTY_MATERIAL_Y, PROPERTY_MATERIAL_RADIUS, PROPERTY_SENSOR_FORBID, PROPERTY_SENSOR_ANGLE, PROPERTY_SENSOR_HEIGHT, PROPERTY_SENSOR_DATA_SOURCE, PROPERTY_HIT_RADUS };
	enum { POSITION_SENSOR_MAX_NUM = 100 };

	CCylinderSurfacePosition *mParent;

	bool mIsInit;
	
	map< DEVICE_CH_IDENTIFIER, vector< HSInt > > mDeviceChannelInfo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnStnClickedMfcpropertygridCylinderSurface();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	LRESULT OnPropertyChanged( WPARAM wparam, LPARAM lparam );
	CMFCPropertyGridCtrl mGridCylinderSurface;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
