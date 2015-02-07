#pragma once

#include "StaticTitle.h"

class CStaticArg : public CStaticTitle
{
public:
	CStaticArg();
	~CStaticArg();

public:
	virtual void Draw( CDC *pDC );
	bool DrawTemplate( CDC *pMemDC, CRect &tRect );

};

