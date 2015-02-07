
// PXUpperMonitor.h : main header file for the PXUpperMonitor application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include "DataTypes.h"
#include "DataHandler.h"
#include "DLLLoader.h"
#include "PXLicenseProtocol.h"
#include "WaveGenProtocol.h"


// CPXUpperMonitorApp:
// See PXUpperMonitor.cpp for the implementation of this class
//

class IDeviceViewSetCardIDProtocol
{
public:
	virtual void SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier ) = 0;

	virtual ~IDeviceViewSetCardIDProtocol(){}
};

enum PX_DOC_TYPE{ GRAPHIC_DOC, ARG_SETTING_DOC };

class IDocInfoProtocol
{
public:
	virtual PX_DOC_TYPE DocType() = 0;

	virtual ~IDocInfoProtocol(){}
};

class CPXUpperMonitorApp : public CWinAppEx
{
public:
	CPXUpperMonitorApp();

public:
	HSVoid OpenDeviceSettingDlg( DEVICE_CH_IDENTIFIER tCardIdentifier );
	HSVoid DeviceSettingDlgClosed( DEVICE_CH_IDENTIFIER tCardIdentifier );
//	HSVoid RefreshDataHandler( HSUInt tCardIdentifier, IDataHandler *pDataHandler );

	HSBool InitLog();

	string PropgramFilePath(){ return mPropgramFilePath; }

	string GetLocalTime( HSInt64 tTimeNeed = -1 );
	string GetLocalDate();

	IPXLicenseProtocol * License(){ return mLicense; }
	HSBool DataExportEnabled(){ return mDataExportEnabled; }
	HSInt DataFileIndex(){ return mDataFileIndex; }
	HSVoid DataFileAddIndex(){ mDataFileIndex++; }
	HSVoid DataFileResetIndex(){ mDataFileIndex = 0; }

	HSInt64 StartTime( HSInt64 tStartTime = -1 );

private:
	CMultiDocTemplate *mNormalDocTemplate;

	bool mInit;

	map< HSUInt, CMultiDocTemplate * > mDeviceSettinsDocTemplate;
	map< DEVICE_CH_IDENTIFIER, CDocument * > mDeviceSettins;

	string mPropgramFilePath;

	IPXLicenseProtocol *mLicense;

	HSBool mDataExportEnabled;
	HSInt mDataFileIndex;	

	HSInt64 mStartTime;

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnLayoutNew();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CPXUpperMonitorApp theApp;
