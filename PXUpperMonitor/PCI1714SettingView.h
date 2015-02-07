#pragma once

#include "DataTypes.h"
#include "DataHandler.h"
#include "AdPCIHandler.h"
#include "PXUpperMonitor.h"
#include "afxpropertygridctrl.h"
#include "AutoAlignManager.h"

// CPCI1714SettingView form view

class CPCI1714SettingView : public CFormView, public IDeviceViewSetCardIDProtocol
{
	DECLARE_DYNCREATE(CPCI1714SettingView)

protected:
	CPCI1714SettingView();           // protected constructor used by dynamic creation
	virtual ~CPCI1714SettingView();

public:
	virtual void SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier );

	BOOL  DigcheckOnly( CString tStr );

private:
	enum { PROPERTY_PCI1714_RATE = 1, PROPERTY_THRESHOLD_ONE, PROPERTY_THRESHOLD_TWO, PROPERTY_THRESHOLD_THREE, PROPERTY_THRESHOLD_FOUR };	
	
	DEVICE_CH_IDENTIFIER mCardIdentifier;
	CAutoAlignManager mAutoAlignManager;

	static CString mstrRate[ 20 ];
	CString mIncorrect;

public:
	enum { IDD = IDD_PCI1714SETTINGVIEW };
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
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	afx_msg void OnBnClickedBtnOk();
	afx_msg void OnBnClickedBtnCancel();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	CMFCPropertyGridCtrl mGridProperty;

	LRESULT  OnPropertyChanged( WPARAM wparam, LPARAM lparam );

	LRESULT OnValueIncorrect( WPARAM wparam, LPARAM lparam );
};


