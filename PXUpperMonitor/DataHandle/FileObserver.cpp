#include "StdAfx.h"
#include "FileObserver.h"
#include "IndexManager.h"
#include "HSLogProtocol.h"

/*************************************************
  Function:		CFileObserver	
  Description:	构造函数
  Input:		tObserverType -- 类型
				pFile -- 文件名
				tFileSize -- 文件大小
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-19
  Last Time Modify:  None
**************************************************/	
CFileObserver::CFileObserver( OBSERVER_TYPE tObserverType, const HSChar *pFile, HSLong tFileSize, CWnd *pParent )
	: mObserverType( tObserverType ), mFileSize( tFileSize )
{
	mFileMutex = CreateEvent( NULL, false, true, NULL );
	mWriteMutex = CreateEvent( NULL, false, true, NULL );
	mFile = "";

	mWriteStream = NULL;
	mDisperseWriteStream = NULL;

	if ( pFile != NULL )
	{
		mWriteStream = fopen( pFile, "wb" );
		mFile = pFile;
	}		

	mDataBlockSize = 10 * 1024 * 1024;

	mIndexManager = new CIndexManager;

	mParent = pParent;

	mCurDisperseWriteStreamPos = 0;
	mDisperseFileLimit = 0;
	mDisperseFileFormat = "";
	mDisperseWriteStreamIndex = -1;
	mDisperseWriteStreamSize = 0;
}

/*************************************************
  Function:		~CFileObserver	
  Description:	析构函数
  Input:		None
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-19
  Last Time Modify:  None
**************************************************/	
CFileObserver::~CFileObserver()
{
	Close();

	CloseAllReadHandle();

	CloseHandle( mFileMutex );
	CloseHandle( mWriteMutex );

	delete mIndexManager;
}

/*************************************************
  Function:		Update	
  Description:	更新数据
  Input:		pBuf -- 数据
				tLength -- 数据长度
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
HSVoid CFileObserver::Update( HSChar *pBuf, HSInt tLength )
{		
	fwrite( pBuf, tLength, 1, mWriteStream );
}

/*************************************************
  Function:		Update	
  Description:	更新数据
  Input:		pBuf -- 数据
				tLength -- 数据长度
  Output:		None  					
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-04-24
  Last Time Modify:  None
**************************************************/
HSVoid CFileObserver::Update( DEVICE_CH_IDENTIFIER tType, HSChar *pBuf, HSInt tLength )
{
	WaitForSingleObject( mWriteMutex, INFINITE );

	HSUInt64 tFilePosition = FilePosition();

	/* data will stored in different data file. */
	if ( mDisperseFileLimit > 0 )
	{		
		if ( mDisperseWriteStreamIndex < 0 || ( mCurDisperseWriteStreamPos + tLength ) > mDisperseFileLimit )
		{
			if ( mDisperseWriteStream != NULL )
			{
				fclose( mDisperseWriteStream );
			}			

			mDisperseWriteStreamIndex++;

			HSChar tFileNameBuf[ 1024 ];
			sprintf_s( tFileNameBuf, mDisperseFileFormat.c_str(), mDisperseWriteStreamIndex );

			mDisperseWriteStream = fopen( tFileNameBuf, "wb" );	
			mCurDisperseWriteStreamPos = 0;
		}		

		fwrite( pBuf, tLength, 1, mDisperseWriteStream );
		fflush( mDisperseWriteStream );	
		
		mIndexManager->AddIndex( tType, mDisperseWriteStreamIndex, ( HSUInt )mCurDisperseWriteStreamPos, tLength );

		tFilePosition = mDisperseWriteStreamSize;
		mDisperseWriteStreamSize += tLength;

		mCurDisperseWriteStreamPos += tLength;
	}
	else
	{
		fwrite( pBuf, tLength, 1, mWriteStream );
		fflush( mWriteStream );

		mIndexManager->AddIndex( tType, ( HSUInt )( tFilePosition / mDataBlockSize ), tFilePosition % mDataBlockSize, tLength );
	}	

	SetEvent( mWriteMutex );

	mParent->PostMessageA( WM_UPDATE_FILE_SIZE_MESSAGE, 
						( UINT )( ( tFilePosition + tLength ) / mDataBlockSize ), 
						( tFilePosition + tLength ) % mDataBlockSize );
}

/*************************************************
  Function:		SetFile	
  Description:	设置保存数据文件
  Input:		pFile -- 文件名
				tForRead -- 只读
				tFileHeaderSize -- 文件头大小
  Output:		None  					
  Return:		1 ok, 0, failed
  Author:		Guo Dengjia
  Date :		2014-02-19
  Last Time Modify:  None
**************************************************/	
HSBool CFileObserver::SetFile( const HSChar *pFile, HSBool tForRead, HSInt64 tDisperseFileSize )
{
	mIndexManager->Reset();

	CloseAllReadHandle();
	mFile = pFile;

	SetDisperseFileSize( tDisperseFileSize );

	if ( tForRead )
	{
		return HSTrue;
	}		

	if ( mWriteStream != NULL )
	{
		fclose( mWriteStream );
	}

	mWriteStream = fopen( pFile, "wb" );

	return ( mWriteStream != NULL ? HSTrue : HSFalse );
}

/*************************************************
  Function:		SetDisperseFileSize	
  Description:	设置保存数据文件
  Input:		pFile -- 文件名
				tForRead -- 只读
				tFileHeaderSize -- 文件头大小
  Output:		None  					
  Return:		1 ok, 0, failed
  Author:		Guo Dengjia
  Date :		2014-02-19
  Last Time Modify:  None
**************************************************/	
HSVoid CFileObserver::SetDisperseFileSize( HSInt64 tSize )
{
	mDisperseFileLimit = tSize;
	mCurDisperseWriteStreamPos = 0;
	mDisperseWriteStreamSize = 0;
	mDisperseWriteStreamIndex = -1;
	if ( mDisperseFileLimit > 0 )
	{		
		HSString tFilePath = mFile.substr( 0, mFile.rfind( '\\' ) + 1 );
		HSString tFileBase = mFile.substr( mFile.rfind( '\\' ) + 1 );
		tFileBase = tFileBase.substr( 0, tFileBase.rfind( '.' ) );

		CreateDirectory( ( tFilePath + tFileBase + "\\" ).c_str(), NULL );

		mDisperseFileFormat = tFilePath + tFileBase + "\\" + tFileBase + "%05d.pac";
	}	

	if ( mDisperseWriteStream != NULL )
	{
		fclose( mDisperseWriteStream );
		mDisperseWriteStream = NULL;
	}	
}

/*************************************************
  Function:		FinalSize	
  Description:	文件保存大小
  Input:		None
  Output:		None  					
  Return:		结果
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
HSLong CFileObserver::FinalSize() const
{
	return this->mFileSize;
}

/*************************************************
  Function:		ObserverType	
  Description:	返回Observer类型
  Input:		None
  Output:		None  					
  Return:		结果
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/
OBSERVER_TYPE CFileObserver::ObserverType() const
{
	return mObserverType;
}

/*************************************************
  Function:		ReadData	
  Description:	读取数据
  Input:		tHandle -- 标示
				pBuf -- 存储数据				
				tLength -- 读取长度
  Output:		pBuf -- 目标数据  					
  Return:		data length readed
  Author:		Guo Dengjia
  Date :		2014-02-20
  Last Time Modify:  None
**************************************************/	
HSInt CFileObserver::ReadData( HSChar *pBuf, HSULong tLength )
{
	FILE *pStream = GetFileHandle( 0 );
	if ( pStream == NULL )
	{
		return 0;
	}	

	return fread( pBuf, 1, tLength, pStream );
}

/*************************************************
  Function:		ReadData	
  Description:	读取数据
  Input:		tHandle -- 标示
				pBuf -- 存储数据				
				tLength -- 读取长度
  Output:		pBuf -- 目标数据  					
  Return:		data length readed
  Author:		Guo Dengjia
  Date :		2014-04-24
  Last Time Modify:  None
**************************************************/	
CFileObserver::READ_DATA_RES CFileObserver::ReadData( DEVICE_CH_IDENTIFIER tType, HSUInt tIndex, HSChar *pBuf, HSInt &tLength )
{
	HSUInt tResIndex = 0;
	HSUInt tResOffset = 0;
	HSUInt tResLength = 0;
	if ( !mIndexManager->GetIndex( tType, tIndex, tResIndex, tResOffset, tResLength ) )
	{		
		HS_ERROR( "Get Frame Index: %d Error\n", tIndex );
		return READ_NO_DATA;
	}

	FILE *pStream = GetFileHandle( tType );

	HSInt64 tReadFilePos = ( ( HSInt64 )tResIndex ) * ( ( HSInt64 )mDataBlockSize ) + tResOffset;
	if ( mDisperseFileLimit > 0 )	
	{
		HSInt64 tReadIndex = tResIndex;
		tReadFilePos = tResOffset;

		if ( tReadIndex != mDisperseFileIndexs[ tType ] || pStream == NULL )
		{
			if ( pStream != NULL )
			{
				fclose( pStream );
			}

			HSChar tFileNameBuf[ 1024 ];
			sprintf_s( tFileNameBuf, mDisperseFileFormat.c_str(), tReadIndex );
			pStream = fopen( tFileNameBuf, "rb" );
			mFileHandles[ tType ] = pStream;
		}	

		mDisperseFileIndexs[ tType ] = tReadIndex;
	}

	if ( pStream == NULL )
	{				
		return READ_NO_DATA;		
	}

	if ( _fseeki64( pStream, tReadFilePos, SEEK_SET ) != 0 )
	{
		return READ_FAILED;
	}

	tLength = tResLength;

	if ( fread( pBuf, tResLength, 1, pStream ) > 0 )
	{
		return READ_SUCCESS;
	}	

	return READ_FAILED;
}

/*************************************************
  Function:		ItemsLengthWithType	
  Description:	返回各数据节点长度
  Input:		tType -- 类型	
  Output:		tItemsLength -- 数据节点长度		
  Return:		1 ok, 0 failed
  Author:		Guo Dengjia
  Date :		2014-08-19
  Last Time Modify:  None
**************************************************/	
HSInt CFileObserver::ItemsLengthWithType( DEVICE_CH_IDENTIFIER tType, vector< HSUInt > &tItemsLength )
{
	HSUInt tResIndex = 0;
	HSUInt tResOffset = 0;
	HSUInt tResLength = 0;
	HSUInt tIndex = 0;
	while ( mIndexManager->GetIndex( tType, tIndex, tResIndex, tResOffset, tResLength ) )
	{
		tItemsLength.push_back( tResLength );
		tIndex++;
	}

	return tItemsLength.size();
}

/*************************************************
  Function:		SetFilePosition	
  Description:	设置文件指针
  Input:		tHandle -- 标示				
				tPosition -- 目标位置				
  Output:		None
  Return:		1 ok, 0 failed
  Author:		Guo Dengjia
  Date :		2014-02-20
  Last Time Modify:  None
**************************************************/	
HSBool CFileObserver::SetFilePosition( HSInt64 tPosition )
{
	FILE *pStream = GetFileHandle( 0 );
	if ( pStream == NULL )
	{
		return HSFalse;
	}	

	if ( _fseeki64( pStream, tPosition, SEEK_SET ) != 0 )
	{
		return HSFalse;
	}

	return HSTrue;
}

/*************************************************
  Function:		SetFileSavePosition	
  Description:	设置文件指针
  Input:		tPosition -- 目标位置				
  Output:		None
  Return:		1 ok, 0 failed
  Author:		Guo Dengjia
  Date :		2014-02-20
  Last Time Modify:  None
**************************************************/	
HSBool CFileObserver::SetFileSavePosition( HSInt64 tPosition )
{
	if ( _fseeki64( mWriteStream, tPosition, SEEK_SET ) != 0 )
	{
		return HSFalse;
	}		

	return HSTrue;
}

/*************************************************
  Function:		FilePosition	
  Description:	返回文件位置
  Input:		None
  Output:		None
  Return:		结果
  Author:		Guo Dengjia
  Date :		2014-04-22
  Last Time Modify:  None
**************************************************/	
HSInt64 CFileObserver::FilePosition()
{	
	return _ftelli64( mWriteStream );
}

/*************************************************
  Function:		FileBlockSize	
  Description:	返回数据块大小
  Input:		None
  Output:		None
  Return:		结果
  Author:		Guo Dengjia
  Date :		2014-04-29
  Last Time Modify:  None
**************************************************/	
HSInt CFileObserver::FileBlockSize()
{
	return mDataBlockSize;
}

/*************************************************
  Function:		FilePosition	
  Description:	返回文件位置
  Input:		None
  Output:		None
  Return:		结果
  Author:		Guo Dengjia
  Date :		2014-04-22
  Last Time Modify:  None
**************************************************/	
HSVoid CFileObserver::Close()
{	
	if ( mWriteStream != NULL )
	{
		fclose( mWriteStream );
		mWriteStream = NULL;
	}

	if ( mDisperseWriteStream != NULL )
	{
		fclose( mDisperseWriteStream );
		mDisperseWriteStream = NULL;
	}
}

/*************************************************
  Function:		GetFileHandle	
  Description:	返回文件句柄
  Input:		tType -- 标示		
  Output:		None
  Return:		结果
  Author:		Guo Dengjia
  Date :		2014-02-21
  Last Time Modify:  None
**************************************************/	
FILE * CFileObserver::GetFileHandle( DEVICE_CH_IDENTIFIER tType )
{
	WaitForSingleObject( mFileMutex, INFINITE );

	map< DEVICE_CH_IDENTIFIER, FILE * >::iterator pIterator = mFileHandles.find( tType );
	FILE *pStream = NULL;
	/* new register object */
	if ( pIterator == mFileHandles.end() )
	{		
		pStream = fopen( mFile.c_str(), "rb" );
		if ( pStream == NULL )
		{
			SetEvent( mFileMutex );			

			return NULL;
		}

		mFileHandles[ tType ] = pStream;
		mDisperseFileIndexs[ tType ] = -1;
	}
	else
	{
		pStream = pIterator->second;
	}

	SetEvent( mFileMutex );

	return pStream;
}

/*************************************************
  Function:		CloseAllReadHandle	
  Description:	关闭所有读数据流
  Input:		None
  Output:		None
  Return:		None
  Author:		Guo Dengjia
  Date :		2014-03-06
  Last Time Modify:  None
**************************************************/	
HSVoid CFileObserver::CloseAllReadHandle()
{
	map< DEVICE_CH_IDENTIFIER, FILE * >::iterator pIterator = mFileHandles.begin();
	while ( pIterator != mFileHandles.end() )
	{
		if ( pIterator->second != NULL )
		{
			fclose( pIterator->second );
		}
		
		pIterator++;
	}

	mFileHandles.clear();
	mDisperseFileIndexs.clear();
}