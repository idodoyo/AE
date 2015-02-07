#pragma once

#include "DataTypes.h"
#include "DataHandler.h"
#include "AdPCIHandler.h"
#include "PXUpperMonitor.h"
#include "AutoAlignManager.h"
#include "afxpropertygridctrl.h"


// CARB1410SettingView form view

class CARB1410SettingView : public CFormView, public IDeviceViewSetCardIDProtocol
{
	DECLARE_DYNCREATE(CARB1410SettingView)

protected:
	CARB1410SettingView();           // protected constructor used by dynamic creation
	virtual ~CARB1410SettingView();

public:
	virtual void SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier );
	
private:
	enum { PROPERTY_ARB1410_RATE = 1 , PROPERTY_ARB1410_CARRIERTYPE, PROPERTY_ARB1410_ENVELOPE ,PROPERTY_ARB1410_DURATIONUNITS, PROPERTY_ARB1410_DURATION ,PROPERTY_ARB1410_STARTFREQUENCY, PROPERTY_ARB1410_ENDFREQUENCY , PROPERTY_ARB1410_FREDURATION, PROPERTY_ARB1410_PRECYCLE,
			PROPERTY_ARB1410_AEFREQUENCY,PROPERTY_ARB1410_RISERATE, PROPERTY_ARB1410_FALLRATE, PROPERTY_ARB1410_COUPLING, PROPERTY_ARB1410_AMPLITUDE, PROPERTY_ARB1410_DCOFFSET, PROPERTY_ARB1410_FILTER, PROPERTY_ARB1410_DELAY, PROPERTY_ARB1410_MODE, PROPERTY_ARB1410_RERATE, PROPERTY_ARB1410_OUTPOLARITY,
			PROPERTY_ARB1410_OUTWIDTH, PROPERTY_ARB1410_INTYPE, PROPERTY_ARB1410_INSLOPE, PROPERTY_ARB1410_WAVEFROM_TYPE };

	enum { IDD = IDD_ARB1410SETTINGVIEW }; 
	CString mIncorrectMsg;

	static CString mStartFrequency[ 4 ];
	static CString mStrEndFrequency[ 6 ];
	static CString mStrRate[ 9 ];
	static CString mStrreRate[ 9 ];
	static CString mStrRiseRate[ 9 ];

	DEVICE_CH_IDENTIFIER mCardIdentifier;

	CAutoAlignManager mAutoAlignManager;

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
	afx_msg void OnClickedBtnOk();
	afx_msg void OnClickedBtnCancel();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	BOOL DigcheckOnly( CString tStr );

	CMFCPropertyGridCtrl mPropertyGrid;

	LRESULT  OnPropertyChanged( WPARAM wparam, LPARAM lparam );

	LRESULT OnValueIncorrect( WPARAM wparam, LPARAM lparam );
};


