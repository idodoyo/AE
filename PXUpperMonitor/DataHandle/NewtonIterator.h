#pragma once

#include "DataTypes.h"

template< HSInt Dim >
class INewtonIteratorProtocol
{
public:
	virtual HSVoid GetFunctionValue( HSDouble tX0[ Dim ], HSDouble *tPoints[ Dim ], HSDouble tRes[ Dim ] ) = 0;
	virtual HSVoid GetFunctionMValue( HSDouble tX0[ Dim ], HSDouble *tPoints[ Dim ], HSDouble tRes[ Dim ][ Dim ] ) = 0;

	virtual ~INewtonIteratorProtocol(){}
};

template< HSInt Dim >
class CNewtonIterator
{
public:
	enum { MATRIX_DIM = Dim };

	HSBool GetFunctionInvMValue( HSDouble A[ MATRIX_DIM ][ MATRIX_DIM ], HSDouble B[ MATRIX_DIM ][ MATRIX_DIM ] )
	{
		const HSInt N = MATRIX_DIM;

		HSDouble tMatrix[ N ][ N ];

		/* 将A矩阵存放在临时矩阵中, 初始化B矩阵为单位阵 */
		for ( HSInt i = 0; i < N; i++ )        
		{        
			for ( HSInt j = 0; j < N; j++)    
			{         
				tMatrix[ i ][ j ] = A[ i ][ j ];
				B[ i ][ j ] = ( i == j ) ? 1 : 0; 
			}  
		}

		HSDouble tTmpValue = 0;
		for ( HSInt i = 0; i < N; i++)    
		{        
			/* 寻找主元 */
			HSDouble tMax = tMatrix[ i ][ i ];     
			HSInt k = i;       
			for ( HSInt j = i + 1; j < N; j++ )      
			{         
				if ( fabs( tMatrix[ j ][ i ] ) > fabs( tMax ) )   
				{           
					tMax = tMatrix[ j ][ i ];    
					k = j;          
				}       
			}        

			/* 如果主元所在行不是第i行，进行行交换   */
			if ( k != i )  
			{          
				for ( HSInt j = 0; j < N; j++ ) 
				{            
					tTmpValue = tMatrix[ i ][ j ];  
					tMatrix[ i ][ j ] = tMatrix[ k ][ j ];   
					tMatrix[ k ][ j ] = tTmpValue;          

					/* B伴随交换 */
					tTmpValue = B[ i ][ j ];      
					B[ i ][ j ] = B[ k ][ j ];      
					B[ k ][ j ] = tTmpValue;      
				}      
			}        

			/* 判断主元是否为0, 若是, 则矩阵A不是满秩矩阵,不存在逆矩阵 */
			if ( tMatrix[ i ][ i ] == 0 )       
			{            			
				return HSFalse;     
			}         

			/* 消去A的第i列除去i行以外的各行元素 */
			tTmpValue = tMatrix[ i ][ i ];        
			for ( HSInt j = 0; j < N; j++ )    
			{             
				tMatrix[ i ][ j ] = tMatrix[ i ][ j ] / tTmpValue;        /* 主对角线上的元素变为1 */
				B[ i ][ j ] = B[ i ][ j ] / tTmpValue;        /* 伴随计算 */      
			}      

			for ( HSInt j = 0; j < N; j++ )
			{           
				if ( j != i )
				{                
					tTmpValue = tMatrix[ j ][ i ];              
					for ( HSInt k = 0; k < N; k++ )
					{                    
						tMatrix[ j ][ k ] = tMatrix[ j ][ k ] - tMatrix[ i ][ k ] * tTmpValue;    
						B[ j ][ k ] = B[ j ][ k ] - B[ i ][ k ] * tTmpValue;         
					}        
				}       
			}     
		}   

		return HSTrue;	
	}

	HSDouble GetIteratorValue( HSDouble tX0[ MATRIX_DIM ], HSDouble tFunc[ MATRIX_DIM ], HSDouble tFuncM[ MATRIX_DIM ][ MATRIX_DIM ], HSDouble tX1[ MATRIX_DIM ] )
	{
		HSDouble tValue = 0;
		HSDouble tDelta = 0;
		for ( HSInt i = 0; i < MATRIX_DIM; i++ )
		{
			tValue = 0;
			for ( HSInt j = 0; j < MATRIX_DIM; j++ )
			{
				tValue += tFuncM[ i ][ j ] * tFunc[ j ];
			}

			tX1[ i ] = tX0[ i ] - tValue;
			tDelta += abs( tX1[ i ] - tX0[ i ] );
		}	

		return tDelta;
	}	
	
	HSBool StartIterator( HSDouble tX0[ MATRIX_DIM ], HSDouble *tPoints[ MATRIX_DIM ], HSDouble tX1[ MATRIX_DIM ], INewtonIteratorProtocol< MATRIX_DIM > *pCalculator )
	{
		HSDouble tResFunc[ MATRIX_DIM ] = { 0 };
		HSDouble tResFuncM[ MATRIX_DIM ][ MATRIX_DIM ] = { 0 };
		HSDouble tResInvFuncM[ MATRIX_DIM ][ MATRIX_DIM ] = { 0 };

		HSInt tIndex = 0;
		HSDouble tDelta = 100;
		while ( tIndex < 10 && tDelta > 0.01 )
		{
			pCalculator->GetFunctionValue( tX0, tPoints, tResFunc );
			pCalculator->GetFunctionMValue( tX0, tPoints, tResFuncM );

			if ( !GetFunctionInvMValue( tResFuncM, tResInvFuncM ) )
			{
				return HSFalse;
			}				

			tDelta = GetIteratorValue( tX0, tResFunc, tResInvFuncM, tX1 );		

			for ( HSInt i = 0; i < MATRIX_DIM; i++ )
			{
				tX0[ i ] = tX1[ i ];
			}			

			tIndex++;
		}			

		if ( tDelta > 0.1 )
		{
			return HSFalse;
		}

		return HSTrue;
	}
};