// ARB1410SettingDoc.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "ARB1410SettingDoc.h"


// CARB1410SettingDoc

IMPLEMENT_DYNCREATE(CARB1410SettingDoc, CDocument)

CARB1410SettingDoc::CARB1410SettingDoc()
{
}

BOOL CARB1410SettingDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	this->SetTitle( "ARB1410 ÉèÖÃ" );

	return TRUE;
}

CARB1410SettingDoc::~CARB1410SettingDoc()
{
}

HSVoid CARB1410SettingDoc::SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier, CString tTitle )
{
	mCardIdentifier = tCardIdentifier;
	
	this->SetTitle( tTitle );
}


BEGIN_MESSAGE_MAP(CARB1410SettingDoc, CDocument)
END_MESSAGE_MAP()


// CARB1410SettingDoc diagnostics

#ifdef _DEBUG
void CARB1410SettingDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CARB1410SettingDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CARB1410SettingDoc serialization

void CARB1410SettingDoc::Serialize(CArchive& ar)
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


// CARB1410SettingDoc commands


void CARB1410SettingDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class

	theApp.DeviceSettingDlgClosed( mCardIdentifier );

	__super::OnCloseDocument();
}
