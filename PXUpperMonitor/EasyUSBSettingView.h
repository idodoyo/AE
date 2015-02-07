#pragma once

#include "DataTypes.h"
#include "DataHandler.h"
#include "AdPCIHandler.h"
#include "PXUpperMonitor.h"
#include "AutoAlignManager.h"
#include "afxpropertygridctrl.h"


// CEasyUSBSettingView form view

class CEasyUSBSettingView : public CFormView, public IDeviceViewSetCardIDProtocol
{
	DECLARE_DYNCREATE(CEasyUSBSettingView)

protected:
	CEasyUSBSettingView();           // protected constructor used by dynamic creation
	virtual ~CEasyUSBSettingView();

public:
	virtual void SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier );

	BOOL DigcheckOnly( CString tStr );

private:	
	enum { PROPERTY_EZUSB_RATEONE = 1, PROPERTY_EZUSB_RATETWO, PROPERTY_THRESHOLD_ONE, PROPERTY_THRESHOLD_TWO };
	CString mIncorrect;
	static CString mStrRate[ 5 ];
	DEVICE_CH_IDENTIFIER mCardIdentifier;

	CAutoAlignManager mAutoAlignManager;

public:
	enum { IDD = IDD_EASYUSBSETTINGVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnClickedBtnOk();
	afx_msg void OnClickedBtnCancel();
	CMFCPropertyGridCtrl mGridPropterty;

	LRESULT  OnPropertyChanged( WPARAM wparam, LPARAM lparam );

	LRESULT  OnValueIncorrect( WPARAM wparam, LPARAM lparam );
};


