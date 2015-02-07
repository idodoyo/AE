#pragma once

#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"

class CArgTable;

// CArgProperty dialog

class CArgProperty : public CDialogEx, public IGraphicPropertyProtocol
{
	DECLARE_DYNAMIC(CArgProperty)

public:
	CArgProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CArgProperty();

// Dialog Data
	enum { IDD = IDD_ARGPROPERTY };

	void OnOK(){}
	void OnCancel(){}

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

	virtual CDialogEx * Wnd(){ return this; }		

private:
	enum { PROPERTY_BYPASS_VOLTAGE = 1, PROPERTY_DATA_SOURCE };

	CArgTable *mParent;
	bool mIsInit;

	map< DEVICE_CH_IDENTIFIER, vector< HSInt > > mDeviceChannelInfo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CMFCPropertyGridCtrl mArgProperty;
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	LRESULT OnPropertyChanged( WPARAM wparam, LPARAM lparam );
};
