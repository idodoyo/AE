#include "StdAfx.h"
#include "AutoAlignManager.h"
#include "HSLogProtocol.h"


CAutoAlignManager::CAutoAlignManager( CWnd *pParent, CRect tMinRect ) 
	: mParent( pParent ), mMinRect( tMinRect )
{
	mPrevViewRect = CRect( 0, 0, 0, 0 );	
}


CAutoAlignManager::~CAutoAlignManager()
{	
}

CRect CAutoAlignManager::ViewRect()
{
	CRect tViewRect;
	mParent->GetClientRect( &tViewRect );

	return tViewRect;
}

HSVoid CAutoAlignManager::AddControl( HSUInt tID, CRect tMargin, HSBool tInitControlPos, HSUInt tAlignType )
{		
	CWnd *pControl = mParent->GetDlgItem( tID );
	if ( pControl == NULL )
	{
		return;
	}		

	HSUInt tTmpAlignType = 0;
	tTmpAlignType |= ( tMargin.left >= 0 ? ALIGN_LEFT : 0 );
	tTmpAlignType |= ( tMargin.top >= 0 ? ALIGN_TOP : 0 );
	tTmpAlignType |= ( tMargin.right >= 0 ? ALIGN_RIGHT : 0 );
	tTmpAlignType |= ( tMargin.bottom >= 0 ? ALIGN_BOTTOM : 0 );

	tAlignType |= tTmpAlignType;
	CONTROL_INFO tControl = { tID, tAlignType };
	mControls.push_back( tControl );

	if ( !tInitControlPos )
	{
		return;
	}

	HSUInt tInitAlignType = ( tTmpAlignType | ALIGN_INIT_SET );

	CRect tControlRect;
	pControl->GetClientRect( &tControlRect );	

	CRect tViewRect = ViewRect();
	CRect tNewRect = GetNewRect( tControlRect, tViewRect, tMargin, tInitAlignType );
	pControl->MoveWindow( tNewRect );
}

CRect CAutoAlignManager::GetNewRect( CRect &tControlRect, CRect &tViewRect, CRect &tInitRect, HSUInt tAlignType )
{
	HSInt tLeft = tControlRect.left;
	HSInt tRight = tControlRect.right;
	HSInt tTop = tControlRect.top;
	HSInt tBottom = tControlRect.bottom;

	if ( tAlignType & ALIGN_LEFT )
	{		
		tLeft = GetPositionWithType( tControlRect.left, tInitRect.left, mPrevViewRect.Width(), tViewRect.Width(), tAlignType );

		if ( !( tAlignType & ALIGN_RIGHT ) )
		{
			tRight = tLeft + tControlRect.Width();
		}		
	}

	if ( tAlignType & ALIGN_RIGHT )
	{		
		tRight = GetPositionWithType( tControlRect.right, tInitRect.right, mPrevViewRect.Width(), tViewRect.Width(), tAlignType, HSTrue );
		
		if ( !( tAlignType & ALIGN_LEFT ) )
		{
			tLeft = tRight - tControlRect.Width();
		}
	}

	if ( tAlignType & ALIGN_TOP )
	{		
		tTop = GetPositionWithType( tControlRect.top, tInitRect.top, mPrevViewRect.Height(), tViewRect.Height(), tAlignType );
		
		if ( !( tAlignType & ALIGN_BOTTOM ) )
		{
			tBottom = tTop + tControlRect.Height();
		}		
	}

	if ( tAlignType & ALIGN_BOTTOM )
	{		
		tBottom = GetPositionWithType( tControlRect.bottom, tInitRect.bottom, mPrevViewRect.Height(), tViewRect.Height(), tAlignType, HSTrue );
		
		if ( !( tAlignType & ALIGN_TOP ) )
		{
			tTop = tBottom - tControlRect.Height();
		}
	}	

	return CRect( tLeft, tTop, tRight, tBottom );
}

HSInt CAutoAlignManager::GetPositionWithType( HSInt tPoint, HSInt tInitPoint, HSDouble tPrevLength, HSDouble tNewLength, HSUInt tAlignType, HSBool tNeedReverse )
{
	if ( tAlignType & ALIGN_INIT_SET )
	{				
		return ( tNeedReverse ? ( HSInt )( tNewLength - tInitPoint ) : tInitPoint );
	}
	else if ( tAlignType & ALIGN_KEEP_PERCENT )
	{		
		return ( HSInt )( tPoint / tPrevLength * tNewLength );
	}
	else
	{				
		return ( tNeedReverse ? ( HSInt )( tNewLength - ( tPrevLength - tPoint ) ) : tPoint );
	}
}

HSVoid CAutoAlignManager::Resize()
{
	CRect tViewRect;
	mParent->GetClientRect( &tViewRect );
	
	if ( tViewRect.Width() < mMinRect.Width() || tViewRect.Height() < mMinRect.Height() )
	{
		return;
	}

	if ( mPrevViewRect.Width() == 0 )
	{		
		mPrevViewRect = tViewRect;
		return;
	}

	list< CONTROL_INFO >::iterator pIterator = mControls.begin();
	while ( pIterator != mControls.end() )
	{
		CWnd *pControl = mParent->GetDlgItem( pIterator->ID );

		CRect tControlRect;
		pControl->GetWindowRect( &tControlRect );
		mParent->ScreenToClient( &tControlRect );				

		CRect tInitRect = CRect( 0, 0, 0, 0 );
		CRect tControlNewRect = GetNewRect( tControlRect, tViewRect, tInitRect, pIterator->AlignType );		
		pControl->MoveWindow( tControlNewRect );
		
		pIterator++;
	}

	mPrevViewRect = tViewRect;
}