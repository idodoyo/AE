#pragma once

#include "DataTypes.h"

template< typename T >
class CThreadControlEx
{
public:
	CThreadControlEx( T *pParent, HSUInt tMilliSeconds, HSInt tID, HSBool tAutoQuit = HSFalse );
	~CThreadControlEx();

public:
	HSBool Start();
	HSVoid Stop();
	HSVoid Pause();	
	HSBool IsPaused(){ return mIsPaused; }
	HSBool IsStart(){ return !( mIsStoped || mIsPaused ); }
	HSBool IsStoped(){ return mIsStoped; }
	HSVoid SetDuration( HSUInt tMilliseconds );	

private:
	static DWORD WINAPI ThreadFunc( LPVOID lpParam );	

private:	
	HANDLE mThreadMutex;
	HSInt mID;

	HSBool mIsStoped;
	HSBool mIsPaused;

	HSBool mAutoQuit;

	HSUInt mTimeDuration;

	T *mParent;

	HANDLE mThreadHandle;
};

/*************************************************
  Function:		CThreadControlEx	
  Description:	构造函数
  Input:		pParent -- parent
				tTimeDuration -- 间隔提醒时间
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
template< typename T >
CThreadControlEx< T >::CThreadControlEx( T *pParent, HSUInt tMilliSeconds, HSInt tID, HSBool tAutoQuit )
	: mParent( pParent ), mTimeDuration( tMilliSeconds ), mID( tID ), mAutoQuit( tAutoQuit )
{
	mThreadHandle = NULL;
	mIsPaused = HSFalse;
	mIsStoped = HSTrue;
	mThreadMutex = CreateEvent( NULL, HSFalse, HSTrue, NULL );
}

/*************************************************
  Function:		~CThreadControlEx	
  Description:	析构函数
  Input:		None
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
template< typename T >
CThreadControlEx< T >::~CThreadControlEx()
{
	CloseHandle( mThreadMutex );
}

/*************************************************
  Function:		SetDuration
  Description:	设置间隔时间
  Input:		tMilliseconds -- 目标时间
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
template< typename T >
HSVoid CThreadControlEx< T >::SetDuration( HSUInt tMilliseconds )
{
	WaitForSingleObject( mThreadMutex, INFINITE );
	mTimeDuration = tMilliseconds;
	SetEvent( mThreadMutex );
}

/*************************************************
  Function:		Start	
  Description:	开始数据采集
  Input:		None
  Output:		None  					
  Return:		1 ok, 0, failed
  Author:		Guo Dengjia
  Date :		2014-02-19
  Last Time Modify:  None
**************************************************/	
template< typename T >
HSBool CThreadControlEx< T >::Start()
{		
	if ( mIsStoped )	
	{
		mIsPaused = HSFalse;
		mIsStoped = HSFalse;

		DWORD tThreadID = 0;
		mThreadHandle = CreateThread( NULL, 0, ThreadFunc, this, 0, &tThreadID );  	
		if ( mThreadHandle == NULL ) 
		{			
			return HSFalse;
		}
	}
	else if ( mIsPaused )
	{
		WaitForSingleObject( mThreadMutex, INFINITE );
		mIsPaused = HSFalse;
		mIsStoped = HSFalse;
		SetEvent( mThreadMutex );
	}

	return HSTrue;
}

/*************************************************
  Function:		Stop	
  Description:	停止数据采集
  Input:		None
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-19
  Last Time Modify:  None
**************************************************/	
template< typename T >
HSVoid CThreadControlEx< T >::Stop()
{
	WaitForSingleObject( mThreadMutex, INFINITE );
	if ( mIsStoped )
	{
		SetEvent( mThreadMutex );
		return;
	}

	mIsStoped = HSTrue;
	mIsPaused = HSFalse;
	SetEvent( mThreadMutex );

	if ( mThreadHandle != NULL && !mAutoQuit )
	{
		WaitForSingleObject( mThreadHandle, INFINITE );
	}
}

/*************************************************
  Function:		Pause	
  Description:	暂停数据采集
  Input:		None
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-19
  Last Time Modify:  None
**************************************************/	
template< typename T >
HSVoid CThreadControlEx< T >::Pause()
{
	WaitForSingleObject( mThreadMutex, INFINITE );
	mIsPaused = HSTrue;
	SetEvent( mThreadMutex );
}

/*************************************************
  Function:		ThreadFunc	
  Description:	线程函数
  Input:		lpParam -- this
  Output:		None  					
  Return:		结果
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
template< typename T >
DWORD CThreadControlEx< T >::ThreadFunc( LPVOID lpParam )
{
	CThreadControlEx *pThreadControl = ( CThreadControlEx * )lpParam;	

	HSBool tSelfStoped = HSFalse;
	HSUInt tBeginTime = 0;	
	while ( 1 )
	{
		WaitForSingleObject( pThreadControl->mThreadMutex, INFINITE );

		if ( tSelfStoped )
		{
			pThreadControl->mIsStoped = HSTrue;
			pThreadControl->mIsPaused = HSFalse;
		}

		/* already stop */
		if ( pThreadControl->mIsStoped )
		{
			SetEvent( pThreadControl->mThreadMutex );
			break;
		}

		/* already pause */
		if ( pThreadControl->mIsPaused )
		{
			SetEvent( pThreadControl->mThreadMutex );
			Sleep( 50 );
			continue;
		}

		SetEvent( pThreadControl->mThreadMutex );

		/* notify parent times up */
		if ( pThreadControl->mTimeDuration == 0 )
		{
			if ( !pThreadControl->mParent->ThreadRuning( pThreadControl->mID ) )
			{
				tSelfStoped = HSTrue;				
			}
		}
		else
		{
			HSUInt tCurrentTime = GetTickCount();
			if ( tCurrentTime - tBeginTime > pThreadControl->mTimeDuration )
			{
				if ( !pThreadControl->mParent->ThreadRuning( pThreadControl->mID ) )
				{
					tSelfStoped = HSTrue;	
				}
				
				tBeginTime = GetTickCount();
			}
			else
			{
				Sleep( 10 );			
			}
		}
	}

	/* notify parent will out */
	pThreadControl->mParent->ThreadWillStop( pThreadControl->mID );	

	return 0;
}