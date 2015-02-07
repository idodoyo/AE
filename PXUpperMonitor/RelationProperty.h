#pragma once


#include "GraphicManager.h"
#include "afxpropertygridctrl.h"
#include "DataTypes.h"
#include "DataHandler.h"

class CGraphicRelation;

// CRelationProperty dialog

class CRelationProperty : public CDialogEx, public IGraphicPropertyProtocol
{
	DECLARE_DYNAMIC(CRelationProperty)

public:
	CRelationProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRelationProperty();

// Dialog Data
	enum { IDD = IDD_RELATIONPROPERTY };

	void OnOK(){}
	void OnCancel(){}

public:
	virtual void SetParent( IGraphicProtocol *pGraphic );	
	virtual void RefreshDataSource();

	virtual CDialogEx * Wnd(){ return this; }	

private:
	enum { PROPERTY_X = 1, PROPERTY_Y, PROPERTY_FILL_RECT, PROPERTY_SQUARE_LENGTH, PROPERTY_DATA_SOURCE };

	CGraphicRelation *mParent;

	bool mIsInit;

	map< DEVICE_CH_IDENTIFIER, vector< HSInt > > mDeviceChannelInfo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CMFCPropertyGridCtrl mGridRelation;
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	LRESULT OnPropertyChanged( WPARAM wparam, LPARAM lparam );
};
