
#pragma once

#include "DataHandle\DataTypes.h"
#include "ListCtrlEx.h"
#include "GraphicManager.h"

class CGraghicToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CGraghicWnd : public CDockablePane
{
// Construction
public:
	CGraghicWnd();

	void AdjustLayout( bool tAuto = true );

// Attributes
public:
	void SetVSDotNetLook(BOOL bSet)
	{
		mWndPropList.SetVSDotNetLook(bSet);
		mWndPropList.SetGroupNameFullWidth(bSet);
	}

	enum GRAPHIC_VIEW_TYPE{ GRAPHIC_ICON, GRAPHIC_LIST, GRAPHIC_PROPERTY };

protected:
	enum { PROPERTY_GRID_CTRL_FILE_NAME = 100,  PROPERTY_GRID_CTRL_FILE_SIZE, PROPERTY_GRID_CTRL_SPLIT_FILE, PROPERTY_GRID_CTRL_FILE_LIMIT_SIZE };

	CFont mFntPropList;
	CGraghicToolBar mFileToolBar;
	CGraghicToolBar mGraphicToolBar;
	CMFCPropertyGridCtrl mWndPropList;
	
	CListCtrlEx mGraphicIcon;
	CImageList mGraphicIconImageList;
	HSInt mIconPrevSelectItem;

	CListCtrlEx mGraphicList;
	CImageList mGraphicListImageList;
	HSInt mListPrevSelectItem;

	GRAPHIC_VIEW_TYPE mGraphicListType;	
	map< string, IGraphicPropertyProtocol * > mGraphicProperties;

	vector< HSString > mGrahpicIdentifiers;

	CString mDefaultFilePath;

	IGraphicProtocol *mFocusedGrahpic;

public:
	CString FilePath();	
	HSVoid UpdateFileSize( HSDouble tSize );
	HSVoid SetFilePath( CString tFilePath );
	HSVoid EnableFileInfo( HSBool tEnable );

	static string GetStrSize( double tSize );
		
	HSInt64 GetDisperseFileLimit();
	HSVoid SetDisperseFileLimit( HSInt64 tSize );

	HSVoid SetGrahpicListType( GRAPHIC_VIEW_TYPE tGraphicListType );
	HSVoid SetFocusedGraphic( IGraphicProtocol *pFocusedGrahpic, HSBool tForceRefresh = HSFalse );
	HSVoid RefreshPropertyDataSource();

	HSVoid SetDefaultFilePath( CString tFilePath ){ mDefaultFilePath = tFilePath; }

private:
	HSVoid HidePropertyWndExcept( CWnd * pWnd = NULL );

// Implementation
public:
	virtual ~CGraghicWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);	
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSortProperties();
	afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);	
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	DECLARE_MESSAGE_MAP()

	void InitPropList();
	void SetPropListFont();
public:
	afx_msg void OnListIcon();
	afx_msg void OnUpdateListIcon(CCmdUI *pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnIconProperties();
	afx_msg void OnUpdateIconProperties(CCmdUI *pCmdUI);
	afx_msg void OnListIconlist();
	afx_msg void OnUpdateListIconlist(CCmdUI *pCmdUI);

	LRESULT OnPropertyChanged( WPARAM wparam, LPARAM lparam );
};

