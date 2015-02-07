#pragma once

#include "DataTypes.h"
#include "IndexManager.h"
#include "DeviceManager.h"
//#include "LinearTransfer.h"
#include "ThreadControlEx.h"

#include <fstream>

using std::ofstream;

const HSInt SMALL_TXT_FILE_SIZE = 1 * 1024 * 1024;

// CDataExportToTXT dialog

class CDataExportToTXT : public CDialogEx
{
	DECLARE_DYNAMIC(CDataExportToTXT)

public:
	CDataExportToTXT(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDataExportToTXT();

// Dialog Data
	enum { IDD = IDD_DATAEXPORTTOTXT };

public:
	HSVoid SetDeviceInfo( CDeviceManager *pDeviceManager, CIndexManager *pIndexManager, HSString tDataFilePath );
	HSBool ThreadRuning( HSInt tThreadID );
	HSVoid ThreadWillStop( HSInt tThreadID );

private:
	HSVoid DataHandlerWithListIndex( HSInt tIndex, IDataHandler * &pHandler, DEVICE_CH_IDENTIFIER &tChannelIdentifier );
	HSVoid ClearDataExportInfo();
	HSVoid UpdateHandleProgress();
	
private:
	CDeviceManager *mDeviceManager;
	CIndexManager *mIndexManager;

	CImageList mImageList;	

	map< DEVICE_CH_IDENTIFIER, vector< HSUInt > > mDeviceChannelInfo;

	HSBool mIsStart;

	typedef struct DATA_EXPORT_INFO
	{
		vector< CLinearTransfer * > LinearTransfer;
		vector< DEVICE_CH_IDENTIFIER > ChannelIdentifer;
		vector< HSChar * > Buffer;
		vector< HSInt > ReadIndex;
		vector< HSBool > GotData;
		ofstream *SaveStream;
		CString FileFormat;
		HSInt Index;				
		IDataHandler *DataHandler;
	} DATA_EXPORT_INFO;

	vector< DATA_EXPORT_INFO > mDataExportInfo;
	HSUInt mDataExportIndex;

	HSBool mSplitToSmallFile;

	HSString mDataFilePath;

	CThreadControlEx< CDataExportToTXT > mExportThread;

	HSInt64 mTotalDataSize;
	HSInt64 mHandledDataSize;
	vector< HSInt64 > mItemDataSize;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnChooseDir();
};
