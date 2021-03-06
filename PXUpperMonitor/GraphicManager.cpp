#include "StdAfx.h"
#include "GraphicManager.h"
#include "GraphicWave.h"
#include "ArgTable.h"
#include "GraphicRelation.h"
#include "GraphicScatter.h"
#include "LinearPosition.h"
#include "PlanePosition.h"
#include "SpherePosition.h"
#include "CylinderPosition.h"
#include "CylinderSurfacePosition.h"
#include "GraphicFFT.h"

CGraphicManager::CGraphicManager()
{
	Init();

	HIT_LEVEL_INFO tHitLevel1 = { RGB( 6, 251, 2 ), "A", 1000 };
	mHitLevels.push_back( tHitLevel1 );

	HIT_LEVEL_INFO tHitLevel2 = { RGB( 0, 2, 251 ), "B", 2000 };
	mHitLevels.push_back( tHitLevel2 );

	HIT_LEVEL_INFO tHitLevel3 = { RGB( 238, 0, 137 ), "C", 3000 };
	mHitLevels.push_back( tHitLevel3 );	

	HIT_LEVEL_INFO tHitLevel4 = { RGB( 255, 255, 0 ), "D", 4000 };
	mHitLevels.push_back( tHitLevel4 );	

	HIT_LEVEL_INFO tHitLevel5 = { RGB( 255, 0, 0 ), "E", 5000 };
	mHitLevels.push_back( tHitLevel5 );	
}

CGraphicManager::~CGraphicManager()
{
	map< HSString, IGraphicProtocol * >::iterator pIterator = mGraphics.begin();
	while ( pIterator != mGraphics.end() )
	{
		delete pIterator->second;
		pIterator++;
	}	
}

void CGraphicManager::Init()
{
	IGraphicProtocol * pGraphicPointer = CGraphicPointer::CreateProto();
	mGraphics[ pGraphicPointer->Identifier() ] = pGraphicPointer;
	mOrderedGraphics.push_back( pGraphicPointer );	

	IGraphicProtocol * pGraphicWave = CGraphicWave::CreateProto();
	mGraphics[ pGraphicWave->Identifier() ] = pGraphicWave;	
	mOrderedGraphics.push_back( pGraphicWave );

	IGraphicProtocol * pWaveArg = CArgTable::CreateProto();
	mGraphics[ pWaveArg->Identifier() ] = pWaveArg;		
	mOrderedGraphics.push_back( pWaveArg );

	IGraphicProtocol * pGraphicRelation = CGraphicRelation::CreateProto();
	mGraphics[ pGraphicRelation->Identifier() ] = pGraphicRelation;	
	mOrderedGraphics.push_back( pGraphicRelation );

	IGraphicProtocol * pGraphicScatter = CGraphicScatter::CreateProto();
	mGraphics[ pGraphicScatter->Identifier() ] = pGraphicScatter;
	mOrderedGraphics.push_back( pGraphicScatter );

	IGraphicProtocol * pGraphicFFT = CGraphicFFT::CreateProto();
	mGraphics[ pGraphicFFT->Identifier() ] = pGraphicFFT;
	mOrderedGraphics.push_back( pGraphicFFT );

	IGraphicProtocol * pGraphicLinearPosition = CLinearPosition::CreateProto();
	mGraphics[ pGraphicLinearPosition->Identifier() ] = pGraphicLinearPosition;
	mOrderedGraphics.push_back( pGraphicLinearPosition );

	IGraphicProtocol * pGraphicPlanePosition = CPlanePosition::CreateProto();
	mGraphics[ pGraphicPlanePosition->Identifier() ] = pGraphicPlanePosition;
	mOrderedGraphics.push_back( pGraphicPlanePosition );

	IGraphicProtocol * pGraphicSpherePosition = CSpherePosition::CreateProto();
	mGraphics[ pGraphicSpherePosition->Identifier() ] = pGraphicSpherePosition;
	mOrderedGraphics.push_back( pGraphicSpherePosition );

	IGraphicProtocol * pGraphicCylinderPosition = CCylinderPosition::CreateProto();
	mGraphics[ pGraphicCylinderPosition->Identifier() ] = pGraphicCylinderPosition;
	mOrderedGraphics.push_back( pGraphicCylinderPosition );

	IGraphicProtocol * pGraphicCylinderSurfacePosition = CCylinderSurfacePosition::CreateProto();
	mGraphics[ pGraphicCylinderSurfacePosition->Identifier() ] = pGraphicCylinderSurfacePosition;
	mOrderedGraphics.push_back( pGraphicCylinderSurfacePosition );
}

HSInt CGraphicManager::Count()
{
	return mGraphics.size();
}

IGraphicProtocol * CGraphicManager::GraphicWithIndex( HSInt tIndex )
{
	if ( ( HSUInt )tIndex < mOrderedGraphics.size() )
	{
		return mOrderedGraphics[ tIndex ];			
	}

	return NULL;	
}

IGraphicProtocol * CGraphicManager::GraphicWithIdentifier( HSString tIdentifier )
{
	map< HSString, IGraphicProtocol * >::iterator pIterator = mGraphics.find( tIdentifier );
	if ( pIterator == mGraphics.end() )
	{
		return NULL;
	}

	return pIterator->second->Clone();
}

COLORREF CGraphicManager::ColorWithEnergy( HSDouble tEnergy )
{
	for ( HSUInt i = 0; i < mHitLevels.size(); i++ )
	{
		if ( tEnergy < mHitLevels[ i ].Energy )
		{
			return mHitLevels[ i ].Color;
		}
	}

	return RGB( 255, 0, 0 );
}

CGraphicManager * CGraphicManager::SharedInstance()
{
	static CGraphicManager sGraphicManager;
	return &sGraphicManager;
}