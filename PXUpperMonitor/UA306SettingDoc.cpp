// UA306SettingDoc.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "UA306SettingDoc.h"
#include "DeviceTypes.h"
#include "MainFrm.h"


// CUA306SettingDoc

IMPLEMENT_DYNCREATE(CUA306SettingDoc, CDocument)

CUA306SettingDoc::CUA306SettingDoc()
{
}

BOOL CUA306SettingDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	this->SetTitle( "UA306 ÉèÖÃ" );

	return TRUE;
}

CUA306SettingDoc::~CUA306SettingDoc()
{
}

HSVoid CUA306SettingDoc::SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier, CString tTitle )
{
	mCardIdentifier = tCardIdentifier;
	
	this->SetTitle( tTitle );
}


BEGIN_MESSAGE_MAP(CUA306SettingDoc, CDocument)
END_MESSAGE_MAP()


// CUA306SettingDoc diagnostics

#ifdef _DEBUG
void CUA306SettingDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CUA306SettingDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CUA306SettingDoc serialization

void CUA306SettingDoc::Serialize(CArchive& ar)
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
#endif


// CUA306SettingDoc commands


void CUA306SettingDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class

	theApp.DeviceSettingDlgClosed( mCardIdentifier );

	__super::OnCloseDocument();
}
