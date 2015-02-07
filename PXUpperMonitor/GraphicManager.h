#pragma once

#include "DataTypes.h"
#include "resource.h"

#include "DataHandler.h"
#include "INIConfig.h"
#include "DataHandler.h"

class IGraphicWindowProtocol
{
public:
	virtual void BeginMoveSubView( CWnd *pWnd, UINT tFlags, CPoint tPoint ) = 0;
	virtual void MoveingSubView( UINT tFlags, CPoint tPoint ) = 0;
	virtual void EndMoveSubView( UINT tFlags, CPoint tPoint ) = 0;
	virtual void SubViewClosed( CWnd *pWnd ) = 0;
	virtual void SubViewFullScreen( CWnd *pWnd, HSBool tIsFullScreen ) = 0;

	virtual ~IGraphicWindowProtocol(){}
};

class IGraphicProtocol;

class IGraphicPropertyProtocol
{
public:	
	virtual void SetParent( IGraphicProtocol *pGraphic ) = 0;
	virtual CDialogEx * Wnd() = 0;
	virtual void RefreshDataSource() = 0;

	virtual ~IGraphicPropertyProtocol(){}
};

class IGraphicProtocol
{
public:
	virtual DEVICE_CH_IDENTIFIER DataIdentifier() = 0;
	virtual HSString Identifier() = 0;
	virtual HSString Name() = 0;
	virtual HSBool Start() = 0;
	virtual HSVoid Pause() = 0;
	virtual HSVoid Stop() = 0;

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup ) = 0;
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup ) = 0;

	virtual HSBool ShowFullScreen( HSBool tFullScreen = HSFalse, CWnd *pWnd = NULL ) = 0;
	virtual HSVoid FocusGraphic( HSBool tIsFocused ) = 0;
	virtual CDialog * Wnd() = 0;
	virtual HSInt LargeIcon() = 0;
	virtual HSInt SmallIcon() = 0;	
	virtual HSInt ResourceID() = 0;	
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent ) = 0;	

	virtual IGraphicProtocol * Clone() = 0;	

	virtual HSVoid SetParent( IGraphicWindowProtocol *pParent ) = 0;
	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier ) = 0;

	virtual ~IGraphicProtocol(){}
};

class CGraphicManager
{
public:
	CGraphicManager();
	~CGraphicManager();

public:
	typedef struct HIT_LEVEL_INFO
	{
		COLORREF Color;
		HSString Desc;
		HSDouble Energy;
	} HIT_LEVEL_INFO;

public:
	HSInt Count();
	IGraphicProtocol * GraphicWithIndex( HSInt tIndex );
	IGraphicProtocol * GraphicWithIdentifier( HSString tIdentifier );

	vector< HIT_LEVEL_INFO > * HitLevels(){ return &mHitLevels; }

	COLORREF ColorWithEnergy( HSDouble tEnergy );

public:
	static CGraphicManager * SharedInstance();
	
private:
	void Init();

private:
	map< HSString, IGraphicProtocol * > mGraphics;

	vector< IGraphicProtocol * > mOrderedGraphics;

	vector< HIT_LEVEL_INFO > mHitLevels;	
};

class CGraphicPointer : public IGraphicProtocol
{
public:
	virtual DEVICE_CH_IDENTIFIER DataIdentifier(){ return 0; }
	virtual HSString Identifier(){ return "GRAPHIC_APOINTER"; }
	virtual HSString Name(){ return "÷∏’Î"; }
	virtual HSBool Start(){ return HSTrue; }
	virtual HSVoid Pause(){}
	virtual HSVoid Stop(){}

	virtual HSBool Save( CINIConfig *pIniConfig, string tGroup ){ return HSTrue; }
	virtual HSBool Load( CINIConfig *pIniConfig, string tGroup ){ return HSTrue; }

	virtual HSBool ShowFullScreen( HSBool tFullScreen = HSFalse, CWnd *pWnd = NULL ){ return HSFalse; }
	virtual HSVoid FocusGraphic( HSBool tIsFocused ){}
	virtual CDialog * Wnd(){ return NULL; }
	virtual HSInt LargeIcon(){ return IDB_BITMAP_ARROW; }
	virtual HSInt SmallIcon(){ return IDB_BITMAP_ARROW_LIST; }
	virtual HSInt ResourceID(){ return 0; }
	virtual IGraphicPropertyProtocol * CreatePropertyDlg( CWnd *pParent ){ return NULL; }

	virtual IGraphicProtocol * Clone(){ return NULL; }

	virtual HSVoid SetParent( IGraphicWindowProtocol *pParent ){}
	virtual HSVoid SetDataIdentifier( DEVICE_CH_IDENTIFIER tIdentifier ){}

	static IGraphicProtocol * CreateProto(){ return new CGraphicPointer; }
};

static HSVoid DataHandlerWithListIndex( HSInt tIndex, DEVICE_CH_IDENTIFIER &tChannelIdentifier, map< DEVICE_CH_IDENTIFIER, vector< HSInt > > &tDeviceChannelInfo )
{
	HSUInt tChannelIndex = 0;
	map< DEVICE_CH_IDENTIFIER, vector< HSInt > >::iterator pIterator = tDeviceChannelInfo.begin();
	while ( pIterator != tDeviceChannelInfo.end() )
	{			
		if ( pIterator->second.size() > ( HSUInt )tIndex )
		{
			tChannelIndex = tIndex;	
			break;
		}	

		tIndex -= pIterator->second.size();

		pIterator++;				
	}	
	
	tChannelIdentifier = DEVICE_CH_IDENTIFIER( pIterator->first.CARD_IDENTIFIER, pIterator->first.CARD_INDEX, pIterator->first.TYPE );
	if ( ( pIterator->second )[ tChannelIndex ] == 1016 )
	{
		tChannelIdentifier.InitChannels( pIterator->second );
	}
	else
	{
		tChannelIdentifier.InitChannel( ( pIterator->second )[ tChannelIndex ] );	
	}
}

static HSInt IndexWithString( CString tValue, CMFCPropertyGridProperty* pProp )
{
	int tCount = pProp->GetOptionCount();
	for ( int i = 0; i < tCount; i++ )
	{
		if ( pProp->GetOption( i ) == tValue )
		{
			return i;
		}
	}

	return -1;
}

static CString GetStrVoltage( HSDouble tVoltage )
{
	CString tStr;
	if ( tVoltage < 0.001 )
	{
		tStr.Format( "%.3f uv", tVoltage * 1000 * 1000 );
	}
	else if ( tVoltage < 1 )
	{
		tStr.Format( "%.3f mv", tVoltage * 1000 );
	}
	else
	{
		tStr.Format( "%.3f v", tVoltage );
	}

	return tStr;
}