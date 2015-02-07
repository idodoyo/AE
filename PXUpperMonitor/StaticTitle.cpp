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

	//mTitleFont.CreatePointFont( 100, "Arial" );
	mTitleFont.CreatePointFont( 110, "ËÎÌו" );
}

CStaticTitle::~CStaticTitle()
{
	CloseHandle( mMutex );
	if ( mTemplateDC != NULL )
	{
		delete mTemplateDC;
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
	if ( mTemplateDC != NULL )
	{
		delete mTemplateDC;
		mTemplateDC = NULL;
	}	
}

void CStaticTitle::Draw( CDC *pDC )
{	
}

void CStaticTitle::DrawFrame( CDC *pMemDC, CRect &tRect )
{
	CPen tTempDigitPen( PS_SOLID, 3, RGB( 10, 10, 10 ) );	
	CPoint tTempPoints[ 3 ];
	tTempPoints[ 0 ].x = this->mLeftDelta;
	tTempPoints[ 0 ].y = this->mTopDelta - 2;
	tTempPoints[ 1 ].x = tRect.Width() - this->mLeftDelta;
	tTempPoints[ 1 ].y = tTempPoints[ 0 ].y;
	pMemDC->SelectObject( &tTempDigitPen );
	pMemDC->Polyline( tTempPoints, 2 );	
	
	tTempPoints[ 0 ].y = tRect.Height() - this->mTopDelta + 1;	
	tTempPoints[ 1 ].y = tTempPoints[ 0 ].y;
	pMemDC->Polyline( tTempPoints, 2 );	

	tTempPoints[ 0 ].x = this->mLeftDelta - 2;
	tTempPoints[ 0 ].y = this->mTopDelta;
	tTempPoints[ 1 ].x = tTempPoints[ 0 ].x;
	tTempPoints[ 1 ].y = tRect.Height() - this->mTopDelta;
	pMemDC->SelectObject( &tTempDigitPen );
	pMemDC->Polyline( tTempPoints, 2 );	
	
	tTempPoints[ 0 ].x = tRect.Width() - this->mLeftDelta + 1;	
	tTempPoints[ 1 ].x = tTempPoints[ 0 ].x;
	pMemDC->Polyline( tTempPoints, 2 );	
}

bool CStaticTitle::DrawTemplate( CDC *pMemDC, CRect &tRect )
{	
	if ( mTemplateDC == NULL )
	{	
		mTemplateDC = new CDC;

		mTemplateDC->CreateCompatibleDC( pMemDC );
		CBitmap tBitmap;
		tBitmap.CreateCompatibleBitmap( pMemDC, tRect.Width(), tRect.Height() );
		mTemplateDC->SelectObject( &tBitmap );
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