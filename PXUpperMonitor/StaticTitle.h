#pragma once

#include "DataHandler.h"

// CStaticGraphic

class CStaticTitle : public CStatic
{
	DECLARE_DYNAMIC(CStaticTitle)

public:
	CStaticTitle();
	virtual ~CStaticTitle();

public:
	void SetTitle( CString tTitle );	
	int TopDelta(){ return mTopDelta; }
	int LeftDelta(){ return mLeftDelta; }
	void SetFocused( bool tFocused ){ mFocused = tFocused; }
	bool Focused(){ return mFocused; }
	void ResizeWave();

protected:	
	virtual void Draw( CDC *pDC ) = 0;
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );		
	void DrawFrame( CDC *pMemDC, CRect &tRect );

protected:
	bool mFocused;

	CString mTitle;
	CString mType;

	HANDLE mMutex;
	CDC *mTemplateDC;	

	int mTopDelta;
	int mLeftDelta;	

	CFont mTitleFont;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();	
};


