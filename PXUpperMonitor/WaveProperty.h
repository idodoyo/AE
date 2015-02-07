#pragma once

#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"

class CGraphicWave;

// CWaveProperty dialog

class CWaveProperty : public CDialogEx, public IGraphicPropertyProtocol
{
	DECLARE_DYNAMIC(CWaveProperty)

public:
	CWaveProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWaveProperty();

// Dialog Data
	enum { IDD = IDD_WAVEPROPERTY };

	void OnOK(){}
	void OnCancel(){}

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

	virtual CDialogEx * Wnd(){ return this; }	

private:
	enum { PROPERTY_VOLTAGE = 1, PROPERTY_TIME, PROPERTY_DATA_SOURCE, PROPERTY_BYPASS_VOLTAGE };

	CGraphicWave *mParent;

	bool mIsInit;

	map< DEVICE_CH_IDENTIFIER, vector< HSInt > > mDeviceChannelInfo;

	static int mVoltages[ 10 ];
	static int mUSeconds[ 6 ];
	static CString mStrVoltages[ 10 ];
	static CString mStrTimes[ 6 ];
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	

	DECLARE_MESSAGE_MAP()	
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();	
	CMFCPropertyGridCtrl mPropertyGrid;	

	LRESULT OnPropertyChanged( WPARAM wparam, LPARAM lparam );
};
