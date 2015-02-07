#pragma once
#include "afxpropertygridctrl.h"

#include "GraphicManager.h"
#include "DataTypes.h"
#include "DataHandler.h"

// CNormalProperty dialog

class CNormalProperty : public CDialogEx, public IGraphicPropertyProtocol
{
	DECLARE_DYNAMIC(CNormalProperty)

public:
	CNormalProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNormalProperty();

// Dialog Data
	enum { IDD = IDD_NORMALPROPERTY };

	void OnOK(){}
	void OnCancel(){}

public:	
	virtual CDialogEx * Wnd(){ return this; }
	virtual void SetParent( IGraphicProtocol *pGraphic );

	HSVoid RefreshDeviceChannel();
	HSVoid SetDataSourceForItem( CMFCPropertyGridProperty* pProp, HSVoid *pInfo = NULL );

public:
	virtual HSBool IsDataHandlerMachedWith( IDataHandler *pDataHandler, HSUInt tChannel ){ return HSFalse; }
	virtual HSBool IsDataHandlerMachedWith( IDataHandler *pDataHandler, HSUInt tChannel, HSVoid *pInfo ){ return HSFalse; }
	virtual HSBool IsDataHandlerMachedWith( DEVICE_CH_IDENTIFIER tChannelIdentifier, HSVoid *pSensorInfo ){ return HSFalse; }	

	virtual HSVoid InitDialog(){}
	virtual HSVoid PropertyChanged( CMFCPropertyGridProperty * pProperty ){}
	virtual HSVoid SensorClicked( HSInt tIndex ){}	
	

protected:	
	bool mIsInit;

	map< DEVICE_CH_IDENTIFIER, vector< HSInt > > mDeviceChannelInfo;

	IGraphicProtocol *mGraphic;

	HSBool mTotalChannelEnabled;	
	HSBool mForPosition;
	HSBool mMultiDataSourceConditionEnabled;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CMFCPropertyGridCtrl mPropertyGrid;
	afx_msg void OnStnClickedMfcpropertygridProp();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();

	LRESULT OnPropertyChanged( WPARAM wparam, LPARAM lparam );
};
