// PCI1714SettingDoc.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "PCI1714SettingDoc.h"
#include "DeviceTypes.h"
#include "MainFrm.h"


// CPCI1714SettingDoc

IMPLEMENT_DYNCREATE(CPCI1714SettingDoc, CDocument)

CPCI1714SettingDoc::CPCI1714SettingDoc()
{
}

BOOL CPCI1714SettingDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	this->SetTitle( "PXDAQ12204 ÉèÖÃ" );

	return TRUE;
}

CPCI1714SettingDoc::~CPCI1714SettingDoc()
{
}

HSVoid CPCI1714SettingDoc::SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier, CString tTitle )
{
	mCardIdentifier = tCardIdentifier;
	
	this->SetTitle( tTitle );
}


BEGIN_MESSAGE_MAP(CPCI1714SettingDoc, CDocument)
END_MESSAGE_MAP()


// CPCI1714SettingDoc diagnostics

#ifdef _DEBUG
void CPCI1714SettingDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CPCI1714SettingDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CPCI1714SettingDoc serialization

void CPCI1714SettingDoc::Serialize(CArchive& ar)
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


// CPCI1714SettingDoc commands


void CPCI1714SettingDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class
	theApp.DeviceSettingDlgClosed( mCardIdentifier );

	CDocument::OnCloseDocument();
}
