#pragma once

#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"

class CGraphicFFT;

// CGraphicFFTProperty dialog

class CGraphicFFTProperty : public CDialogEx, public IGraphicPropertyProtocol
{
	DECLARE_DYNAMIC(CGraphicFFTProperty)

public:
	CGraphicFFTProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphicFFTProperty();

// Dialog Data
	enum { IDD = IDD_GRAPHICFFTPROPERTY };

	void OnOK(){}
	void OnCancel(){}

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

	virtual CDialogEx * Wnd(){ return this; }	

private:
	enum { PROPERTY_AMPLITUDE = 1, PROPERTY_BEGIN_AMPLITUDE, PROPERTY_POINT_LENGTH, PROPERTY_DATA_SOURCE };

	CGraphicFFT *mParent;

	bool mIsInit;

	map< DEVICE_CH_IDENTIFIER, vector< HSInt > > mDeviceChannelInfo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CMFCPropertyGridCtrl mGridFFT;

	LRESULT OnPropertyChanged( WPARAM wparam, LPARAM lparam );
};
