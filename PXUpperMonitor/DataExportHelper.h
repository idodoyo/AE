#pragma once

#include "stdafx.h"

#include "DataTypes.h"

class CDataExportHelper
{
public:
	CDataExportHelper( CDialogEx *pParent ) : mParent( pParent ){}

	~CDataExportHelper()
	{
		if ( mStream.is_open() ) 
		{
			mStream.close();
		}
	}

	HSBool GetFilePath()
	{
		CFileDialog tFileDialog( FALSE, "txt", NULL, OFN_HIDEREADONLY, "TXT Files (*.txt)|*.txt|" );
		if ( IDOK == tFileDialog.DoModal() )
		{
			ofstream tStream( ( LPCSTR )tFileDialog.GetPathName() );
			mStream.open( ( LPCSTR )tFileDialog.GetPathName() );
			if ( mStream.is_open() )
			{
				return HSTrue;				
			}			
			
			mParent->MessageBox( "文件打开失败!", "警告", MB_ICONWARNING );
		}

		return HSFalse;
	}

	HSVoid Write( const HSChar * pTxt )
	{
		mStream << pTxt;
	}	

	HSVoid Finish()
	{
		mParent->MessageBox( "数据已导出!", "警告", MB_ICONWARNING );
	}

private:
	ofstream mStream;
	CDialogEx *mParent;
};