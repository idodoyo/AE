#include "StdAfx.h"
#include "ArgListTransfer.h"
#include "HSLogProtocol.h"
#include "ArgCalculator.h"

/*************************************************
  Function:		CArgListTransfer	
  Description:	Constructor
  Input:		pDataHandler -- device;
				tIdentifier -- channel
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
CArgListTransfer::CArgListTransfer( IDataHandler *pDataHandler, DEVICE_CH_IDENTIFIER tIdentifier )
	: mDataHandler( pDataHandler ), mIdentifier( tIdentifier )
{
	CArgCalculator::SharedInstance()->AddChannel( tIdentifier );
	Reset();	
}

/*************************************************
  Function:		~CArgListTransfer	
  Description:	Destructor
  Input:		None
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/	
CArgListTransfer::~CArgListTransfer()
{	
}

/*************************************************
  Function:		GetArg
  Description:	Get the arglist
  Input:		None
  Output:		tArgList -- arglist got
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/
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

/*************************************************
  Function:		Reset
  Description:	Reset the time of arg get.
  Input:		None
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-09-17
  Last Time Modify:  None
**************************************************/
HSVoid CArgListTransfer::Reset()
{	
	mNSecondBegin = 0;	
	mNDuration = 1 * 1000 * 1000 * 1000;
	mNSecondEnd = mNDuration;	
}