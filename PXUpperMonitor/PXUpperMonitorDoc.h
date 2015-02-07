
// PXUpperMonitorDoc.h : interface of the CPXUpperMonitorDoc class
//

#pragma once

#include "PXUpperMonitor.h"
#include "GraphicManager.h"

class CPXUpperMonitorDoc : public CDocument, public IDocInfoProtocol
{
protected: // create from serialization only
	CPXUpperMonitorDoc();
	DECLARE_DYNCREATE(CPXUpperMonitorDoc)

// Attributes
public:
	virtual PX_DOC_TYPE DocType(){ return GRAPHIC_DOC; }

// Operations
public:
	void StartView();
	void StopView();
	void PauseView();
	void Reset();

	IGraphicProtocol * OpenLayout( CString tLayout );
	void SetLayout( CString tLayout ){ mLayout = tLayout; }
	void SaveLayout();
	CString GetLayout(){ return mLayout; }

private:
	CString mLayout;

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CPXUpperMonitorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
