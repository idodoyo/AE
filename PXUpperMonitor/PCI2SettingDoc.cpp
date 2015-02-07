// PCI2SettingDoc.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "PCI2SettingDoc.h"


// CPCI2SettingDoc

IMPLEMENT_DYNCREATE(CPCI2SettingDoc, CDocument)

CPCI2SettingDoc::CPCI2SettingDoc()
{
}

BOOL CPCI2SettingDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	this->SetTitle( "PAPCI2 ÉèÖÃ" );

	return TRUE;
}

CPCI2SettingDoc::~CPCI2SettingDoc()
{
}

HSVoid CPCI2SettingDoc::SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier, CString tTitle )
{
	mCardIdentifier = tCardIdentifier;
	
	this->SetTitle( tTitle );
}


BEGIN_MESSAGE_MAP(CPCI2SettingDoc, CDocument)
END_MESSAGE_MAP()


// CPCI2SettingDoc diagnostics

#ifdef _DEBUG
void CPCI2SettingDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CPCI2SettingDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CPCI2SettingDoc serialization

void CPCI2SettingDoc::Serialize(CArchive& ar)
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


// CPCI2SettingDoc commands


void CPCI2SettingDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class

	theApp.DeviceSettingDlgClosed( mCardIdentifier );

	__super::OnCloseDocument();
}
