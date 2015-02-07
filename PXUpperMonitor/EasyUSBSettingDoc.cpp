// EasyUSBSettingDoc.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "EasyUSBSettingDoc.h"


// CEasyUSBSettingDoc

IMPLEMENT_DYNCREATE(CEasyUSBSettingDoc, CDocument)

CEasyUSBSettingDoc::CEasyUSBSettingDoc()
{
}

BOOL CEasyUSBSettingDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	this->SetTitle( "PXUAE1005 ÉèÖÃ" );

	return TRUE;
}

CEasyUSBSettingDoc::~CEasyUSBSettingDoc()
{
}

HSVoid CEasyUSBSettingDoc::SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier, CString tTitle )
{
	mCardIdentifier = tCardIdentifier;
	
	this->SetTitle( tTitle );
}

BEGIN_MESSAGE_MAP(CEasyUSBSettingDoc, CDocument)
END_MESSAGE_MAP()


// CEasyUSBSettingDoc diagnostics

#ifdef _DEBUG
void CEasyUSBSettingDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CEasyUSBSettingDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CEasyUSBSettingDoc serialization

void CEasyUSBSettingDoc::Serialize(CArchive& ar)
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


// CEasyUSBSettingDoc commands


void CEasyUSBSettingDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class

	theApp.DeviceSettingDlgClosed( mCardIdentifier );	

	__super::OnCloseDocument();
}
