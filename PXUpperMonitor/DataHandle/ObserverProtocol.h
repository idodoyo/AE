#ifndef OBSERVER_PROTOCOL_HEADER_FILE
#define OBSERVER_PROTOCOL_HEADER_FILE

#include "DataTypes.h"

typedef enum { OBSERVER_REAL_TIME, OBSERVER_FINAL_POINT, OBSERVER_BACKGROUND } OBSERVER_TYPE;

class IObserverProtocol
{
public:
	virtual HSVoid Update( HSChar *pBuf, HSInt tLength ) = 0;	
	virtual HSLong FinalSize() const = 0;
	virtual OBSERVER_TYPE ObserverType() const = 0;		

	virtual ~IObserverProtocol(){}
};

#endif