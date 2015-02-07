#pragma once

#include "DataTypes.h"

class CBmpLoader
{
public:
	typedef struct BITMAPFILEHEADER {
		unsigned short    bfType;
		unsigned long     bfSize;
		unsigned short    bfReserved1;
		unsigned short    bfReserved2;
		unsigned long     bfOffBits;
	} BITMAPFILEHEADER;  

	typedef struct BITMAPINFOHEADER {
		unsigned long      biSize;
		long               biWidth;
		long               biHeight;
		unsigned short     biPlanes;
		unsigned short     biBitCount;
		unsigned long      biCompression;
		unsigned long      biSizeImage;
		long               biXPelsPerMeter;
		long               biYPelsPerMeter;
		unsigned long      biClrUsed;
		unsigned long      biClrImportant;
	} BITMAPINFOHEADER;
		
public:		
	CBmpLoader(){ mFile = NULL; }
	~CBmpLoader(){ CloseBmpFile(); }
	
	HSBool LoadFile( HSString tFileName )
	{
		CloseBmpFile();
		mFile = fopen( tFileName.c_str(), "rb" );
		if( mFile != NULL )
		{			
			fread( &mBMPFileHead, 1, 14, mFile );
			fread( &mBMPFileInfo, 1, 40, mFile );

			mImageDataSize = mBMPFileInfo.biWidth * mBMPFileInfo.biHeight * mBMPFileInfo.biBitCount / 8;
			return HSTrue;
		}
		else
		{
			return HSFalse;
		}		
	}

	HSVoid ReadImageData( HSChar *pData, HSInt tLen )
	{
		if( mFile != NULL )
		{
			fseek( mFile, mBMPFileHead.bfOffBits, SEEK_SET );
			fread( pData, 1, tLen, mFile );
		}		
	}

private:
	HSVoid CloseBmpFile()
	{
		if( mFile != NULL )
		{
			fclose( mFile );
			mFile = NULL;
		}
	}

private:	
	FILE *mFile;

	BITMAPFILEHEADER mBMPFileHead;
	
	BITMAPINFOHEADER mBMPFileInfo;
	
	HSULong mImageDataSize;
};