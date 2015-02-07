
// PXUpperMonitorDoc.cpp : implementation of the CPXUpperMonitorDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "PXUpperMonitor.h"
#endif

#include "PXUpperMonitorDoc.h"
#include "PXUpperMonitorView.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CPXUpperMonitorDoc

IMPLEMENT_DYNCREATE(CPXUpperMonitorDoc, CDocument)

BEGIN_MESSAGE_MAP(CPXUpperMonitorDoc, CDocument)
END_MESSAGE_MAP()


// CPXUpperMonitorDoc construction/destruction

CPXUpperMonitorDoc::CPXUpperMonitorDoc()
{
	// TODO: add one-time construction code here
	mLayout = "";

}

CPXUpperMonitorDoc::~CPXUpperMonitorDoc()
{
}

BOOL CPXUpperMonitorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	SetTitle( "ÐÂÒ³Ãæ" );

	return TRUE;
}




// CPXUpperMonitorDoc serialization

void CPXUpperMonitorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CPXUpperMonitorDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CPXUpperMonitorDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CPXUpperMonitorDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CPXUpperMonitorDoc diagnostics

#ifdef _DEBUG
void CPXUpperMonitorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPXUpperMonitorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CPXUpperMonitorDoc commands
void CPXUpperMonitorDoc::StartView()
{
	POSITION tPos = GetFirstViewPosition();
	CPXUpperMonitorView *pView = ( CPXUpperMonitorView * )GetNextView( tPos );
	pView->Start();
}

void CPXUpperMonitorDoc::StopView()
{
	POSITION tPos = GetFirstViewPosition();
	CPXUpperMonitorView *pView = ( CPXUpperMonitorView * )GetNextView( tPos );
	pView->Stop();
}

void CPXUpperMonitorDoc::PauseView()
{
	POSITION tPos = GetFirstViewPosition();
	CPXUpperMonitorView *pView = ( CPXUpperMonitorView * )GetNextView( tPos );
	pView->Pause();
}

void CPXUpperMonitorDoc::Reset()
{
	POSITION tPos = GetFirstViewPosition();
	CPXUpperMonitorView *pView = ( CPXUpperMonitorView * )GetNextView( tPos );
	pView->Reset();
}

IGraphicProtocol * CPXUpperMonitorDoc::OpenLayout( CString tLayout )
{
	mLayout = tLayout;

	POSITION tPos = GetFirstViewPosition();
	CPXUpperMonitorView *pView = ( CPXUpperMonitorView * )GetNextView( tPos );
	return pView->LoadLayout( tLayout );	
}

void CPXUpperMonitorDoc::SaveLayout()
{
	if ( mLayout != "" )
	{
		POSITION tPos = GetFirstViewPosition();
		CPXUpperMonitorView *pView = ( CPXUpperMonitorView * )GetNextView( tPos );
		if ( mLayout.GetLength() > 2 && mLayout[ 1 ] != ':' )
		{
			CString tLayout = theApp.PropgramFilePath().c_str();
			tLayout += "\\";
			mLayout = tLayout + mLayout;
		}

		pView->SaveLayout( mLayout, "" );
	}	
}