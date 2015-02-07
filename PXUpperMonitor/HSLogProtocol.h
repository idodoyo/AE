#pragma once

class IHSLogProtocol
{
public:
	enum HSLOG_LEVEL{ HSLOG_ERROR, HSLOG_WARNING, HSLOG_INFO };
	enum HSLOG_PRINT_TYPE{ PRINT_TO_FILE, PRINT_TO_TERMINAL, PRINT_TO_BOTH };
	enum HSLOG_PREVENT_LIST_TYPE{ HSLOG_BLACK_LIST, HSLOG_WHITE_LIST };

	virtual void Init( HSLOG_LEVEL tLevel = HSLOG_INFO, 
				HSLOG_PRINT_TYPE tPrintType = PRINT_TO_BOTH, 
				bool tFlush = true, 
				int tFileSize = 1024 * 1024, 
				int tBackupCount = 16, 
				HSLOG_PREVENT_LIST_TYPE tListType = HSLOG_BLACK_LIST ) = 0;

	virtual void AddFormat( HSLOG_LEVEL tLevel, const char *pFormat = NULL, const char *pFile = NULL ) = 0;
	virtual void Print( const char *pFormat, ... ) = 0;
	virtual void SetPrintInfo( const char *pFile, const char *pFunc, int tLine, HSLOG_LEVEL tLevel ) = 0;
	virtual void MarkFile( const char *pFile ) = 0;

	virtual ~IHSLogProtocol(){}
};

class CHSLogLoader
{
public:
	static CHSLogLoader * SharedInstance()
	{
		static CHSLogLoader tLogLoader;
		return &tLogLoader;
	}

	bool Init( IHSLogProtocol::HSLOG_LEVEL tLevel = IHSLogProtocol::HSLOG_INFO, 
			IHSLogProtocol::HSLOG_PRINT_TYPE tPrintType = IHSLogProtocol::PRINT_TO_BOTH, 
			bool tFlush = true, 
			int tFileSize = 1024 * 1024, 
			int tBackupCount = 16,
			IHSLogProtocol::HSLOG_PREVENT_LIST_TYPE tListType = IHSLogProtocol::HSLOG_BLACK_LIST )
	{
		mLibInstance = LoadLibrary( "HSLog.dll" );
		if ( mLibInstance == NULL )
		{
			ReleaseLib();
			return false;
		}		

		typedef IHSLogProtocol * ( *GetLogInstance )();
		GetLogInstance pGetLogInstance = ( GetLogInstance )GetProcAddress( mLibInstance,"GetLogInstance" );
		if ( pGetLogInstance == NULL )
		{
			ReleaseLib();
			return false;
		}

		mLogProtocol = pGetLogInstance();
		mLogProtocol->Init( tLevel, tPrintType, tFlush, tFileSize, tBackupCount, tListType );

		return true;
	}

	IHSLogProtocol * LogProtocol(){ return mLogProtocol; }

	~CHSLogLoader(){ ReleaseLib(); }

private:
	CHSLogLoader(){}

	void ReleaseLib()
	{
		if ( mLibInstance != NULL )
		{
			FreeLibrary( mLibInstance );
		}

		mLibInstance = NULL;
	}

private:
	HINSTANCE mLibInstance;

	IHSLogProtocol *mLogProtocol;
};

#define HS_LOG CHSLogLoader::SharedInstance()->LogProtocol()

#define HS_INFO \
		HS_LOG->SetPrintInfo( __FILE__, __FUNCTION__, __LINE__, IHSLogProtocol::HSLOG_INFO ),\
		HS_LOG->Print

#define HS_WARNING \
		HS_LOG->SetPrintInfo( __FILE__, __FUNCTION__, __LINE__, IHSLogProtocol::HSLOG_WARNING ),\
		HS_LOG->Print

#define HS_ERROR \
		HS_LOG->SetPrintInfo( __FILE__, __FUNCTION__, __LINE__, IHSLogProtocol::HSLOG_ERROR ),\
		HS_LOG->Print

#define HS_MARK_FILE \
		HS_LOG->MarkFile( __FILE__ );
		