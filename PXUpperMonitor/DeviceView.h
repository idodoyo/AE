
#pragma once

#include "ViewTree.h"
#include <vector>
#include "DataTypes.h"

using std::vector;

#define ID_DEVICE_TREE	60000

class CDeviceViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CDeviceView : public CDockablePane
{
// Construction
public:
	CDeviceView();

	void AdjustLayout();
	void OnChangeVisualStyle();

	HSVoid EnableChooseDevice( HSBool tEnable );

// Attributes
protected:
	CViewTree mWndDeviceView;
	CImageList mDeviceViewImages;
	CDeviceViewToolBar mWndToolBar;

	HSBool mIsChooseDeviceEnable;

public:
	vector< HSInt > OpenChannelsWithDevice( HSInt tIndex );

public:
	void FillDeviceView( HSBool tScanNewDevices = HSTrue, vector< vector< HSInt > > *pDevicesOpenChannels = NULL );
	void SetChildCheck( HTREEITEM tNode, BOOL tCheck );
	void CheckSelectChannels( HTREEITEM tRoot, vector< vector< HSInt > > *pDevicesOpenChannels );

// Implementation
public:
	virtual ~CDeviceView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRefreshDevices();	
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnClickDevices(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnViewDevices();

	DECLARE_MESSAGE_MAP()
};

