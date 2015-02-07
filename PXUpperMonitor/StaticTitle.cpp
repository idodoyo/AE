// StaticGraphic.cpp : implementation file
//

#include "stdafx.h"
#include "PXUpperMonitor.h"
#include "StaticTitle.h"
#include "DataTypes.h"
#include "HSLogProtocol.h"

#include <fstream>
#include <sstream>

using std::ofstream;
using std::ostringstream;


// CStaticTitle

IMPLEMENT_DYNAMIC(CStaticTitle, CStatic)

CStaticTitle::CStaticTitle()
{
	mTitle = "DEVICE < CHANNEL >";

	mMutex = CreateEvent( NULL, false, true, NULL );
	mTemplateDC = NULL;

	mTopDelta = 30;
	mLeftDelta = 30;	

	mFocused = false;	

	mType = "";
	
	mTitleFont.CreatePointFont( 110, "ËÎÌו" );

	mNeedResizeWave = HSTrue;
}

CStaticTitle::~CStaticTitle()
{
	CloseHandle( mMutex );
	if ( mTemplateDC != NULL )
	{
		delete mTemplateDC;
		delete mFrameTemplateDC;
	}		
}

BEGIN_MESSAGE_MAP(CStaticTitle, CStatic)
	ON_WM_PAINT()	
END_MESSAGE_MAP()



// CStaticTitle message handlers

void CStaticTitle::SetTitle( CString tTitle )
{
	mTitle = tTitle;

	ResizeWave();
}

void CStaticTitle::ResizeWave()
{	
	mNeedResizeWave = HSTrue;	
}

void CStaticTitle::Draw( CDC *pDC )
{	
}

void CStaticTitle::ClearFrame( CDC *pMemDC, CRect &tRect )
{
	CRect tLeftRect( 0, 0, mLeftDelta, tRect.bottom );
	pMemDC->FillSolidRect( &tLeftRect, RGB( 0, 0, 0 ) );

	CRect tRightRect( tRect.right - mLeftDelta, 0, tRect.right, tRect.bottom );
	pMemDC->FillSolidRect( &tRightRect, RGB( 0, 0, 0 ) );

	CRect tTopRect( 0, 0, tRect.right, mTopDelta );
	pMemDC->FillSolidRect( &tTopRect, RGB( 0, 0, 0 ) );

	CRect tBottomRect( 0, tRect.bottom - mTopDelta, tRect.right, tRect.bottom );
	pMemDC->FillSolidRect( &tBottomRect, RGB( 0, 0, 0 ) );
}

void CStaticTitle::DrawFrameRect( CDC *pMemDC, CRect &tRect )
{
/*	CRect tLeftRect( 0, 0, mLeftDelta, tRect.bottom );
	pMemDC->FillSolidRect( &tLeftRect, RGB( 0, 0, 0 ) );

	CRect tRightRect( tRect.right - mLeftDelta, 0, tRect.right, tRect.bottom );
	pMemDC->FillSolidRect( &tRightRect, RGB( 0, 0, 0 ) );

	CRect tTopRect( 0, 0, tRect.right, mTopDelta );
	pMemDC->FillSolidRect( &tTopRect, RGB( 0, 0, 0 ) );

	CRect tBottomRect( 0, tRect.bottom - mTopDelta, tRect.right, tRect.bottom );
	pMemDC->FillSolidRect( &tBottomRect, RGB( 0, 0, 0 ) );*/

	ClearFrame( pMemDC, tRect );

	pMemDC->BitBlt( tRect.left, tRect.top, tRect.Width(), tRect.Height(), mFrameTemplateDC, 0, 0, SRCPAINT );
}

bool CStaticTitle::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	if ( mNeedResizeWave )
	{
		if ( mTemplateDC != NULL )
		{
			delete mTemplateDC;
			mTemplateDC = NULL;

			delete mFrameTemplateDC;
			mFrameTemplateDC = NULL;

			mTemplateBitmap.DeleteObject();
			mFrameTemplateBitmap.DeleteObject();
		}

		mNeedResizeWave = HSFalse;
	}

	if ( mTemplateDC == NULL )
	{	
		mFrameTemplateDC = new CDC;
		mFrameTemplateDC->CreateCompatibleDC( pMemDC );		
		mFrameTemplateBitmap.CreateCompatibleBitmap( pMemDC, tRect.Width(), tRect.Height() );
		mFrameTemplateDC->SelectObject( &mFrameTemplateBitmap );


		mTemplateDC = new CDC;
		mTemplateDC->CreateCompatibleDC( pMemDC );		
		mTemplateBitmap.CreateCompatibleBitmap( pMemDC, tRect.Width(), tRect.Height() );
		mTemplateDC->SelectObject( &mTemplateBitmap );

		if ( mFocused )
		{
			mTemplateDC->FillSolidRect( &tRect, RGB( 185, 185, 185 ) );
		}
		else
		{
			mTemplateDC->FillSolidRect( &tRect, RGB( 210, 210, 210 ) );
		}

		mTemplateDC->SetBkMode( TRANSPARENT );

		int tTitleDelta = ( mTopDelta - 16 ) / 2;
				
		CFont *tOldFont = mTemplateDC->SelectObject( &mTitleFont );

		CRect tTitleRect( tRect.left, tRect.top + tTitleDelta, tRect.right, tRect.top + mTopDelta - tTitleDelta );
		mTemplateDC->DrawText( mType + mTitle, &tTitleRect, DT_CENTER | DT_VCENTER );	

		mTemplateDC->SelectObject( tOldFont );

		if ( mLeftDelta == 0 )
		{
			return true;
		}

		int tTopDelta =  mTopDelta - 3;	
		int tLeftDelta = mLeftDelta - 3;
		CRect tInlineRect( tRect.left + tLeftDelta, tRect.top + tTopDelta, tRect.right - tLeftDelta, tRect.bottom - tTopDelta );
		mTemplateDC->FillSolidRect( &tInlineRect, RGB( 10, 10, 10 ) );	
		
		CRect tViewRect( tRect.left + mLeftDelta, tRect.top + mTopDelta, tRect.right - mLeftDelta, tRect.bottom - mTopDelta );
		mTemplateDC->FillSolidRect( &tViewRect, RGB( 240, 240, 240 ) );	

		return true;
	}

	return false;	
}

void CStaticTitle::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CStatic::OnPaint() for painting messages

	Draw( &dc );
}