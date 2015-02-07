#pragma once

#include "DataTypes.h"
#include "DataHandler.h"
#include "AdPCIHandler.h"
#include "PXUpperMonitor.h"
#include "AutoAlignManager.h"
#include "afxpropertygridctrl.h"

// CPCI2SettingView form view
class CPCI2SettingView : public CFormView, public IDeviceViewSetCardIDProtocol
{
	DECLARE_DYNCREATE(CPCI2SettingView)

protected:
	CPCI2SettingView();           // protected constructor used by dynamic creation
	virtual ~CPCI2SettingView();

public:
	virtual void SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier );

private:
	HSBool DigitOnlyCheck( CString &tValue );

private:	
	
	DEVICE_CH_IDENTIFIER mCardIdentifier;

	CAutoAlignManager mAutoAlignManager;

	enum { PROPERTY_PCI_THRESHOLD_TYPE = 1, PROPERTY_PCI_THRESHOLD_VALUE, PROPERTY_PCI_THRESHOLD_FLOADBAND, PROPERTY_PCI_INPUT_GAIN, PROPERTY_PCI_MAXDURA, PROPERTY_PCI_PDT, PROPERTY_PCI_HDT, PROPERTY_PCI_HLT, PROPERTY_PCI_RATE,
			PROPERTY_PCI_PRETRIGGER, PROPERTY_PCI_LOW_FILTER, PROPERTY_PCI_HIGH_FILTER, PROPERTY_PCI_AMP_GAIN, PROPERTY_PCI_WAVEFROM_LENGTH, PROPERTY_PCI_DRIVEN_RATE, PROPERTY_PCI_RMS_ASL_TIME,PROPERTY_PCI_CYCLE_COUNTER };
		
	static CString mStrLowFilter[ 6 ];
	static CString mStrHighFilter[ 4 ];
	static CString mStrRate[ 9 ];
	CString mIncorrectMsg;

public:
	enum { IDD = IDD_PCI2SETTINGVIEW };
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
	afx_msg void OnBnClickedBtnOk();
	afx_msg void OnBnClickedBtnCancel();
	CMFCPropertyGridCtrl mGridProperty;

	LRESULT OnPropertyChanged( WPARAM wparam, LPARAM lparam );

	LRESULT OnValueIncorrect( WPARAM wparam, LPARAM lparam );
};


