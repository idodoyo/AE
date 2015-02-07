#pragma once

// CPCI1714SettingDoc document
#include "PXUpperMonitor.h"

class CPCI1714SettingDoc : public CDocument, public IDocInfoProtocol
{
	DECLARE_DYNCREATE(CPCI1714SettingDoc)

public:
	CPCI1714SettingDoc();
	virtual ~CPCI1714SettingDoc();

	HSVoid SetCardIdentifier( DEVICE_CH_IDENTIFIER tCardIdentifier, CString tTitle );

public:
	virtual PX_DOC_TYPE DocType(){ return ARG_SETTING_DOC; }

private:
	DEVICE_CH_IDENTIFIER mCardIdentifier;

#ifndef _WIN32_WCE
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual BOOL OnNewDocument();

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnCloseDocument();
};
