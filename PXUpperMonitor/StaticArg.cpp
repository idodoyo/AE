#include "StdAfx.h"
#include "StaticArg.h"


CStaticArg::CStaticArg()
{
	this->mTitle = "DEVICE < CHANNEL >";
	this->mTopDelta = 24;
	this->mLeftDelta = 0;

	mType = "[²ÎÊý±í] ";
}


CStaticArg::~CStaticArg()
{	
}

void CStaticArg::Draw( CDC *pDC )
{
	CRect tRect;
	this->GetClientRect( &tRect );

	CDC tMemDC;
	tMemDC.CreateCompatibleDC( pDC );
	CBitmap tBitmap;
	tBitmap.CreateCompatibleBitmap( pDC, tRect.Width(), tRect.Height() );
	tMemDC.SelectObject( &tBitmap );	
	
	DrawTemplate( &tMemDC, tRect );	

	pDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), &tMemDC, 0, 0, SRCCOPY );
}

bool CStaticArg::DrawTemplate( CDC *pMemDC, CRect &tRect )
{
	WaitForSingleObject( mMutex, INFINITE );

	CStaticTitle::DrawTemplate( pMemDC, tRect );	

	pMemDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mTemplateDC, 0, 0, SRCCOPY );

	SetEvent( mMutex );

	return true;
}