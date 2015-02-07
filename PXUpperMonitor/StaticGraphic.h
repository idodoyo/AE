#pragma once

#include "DataHandler.h"
#include "StaticGrid.h"

// CStaticGraphic

class CStaticGraphic : public CStaticGrid
{	
public:
	CStaticGraphic();
	virtual ~CStaticGraphic();

public:		
	double TotalVoltage();
	double BeginVoltage();
	void SetVoltage( double tTotalVoltage, double tBeginVoltage );

	double BeginSecond(){ return mBeginSeconds; }
	double TotalSecond(){ return mTotalSeconds; }
	void SetSeconds( double tTotalSeconds, double tBeginSeconds );	

	void SetData( char *pData, int tLength, HSDouble tBeginSecond );
	void SetDataHandler( IDataHandler *pDataHandler, DEVICE_CH_IDENTIFIER tChannelIdentifier );
	bool ExportDataToFile( CDialogEx *pParent );
	int  DataLength(){ return mLength; }	

	void ResizeWave();

protected:	
	void Draw( CDC *pDC );
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );
	void DrawWave( CDC *pMemDC, CRect &tRect );			

	bool ValueInRange( int tStart, int tEnd, double &tBigValue, double &tSmallValue, double &tFirstValue, double &tLastValue, int tEachSampleSize );
	long DigitWithVoltage( CRect &tRect, double tVoltage, double tDigitPerVoltage );
	double GetVerStart( double tVerDelta, int tTop, double &tBeginVoltage );	

private:	
	double mTotalVoltage;
	double mBeginVoltage;	
	double mTotalSeconds;
	double mBeginSeconds;		

	double mDataBeginSecond;

	CPoint *mPoints;
	int mPointLength;

	CString mSecondUnit;
	CString mVoltageUnit;

	char *mData;
	int mLength;

	IDataHandler *mDataHandler;
	DEVICE_CH_IDENTIFIER mChannelIdentifier;
	
	CString mVoltageFormat;
};


