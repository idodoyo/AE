#pragma once

#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"

class CGraphicScatter;

// CScatterProperty dialog

class CScatterProperty : public CDialogEx, public IGraphicPropertyProtocol
{
	DECLARE_DYNAMIC(CScatterProperty)

public:
	CScatterProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CScatterProperty();

// Dialog Data
	enum { IDD = IDD_SCATTERPROPERTY };

	void OnOK(){}
	void OnCancel(){}

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

	virtual CDialogEx * Wnd(){ return this; }	

private:
	enum { PROPERTY_X = 1, PROPERTY_Y, PROPERTY_FILL_RECT, PROPERTY_SQUARE_LENGTH, PROPERTY_DATA_SOURCE };

	CGraphicScatter *mParent;

	bool mIsInit;

	map< DEVICE_CH_IDENTIFIER, vector< HSInt > > mDeviceChannelInfo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CMFCPropertyGridCtrl mPropertyScatter;
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	LRESULT OnPropertyChanged( WPARAM wparam, LPARAM lparam );
};
