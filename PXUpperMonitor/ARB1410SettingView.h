#pragma once

#include "DataTypes.h"
#include "DataHandler.h"
#include "AdPCIHandler.h"
#include "PXUpperMonitor.h"
#include "AutoAlignManager.h"
#include "afxwin.h"


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
	
	DEVICE_CH_IDENTIFIER mCardIdentifier;

	CAutoAlignManager mAutoAlignManager;

	HSBool mInitValue;

public:
	enum { IDD = IDD_ARB1410SETTINGVIEW };
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
	CEdit mBeginRate;
	CEdit mEndRate;
	CEdit mDuration;
	CEdit mCycleCount;
	CComboBox mBypassRate;
	CEdit mSignalLate;
	CEdit mMinAmplitude;
	CEdit mSignalOffset;
	CButton mAC;
	CButton mRateStep;
	CButton mTriggerContinue;
	afx_msg void OnEnKillfocusEditStartRate();
	afx_msg void OnEnKillfocusEditEndRate();
	afx_msg void OnEnKillfocusEditDuration();
	afx_msg void OnEnKillfocusEditCycleCount();
	afx_msg void OnEnKillfocusEditMinAmplitude();
	afx_msg void OnEnKillfocusEditOffset();
	afx_msg void OnEnKillfocusEditLate();
};


