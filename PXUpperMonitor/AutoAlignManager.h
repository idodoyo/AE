#pragma once

#include "DataTypes.h"

class CAutoAlignManager
{
public:
	enum { ALIGN_KEEP_PERCENT = 1, ALIGN_KEEP_DISTANCE = 2  };
	
private:
	enum { ALIGN_INIT_SET = 4, ALIGN_LEFT = 8, ALIGN_RIGHT = 16, ALIGN_TOP = 32, ALIGN_BOTTOM = 64  };

public:
	CAutoAlignManager( CWnd *pParent, CRect tMinRect = CRect( 0, 0, 0, 0 ) );
	~CAutoAlignManager();

	HSVoid AddControl( HSUInt tID, CRect tMargin, HSBool tInitControlPos = HSTrue, HSUInt tAlignType = ALIGN_KEEP_DISTANCE );
	HSVoid Resize();
	
	CRect ViewRect();
	HSInt ViewWidth(){ return ViewRect().Width(); }
	HSInt ViewHeight(){ return ViewRect().Height(); }

private:
	CRect GetNewRect( CRect &tControlRect, CRect &tViewRect, CRect &tInitRect, HSUInt tAlignType );
	HSInt GetPositionWithType( HSInt tPoint, HSInt tInitPoint, HSDouble tPrevLength, HSDouble tNewLength, HSUInt tAlignType, HSBool tNeedReverse = HSFalse );

private:
	CWnd *mParent;

	typedef struct CONTROL_INFO
	{		
		HSUInt ID;
		HSUInt AlignType;
	} CONTROL_INFO;

	list< CONTROL_INFO > mControls;

	CRect mPrevViewRect;
	CRect mMinRect;
};

