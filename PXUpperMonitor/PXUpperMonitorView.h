
// PXUpperMonitorView.h : interface of the CPXUpperMonitorView class
//

#pragma once

#include <afxcview.h>
#include "DataHandle\DataTypes.h"
#include "GraphicManager.h"
#include "DataHandler.h"


#include <list>
#include <string>

using std::list;
using std::string;

class CPXUpperMonitorView : public CView, public IGraphicWindowProtocol
{
protected: // create from serialization only
	CPXUpperMonitorView();
	DECLARE_DYNCREATE(CPXUpperMonitorView)

// Attributes
public:
	CPXUpperMonitorDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

public:
	virtual void BeginMoveSubView( CWnd *pWnd, UINT tFlags, CPoint tPoint );
	virtual void MoveingSubView( UINT tFlags, CPoint tPoint );
	virtual void EndMoveSubView( UINT tFlags, CPoint tPoint );
	virtual void SubViewClosed( CWnd *pWnd );
	virtual void SubViewFullScreen( CWnd *pWnd, HSBool tIsFullScreen );
	virtual IGraphicProtocol * GraphicWithIdentifier( HSString tIdentifier );

public:
	void Start();
	void Pause();
	void Stop();
	void Reset();
	IGraphicProtocol *  LoadLayout( CString tLayout );
	bool SaveLayout( CString tLayout, CString tTitle );

private:
	HSBool mIsConstructNewGraphic;
	CPoint mBeginDrawPoint;
	CRect mGraphicRect;
	HSBool mFirstDraw;

	HSBool mTracking;

	typedef struct GraphicDetail
	{
		IGraphicProtocol *Graphic;
		HSString Identifier;
		DEVICE_CH_IDENTIFIER  DataIdentifier;
		HSFloat LeftPercent;
		HSFloat TopPercent;
		HSFloat RightPercent;
		HSFloat BottomPercent;
	} GraphicDetail;

	list< GraphicDetail > mGraphics;

	CWnd *mMoveingWnd;
	CPoint mBeginMoveWndPoint;
	CRect mMoveingRect;

//	HSBool mIsStartShowAllView;

private:
	void ArrangeGrapics( HSInt tColumnNum );
	void CalcGraphicPos();
	void ClearGraphics( HSBool tUnfocusWnd = HSFalse );
	void ResizeGraphics();	

// Implementation
public:
	virtual ~CPXUpperMonitorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileLoadlayout();
	afx_msg void OnFileSavelayout();	
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnAutoArraneMulCol();
	afx_msg void OnAutoArraneOneCol();
	afx_msg void OnAutoArraneTueCol();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseLeave();
	afx_msg void OnDestroy();
};

#ifndef _DEBUG  // debug version in PXUpperMonitorView.cpp
inline CPXUpperMonitorDoc* CPXUpperMonitorView::GetDocument() const
   { return reinterpret_cast<CPXUpperMonitorDoc*>(m_pDocument); }
#endif

