#include "StdAfx.h"
#include "ArgListTransfer.h"
#include "HSLogProtocol.h"
#include "ArgCalculator.h"

CArgListTransfer::CArgListTransfer( IDataHandler *pDataHandler, DEVICE_CH_IDENTIFIER tIdentifier )
	: mDataHandler( pDataHandler ), mIdentifier( tIdentifier )
{
	CArgCalculator::SharedInstance()->AddChannel( tIdentifier );
	Reset();	
}

CArgListTransfer::~CArgListTransfer()
{	
}

HSVoid CArgListTransfer::GetArg( vector< CArgTableManager::ArgItem > &tArgList )
{	
	HSDouble tLastSecond = 0;
	HSDouble tLastArgSecond = 0;

	mDataHandler->GetArgList( mIdentifier, mNSecondBegin, mNSecondEnd - mNSecondBegin, tArgList, tLastSecond, tLastArgSecond );	
	
	if ( tArgList.size() > 0 )
	{
		mNSecondBegin = tArgList[ tArgList.size() - 1 ].NBeginTime + 1;
	}

	mNSecondEnd += mNDuration;
}

HSVoid CArgListTransfer::Reset()
{	
	mNSecondBegin = 0;	
	mNDuration = 1 * 1000 * 1000 * 1000;
	mNSecondEnd = mNDuration;	
}