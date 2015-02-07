#pragma once

#include "DataTypes.h"
#include "DataHandler.h"
#include "UA306Handler.h"
#include "PXUpperMonitor.h"
#include "afxpropertygridctrl.h"
#include "AutoAlignManager.h"

// CUA306SettingView form view

class CUA306SettingView : public CFormView, public IDeviceViewSetCardIDProtocol
{
	DECLARE_DYNCREATE(CUA306SettingView)

protected:
	CUA306SettingView();           // protected constructor used by dynamic creation
	virtual ~CUA306SettingView();

public:
	virtual void SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier );

private:
	enum { PROPERTY_RATE = 1, PROPERTY_THRESHOLD };	
	
	DEVICE_CH_IDENTIFIER mCardIdentifier;
	CAutoAlignManager mAutoAlignManager;
	
	CString mIncorrect;

public:
	enum { IDD = IDD_UA306SETTINGVIEW };
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
	CMFCPropertyGridCtrl mPropertyGrid;
	afx_msg void OnClickedBtnOk();
	afx_msg void OnClickedBtnCancel();
	virtual void OnInitialUpdate();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	LRESULT  OnPropertyChanged( WPARAM wparam, LPARAM lparam );

	LRESULT OnValueIncorrect( WPARAM wparam, LPARAM lparam );
};


